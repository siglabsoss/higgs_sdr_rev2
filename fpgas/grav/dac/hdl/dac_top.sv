
/*
 * Module: dac_top
 *
 * This module receives sample from the ENET FPGA and
 * upconverts them to a suitable IF before sending them
 * to the DAC for further RF processing.
 *
 */

`include "udp_cmd_pkg.sv" // located in ip-library/lattice_support/gbit_mac/packages

`default_nettype none

module dac_top #(
      parameter VERILATE = 1'b0)
         (

    /* NAMES CHOSEN TO CLOSELY MATCH GRAVITON SCHEMATIC, BUT THERE MIGHT BE DIFFERENCES WHERE I THOUGHT SCHEMATIC NAMES WERE UNCLEAR */
    input wire          logic FPGA0_CLK,
    output wire         logic LED_D6, // RED
    output wire         logic LED_D14, // GREEN
    output wire         logic LED_D15, // YELLOW

    /* INTERFACE to ETHERNET */
    input wire          logic [34:2] HS_EAST_IN, // 31:0 = data, 32 = valid bit
    output wire         logic [35:35] HS_EAST_OUT, // 33 = ready signal sent to risc processor
    output wire [37:37] HS_EAST_OUT_SAT,
    output wire [38:38] HS_EAST_IN_CH_A,
    output wire [39:39] HS_EAST_IN_CH_B,

    /* TX/RX SWITCH CONTROL */
    output logic        TX_3V3_A,
    output logic        TX_3V3_B,

    /* DAC DATA INTERFACE */
    output wire         DAC_OSTR,
    output wire         logic DAC_DCLK,
    output wire         logic DAC_SYNC,
    output wire         logic DAC_FRAME,
    output wire         logic DAC_PARITY,
    output wire         logic [15:0] DAC_D,

    /* MIB (A.K.A. GPMC) */
//    input  wire              MIB_TBIT, // Training bit toggle pattern from MIB master
//    input  wire              MIB_START,
//    input  wire              MIB_RD_WR_N,
//    output wire              MIB_SLAVE_ACK,
    //input  wire    [16:15]    MIB_AD,

    /* CFG FPGA MASTER RESET AND LOCK STROBE (USES MIB/GPMC BUS SPARE SIGNAL LINES, BUT OPERATES IN THE SYS CLOCK DOMAIN) */

    input               MIB_MASTER_RESET // GPMC SIGNAL LINE 22, USED AS A MASTER RESET TO THE OTHER GRAVITON FPGAS AND THE CFG FPGA ON CS
//    input                    MIB_COUNTER_LOCK  // GPMC SIGNAL LINE 21, USED AS A WAY FOR SOFTWARE TO LOCK VARIOUS COUNTERS IN DIFFERENT FPGAS PRIOR TO READING THEM TO GET THEIR DELTAS (YOU SHOULD TRIPLE FLOP THIS AND DO RISING EDGE DETECTION ON IT)

);

    // SYSTEM SPECIFIC LOCAL PARAMETERS - DO NOT MODIFY
    localparam logic [3:0]  SLAVE_MIB_ADDR_MSN = 4'd1;         // Unique MIB Address Most Significant Nibble
    localparam logic [23:0] FPGA_UID           = 24'h44_41_43; // Software readable Unique FPGA ID stored in base register (ASCII for DAC)
    // End SYSTEM SPECIFIC LOCAL PARAMETERS



    /*
     *
     * CLOCKING, RESETS, AND MIB SLAVE
     *
     */

    localparam int NUM_SYS_CLK_RESETS                                   = 1;
    localparam int SYS_CLK_RESETS_EXTRA_CLOCKS [0:NUM_SYS_CLK_RESETS-1] = '{100}; // 100 extra clocks so that I release the system reset after dac_sclk_srst

    logic       sys_clk                 /* synthesis syn_keep=1 */;
    logic       sys_clk_srst            /* synthesis syn_keep=1 GSR=DISABLED */;
    logic       mib_clk                 /* synthesis syn_keep=1 */;
    logic       mib_clk_srst            /* synthesis syn_keep=1 GSR=DISABLED */;
    logic       int_osc_clk             /* synthesis syn_keep=1 */;
    logic       int_osc_clk_srst        /* synthesis syn_keep=1 GSR=DISABLED */;
    logic       sys_pll_locked          /* synthesis syn_keep=1 */;
    logic       mib_clk_deskew_done     /* synthesis syn_keep=1 */;
    logic       dac_sclk                /* synthesis syn_keep=1 */;
    logic       dac_sclk_srst           /* synthesis syn_keep=1 GSR=DISABLED */;
    logic       dac_tx_interface_ready; // this is in the sync_clk domain of dac_tx_2x_ddr which is currently int_osc_clk

    intf_cmd #(CMD_ADDR_BITS, CMD_DATA_BITS) cmd_sys(); // parameters specified in udp_cmd_pkg.sv

    // core_top #(
    //      // most params use defaults, which makes it easier to apply global changes (i.e. you don't have to go FPGA-to-FPGA to make changes)
    //      .NUM_SYS_CLK_SRSTS                  (NUM_SYS_CLK_RESETS),
    //      .SYS_CLK_SRSTS_EXTRA_CLOCKS         (SYS_CLK_RESETS_EXTRA_CLOCKS),
    //      .MIB_SLAVE_ADDR_MSN                 (SLAVE_MIB_ADDR_MSN),
    //      .INCLUDE_MIB_SLAVE               (1'b0),
    //      .MIB_CLOCK_DESKEW_ENABLE         (1'b0)
    //   ) core_top (
    //      .i_fpga_clk          (FPGA0_CLK),
    //      .i_fpga_ext_arst     (MIB_MASTER_RESET), // loop back internal oscillator srst to reset sys clock domain (we're the MIB_MASTER_RESET driver, so we don't use that to reset sys clock domain)
    //      .o_int_osc_clk       (int_osc_clk),
    //      .o_int_osc_clk_srst  (int_osc_clk_srst),
    //      .o_sys_clk           (sys_clk),
    //      .o_sys_clk_srsts     ({sys_clk_srst}),
    //      .o_mib_clk           (mib_clk),
    //      .o_mib_clk_srsts     ({mib_clk_srst}),
    //      .o_mib_deskew_done   (),
    //      .o_sys_pll_locked    (sys_pll_locked),
    //      .i_mib_tbit          (),
    //      .i_mib_start         (),
    //      .i_mib_rd_wr_n       (),
    //      .b_mib_ad            (),
    //      .o_mib_slave_ack     (),
    //      .cmd_sys             (cmd_sys)
    //   );

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
        .i_fpga_clk          (FPGA0_CLK),
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


    assign LED_D14  = sys_pll_locked & dac_tx_interface_ready;

`ifndef VERILATE_DEF
    core_reset #(
        .NUM_OUTPUTS        (1),
        .EXTRA_RESET_CLOCKS ('{10})
    ) dac_sclk_resets (
        .i_ext_arst         (MIB_MASTER_RESET | ~dac_tx_interface_ready),
        .i_clk              (dac_sclk),
        .i_clk_pll_unlocked (~dac_tx_interface_ready),
        .o_sync_resets      ({dac_sclk_srst})
    );
`endif


/********************************************************************************/

    /*
     * NOTE: THIS IS CURRENTLY JUST A TEST TO SEE IF THIS WORKS AS A GOOD WAY OF SIMULTANEOUSLY RESETTING ALL FPGAS ON BOTH GRAVITON AND CS
     *
     * After issuing the mib master reset via software, issue the software counter lock, and then read this counter and the counters in DAC and ETH FPGAs and compare their count values.
     */

    logic [31:0] mib_rst_test_cntr;
    logic [31:0] mib_rst_test_cntr_lock_reg;
    logic [2:0]  mib_counter_lock_sync_ffs;

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

    always_ff @(posedge sys_clk) begin
        if (mib_counter_lock_sync_ffs[1] & ~mib_counter_lock_sync_ffs[2]) begin // only lock on rising edge
            mib_rst_test_cntr_lock_reg <= mib_rst_test_cntr;
        end
    end

/********************************************************************************/


    /*
     *
     * TX SAMPLE RECEPTION AND PARITY CHECK (FROM CS TO GRAV)
     *
     */

    logic [33:0] temp;
    logic [18:0] dac_sample_cdc_fifo_wdata;
    logic upconverter_in_data_ready;
    logic [31:0] upconverter_in_data;
    logic upconverter_in_data_valid;

`ifndef VERILATE_DEF
    genvar i;
    generate for (i = 0; i < 33; i = i + 1) begin



            defparam in_delay.DEL_VALUE = 96 ;
         defparam in_delay.DEL_MODE = "USER_DEFINED" ;
         DELAYG in_delay (.A(HS_EAST_IN[i+2]), .Z(temp[i]));
      end
    endgenerate
`else
        assign temp[33:0] = HS_EAST_IN[34:2];
        integer f0;
        integer f1;

        initial begin
`ifdef DAC_DUMP_UPCONVERTER_OUTPUT_MIF
            f0 = $fopen("out_data.mif","w");
`endif
`ifdef DAC_DUMP_CS10_OUTPUT_MIF
            f1 = $fopen("upconverter_in_data.mif","w");
`endif
        end
        logic flag;
        always_ff @(posedge sys_clk) begin
            if (sys_clk_srst) begin
                flag <= 0;
            end else begin
                // if (upconverter_in_data_ready && upconverter_in_data_valid) begin
                //     $fwrite(f1,"%h\n",upconverter_in_data);
                // end
`ifdef DAC_DUMP_CS10_OUTPUT_MIF
                if (HS_EAST_OUT[35] && HS_EAST_IN[34]) begin
                    $fwrite(f1,"%h\n",P2B_DDR_IN[31:0]);
                end
`endif

`ifdef DAC_DUMP_UPCONVERTER_OUTPUT_MIF
                if (upconverter_in_data_valid) begin
                    flag <= 1;
                end
                if (flag) begin
                    $fwrite(f0,"%h\n",upconverter_out_data[15:0]);
                end
`endif

            end
        end
        // Uncomment to send data directly
        // localparam IN_DEPTH = 1024;

        // logic [31:0] mem_bs [IN_DEPTH:0];
        // logic [$clog2(IN_DEPTH)-1:0] read_addr;

        // initial $readmemh("in_data.mif",mem_bs);
        // logic [31:0] dummy_counter;
        // always_ff @(posedge sys_clk) begin
        //     if (sys_clk_srst) begin
        //         upconverter_in_data_valid <= 0;
        //         dummy_counter <= 0;
        //         read_addr <= {$clog2(IN_DEPTH){1'b0}};
        //     end else begin
        //         upconverter_in_data_valid <= 0;
        //         if (upconverter_in_data_ready) begin
        //             dummy_counter <= dummy_counter + 1;
        //         end
        //         if (upconverter_in_data_ready && (dummy_counter == 1000)) begin
        //             read_addr <= read_addr + 1;
        //             upconverter_in_data_valid <= 1;
        //             upconverter_in_data <= mem_bs[read_addr];
        //             dummy_counter <= dummy_counter;
        //         end
        //     end
        // end
        // Uncomment to send data directly

`endif

    // localparam IN_DEPTH = 1024;

    //     logic [31:0] mem_bs [IN_DEPTH:0];
    //     logic [$clog2(IN_DEPTH)-1:0] read_addr;

    //     initial $readmemh("in_data.mif",mem_bs);
        
    //     always_ff @(posedge sys_clk) begin
    //         if (sys_clk_srst) begin
    //             upconverter_in_data_valid <= 0;
                
    //             read_addr <= {$clog2(IN_DEPTH){1'b0}};
    //         end else begin
    //             upconverter_in_data_valid <= 0;
                
    //             if (upconverter_in_data_ready) begin
    //                 read_addr <= read_addr + 1;
    //                 upconverter_in_data_valid <= 1;
    //                 upconverter_in_data <= mem_bs[read_addr];
    //             end
    //         end
    //     end
    

    // Comment out this block to send data directly
    eb2a #(.T_0_WIDTH (32),
         .I_0_WIDTH (32))
      eb1_5_inst_in (
         .clk                 (sys_clk),
         .reset_n             (!sys_clk_srst),

         .t0_data             (temp[31:0]),
         .t0_valid            (temp[32]),
         .t0_ready            (HS_EAST_OUT[35]),

         .i0_data             (upconverter_in_data),
         .i0_valid            (upconverter_in_data_valid),
         .i0_ready            (upconverter_in_data_ready)
      );

   /*data_generator dg_0
     (.i_data(upconverter_in_data),
      .i_valid(upconverter_in_data_valid),
      .i_ready(1'b1),//upconverter_in_data_ready),
      .clk(sys_clk),
      .rstf(!sys_clk_srst)
      );*/
    // Comment out this block to send data directly
    // logic dummy_flag = 1;
    // // assign dummy_flag = (temp[32]) ? 1'b0 : dummy_flag;

    // always_ff @(posedge sys_clk) begin
    //     if (sys_clk_srst) begin
    //         dummy_flag <= 1;            
    //     end else begin
    //         if (temp[32]) begin
    //             dummy_flag <= 0;
    //         end
    //     end
    // end 

    logic [15:0] dac_sample;
    logic        dac_sample_vld;
    logic [15:0] upconverter_out_data;
    
    always_ff @(posedge sys_clk) begin
        if (sys_clk_srst) begin
            dac_sample_cdc_fifo_wdata <= 0;
            
        end else begin
            dac_sample_cdc_fifo_wdata[15:0] <= upconverter_out_data[15:0];
        end
    end

    upconverter #(
         .WIDTH        (16),
         .INTERP_ONLY  (1'b0) // set to 1 to bypass the DDS and just perform interpolation
      ) UPCONVERTER (
         .i_inph_data  (upconverter_in_data[15:0]),
         .i_quad_data  (upconverter_in_data[31:16]),
         .o_ready      (upconverter_in_data_ready),
         .o_inph_data  (upconverter_out_data[15:0]),
         //        .o_quad_data  (upconv_o_quad_data),
         .o_quad_data  (),  // not used when operating as full upconverter (i.e. not in INTERP_ONLY mode)
         .i_ready      (1'b1),
         .i_clock      (sys_clk),
         .i_reset      (sys_clk_srst));
            // .i_reset      (dummy_flag));

   // Saturation logic
   reg dac_saturated;

   assign HS_EAST_OUT_SAT[37] = dac_saturated;

   always_ff @(posedge sys_clk) begin
      if (sys_clk_srst) begin
         dac_saturated <= 0;
      end
      else begin
         dac_saturated <= (upconverter_out_data[15:0] == 'h7fff) || (upconverter_out_data[15:0] == 'h8000);
      end
   end

`ifndef VERILATE_DEF
    logic [15:0] tx_sample_reg;
    logic        tx_sample_parity_reg;
    logic        tx_sample_vld_reg;
    logic        tx_tslot_first_sample_reg;
    logic        tx_tslot_last_sample_reg;
    logic        tx_sample_parity_error;
    logic [31:0] tx_sample_parity_error_cnt;

//    always_ff @(posedge sys_clk) begin
//        if (sys_clk_srst) begin
//            tx_sample_vld_reg          <= 0;
//            tx_tslot_first_sample_reg  <= 0;
//            tx_tslot_last_sample_reg   <= 0;
//            tx_sample_parity_error     <= 0;
//            tx_sample_parity_error_cnt <= '0;
//        end else begin
//            tx_tslot_last_sample_reg   <= temp[19];
//            tx_tslot_first_sample_reg  <= temp[18];
//            tx_sample_vld_reg          <= temp[17]; // this is expected to go high when the first sample arrives and stay high after that
//            tx_sample_parity_reg       <= temp[16];
//            tx_sample_reg              <= temp[15:0];
//            tx_sample_parity_error     <= tx_sample_vld_reg & (^{tx_sample_parity_reg, tx_sample_reg});
//            tx_sample_parity_error_cnt <= (tx_sample_parity_error) ? tx_sample_parity_error_cnt + 1 : tx_sample_parity_error_cnt;
//        end
//    end


    /*
     *
     * SYS CLOCK TO DAC CLOCK DOMAIN CROSSING
     *
     */

//    logic [15:0] dac_sample;
//    logic        dac_sample_vld;
    logic        dac_tslot_first_sample;
    logic        dac_tslot_last_sample;
    logic        tx_first_vld_sample_seen;
//
//    logic [18:0] dac_sample_cdc_fifo_wdata;
//
//    always_ff @(posedge sys_clk) begin
//        if (sys_clk_srst) begin
//            tx_first_vld_sample_seen  <= 0;
//            dac_sample_cdc_fifo_wdata <= '0;
//        end else begin
//            if (tx_sample_vld_reg) begin
//                tx_first_vld_sample_seen <= 1;
//            end
//            /*
//             * Zero stuff until first valid sample comes in.
//             * After that all samples are assumed valid and upstream logic should handle zero stuffing.
//             */
//            if (tx_sample_vld_reg | tx_first_vld_sample_seen) begin
//                dac_sample_cdc_fifo_wdata <= {tx_tslot_last_sample_reg, tx_tslot_first_sample_reg, 1'b1, tx_sample_reg};
//            end else begin
//                dac_sample_cdc_fifo_wdata <= '0;
//            end
//        end
//    end
//
    pmi_fifo_dc_fwft_v1_0 #(
        .WR_DEPTH        (8),
        .WR_DEPTH_AFULL  (7),
        .WR_WIDTH        (19),
        .RD_WIDTH        (19),
        .FAMILY          ("ECP5U"),
        .IMPLEMENTATION  ("LUT"),
        .RESET_MODE      ("sync"),
        .WORD_SWAP       (0),
        .SIM_MODE        (0)
        ) DAC_SAMPLE_CDC_FIFO (
        .wrclk           (sys_clk),
        .wrclk_rst       (sys_clk_srst),
        .wren            (1'b1),
        .wdata           (dac_sample_cdc_fifo_wdata),
        .full            (),
        .afull           (),
        .rdclk           (dac_sclk),
        .rdclk_rst       (dac_sclk_srst),
        .rden            (1'b1),
        .rdata           ({dac_tslot_last_sample, dac_tslot_first_sample, dac_sample_vld, dac_sample}),
        .rdata_vld       ());

    /*
     * delay dac data samples by enough time to allow the time slot first sample strobe to put the Tx/Rx switch into Tx mode.
     */

    localparam int unsigned DAC_DATA_PIPE_DEPTH = 625; // should give 5 microseconds at 125MHz dac_sclk

    logic [DAC_DATA_PIPE_DEPTH-1:0] [15:0] dac_sample_pipe;
    logic [DAC_DATA_PIPE_DEPTH-1:0]        dac_sample_vld_pipe;
    logic [15:0]                           dac_tx_chan_a_s0;
    logic [15:0]                           dac_tx_chan_a_s1;
    logic [15:0]                           dac_tx_chan_b_s0;
    logic [15:0]                           dac_tx_chan_b_s1;
    logic                                  dac_tx_sync;

   function logic[15:0] dac_sample_invert(logic[15:0] s);
      return {s[15],~s[14],~s[13],~s[12],~s[11],s[10],s[9],~s[8],~s[7],~s[6],~s[5],s[4],s[3],~s[2],~s[1],s[0]};
   endfunction

    always_ff @(posedge dac_sclk) begin
        if (dac_sclk_srst) begin
            dac_sample_vld_pipe <= '0;
            dac_sample_pipe     <= '0;
        end else begin
            dac_sample_vld_pipe <= {dac_sample_vld_pipe[DAC_DATA_PIPE_DEPTH-2:0], !dac_sclk_srst};
            dac_sample_pipe     <= {dac_sample_pipe[DAC_DATA_PIPE_DEPTH-2:0], dac_sample_invert(dac_sample)};
        end
    end

    // copied from how Graviton SDR was doing things

    always_ff @(posedge sys_clk) begin
      if (sys_clk_srst) begin
         TX_3V3_A <= 1'b1;
         TX_3V3_B <= 1'b1;
      end else begin
         TX_3V3_A <= 1'b0;//HS_EAST_IN_CH_A;//MIB_AD[16];
         TX_3V3_B <= 1'b0;
//HS_EAST_IN_CH_B;//MIB_AD[15];
      end
    end
    assign dac_tx_chan_a_s0 = '0;
    assign dac_tx_chan_a_s1 = (TX_3V3_A) ? '0 : dac_sample_pipe[DAC_DATA_PIPE_DEPTH-1];
    assign dac_tx_chan_b_s0 = (TX_3V3_B) ? '0 : dac_sample_pipe[DAC_DATA_PIPE_DEPTH-1];
    assign dac_tx_chan_b_s1 = '0;
    assign dac_tx_sync      = dac_sample_vld_pipe[DAC_DATA_PIPE_DEPTH-1];


    /*
     * NOTE:
     *
     * INITIALLY I CASCADED sys_pll WITH ANOTHER PLL CALLED mib_pll BECAUSE I NEEDED A STATIC 90 DEGREE PHASE SHIFTED VERSION OF THE 250MHz DAC CLOCK AS WELL AS THE ABILITY TO
     * DYNAMICALLY SHIFT THE MIB CLOCK, BUT I COULDN'T DO THAT WITHIN A SINGLE PLL.
     *
     * HOWEVER, DOING SO RESULTED IN FAILURE TO MEET CLOCK-TO-OUT CONSTRAINT REQUIREMENTS BY A HEALTHY AMOUNT (approx 1ns).  THEREFORE, I NOW INCLUDE A PLL IN dac_tx_2x_ddr THAT IS FED
     * BY sys_clk OUT OF sys_pll AND GENERATES THE 250MHz AND 250MHz 90 DEGREES PHASE SHIFTED CLOCKS FOR DRIVING THE DAC DATA INTERFACE.  THE CASCADING OF PLLs WILL INCREASE THE JITTER,
     * BUT SINCE THE INTERFACE TO THE DAC IS SOURCE SYNCHRONOUS AND FEEDS A DUAL CLOCK FIFO IN THE DAC, THIS INCREASED JITTER WILL HOPEFULLY MAKE NO DIFFERENCE IN THE OPERATION OF THE
     * DAC.
     *
     * THAT SAID, IF YOU'RE EXPERIENCING BAD PERFORMANCE OUT OF THE DAC YOU MAY WANT TO LOOK INTO AT LEAST TEMPORARILY REPLACING sys_pll WITH A DIFFERENT PLL THAT OUTPUTS 250MHz,
     * 250MHz 90 DEGREES PHASE SHIFTED, 125MHz, AND 12.5MHz ON CLKOP, CLKOS, CLKOS2, AND CLKOS3 RESPECTIVELY (MAKING SURE TO USE CLKOS2 AS THE PLL FEEDBACK SOURCE) AND FOREGOING
     * THE MIB CLOCK SHIFTING.  YOU WILL ALSO NEED TO REGENERATE dac_tx_2x_ddr TO REMOVE THE PLL FROM INSIDE OF IT.
     *
     * YOU WILL NOT MEET CLOCK-TO-OUT CONSTRAINTS FOR BOTH MIB/GPMC AND P2A_DDR_OUT, BUT IF YOUR DAC PERFORMANCE IS FIXED THEN AT LEAST YOU KNOW WHERE THE PROBLEM WAS.
     *
     *
     */

    logic custom_reset = 1;
    logic custom_flag = 0;
    logic [31:0] custom_counter;
    always_ff @(posedge sys_clk) begin
        if (sys_clk_srst) begin
            custom_reset <= 1;
            custom_flag <= 0;
            custom_counter <= 0;
        end else begin
            custom_reset <= !custom_flag;

            if (!TX_3V3_A || !TX_3V3_B) begin
                custom_counter <= custom_counter + 1;
            end else begin
                custom_counter <= 0;
            end

            if (custom_counter == 62500000) begin
                custom_flag <= 1;
            end
        end
    end

    wire dac_tx_chan_a_s0_parity;
    wire dac_tx_chan_a_s1_parity;
    wire dac_tx_chan_b_s0_parity;
    wire dac_tx_chan_b_s1_parity;

    assign dac_tx_chan_a_s0_parity = ^dac_tx_chan_a_s0;
    assign dac_tx_chan_a_s1_parity = ^dac_tx_chan_a_s1;
    assign dac_tx_chan_b_s0_parity = ^dac_tx_chan_b_s0;
    assign dac_tx_chan_b_s1_parity = ^dac_tx_chan_b_s1;

    assign LED_D6 = custom_flag;

    dac_tx_2x_ddr dac_tx_2x_ddr (
        .refclk     (sys_clk),
        .pll_reset  (~custom_flag),
        .ready      (dac_tx_interface_ready), // synchronous to sync_clk
        .sync_clk   (int_osc_clk),
        .sync_reset (int_osc_clk_srst),
        .sclk       (dac_sclk),
        .data       ({dac_tx_sync, dac_tx_chan_b_s1, dac_tx_sync, dac_tx_chan_b_s0, dac_tx_sync, dac_tx_chan_a_s1, dac_tx_sync, dac_tx_chan_a_s0} ),
        .clkout     (DAC_DCLK),
        .dout       ({DAC_SYNC, DAC_D})
        );
    //assign DAC_D[8] = 1;
    //assign DAC_D[9] = 0;
    //assign DAC_D = 16'hA0A0;
   //assign DAC_SYNC = !dac_sclk_srst;
   //assign DAC_FRAME = !0;
    assign DAC_PARITY = !0;
    assign DAC_OSTR = 0;

    logic [2:0] dac_sclk_srst_n_clk125;




    /*
     *
     * TX/RX SWITCH CONTROL
     *
     */

    logic                                          enable_tx_n;
    logic [$clog2(DAC_DATA_PIPE_DEPTH)-1:0]        switch_delay_cnt;
    enum { RX_STATE, TX_STATE, TX_TO_RX_LAG_STATE} tx_switch_fsm_state;

//    always_ff @(posedge dac_sclk) begin
//        if (dac_sclk_srst) begin
//            tx_switch_fsm_state <= RX_STATE;
//            switch_delay_cnt    <= '0;
//            enable_tx_n         <= 1;
//        end else begin
//
//            switch_delay_cnt <= switch_delay_cnt + 1;
//
//            case (tx_switch_fsm_state)
//
//                RX_STATE: begin
//                    enable_tx_n <= 1;
//                    if (dac_tslot_first_sample) begin
//                        enable_tx_n         <= 0;
//                        tx_switch_fsm_state <= TX_STATE;
//                    end
//                end
//
//                TX_STATE: begin
//                    enable_tx_n <= 0;
//                    if (dac_tslot_last_sample) begin
//                        switch_delay_cnt    <= '0;
//                        tx_switch_fsm_state <= TX_TO_RX_LAG_STATE;
//                    end
//                end
//
//                TX_TO_RX_LAG_STATE: begin
//                    enable_tx_n <= 0;
//                    if (dac_tslot_first_sample) begin
//                        enable_tx_n         <= 0;
//                        tx_switch_fsm_state <= TX_STATE;
//                    end else if (switch_delay_cnt == DAC_DATA_PIPE_DEPTH-1) begin
//                        enable_tx_n         <= 1;
//                        tx_switch_fsm_state <= RX_STATE;
//                    end
//                end
//            endcase
//        end
//    end
    logic l2h_chan_sel_channel_select_a_transmit_channel_a_r;
    logic l2h_chan_sel_channel_select_b_transmit_channel_b_r;
//    assign TX_3V3_A = !l2h_chan_sel_channel_select_a_transmit_channel_a_r;  // enable/disable TX on RF port A
//    assign TX_3V3_B = !l2h_chan_sel_channel_select_b_transmit_channel_b_r;         // disable TX on RF port B
    assign LED_D15  = !TX_3V3_B || !TX_3V3_A; //~enable_tx_n; // Report whether TX is enabled



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

    logic [23:0] h2l_rf0_fpga_uid_uid_w;
    logic [31:0] h2l_rf0_mib_cntr_lock_test_cnt_w;
    logic [31:0] h2l_rf0_tx_sample_perrs_parity_errors_w;

    assign h2l_rf0_fpga_uid_uid_w                  = FPGA_UID;
    assign h2l_rf0_mib_cntr_lock_test_cnt_w        = mib_rst_test_cntr_lock_reg;
    assign h2l_rf0_tx_sample_perrs_parity_errors_w = tx_sample_parity_error_cnt;

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
`endif

endmodule

`default_nettype wire
