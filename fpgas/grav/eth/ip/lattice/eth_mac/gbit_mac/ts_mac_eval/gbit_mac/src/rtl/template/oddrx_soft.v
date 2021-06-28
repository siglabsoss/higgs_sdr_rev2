
// ====================================================================
// File Details
// ====================================================================
// Project              :       TSMAC IP Core For EC2
// Filename             :       ODDRX_SFT.v
// Author               :       lattice Semiconductor
//=====================================================================
//
//* Notes:
//*
//* # Soft ODDR model for TSMAC 
//*

module oddrx_soft
(
// Primary I/O 
DA, DB, RST, CLK_I, CLK_IX2, Q
);

input DA; 
input DB; 
input CLK_I; 
input CLK_IX2; 
input RST; 
output Q;

reg  DA_f;
reg  DB_f;
reg  Q;
wire MUX_OUT;


   assign MUX_OUT = CLK_I ? DA_f:DB_f;

   always @(posedge CLK_I or posedge RST) begin
	  if (RST) begin
             DA_f <= 1'b0;
             DB_f <= 1'b0;
	  end
	  else begin
             DA_f <= DA;
             DB_f <= DB;
	  end
   end

   always @(negedge CLK_IX2 or posedge RST) begin
	  if (RST) begin
             Q <= 1'b0;
	  end
	  else begin
             Q <= MUX_OUT;
	  end
   end

endmodule
