module fb_eq_split
  (input wire[31:0] t_data,
   input wire        t_valid,
   output reg        t_ready,

   output reg [31:0] i_fb_data,
   output reg        i_fb_valid,
   output reg        i_fb_last,
   input wire        i_fb_ready,

   output reg [31:0] i_eq_data,
   output reg        i_eq_valid,
   output reg        i_eq_last,
   input wire        i_eq_ready,

   input wire        clk, rstf
   );


   reg [31:0]      n_header[8], q_header[8];
   reg [31:0]      n_cnt, q_cnt;
   reg             n_is_eq, q_is_eq;

   reg [31:0]      s_data;
   reg             s_valid;
   reg             s_last;

   wire [31:0]      s_type, s_length, s_vector_type;
   wire             s_ready;
   

   enum                reg[3:0] {INIT, //0
                                   GET_HEADER, //1
                                   SEND_HEADER, //2
                                   SEND_DATA //3
                                   } q_state,n_state;

   assign i_eq_last = q_is_eq ? s_last:0;
   assign i_fb_last = q_is_eq ? 0:s_last;
   
   assign i_eq_valid = q_is_eq ? s_valid:0;
   assign i_fb_valid = q_is_eq ? 0:s_valid;

   assign i_eq_data = s_data;
   assign i_fb_data = s_data;

   assign s_ready = q_is_eq ? i_eq_ready:i_fb_ready;
   
   assign s_type = q_header[0];
   assign s_length = q_header[1];
   assign s_vector_type = q_header[4];

   reg [7:0]      state_name;
   
   always @(*) begin
      s_data = 0;
      s_valid = 0;
      s_last = 0;
      n_header = q_header;
      n_cnt = q_cnt;
      n_is_eq = q_is_eq;
      n_state = q_state;
      t_ready = 0;
      state_name = 65; // A

      case(q_state)
        INIT: begin
           n_cnt = 0;
           t_ready = (t_data == 0) ? 1:0;
           state_name = 66; // B
           if(t_valid && t_data != 0) begin
              n_state = GET_HEADER;
              state_name = 67; // C
           end
        end
        GET_HEADER: begin
           t_ready = 1;
           state_name = 68; // D
           if(t_valid) begin
              n_cnt = q_cnt + 1;
              n_header[q_cnt[2:0]] = t_data;
              state_name = 69; // E
              if(q_cnt[2:0] == 3'b111) begin
                 n_is_eq = (s_type == 2 && (s_vector_type == 25 || s_vector_type == 26)) ? 1:0;
                 n_cnt = 0;
                 n_state = SEND_HEADER;
                 state_name = 70; // F
              end
           end
        end
        SEND_HEADER: begin
           s_data = q_header[q_cnt[2:0]];
           s_valid = 1;
           t_ready = 0;
           state_name = 71; // G
           if(s_ready) begin
              n_cnt = q_cnt + 1;
              state_name = 72; // H
              if(q_cnt[2:0] == 3'b111) begin
                n_state = SEND_DATA;
                state_name = 73; // I
              end
           end
            
        end
        SEND_DATA: begin
           s_data = t_data;
           s_valid = t_valid;
           t_ready = q_is_eq ? i_eq_ready:i_fb_ready;
           s_last = (q_cnt == s_length-1) ? 1:0;
           state_name = 74; // J
           if(t_valid && t_ready) begin
              n_cnt = q_cnt + 1;
              state_name = 75; // K
              if(q_cnt == s_length - 1) begin
                n_state = INIT;
                state_name = 76; // L
              end
           end
        end
        default:
          n_state = INIT;
      endcase // case (q_state)
   end

   always @(posedge clk) begin
      if(!rstf) begin
         q_state <= INIT;
         q_header <= '{default:0};
         q_cnt <= 0;
         q_is_eq <= 0;
      end
      else begin
         q_state <= n_state;
         q_header <= n_header;
         q_cnt <= n_cnt;
         q_is_eq <= n_is_eq;
      end
   end // always @ (posedge clk)

`ifdef FORMAL
   
`endif
endmodule
