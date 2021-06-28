module mdio_setconfig
  (
   output wire mdc,
   inout wire  mdio,
   input wire  en,
   input wire  clk, rstf
   );

   
   
   enum logic [3:0] {IDLE,
                     WAIT1,
                     SET_PAGE,
                     WAIT2,
                     READ_CMD,
                     TA,
                     READ_DATA,
                     WAIT3,
                     WRITE_REG } q_state, n_state;

   logic [7:0] q_cnt, n_cnt;
   logic [31:0] q_send_data, n_send_data;
   logic [15:0] q_get_data, n_get_data;
   logic       dout, dout_en;

   logic [31:0] PAGE_PARAM = {2'b01, 2'b01, 5'b0, 5'b10110, 2'b10, 16'h2}; //start, write, phy address, reg addr, ta, reg data
   localparam logic [13:0] READ_CMD_PARAM = {2'b01, 2'b10, 5'b0, 5'b11000}; //start, write, phy address, reg addr, ta, reg data
   localparam logic [31:0] WRITE_REG_PARAM = {2'b01, 2'b01, 5'b0, 5'b11000, 2'b10, 16'h2000};
   

   assign mdio = dout_en ? dout : 'z;
   assign mdc = (q_state == IDLE) ? 0:clk;
   
   always_comb begin
      n_get_data = q_get_data;
      n_send_data = q_send_data;
      n_state = q_state;
      n_cnt = q_cnt;
      dout = 0;
      dout_en = 0;
      case(q_state)
        IDLE: begin
           n_cnt = 0;
           if(en)
             n_state = WAIT1;
        end
        WAIT1: begin
           n_send_data = PAGE_PARAM;
           n_state = SET_PAGE;
        end
        SET_PAGE: begin
           dout_en = 1;
           n_cnt = q_cnt + 1;
           dout = q_send_data[31];
           n_send_data = q_send_data << 1;
           if(q_cnt == 31)
             n_state = WAIT2;
        end
        WAIT2: begin
           n_cnt = 0;
           n_send_data = {READ_CMD_PARAM,18'b0};
           n_state = READ_CMD;
        end
        READ_CMD: begin
           dout_en = 1;
           n_cnt = q_cnt + 1;
           dout = q_send_data[31];
           n_send_data = q_send_data << 1;
           if(q_cnt == 13) begin
              n_cnt = 0;
              n_state = TA;
           end
        end
        TA: begin
           n_cnt = q_cnt + 1;
           if(q_cnt == 1) begin
              n_cnt = 0;
              n_state = READ_DATA;
           end
        end
        READ_DATA: begin
           n_get_data = {q_get_data[14:0], mdio};
           n_cnt = q_cnt + 1;
           if(q_cnt == 15) begin
              n_cnt = 0;
              n_state = WAIT3;
           end
        end
        WAIT3: begin
           n_cnt = 0;
           n_send_data = WRITE_REG_PARAM;
           n_state = WRITE_REG;
        end
        WRITE_REG: begin
           dout_en = 1;
           n_cnt = q_cnt + 1;
           dout = q_send_data[31];
           n_send_data = q_send_data << 1;
           if(q_cnt == 31)
             n_state = IDLE;
        end
        default:
          n_state = IDLE;
      endcase
   end

   always @(negedge clk or negedge rstf)  begin
      if(~rstf) begin
         q_state <= IDLE;
         q_cnt <= 0;
         q_send_data <= 0;
         q_get_data <= 0;
      end
      else begin
         q_state <= n_state;
         q_cnt <= n_cnt;
         q_send_data <= n_send_data;
         q_get_data <= n_get_data;
      end
   end


endmodule
