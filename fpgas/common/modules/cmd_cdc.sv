/*
 * Module: cmd_cdc
 * 
 * 
 * Converts between MIB bus clock domain (currently 25MHz) and system clock domain (currently 125MHz).
 * 
 * This is just a helper module that helps encapsulate and localize the clock domain crossing logic.
 * 
 * This idea is to hook up the MIB master module's slave command interface to the master command interface of this module
 * and the slave command interface of this module to the master command interface of the module that produces the commands. 
 * 
 * 
 */
 
 
`default_nettype none

module cmd_cdc #(
    parameter int unsigned ADDR_BITS = 24,
    parameter int unsigned DATA_BITS = 32,
    parameter bit          SIM_MODE  = 0,
    parameter bit          VERILATE  = 1'b0
)(        

    input           i_sys_clk,
    input           i_sys_srst, // synchronous to i_sys_clk
    input           i_mib_clk,
    input           i_mib_srst, // synchronous to i_mib_clk
    intf_cmd.slave  cmd_sys,    // connect this to the master command interface of the module that produces commands 
    intf_cmd.master cmd_mib     // connect MIB Master's slave command interface to this port
);
    

    /* 
     * 
     * This FIFO takes the command from the Cmd source module's command master interface (cmd_sys) in the sys clock domain (i_sys_clk) and turns it into a MIB master cmd slave interface (cmd_mib) command
     * in the MIB clock domain (i_mib_clk).
     * 
     * NOTE: 
     * 
     * All commands written into this FIFO are processed before the next command comes in (due to the MIB master waiting for an ACK)
     * so there's no concern that this FIFO will overflow.
     *       
     */

    localparam int unsigned TOT_BITS = ADDR_BITS + DATA_BITS + 1; // +1 for command interface rd_wr_n signal

    logic                 sys_to_mib_fifo_wren;
    logic [TOT_BITS-1:0]  sys_to_mib_fifo_wdata;
    logic                 sys_to_mib_fifo_rden;
    logic [TOT_BITS-1:0]  sys_to_mib_fifo_rdata;
    logic                 sys_to_mib_fifo_rdata_vld;
    
    assign sys_to_mib_fifo_wren  = cmd_sys.sel; // this is asserted for one clock cycle per transaction so it's ok to directly tie it to the write enable of the FIFO
    assign sys_to_mib_fifo_wdata = {cmd_sys.rd_wr_n, cmd_sys.byte_addr, cmd_sys.wdata};

generate
if (VERILATE) begin // since sys_clk and mib_clk are same in verilator
            fwft_sc_fifo #(
                    .DEPTH        (4       ), 
                    .WIDTH        (TOT_BITS), 
                    .ALMOST_FULL  (3 )
                ) fwft_sc_fifo (
                    .clk          (i_sys_clk   ), 
                    .rst          (i_sys_srst  ), 
                    .wren         (sys_to_mib_fifo_wren), 
                    .wdata        (sys_to_mib_fifo_wdata), 
                    .full         (        ), 
                    .o_afull      (     ), 
                    .rden         (sys_to_mib_fifo_rden), 
                    .rdata        (sys_to_mib_fifo_rdata), 
                    .rdata_vld    (sys_to_mib_fifo_rdata_vld));
    
        end else begin
    pmi_fifo_dc_fwft_v1_0 #(
        .WR_DEPTH        (4        ), 
        .WR_DEPTH_AFULL  (3        ), 
        .WR_WIDTH        (TOT_BITS ), 
        .RD_WIDTH        (TOT_BITS ), 
        .FAMILY          ("ECP5U"  ), 
        .IMPLEMENTATION  ("LUT"    ), 
        .RESET_MODE      ("sync"   ), 
        .WORD_SWAP       (0        ), 
        .SIM_MODE        (SIM_MODE )
        ) SYS_TO_MIB_FIFO (
        .wrclk           (i_sys_clk                 ), 
        .wrclk_rst       (i_sys_srst                ), 
        .rdclk           (i_mib_clk                 ), 
        .rdclk_rst       (i_mib_srst                ), 
        .wren            (sys_to_mib_fifo_wren      ), 
        .wdata           (sys_to_mib_fifo_wdata     ), 
        .full            (), 
        .afull           (), 
        .rden            (sys_to_mib_fifo_rden      ), 
        .rdata           (sys_to_mib_fifo_rdata     ), 
        .rdata_vld       (sys_to_mib_fifo_rdata_vld ));
        end 
    endgenerate
    
    assign sys_to_mib_fifo_rden = sys_to_mib_fifo_rdata_vld; // auto-read this FIFO
    assign cmd_mib.sel          = sys_to_mib_fifo_rdata_vld;
    assign cmd_mib.rd_wr_n      = sys_to_mib_fifo_rdata[TOT_BITS-1];
    assign cmd_mib.byte_addr    = sys_to_mib_fifo_rdata[ADDR_BITS+DATA_BITS-1:DATA_BITS];
    assign cmd_mib.wdata        = sys_to_mib_fifo_rdata[DATA_BITS-1:0];


    /* 
     * 
     * This FIFO takes the command responses coming into this module's command master interface (cmd_mib) in the MIB clock domain (i_mib_clk) and turns it into a command slave interface (cmd_sys) 
     * response in the system clock domain (i_sys_clk).
     * 
     * NOTE: 
     * 
     * All command responses written into this FIFO are processed before the next response comes in (due to the command generating source waiting for an ACK)
     * so there's no concern that this FIFO will overflow.
     *       
     */

    logic                 mib_to_sys_fifo_wren;
    logic [DATA_BITS-1:0] mib_to_sys_fifo_wdata;
    logic                 mib_to_sys_fifo_rden;
    logic [DATA_BITS-1:0] mib_to_sys_fifo_rdata;
    logic                 mib_to_sys_fifo_rdata_vld;
    
    assign mib_to_sys_fifo_wren  = cmd_mib.ack;   // slave asserts ack for one clock cycle so it's ok to directly tie it to the FIFO write enable
    assign mib_to_sys_fifo_wdata = cmd_mib.rdata;

generate 
if (VERILATE) begin // since sys_clk and mib_clk are same in verilator
            fwft_sc_fifo #(
                .DEPTH        (4       ), 
                .WIDTH        (DATA_BITS), 
                .ALMOST_FULL  (3 )
                ) fwft_sc_fifo (
                .clk          (i_sys_clk   ), 
                .rst          (i_sys_srst  ), 
                .wren         (mib_to_sys_fifo_wren), 
                .wdata        (mib_to_sys_fifo_wdata), 
                .full         (        ), 
                .o_afull      (     ), 
                .rden         (mib_to_sys_fifo_rden), 
                .rdata        (mib_to_sys_fifo_rdata), 
                .rdata_vld    (mib_to_sys_fifo_rdata_vld));
    
        end else begin
    pmi_fifo_dc_fwft_v1_0 #(
        .WR_DEPTH        (4         ), 
        .WR_DEPTH_AFULL  (3         ), 
        .WR_WIDTH        (DATA_BITS ), 
        .RD_WIDTH        (DATA_BITS ), 
        .FAMILY          ("ECP5U"   ), 
        .IMPLEMENTATION  ("LUT"     ), 
        .RESET_MODE      ("sync"    ), 
        .WORD_SWAP       (0         ), 
        .SIM_MODE        (SIM_MODE  )
        ) MIB_TO_SYS_FIFO (
        .wrclk           (i_mib_clk                 ), 
        .wrclk_rst       (i_mib_srst                ), 
        .rdclk           (i_sys_clk                 ), 
        .rdclk_rst       (i_sys_srst                ), 
        .wren            (mib_to_sys_fifo_wren      ), 
        .wdata           (mib_to_sys_fifo_wdata     ), 
        .full            (), 
        .afull           (), 
        .rden            (mib_to_sys_fifo_rden      ), 
        .rdata           (mib_to_sys_fifo_rdata     ), 
        .rdata_vld       (mib_to_sys_fifo_rdata_vld ));
         end 
    endgenerate
    
    assign mib_to_sys_fifo_rden = mib_to_sys_fifo_rdata_vld; // auto read the FIFO
    assign cmd_sys.ack          = mib_to_sys_fifo_rdata_vld;
    assign cmd_sys.rdata        = mib_to_sys_fifo_rdata;
    
endmodule

`default_nettype wire
