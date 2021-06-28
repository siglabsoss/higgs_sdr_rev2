/*
 * Module: eth_top
 *
 */

`ifndef VERILATE_DEF
// This is used during bitfile compilation
`define QENGINE_LITE
`define NO_NCO
`else
// This if is only used for Verilator
`ifdef ETH_QENGINE_LITE
`define QENGINE_LITE
`endif
`endif


// If verilator but not ETH_USE_MEGA_WRAPPER
`ifdef VERILATE_DEF
`ifndef ETH_USE_MEGA_WRAPPER
`define EXCLUDE_MEGAWRAPPER
`endif
`endif

`include "udp_cmd_pkg.sv" // located in ip-library/lattice_support/gbit_mac/packages
`include "higgs_sdr_global_pkg.sv"

`default_nettype none

module eth_top #(
                 parameter VERILATE = 1'b0,
                 parameter VMEM_DEPTH = 8, // WARNING THIS IS NOT USED, actually is 4096.  see higgs issue #109
                 parameter UDP_RINGBUS_PACKET_SIZE_WORDS = 1,  // must match UDP_RINGBUS_PAYLOAD_BYTES / 4
                 parameter UDP_CS30_PACKET_SIZE_WORDS = 367,
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

    /* NAMES CHOSEN TO CLOSELY MATCH GRAVITON SCHEMATIC, BUT THERE MIGHT BE DIFFERENCES WHERE I THOUGHT SCHEMATIC NAMES WERE UNCLEAR */

    input               CLK,
    output              LED_D4, // Red
    output              LED_D12, // Yellow
    //output              LED_D13, // Green
    output wire         FPGA_LED,

    /* HIGH SPEED BUS TO/FROM CS FPGA30 */

    input wire [34:2]   HS_SOUTH_IN, // 32 = data Valid, 31:0 = Data
    output wire [35:35] HS_SOUTH_OUT,


    /* HIGH SPEED BUS TO/FROM CS FPGA20 */

    output [34:2]       HS_EAST_OUT, // 32 = DAC data Valid, 31:0 = DAC Data
    input wire [35:35]  HS_EAST_IN, // from cs 20
    output wire [36:36] HS_EAST_OUT_LAST,

    /* LOW SPEED BUS TO/FROM CS FPGA20 */
    output [46:46]      HS_EAST_OUT_RST,

    /*********** Ring bus ***************/
    input wire [47:47]  HS_SOUTH_IN_RB,
    output reg [47:47]  HS_EAST_OUT_RB,
    /************************************/

    /* MIB/GPMC (GPMC on Graviton Rev2. Schematic, MIB in practice) */
    /*output wire           MIB_TBIT,
     output wire           MIB_START,             // from master, starts a mib transaction
     output wire           MIB_RD_WR_N,           // 1 = read, 0 = write
     input  wire           MIB_SLAVE_ACK,*/
    //output wire [20:0]  MIB_AD,
    /*
     * 20 = DAC_CTRL_SDIO
     * 19 = DAC_CTRL_SDENN
     * 18 = DAC_CTRL_SCLK
     * 17 = DAC_CTRL_RESETN
     *
     * 16 = TX_3V3_A
     * 15 = TX_3V3_B
     *
     * 14:10 = DSA_CTRL_A
     * 9:6 = DSA_CTRL_B
     *
     * 5  = VGA_CTRL_A MOSI
     * 4  = VGA_CTRL_A SCLK
     * 3  = VGA_CTRL_A CS_N
     *
     * 2  = VGA_CTRL_B MOSI
     * 1  = VGA_CTRL_B SCLK
     * 0  = VGA_CTRL_B CS_N
     */

    input wire          MIB_MASTER_RESET,

    output wire         UART_TX,
    input wire          UART_RX,
    output wire         UART_INTERRUPT,

`ifdef VERILATE_DEF
    // data bound for cs20, in verilator only
    // normally these bytes are generated from inside eth_mega_wrapper
    input wire [31:0]   tx_turnstile_data_in,
    input wire          tx_turnstile_data_valid,
    output wire         tx_turnstile_data_ready,

    input wire [31:0]   ringbus_in_data,
    input wire          ringbus_in_data_vld,
    output wire         ringbus_in_data_ready,

    output wire [31:0]  ringbus_out_data,
    output wire         ringbus_out_data_vld,

    input wire          ring_bus_i0_ready,

    output wire [31:0]  snap_mapmov_in_data,
    output wire         snap_mapmov_in_valid,
    output wire         snap_mapmov_in_ready,

    output wire [31:0]  split_fb_data,
    output wire         split_fb_valid,
    input wire          split_fb_ready,
    
    output wire [31:0]  cs30_data_in,
    output wire         cs30_in_data_valid,
    input wire          cs30_data_in_ready,

`endif
`ifndef VERILATE_DEF
    /* ETHERNET PHY (RGMII) */

    output              ENET_CTRL_RESETN,
/*    inout wire          ENET_CTRL_CONFIG,
    output              ENET_CTRL_MDC,
    output wire         ENET_CTRL_MDIO,*/

    input               RGMII_RXCLK,
    input               RGMII_RXCTRL,
    input [3:0]         RGMII_RXD,

    output              RGMII_TXCLK,
    output              RGMII_TXCTRL,
    output [3:0]        RGMII_TXD
`else
    input               MAC_RX_WRITE,
    input               MAC_RX_EOF,
    input [7:0]         MAC_RX_FIFODATA,
    output              MAC_TX_FIFOAVAIL,
    output [7:0]        MAC_TX_FIFODATA,
    output              MAC_TX_FIFOEOF,
    input               MAC_TX_MACREAD
                        `endif


    /* DDR3 SDRAM */

    //    inout   [31:0]   DDR_D,
    //    output  [14:0]   DDR_A,
    //    inout   [ 3:0]   DDR_DQS,
    //    output  [ 2:0]   DDR_BA,
    //    output           DDR_CKE,
    //    output           DDR_CLK,
    //    output           DDR_CS_N,
    //    output  [ 3:0]   DDR_DQM,
    //    output           DDR_ODT,
    //    output           DDR_CAS_N,
    //    output           DDR_RAS_N,
    //    output           DDR_RST_N,
    //    output           DDR_WE_N,


    );


   //assign ENET_CTRL_RESETN = 1'b1;


// change these as flags
// `define PMI_DC_FIFO_OUTPUT_IN_VERILATOR
// `define HALF_CLOCK_IS_CLOCK

// this construct does:
//   hardware:
//     use pmi_fifo_dc_fwft_v1_0
//   v e r i l a t o r:
//     optionally use pmi_fifo_dc_fwft_v1_0 based on PMI_DC_FIFO_OUTPUT_IN_VERILATOR
`ifdef VERILATE_DEF
// v e r i l a t o r

// only use this if asked to
`ifdef PMI_DC_FIFO_OUTPUT_IN_VERILATOR
`define USE_PMI_DC_FIFO_OUTPUT
`endif

`else
// hardware

// always use this
`define USE_PMI_DC_FIFO_OUTPUT
`endif


   // SYSTEM SPECIFIC LOCAL PARAMETERS - DO NOT MODIFY
   localparam logic [23:0] FPGA_UID           = {8'h45, 8'h54, 8'h48}; // Software readable Unique FPGA ID stored in base register
   // End SYSTEM SPECIFIC LOCAL PARAMETERS

   logic                   sys_clk          /* synthesis syn_keep=1 */;
   logic                   sys_clk_srst     /* synthesis syn_keep=1 GSR=DISABLED */;
   logic                   eth_mac_rx_srst  /* synthesis syn_keep=1 GSR=DISABLED */;

   logic [1:0]             fpga_int_clk_srst_regs_0;
   logic [1:0]             fpga_int_clk_srst_regs_1;
   logic                   sys_pll_locked;


   reg [31:0]              temp_data;
   reg                     temp_valid;
   wire                    temp_ready;
   reg                     temp_ready_delay;
   reg                     out_buffer_read;


   /* ETHERNET SIGNALS */

`ifndef SIM_MODE
   logic                   eth_mac_rx_clk      /* synthesis syn_keep=1 */;
   logic                   eth_gbit_mode       /* synthesis syn_keep=1 */;
   logic                   eth_cfg_done        /* synthesis syn_keep=1 */;

   assign LED_D12 = sys_pll_locked & eth_cfg_done & eth_gbit_mode;
`else
   logic                   eth_cfg_done = 1;
   assign LED_D12       = sys_pll_locked;
`endif

   // logic                   led_gpio;
   // assign LED_D4 = led_gpio;
   //assign LED_D13 = sys_clk_srst;

   assign FPGA_LED =  gpio[21]; //led_gpio; //i_ringbus; //led_cnt[23];
   // always @(posedge sys_clk) 
   //    FPGA_LED <= HS_SOUTH_IN_RB;
   logic [30:0]            led_cnt;
   always @(posedge sys_clk)
      if(sys_clk_srst)
         led_cnt <= 0;
      else
         led_cnt <= led_cnt + 1;


   /* ADC UDP PACKETIZER SIGNALS */

   logic                   adc_pktzr_start;
   logic                   adc_pktzr_start_ack;
   logic                   adc_pktzr_done;
   logic                   adc_pktzr_data_byte_vld;
   logic [ 7:0]            adc_pktzr_data_byte;
   logic                   adc_pktzr_data_byte_rd;

   logic                   fpga_int_clk                   /* synthesis syn_keep=1 */; // fpga internal configuration oscillator
   logic                   fpga_int_clk_srst = 1          /* synthesis syn_keep=1 GSR=DISABLED */;


   // bring internal oscillator domain reset signal into the ethernet rx clock domain
   always_ff @(posedge eth_mac_rx_clk) begin
      fpga_int_clk_srst_regs_1 <= {fpga_int_clk_srst_regs_1[0], sys_clk_srst};
      eth_mac_rx_srst          <= fpga_int_clk_srst_regs_1[1];
   end

   /* COMMAND BUS SIGNALS */

   // see udp_cmd_pkg.sv (look at top of this file for where it's included)
   localparam int unsigned ETH_CMD_EXT_SEL_BITS     = FPGA_SEL_BITS; // THIS IS THE NUMBER OF UDP COMMAND ADDRESS BITS USED FOR SELECTING AN FPGA WHEN THE MIB MASTER MODULE IS TARGETED
   localparam int unsigned ETH_CMD_INT_SEL_BITS     = FPGA_SEL_BITS + MODULE_SEL_BITS; // THIS IS THE NUMBER OF UDP COMMAND ADDRESS BITS USED FOR SELECTING A MODULE WITHIN THIS FPGA WHEN AN INTERNAL REGISTER IS TARGETED (SOFTWARE SHOULD REALLY ONLY USE MODULE_SEL_BITS WORTH AND SKIP OVER FPGA_SEL_BITS TO MANTAIN CONSISTENCY)
   localparam int unsigned ETH_CMD_ACK_TIMEOUT_CLKS = 64;

   localparam bit [ETH_CMD_INT_SEL_BITS-1:0] CMD_INT_SLAVE_SEL      = 'h0;
   localparam bit [ETH_CMD_INT_SEL_BITS-1:0] CMD_COUNTERS_SEL       = 'h1;
   localparam bit [ETH_CMD_INT_SEL_BITS-1:0] CMD_UNDERFLOW_CNTR_SEL = 'h2;

   localparam int unsigned                   FPGA_INT_CLK_HZ = 25_800_000; // UPDATE THIS IF YOU CHANGE THE DIV() VALUE OF OSCG BELOW!
   // for ethernet udp commands, currently those commands go one of two places: internal command bus for internal registers, MIB bus for other FPGA registers
   intf_cmd #(.ADDR_BITS(MIB_ADDR_BITS), .DATA_BITS(CMD_DATA_BITS)) cmd_eth[2**MIB_SEL_BITS](); // parameters defined in udp_cmd_pkg

   localparam logic [3:0]                    SLAVE_MIB_ADDR_MSN   = 4'h3;

   intf_cmd #(CMD_ADDR_BITS, CMD_DATA_BITS) cmd_dummy(); // parameters specified in udp_cmd_pkg.sv

   localparam int                            NUM_SYS_CLK_RESETS                                   = 1;
   localparam int                            SYS_CLK_RESETS_EXTRA_CLOCKS [0:NUM_SYS_CLK_RESETS-1] = '{0};



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
                          .o_int_osc_clk       (fpga_int_clk),
                          .o_int_osc_clk_srst  (),
                          .o_sys_clk           (sys_clk),
                          .o_sys_clk_srsts     (sys_clk_srst),
                          .o_mib_clk           (),
                          .o_mib_clk_srsts     (),
                          .o_mib_deskew_done   (), // not used in FPGA with MIB Master
                          .o_sys_pll_locked    (sys_pll_locked),
                          .i_mib_tbit          (1'b0),
                          .i_mib_start         (1'b0),
                          .i_mib_rd_wr_n       (1'b0),
                          .b_mib_ad            (),
                          .o_mib_slave_ack     (),
                          .cmd_sys             (cmd_dummy)
                          );

   // bring internal oscillator domain reset signal into ethernet tx clock domain
   always_ff @(posedge fpga_int_clk) begin
      fpga_int_clk_srst_regs_0 <= {fpga_int_clk_srst_regs_0[0], sys_clk_srst};
      fpga_int_clk_srst <= fpga_int_clk_srst_regs_0[1];
   end

   /*
    * RESET CS FPGA20
    */

   logic [7:0] cs20_rst_cnt;
   logic       cs20_srst;

   always_ff @(posedge sys_clk) begin
      if (sys_clk_srst) begin
         cs20_rst_cnt <= 0;
         cs20_srst    <= 1;
      end else begin
         if (cs20_rst_cnt != 8'hff) begin
            cs20_rst_cnt <= cs20_rst_cnt + 1;
            cs20_srst <= 1;
         end else begin
            cs20_srst <= 0;
         end
      end
   end

   assign HS_EAST_OUT_RST = cs20_srst;




   /*
    *
    * ETHERNET
    *
    */

   // UDP_CS_CMD_TX_PORT IS SPECIFIED IN THE CS FPGA THAT HAS THE UDP CMD MODULE IN IT
   localparam int unsigned CBUF_MAX_ETH_FRAME_DEPTH  = 6; // Number of maximum sized Ethernet frames that can be simultaneously stored in the circular buffer
   localparam int unsigned IPV4_PKT_FIFO_PKT_DEPTH   = 4; // Number of maximum sized IPv4 packets that can be simultaneously stored in the output FIFO
   localparam int unsigned ARP_PKT_FIFO_PKT_DEPTH    = 2; // Number of maximum sized ARP packets that can be simultaneously stored in the output FIFO
   // localparam int unsigned UDP_ADC_ETH_FRAME_BUFFERS = 4;
   localparam int unsigned TX_ARB_ETH_FRAME_BUFFERS  = 2;  // Number of Max Size Ethernet II Frames that can be simultaneously buffered

   /*
    *
    * PROCESSED RX SAMPLES ETHERNET FRAME RECEPTION
    *
    */
   logic i0_ready;
   logic [31:0] eth_o_data;
   logic        eth_o_data_vld;
   logic        eth_o_data_ready;
   logic        i_ringbus;



   logic [32-1:0] fifo_sc_dout;
   logic          fifo_sc_rd_en;
   logic          fifo_sc_afull;
   logic          fifo_sc_full;
   logic          fifo_sc_dout_vld;

    logic [31:0] before_mapmov_data;
    logic        before_mapmov_valid;
    logic        before_mapmov_ready;

   logic          fifo_dc_adc_full;
   logic [31:0]   cs11_after_mega_wrapper_fillcount;
   logic        cs30_data_buf_afull;

   // bjm
   // this is where we can lose samples because afull/full is not connected
   // comes directly out from the eth_mega_wrapper
   // this buffer is for data bound for cs11
   // this outputs to a eb2a below
   // FIXME: add as outputs to eth self/check
   // FIXME: assuming that my changes so that ready works coming from cs20
   // we should be able to SHRINK this buffer, and increase buffer on cs20
   // if eth needs timing
   logic          cs11_after_mega_wrapper_afull;
   fwft_sc_fifo #(
                  .DEPTH        (1024*60), // number of locations in the fifo
                  .WIDTH        (32), // address width

                  .ALMOST_FULL  (1024*40) // number of locations for afull to be active
                  ) cs11_after_mega_wrapper (
                                             .clk          (sys_clk         ),
                                             .rst          (sys_clk_srst       ),
                                             .wren         (eth_o_data_vld        ),
                                             .wdata        (eth_o_data       ),
                                             .full      (),
                                             .o_afull      (cs11_after_mega_wrapper_afull ),
                                             .fillcount  (cs11_after_mega_wrapper_fillcount),
                                             .rden         (before_mapmov_ready        ),
                                             .rdata        (before_mapmov_data),
                                             .rdata_vld    (before_mapmov_valid));


    eb2a #(
         .T_0_WIDTH(32),
         .I_0_WIDTH(32)
      )eb2a_before_mapmov(
         .clk(sys_clk),
         .reset_n(!sys_clk_srst),
         .t0_data(before_mapmov_data),
         .t0_valid(before_mapmov_valid),
         .t0_ready(before_mapmov_ready),
         .i0_data(fifo_sc_dout),
         .i0_valid(fifo_sc_dout_vld),
         .i0_ready(fifo_sc_rd_en)
      );


   logic [31:0]   mapmov_data;
   logic          mapmov_last;
   logic          mapmov_valid;
   logic          mapmov_ready;
   logic [1023:0] mapmov_mover_active;
   logic [31:0]   mapmov_trim_start;
   logic [31:0]   mapmov_trim_end;
   logic [9:0]    mapmov_pilot_ram_addr;
   logic [31:0]   mapmov_pilot_ram_wdata;
   logic          mapmov_pilot_ram_we;
   logic          mapmov_reset;

`ifdef VERILATE_DEF
    assign snap_mapmov_in_data = fifo_sc_dout;
    assign snap_mapmov_in_valid = fifo_sc_dout_vld;
    assign snap_mapmov_in_ready = fifo_sc_rd_en;
`endif
   
   
   mapper_mover mapmov
     (.t_data(fifo_sc_dout),
      .t_valid(fifo_sc_dout_vld),
      .t_ready(fifo_sc_rd_en),
      .i_data(mapmov_data),
      .i_last(mapmov_last),
      .i_valid(mapmov_valid),
      .i_ready(mapmov_ready),
      .mover_active(mapmov_mover_active),
      .trim_start(mapmov_trim_start),
      .trim_end(mapmov_trim_end),
      .pilot_ram_addr(mapmov_pilot_ram_addr),
      .pilot_ram_wdata(mapmov_pilot_ram_wdata),
      .pilot_ram_we(mapmov_pilot_ram_we),
      .mapmov_reset(mapmov_reset),
      .clk(sys_clk),
      .rst(sys_clk_srst)
      );

   reg [31:0]     split_eq_data;
   reg            split_eq_valid, split_eq_last, split_eq_ready;

   reg [31:0]     fb_eq_join_data;
   reg            fb_eq_join_valid, fb_eq_join_last, fb_eq_join_ready;

   reg [31:0]     split_fb_data;
   reg            split_fb_valid, split_fb_last, split_fb_ready;

   reg [31:0]     rx_in_data;
   reg            rx_in_valid, rx_in_ready;
  

   // From a guess.  It takes about ~5 to ~10 us for cs11 to start after reading the first 16 of
   // feedback bus.  Adding another 1040 to read through the data, so this number is (1040*3)
   // this number is how many clocks to wait before dropping a message on the .t_eq_data() interface
   fb_eq_join #(.DROP_AFTER(256)) fb_eq_join_0 
     (.t_fb_data(mapmov_data),
      .t_fb_valid(mapmov_valid),
      .t_fb_last(mapmov_last),
      .t_fb_ready(mapmov_ready),
      //eq
      .t_eq_data(split_eq_data),
      .t_eq_valid(split_eq_valid),
      .t_eq_last(split_eq_last),
      .t_eq_ready(split_eq_ready),
      //initiator
      .i_data(fb_eq_join_data),
      .i_valid(fb_eq_join_valid),
      .i_last(fb_eq_join_last),
      .i_ready(fb_eq_join_ready),
      .clk(sys_clk),
      .rstf(~sys_clk_srst)
      );


   fb_eq_split fb_eq_split_0
     (.t_data(rx_in_data),
      .t_valid(rx_in_valid),
      .t_ready(rx_in_ready),
      //fb
      .i_fb_data(split_fb_data),
      .i_fb_valid(split_fb_valid),
      .i_fb_last(split_fb_last),
      .i_fb_ready(split_fb_ready),
      //eq
      .i_eq_data(split_eq_data),
      .i_eq_valid(split_eq_valid),
      .i_eq_last(split_eq_last),
      .i_eq_ready(split_eq_ready),
      .clk(sys_clk),
      .rstf(~sys_clk_srst)
      );

   /*
   assign split_fb_data = rx_in_data;
   assign split_fb_valid = rx_in_valid;
   assign rx_in_ready = split_fb_ready;

   assign split_eq_data = 0;
   assign split_eq_valid = 0;
   assign split_eq_last = 0;
   */
   

   // this eb2a sets up the delayed valid/ready signals
   // needed for off-chip destinations
   // eb2a #(.T_0_WIDTH (32),
   //      .I_0_WIDTH (32))
   //   eb1_5_inst_out (
   //      .clk                 (sys_clk),
   //      .reset_n             (!sys_clk_srst),

   //      .t0_data             (mapmov_data),
   //      .t0_valid               (mapmov_valid),
   //      .t0_ready               (mapmov_ready),

   //      .i0_data             (P1B_DDR_OUT[31:0]),
   //      .i0_valid               (P1B_DDR_OUT[32]),
   //      .i0_ready               (i0_ready) // ready coming from cs20
   //   );

    // localparam EXTRA_INPUT_CAPACITY = 0;
    localparam EXTRA_OUTPUT_CAPACITY = 16-8;

    localparam EXTRA_CAPACITY_UPPER_MARGIN = 8;
    localparam EXTRA_CAPACITY_LOWER_MARGIN = 2;





   fwft_sc_fifo #(
                  .DEPTH        (EXTRA_CAPACITY_UPPER_MARGIN+EXTRA_OUTPUT_CAPACITY), // number of locations in the fifo
                  .WIDTH        (32 + 1      ), // address width

                  .ALMOST_FULL  (EXTRA_CAPACITY_LOWER_MARGIN+EXTRA_OUTPUT_CAPACITY) // number of locations for afull to be active
                  ) out_buffer (
                                .clk          (sys_clk),
                                .rst          (sys_clk_srst),
                                .wren         (fb_eq_join_valid && fb_eq_join_ready),
                                .wdata        ({fb_eq_join_last, fb_eq_join_data}),
                                .full          (),
                                .o_afull_n      (fb_eq_join_ready),
                                .rden         (out_buffer_read),
                                .rdata        ({HS_EAST_OUT_LAST[36], HS_EAST_OUT[33:2]}),
                                .rdata_vld    (HS_EAST_OUT[34]));

   /*
    *  Monitor ethernet flags
    *
    */
/* verilator lint_off LITENDIAN */

    logic             eth_frame_circ_buf_overflow;    // FATAL
    logic             ipv4_pkt_fifo_overflow;         // FATAL
    logic             arp_pkt_fifo_overflow;          // FATAL
    logic             udp_pkt_fifo_overflow;          // FATAL
    logic             icmp_pkt_fifo_overflow;         // FATAL
    logic [0:3]       udp_port_fifo_overflow;         // FATAL

    // logic [7:0]       snap_cbuf_rd_fsm_state;
    // logic [7:0]       snap_cbuf_wr_fsm_state;

   logic [31:0]   riscv_status;
   logic [31:0]   riscv_control;

    // logic [0:3] rr_arb_grant_mask;
    // logic [0:3] rr_arb_grant;
    // logic [0:3] rr_arb_req_raw;
    // logic [0:3] rr_arb_req_masked;
/* verilator lint_on LITENDIAN */

    logic riscv_eth_reset_0;
    logic riscv_eth_reset_1;

`ifndef VERILATE_DEF


    // bring status [31] into eth_mac_rx_clk clock domain
    always @(posedge eth_mac_rx_clk) begin
        if( eth_mac_rx_srst ) begin
            riscv_eth_reset_0 <= 1'b0;
            riscv_eth_reset_1 <= 1'b0;
        end else begin
            riscv_eth_reset_0 <= riscv_control[31];
            riscv_eth_reset_1 <= riscv_eth_reset_0;
        end
    end



   always @(posedge sys_clk) begin
        if(riscv_control == 32'h00000000) begin
            riscv_status <= 32'hdeadbeef;
        end else if(riscv_control == 32'h00000001) begin
            riscv_status <= mac_rx_err_cntr;
        end else if(riscv_control == 32'h00000002) begin
            riscv_status <= mac_rx_crc_err_cntr;
        end else if(riscv_control == 32'h00000003) begin
            riscv_status <= mac_rx_len_chk_err_cntr;
        end else if(riscv_control == 32'h00000004) begin
            riscv_status <= mac_rx_pkt_rx_ok_cntr;
        end else if(riscv_control == 32'h00000005) begin
            riscv_status <= unsupported_eth_type_error_cntr;
        end else if(riscv_control == 32'h00000006) begin
            riscv_status <= unsupported_ipv4_protocol_cntr;
        end else if(riscv_control == 32'h00000007) begin
            riscv_status <= unsupported_dest_port_cntr;
        end else if(riscv_control == 32'h00000008) begin
            riscv_status <= cs20_data_buf_overflow_cntr;
        end else if(riscv_control == 32'h0000000b) begin
            riscv_status <= rb_in_buf_overflow_cntr;
        end else if(riscv_control == 32'h0000000c) begin
           riscv_status <= cs30_data_buf_overflow_cntr;
        end else if(riscv_control == 32'h0000000d) begin
           riscv_status <= rb_out_buf_overflow_cntr;

        end else if(riscv_control == 32'h0000000e) begin
           riscv_status <= eth_frame_circ_buf_overflow;
        end else if(riscv_control == 32'h0000000f) begin
           riscv_status <= ipv4_pkt_fifo_overflow;
        end else if(riscv_control == 32'h00000010) begin
           riscv_status <= arp_pkt_fifo_overflow;
        end else if(riscv_control == 32'h00000011) begin
           riscv_status <= udp_pkt_fifo_overflow;
        end else if(riscv_control == 32'h00000012) begin
           riscv_status <= icmp_pkt_fifo_overflow;
        end else if(riscv_control == 32'h00000013) begin
           riscv_status <= udp_port_fifo_overflow[0];
        end else if(riscv_control == 32'h00000014) begin
           riscv_status <= udp_port_fifo_overflow[1];
        end else if(riscv_control == 32'h00000015) begin
           riscv_status <= udp_port_fifo_overflow[2];
        end else if(riscv_control == 32'h00000016) begin
           riscv_status <= udp_port_fifo_overflow[3];

        // end else if(riscv_control == 32'h00000017) begin
        //    riscv_status <= snap_cbuf_rd_fsm_state;
        // end else if(riscv_control == 32'h00000018) begin
        //    riscv_status <= snap_cbuf_wr_fsm_state;
        // end else if(riscv_control == 32'h00000019) begin
        //    riscv_status <= rr_arb_grant_mask;
        // end else if(riscv_control == 32'h0000001a) begin
        //    riscv_status <= rr_arb_grant;
        // end else if(riscv_control == 32'h0000001b) begin
        //    riscv_status <= rr_arb_req_raw;
        // end else if(riscv_control == 32'h0000001c) begin
        //    riscv_status <= rr_arb_req_masked;

        end else if(riscv_control == 32'h00001000) begin
           riscv_status <= { {31{1'b0}}, cs11_after_mega_wrapper_afull};
        end else if(riscv_control == 32'h00001001) begin
           riscv_status <= cs11_after_mega_wrapper_fillcount;
        end else begin
           riscv_status <= 32'h0;
        end
   end
`else
`ifdef EXCLUDE_MEGAWRAPPER
    always @(posedge sys_clk) begin
        if(riscv_control == 32'h00000000) begin
            riscv_status <= 32'hdeadbeef;
        end else begin
            riscv_status <= 32'h0;
        end
    end
`else

    always @(posedge sys_clk) begin
        if(riscv_control == 32'h00000000) begin
            riscv_status <= 32'hdeadbeef;
        end else if(riscv_control == 32'h00000001) begin
            riscv_status <= mac_rx_err_cntr;
        end else if(riscv_control == 32'h00000002) begin
            riscv_status <= mac_rx_crc_err_cntr;
        end else if(riscv_control == 32'h00000003) begin
            riscv_status <= mac_rx_len_chk_err_cntr;
        end else if(riscv_control == 32'h00000004) begin
            riscv_status <= mac_rx_pkt_rx_ok_cntr;
        end else if(riscv_control == 32'h00000005) begin
            riscv_status <= unsupported_eth_type_error_cntr;
        end else if(riscv_control == 32'h00000006) begin
            riscv_status <= unsupported_ipv4_protocol_cntr;
        end else if(riscv_control == 32'h00000007) begin
            riscv_status <= unsupported_dest_port_cntr;
        end else if(riscv_control == 32'h00000008) begin
            riscv_status <= cs20_data_buf_overflow_cntr;
        end else if(riscv_control == 32'h0000000b) begin
            riscv_status <= rb_in_buf_overflow_cntr;
        end else if(riscv_control == 32'h0000000c) begin
           riscv_status <= cs30_data_buf_overflow_cntr;
        end else if(riscv_control == 32'h0000000d) begin
           riscv_status <= rb_out_buf_overflow_cntr;

        end else if(riscv_control == 32'h0000000e) begin
           riscv_status <= eth_frame_circ_buf_overflow;
        end else if(riscv_control == 32'h0000000f) begin
           riscv_status <= ipv4_pkt_fifo_overflow;
        end else if(riscv_control == 32'h00000010) begin
           riscv_status <= arp_pkt_fifo_overflow;
        end else if(riscv_control == 32'h00000011) begin
           riscv_status <= udp_pkt_fifo_overflow;
        end else if(riscv_control == 32'h00000012) begin
           riscv_status <= icmp_pkt_fifo_overflow;
        end else if(riscv_control == 32'h00000013) begin
           riscv_status <= udp_port_fifo_overflow[0];
        end else if(riscv_control == 32'h00000014) begin
           riscv_status <= udp_port_fifo_overflow[1];
        end else if(riscv_control == 32'h00000015) begin
           riscv_status <= udp_port_fifo_overflow[2];
        end else if(riscv_control == 32'h00000016) begin
           riscv_status <= udp_port_fifo_overflow[3];

        // end else if(riscv_control == 32'h00000017) begin
        //    riscv_status <= snap_cbuf_rd_fsm_state;
        // end else if(riscv_control == 32'h00000018) begin
        //    riscv_status <= snap_cbuf_wr_fsm_state;

        end else if(riscv_control == 32'h00001000) begin
           riscv_status <= { {31{1'b0}}, cs11_after_mega_wrapper_afull};
        end else if(riscv_control == 32'h00001001) begin
           riscv_status <= cs11_after_mega_wrapper_fillcount;
        end else begin
           riscv_status <= 32'h0;
        end
   end
`endif
`endif

   /*
    *
    * RISC-V processor
    *
    */

   logic [31:0] riscv_in_data;
   logic        riscv_in_valid;
   logic        riscv_in_ready; // output from t0_ready


   logic [31:0] riscv_out_data;
   logic        riscv_out_valid;
`ifndef VERILATE_DEF
    logic       fifo_dc_rb_full;
`endif
   logic        o_ringbus;

   logic        riscv_out_ready;

   wire [21:0]  gpio;
   
   // this change adds fix for when you are sending large data udp packets
   // at the same time as sending ring packets.  On rx s-modem you see duplicate
   // repeated ringbus messages.
   //
`ifndef VERILATE_DEF
   assign riscv_out_ready = !fifo_dc_rb_full && !adc_pktzr_start;
`endif

`define EXTRA_RINGBUS

   logic        ringbus_inbetween;

   q_engine #(
`ifdef ETH_NO_RISCV
              .NO_RISCV(1),
`endif
`ifndef VERILATE_DEF
              .VMEM_DEPTH (VMEM_DEPTH),
`else
              .VMEM_DEPTH (4096),
`endif
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
   q_engine_inst (
                  .clk                       (sys_clk),
                  .srst                      (sys_clk_srst),
                  .debugReset                   (sys_clk_srst),

                  .t0_data                   (riscv_in_data),
                  .t0_valid                     (riscv_in_valid),
                  .t0_ready                     (riscv_in_ready),

                  .i0_data                   (riscv_out_data),
                  .i0_valid                     (riscv_out_valid),
            `ifndef EXCLUDE_MEGAWRAPPER
         .i0_ready                     (riscv_out_ready), // hardware
            `else
            .i0_ready                           (ring_bus_i0_ready), // simulator
            `endif

                  .proc_interrupt                  (),

                  .gpio                      ({gpio[21:1], UART_INTERRUPT}), // gpio[21] used for led
                  //uart
                  .io_uart_txd                (UART_TX),
                  .io_uart_rxd                (UART_RX),

                  .status(riscv_status),
                  .control(riscv_control),

                  .mapmov_mover_active(mapmov_mover_active),
                  .mapmov_trim_start(mapmov_trim_start),
                  .mapmov_trim_end(mapmov_trim_end),
                  .mapmov_pilot_ram_addr(mapmov_pilot_ram_addr),
                  .mapmov_pilot_ram_wdata(mapmov_pilot_ram_wdata),
                  .mapmov_pilot_ram_we(mapmov_pilot_ram_we),
                  .mapmov_reset(mapmov_reset),

                  .i_ringbus_0                     (i_ringbus),
                  .o_ringbus_0                     (ringbus_inbetween),

                  .i_ringbus_1                            (ringbus_inbetween),
                  .o_ringbus_1                            (o_ringbus)
                  );

   // assign LED_D12 = led_driver;
   // sys_pll_locked;

   always_ff @(posedge sys_clk) begin
      HS_EAST_OUT_RB[47] <= o_ringbus;
   end


   /*
    *
    * Data coming from CS30
    *
    */
   logic [32:0] temp;


   // genvar       i;
   // generate if (!VERILATE) begin
   //    for (i = 0; i < 33; i = i + 1) begin

   //       /* synthesis IO_TYPE="SSTL15_I" */;

   //       defparam input_delay.DEL_VALUE = 96 ;
   //       defparam input_delay.DEL_MODE = "USER_DEFINED" ;
   //       DELAYG input_delay (.A(HS_SOUTH_IN[i+2]), .Z(temp[i]));
   //    end

   //    defparam input_delay_inst.DEL_VALUE = 96 ;
   //    defparam input_delay_inst.DEL_MODE = "USER_DEFINED" ;
   //    DELAYG input_delay_inst (.A(HS_EAST_IN[33]), .Z(i0_ready));

   //    defparam input_delay_rb.DEL_VALUE = 96 ;
   //    defparam input_delay_rb.DEL_MODE = "USER_DEFINED" ;
   //    DELAYG input_delay_rb (.A(HS_SOUTH_IN_RB[47]), .Z(i_ringbus));

   // end else begin
      assign temp[32:0] = HS_SOUTH_IN[34:2];
      assign i0_ready = HS_EAST_IN[35]; // ready coming from cs20
      assign i_ringbus = HS_SOUTH_IN_RB[47];
   // end
   // endgenerate
`ifndef VERILATE_DEF
   logic [31:0] cs30_data_in;
   logic        cs30_in_data_valid;
    logic        cs30_data_in_ready;
`endif


   ////////////// This fifo / always@ was copied from vex_machine_top


   // This delay buffer accounts for the eth->cs30 connection lacking
   // a buffer delay on eth's side. (cs30 always has this buffer because
   // of vex_machine_top)
   always @(posedge sys_clk) begin
      temp_data <= temp[31:0];          // cs30
      temp_valid <= temp[32];           // cs30
      temp_ready_delay <= temp_ready;   // cs30
//`ifndef USE_PMI_DC_FIFO_OUTPUT
      out_buffer_read <= i0_ready;      // cs20
//`endif
   end


   fwft_sc_fifo #(
                  .DEPTH        (6), // number of locations in the fifo
                  .WIDTH        (32       ), // address width

                  .ALMOST_FULL  (2) // number of locations for afull to be active
                  ) cs20_in_buffer (
                                    .clk          (sys_clk         ),
                                    .rst          (sys_clk_srst    ),
                                    .wren         (temp_valid && temp_ready_delay),
                                    .wdata        (temp_data       ),
                                    .o_afull_n    (HS_SOUTH_OUT[35]),
                                    .o_afull_n_d  (temp_ready),
                                    .rden         (rx_in_ready),
                                    .rdata        (rx_in_data),
                                    .rdata_vld    (rx_in_valid));

   ////////////// end


   logic vd_buffer_rd;
   logic rb_buffer_rd;

   // These are the fix for the repeated ringbus message crash
   assign vd_buffer_rd = !sys_clk_srst;
   assign rb_buffer_rd = !sys_clk_srst;

`ifdef EXCLUDE_MEGAWRAPPER
   // data bound for cs20
   // since we are in verilator, we skip eth_mega_wrapper
   // and bytes are generated from tb.  ready pushing back
   // against tb is cs20->offchip->eth->eb2a->fwft_sc_fifo->tb
   assign eth_o_data = tx_turnstile_data_in;
   assign eth_o_data_vld = tx_turnstile_data_valid;
   assign tx_turnstile_data_ready = vd_buffer_rd;

   assign riscv_in_data = ringbus_in_data;
   assign riscv_in_valid = ringbus_in_data_vld;
   assign ringbus_in_data_ready = riscv_in_ready;
//rb_buffer_rd;

   assign ringbus_out_data = riscv_out_data;
   assign ringbus_out_data_vld = riscv_out_valid;

`else

   /* Custom_reveal UDP PACKETIZER SIGNALS */

   logic rb_pktzr_start;
   logic rb_pktzr_start_ack;
   logic rb_pktzr_done;
   logic rb_pktzr_data_byte_vld;
   logic [ 7:0] rb_pktzr_data_byte;
   logic        rb_pktzr_data_byte_rd;

`ifndef VERILATE_DEF

   // stores processed custom_reveal samples for the UDP packetizer to pack into UDP packets to be sent to the host
   pmi_fifo_dc_fwft_v1_0 #(
                           .WR_DEPTH        (64),
                           .WR_DEPTH_AFULL  (UDP_RINGBUS_PACKET_SIZE_WORDS),
                           .WR_WIDTH        (32),
                           .RD_WIDTH        (8),
                           .FAMILY          ("ECP5U"),
                           .IMPLEMENTATION  ("EBR"),
                           .RESET_MODE      ("sync"),
                           .WORD_SWAP       (0),
 `ifndef SIM_MODE
                           .SIM_MODE        (0)
 `else
                           .SIM_MODE        (1)
 `endif
                           ) fifo_dc_rb (
                                         .wrclk           (sys_clk),
                                         .wrclk_rst       (sys_clk_srst),
                                         .rdclk           (sys_clk),
                                         .rdclk_rst       (sys_clk_srst),
                                         .wdata           (riscv_out_data),
                                         .wren            (riscv_out_valid & riscv_out_ready),
                                         .full            (fifo_dc_rb_full),
                                         .afull           (rb_pktzr_start),
                                         .rden            (rb_pktzr_data_byte_rd),
                                         .rdata           (rb_pktzr_data_byte),
                                         .rdata_vld       (rb_pktzr_data_byte_vld));
   
   assign split_fb_ready = ~fifo_dc_adc_full;
   
   // stores processed samples from cs30 for the UDP packetizer to pack into UDP packets to be sent to the host
   pmi_fifo_dc_fwft_v1_0 #(
                           .WR_DEPTH        (1024),
                           .WR_DEPTH_AFULL  (UDP_CS30_PACKET_SIZE_WORDS), // 1 udp payload is 1472 bytes = 368 32-bit dwords, but we use 2 dwords for sequence number and flags
                           .WR_WIDTH        (32),
                           .RD_WIDTH        (8),
                           .FAMILY          ("ECP5U"),
                           .IMPLEMENTATION  ("EBR"),
                           .RESET_MODE      ("sync"),
                           .WORD_SWAP       (0),
 `ifndef SIM_MODE
                           .SIM_MODE        (0)
 `else
                           .SIM_MODE        (1)
 `endif
                           ) fifo_dc_adc (
                                          .wrclk           (sys_clk),
                                          .wrclk_rst       (sys_clk_srst),
                                          .rdclk           (sys_clk),
                                          .rdclk_rst       (sys_clk_srst),
                                          .wren            (split_fb_valid),
                                          .wdata           (split_fb_data),
                                          .full            (fifo_dc_adc_full),
                                          .afull           (adc_pktzr_start),
                                          .rden            (adc_pktzr_data_byte_rd),
                                          .rdata           (adc_pktzr_data_byte),
                                          .rdata_vld       (adc_pktzr_data_byte_vld));
`else

  // megawrapper mode under verilator

        logic [31:0] fifo_dc_rb_fill;
        width_32_8 #(
            .CAPACITY    (64)
            ) fifo_dc_rb (
                .clk         (sys_clk),
                .reset       (sys_clk_srst),

                .t0_data     (riscv_out_data),
                .t0_valid    (riscv_out_valid),
                .t0_ready    (riscv_out_ready),

                .i0_data     (rb_pktzr_data_byte),
                .i0_valid    (rb_pktzr_data_byte_vld),
                .i0_ready    (rb_pktzr_data_byte_rd),

                .fillcount   (fifo_dc_rb_fill)
            );
        assign rb_pktzr_start = fifo_dc_rb_fill > ((UDP_RINGBUS_PACKET_SIZE_WORDS)-1);

        logic [31:0] fifo_cs30_rb_fill;
        width_32_8 #(
            .CAPACITY        (1024)
            ) fifo_dc_cs30 (
                .clk         (sys_clk),
                .reset       (sys_clk_srst),

                .t0_data     (split_fb_data),
                .t0_valid    (split_fb_valid),
                .t0_ready    (), // FIXME

                .i0_data     (adc_pktzr_data_byte),
                .i0_valid    (adc_pktzr_data_byte_vld),
                .i0_ready    (adc_pktzr_data_byte_rd),

                .fillcount   (fifo_cs30_rb_fill)
            );
        assign adc_pktzr_start = fifo_cs30_rb_fill > ((UDP_CS30_PACKET_SIZE_WORDS)-1);

`endif

   /*
    *
    * ETHERNET FRAME RX AND TX
    *
    */

   logic [47:0] host_mac;
   logic [31:0] mac_rx_err_cntr;
   logic [31:0] mac_rx_crc_err_cntr;
   logic [31:0] mac_rx_len_chk_err_cntr;
   logic [31:0] mac_rx_pkt_rx_ok_cntr;
   logic [31:0] unsupported_eth_type_error_cntr;
   logic [31:0] unsupported_ipv4_protocol_cntr;
   logic [31:0] unsupported_dest_port_cntr;
   logic [31:0] cs20_data_buf_overflow_cntr;
   logic [31:0] rb_in_buf_overflow_cntr;
   logic [31:0] cs30_data_buf_overflow_cntr;
   logic [31:0] rb_out_buf_overflow_cntr;

   // assign RGMII_TXCTRL = 1'b1;
   // assign RGMII_TXD = 4'b1111;
   //assign ENET_CTRL_RESETN = ~sys_clk_srst;

   eth_mega_wrapper #(
                      .LOCAL_MAC_ADDR             (LOCAL_MAC_ADDR            ), // see higgs_sdr_global_pkg.sv
                      .CFG_CLK_HZ                 (FPGA_INT_CLK_HZ           ),
                      .CBUF_MAX_ETH_FRAME_DEPTH   (CBUF_MAX_ETH_FRAME_DEPTH  ),
                      .IPV4_PKT_FIFO_PKT_DEPTH    (IPV4_PKT_FIFO_PKT_DEPTH   ),
                      .ARP_PKT_FIFO_PKT_DEPTH     (ARP_PKT_FIFO_PKT_DEPTH    ),
                      .UDP_CMD_ACK_TIMEOUT_CLKS   (UDP_CMD_ACK_TIMEOUT_CLKS  ), // see udp_cmd_pkg.sv
                      .TX_ARB_ETH_FRAME_BUFFERS   (TX_ARB_ETH_FRAME_BUFFERS  ),
                      .HOST_IP_ADDR               (HOST_IP_ADDR              ), // see higgs_sdr_global_pkg.sv
                      .LOCAL_IP_ADDR              (LOCAL_IP_ADDR             ), // see higgs_sdr_global_pkg.sv
 `ifndef SIM_MODE
                      .SIM_MODE                   (0                         )
 `else
                      .SIM_MODE                   (1                         )
 `endif
                      ) ETH_MEGA_WRAPPER (
 `ifdef VERILATE_DEF
                                          .i_mac_rx_write              (MAC_RX_WRITE),
                                          .i_mac_rx_eof                (MAC_RX_EOF),
                                          .i_mac_rx_fifodata           (MAC_RX_FIFODATA),
                                          .o_mac_tx_fifodata           (MAC_TX_FIFODATA),
                                          .o_mac_tx_fifoavail          (MAC_TX_FIFOAVAIL),
                                          .o_mac_tx_fifoeof            (MAC_TX_FIFOEOF),
                                          .i_mac_tx_macread            (MAC_TX_MACREAD),
 `endif
                                          .i_cfg_clk                    (fpga_int_clk),
                                          .i_cfg_srst                   (fpga_int_clk_srst),
                                          .o_gbit_mode                  (eth_gbit_mode),
                                          .o_cfg_done                   (eth_cfg_done),
                                          .o_rx_clk125                  (eth_mac_rx_clk),
                                          .i_rx_clk125_srst             (eth_mac_rx_srst | riscv_eth_reset_1),
                                          .i_tx_clk125                  (sys_clk),
                                          .i_tx_clk125_srst             (sys_clk_srst),
 `ifndef VERILATE_DEF
                                          .o_phy_rst_n                  (ENET_CTRL_RESETN),
                                          .i_rgmii_rxclk                (RGMII_RXCLK),
                                          .i_rgmii_rxctrl               (RGMII_RXCTRL),
                                          .i_rgmii_rxd                  (RGMII_RXD),
                                          .o_rgmii_txclk                (RGMII_TXCLK),
                                          .o_rgmii_txctrl               (RGMII_TXCTRL),
                                          .o_rgmii_txd                  (RGMII_TXD),
 `endif
                                          .i_dac_buffer_rd              (1'b0),
                                          .o_dac_buffer_data_vld        (           ),
                                          .o_dac_buffer_data_parity     (           ),
                                          .o_dac_buffer_data            (          ),
                                          .i_cmd_clk                    (sys_clk                     ),
                                          .i_cmd_srst                   (sys_clk_srst                ),
                                          .cmd                          (cmd_eth                    ),

                                          .i_vc_pktzr_start           (adc_pktzr_start          ),
                                          .o_vc_pktzr_start_ack       (adc_pktzr_start_ack      ),
                                          .o_vc_pktzr_done            (adc_pktzr_done           ),
                                          .i_vc_pktzr_data_byte_vld   (adc_pktzr_data_byte_vld  ),
                                          .i_vc_pktzr_data_byte       (adc_pktzr_data_byte      ),
                                          .o_vc_pktzr_data_byte_rd    (adc_pktzr_data_byte_rd   ),


                                          .i_vd_buffer_rd              (vd_buffer_rd),
                                          //        .i_vd_buffer_rd                (eth_o_data_ready),
                                          .o_vd_buffer_data_vld        (eth_o_data_vld),
                                          .o_vd_buffer_data            (eth_o_data),

                                          .i_rb_buffer_rd              (rb_buffer_rd),
                                          .o_rb_buffer_data_vld        (riscv_in_valid),
                                          .o_rb_buffer_data            (riscv_in_data),

                                          .i_rb_pktzr_start           (rb_pktzr_start          ),
                                          .o_rb_pktzr_start_ack       (rb_pktzr_start_ack      ),
                                          .o_rb_pktzr_done            (rb_pktzr_done           ),
                                          .i_rb_pktzr_data_byte_vld   (rb_pktzr_data_byte_vld  ),
                                          .i_rb_pktzr_data_byte       (rb_pktzr_data_byte      ),
                                          .o_rb_pktzr_data_byte_rd    (rb_pktzr_data_byte_rd   ),

                                          // status and error, synchronous to cmd clk
                                          .o_host_mac                        (host_mac                        ), // software needs to update UDP packetizer modules in other FPGAs (e.g. on Copper Suicide) with this once learned so that they can correctly make Ethernet Frames
                                          .o_mac_rx_err_cntr                 (mac_rx_err_cntr                 ),
                                          .o_mac_rx_crc_err_cntr             (mac_rx_crc_err_cntr             ),
                                          .o_mac_rx_len_chk_err_cntr         (mac_rx_len_chk_err_cntr         ),
                                          .o_mac_rx_pkt_rx_ok_cntr           (mac_rx_pkt_rx_ok_cntr           ),
                                          .o_unsupported_eth_type_error_cntr (unsupported_eth_type_error_cntr ),
                                          .o_unsupported_ipv4_protocol_cntr  (unsupported_ipv4_protocol_cntr  ),
                                          .o_unsupported_dest_port_cntr      (unsupported_dest_port_cntr      ),

                                          .o_cs20_data_buf_overflow_cntr  (cs20_data_buf_overflow_cntr),
                                          .o_rb_in_buf_overflow_cntr  (rb_in_buf_overflow_cntr),
                                          .o_cs30_data_buf_afull          (cs30_data_buf_afull),
                                          .o_cs30_data_buf_overflow_cntr  (cs30_data_buf_overflow_cntr),
                                          .o_rb_out_buf_overflow_cntr  (rb_out_buf_overflow_cntr),

                                          // if any of these errors occur you'll need to reset this module to recover and clear the error
                                          .eth_frame_circ_buf_overflow    (eth_frame_circ_buf_overflow),
                                          .ipv4_pkt_fifo_overflow    (ipv4_pkt_fifo_overflow),
                                          .arp_pkt_fifo_overflow    (arp_pkt_fifo_overflow),
                                          .udp_pkt_fifo_overflow    (udp_pkt_fifo_overflow),
                                          .icmp_pkt_fifo_overflow    (icmp_pkt_fifo_overflow),
                                          .udp_port_fifo_overflow    (udp_port_fifo_overflow)
                                          // .snap_cbuf_rd_fsm_state    (snap_cbuf_rd_fsm_state),
                                          // .snap_cbuf_wr_fsm_state    (snap_cbuf_wr_fsm_state)

                                          // .rr_arb_grant_mask          (rr_arb_grant_mask),
                                          // .rr_arb_grant               (rr_arb_grant),
                                          // .rr_arb_req_raw             (rr_arb_req_raw),
                                          // .rr_arb_req_masked          (rr_arb_req_masked)
                                          );


`endif



endmodule

`default_nettype wire
