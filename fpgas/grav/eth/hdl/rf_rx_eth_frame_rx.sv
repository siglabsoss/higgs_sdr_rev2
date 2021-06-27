
/*
 * Module: rf_rx_eth_frame_rx
 * 
 */

`include "ethernet_support_pkg.sv"
 
`default_nettype none
 
module rf_rx_eth_frame_rx #(
    parameter int unsigned NUM_ETH_FRAME_BUFFERS = 4,
    parameter bit          GBIT_MAC_DIRECT_MODE  = 0, // Set to: 0 when this module is connected to the mac_tx_arbiter, 1 when connected directly to the LATTICE GBIT MAC Tx interface 
    parameter bit          SIM_MODE = 1'b0
)(
    input  wire logic        i_sys_clk,
    input  wire logic        i_sys_clk_srst,
    input  wire logic        i_txmac_clk,
    input  wire logic        i_txmac_clk_srst,
    input  wire logic        i_byte_vld,
    input  wire logic        i_last_byte,
    input  wire logic        i_byte_parity,
    input  wire logic [7:0]  i_byte,
    
    input  wire logic        i_eth_byte_rd,
    output wire logic 		 o_afull, // connects to VershaCapture to stall packets
    output wire logic        o_eth_avail, // note: use this for request to mac_tx_arbiter so that we don't make a request until the full Ethernet II Frame is ready
    output wire logic        o_eth_byte_vld, 
    output wire logic        o_eth_eof,
    output wire logic [7:0]  o_eth_byte,
    output wire logic        o_fifo_overflow, // pulses for one i_sys_clk period if the CDC FIFO in this module overflows
    output wire logic [31:0] o_parity_errors  // number of parity errors encountered in the bytes received from neighboring FPGA since last reset
);

    localparam FRAME_FIFO_DEPTH      = 2**($clog2(NUM_ETH_FRAME_BUFFERS * ETH_FRAME_MAX_BYTES));
    localparam FRAME_FIFO_AFULL_LVL  = FRAME_FIFO_DEPTH - ETH_FRAME_MAX_BYTES;
    localparam FRAME_AVAIL_CNTR_BITS = $clog2( int'( (1.0 * FRAME_FIFO_DEPTH) / (1.0 * ETH_FRAME_MIN_BYTES) ) ); // int' casting rounds, so 3.5 --> 4 and NOT 3

    /* PARAMETER RANGE CHECKING */
    initial begin
        assert (NUM_ETH_FRAME_BUFFERS >= 1 && NUM_ETH_FRAME_BUFFERS <= 10) else $fatal(1, "NUM_ETH_FRAME_BUFFERS MUST BE IN THE RANGE [1:10]!"); 
    end
    
    /* INPUT REGISTERS, PARITY CHECK, & CDC */

    logic        eth_frame_byte_vld;
    logic        eth_frame_last_byte;
    logic        eth_frame_byte_parity;
    logic [7:0]  eth_frame_byte;

    always_ff @(posedge i_sys_clk) begin
        if (i_sys_clk_srst) begin
            eth_frame_byte_vld <= 0;
        end else begin
            eth_frame_byte_vld          <= i_byte_vld; 
            eth_frame_last_byte         <= i_last_byte;
            eth_frame_byte_parity       <= i_byte_parity;
            eth_frame_byte              <= i_byte;
        end
    end

    logic        eth_frame_byte_parity_error;
    logic [31:0] eth_frame_byte_parity_errors;
    
    always_ff @(posedge i_sys_clk) begin
        if (i_sys_clk_srst) begin
            eth_frame_byte_parity_error  <= 0; 
            eth_frame_byte_parity_errors <= '0;
        end else begin
            eth_frame_byte_parity_error <= (eth_frame_byte_parity ^ (^eth_frame_byte)) & eth_frame_byte_vld;
            if (eth_frame_byte_parity_error) begin
                eth_frame_byte_parity_errors <= eth_frame_byte_parity_errors + 1;
            end
        end
    end
    
    assign o_parity_errors = eth_frame_byte_parity_errors;

    logic       eth_frame_cdc_fifo_wren;
    logic       eth_frame_cdc_fifo_rdata_vld;
    logic [8:0] eth_frame_cdc_fifo_rdata;
    logic       eth_frame_cdc_fifo_rden;
    logic       eth_frame_cdc_fifo_full;
    logic       eth_frame_cdc_fifo_overflow;
    
    pmi_fifo_dc_fwft_v1_0 #(
        .WR_DEPTH        (128), 
        .WR_DEPTH_AFULL  (105), 
        .WR_WIDTH        (9), 
        .RD_WIDTH        (9), 
        .FAMILY          ("ECP5U"), 
        .IMPLEMENTATION  ("EBR"), 
        .RESET_MODE      ("sync"), 
        .WORD_SWAP       (0), 
        .SIM_MODE        (SIM_MODE       )
        ) eth_frame_cdc_fifo (
        .wrclk           (i_sys_clk), 
        .wrclk_rst       (i_sys_clk_srst), 
        .rdclk           (i_txmac_clk), 
        .rdclk_rst       (i_txmac_clk_srst), 
        .wren            (eth_frame_cdc_fifo_wren), 
        .wdata           ({eth_frame_last_byte, eth_frame_byte}), 
        .full            (eth_frame_cdc_fifo_full), 
        .afull           (o_afull), 
        .rden            (eth_frame_cdc_fifo_rden), 
        .rdata           (eth_frame_cdc_fifo_rdata          ), 
        .rdata_vld       (eth_frame_cdc_fifo_rdata_vld      ));
    
    assign eth_frame_cdc_fifo_wren = eth_frame_byte_vld;
    
    always_ff @(posedge i_sys_clk) begin
        if (i_sys_clk_srst) begin
            eth_frame_cdc_fifo_overflow <= 0;
        end else begin
            eth_frame_cdc_fifo_overflow <= eth_frame_cdc_fifo_wren & eth_frame_cdc_fifo_full;
        end
    end
    
    assign o_fifo_overflow = eth_frame_cdc_fifo_overflow;

    /* Tx MAC RR ARBITER INTERFACE LOGIC */
    
    logic       eth_fifo_empty;
    logic       eth_fifo_wren;
    logic       eth_fifo_rden;
    logic       eth_fifo_full;
    logic       eth_fifo_afull;
    logic [8:0] eth_fifo_wdata;
    logic [8:0] eth_fifo_dout;
    logic [8:0] eth_fifo_dout_reg_0; // for direct connect mode
    logic [8:0] eth_fifo_dout_reg_1; // for direct connect mode
    logic       eth_fifo_dout_vld;
    
    logic [FRAME_AVAIL_CNTR_BITS-1:0] eth_frame_avail_cntr; // keeps tabs on how many complete ethernet frames are sitting in the output FIFO.

    pmi_fifo_sc_fwft_v1_0 #(
        .DEPTH           (FRAME_FIFO_DEPTH    ), 
        .DEPTH_AFULL     (FRAME_FIFO_AFULL_LVL), 
        .WIDTH           (9                   ), 
        .FAMILY          ("ECP5U"             ), 
        .IMPLEMENTATION  ("EBR"               ),
        .SIM_MODE        (SIM_MODE            )
        ) eth_frame_fifo (
        .clk             (i_txmac_clk         ), 
        .rst             (i_txmac_clk_srst    ), 
        .wren            (eth_fifo_wren       ), 
        .wdata           (eth_fifo_wdata      ), 
        .full            (eth_fifo_full       ), 
        .afull           (eth_fifo_afull      ), 
        .rden            (eth_fifo_rden       ), 
        .rdata           (eth_fifo_dout       ), 
        .rdata_vld       (eth_fifo_dout_vld   ));
    
    assign eth_fifo_empty = ~eth_fifo_dout_vld;
    assign eth_fifo_rden  = i_eth_byte_rd; 
    
    assign eth_frame_cdc_fifo_rden = ~eth_fifo_afull;
    assign eth_fifo_wren           = eth_frame_cdc_fifo_rden & eth_frame_cdc_fifo_rdata_vld;
    assign eth_fifo_wdata          = eth_frame_cdc_fifo_rdata;

    always_ff @(posedge i_txmac_clk) begin
        if (i_txmac_clk_srst) begin
            eth_fifo_dout_reg_0 <= '0;
            eth_fifo_dout_reg_1 <= '0;
        end else begin
            eth_fifo_dout_reg_0 <= eth_fifo_dout;
            eth_fifo_dout_reg_1 <= eth_fifo_dout_reg_0;
        end
    end
    
    // need to delay data and eof by 1 clock if directly connected to Lattice Gbit MAC (see Lattice IPUG51 Transmission Waveforms)
    generate
        if (GBIT_MAC_DIRECT_MODE) begin
            assign o_eth_eof  = eth_fifo_dout_reg_0[8] & ~eth_fifo_dout_reg_1[8]; // one clock wide pulse
            assign o_eth_byte = eth_fifo_dout_reg_0[7:0];
        end else begin
            assign o_eth_eof  = eth_fifo_dout[8] & ~eth_fifo_dout_reg_0[8];  // one clock wide pulse
            assign o_eth_byte = eth_fifo_dout[7:0];
        end 
    endgenerate
                
    assign o_eth_byte_vld  = eth_fifo_dout_vld; // NOT USED BY LATTICE GBIT MAC SO NO NEED TO ANYTHING SPECIAL IF GBIT_MAC_DIRECT_MODE = 1
    assign o_eth_avail     = |eth_frame_avail_cntr & ~eth_fifo_empty; 
    

    always_ff @(posedge i_txmac_clk) begin
        if (i_txmac_clk_srst) begin
            eth_frame_avail_cntr <= '0;
        end else begin

            // NOTE: THE ORDER OF THESE IF STATEMENTS IS CRITICAL

            if (eth_fifo_wdata[8] & eth_fifo_wren) begin // end of packet marker written into output fifo so there's now another full udp packet ready to be consumed
                eth_frame_avail_cntr <= eth_frame_avail_cntr + 1;
            end
            
            if (eth_fifo_dout[8] & eth_fifo_dout_vld & eth_fifo_rden) begin // end of packet mark read out of output fifo so there's now one fewer udp packets ready to be consumed
                eth_frame_avail_cntr <= eth_frame_avail_cntr - 1;
            end
            
            if (eth_fifo_wdata[8] & eth_fifo_wren & eth_fifo_dout[8] & eth_fifo_dout_vld & eth_fifo_rden) begin // simultaneous packet read out of fifo and new one written into fifo
                eth_frame_avail_cntr <= eth_frame_avail_cntr;
            end
        end
    end


endmodule


`default_nettype wire