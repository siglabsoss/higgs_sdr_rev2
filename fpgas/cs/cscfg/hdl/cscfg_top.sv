//---------------------------------------------------------
// Design Name : higgs_sdr
// File Name   : cscfg_top.sv
// Authors     : FPGA Group
// Modified    :
// Function    :
//-----------------------------------------------------

`include "udp_cmd_pkg.sv" // located in ip-library/lattice_support/gbit_mac/packages

`default_nettype none

module cscfg_top #(
    parameter bit        SIM_MODE = 0,
    parameter            VERILATE = 1'b0
)(
    input  wire          CLK,             // System Clock
    output wire          FPGA_LED,
//    input wire ext_mcu_arst,    // System reset.

    /* MIB FROM GRAVITON */
    // output  wire         GRAV_MIB_MASTER_RESET,
    // input  wire          GRAV_MIB_COUNTER_LOCK,
    // input  wire          GRAV_MIB_TBIT, // Training bit toggle pattern from MIB master
    // input  wire          GRAV_MIB_START,
    // input  wire          GRAV_MIB_RD_WR_N,
    // output wire          GRAV_MIB_SLAVE_ACK,
    // inout  wire  [15:0]  GRAV_MIB_AD,

    /* MIB ON CS */
    output wire           CS_MIB_MASTER_RESET,
    input wire            CS_MIB_COUNTER_LOCK,

    // FMC Interface
//    input wire         clk54_ext_a, // FMC source synchronous clock
//    input wire [24:0]  fmc_a,       // address bus (we don't use FMC_A[0] because we only need the address from the first write or read cycle)
//    inout wire [15:0]  fmc_d,       // data bus
//    input wire         fmc_ne1,     // chip select (active low), doesn't toggle between consecutive accesses
//    input wire         fmc_noe,     // output enable (active low)
//    input wire         fmc_nwe,     // write enable (active low)
//    output wire        fmc_nwait,   // wait (active low)

    // internal interface - memory
    // Not needed, we'll wire them up


    // internal interface - MIB master copper suicide
    // Not needed, we'll wire them up


    // clk out for slaves making it source synchronous.
    output wire [15:0]  PROG_N
);

    assign PROG_N = '1;

    // NOTE: KEEP IN MIND THAT THE CFG FPGA ON COPPER SUICIDE IS TREATED AS PART OF GRAVITON'S MIB BUS

    // SYSTEM SPECIFIC LOCAL PARAMETERS - DO NOT MODIFY
    localparam logic [3:0]  SLAVE_MIB_ADDR_MSN   = 4'd4;                        // Unique MIB Address Most Significant Nibble
    localparam logic [31:0] FPGA_UID            = {8'h43, 8'h43, 8'h46, 8'h47}; // Software readable Unique FPGA ID stored in base register (ASCII "CCFG")
    // End SYSTEM SPECIFIC LOCAL PARAMETERS


    /*
     *
     * CLOCKING, RESETS, AND MIB SLAVE
     *
     * NOTE: KEEP IN MIND THAT THE CFG FPGA ON COPPER SUICIDE IS TREATED AS BEING PART OF GRAVITON'S MIB BUS, NOT CS's MIB BUS
     *
     */

    localparam int NUM_SYS_CLK_RESETS                                   = 1;
    localparam int SYS_CLK_RESETS_EXTRA_CLOCKS [0:NUM_SYS_CLK_RESETS-1] = '{0};

    logic       sys_clk               /* synthesis syn_keep=1 */;
    logic       sys_clk_srst          /* synthesis syn_keep=1 GSR=DISABLED */;
    logic       mib_clk               /* synthesis syn_keep=1 */;
    logic       mib_clk_srst          /* synthesis syn_keep=1 GSR=DISABLED */;
    logic       int_osc_clk           /* synthesis syn_keep=1 */;
    logic       int_osc_clk_srst      /* synthesis syn_keep=1 GSR=DISABLED */;
    logic       sys_pll_locked        /* synthesis syn_keep=1 */;
    logic       mib_clk_deskew_done   /* synthesis syn_keep=1 */;

    intf_cmd #(CMD_ADDR_BITS, CMD_DATA_BITS) cmd_sys(); // parameters specified in udp_cmd_pkg.sv

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
    //     .i_fpga_ext_arst     (CS_MIB_MASTER_RESET),
    //     .o_int_osc_clk       (int_osc_clk),
    //     .o_int_osc_clk_srst  (int_osc_clk_srst),
    //     .o_sys_clk           (sys_clk),
    //     .o_sys_clk_srsts     ({sys_clk_srst}),
    //     .o_mib_clk           (mib_clk),
    //     .o_mib_clk_srsts     ({mib_clk_srst}),
    //     .o_mib_deskew_done   (mib_clk_deskew_done),
    //     .o_sys_pll_locked    (sys_pll_locked),
    //     .i_mib_tbit          (GRAV_MIB_TBIT),
    //     .i_mib_start         (GRAV_MIB_START),
    //     .i_mib_rd_wr_n       (GRAV_MIB_RD_WR_N),
    //     .b_mib_ad            (GRAV_MIB_AD),
    //     .o_mib_slave_ack     (GRAV_MIB_SLAVE_ACK),
    //     .cmd_sys             (cmd_sys)
    // );

    core_top #(
        // most params use defaults, which makes it easier to apply global changes (i.e. you don't have to go FPGA-to-FPGA to make changes)
        .NUM_SYS_CLK_SRSTS                  (NUM_SYS_CLK_RESETS),
        .SYS_CLK_SRSTS_EXTRA_CLOCKS         (SYS_CLK_RESETS_EXTRA_CLOCKS),
        // .MIB_SLAVE_ADDR_MSN                 (SLAVE_MIB_ADDR_MSN),
        .INCLUDE_MIB_SLAVE                  (1'b0),
        .MIB_CLOCK_DESKEW_ENABLE            (1'b0),
        .VERILATE                           (VERILATE)
    ) core_top (
        .i_fpga_clk          (CLK),
        .i_fpga_ext_arst     (int_osc_clk_srst), // loop back internal oscillator srst to reset sys clock domain (we're the MIB_MASTER_RESET driver, so we don't use that to reset sys clock domain)
        .o_int_osc_clk       (int_osc_clk),
        .o_int_osc_clk_srst  (int_osc_clk_srst),
        .o_sys_clk           (sys_clk),
        .o_sys_clk_srsts     (sys_clk_srst),
        .o_mib_clk           (mib_clk),
        .o_mib_clk_srsts     (mib_clk_srst),
        .o_mib_deskew_done   (),
        .o_sys_pll_locked    (sys_pll_locked),
        .i_mib_tbit          (),
        .i_mib_start         (),
        .i_mib_rd_wr_n       (),
        .b_mib_ad            (),
        .o_mib_slave_ack     (),
        .cmd_sys             (cmd_sys)
    );

    assign FPGA_LED = 1'b1; //sys_pll_locked & mib_clk_deskew_done;


    /*
     *
     * GRAVITION MIB MASTER RESET AND COUNTER LOCK FORWARDING TO CS MIB MASTER RESET AND COUNTER LOCK
     *
     */

    // see Lattice ECP5 Library guide for descriptions of OFS1P3BX and IFS1P3BX

    // OFS1P3BX CS_MASTER_RESET_OUT_FF (
    //     .SCLK (sys_clk),
    //     .SP   (1'b1),
    //     .PD   (GRAV_MIB_MASTER_RESET),
    //     .D    (1'b0),
    //     .Q    (CS_MIB_MASTER_RESET));

   logic custom_reset = 1;

   core_reset #(
       .NUM_OUTPUTS        (1),
       .VERILATE           (VERILATE),
       // .EXTRA_RESET_CLOCKS ('{CLKS_TO_DRIVE_MIB_STROBES}) // use this to extend the duration of the MIB_MASTER_RESET to as long as needed
       .EXTRA_RESET_CLOCKS (1) // use this to extend the duration of the MIB_MASTER_RESET to as long as needed
   ) mib_master_reset (
       .i_ext_arst         (int_osc_clk_srst | custom_reset),
       .i_clk              (sys_clk),
       .i_clk_pll_unlocked (~sys_pll_locked),
       .o_sync_resets      ({CS_MIB_MASTER_RESET})
   );

   logic deassert_reset;
   always_ff @(posedge sys_clk) begin
       if (sys_clk_srst) begin
           custom_reset <= 1;
           deassert_reset <= 0;
       end else begin
          deassert_reset <= CS_MIB_COUNTER_LOCK;
          custom_reset <= !deassert_reset;
       end
   end

    // OFS1P3BX CS_MASTER_RESET_OUT_FF (
    //     .SCLK (sys_clk),
    //     .SP   (1'b1),
    //     .PD   (CS_MIB_MASTER_RESET),
    //     .D    (1'b0),
    //     .Q    (GRAV_MIB_MASTER_RESET));

    // wire cntr_lock_int;
    //
    // IFS1P3BX CS_COUNTER_LOCK_IN_FF (
    //     .SCLK (sys_clk),
    //     .SP   (1'b1),
    //     .PD   (1'b0),
    //     .D    (GRAV_MIB_COUNTER_LOCK),
    //     .Q    (cntr_lock_int));
    //
    // OFS1P3BX CS_COUNTER_LOCK_OUT_FF (
    //     .SCLK (sys_clk),
    //     .SP   (1'b1),
    //     .PD   (1'b0),
    //     .D    (cntr_lock_int),
    //     .Q    (CS_MIB_COUNTER_LOCK));


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

    logic [31:0] h2l_rf0_fpga_uid_uid_w;

    assign h2l_rf0_fpga_uid_uid_w = FPGA_UID;

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

//cfg_fpga_comm # (
//    .SIM_MODE(SIM_MODE)
//)
//_cfg_fpga_comm
//(
//    .clk54_ext_a        (clk54_ext_a      ),
//    .CLK                (CLK              ),
//    .ext_mcu_arst       (ext_mcu_arst     ),
//    .fmc_a              (fmc_a            ),
//    .fmc_d              (fmc_d            ),
//    .fmc_ne1            (fmc_ne1          ),
//    .fmc_noe            (fmc_noe          ),
//    .fmc_nwe            (fmc_nwe          ),
//    .fmc_nwait          (fmc_nwait        ),
//    //.cfg_mib_timeout    (cfg_mib_timeout  ),
//    .cfg_mib_d          (cfg_mib_d        ),
//    .cfg_mib_start      (cfg_mib_start    ),
//    .cfg_mib_rd_wr_n    (cfg_mib_rd_wr_n  ),
//    .cfg_mib_slave_ack  (cfg_mib_slave_ack),
//    //.cfg_mib_ad_high_z  (cfg_mib_ad_high_z),
//    .o_clk_out          (o_clk_out        ),
//    .o_rst_out          (o_rst_out        )
//    );

endmodule

`default_nettype wire
