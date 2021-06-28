//   ==================================================================
//   >>>>>>>>>>>>>>>>>>>>>>> COPYRIGHT NOTICE <<<<<<<<<<<<<<<<<<<<<<<<<
//   ------------------------------------------------------------------
//   Copyright (c) 2013 by Lattice Semiconductor Corporation
//   ALL RIGHTS RESERVED 
//   ------------------------------------------------------------------
//
//   Permission:
//
//      Lattice SG Pte. Ltd. grants permission to use this code
//      pursuant to the terms of the Lattice Reference Design License Agreement. 
//
//
//   Disclaimer:
//
//      This VHDL or Verilog source code is intended as a design reference
//      which illustrates how these types of functions can be implemented.
//      It is the user's responsibility to verify their design for
//      consistency and functionality through the use of formal
//      verification methods.  Lattice provides no warranty
//      regarding the use or functionality of this code.
//
//   --------------------------------------------------------------------
//
//                  Lattice SG Pte. Ltd.
//                  101 Thomson Road, United Square #07-02 
//                  Singapore 307591
//
//
//                  TEL: 1-800-Lattice (USA and Canada)
//                       +65-6631-2000 (Singapore)
//                       +1-503-268-8001 (other locations)
//
//                  web: http://www.latticesemi.com/
//                  email: techsupport@latticesemi.com
//
//   --------------------------------------------------------------------
//
//
//  Project:           RGMII to GMII interface
//  File:              ddr_input_ecp5.v
//  Title:             DDR
//  Description:       DDR shift register of this interface
//
// --------------------------------------------------------------------
//
// Revision History :
// --------------------------------------------------------------------
// Revision 1.0  2016-09-30 RK
// Initial revision for ECP5 design
//
//
// --------------------------------------------------------------------

`timescale 1ns/100ps

module DDR	 (rstn,
              rx_clk,
	      rx_clk_out,
              din,
              rx_ctl,
              dout_a,
              dout_b,
              rx_ctl_a,
              rx_ctl_b,
              tx_clk,
              txd,
              td,
              tx_ctl,
              tx_er,
              tx_en

          );

input        rstn;
input        rx_clk;
input [3:0]  din;
input        rx_ctl;
input        tx_clk;
input [7:0]  txd;
input        tx_er;
input        tx_en;


output[3:0]  dout_a /* synthesis syn_keep=1 */;
output[3:0]  dout_b /* synthesis syn_keep=1 */;
output       rx_ctl_a;
output       rx_ctl_b;
output[3:0]  td;
output       tx_ctl;

output 	     rx_clk_out;

wire [3:0]	ireg_pos;
wire [3:0]	ireg_neg;
reg [3:0]	din_clk_a /* synthesis syn_keep=1 */;
reg [3:0]	din_clk_b /* synthesis syn_keep=1 */;
wire			ireg_ctl_pos;
wire			ireg_ctl_neg;
reg			rx_ctrl_a;
reg			rx_ctrl_b;


//Input DDR
//
ddr_input I_DDR_INPUT (
	.clkin(rx_clk ), 
	.reset(!rstn ), 
	.sclk(rx_clk_out ), 
	.datain({rx_ctl,din}), 
	.q({rx_ctl_b, dout_b, rx_ctl_a, dout_a}) 
);


// output DDR
ODDRX1F U_TX_OUT0(
			
            .D0     (txd[0]     ),
            .D1     (txd[4]     ),
            .SCLK   (tx_clk     ),
	    .RST    (!rstn),
            .Q      (td[0]      )
        )/* synthesis ODDRAPPS = "SCLK_ALIGNED" */;

ODDRX1F U_TX_OUT1(
            .D0     (txd[1]     ),
            .D1     (txd[5]     ),
            .SCLK   (tx_clk     ),
	    .RST    (!rstn),
            .Q      (td[1]      )
        )/* synthesis ODDRAPPS = "SCLK_ALIGNED" */;

ODDRX1F U_TX_OUT2(
            .D0     (txd[2]     ),
            .D1     (txd[6]     ),
            .SCLK   (tx_clk     ),
	    .RST    (!rstn),
            .Q      (td[2]      )
        )/* synthesis ODDRAPPS = "SCLK_ALIGNED" */;

ODDRX1F U_TX_OUT3(
            .D0     (txd[3]     ),
            .D1     (txd[7]     ),
            .SCLK   (tx_clk     ),
	    .RST    (!rstn),
            .Q      (td[3]      )
        )/* synthesis ODDRAPPS = "SCLK_ALIGNED" */;

ODDRX1F U_TX_CTL(
            .D0     (tx_en      ),
            .D1     (tx_er      ),
            .SCLK   (tx_clk     ),
	    .RST    (!rstn),
            .Q      (tx_ctl     )
        )/* synthesis ODDRAPPS = "SCLK_ALIGNED" */;




endmodule
