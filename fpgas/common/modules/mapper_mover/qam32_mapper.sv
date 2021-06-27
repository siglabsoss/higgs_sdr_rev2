module qam32_mapper
  (
   input wire [31:0]  t0_data,
   input wire         t0_last,
   input wire         t0_valid,
   output reg         t0_ready,
   output wire [31:0] i_data,
   output wire        i_valid,
   input wire         i_ready,
   input wire         clk, 
   input wire         rstf
   );

   logic [2:0]   t_mem_addr;
   logic [31:0]  t_mem_data;
   logic         t_mem_we;
   logic         t_mem_valid;
   logic         t_mem_ready;
   
   logic [31:0] i_mem_data;
   logic i_mem_valid, i_mem_ready;

   logic [31:0] t_data;
   logic        t_last;
   logic        t_valid;
   logic        t_ready;


    eb2a #(
       .T_0_WIDTH(32+1),
       .I_0_WIDTH(32+1)
    )inbuf(
       .clk(clk),
       .reset_n(rstf),
       .t0_data({t0_last,t0_data}),
       .t0_valid(t0_valid),
       .t0_ready(t0_ready),
       .i0_data({t_last,t_data}),
       .i0_valid(t_valid),
       .i0_ready(t_ready)
    );


   mapper_memory
     #(.DEPTH(32),
`ifdef QAM32_MIF
       .MEMINIT(`QAM32_MIF)
`else
       .MEMINIT("qam32.mif")
`endif
)
    map_mem
     (.t_addr(t_mem_addr),
      .t_we(t_mem_we),
      .t_data(t_mem_data),
      .t_valid(t_mem_valid),
      .t_ready(t_mem_ready),
      .i_data(i_mem_data),
      .i_valid(i_mem_valid),
      .i_ready(i_mem_ready),
      .clk(clk),
      .rstf(rstf)
      );

      eb2a #(
         .T_0_WIDTH(32),
         .I_0_WIDTH(32)
      )buffer(
         .clk(clk),
         .reset_n(rstf),
         .t0_data(i_mem_data),
         .t0_valid(i_mem_valid),
         .t0_ready(i_mem_ready),
         .i0_data(i_data),
         .i0_valid(i_valid),
         .i0_ready(i_ready)
      );

   logic [63:0]  n_raw_data, q_raw_data;
   logic [2:0]   n_rem, q_rem;
   logic [3:0]   n_cnt, q_cnt;
   logic         n_last, q_last;
   
   enum          logic[2:0] {RST, IDLE, MAP, LAST_REM} q_state, n_state;

   //eb1 mapper_mem
   

   always @(*) begin
      t_mem_data = 0;
      t_mem_we = 0;
      n_raw_data = q_raw_data;
      n_rem = q_rem;
      n_cnt = q_cnt;
      n_last = q_last;
      t_ready = 0;
      t_mem_valid = 0;
      n_state = q_state;
      case(q_state)
        RST: begin
           t_ready = 0;
           n_state = IDLE;
        end
        IDLE: begin
           n_raw_data[31:0] = t_data;
           n_last = t_last;
           n_cnt = 0;
           n_rem = 0;
           if(t_valid) begin
              n_state = MAP;
              t_ready = 1;
           end
        end
        MAP: begin
           t_mem_addr = q_raw_data[4:0];
           t_mem_valid = 1;
           if(q_cnt == 5 && q_rem == 0 && t_mem_ready) begin
              /* verilator lint_off WIDTH */
              t_mem_valid = q_last ? 1:t_valid;
              if(q_last) begin
                 n_state = LAST_REM;
                 n_raw_data = q_raw_data >> 5;
              end
              else if(t_valid) begin
                 n_raw_data = q_raw_data >> 5 | t_data << 2;
                 t_ready = 1;
                 n_last = t_last;
                 n_cnt = 0;
                 n_rem = 1;
              end
           end
           else if(q_cnt == 5 && q_rem == 1 && t_mem_ready) begin
              t_mem_valid = q_last ? 1:t_valid;
              if(q_last) begin
                 n_state = LAST_REM;
                 n_raw_data = q_raw_data >> 5;
              end
              else if(t_valid) begin
                 n_raw_data = q_raw_data >> 5 | t_data << 4;
                 t_ready = 1;
                 n_last = t_last;
                 n_cnt = 0;
                 n_rem = 2;
              end
           end // if (q_cnt == 5 && q_rem == 1 && t_mem_ready)
           else if(q_cnt == 6 && q_rem == 2 && t_mem_ready) begin
              t_mem_valid = q_last ? 1:t_valid;
              if(q_last) begin
                 n_state = LAST_REM;
                 n_raw_data = q_raw_data >> 5;
              end
              else if(t_valid) begin
                 n_raw_data = q_raw_data >> 5 | t_data << 1;
                 t_ready = 1;
                 n_last = t_last;
                 n_cnt = 0;
                 n_rem = 3;
              end
           end // if (q_cnt == 6 && q_rem == 2 && t_mem_ready)
           else if(q_cnt == 5 && q_rem == 3 && t_mem_ready) begin
              t_mem_valid = q_last ? 1:t_valid;
              if(q_last) begin
                 n_state = LAST_REM;
                 n_raw_data = q_raw_data >> 5;
              end
              else if(t_valid) begin
                 n_raw_data = q_raw_data >> 5 | t_data << 3;
                 t_ready = 1;
                 n_last = t_last;
                 n_cnt = 0;
                 n_rem = 4;
              end
           end
           else if (q_cnt == 7 && q_rem == 4 && t_mem_ready) begin
              t_mem_valid = q_last ? 1:t_valid;
              if(q_last)
                n_state = IDLE;
              else if(t_valid) begin
                 n_raw_data = q_raw_data >> 5 | t_data;
                 t_ready = 1;
                 n_last = t_last;
                 n_cnt = 0;
                 n_rem = 0;
              end
           end
           else begin
              if(t_mem_ready) begin
                 n_cnt = q_cnt +1;
                 n_raw_data = q_raw_data >> 5;
              end
           end
           
        end // case: MAP
        LAST_REM: begin
           t_mem_addr = q_raw_data[4:0];
           t_mem_valid = 1;
           if(t_mem_ready)
             n_state = IDLE;
        end
        default:
          n_state = RST;
      endcase // case (q_state)
   end // always @ (*)
   

   always @(posedge clk or negedge rstf) begin
      if(~rstf) begin
         q_state <= RST;
         q_cnt <= 0;
         q_raw_data <= 0;
         q_last <= 0;
         q_rem <= 0;
      end
      else begin
         q_state <= n_state;
         q_cnt <= n_cnt;
         q_raw_data <= n_raw_data;
         q_last <= n_last;
         q_rem <= n_rem;
      end
   end
   
endmodule
