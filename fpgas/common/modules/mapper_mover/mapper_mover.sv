module mapper_mover
  (input wire[31:0] t_data,
   input wire          t_valid,
   output reg          t_ready,
   output reg [31:0]   i_data,
   output reg          i_last,
   output reg          i_valid,
   input wire          i_ready,
   input wire [1023:0] mover_active,
   input wire [9:0]    trim_start,
   input wire [9:0]    trim_end,
   input wire [9:0]    pilot_ram_addr,
   input wire [31:0]   pilot_ram_wdata,
   input wire          pilot_ram_we,
   input wire          mapmov_reset,
   input wire          clk, rst
   );

   localparam logic [3:0] QPSK = 0;
   localparam logic [3:0] QAM8 = 1;
   localparam logic [3:0] QAM16 = 2;
   localparam logic [3:0] QAM32 = 3;
   localparam logic [3:0] QAM64 = 4;
   localparam logic [3:0] QAM128 = 5;
   
   logic [3:0]         n_constellation, q_constellation;
   

   function automatic reg[31:0] map(reg[31:0] data, reg[31:0] cnt);
      reg [15:0]       data_l = {data[30],data[28],data[26],data[24],data[22],data[20],data[18],data[16],data[14],data[12],data[10],data[8],data[6],data[4],data[2],data[0]};//data[15:0];
      reg [15:0]       data_h = {data[31],data[29],data[27],data[25],data[23],data[21],data[19],data[17],data[15],data[13],data[11],data[9],data[7],data[5],data[3],data[1]};//data[31:16];

      map[15:0]=data_h[cnt[3:0]]?'h16a1:'he95f;
      map[31:16]=data_l[cnt[3:0]]?'h16a1:'he95f;
   endfunction // map

   logic [31:0]        pilot_ram[0:1023] /* synthesis syn_ramstyle = "block_ram */ ;


   

   logic [31:0]        q_type, n_type;
   logic [31:0]        q_length, n_length;
   logic [31:0]        q_word_2, n_word_2;
   logic [31:0]        q_word_3, n_word_3;
   logic [31:0]        q_vector_type, n_vector_type;
   logic [31:0]        q_cnt, n_cnt;
   logic [9:0]         q_trim_cnt, n_trim_cnt;
   logic [9:0]         q_trim_idx, n_trim_idx;
   logic [3:0]         q_map_cnt, n_map_cnt;
   logic               q_subcarrier_active_reg, n_subcarrier_active_reg;
   logic [31:0]        pilot_ram_rdata;

   logic [31:0]        q_pkt_cnt, n_pkt_cnt;
   logic [31:0]        q_pkt_length, n_pkt_length;
   

   enum                logic[3:0] {GET_TYPE, //0
                                   GET_LENGTH, //1
                                   GET_2, //2
                                   GET_3, //3
                                   GET_VECTOR_TYPE, //4
                                   SEND_TYPE, //5
                                   SEND_LENGTH, //6
                                   SEND_2, //7
                                   SEND_3, //8
                                   SEND_VECTOR_TYPE, //9
                                   SEND_HEADER, //A
                                   SEND_DATA, //B
                                   SEND_MAPMOV_DATA //C
                                   } q_state,n_state;



   logic [31:0]        t_map_data;
   logic               t_map_last;
   logic               t_map_valid;
   wire               t_map_ready;
   wire [31:0]        i_map_data;
   wire               i_map_valid;
   logic               i_map_ready;

   
   wire               t_qam8_valid;
   logic               t_qam8_ready;
   logic [31:0]        i_qam8_data;
   logic               i_qam8_valid;
   logic               qam_rstf;
   
   qam8_mapper qam8
     (.t0_data(t_map_data),
      .t0_last(t_map_last),
      .t0_valid(t_qam8_valid),
      .t0_ready(t_qam8_ready),
      .i_data(i_qam8_data),
      .i_valid(i_qam8_valid),
      .i_ready(i_map_ready),
      .clk(clk),
      .rstf(qam_rstf)
      );

   wire               t_qam16_valid;
   logic               t_qam16_ready;
   logic [31:0]        i_qam16_data;
   logic               i_qam16_valid;
   
   qam16_mapper qam16
     (.t0_data(t_map_data),
      .t0_last(t_map_last),
      .t0_valid(t_qam16_valid),
      .t0_ready(t_qam16_ready),
      .i_data(i_qam16_data),
      .i_valid(i_qam16_valid),
      .i_ready(i_map_ready),
      .clk(clk),
      .rstf(qam_rstf)
      );

   wire               t_qam32_valid;
   logic               t_qam32_ready;
   logic [31:0]        i_qam32_data;
   logic               i_qam32_valid;
   
   qam32_mapper qam32
     (.t0_data(t_map_data),
      .t0_last(t_map_last),
      .t0_valid(t_qam32_valid),
      .t0_ready(t_qam32_ready),
      .i_data(i_qam32_data),
      .i_valid(i_qam32_valid),
      .i_ready(i_map_ready),
      .clk(clk),
      .rstf(qam_rstf)
      );

   wire               t_qam64_valid;
   logic               t_qam64_ready;
   logic [31:0]        i_qam64_data;
   logic               i_qam64_valid;
   
   qam64_mapper qam64
     (.t0_data(t_map_data),
      .t0_last(t_map_last),
      .t0_valid(t_qam64_valid),
      .t0_ready(t_qam64_ready),
      .i_data(i_qam64_data),
      .i_valid(i_qam64_valid),
      .i_ready(i_map_ready),
      .clk(clk),
      .rstf(qam_rstf)
      );

   assign t_qam8_valid = (q_state == SEND_MAPMOV_DATA && q_constellation == QAM8) ? t_map_valid:0;

   assign t_qam16_valid = (q_state == SEND_MAPMOV_DATA && q_constellation == QAM16) ? t_map_valid:0;

   assign t_qam32_valid = (q_state == SEND_MAPMOV_DATA && q_constellation == QAM32) ? t_map_valid:0;

   assign t_qam64_valid = (q_state == SEND_MAPMOV_DATA && q_constellation == QAM64) ? t_map_valid:0;
   
   assign t_map_ready = (q_state == SEND_MAPMOV_DATA && q_constellation == QAM8) ? t_qam8_ready:
                        (q_state == SEND_MAPMOV_DATA && q_constellation == QAM16) ? t_qam16_ready:
                        (q_state == SEND_MAPMOV_DATA && q_constellation == QAM32) ? t_qam32_ready:
                        (q_state == SEND_MAPMOV_DATA && q_constellation == QAM64) ? t_qam64_ready:0;

   assign i_map_data = (q_state == SEND_MAPMOV_DATA && q_constellation == QAM8) ? i_qam8_data:
                       (q_state == SEND_MAPMOV_DATA && q_constellation == QAM16) ? i_qam16_data:
                       (q_state == SEND_MAPMOV_DATA && q_constellation == QAM32) ? i_qam32_data:
                       (q_state == SEND_MAPMOV_DATA && q_constellation == QAM64) ? i_qam64_data:0;

   assign i_map_valid = (q_state == SEND_MAPMOV_DATA && q_constellation == QAM8) ? i_qam8_valid:
                        (q_state == SEND_MAPMOV_DATA && q_constellation == QAM16) ? i_qam16_valid:
                        (q_state == SEND_MAPMOV_DATA && q_constellation == QAM32) ? i_qam32_valid:
                        (q_state == SEND_MAPMOV_DATA && q_constellation == QAM64) ? i_qam64_valid:0;
   
   reg [7:0] state_name;

   always @(*) begin
      n_state = q_state;
      n_type = q_type;
      n_length = q_length;
      n_word_2 = q_word_2;
      n_word_3 = q_word_3;
      n_vector_type = q_vector_type;
      n_cnt = q_cnt;
      n_trim_cnt = q_trim_cnt;
      n_trim_idx = q_trim_idx;
      n_map_cnt = q_map_cnt;
      t_ready = i_ready;
      i_valid = t_valid;
      n_subcarrier_active_reg = q_subcarrier_active_reg;
      n_constellation = q_constellation;
      i_data=0;
      n_pkt_cnt = q_pkt_cnt;
      n_pkt_length = q_pkt_length;
      i_last = 0;
      qam_rstf = 1;
      case(q_state)
        GET_TYPE: begin
           n_type = t_data;
           i_valid=(t_data==0)?t_valid:0;
           t_ready=(t_data==0)?i_ready:1;
           i_last=(t_data==0)?1:0;
           state_name = 65; // A
           if(t_valid && t_data != 0) begin
             qam_rstf = 0;
             n_state = GET_LENGTH;
             state_name = 66; // B
           end
        end
        GET_LENGTH: begin
           n_length = t_data;
           t_ready=1;
           i_valid=0;
           state_name = 67; // C
           if(t_valid)
             n_state = GET_2;
        end
        GET_2: begin
           n_word_2 = t_data;
           t_ready=1;
           i_valid = 0;
           state_name = 68; // D
           if(t_valid)
             n_state = GET_3;
        end
        GET_3: begin
           n_word_3 = t_data;
           t_ready=1;
           i_valid = 0;
           state_name = 69; // E
           if(t_valid)
             n_state = GET_VECTOR_TYPE;
        end
        GET_VECTOR_TYPE: begin
           n_vector_type = t_data;
           t_ready=1;
           i_valid = 0;
           state_name = 70; // F
           if(t_valid)
             n_state = SEND_TYPE;
        end
        SEND_TYPE: begin
           i_data = q_type;
           i_valid = 1;
           t_ready = 0;
           state_name = 71; // G
           if(i_ready)
             n_state = SEND_LENGTH;
        end
        SEND_LENGTH: begin
           i_data = q_length;
           i_valid = 1;
           t_ready = 0;
           if(i_ready)
             n_state = SEND_2;
        end
        SEND_2: begin
           i_data = q_word_2;
           i_valid = 1;
           t_ready = 0;
           if(i_ready)
             n_state = SEND_3;
        end
        SEND_3: begin
           i_data = q_word_3;
           i_valid = 1;
           t_ready = 0;
           if(i_ready)
             n_state = SEND_VECTOR_TYPE;
        end
        SEND_VECTOR_TYPE: begin
           i_data = q_vector_type;
           i_valid = 1;
           t_ready = 0;
           n_cnt = 5;
           state_name = 72; // H
           if(i_ready)
             n_state = SEND_HEADER;
        end
        SEND_HEADER: begin
           i_data = t_data;
           n_trim_cnt=0;
           n_trim_idx=trim_start;
           n_map_cnt = 0;
           n_pkt_cnt = 0;
           state_name = 73; // I
           if(t_valid & i_ready) begin
              n_cnt = q_cnt+1;
              state_name = 74; // J
              if(q_cnt == 15 || q_cnt > q_length) begin
                n_state = (q_type == 2 && q_vector_type == 5) ? SEND_MAPMOV_DATA:SEND_DATA;
                state_name = 75; // K
              end
              if(q_cnt == 7) begin
                n_pkt_length = t_data;
                state_name = 76; // L
              end
              if(q_cnt == 8) begin
                n_constellation = t_data;
                state_name = 77; // M
              end
           end
           n_subcarrier_active_reg = mover_active[trim_start];
        end
        SEND_DATA: begin
           i_data = t_data;
           i_last = (q_cnt == q_length-1) ? 1:0; // Issue #112 was here, only affects non mapmov
           if(t_valid & i_ready) begin
              n_cnt = q_cnt+1;
              state_name = 78; // N
              if(q_cnt >= q_length-1) begin
                n_state = GET_TYPE;
                state_name = 79; // O
              end
           end
        end
        SEND_MAPMOV_DATA: begin

           case(q_constellation)
             QPSK: begin
                if(~q_subcarrier_active_reg)
                  i_valid = 1;
                else 
                  i_valid = t_valid;
                t_ready = (q_map_cnt == 'hF && q_subcarrier_active_reg) ? t_valid&i_ready:0;
                i_data = q_subcarrier_active_reg ? map(t_data,q_map_cnt):pilot_ram_rdata;//pilot_ram[q_trim_idx];
                i_last = (q_cnt == q_length - 1) ? 1:0;
                state_name = 80; // P
                
                //n_map_cnt = (q_map_cnt == 'hF && (t_valid&i_ready) ) ? 0 : q_map_cnt;
                if((t_valid | ~q_subcarrier_active_reg) & i_ready) begin
                   n_cnt = q_cnt + 1;
                   n_map_cnt = q_subcarrier_active_reg ? q_map_cnt + 1: q_map_cnt;
                   n_trim_cnt = (q_trim_idx == trim_end) ? 0:q_trim_cnt + 1;
                   n_trim_idx = (q_trim_idx == trim_end) ? trim_start:q_trim_idx + 1;
                   n_subcarrier_active_reg = mover_active[n_trim_cnt];

                   state_name = 81; // Q
                   if(q_cnt == q_length-1) begin
                     n_state = GET_TYPE;
                     state_name = 82; // R
                   end
                end
             end // case: QPSK
             default: begin // QAM
                if(~q_subcarrier_active_reg) 
                  i_valid = 1;
                else
                  i_valid = i_map_valid;
                i_map_ready = q_subcarrier_active_reg ? i_map_valid & i_ready:0;
                i_data = q_subcarrier_active_reg ? i_map_data:pilot_ram_rdata;
                i_last = (q_cnt == q_length - 1) ? 1:0;
                t_map_data = t_data;
                t_map_last = (q_pkt_cnt == q_pkt_length -1) ? 1:0;
                t_map_valid = (q_pkt_cnt == q_pkt_length) ? 0 : t_valid;
                t_ready = t_map_ready;
                if(t_valid && t_ready) begin
                  n_pkt_cnt = q_pkt_cnt + 1;
                end
                
                if((i_map_valid | ~q_subcarrier_active_reg) & i_ready) begin
                   n_cnt = q_cnt + 1;
                   n_trim_cnt = (q_trim_idx == trim_end) ? 0:q_trim_cnt + 1;
                   n_trim_idx = (q_trim_idx == trim_end) ? trim_start:q_trim_idx + 1;
                   n_subcarrier_active_reg = mover_active[n_trim_cnt];
                   if(q_cnt == q_length - 1)
                     n_state = GET_TYPE;
                end
             end // case: default
           endcase
        end // case: SEND_MAPMOV_DATA
        
        default:
          n_state = GET_TYPE;
      endcase
      
   end // always @ (*)

   

   always @(posedge clk) begin
      if(rst|mapmov_reset) begin
         q_state <= GET_TYPE;
         q_type <= 0;
         q_length <= 0;
         q_word_2 <= 0;
         q_word_3 <= 0;
         q_vector_type <= 0;
         q_cnt <= 0;
         q_trim_cnt <= 0;
         q_trim_idx <= 0;
         q_map_cnt <= 0;
         q_subcarrier_active_reg <= 0;
         q_constellation <= 0;
         q_pkt_cnt <= 0;
         q_pkt_length <= 0;
      end
      else begin
         q_state <= n_state;
         q_type <= n_type;
         q_length <= n_length;
         q_word_2 <= n_word_2;
         q_word_3 <= n_word_3;
         q_vector_type <= n_vector_type;
         q_cnt <= n_cnt;
         q_trim_cnt <= n_trim_cnt;
         q_trim_idx <= n_trim_idx;
         q_map_cnt <= n_map_cnt;
         q_subcarrier_active_reg <= n_subcarrier_active_reg;
         q_constellation <= n_constellation;
         q_pkt_cnt <= n_pkt_cnt;
         q_pkt_length <= n_pkt_length;
      end
   end // always @ (posedge clk or posedge rst)


   always @(posedge clk) begin
      pilot_ram_rdata <= pilot_ram[n_trim_idx];
      if(pilot_ram_we) 
        pilot_ram[pilot_ram_addr] <= pilot_ram_wdata;
   end
   
endmodule

