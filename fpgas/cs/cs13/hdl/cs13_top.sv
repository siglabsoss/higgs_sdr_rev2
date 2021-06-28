//---------------------------------------------------------
// Design Name : higgs_sdr
// File Name   : cs13.sv
// Authors     : FPGA Group
// Modified    : 
// Function    : 
//-----------------------------------------------------

`include "udp_cmd_pkg.sv"           // located in ip-library/lattice_support/gbit_mac/packages

`default_nettype none

module cs13_top (
    input  wire              CLK,
    output wire              FPGA_LED,
    input  wire    [12:2]    HS_NORTH_IN,  // 12 = UDP packet byte vld, 11 = UDP packet last byte, 10 = UDP packet byte parity (even), 9:2 = UDP packet byte
    output wire    [12:2]    HS_SOUTH_OUT, // SAME AS HS_NORTH_IN[12:2]
    // wire [47:0] HS_NORTH,
    // wire [47:0] HS_EAST,
    // wire [47:0] HS_SOUTH,
    // wire [47:0] HS_WEST,
    //output wire    [22:0]    LS_NORTH,
    //input  wire    [22:0]    LS_EAST,
    //output wire    [22:0]    LS_SOUTH,
    //output wire    [22:0]    LS_WEST,
    input  wire              MIB_MASTER_RESET,
    input  wire              MIB_COUNTER_LOCK,
    input  wire              MIB_TBIT,     // Training bit toggle pattern from MIB master
    input  wire              MIB_START,    // from master, starts a mib transaction
    input  wire              MIB_RD_WR_N,  // 1 = read, 0 = write
    output wire              MIB_SLAVE_ACK,
    inout  wire    [15:0]    MIB_AD    
);

    // SYSTEM SPECIFIC LOCAL PARAMETERS - DO NOT MODIFY
    localparam logic [3:0] SLAVE_MIB_ADDR_MSN = 4'd7;  // Unique MIB Address Most Significant Nibble  
    localparam logic [7:0] FPGA_UID           = 8'h13; // Software readable Unique FPGA ID stored in base register
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

    core_top #(
        .CLOCK_SHIFT_TRAINING_COUNTER_LIMIT (100),
        .NUM_SYS_CLK_SRSTS                  (NUM_SYS_CLK_RESETS),
        .SYS_CLK_SRSTS_EXTRA_CLOCKS         (SYS_CLK_RESETS_EXTRA_CLOCKS), 
        .NUM_MIB_CLK_SRSTS                  (1),
        .MIB_CLK_SRSTS_EXTRA_CLOCKS         ('{100}),
        .INT_OSC_DIV_VAL                    (12),
        .NUM_INT_OSC_SRST_CLOCKS            (128),
        .MIB_SLAVE_ADDR_MSN                 (SLAVE_MIB_ADDR_MSN) 
    ) core_top (
        .i_fpga_clk          (CLK),
        .i_fpga_ext_arst     (MIB_MASTER_RESET),
        .o_int_osc_clk       (),
        .o_int_osc_clk_srst  (),
        .o_sys_clk           (sys_clk),
        .o_sys_clk_srsts     ({sys_clk_srst}),
        .o_mib_clk           (mib_clk),
        .o_mib_clk_srsts     ({mib_clk_srst}), 
        .o_mib_deskew_done   (mib_clk_deskew_done),
        .o_sys_pll_locked    (sys_pll_locked),
        .i_mib_tbit          (MIB_TBIT),
        .i_mib_start         (MIB_START),
        .i_mib_rd_wr_n       (MIB_RD_WR_N),
        .b_mib_ad            (MIB_AD),
        .o_mib_slave_ack     (MIB_SLAVE_ACK),
        .cmd_sys             (cmd_sys)
    );

    assign FPGA_LED = sys_pll_locked & mib_clk_deskew_done;

    
    /*
     * 
     * PASSTHROUGH REGISTERS
     * 
     */

    /*
     * NORTH TO SOUTH
     */
    
    logic [12:2] hs_n2s_in_regs;
    logic [12:2] hs_n2s_mid_regs;
    logic [12:2] hs_n2s_out_regs;
    
    logic        rx_sample_udp_byte_parity_error;
    
    always_ff @(posedge sys_clk) begin
        if (sys_clk_srst) begin
            hs_n2s_out_regs                 <= '0;
            hs_n2s_mid_regs                 <= '0;
            hs_n2s_in_regs                  <= '0;
            rx_sample_udp_byte_parity_error <= 0;
        end else begin
            hs_n2s_in_regs                  <= HS_NORTH_IN[12:2];
            hs_n2s_mid_regs                 <= hs_n2s_in_regs;
            hs_n2s_out_regs                 <= hs_n2s_mid_regs;
            rx_sample_udp_byte_parity_error <= hs_n2s_mid_regs[12] & (^hs_n2s_mid_regs[10:2]);
        end
    end
    
    assign HS_SOUTH_OUT[12:2] = hs_n2s_out_regs;
    
    
    /*
     * REGISTERS 
     * 
     * NOTE:
     *
     * I used .* for register field connections on purpose because if you update the reg.rdl file under the regs folder you might get additional 
     * top level ports I want an error to be raised in the event this logic fails to get updated too.
     * 
     */
    
    logic [ 7:0] h2l_rf0_fpga_uid_uid_w;
    logic        h2l_rf0_rx_sample_udp_byte_perrs_cnt_incr;

    assign h2l_rf0_fpga_uid_uid_w                    = FPGA_UID;
    assign h2l_rf0_rx_sample_udp_byte_perrs_cnt_incr = rx_sample_udp_byte_parity_error;


    REGS_pio regs (
        .*,
        .clk                    (sys_clk), 
        .reset                  (sys_clk_srst), 
        .h2d_pio_dec_address    (cmd_sys.byte_addr[CMD_ADDR_BITS-1:2]), // dword addressing so bits [1:0] are always 0 
        .h2d_pio_dec_write_data (cmd_sys.wdata),
        .h2d_pio_dec_write      (cmd_sys.sel & ~cmd_sys.rd_wr_n),
        .h2d_pio_dec_read       (cmd_sys.sel & cmd_sys.rd_wr_n),
        .d2h_dec_pio_read_data  (cmd_sys.rdata),
        .d2h_dec_pio_ack        (cmd_sys.ack),
        .d2h_dec_pio_nack       ());

endmodule

`default_nettype wire
