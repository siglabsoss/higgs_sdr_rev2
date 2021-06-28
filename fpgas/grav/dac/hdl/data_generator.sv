module data_generator 
  #(parameter DATA_ROM = "tone.mif"
    )
   (output logic[31:0] i_data,
    output logic i_valid,
    input wire   i_ready,
    input wire   clk, rstf
    );


   logic [31:0]  cooked_data[1024];

   initial begin
      $readmemh(DATA_ROM, cooked_data);
   end

   logic[9:0] q_cnt, n_cnt;
   logic[31:0] q_lfsr, n_lfsr, lfsr; //poly: 0xA3000000

   assign i_data=lfsr;

   always @* begin
      n_cnt=(i_valid & i_ready)?q_cnt+1:q_cnt;
      //i_data=cooked_data[q_cnt];
      //n_lfsr[0] = q_lfsr[31]^q_lfsr[29]^q_lfsr[25]^q_lfsr[24];
      //n_lfsr[31:1] = q_lfsr[30:0];
   end

   always @(posedge clk or negedge rstf) begin
      if(~rstf) begin
         q_cnt<=0;
         i_valid<=0;
         q_lfsr<=32'hFFFFFFFF;
         lfsr <= 32'hFFFFFFFF;
      end
      else begin
         i_valid<=1;
         q_cnt<=n_cnt;
         lfsr<=q_lfsr;
         q_lfsr<={q_lfsr[30:0],q_lfsr[31]^q_lfsr[29]^q_lfsr[25]^q_lfsr[24]};// n_lfsr;
      end
   end

endmodule

