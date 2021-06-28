//---------------------------------------------------------
// Design Name : higgs_sdr
// File Name   : cs22_top.sv
// Authors     : FPGA Group
// Modified    :
// Function    :
//-----------------------------------------------------

`ifndef VERILATE_DEF
// This is used during bitfile compilation
// `define QENGINE_LITE
`else
// This if is only used for Verilator
`ifdef CS22_QENGINE_LITE
`define QENGINE_LITE
`endif
`endif

`include "udp_cmd_pkg.sv"           // located in ip-library/lattice_support/gbit_mac/packages

`default_nettype none

module cs22_top #(
      parameter VERILATE = 1'b0,
      parameter VMEM_DEPTH = 4096,
        parameter SCALAR_MEM_0 = "scalar0.mif",
        parameter SCALAR_MEM_1 = "scalar1.mif",
        parameter SCALAR_MEM_2 = "scalar2.mif",
        parameter SCALAR_MEM_3 = "scalar3.mif",
        parameter VMEM0 = "vmem0.mif",
        parameter VMEM1 = "vmem1.mif",
        parameter VMEM2 = "vmem2.mif",
        parameter VMEM3 = "vmem3.mif",
        parameter VMEM4 = "vmem4.mif",
        parameter VMEM5 = "vmem5.mif",
        parameter VMEM6 = "vmem6.mif",
        parameter VMEM7 = "vmem7.mif",
        parameter VMEM8 = "vmem8.mif",
        parameter VMEM9 = "vmem9.mif",
        parameter VMEM10 = "vmem10.mif",
        parameter VMEM11 = "vmem11.mif",
        parameter VMEM12 = "vmem12.mif",
        parameter VMEM13 = "vmem13.mif",
        parameter VMEM14 = "vmem14.mif",
        parameter VMEM15 = "vmem15.mif"
        )
      (
    input wire          CLK,
    output wire         FPGA_LED,

    input wire [34:2]   HS_SOUTH_IN, // 34 = valid, 33:2 = data
    output wire [35:35] HS_SOUTH_OUT, // ready
    input wire [36:36]  HS_SOUTH_IN_LAST,

    output wire [34:2]  HS_WEST_OUT, // 34 = valid, 33:2 = data
    input wire [35:35]  HS_WEST_IN, // ready
    output wire [36:36] HS_WEST_OUT_LAST,
    // wire [47:0] HS_NORTH,
    // wire [47:0] HS_EAST,
    // wire [47:0] HS_SOUTH,
    // wire [47:0] HS_WEST,
    //output wire    [22:0]    LS_NORTH,
    //input wire    [22:0]     LS_EAST,
    //output wire    [22:0]    LS_SOUTH,
    //output wire    [22:0]    LS_WEST,
    /*********** Ring bus ***************/
    input wire [47:47]  HS_NORTH_IN_RB,
    output reg [47:47]  HS_SOUTH_OUT_RB, 
    /************************************/

`ifdef VERILATE_DEF
    output wire [31:0]  snap_riscv_out_data,
    output wire         snap_riscv_out_last,
    output wire         snap_riscv_out_valid,
    output wire         snap_riscv_out_ready,

    output wire [31:0]  snap_riscv_in_data,
    output wire         snap_riscv_in_last,
    output wire         snap_riscv_in_valid,
    output wire         snap_riscv_in_ready,
    output              snap_io_uart_txd,
    input               snap_io_uart_rxd,
`endif

    input wire          MIB_MASTER_RESET
//    input  wire              MIB_COUNTER_LOCK,
//    input  wire              MIB_TBIT,     // Training bit toggle pattern from MIB master
//    input  wire              MIB_START,    // from master, starts a mib transaction
//    input  wire              MIB_RD_WR_N,  // 1 = read, 0 = write
//    output wire              MIB_SLAVE_ACK,
//    inout  wire    [15:0]    MIB_AD,
//    output wire    [14:0]    DDR_A,
//    output wire    [2:0]     DDR_BA,
//    output wire              DDR_CAS_N,
//    output wire              DDR_CKE,
//    output wire              DDR_CK,
//    output wire              DDR_CS_N,
//    inout  wire    [31:0]    DDR_DQ,
//    inout  wire    [3:0]     DDR_DQS,
//    output wire    [3:0]     DDR_DM,
//    output wire              DDR_ODT,
//    output wire              DDR_RAS_N,
//    output wire              DDR_RESET_N,
//    output wire              DDR_WE_N,
// /* output wire    [31:0]    o_data,
// output wire           o_data_valid, */
// output wire           o_waste
);



    // SYSTEM SPECIFIC LOCAL PARAMETERS - DO NOT MODIFY
    localparam logic [3:0] SLAVE_MIB_ADDR_MSN = 4'd1;  // Unique MIB Address Most Significant Nibble
    localparam logic [7:0] FPGA_UID           = 8'h01; // Software readable Unique FPGA ID stored in base register
    // End SYSTEM SPECIFIC LOCAL PARAMETERS


    /*
     *
     * CLOCKING, RESETS, AND MIB SLAVE
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

//    core_top #(
//        .CLOCK_SHIFT_TRAINING_COUNTER_LIMIT (100),
//        .NUM_SYS_CLK_SRSTS                  (NUM_SYS_CLK_RESETS),
//        .SYS_CLK_SRSTS_EXTRA_CLOCKS         (SYS_CLK_RESETS_EXTRA_CLOCKS),
//        .NUM_MIB_CLK_SRSTS                  (1),
//        .MIB_CLK_SRSTS_EXTRA_CLOCKS         ('{100}),
//        .INT_OSC_DIV_VAL                    (12),
//        .NUM_INT_OSC_SRST_CLOCKS            (128),
//        .MIB_SLAVE_ADDR_MSN                 (SLAVE_MIB_ADDR_MSN)
//    ) core_top (
//        .i_fpga_clk          (CLK),
//        .i_fpga_ext_arst     (MIB_MASTER_RESET),
//        .o_int_osc_clk       (),
//        .o_int_osc_clk_srst  (),
//        .o_sys_clk           (sys_clk),
//        .o_sys_clk_srsts     ({sys_clk_srst}),
//        .o_mib_clk           (mib_clk),
//        .o_mib_clk_srsts     ({mib_clk_srst}),
//        .o_mib_deskew_done   (mib_clk_deskew_done),
//        .o_sys_pll_locked    (sys_pll_locked),
//        .i_mib_tbit          (MIB_TBIT),
//        .i_mib_start         (MIB_START),
//        .i_mib_rd_wr_n       (MIB_RD_WR_N),
//        .b_mib_ad            (MIB_AD),
//        .o_mib_slave_ack     (MIB_SLAVE_ACK),
//        .cmd_sys             (cmd_sys)
//    );
//
//
//    assign FPGA_LED = sys_pll_locked & mib_clk_deskew_done;

    core_top #(
         .CLOCK_SHIFT_TRAINING_COUNTER_LIMIT (100),
         .NUM_SYS_CLK_SRSTS                  (NUM_SYS_CLK_RESETS),
         .SYS_CLK_SRSTS_EXTRA_CLOCKS         (SYS_CLK_RESETS_EXTRA_CLOCKS),
         .NUM_MIB_CLK_SRSTS                  (1),
         .MIB_CLK_SRSTS_EXTRA_CLOCKS         (1'b0),
         .INT_OSC_DIV_VAL                    (12),
         .NUM_INT_OSC_SRST_CLOCKS            (128),
         .MIB_SLAVE_ADDR_MSN                 (SLAVE_MIB_ADDR_MSN),
         .MIB_CLOCK_DESKEW_ENABLE            (1'b0),
         .INCLUDE_MIB_SLAVE                  (1'b0),
         .VERILATE                           (VERILATE)
      ) core_top (
         .i_fpga_clk          (CLK),
         .i_fpga_ext_arst     (MIB_MASTER_RESET),
         .o_int_osc_clk       (),
         .o_int_osc_clk_srst  (),
         .o_sys_clk           (sys_clk),
         .o_sys_clk_srsts     (sys_clk_srst),
         .o_mib_clk           (mib_clk),
         .o_mib_clk_srsts     ({mib_clk_srst}),
         .o_mib_deskew_done   (), // not used in FPGA with MIB Master
         .o_sys_pll_locked    (sys_pll_locked),
         .i_mib_tbit          (1'b0),
         .i_mib_start         (1'b0),
         .i_mib_rd_wr_n       (1'b0),
         .b_mib_ad            (),
         .o_mib_slave_ack     (),
         .cmd_sys             (cmd_sys)
      );

         logic [21:0] gpio;
       assign FPGA_LED = gpio[21];


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /*
     *
     * RISC-V processor
     *
     */

`ifdef CS22_NO_RISCV
    vex_machine_top_gutted #(
            .NO_RISCV(1),
`else
    vex_machine_top #(
`endif
            .VMEM_DEPTH (VMEM_DEPTH),
            .SCALAR_MEM_0 (SCALAR_MEM_0),
            .SCALAR_MEM_1 (SCALAR_MEM_1),
            .SCALAR_MEM_2 (SCALAR_MEM_2),
            .SCALAR_MEM_3 (SCALAR_MEM_3),
            .VMEM0 (VMEM0),
            .VMEM1 (VMEM1),
            .VMEM2 (VMEM2),
            .VMEM3 (VMEM3),
            .VMEM4 (VMEM4),
            .VMEM5 (VMEM5),
            .VMEM6 (VMEM6),
            .VMEM7 (VMEM7),
            .VMEM8 (VMEM8),
            .VMEM9 (VMEM9),
            .VMEM10 (VMEM10),
            .VMEM11 (VMEM11),
            .VMEM12 (VMEM12),
            .VMEM13 (VMEM13),
            .VMEM14 (VMEM14),
            .VMEM15 (VMEM15)
      )
      vex_machine_top_inst (
         .clk                       (sys_clk),
         .reset                        (sys_clk_srst),
         .debugReset                   (sys_clk_srst),

         .t0_data                   (HS_SOUTH_IN[33:2]),
                            .t0_last(HS_SOUTH_IN_LAST[36:36]),
         .t0_valid                     (HS_SOUTH_IN[34]),
         .t0_ready                     (HS_SOUTH_OUT[35]),
         .i0_data                   (HS_WEST_OUT[33:2]),
                            .i0_last(HS_WEST_OUT_LAST[36:36]),
         .i0_valid                     (HS_WEST_OUT[34]),
         .i0_ready                     (HS_WEST_IN[35]),
         .i_ringbus                    (HS_NORTH_IN_RB[47]),
         .o_ringbus                    (HS_SOUTH_OUT_RB[47]),

`ifdef VERILATE_DEF
            .snap_riscv_out_data                (snap_riscv_out_data),
            .snap_riscv_out_last                (snap_riscv_out_last),
            .snap_riscv_out_valid               (snap_riscv_out_valid),
            .snap_riscv_out_ready               (snap_riscv_out_ready),
            .snap_riscv_in_data                 (snap_riscv_in_data),
            .snap_riscv_in_last                 (snap_riscv_in_last),
            .snap_riscv_in_valid                (snap_riscv_in_valid),
            .snap_riscv_in_ready                (snap_riscv_in_ready),
            .io_uart_txd                        (snap_io_uart_txd),
            .io_uart_rxd                        (snap_io_uart_rxd),
`endif


            .gpio                               (gpio)
      );

endmodule

`default_nettype wire
