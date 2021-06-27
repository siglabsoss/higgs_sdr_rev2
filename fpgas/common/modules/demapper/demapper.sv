module demapper
  (
   input [31:0]  t_data,
   input wire        t_last,
   input wire        t_valid,
   output reg       t_ready,
   output wire [31:0] i_data,
   output logic        i_last,
   output logic        i_valid,
   input wire         i_ready,
   input wire [3:0]   constellation,
   input wire [15:0]  two_over_sigma_sq,
   input wire        clk,
   input wire        rstf
   );

   enum          logic [2:0] {IDLE,
                              QPSK,
                              PSK8,
                              QAM16,
                              QAM64} q_state, n_state;

   logic [2:0]   q_stage, n_stage;
   logic [31:0]  demap_data;
   logic         demap_last;
   logic         demap_valid;
   //logic         mult_ready;

   function logic[15:0] twos_complement(logic[15:0] din);
      return ~din + 1'b1;
   endfunction

   function logic[15:0] absolute(logic[15:0] din);
      return din[15] ? twos_complement(din):din;
   endfunction // absolute

   /////////// MULTIPLIER ///////////////
   logic          mult_ready;
   logic         data_en;
   logic [31:0]   mult_real, mult_imag;
   
   assign mult_ready = ~i_valid | i_ready;
   assign data_en = demap_valid & mult_ready;

   assign i_data = {mult_imag[31-4:16-4],mult_real[31-4:16-4]};
   
   always @(posedge clk or negedge rstf) begin
      if(~rstf) begin
         mult_real <= 0;
         mult_imag <= 0;
         i_valid <= 0;
         i_last <= 0;
      end
      else begin
         i_valid <= ~mult_ready | demap_valid;
         if(data_en) begin
            mult_real <= demap_data[15:0] * two_over_sigma_sq;
            mult_imag <= demap_data[31:16] * two_over_sigma_sq;
            i_last <= demap_last;
         end
      end
   end // always @ (posedge ck or negedge rstf)
   ///////////////////////////////////////////

   logic [15:0]  q_psk8_stage1, n_psk8_stage1;
   
   logic [15:0]  q_qam16_stage1_real, n_qam16_stage1_real;
   logic [15:0]  q_qam16_stage1_imag, n_qam16_stage1_imag;

   logic [15:0]  q_qam64_stage1_real, n_qam64_stage1_real;
   logic [15:0]  q_qam64_stage1_imag, n_qam64_stage1_imag;
   logic [15:0]  q_qam64_stage2_real, n_qam64_stage2_real;
   logic [15:0]  q_qam64_stage2_imag, n_qam64_stage2_imag;
   
   
   always_comb begin
      n_psk8_stage1 = absolute(t_data[15:0]) - absolute(t_data[31:16]);

      n_qam16_stage1_real = absolute(t_data[15:0]) - 16'h2000;

      n_qam16_stage1_imag = absolute(t_data[31:16]) - 16'h2000;

      n_qam64_stage1_real = 16'h4000 + twos_complement(absolute(t_data[15:0]));

      n_qam64_stage1_imag = 16'h4000 + twos_complement(absolute(t_data[31:16]));

      n_qam64_stage2_real = twos_complement(absolute((absolute(t_data[15:0] - 16'h4000)))) + 16'h2000;

      n_qam64_stage2_imag = twos_complement(absolute((absolute(t_data[31:16] - 16'h4000)))) + 16'h2000;
      
      n_state = q_state;
      demap_data = 0;
      demap_valid = 0;
      t_ready = 0;
      demap_last =0;

      n_stage = q_stage;
      
      case(q_state)
        IDLE: begin
           n_stage = 0;
           if(t_valid)
             n_state = (constellation == 0) ? QPSK:
                       (constellation == 1) ? PSK8:
                       (constellation == 2) ? QAM16:
                       (constellation == 3) ? QAM64:IDLE;
        end
        QPSK: begin
           demap_data = t_data;
           demap_valid = t_valid;
           demap_last = t_last;
           t_ready = mult_ready;
           if (t_valid & t_ready & t_last)
             n_state = IDLE;
        end
        PSK8: begin
           demap_data = (q_stage == 0) ? t_data:{16'h0, q_psk8_stage1};
           demap_valid = t_valid;
           t_ready = (q_stage == 1) ? mult_ready:0;
           if(demap_valid && mult_ready) begin
              n_stage = q_stage + 1;
              if(q_stage == 1) begin
                 n_stage = 0;
                 if(t_last) begin
                   n_state = IDLE;
                    demap_last = 1;
                 end
              end
           end
        end
        QAM16: begin
           demap_data = (q_stage == 0) ? t_data:{q_qam16_stage1_imag, q_qam16_stage1_real};
           demap_valid = t_valid;
           t_ready = (q_stage == 1) ? mult_ready:0;
           if(demap_valid && mult_ready) begin
              n_stage = q_stage + 1;
              if(q_stage == 1) begin
                 n_stage = 0;
                 if(t_last) begin
                    n_state = IDLE;
                    demap_last = 1;
                 end
              end
           end
        end
        QAM64: begin
           demap_data = (q_stage == 0) ? t_data:
                        (q_stage == 1) ? {q_qam64_stage1_imag, q_qam64_stage1_real}:
                        {q_qam64_stage2_imag, q_qam64_stage2_real};
           demap_valid = t_valid;
           t_ready = (q_stage == 2) ? mult_ready:0;
           if(demap_valid && mult_ready) begin
              n_stage = q_stage + 1;
              if(q_stage == 2) begin
                 n_stage = 0;
                 if(t_last) begin
                    n_state = IDLE;
                    demap_last = 1;
                 end
              end
           end
        end
        default:
          n_state = IDLE;
      endcase // case (q_state)
      
   end // always_comb
   

   always @(posedge clk or negedge rstf) begin
      if(~rstf) begin
         q_state <= IDLE;
         q_stage <= 0;
         
         q_psk8_stage1 <= 0;
         
         q_qam16_stage1_real <= 0;
         q_qam16_stage1_imag <= 0;

         q_qam64_stage1_real <= 0;
         q_qam64_stage1_imag <= 0;
         q_qam64_stage2_real <= 0;
         q_qam64_stage2_imag <= 0;
      end
      else begin
         q_state <= n_state;
         q_stage <= n_stage;
         
         q_psk8_stage1 <= n_psk8_stage1;
         
         q_qam16_stage1_real <= n_qam16_stage1_real;
         q_qam16_stage1_imag <= n_qam16_stage1_imag;

         q_qam64_stage1_real <= n_qam64_stage1_real;
         q_qam64_stage1_imag <= n_qam64_stage1_imag;
         q_qam64_stage2_real <= n_qam64_stage2_real;
         q_qam64_stage2_imag <= n_qam64_stage2_imag;
      end
   end
   
endmodule // demapper

