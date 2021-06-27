
/*
 * Module: fifo_cdc
 * 
 * 
 * Simple FIFO based clock-domain-crossing module
 * 
 */
 
`default_nettype none 
 
module fifo_cdc #(
    parameter int unsigned DEPTH          = 16,     // should be a power of 2
    parameter int unsigned WIDTH          = 32,
    parameter              FAMILY         = "ECP5U", 
    parameter              IMPLEMENTATION = "LUT",    // "LUT" or "EBR"
    parameter              RESET_MODE     = "sync",
    parameter bit          SIM_MODE       = 1'b0
)(

    input  wire logic             i_wclk,
    input  wire logic             i_wclk_rst,
    input  wire logic             i_wdata_vld,
    input  wire logic [WIDTH-1:0] i_wdata,
    input  wire logic             i_rclk,
    input  wire logic             i_rclk_rst,
    output wire logic             o_rdata_vld,
    output wire logic [WIDTH-1:0] o_rdata
);

`ifndef VERILATE_DEF
    pmi_fifo_dc_fwft_v1_0 #(
        .WR_DEPTH        (DEPTH), 
        .WR_DEPTH_AFULL  (DEPTH-1), 
        .WR_WIDTH        (WIDTH), 
        .RD_WIDTH        (WIDTH), 
        .FAMILY          (FAMILY), 
        .IMPLEMENTATION  (IMPLEMENTATION), 
        .RESET_MODE      (RESET_MODE), 
        .WORD_SWAP       (0), 
        .SIM_MODE        (SIM_MODE       )
        ) pmi_fifo_dc_fwft_v1_0 (
        .wrclk           (i_wclk), 
        .wrclk_rst       (i_wclk_rst), 
        .rdclk           (i_rclk), 
        .rdclk_rst       (i_rclk_rst), 
        .wren            (i_wdata_vld), 
        .wdata           (i_wdata), 
        .full            (), 
        .afull           (), 
        .rden            (1'b1), 
        .rdata           (o_rdata), 
        .rdata_vld       (o_rdata_vld));
`else

assign o_rdata = i_wdata;

`endif
endmodule

`default_nettype wire