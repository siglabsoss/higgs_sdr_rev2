/*
 * Module: cs_udp_cmd_rx_tx
 * 
 * 
 * Encapsulates the logic for sending UDP commands down to CS and receiving command replies.
 * 
 */

`include "ethernet_support_pkg.sv"
 
`default_nettype none

module cs_udp_cmd_rx_tx #(
    parameter bit SIM_MODE = 0
)(
    input  wire logic       i_eth_mac_rx_clk,
    input  wire logic       i_eth_mac_rx_clk_srst,
    input  wire logic       i_eth_mac_tx_clk,
    input  wire logic       i_eth_mac_tx_clk_srst,
    input  wire logic       i_sys_clk,
    input  wire logic       i_sys_clk_srst,
    
    /* ETH LOGIC ---> THIS MODULE */
    input  wire logic       i_udp_cmd_byte_vld,
    input  wire logic       i_udp_cmd_last_byte,
    input  wire logic [7:0] i_udp_cmd_byte,
    output wire logic       o_udp_cmd_byte_rd,
    
    /* ETH FGPA ---> CS FPGA */
    input  wire logic       i_cs_udp_cmd_byte_rd,
    output wire logic       o_cs_udp_cmd_byte_vld,
    output wire logic       o_cs_udp_cmd_last_byte,
    output wire logic       o_cs_udp_cmd_byte_parity,
    output wire logic [7:0] o_cs_udp_cmd_byte,
    
    /* CS FPGA ---> ETH FGPA */
    input  wire logic       i_cs_eth_frame_byte_vld,
    input  wire logic       i_cs_eth_frame_last_byte,
    input  wire logic       i_cs_eth_frame_byte_parity,
    input  wire logic [7:0] i_cs_eth_frame_byte,
    output wire logic       o_cs_eth_frame_byte_rd,
    
    /* THIS MODULE ---> ETH LOGIC */
    input  wire logic       i_mac_tx_arb_byte_rd,
    output wire logic       o_mac_tx_arb_frame_avail, // note: use this for request to mac_tx_arbiter so that we don't make a request until the full Ethernet II Frame is ready
    output wire logic       o_mac_tx_arb_byte_vld,    
    output wire logic       o_mac_tx_arb_last_byte,
    output wire logic [7:0] o_mac_tx_arb_byte,
    
    /* ERROR REPORTING */
    output wire logic [31:0] o_parity_errors
);
    
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /*
     * 
     * GRAVITON ---> CS
     * 
     */
    
    logic       udp_cmd_fifo_afull;
    logic       udp_cmd_fifo_rden;
    logic [9:0] udp_cmd_fifo_rdata;
    logic [9:0] udp_cmd_fifo_rdata_reg;
    logic       udp_cmd_fifo_rdata_vld;
    logic       udp_cmd_fifo_rdata_vld_reg;


    // register read signal from CS to help with timing
    always_ff @(posedge i_sys_clk) begin
        if (i_sys_clk_srst) begin
            udp_cmd_fifo_rden <= 1'b0;
        end else begin
            udp_cmd_fifo_rden <= i_cs_udp_cmd_byte_rd;
        end
    end
 
    pmi_fifo_dc_fwft_v1_0 #(
    	.WR_DEPTH        (128), 
    	.WR_DEPTH_AFULL  (120), // ensures we don't have to hit the brakes too hard
    	.WR_WIDTH        (10), 
    	.RD_WIDTH        (10), 
    	.FAMILY          ("ECP5U"), 
    	.IMPLEMENTATION  ("EBR"), 
    	.RESET_MODE      ("sync"), 
    	.WORD_SWAP       (0), 
        .SIM_MODE        (SIM_MODE)
    	) udp_cmd_fifo (
    	.wrclk           (i_eth_mac_rx_clk), 
    	.wrclk_rst       (i_eth_mac_rx_clk_srst), 
    	.rdclk           (i_sys_clk), 
    	.rdclk_rst       (i_sys_clk_srst), 
    	.wren            (i_udp_cmd_byte_vld & (~udp_cmd_fifo_afull)), 
    	.wdata           ({i_udp_cmd_last_byte, ^i_udp_cmd_byte, i_udp_cmd_byte}), // last byte, parity (even), data byte
    	.full            (), 
    	.afull           (udp_cmd_fifo_afull), 
    	.rden            (udp_cmd_fifo_rden), 
    	.rdata           (udp_cmd_fifo_rdata),
    	.rdata_vld       (udp_cmd_fifo_rdata_vld));

    // goes to Ethernet logic in this FPGA
    assign o_udp_cmd_byte_rd = ~udp_cmd_fifo_afull; 
    
    // goes to CS FPGA
    always_ff @(posedge i_sys_clk) begin
        if (i_sys_clk_srst) begin
            udp_cmd_fifo_rdata_vld_reg <= 0;
        end else begin
            // avoid driving valid to CS FPGA when not being read.  CS FPGA is set up to write to its input FIFO whenever it sees valid high.
            // It avoids overflows by de-asserting its read early enough using the FIFOs afull signal, but it does not gate its write enable.
            udp_cmd_fifo_rdata_vld_reg <= udp_cmd_fifo_rdata_vld & udp_cmd_fifo_rden; 
            udp_cmd_fifo_rdata_reg     <= udp_cmd_fifo_rdata;
        end
    end
    
    assign o_cs_udp_cmd_byte_vld    = udp_cmd_fifo_rdata_vld_reg;
    assign o_cs_udp_cmd_last_byte   = udp_cmd_fifo_rdata_reg[9];
    assign o_cs_udp_cmd_byte_parity = udp_cmd_fifo_rdata_reg[8];
    assign o_cs_udp_cmd_byte        = udp_cmd_fifo_rdata_reg[7:0];


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /*
     * 
     * CS ---> GRAVITON
     * 
     */
    
    
    // Logic to take command replies from CS and pass them up to Ethernet
    
    localparam int unsigned ETH_FRAME_FIFO_DEPTH  = 2**$clog2(ETH_FRAME_MAX_BYTES); // next power of two (Ethernet command replies are much smaller that the MTU so setting the input buffer depth to 1 MTU is plenty)
    localparam int unsigned FRAME_AVAIL_CNTR_BITS = $clog2( int'( (1.0 * ETH_FRAME_FIFO_DEPTH) / (1.0 * ETH_FRAME_MIN_BYTES) ) ); // int' casting rounds, so 3.5 --> 4 and NOT 3

    logic        eth_frame_byte_vld;
    logic        eth_frame_last_byte;
    logic        eth_frame_byte_parity;
    logic [7:0]  eth_frame_byte;
    
    logic        eth_frame_rx_fifo_wren;
    logic [8:0]  eth_frame_rx_fifo_wdata;
    logic        eth_frame_rx_fifo_afull;
    logic        eth_frame_rx_fifo_not_afull_reg;
    logic        eth_frame_rx_fifo_empty;
    logic        eth_frame_rx_fifo_rden;
    logic        eth_frame_rx_fifo_rdata_vld;
    logic [8:0]  eth_frame_rx_fifo_rdata;
    logic [8:0]  eth_frame_rx_fifo_rdata_reg;

    logic        eth_frame_rx_byte_parity_error;
    logic [31:0] eth_frame_rx_byte_parity_errors;

    
    // register inputs from CS FPGA to help with timing
    always_ff @(posedge i_sys_clk) begin
        if (i_sys_clk_srst) begin
            eth_frame_byte_vld <= 0;
        end else begin
            eth_frame_byte_vld    <= i_cs_eth_frame_byte_vld; 
            eth_frame_last_byte   <= i_cs_eth_frame_last_byte;
            eth_frame_byte_parity <= i_cs_eth_frame_byte_parity;
            eth_frame_byte        <= i_cs_eth_frame_byte;
        end
    end

    assign eth_frame_rx_fifo_wren = eth_frame_byte_vld; // don't gate writing to FIFO.  Logic in CS FPGA will stop after several clocks of us de-asserting our read and will gate its valid output while our read is low
    assign eth_frame_rx_fifo_wdata = {eth_frame_last_byte, eth_frame_byte};

    // check parity of UDP Command reply data byte
    always_ff @(posedge i_sys_clk) begin
        if (i_sys_clk_srst) begin
            eth_frame_rx_byte_parity_error  <= 1'b0;
            eth_frame_rx_byte_parity_errors <= '0;
        end else begin
            eth_frame_rx_byte_parity_error <= eth_frame_byte_vld & (^{eth_frame_byte_parity, eth_frame_byte});
            if (eth_frame_rx_byte_parity_error) begin
                eth_frame_rx_byte_parity_errors <= eth_frame_rx_byte_parity_errors + 1;
            end
        end
    end
    
    assign o_parity_errors = eth_frame_rx_byte_parity_errors;
    
    pmi_fifo_dc_fwft_v1_0 #(
        .WR_DEPTH        (ETH_FRAME_FIFO_DEPTH), 
        .WR_DEPTH_AFULL  (ETH_FRAME_FIFO_DEPTH-16), // allows the output logic in the CS FPGA 16 clocks to respond to our read signal going low
        .WR_WIDTH        (9), 
        .RD_WIDTH        (9), 
        .FAMILY          ("ECP5U"), 
        .IMPLEMENTATION  ("EBR"), 
        .RESET_MODE      ("sync"), 
        .WORD_SWAP       (0), 
        .SIM_MODE        (SIM_MODE)
        ) eth_frame_rx_fifo (
        .wrclk           (i_sys_clk                   ), 
        .wrclk_rst       (i_sys_clk_srst              ), 
        .rdclk           (i_eth_mac_tx_clk            ), 
        .rdclk_rst       (i_eth_mac_tx_clk_srst       ), 
        .wren            (eth_frame_rx_fifo_wren      ), 
        .wdata           (eth_frame_rx_fifo_wdata     ), 
        .full            (), 
        .afull           (eth_frame_rx_fifo_afull     ), 
        .rden            (eth_frame_rx_fifo_rden      ), 
        .rdata           (eth_frame_rx_fifo_rdata     ),
        .rdata_vld       (eth_frame_rx_fifo_rdata_vld ));

    // register output to CS to help with timing
    always_ff @(posedge i_sys_clk) begin
        if (i_sys_clk_srst) begin
            eth_frame_rx_fifo_not_afull_reg <= 1'b1;
        end else begin
            eth_frame_rx_fifo_not_afull_reg <= ~eth_frame_rx_fifo_afull;
        end
    end

    assign o_cs_eth_frame_byte_rd = eth_frame_rx_fifo_not_afull_reg;
    

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /*
     * 
     * TO ETHERNET LOGIC IN THIS FPGA 
     * 
     */

    logic [FRAME_AVAIL_CNTR_BITS-1:0] eth_frame_avail_cntr; // keeps tabs on how many complete ethernet frames are sitting in the output FIFO in this module waiting to be sent back to the host PC
    
    assign eth_frame_rx_fifo_empty = ~eth_frame_rx_fifo_rdata_vld;
    assign eth_frame_rx_fifo_rden  = i_mac_tx_arb_byte_rd; 

    always_ff @(posedge i_eth_mac_tx_clk) begin
        if (i_eth_mac_tx_clk_srst) begin
            eth_frame_rx_fifo_rdata_reg <= '0;
        end else begin
            eth_frame_rx_fifo_rdata_reg <= eth_frame_rx_fifo_rdata;
        end
    end
    
    assign o_mac_tx_arb_frame_avail = |eth_frame_avail_cntr & ~eth_frame_rx_fifo_empty; 
    assign o_mac_tx_arb_last_byte   = eth_frame_rx_fifo_rdata[8] & ~eth_frame_rx_fifo_rdata_reg[8];  // one clock wide pulse
    assign o_mac_tx_arb_byte        = eth_frame_rx_fifo_rdata[7:0];
    

    always_ff @(posedge i_eth_mac_tx_clk) begin
        if (i_eth_mac_tx_clk_srst) begin
            eth_frame_avail_cntr <= '0;
        end else begin

            // NOTE: THE ORDER OF THESE IF STATEMENTS IS CRITICAL

            if (eth_frame_rx_fifo_wdata[8] & eth_frame_rx_fifo_wren) begin // end of packet marker written into output fifo so there's now another full udp packet ready to be consumed
                eth_frame_avail_cntr <= eth_frame_avail_cntr + 1;
            end
            
            if (eth_frame_rx_fifo_rdata[8] & eth_frame_rx_fifo_rdata_vld & eth_frame_rx_fifo_rden) begin // end of packet mark read out of output fifo so there's now one fewer udp packets ready to be consumed
                eth_frame_avail_cntr <= eth_frame_avail_cntr - 1;
            end
            
            if (eth_frame_rx_fifo_wdata[8] & eth_frame_rx_fifo_wren & eth_frame_rx_fifo_rdata[8] & eth_frame_rx_fifo_rdata_vld & eth_frame_rx_fifo_rden) begin // simultaneous packet read out of fifo and new one written into fifo
                eth_frame_avail_cntr <= eth_frame_avail_cntr;
            end
        end
    end


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


endmodule

`default_nettype wire
