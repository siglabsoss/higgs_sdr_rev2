// ===========================================================================
// Verilog module generated by IPexpress
// Filename: tst_logic.v  
// Copyright 2012 (c) Lattice Semiconductor Corporation. All rights reserved.
// ===========================================================================

//`timescale 1ns/100ps

module tst_logic (

	// -------
	// inputs
	// -------

	// from IO
        reset_n,
        txmac_clk,
        rxmac_clk,
        txmac_clk_en,
        rxmac_clk_en,

	// from tsmac core
        rx_write,
        rx_dbout,
        rx_eof,
        rx_error,
        rx_fifo_error,
	tx_macread,
	tx_done,
	tx_disfrm,
//gbit_en,
	
	// from reg_intf
        pkt_add_swap_ri,
        pkt_loop_enb_ri,
	tx_sndpaustim_ri,
	tx_sndpausreq_ri,
	tx_fifoctrl_ri,
	rx_fifo_full_ri,
	tx_fifo_empty_ri,
	ignore_next_pkt_ri,
	aff_thrhd,
	afe_thrhd,

	// -------
	// outputs
	// -------

	// to tsmac core
	tx_fifodata,
	tx_fifoeof,
	tx_fifoavail,
	tx_fifoempty,
	tx_sndpaustim,
	tx_sndpausreq,
	tx_fifoctrl,
	rx_fifo_full,
	ignore_next_pkt,

	// to reg_intf
        rxc_clk,
        txc_clk,
        rx_error_ri,
        rx_fifo_error_ri,
	tx_disfrm_ri,
        tx_fifo_full_ri
	);

	//======================
	// inputs and outputs 
	//======================
	input reset_n;                   // active low global reset
        input txmac_clk;
        input rxmac_clk;
        input txmac_clk_en;
        input rxmac_clk_en;

        // from tsmac core
        input rx_write;
        input [7:0] rx_dbout;
        input rx_eof;
        input rx_error;
        input rx_fifo_error;
        input tx_macread;
        input tx_done;
        input tx_disfrm;
//        input gbit_en;

        // from reg_intf
        input pkt_add_swap_ri;
        input pkt_loop_enb_ri;
        input [15:0] tx_sndpaustim_ri;
        input tx_sndpausreq_ri;
        input tx_fifoctrl_ri;
        input rx_fifo_full_ri;
        input tx_fifo_empty_ri;
        input ignore_next_pkt_ri; 
	input [8:0] aff_thrhd;		// almost full threshold from reg intf
	input [8:0] afe_thrhd;		// almost empty threshold from reg intf

        // to tsmac core
        output [7:0] tx_fifodata;
        output tx_fifoeof;
        output tx_fifoavail;
        output tx_fifoempty;
        output [15:0] tx_sndpaustim;
        output tx_sndpausreq;
        output tx_fifoctrl;
        output rx_fifo_full;
        output ignore_next_pkt;

        // to reg_intf
        output rx_error_ri;
        output rx_fifo_error_ri;
        output tx_disfrm_ri;
        output tx_fifo_full_ri;
        output rxc_clk;                    // MAC Client side Rx clock 
        output txc_clk;                    // MAC Client side Tx clock

parameter pdevice_family = "EC";

	//======================
	// Regs and wires
	//======================
        wire tx_fifoempty;
        reg [15:0] tx_sndpaustim;
        reg tx_sndpausreq;
        reg tx_fifoctrl;
        reg ignore_next_pkt;
        reg write_eof_rxclk;
        reg [1:0] clr_write_eof_rxclk;
        reg [2:0] write_eof_txclk;
        reg flop_tx_macread;
        reg flop_flop_tx_macread;
        reg flop_tx_fifo_eof;
        reg [3:0] frames_present;

        reg rx_error_ri;
        reg rx_fifo_error_ri;
        reg tx_disfrm_ri;
        reg tx_fifo_full_ri;
        reg tx_fifo_empty_f;		 // tx fifo empty pipelined and synced with clk_enb

        wire rxc_clk;                    // MAC Client side Rx clock 
        wire txc_clk;                    // MAC Client side Tx clock 

        wire [7:0] tx_fifodata;
        wire tx_fifoeof;
        reg  tx_fifoavail;
        reg  tx_fifoavail_int;
        wire rx_fifo_full;

	wire [8:0] tx_fifo_dout;	// data out of tx_fifo
	wire tx_fifo_full;		// tx fifo full
	wire tx_fifo_afull;		// tx fifo almost full
        wire tx_fifo_empty;		// tx fifo empty
        wire tx_fifo_aempty;		// tx fifo almost empty

        wire [7:0] tx_dbin;
        wire tx_write;
        wire tx_eof;

        //-------------------------------------------------------------------
        // glue logic 
        //-------------------------------------------------------------------
	assign rxc_clk = rxmac_clk;	
	assign txc_clk = txmac_clk;	
        assign tx_fifodata[7:0] = tx_fifo_dout[7:0];
        assign tx_fifoeof = tx_fifo_dout[8];
        assign rx_fifo_full = (tx_fifo_full | rx_fifo_full_ri);

        assign tx_fifoempty = (tx_fifo_empty|tx_fifo_empty_ri);

        always @(posedge rxmac_clk or negedge reset_n) begin
          if (~reset_n) begin
        	ignore_next_pkt <=  1'b0;
        	tx_fifoavail <=  1'b0;
        	tx_fifoavail_int <=  1'b0;
        	write_eof_rxclk <=  1'b0;
        	clr_write_eof_rxclk <=  2'b00;
        	tx_fifo_empty_f <=  1'b1;
           end
          else if (rxmac_clk_en) begin
        	tx_fifo_empty_f <=  tx_fifo_empty;
        	ignore_next_pkt <=  ignore_next_pkt_ri;

                tx_fifoavail_int <= (|frames_present) | (~tx_fifo_aempty);
                tx_fifoavail <= tx_fifoavail_int;

		clr_write_eof_rxclk[1] <= clr_write_eof_rxclk[0];
		clr_write_eof_rxclk[0] <= write_eof_rxclk;
		if (rx_write & rx_eof) begin
		   write_eof_rxclk <= 1;
	        end else if (clr_write_eof_rxclk[1]) begin
	           write_eof_rxclk <= 0;
                end		   
           end // else
        end // always

        always @(posedge txmac_clk or negedge reset_n) begin
          if (~reset_n) begin
                tx_sndpausreq <=  1'b0;
                tx_fifo_full_ri <=  1'b0;
		tx_sndpaustim[15:0] <=  16'h0000;
                tx_disfrm_ri <=  1'b0;
                tx_fifoctrl <=  1'b0;
        	rx_error_ri <=  1'b0;
        	rx_fifo_error_ri <=  1'b0;
        	write_eof_txclk <=  1'b0;
        	flop_tx_macread <=  1'b0;
        	flop_flop_tx_macread <=  1'b0;
        	flop_tx_fifo_eof <=  1'b0;
        	frames_present <=  1'b0;
          end
          else if (txmac_clk_en) begin
		if (tx_sndpausreq_ri == 1) begin
                      tx_sndpausreq <= 1;
		end else if (tx_macread == 1) begin
		   if (tx_fifo_afull) begin
                      tx_sndpausreq <= 1;
		   end else begin
                      tx_sndpausreq <= 0;
		   end
		end else begin
                   tx_sndpausreq <= 0;
		end
		tx_sndpaustim[15:0] <=  tx_sndpaustim_ri[15:0];
                tx_fifoctrl <=  tx_fifoctrl_ri;

                tx_disfrm_ri <=  tx_disfrm;	// an error condition -fifo underun etc.
        	rx_error_ri <=  rx_error;
        	rx_fifo_error_ri <=  rx_fifo_error;

                tx_fifo_full_ri <=  tx_fifo_full;

                write_eof_txclk[2] <= write_eof_txclk[1];
                write_eof_txclk[1] <= write_eof_txclk[0];
                write_eof_txclk[0] <= write_eof_rxclk;
                flop_tx_macread    <= tx_macread;
                flop_flop_tx_macread <= flop_tx_macread;
                flop_tx_fifo_eof <= tx_fifo_dout[8];
	        if (tx_fifo_empty == 1) begin
                    frames_present <= 0;  // When fifo is empty clear frames present
	        end else if ((write_eof_txclk[1] & !write_eof_txclk[2])
		         	&& !(flop_flop_tx_macread & flop_tx_fifo_eof)) begin
                    frames_present <= frames_present + 4'b0001;  // Increment # frames
                end else if (!(write_eof_txclk[1] & !write_eof_txclk[2])
		         	&& (flop_flop_tx_macread & flop_tx_fifo_eof)) begin
                    frames_present <= frames_present - 4'b0001;  // Dec # frames in buf
                end
          end // else
        end // always

        //-------------------------------------------------------------------
        // instanciate modules 
        //-------------------------------------------------------------------

	rx_loopbk rx_loopbk(
                .clk(rxmac_clk),
                .reset_n(reset_n),
                .rxmac_clk_en(rxmac_clk_en),
                .add_swap(pkt_add_swap_ri),
                .loop_enb(pkt_loop_enb_ri),
                .rx_dbout(rx_dbout[7:0]),
                .rx_write(rx_write),
                .rx_eof(rx_eof),
                .tx_dbin(tx_dbin),
                .tx_write(tx_write),
                .tx_eof(tx_eof)
                );

        //defparam tx_fifo.ff_ctl.SYNC_MODE = "ASYNC"; 
        //defparam tx_fifo.ff_ctl.RAM_MODE = "NOREG"; 
	fifo_2048x9 #(.pdevice_family(pdevice_family))
	      tx_fifo (
                // INPUTS
                .wclk(rxmac_clk),
                .wren(tx_write),
                .datain({tx_eof,tx_dbin}),
                .reset(reset_n),
                .rclk(txmac_clk),
                .rden(tx_macread),
                .aff_thrhd(aff_thrhd),
                .afe_thrhd(afe_thrhd),
                .wclk_en(rxmac_clk_en),
                .rclk_en(txmac_clk_en),

                // OUTPUTS
                .daout(tx_fifo_dout[8:0]),
                .empty(tx_fifo_empty),
                .almost_full(tx_fifo_afull),
                .almost_empty(tx_fifo_aempty),
                .full(tx_fifo_full)
                );
endmodule
