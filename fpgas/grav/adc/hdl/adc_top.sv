/*
 * Module: adc_top
 *
 */

`include "udp_cmd_pkg.sv" // located in ip-library/lattice_support/gbit_mac/packages

`default_nettype none

module adc_top #(
        parameter VERILATE = 1'b0)
    (

    /* NAMES CHOSEN TO CLOSELY MATCH GRAVITON SCHEMATIC, BUT THERE MIGHT BE DIFFERENCES WHERE I THOUGHT SCHEMATIC NAMES WERE UNCLEAR */

    input  wire logic        FPGA1_CLK,  // system clock

    /* LED Outputs */
			
    // output wire 	logic LED_D5, // Red
    // output wire 	logic LED_D16, // Green
    // output wire 	logic LED_D17, // Yellow
			
    /* HIGH SPEED BUS TO CS FPGA00 */
    output wire [38:38] HS_EAST_OUT_SAT,
//    input wire [38:38] 	P2A_DDR_in, 
    input wire [40:39] 	HS_EAST_IN_GPIO, // 35 = gpio[20] high selects counter, 0 selects the adc data

    input wire [35:35] HS_EAST_IN, //ready
    output reg [34:2] 	HS_EAST_OUT, // 34 = ADC chan a sample 0,1 valid, 33:2 = ADC chan a sample 0,1
	output reg [36:36] HS_EAST_OUT_LAST,
    /* LOW SPEED BUS TO CS FPGA00 */
//    output wire logic [21:0] P2A_SDR,

    /* ADC DATA INTERFACE */
    input  wire logic        ADC_DCLK,
    input  wire logic [15:0] ADC_D,

`ifdef VERILATE_DEF
    input wire [31:0]   i_data_adc,
    input wire          i_data_valid_adc,
`endif

    /* MIB (A.K.A. GPMC) */
//    input  wire              MIB_TBIT, // Training bit toggle pattern from MIB master
//    input  wire              MIB_START,
//    input  wire              MIB_RD_WR_N,
//    output wire              MIB_SLAVE_ACK,
    // input  wire    [16:0]    MIB_A// D,

    /* CFG FPGA MASTER RESET AND LOCK STROBE (USES MIB/GPMC BUS SPARE SIGNAL LINES, BUT OPERATES IN THE SYS CLOCK DOMAIN) */

    input                    MIB_MASTER_RESET // GPMC SIGNAL LINE 22, USED AS A MASTER RESET TO THE OTHER GRAVITON FPGAS AND THE CFG FPGA ON CS
//    input                    MIB_COUNTER_LOCK, // GPMC SIGNAL LINE 21, USED AS A WAY FOR SOFTWARE TO LOCK VARIOUS COUNTERS IN DIFFERENT FPGAS PRIOR TO READING THEM TO GET THEIR DELTAS (YOU SHOULD TRIPLE FLOP THIS AND DO RISING EDGE DETECTION ON IT)

    /* DIGITAL STEP ATTENUATOR */
    // output  reg [5:0]       DSA_CTRL_A,
    // output  reg [5:0]       DSA_CTRL_B,

    /* VARIABLE GAIN ATTENUATOR */
    // output  reg [2:0]       VGA_CTRL_A, //0 - CS_N, 1 - SCLK, 2 - MOSI
    // output  reg [2:0]       VGA_CTRL_B  //0 - CS_N, 1 - SCLK, 2 - MOSI

);

    assign HS_EAST_OUT_LAST = 1'b0;
    // SYSTEM SPECIFIC LOCAL PARAMETERS - DO NOT MODIFY
    localparam logic [3:0] SLAVE_MIB_ADDR_MSN   = 4'd2; // Unique MIB Address Most Significant Nibble
    localparam logic [23:0] FPGA_UID            = {8'h41, 8'h44, 8'h43}; // Software readable Unique FPGA ID stored in base register (ASCII "ADC")
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

    /*always_ff @(posedge sys_clk) begin
        if (sys_clk_srst) begin
            DSA_CTRL_A <= 'b0;
            DSA_CTRL_B <= 'b0;
            VGA_CTRL_A <= 3'b001;
            VGA_CTRL_B <= 3'b001;
        end else begin
            DSA_CTRL_A[5:1] <= {MIB_AD[14:10]};//, MIB_AD[16]};
            DSA_CTRL_B[5:2] <= {MIB_AD[9:6]};//, 1'b0, MIB_AD[15]};
            VGA_CTRL_A <= MIB_AD[5:3];
            VGA_CTRL_B <= MIB_AD[2:0];
        end
    end*/
    intf_cmd #(CMD_ADDR_BITS, CMD_DATA_BITS) cmd_sys(); // parameters specified in udp_cmd_pkg.sv

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
        .i_fpga_clk          (FPGA1_CLK),
        .i_fpga_ext_arst     (MIB_MASTER_RESET), // LS_WEST_IN[0] allows the ETH FPGA on Graviton to reset this FPGA.  This is only really needed when doing JTAG programming because the CFG FPGA on Graviton doesn't know when the ETH FPGA actually gets programmed in that case.
        .o_int_osc_clk       (int_osc_clk),
        .o_int_osc_clk_srst  (int_osc_clk_srst),
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

    //assign LED_D16 = sys_pll_locked;
    //assign LED_D5  = 0;
    //assign LED_D17 = sys_clk_srst;

/********************************************************************************/

    /*
     * NOTE: THIS IS CURRENTLY JUST A TEST TO SEE IF THIS WORKS AS A GOOD WAY OF SIMULTANEOUSLY RESETTING ALL FPGAS ON BOTH GRAVITON AND CS
     *
     * After issuing the mib master reset via software, issue the software counter lock, and then read this counter and the counters in DAC and ETH FPGAs and compare their count values.
     */

    logic [31:0] mib_rst_test_cntr;
    logic [31:0] mib_rst_test_cntr_lock_reg;
    logic [2:0]  mib_counter_lock_sync_ffs;
    logic        mib_counter_lock_redge;

    always_ff @(posedge sys_clk) begin
        if (sys_clk_srst) begin
            mib_rst_test_cntr <= '0;
        end else begin
            mib_rst_test_cntr <= mib_rst_test_cntr + 1;
        end
    end

//    always_ff @(posedge sys_clk) begin
//        if (sys_clk_srst) begin
//            mib_counter_lock_sync_ffs <= '0;
//        end else begin
//            mib_counter_lock_sync_ffs <= {mib_counter_lock_sync_ffs[1:0], MIB_COUNTER_LOCK};
//        end
//    end

    assign  mib_counter_lock_redge = mib_counter_lock_sync_ffs[1] & ~mib_counter_lock_sync_ffs[2];

    always_ff @(posedge sys_clk) begin
        if (mib_counter_lock_redge) begin // only lock on rising edge
            mib_rst_test_cntr_lock_reg <= mib_rst_test_cntr;
        end
    end

/********************************************************************************/


    /*
     *
     * ADC DATA CAPTURE AND FORWARDING TO CS FPGA00
     *
     */

    logic        adc_sclk        /* synthesis syn_keep=1 */;
    logic [15:0] chan_a_s0;
    logic [15:0] chan_a_s1;
    logic [15:0] chan_b_s0;
    logic [15:0] chan_b_s1;

    logic [15:0] chan_a_s0_reg   /* synthesis syn_noprune=1 */;
    logic [15:0] chan_a_s1_reg   /* synthesis syn_noprune=1 */;
    logic [15:0] chan_b_s0_reg   /* synthesis syn_noprune=1 */;
    logic [15:0] chan_b_s1_reg   /* synthesis syn_noprune=1 */;

    logic        adc_cdc_fifo_dout_vld;
    logic [31:0] adc_cdc_fifo_dout;

    logic        adc_cdc_fifo_dout_vld_reg;
    logic [31:0] adc_cdc_fifo_dout_reg;
    logic        adc_cdc_fifo_dout_parity_reg;
    // For GRAV3 
    // Pins: 0-6,9-11,14,15 are inverted
    // SYNC is inverted too
    function automatic reg[15:0] ch_b_transform(reg [15:0] din);
        return {din[15:14],~din[13:0]};
    endfunction

    function automatic reg[15:0] ch_a_transform(reg [15:0] din);
        return {din[15:14], ~din[13:12], din[11:8], ~din[7:2], din[1:0]};
    endfunction

`ifndef VERILATE_DEF
    adc_data_rx adc_data_rx (
        .i_adc_data_clk     (ADC_DCLK),
        .i_adc_chan_a_data  (ADC_D[15:8]),
        .i_adc_chan_b_data  (ADC_D[ 7:0]),
        .o_sample_clk       (adc_sclk),
        .o_chan_a_sample_0  (chan_a_s0),
        .o_chan_a_sample_1  (chan_a_s1),
        .o_chan_b_sample_0  (chan_b_s0),
        .o_chan_b_sample_1  (chan_b_s1));

    always @(posedge adc_sclk) begin
        chan_a_s0_reg <= ch_a_transform(chan_a_s0);
        chan_a_s1_reg <= ch_a_transform(chan_a_s1);
        chan_b_s0_reg <= ch_b_transform(chan_b_s0);
        chan_b_s1_reg <= ch_b_transform(chan_b_s1);
    end


    logic [31:0] adc_cdc_fifo_data_in;
    logic       l2h_chan_sel_channel_select_recv_channel_r;
    // this commented code allows for selection between a/b
    // assign adc_cdc_fifo_data_in = (l2h_chan_sel_channel_select_recv_channel_r) ? {chan_b_s0_reg, chan_b_s1_reg} : {chan_a_s0_reg, chan_a_s1_reg};
    // this version is hardcoded to B
    assign adc_cdc_fifo_data_in = {chan_b_s0_reg, chan_b_s1_reg};

    pmi_fifo_dc_fwft_v1_0 #(
        .WR_DEPTH        (256),
        .WR_DEPTH_AFULL  (255),
        .WR_WIDTH        (32),
        .RD_WIDTH        (32),
        .FAMILY          ("ECP5U"),
        .IMPLEMENTATION  ("EBR"),
        .RESET_MODE      ("sync"),
        .WORD_SWAP       (0),
        .SIM_MODE        (0)
        ) adc_cdc_fifo (
        .wrclk           (adc_sclk),
        .wrclk_rst       (1'b0),
        .rdclk           (sys_clk),
        .rdclk_rst       (1'b0),
        .wren            (1'b1),
        .wdata           (adc_cdc_fifo_data_in),
        .full            (),
        .afull           (),
        .rden            (1'b1),
        .rdata           (adc_cdc_fifo_dout),
        .rdata_vld       (adc_cdc_fifo_dout_vld));

`endif

   reg 			adc_saturated;
   reg 			adc_saturation_clear;

   assign HS_EAST_OUT_SAT[37] = adc_saturated;
   // assign P2A_DDR_out[38] = adc_saturation_clear;
`ifndef VERILATE_DEF      
    always_ff @(posedge sys_clk) begin
        if (sys_clk_srst) begin
            adc_cdc_fifo_dout_vld_reg <= 0;
        end else begin
           adc_cdc_fifo_dout_vld_reg    <= adc_cdc_fifo_dout_vld;
           adc_cdc_fifo_dout_reg        <= adc_cdc_fifo_dout;
           adc_cdc_fifo_dout_parity_reg <= ^adc_cdc_fifo_dout;
	   adc_saturated<=0;
	   if(adc_cdc_fifo_dout_vld && (adc_cdc_fifo_dout[15:0]=='h7fff || adc_cdc_fifo_dout[31:16]=='h7fff || adc_cdc_fifo_dout[15:0]=='h8000 || adc_cdc_fifo_dout[31:16]=='h8000))
	     adc_saturated<=1;
	     
        end
    end
`else 
    always_ff @(posedge sys_clk) begin 
        adc_saturated<=0;
        if(i_data_valid_adc && (i_data_adc[15:0]=='h7fff || i_data_adc[31:16]=='h7fff || i_data_adc[15:0]=='h8000 || i_data_adc[31:16]=='h8000))
         adc_saturated<=1;
    end
`endif
    // assign P2A_DDR_OUT[33:0] = {adc_cdc_fifo_dout_vld_reg, adc_cdc_fifo_dout_parity_reg, adc_cdc_fifo_dout_reg};

    /*
     *
     * ADC DSP
     *
     */
    logic        adc_dsp_i_valid;
    logic [15:0] adc_dsp_i_inph_data;
    logic [15:0] adc_dsp_i_inph_delay_data;
    logic [15:0] dsp_inph;
    logic [15:0] dsp_quad;
    logic        dsp_valid;
    logic        bb_oflow_error;
    logic [15:0] downconv_inph;
    logic [15:0] downconv_quad;
    logic        downconv_valid;
    logic [11:0] adc_dsp_phase_inc_new;
    logic        adc_dsp_phase_inc_update_strb;
    logic        adc_dsp_sw_rst_strb;
    logic        adc_dsp_local_rst_status;

    logic [36:34] temp;
`ifdef VERILATE_DEF
    assign temp [36:34] = {HS_EAST_IN_GPIO[40:39], HS_EAST_IN[35]};
    downconverter #(
        .WIDTH(16))
    downconverter_inst (
        .i_inph_data      (i_data_adc[31:16]      ),
        .i_inph_delay_data(i_data_adc[15:0]),
        .i_valid          (i_data_valid_adc          ),
        .o_inph_data      (downconv_inph       ),
        .o_quad_data      (downconv_quad       ),
        .o_valid          (downconv_valid      ),
        .i_clock          (sys_clk          ),
        .i_reset          (sys_clk_srst      ));

`else
    genvar i;
    /*generate for (i = 34; i < 37; i = i + 1) begin



                defparam in_delay.DEL_VALUE = 96 ;
            defparam in_delay.DEL_MODE = "USER_DEFINED" ;
            DELAYG in_delay (.A(P2A_DDR_IN[i]), .Z(temp[i]));
        end
    endgenerate*/
assign temp [36:34] = {HS_EAST_IN_GPIO[40:39], HS_EAST_IN[35]};

    adc_dsp #(
            .WIDTH(16),
            .ACTIVE_CHANNEL(10))
        adc_dsp_inst (
            
            .i_inph_data        (adc_cdc_fifo_dout_reg[31:16]),
            .i_inph_delay_data  (adc_cdc_fifo_dout_reg[15:0]),
            .i_valid            (adc_cdc_fifo_dout_vld_reg),
            
            
            .o_inph             (dsp_inph),
            .o_quad             (dsp_quad),
            .o_valid            (dsp_valid),
            .o_downconv_inph    (downconv_inph),
            .o_downconv_quad    (downconv_quad),
            .o_downconv_valid   (downconv_valid),
            .i_clock            (sys_clk),
            .i_reset            (sys_clk_srst), // TODO: HAVE DARYL UPDATE HOW THIS BLOCK IS RESET SINCE WE NO LONGER HAVE THE CDC FIFO IN THE SAME FPGA AS THIS MODULE
            .i_phase_inc_update (adc_dsp_phase_inc_update_strb),
            .i_phase_inc_new    (adc_dsp_phase_inc_new),
            .i_sw_rst_strb      (adc_dsp_sw_rst_strb),
            .o_local_rst_status (adc_dsp_local_rst_status),
            .o_flow_problem     (bb_oflow_error));
`endif
    logic [31:0] adc_counter;
    logic gpio_enable;
    logic cs00_ready;
    logic cs00_reset;
    always_ff @(posedge sys_clk) begin
        if (sys_clk_srst) begin
            adc_counter <= 0;
            HS_EAST_OUT <= '0;
            cs00_ready <= 0;
            gpio_enable <= 0;
            cs00_reset <= 0;
        end else begin
            gpio_enable <= temp[35];
            cs00_ready <= temp[34];
            cs00_reset <= temp[36];
            if (cs00_reset) begin
                adc_counter <= 0;
            end
            
            if (gpio_enable) begin
                if (cs00_ready) begin
                    if (downconv_valid) begin
                        adc_counter <= adc_counter + 1;
                        HS_EAST_OUT[33:2] <= adc_counter;
                        HS_EAST_OUT[34] <= 1;
                    end else begin
                        HS_EAST_OUT[34] <= 0;
                    end
                end
            end else begin
                //HS_EAST_OUT[33:2] <= 32'h12345678; //adc_cdc_fifo_dout[31:0]; 
                HS_EAST_OUT[33:2] <= {
                    downconv_quad,
                    downconv_inph
                }; // Data
                //P2A_DDR_OUT[32] <= ^(downconv_inph) ^ (^(downconv_quad)); // Parity
                HS_EAST_OUT[34] <= downconv_valid; // Valid
            end
        end
        
    end

`ifndef VERILATE_DEF
    /*
     *
     * VARIABLE GAIN AMPLIFIER GAIN ADJUSTMENT
     *
     */

    logic        vga_pwr_dwn;
    logic [5:0]  vga_gain_ctrl;
    logic        vga_update_strb;
    logic        VGA_CS_N;
    logic        VGA_MOSI;
    logic        VGA_SCLK;
    logic        VGA_SPI_BUSY;

    vga_ctrl vga_ctrl_inst (
        .i_clk125           (sys_clk),
        .i_srst             (sys_clk_srst),
        .i_update_gain_ctrl (vga_update_strb),              // pulse for one clock when new gain ctrl reg value ready
        .i_gain_ctrl_val    ({vga_pwr_dwn, vga_gain_ctrl}), // bit 6 = VGA power down
        .o_cs_n             (VGA_CS_N),
        .o_mosi             (VGA_MOSI),
        .o_sclk             (VGA_SCLK),
        .o_spi_busy         (VGA_SPI_BUSY));

//    assign VGA_CTRL_A[0] = VGA_CS_N;
//    assign VGA_CTRL_A[1] = VGA_SCLK;
//    assign VGA_CTRL_A[2] = VGA_MOSI;
//
//    assign VGA_CTRL_B[0] = VGA_CS_N;
//    assign VGA_CTRL_B[1] = VGA_SCLK;
//    assign VGA_CTRL_B[2] = VGA_MOSI;


    /*
     *
     * DIGITAL STEP ATTENUATOR
     *
     */

    logic [5:0]  dsa_atten;

//    assign DSA_CTRL_A = dsa_atten;
//    assign DSA_CTRL_B = dsa_atten;


    /*
     * REGISTERS
     *
     * NOTE:
     *
     * I used .* for register field connections on purpose because if you update the reg.rdl file under the regs folder you might get additional
     * top level ports I want an error to be raised in the event this logic fails to get updated too.
     *
     */

    logic [23:0] h2l_rf0_fpga_uid_uid_w;
    logic [31:0] h2l_rf0_mib_cntr_lock_test_cnt_w;
    logic        h2l_rf2_second_amp_vga_busy_w;

    logic [5:0] l2h_rf1_first_amp_dsa_atten_r;
    logic       l2h_rf2_second_amp_vga_pwr_dwn_swmod_o;
    logic       l2h_rf2_second_amp_vga_pwr_dwn_r;
    logic       l2h_rf2_second_amp_vga_gain_ctrl_swmod_o;
    logic [5:0] l2h_rf2_second_amp_vga_gain_ctrl_r;

    assign h2l_rf0_fpga_uid_uid_w           = FPGA_UID;
    assign h2l_rf0_mib_cntr_lock_test_cnt_w = mib_rst_test_cntr_lock_reg;
    assign h2l_rf2_second_amp_vga_busy_w    = VGA_SPI_BUSY;

    assign vga_pwr_dwn   = l2h_rf2_second_amp_vga_pwr_dwn_r;
    assign vga_gain_ctrl = l2h_rf2_second_amp_vga_gain_ctrl_r;
    assign dsa_atten     = l2h_rf1_first_amp_dsa_atten_r;

    REGS_pio regs (
        .clk                    (sys_clk),
        .reset                  (sys_clk_srst),
        .h2d_pio_dec_address    (cmd_sys.byte_addr[CMD_ADDR_BITS-1:2]), // dword addressing so bits [1:0] are always 0
        .h2d_pio_dec_write_data (cmd_sys.wdata),
        .h2d_pio_dec_write      (cmd_sys.sel & ~cmd_sys.rd_wr_n),
        .h2d_pio_dec_read       (cmd_sys.sel & cmd_sys.rd_wr_n),
        .d2h_dec_pio_read_data  (cmd_sys.rdata),
        .d2h_dec_pio_ack        (cmd_sys.ack),
        .d2h_dec_pio_nack       (),
        .*);



    // fix for ORDT software mod strobes happening 1 clock too early.
    always_ff @(posedge sys_clk) begin
        if (sys_clk_srst) begin
            vga_update_strb <= 0;
        end else begin
            vga_update_strb <= l2h_rf2_second_amp_vga_gain_ctrl_swmod_o | l2h_rf2_second_amp_vga_pwr_dwn_swmod_o;
        end
    end
`endif
endmodule

`default_nettype wire
