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
//  File:              rgmii2gmii.v
//  Title:             rgmii2gmii
//  Description:       Reduced GMII to GMII module
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

module rgmii2gmii(
            rstn,
            tx_clk,
            rx_clk,
	    rx_clk_out,

        // port for RGMII
            td,
            tx_ctl,
        
            rd,
            rx_ctl,
        
        // port for GMII
            rxd,
            rx_dv,
            rx_er,

            txd,
            tx_en,
            tx_er
        );

input       rstn       ;// /* synthesis IO_TYPE="LVTTL33" */;
input       tx_clk     ;// /* synthesis IO_TYPE="HSTL15_I" */;
input       rx_clk     ;

output 		rx_clk_out;

input [7:0] txd        ;// /* synthesis IO_TYPE="HSTL15_I" */;
input       tx_en      ;// /* synthesis IO_TYPE="HSTL15_I" */;
input       tx_er      ;// /* synthesis IO_TYPE="HSTL15_I" */;

output[7:0] rxd        ;// /* synthesis IO_TYPE="HSTL15_I" */;
output      rx_dv      ;// /* synthesis IO_TYPE="HSTL15_I" */;
output      rx_er      ;// /* synthesis IO_TYPE="HSTL15_I" */;

input [3:0] rd         ;// /* synthesis IO_TYPE="HSTL15_I" */;
input       rx_ctl     ;// /* synthesis IO_TYPE="HSTL15_I" */;

output[3:0] td         ;// /* synthesis IO_TYPE="HSTL15_I" */;
output      tx_ctl     ;// /* synthesis IO_TYPE="HSTL15_I" */;

reg         tx_en_buf   ;///* synthesis din="" */;
reg         tx_er_buf   ;///* synthesis din="" */;
reg [7:0]   txd_buf     ;///* synthesis din="" */;

reg         rx_dv       ;///* synthesis dout="" */;
reg         rx_er       ;///* synthesis dout="" */;
reg [7:0]   rxd         ;///* synthesis dout="" */;

wire        tx_er_gen;

wire[3:0]   rx_out_a;
wire[3:0]   rx_out_b;
wire        rx_ctl_a;
wire        rx_ctl_b;

wire        tx_clk_out  /* synthesis syn_keep=1 frequency=125.00 */;

GSR GSR(rstn);

// gmii input signals buffer
always @(posedge tx_clk_out or negedge rstn)
begin
    if(!rstn) begin
        tx_en_buf <= 1'b0;
        tx_er_buf <= 1'b0;
        txd_buf   <= 8'h00;
    end else begin
        tx_en_buf <= tx_en;
        tx_er_buf <= tx_er;
        txd_buf   <= txd;
    end
end

assign tx_er_gen = tx_en_buf ^ tx_er_buf;

always @(posedge rx_clk_out or negedge rstn)
begin
    if(!rstn) begin
        rxd     <= 8'h00;
        rx_dv   <= 1'b0;
        rx_er   <= 1'b0;
    end else begin
        rxd     <= {rx_out_b,rx_out_a};
        rx_dv   <= rx_ctl_a;
        rx_er   <= rx_ctl_a ^ rx_ctl_b;
    end
end
 
assign tx_clk_out = tx_clk;
 
// DDR shift register.
DDR	 U_RX(
            .rstn       (rstn       ),
            .rx_clk     (rx_clk     ),
	    .rx_clk_out (rx_clk_out ),
            .din        (rd         ),
            .rx_ctl     (rx_ctl     ),
            .dout_a     (rx_out_a   ),
            .dout_b     (rx_out_b   ),
            .rx_ctl_a   (rx_ctl_a   ),
            .rx_ctl_b   (rx_ctl_b   ),
            .tx_clk     (tx_clk_out ),
            .txd        (txd_buf    ),
            .td         (td         ),
            .tx_en      (tx_en_buf  ),
            .tx_er      (tx_er_gen  ),
            .tx_ctl     (tx_ctl     )
        );

endmodule
