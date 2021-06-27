//---------------------------------------------------------
// Design Name : higgs_sdr
// File Name   : cs03.sv
// Authors     : FPGA Group
// Modified    :
// Function    :
//-----------------------------------------------------

//`include "udp_cmd_pkg.sv" // located in ip-library/lattice_support/gbit_mac/packages

`default_nettype none

module cs03_top
    #(parameter VERILATE = 1'b0)
    (
    input  wire               CLK,
    output wire               FPGA_LED,
    //output wire    [22:0]    LS_NORTH,
    //output wire    [47:0]    HS_NORTH,
    //output wire    [22:0]    LS_EAST,
    //output wire    [47:0]    HS_EAST,
    //output wire    [22:0]    LS_WEST,
    //output wire    [47:0]    HS_WEST,
    //output wire    [22:0]    LS_SOUTH,
    //output wire    [47:0]    HS_SOUTH,
    // output  wire              MIB_MASTER_RESET,
    output  wire             MIB_COUNTER_LOCK,
    input  wire              MIB_TBIT, // Training bit toggle pattern from MIB master
    input  wire              MIB_START,
    input  wire              MIB_RD_WR_N,
    output wire              MIB_SLAVE_ACK,
    inout  wire    [15:0]    MIB_AD
    );

    // SYSTEM SPECIFIC LOCAL PARAMETERS - DO NOT MODIFY
    localparam SLAVE_MIB_ADDR_MSN   = 4'd15; // Unique MIB Address Most Significant Nibble
    localparam FPGA_UID             = 8'h33; // Software readable Unique FPGA ID stored in base register
    // End SYSTEM SPECIFIC LOCAL PARAMETERS


    /*
     *
     * CLOCKING, RESETS, AND MIB SLAVE
     *
     */

    //localparam int NUM_SYS_CLK_RESETS                                   = 1;
    //localparam int SYS_CLK_RESETS_EXTRA_CLOCKS [0:NUM_SYS_CLK_RESETS-1] = '{0};

    logic       sys_clk               /* synthesis syn_keep=1 */;
    logic       sys_clk_srst          /* synthesis syn_keep=1 GSR=DISABLED */;
    logic       mib_clk               /* synthesis syn_keep=1 */;
    logic       mib_clk_srst          /* synthesis syn_keep=1 GSR=DISABLED */;
    logic       int_osc_clk           /* synthesis syn_keep=1 */;
    logic       int_osc_clk_srst      /* synthesis syn_keep=1 GSR=DISABLED */;
    logic       sys_pll_locked        /* synthesis syn_keep=1 */;
    logic       mib_clk_deskew_done   /* synthesis syn_keep=1 */;

    //intf_cmd #(CMD_ADDR_BITS, CMD_DATA_BITS) cmd_sys(); // parameters specified in udp_cmd_pkg.sv

    // core_top #(
    //     .CLOCK_SHIFT_TRAINING_COUNTER_LIMIT (100),
    //     .NUM_SYS_CLK_SRSTS                  (NUM_SYS_CLK_RESETS),
    //     .SYS_CLK_SRSTS_EXTRA_CLOCKS         (SYS_CLK_RESETS_EXTRA_CLOCKS),
    //     .NUM_MIB_CLK_SRSTS                  (1),
    //     .MIB_CLK_SRSTS_EXTRA_CLOCKS         ('{100}),
    //     .INT_OSC_DIV_VAL                    (12),
    //     .NUM_INT_OSC_SRST_CLOCKS            (128),
    //     .MIB_SLAVE_ADDR_MSN                 (SLAVE_MIB_ADDR_MSN)
    // ) core_top (
    //     .i_fpga_clk          (CLK),
    //     .i_fpga_ext_arst     (MIB_MASTER_RESET),
    //     .o_int_osc_clk       (),
    //     .o_int_osc_clk_srst  (),
    //     .o_sys_clk           (sys_clk),
    //     .o_sys_clk_srsts     ({sys_clk_srst}),
    //     .o_mib_clk           (mib_clk),
    //     .o_mib_clk_srsts     ({mib_clk_srst}),
    //     .o_mib_deskew_done   (mib_clk_deskew_done),
    //     .o_sys_pll_locked    (sys_pll_locked),
    //     .i_mib_tbit          (MIB_TBIT),
    //     .i_mib_start         (MIB_START),
    //     .i_mib_rd_wr_n       (MIB_RD_WR_N),
    //     .b_mib_ad            (MIB_AD),
    //     .o_mib_slave_ack     (MIB_SLAVE_ACK),
    //     .cmd_sys             (cmd_sys)
    // );

    // core_top #(
    //     // most params use defaults, which makes it easier to apply global changes (i.e. you don't have to go FPGA-to-FPGA to make changes)
    //     .NUM_SYS_CLK_SRSTS                  (NUM_SYS_CLK_RESETS),
    //     .SYS_CLK_SRSTS_EXTRA_CLOCKS         (SYS_CLK_RESETS_EXTRA_CLOCKS),
    //     // .MIB_SLAVE_ADDR_MSN                 (SLAVE_MIB_ADDR_MSN),
    //     .INCLUDE_MIB_SLAVE                  (1'b0),
    //     .MIB_CLOCK_DESKEW_ENABLE            (1'b0),
    //     .VERILATE                           (VERILATE)
    // ) core_top (
    //     .i_fpga_clk          (CLK),
    //     .i_fpga_ext_arst     (int_osc_clk_srst), // loop back internal oscillator srst to reset sys clock domain (we're the MIB_MASTER_RESET driver, so we don't use that to reset sys clock domain)
    //     .o_int_osc_clk       (int_osc_clk),
    //     .o_int_osc_clk_srst  (int_osc_clk_srst),
    //     .o_sys_clk           (sys_clk),
    //     .o_sys_clk_srsts     (sys_clk_srst),
    //     .o_mib_clk           (mib_clk),
    //     .o_mib_clk_srsts     (mib_clk_srst),
    //     .o_mib_deskew_done   (),
    //     .o_sys_pll_locked    (sys_pll_locked),
    //     .i_mib_tbit          (),
    //     .i_mib_start         (),
    //     .i_mib_rd_wr_n       (),
    //     .b_mib_ad            (),
    //     .o_mib_slave_ack     (),
    //     .cmd_sys             (cmd_sys)
    // );
    assign MIB_COUNTER_LOCK = 1'b1;
    assign FPGA_LED = MIB_COUNTER_LOCK;
    // assign FPGA_LED = sys_pll_locked && !MIB_MASTER_RESET;

    //  logic custom_reset = 1;
    // core_reset #(
    //     .NUM_OUTPUTS        (1),
    //     .VERILATE           (VERILATE),
    //     // .EXTRA_RESET_CLOCKS ('{CLKS_TO_DRIVE_MIB_STROBES}) // use this to extend the duration of the MIB_MASTER_RESET to as long as needed
    //     .EXTRA_RESET_CLOCKS (1) // use this to extend the duration of the MIB_MASTER_RESET to as long as needed
    // ) mib_master_reset (
    //     .i_ext_arst         (int_osc_clk_srst | custom_reset),
    //     .i_clk              (sys_clk),
    //     .i_clk_pll_unlocked (~sys_pll_locked),
    //     .o_sync_resets      ({MIB_MASTER_RESET})
    // );
    //
    // logic [35:0] timer_counter;
    // always_ff @(posedge sys_clk) begin
    //     if (sys_clk_srst) begin
    //         timer_counter <= 0;
    //         custom_reset <= 1;
    //     end else begin
    //         timer_counter <= timer_counter + 1;
    //         if (timer_counter == 36'd125_000_000 /*1 second */) begin
    //         // if (timer_counter == 36'd18_750_000_000 /*150 seconds */) begin
    //             custom_reset <= 0;
    //             timer_counter <= timer_counter;
    //         end
    //     end
    // end

    /*
     *
     * REGISTERS
     *
     * NOTE:
     *
     * I used .* for register field connections on purpose because if you update the reg.rdl file under the regs folder you might get additional
     * top level ports I want an error to be raised in the event this logic fails to get updated too.
     *
     */

    // logic [ 7:0]              h2l_rf0_fpga_uid_uid_w;

    // assign h2l_rf0_fpga_uid_uid_w = FPGA_UID;

    // REGS_pio regs (
    //     .clk                    (sys_clk),
    //     .reset                  (sys_clk_srst),
    //     .h2d_pio_dec_address    (cmd_sys.byte_addr[CMD_ADDR_BITS-1:2]), // dword addressing so bits [1:0] are always 0
    //     .h2d_pio_dec_write_data (cmd_sys.wdata),
    //     .h2d_pio_dec_write      (cmd_sys.sel & ~cmd_sys.rd_wr_n),
    //     .h2d_pio_dec_read       (cmd_sys.sel & cmd_sys.rd_wr_n),
    //     .d2h_dec_pio_read_data  (cmd_sys.rdata),
    //     .d2h_dec_pio_ack        (cmd_sys.ack),
    //     .d2h_dec_pio_nack       (),
    //     .*);

endmodule

`default_nettype wire
