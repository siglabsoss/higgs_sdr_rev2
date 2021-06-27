/*
 * Module: cfg_top
 *
 */

module USRMCLK (USRMCLKI, USRMCLKTS) /* synthesis syn_blackbox=1 */;
input USRMCLKI, USRMCLKTS;
endmodule

`include "udp_cmd_pkg.sv" // located in ip-library/lattice_support/gbit_mac/packages

`default_nettype none

module cfg_top #(parameter VERILATE = 1'b0)(

    /* NAMES CHOSEN TO CLOSELY MATCH GRAVITON SCHEMATIC, BUT THERE MIGHT BE DIFFERENCES WHERE I THOUGHT SCHEMATIC NAMES WERE UNCLEAR */

    input            CFG_CLK,
    output           LED_D3,
    output           LED_D10,
    output           LED_D11,

    /* MIB (A.K.A. GPMC) */
//    input  wire              MIB_TBIT, // Training bit toggle pattern from MIB master
//    input  wire              MIB_START,
//    input  wire              MIB_RD_WR_N,
//    output wire              MIB_SLAVE_ACK,
    input  wire    [20:17]    MIB_AD,

    /* CFG FPGA MASTER RESET AND LOCK STROBE (USES MIB/GPMC BUS SPARE SIGNAL LINES, BUT OPERATES IN THE SYS CLOCK DOMAIN) */
    
    // output wire      MIB_MASTER_RESET, // GPMC SIGNAL LINE 22, USED AS A MASTER RESET TO THE OTHER GRAVITON FPGAS AND THE CFG FPGA ON CS
    // output wire      MIB_COUNTER_LOCK,  // GPMC SIGNAL LINE 21, USED AS A WAY FOR SOFTWARE TO LOCK VARIOUS COUNTERS IN DIFFERENT FPGAS PRIOR TO READING THEM TO GET THEIR DELTAS (YOU SHOULD TRIPLE FLOP THIS AND DO RISING EDGE DETECTION ON IT)
    

    /* CFG FLASH */

    output           CFG_SPI_SN,
    inout            CFG_SPI_DQ0,
    input            CFG_SPI_DQ1,
    input            CFG_SPI_DQ2,
    input            CFG_SPI_DQ3,

    /* ETH FPGA CFG (FPGACFG2) */

    output           ETH_CFG_PROGN,
    output           ETH_CFG_CCLK,
    output           ETH_CFG_DI,
    input            ETH_CFG_INITN,
    input            ETH_CFG_DONE /* synthesis syn_force_pads=1 */,

    /* ADC FPGA CFG (FPGACFG1) */

    output           ADC_CFG_PROGN,
    output           ADC_CFG_CCLK,
    output           ADC_CFG_DI,
    input            ADC_CFG_INITN,
    input            ADC_CFG_DONE /* synthesis syn_force_pads=1 */,

    /* DAC FPGA CFG (FPGACFG0) */

    output           DAC_CFG_PROGN,
    output           DAC_CFG_CCLK,
    output           DAC_CFG_DI,
    input            DAC_CFG_INITN,
    input            DAC_CFG_DONE /* synthesis syn_force_pads=1 */,

    /* DAC-CTRL */

    output           DAC_CTRL_TXENABLE, // currently set to 0
    output           DAC_CTRL_SLEEP, // currently set to 0
    output  reg      DAC_CTRL_SDIO, // technically an inout, but we currently don't support reads from DAC SIF
    output  reg      DAC_CTRL_SDENN,
    output  reg      DAC_CTRL_SCLK,
    output  reg      DAC_CTRL_RESETN,
 //   input            DAC_CTRL_SDO,
    input            DAC_CTRL_ALARM,

    /* ADC-CTRL */

    output           ADC_CTRL_SEN,
    output           ADC_CTRL_SDATA,
    output           ADC_CTRL_SCLK,
    output           ADC_CTRL_RESET,
//    input          ADC_CTRL_SDOUT, // we currently don't support reads from ADC SIF
//    input          ADC_CTRL_CTRL1, // used for channel a over range indication (need to configure ADC register 0x20 bit 0 to 1)
//    input          ADC_CTRL_CTRL2, // used for channel b over range indication (need to configure ADC register 0x20 bit 0 to 1)

    /* CLK-CTRL (LMK04826) */

    output           CLK_CTRL_SEL0,
    output           CLK_CTRL_SEL1,
    output           CLK_CTRL_SDIO, // technically and inout, but we currently don't support reads from LMK04826 SIF
    output           CLK_CTRL_SCK,
    output           CLK_CTRL_RESET,
    output           CLK_CTRL_CSN,
    output           CLK_CTRL_SYNC,
//    input          CLK_CTRL_LD1,
//    input          CLK_CTRL_LD2

    /* SYNTH-CTRL (LMK04133) */

    output reg       SYNTH_CTRL_LE,   // ACTIVE-LOW (named this way to match schematic)
    output reg       SYNTH_CTRL_DATA,
    output reg       SYNTH_CTRL_CLK,
    output reg       SYNTH_CTRL_GOE   // Global Output Enable.  It has an internal pull-up and we can selectively enable/disable clock outputs via register writes so no need for this at the moment (NOTE: SCHEMATIC ERROR, THIS SIGNAL IS NAMED SYNTH-CTRL.CE INSTEAD OF SYNTH-CTRL.GOE)

);

    // SYSTEM SPECIFIC LOCAL PARAMETERS - DO NOT MODIFY
    localparam logic [3:0]  SLAVE_MIB_ADDR_MSN = 4'd0;                  // Unique MIB Address Most Significant Nibble  
    localparam logic [23:0] FPGA_UID           = {8'h43, 8'h46, 8'h47}; // Software readable Unique FPGA ID stored in base register (ASCII "CFG")
    // End SYSTEM SPECIFIC LOCAL PARAMETERS
    
    
    /*
     * 
     * CLOCKING, RESETS, AND MIB SLAVE
     * 
     */
    logic MIB_MASTER_RESET;
    localparam int NUM_SYS_CLK_RESETS                                   = 1;
    localparam int SYS_CLK_RESETS_EXTRA_CLOCKS [0:NUM_SYS_CLK_RESETS-1] = '{100}; // 100 extra clocks so that I release the system reset after dac_sclk_srst

    logic       sys_clk                  /* synthesis syn_keep=1 */;
    logic       sys_clk_srst             /* synthesis syn_keep=1 GSR=DISABLED */;
    logic       mib_clk                  /* synthesis syn_keep=1 */;
    logic       mib_clk_srst             /* synthesis syn_keep=1 GSR=DISABLED */;
    logic       int_osc_clk              /* synthesis syn_keep=1 */;
    logic       int_osc_clk_srst         /* synthesis syn_keep=1 GSR=DISABLED */;
    logic       sys_pll_locked           /* synthesis syn_keep=1 */;
    logic       mib_clk_deskew_done      /* synthesis syn_keep=1 */;
    logic       soft_mib_rst_strb       /* synthesis syn_keep=1 */;
    logic       soft_mib_cntr_lock_strb /* synthesis syn_keep=1 */;
    logic       sif_cfg_done             /* synthesis syn_keep=1 */;

    intf_cmd #(CMD_ADDR_BITS, CMD_DATA_BITS) cmd_sys(); // parameters specified in udp_cmd_pkg.sv

    core_top #(
        // most params use defaults, which makes it easier to apply global changes (i.e. you don't have to go FPGA-to-FPGA to make changes)
        .NUM_SYS_CLK_SRSTS                  (NUM_SYS_CLK_RESETS),
        .SYS_CLK_SRSTS_EXTRA_CLOCKS         (SYS_CLK_RESETS_EXTRA_CLOCKS), 
        .MIB_SLAVE_ADDR_MSN                 (SLAVE_MIB_ADDR_MSN),
        .INCLUDE_MIB_SLAVE					(1'b0),
        .MIB_CLOCK_DESKEW_ENABLE			(1'b0),
        .VERILATE                           (VERILATE)
    ) core_top (
        .i_fpga_clk          (CFG_CLK),
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
    
    
    /* 
     * MIB MASTER RESET AND COUNTER LOCK
     */

    // drive the software counter lock signal that goes out on MIB_COUNTER_LOCK for many clock cycles to make sure it is seen by everybody on the bus
    // this is because the GPMC/MIB bus is stupidly long (over 2 feet on Copper Suicide) so the Lattice FPGA output drivers have a hard time actually driving the signal lines.
    localparam int unsigned CLKS_TO_DRIVE_MIB_STROBES = 50;

    logic custom_reset = 1;
    core_reset #(
        .NUM_OUTPUTS        (1),
        .VERILATE           (VERILATE),
        // .EXTRA_RESET_CLOCKS ('{CLKS_TO_DRIVE_MIB_STROBES}) // use this to extend the duration of the MIB_MASTER_RESET to as long as needed 
        .EXTRA_RESET_CLOCKS (1) // use this to extend the duration of the MIB_MASTER_RESET to as long as needed 
    ) mib_master_reset (
    	.i_ext_arst         (int_osc_clk_srst | ~sif_cfg_done | soft_mib_rst_strb | custom_reset),
        .i_clk              (sys_clk),
        .i_clk_pll_unlocked (~sys_pll_locked),
        .o_sync_resets      ({MIB_MASTER_RESET})
    );

    
    logic mib_cntr_lock_strb /* synthesis syn_keep=1*/;
    int unsigned mib_cntr_lock_strb_cnt;
    logic [35:0] timer_counter;
    always_ff @(posedge sys_clk) begin
        if (sys_clk_srst) begin
            mib_cntr_lock_strb     <= 1'b0;
            mib_cntr_lock_strb_cnt <= 0;
            timer_counter <= 0;
            custom_reset <= 1;
        end else begin
        	timer_counter <= timer_counter + 1;
       	    if (timer_counter == 36'd28_125_000_000 /*225 seconds */) begin
        	// if (timer_counter == 36'd18_750_000_000 /*150 seconds */) begin
        		custom_reset <= 0;
        		timer_counter <= timer_counter;
        	end
            if (soft_mib_cntr_lock_strb) begin
                mib_cntr_lock_strb     <= 1'b1;
                mib_cntr_lock_strb_cnt <= 0;
            end else begin
                if (mib_cntr_lock_strb_cnt == CLKS_TO_DRIVE_MIB_STROBES-1) begin
                    mib_cntr_lock_strb     <= 0;
                    mib_cntr_lock_strb_cnt <= mib_cntr_lock_strb_cnt;
                end else begin
                    mib_cntr_lock_strb     <= 1'b1;
                    mib_cntr_lock_strb_cnt <= mib_cntr_lock_strb_cnt + 1;
                end
            end
        end
    end
    
    // assign MIB_COUNTER_LOCK = mib_cntr_lock_strb;
    
    
    assign LED_D3  = sif_cfg_done & sys_pll_locked;
    assign LED_D10 = 0;
    assign LED_D11 = 0;

`ifndef VERILATE_DEF

    /* CFG FLASH READER SIGNALS */

    logic       cfg_flash_rd_pause;
    logic       cfg_flash_rd_busy;
    logic       cfg_flash_rd_byte_vld;
    logic [7:0] cfg_flash_rd_byte;
    logic       cfg_spi_clk;
    logic       cfg_spi_d0_high_z;
    logic       cfg_spi_dq0_out;

    enum {
        ETH_FPGA,
        ADC_FPGA,
        DAC_FPGA
    } cfg_flash_rd_byte_dest;


    /* FPGA SLAVE SERIAL PROGRAMMER SIGNALS */

    logic       fpga_ss_start;

    logic       eth_ss_byte_vld;
    logic [7:0] eth_ss_byte;
    logic       eth_ss_byte_ack;
    logic       eth_ss_idle;
    logic       eth_ss_stat_vld       /* synthesis syn_noprune=1 */;
    logic       eth_ss_cfg_err        /* synthesis syn_noprune=1 */;
    logic       eth_ss_programmed     /* synthesis syn_noprune=1 */;
    logic       eth_cfg_dout_high_z;
    logic       eth_cfg_dout;

    logic       adc_ss_byte_vld;
    logic [7:0] adc_ss_byte;
    logic       adc_ss_byte_ack;
    logic       adc_ss_idle;
    logic       adc_ss_stat_vld       /* synthesis syn_noprune=1 */;
    logic       adc_ss_cfg_err        /* synthesis syn_noprune=1 */;
    logic       adc_ss_programmed     /* synthesis syn_noprune=1 */;
    logic       adc_cfg_dout_high_z;
    logic       adc_cfg_dout;

    logic       dac_ss_byte_vld;
    logic [7:0] dac_ss_byte;
    logic       dac_ss_byte_ack;
    logic       dac_ss_idle;
    logic       dac_ss_stat_vld     /* synthesis syn_noprune=1 */;
    logic       dac_ss_cfg_err      /* synthesis syn_noprune=1 */;
    logic       dac_ss_programmed   /* synthesis syn_noprune=1 */;
    logic       dac_cfg_dout_high_z;
    logic       dac_cfg_dout;


    /* TI CHIP CONFIG SIGNALS */

    logic lmk04826_cfg_start;
    logic lmk04826_cfg_done     /* synthesis syn_noprune=1 */;
    logic lmk04826_sif_reset_n;
    logic lmk04133_cfg_start;
    logic lmk04133_cfg_done     /* synthesis syn_noprune=1 */;
    logic lmk04133_sif_sel_n;
    logic lmk04133_sif_clk;
    logic lmk04133_sif_dout;
    logic lmk04133_drive_le_low;
    logic lmk04133_goe;
    logic adc_cfg_start;
    logic adc_cfg_done          /* synthesis syn_noprune=1 */;
    logic adc_sif_reset_n;
    logic dac_cfg_start;
    logic dac_cfg_done          /* synthesis syn_noprune=1 */;
    logic dac_sif_reset_n;

    logic [1:0] dac_alarm_regs         /* synthesis syn_noprune=1 */;



    typedef enum {
        IDLE,
        CFG_LMK04826,
        WAIT_FOR_LMK04826_CFG,
        CFG_LMK04133,
        WAIT_FOR_LMK04133_CFG,
        PROGRAM_FPGAS,
        WAIT_FOR_FPGA_PROGRAMMING,
        CFG_ADC,
        WAIT_FOR_ADC_CFG,
        CFG_DAC,
        WAIT_FOR_DAC_CFG
    } STARTUP_FSM_STATES;

    STARTUP_FSM_STATES startup_fsm_state;

    typedef enum {
        DAC_ALARM_CLR_0,
        DAC_ALARM_CLR_1,
        DAC_ALARM_CLR_2,
        DAC_ALARM_CLR_3,
        DAC_ALARM_CLR_4,
        DAC_ALARM_CLR_5
    } DAC_ALARM_CLR_FSM_STATES;

    DAC_ALARM_CLR_FSM_STATES dac_alarm_clr_fsm_state;

    logic [7:0] dac_alarm_clr_delay_cntr;

    intf_cmd dac_cmd();
    intf_cmd adc_cmd();
    intf_cmd lmk04826_cmd();
    intf_cmd lmk04133_cmd();


/********************************************************************************/


    /*
     *
     * POST RESET CONFIGURATION FSM
     *
     */

    always_ff @(posedge int_osc_clk) begin: STARTUP_FSM

        if (int_osc_clk_srst) begin
            sif_cfg_done          <= 0;
            lmk04826_cfg_start    <= 0;
            lmk04133_cfg_start    <= 0;
            lmk04133_drive_le_low <= 0;
            adc_cfg_start         <= 0;
            dac_cfg_start         <= 0;
            fpga_ss_start         <= 0;
            lmk04133_goe          <= 0;
            startup_fsm_state     <= CFG_LMK04826;
        end else begin

            /* defaults */
            sif_cfg_done         <= 0;
            lmk04826_cfg_start   <= 0;
            lmk04133_cfg_start   <= 0;
            adc_cfg_start        <= 0;
            dac_cfg_start        <= 0;
            fpga_ss_start        <= 0;

            case (startup_fsm_state)

                CFG_LMK04826: begin
                    lmk04826_cfg_start <= 1;
                    startup_fsm_state  <= WAIT_FOR_LMK04826_CFG;
                end

                WAIT_FOR_LMK04826_CFG: begin
                    if (lmk04826_cfg_done) begin
                        startup_fsm_state <= CFG_LMK04133;
                    end
                end

                CFG_LMK04133: begin
                    lmk04133_cfg_start    <= 1;
                    lmk04133_drive_le_low <= 0; // allows programmer for LMK04133 to control the uWire LE line
                    startup_fsm_state     <= WAIT_FOR_LMK04133_CFG;
                end

                WAIT_FOR_LMK04133_CFG: begin
                    if (lmk04133_cfg_done) begin
                        lmk04133_drive_le_low <= 1; // forces uWire LE line low after programming, which is needed to latch in the last register written
                        lmk04133_goe          <= 1;
                        startup_fsm_state     <= PROGRAM_FPGAS;
                    end
                end

                PROGRAM_FPGAS: begin
                    fpga_ss_start     <= 1;
                    startup_fsm_state <= WAIT_FOR_FPGA_PROGRAMMING;
                end

                WAIT_FOR_FPGA_PROGRAMMING: begin
                    if (eth_ss_stat_vld & adc_ss_stat_vld & dac_ss_stat_vld) begin
                        startup_fsm_state <= CFG_ADC;
                    end
                end

                CFG_ADC: begin
                    adc_cfg_start     <= 1;
                    startup_fsm_state <= WAIT_FOR_ADC_CFG;
                end

                WAIT_FOR_ADC_CFG: begin
                    if (adc_cfg_done) begin
                        startup_fsm_state <= CFG_DAC;
                    end
                end

                CFG_DAC: begin
                    dac_cfg_start     <= 1;
                    startup_fsm_state <= WAIT_FOR_DAC_CFG;
                end

                WAIT_FOR_DAC_CFG: begin
                    if (dac_cfg_done) begin
                        startup_fsm_state <= IDLE;
                    end
                end

                IDLE: begin
                    sif_cfg_done      <= 1;
                    startup_fsm_state <= IDLE;
                end

                default: begin
                    startup_fsm_state <= IDLE;
                end

            endcase
        end
    end


    /*
     *
     * CONFIG FLASH READER
     *
     */

    localparam FLASH_ADDR_BITS         = 32; // N25Q/MT25Q parts larger than 128Mbit use 4-byte addresses, while 128Mbit and smaller use 3-byte addresses
    localparam FLASH_256MBIT_OR_LARGER = (FLASH_ADDR_BITS > 24) ? 1 : 0;
    localparam FPGA_CONFIG_BYTES       = 2_293_750;             // max LFE5U-85F bit file byte size
    localparam FLASH_RD_BYTES          = 3 * FPGA_CONFIG_BYTES; // 3 bit files (ADC, DAC, & ETH FPGAs)


    n25q_qspi_reader #(
        .P_256MBIT_OR_LARGER (FLASH_256MBIT_OR_LARGER  )
    )
    cfg_flash_reader
    (
        /* USER INTERFACE */

        .i_clk                (int_osc_clk),
        .i_srst               (int_osc_clk_srst),
        .i_rd_start           (fpga_ss_start),
        .i_rd_start_byte_addr (FPGA_CONFIG_BYTES),            // next byte after CFG FPGA bit file (i.e. the start of the interleaved ADC, DAC, and ETH FPGA bit files)
        .i_rd_num_bytes       (FLASH_RD_BYTES),
        .i_rd_pause           (cfg_flash_rd_pause),
        .o_rd_busy            (cfg_flash_rd_busy),
        .o_rd_byte_vld        (cfg_flash_rd_byte_vld),
        .o_rd_byte            (cfg_flash_rd_byte),

        /* QSPI INTERFACE */

        .o_qspi_clk       (cfg_spi_clk),
        .o_qspi_sel_n     (CFG_SPI_SN),
        .o_qspi_d0_high_z (cfg_spi_d0_high_z),
        .i_qspi_d3        (CFG_SPI_DQ3),
        .i_qspi_d2        (CFG_SPI_DQ2),
        .i_qspi_d1        (CFG_SPI_DQ1),
        .i_qspi_d0        (CFG_SPI_DQ0),
        .o_qspi_d0        (cfg_spi_dq0_out)

    );

    /* cfg flash reader dq0 tri-state */
    assign CFG_SPI_DQ0 = (~cfg_spi_d0_high_z) ? cfg_spi_dq0_out : 1'bz;

    /* MACRO to provided user logic access to BANK8 MCLK pin */
    logic       usrmclk_ts;

    assign usrmclk_ts = int_osc_clk_srst;

    USRMCLK u1 (.USRMCLKI(cfg_spi_clk), .USRMCLKTS(usrmclk_ts)) /* synthesis syn_noprune=1 */;


    /*
     *
     *  SLAVE SERIAL PROGRAMMERS
     *
     */

    // ETHERNET FPGA (U19A)
    ecp5_slave_serial_programmer #(
        .P_CONFIG_BYTES(FPGA_CONFIG_BYTES)
    )
    eth_ss
    (
        /* USER INTERFACE */

        .i_clk             (int_osc_clk),
        .i_srst            (int_osc_clk_srst),
        .i_start           (fpga_ss_start),
        .i_byte_vld        (eth_ss_byte_vld),
        .i_byte            (eth_ss_byte),
        .o_byte_ack        (eth_ss_byte_ack),
        .o_idle            (eth_ss_idle),
        .o_fpga_status_vld (eth_ss_stat_vld),
        .o_fpga_cfg_err    (eth_ss_cfg_err),
        .o_fpga_programmed (eth_ss_programmed),

        /* FPGA PROGRAMMING INTERFACE */

        .i_init_n          (ETH_CFG_INITN),
        .i_done            (ETH_CFG_DONE),
        .o_mclk            (ETH_CFG_CCLK),
        .o_prog_n          (ETH_CFG_PROGN),
        .o_dout_high_z     (eth_cfg_dout_high_z),
        .o_dout            (eth_cfg_dout)

    );

    // ADC FPGA (U19B)
    ecp5_slave_serial_programmer #(
        .P_CONFIG_BYTES(FPGA_CONFIG_BYTES)
    )
    adc_ss
    (
        /* USER INTERFACE */

        .i_clk             (int_osc_clk),
        .i_srst            (int_osc_clk_srst),
        .i_start           (fpga_ss_start),
        .i_byte_vld        (adc_ss_byte_vld),
        .i_byte            (adc_ss_byte),
        .o_byte_ack        (adc_ss_byte_ack),
        .o_idle            (adc_ss_idle),
        .o_fpga_status_vld (adc_ss_stat_vld),
        .o_fpga_cfg_err    (adc_ss_cfg_err),
        .o_fpga_programmed (adc_ss_programmed),

        /* FPGA PROGRAMMING INTERFACE */

        .i_init_n          (ADC_CFG_INITN),
        .i_done            (ADC_CFG_DONE),
        .o_mclk            (ADC_CFG_CCLK),
        .o_prog_n          (ADC_CFG_PROGN),
        .o_dout_high_z     (adc_cfg_dout_high_z),
        .o_dout            (adc_cfg_dout)

    );

    // DAC FPGA (U19C)
    ecp5_slave_serial_programmer #(
        .P_CONFIG_BYTES(FPGA_CONFIG_BYTES)
    )
    dac_ss
    (
        /* USER INTERFACE */

        .i_clk             (int_osc_clk),
        .i_srst            (int_osc_clk_srst),
        .i_start           (fpga_ss_start),
        .i_byte_vld        (dac_ss_byte_vld),
        .i_byte            (dac_ss_byte),
        .o_byte_ack        (dac_ss_byte_ack),
        .o_idle            (dac_ss_idle),
        .o_fpga_status_vld (dac_ss_stat_vld),
        .o_fpga_cfg_err    (dac_ss_cfg_err),
        .o_fpga_programmed (dac_ss_programmed),

        /* FPGA PROGRAMMING INTERFACE */

        .i_init_n          (DAC_CFG_INITN),
        .i_done            (DAC_CFG_DONE),
        .o_mclk            (DAC_CFG_CCLK),
        .o_prog_n          (DAC_CFG_PROGN),
        .o_dout_high_z     (dac_cfg_dout_high_z),
        .o_dout            (dac_cfg_dout)

    );

    /* tri-states for slave serial programmer douts */
    assign ETH_CFG_DI = (~eth_cfg_dout_high_z) ? eth_cfg_dout : 1'bz;
    assign ADC_CFG_DI = (~adc_cfg_dout_high_z) ? adc_cfg_dout : 1'bz;
    assign DAC_CFG_DI = (~dac_cfg_dout_high_z) ? dac_cfg_dout : 1'bz;


    /*
     *
     * SLAVE SERIAL PROGRAMMER BYTE ROUTING & FLOW CONTROL
     *
     * It's expected that the first byte read from the config flash is for the Ethernet FPGA, the next byte is for the ADC FPGA, and the next byte is for the DAC FPGA.
     * Subsequent bytes repeat this pattern: ETH, ADC, DAC, ETH, ADC, DAC...
     *
     */

    always_ff @(posedge int_osc_clk) begin

        /* defaults */
        eth_ss_byte_vld <= 0;
        adc_ss_byte_vld <= 0;
        dac_ss_byte_vld <= 0;

        if (int_osc_clk_srst | ~cfg_flash_rd_busy) begin
            cfg_flash_rd_byte_dest <= ETH_FPGA;
        end
        else if (cfg_flash_rd_byte_vld) begin

            if (eth_ss_byte_ack | adc_ss_byte_ack | dac_ss_byte_ack) begin
                case (cfg_flash_rd_byte_dest)
                    ETH_FPGA: cfg_flash_rd_byte_dest <= ADC_FPGA;
                    ADC_FPGA: cfg_flash_rd_byte_dest <= DAC_FPGA;
                    DAC_FPGA: cfg_flash_rd_byte_dest <= ETH_FPGA;
                endcase
            end

            case (cfg_flash_rd_byte_dest)
                ETH_FPGA: begin
                    eth_ss_byte     <= cfg_flash_rd_byte;
                    eth_ss_byte_vld <= 1;
                end
                ADC_FPGA: begin
                    adc_ss_byte     <= cfg_flash_rd_byte;
                    adc_ss_byte_vld <= 1;
                end
                DAC_FPGA: begin
                    dac_ss_byte     <= cfg_flash_rd_byte;
                    dac_ss_byte_vld <= 1;
                end
            endcase
        end
    end

    assign cfg_flash_rd_pause = (~eth_ss_byte_ack) & (~adc_ss_byte_ack) & (~dac_ss_byte_ack);

`endif
    /*
     *
     * TI CLOCK CHIP CONFIG (LMK04826B)
     *
     */

`ifndef VERILATE_DEF
    localparam TI_CFG_SYSCLK_DIV = 5;                     // SET THIS SO THAT int_osc_clk divided by this is less than 10MHz!

    graviton_ti_cfg #(
        .P_SYSCLK_DIV     (TI_CFG_SYSCLK_DIV),
        .P_SIF_ADDR_BITS  (15),
        .P_SIF_DATA_BITS  (8),
        .P_SIF_RD_WR_BITS (1),
        .P_CFG_ROM_WORDS  (115),
        .P_CFG_ROM_FILE   ("../ti_cfg_rom_files/lmk04826_cfg_rom_ti_code_loader.hex")
    ) lmk04826_cfg (

        .i_sysclk      (int_osc_clk),
        .i_srst        (int_osc_clk_srst),
        //.cmd           (lmk04826_cmd),
        .i_cfg_start   (lmk04826_cfg_start),
        .o_cfg_done    (lmk04826_cfg_done),
        .o_sif_reset_n (lmk04826_sif_reset_n),
        .o_sif_sclk    (CLK_CTRL_SCK),
        .o_sif_sel_n   (CLK_CTRL_CSN),
        .o_sif_sdout   (CLK_CTRL_SDIO)
    );

    assign CLK_CTRL_RESET = ~lmk04826_sif_reset_n; // clock chip reset is active high
    assign CLK_CTRL_SEL0  = 0;
    assign CLK_CTRL_SEL1  = 0;
    assign CLK_CTRL_SYNC  = 0; // sync comes via SIF register config by toggling SPI_POL from 0 to 1 to 0 (must also have sync_en = 1, sync_mode = 1 in config registers)
    
    assign lmk04826_cmd.sel = 0;


    /*
     *
     * TI CLOCK SYNTH CONFIG (LMK04133)
     *
     */

    graviton_ti_cfg #(
        .P_SYSCLK_DIV     (TI_CFG_SYSCLK_DIV),
        .P_SIF_ADDR_BITS  (4),
        .P_SIF_DATA_BITS  (28),
        .P_SIF_RD_WR_BITS (0),
        .P_CFG_ROM_WORDS  (13),
`ifdef LMK04133_CFG_PATH
        .P_CFG_ROM_FILE   (`LMK04133_CFG_PATH)
`else
        .P_CFG_ROM_FILE   ("../ti_cfg_rom_files/lmk04133_cfg_rom_ti_code_loader.hex")
`endif
    ) lmk04133_cfg (

        .i_sysclk      (int_osc_clk),
        .i_srst        (int_osc_clk_srst),
        //.cmd           (lmk04133_cmd),
        .i_cfg_start   (lmk04133_cfg_start),
        .o_cfg_done    (lmk04133_cfg_done),
        .o_sif_reset_n (),
        .o_sif_sclk    (lmk04133_sif_clk),
        .o_sif_sel_n   (lmk04133_sif_sel_n),
        .o_sif_sdout   (lmk04133_sif_dout)
    );
    
    assign lmk04133_cmd.sel = 0;

    always_ff @(posedge int_osc_clk) begin
        SYNTH_CTRL_CLK  <= lmk04133_sif_clk;
        SYNTH_CTRL_DATA <= lmk04133_sif_dout;
        SYNTH_CTRL_LE   <= lmk04133_drive_le_low ? 1'b0 : lmk04133_sif_sel_n; // needed in order to drive uWire LE line low after last register write, but still allow for the LMK04133 config module to control the LE line the rest of the time
        SYNTH_CTRL_GOE  <= lmk04133_goe;
    end


    /*
     *
     * TI ADC CONFIG
     *
     */

    graviton_ti_cfg #(
        .P_SYSCLK_DIV     (TI_CFG_SYSCLK_DIV),
        .P_SIF_ADDR_BITS  (7),
        .P_SIF_DATA_BITS  (8),
        .P_SIF_RD_WR_BITS (1),
        .P_CFG_ROM_WORDS  (4),
        .P_CFG_ROM_FILE   ("../ti_cfg_rom_files/adc_cfg_rom.hex")
    ) adc_cfg (
        .i_sysclk      (int_osc_clk),
        .i_srst        (int_osc_clk_srst),
        //.cmd           (adc_cmd),
        .i_cfg_start   (adc_cfg_start),
        .o_cfg_done    (adc_cfg_done),
        .o_sif_reset_n (adc_sif_reset_n),
        .o_sif_sclk    (ADC_CTRL_SCLK),
        .o_sif_sel_n   (ADC_CTRL_SEN),
        .o_sif_sdout   (ADC_CTRL_SDATA)
    );

    assign ADC_CTRL_RESET = ~adc_sif_reset_n;
    
    assign adc_cmd.sel = 0;



    /*
     *
     * TI DAC CONFIG
     *
     */

    logic [23:0] l2h_rf2_dac_cfg_setting_dac_cfg_setting_r;
    /*graviton_ti_cfg #(
        .P_SYSCLK_DIV     (TI_CFG_SYSCLK_DIV),
        .P_SIF_ADDR_BITS  (7),
        .P_SIF_DATA_BITS  (16),
        .P_SIF_RD_WR_BITS (1),
        .P_CFG_ROM_WORDS  (8),
        .P_CFG_ROM_FILE   ("../ti_cfg_rom_files/dac_cfg_rom.hex")
    ) dac_cfg (

        .i_sysclk      (int_osc_clk),
        .i_srst        (int_osc_clk_srst),
        //.cmd           (dac_cmd),
        .i_cmd_sif_addr_reg (l2h_rf2_dac_cfg_setting_dac_cfg_setting_r[23:16]),
        .i_cmd_sif_wdata_reg (l2h_rf2_dac_cfg_setting_dac_cfg_setting_r[15:0]),         
        .i_cfg_start   (dac_cfg_start),
        .o_cfg_done    (dac_cfg_done),
        .o_sif_reset_n (dac_sif_reset_n),
        .o_sif_sclk    (DAC_CTRL_SCLK),
        .o_sif_sel_n   (DAC_CTRL_SDENN),
        .o_sif_sdout   (DAC_CTRL_SDIO)
    );*/


    always_ff @(posedge sys_clk) begin
    	DAC_CTRL_SDIO	<= MIB_AD[20];
    	DAC_CTRL_SDENN  <= MIB_AD[19];
    	DAC_CTRL_SCLK   <= MIB_AD[18];
    	DAC_CTRL_RESETN <= MIB_AD[17];
    end
    
    
    assign DAC_CTRL_TXENABLE = 0;
    assign DAC_CTRL_SLEEP    = 0;

    assign dac_cmd.sel = 0;

    always_ff @(posedge int_osc_clk) begin
        dac_alarm_regs <= {dac_alarm_regs[0], DAC_CTRL_ALARM};
    end
    
    
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
    logic        h2l_rf1_adc_cfg_stat_vld_w;
    logic        h2l_rf1_adc_cfg_err_w;
    logic        h2l_rf1_adc_cfg_done_w;
    logic        h2l_rf1_dac_cfg_stat_vld_w;
    logic        h2l_rf1_dac_cfg_err_w;
    logic        h2l_rf1_dac_cfg_done_w;
    logic        h2l_rf1_eth_cfg_stat_vld_w;
    logic        h2l_rf1_eth_cfg_err_w;
    logic        h2l_rf1_eth_cfg_done_w;
    
    logic        l2h_rf0_soft_rst_strb_r;
    logic        l2h_rf0_soft_cntr_lock_strb_r;
    
    
    assign h2l_rf0_fpga_uid_uid_w     = FPGA_UID;
    assign h2l_rf1_adc_cfg_stat_vld_w = adc_ss_stat_vld;
    assign h2l_rf1_adc_cfg_err_w      = adc_ss_cfg_err;
    assign h2l_rf1_adc_cfg_done_w     = adc_ss_programmed;
    assign h2l_rf1_dac_cfg_stat_vld_w = dac_ss_stat_vld;
    assign h2l_rf1_dac_cfg_err_w      = dac_ss_cfg_err;
    assign h2l_rf1_dac_cfg_done_w     = dac_ss_programmed;
    assign h2l_rf1_eth_cfg_stat_vld_w = eth_ss_stat_vld;
    assign h2l_rf1_eth_cfg_err_w      = eth_ss_cfg_err;
    assign h2l_rf1_eth_cfg_done_w     = eth_ss_programmed;

    assign soft_mib_rst_strb         = l2h_rf0_soft_rst_strb_r;
    assign soft_mib_cntr_lock_strb   = l2h_rf0_soft_cntr_lock_strb_r;

    /*REGS_pio regs (
        .*,
        .clk                    (sys_clk), 
        .reset                  (sys_clk_srst), 
        .h2d_pio_dec_address    (cmd_sys.byte_addr[CMD_ADDR_BITS-1:2]), // dword addressing so bits [1:0] are always 0 
        .h2d_pio_dec_write_data (cmd_sys.wdata),
        .h2d_pio_dec_write      (cmd_sys.sel & ~cmd_sys.rd_wr_n),
        .h2d_pio_dec_read       (cmd_sys.sel & cmd_sys.rd_wr_n),
        .d2h_dec_pio_read_data  (cmd_sys.rdata),
        .d2h_dec_pio_ack        (cmd_sys.ack),
        .d2h_dec_pio_nack       ());*/
`endif
    
endmodule


`default_nettype wire
    

// OLD JUNK CODE BELOW HERE
/********************************************************************************/

    /* This logic monitors DAC alarms and clears the DAC alarm and IO Test Bits.
     * This logic should ultimately be replaced by ARM MCU control down the road, but I need something to
     * avoid ridiculous kludges for getting the DAC IO Test to work (see DAC datasheet).
     */

//    always_ff @(posedge int_osc_clk) begin
//
//        if (int_osc_clk_srst) begin
//            dac_alarm_clr_fsm_state <= DAC_ALARM_CLR_0;
//            dac_cmd.sel             <= 0;
//        end else begin
//
//            /* defaults */
//            dac_cmd.sel <= 0;
//
//            case (dac_alarm_clr_fsm_state)
//
//                DAC_ALARM_CLR_0: begin
//                    dac_alarm_clr_delay_cntr <= 0;
//                    if (dac_alarm_regs[1] & dac_cfg_done) begin
//                        dac_cmd.sel             <= 1;
//                        dac_cmd.rd_wr_n         <= 0;
//                        dac_cmd.byte_addr       <= {16'h00000, 8'h10};  // dac config4
//                        dac_cmd.wdata           <= {16'h0000, 16'h0000}; // clear IO Test results
//                        dac_alarm_clr_fsm_state <= DAC_ALARM_CLR_1;
//                    end
//                end
//
//                DAC_ALARM_CLR_1: begin
//                    if (dac_cmd.ack) begin
//                        dac_cmd.sel             <= 1;
//                        dac_cmd.rd_wr_n         <= 0;
//                        dac_cmd.byte_addr       <= {16'h0000, 8'h1c};  // dac config7
//                        dac_cmd.wdata           <= {16'h0000, 16'hffff};  // mask alarms
//                        dac_alarm_clr_fsm_state <= DAC_ALARM_CLR_2;
//                    end
//                end
//
//                DAC_ALARM_CLR_2: begin
//                    if (dac_cmd.ack) begin
//                        dac_cmd.sel             <= 1;
//                        dac_cmd.rd_wr_n         <= 0;
//                        dac_cmd.byte_addr       <= {16'h0000, 8'h14};  // dac config5
//                        dac_cmd.wdata           <= {16'h0000, 16'h0000};  // clear alarms
//                        dac_alarm_clr_fsm_state <= DAC_ALARM_CLR_3;
//                    end
//                end
//
//                DAC_ALARM_CLR_3: begin
//                    if (dac_cmd.ack) begin
//                        dac_cmd.sel             <= 1;
//                        dac_cmd.rd_wr_n         <= 0;
//                        dac_cmd.byte_addr       <= {16'h0000, 8'h1c};  // dac config7
//                        dac_cmd.wdata           <= {16'h0000, 16'hff7f};  // unmask alarm_from_iotest
//                        dac_alarm_clr_fsm_state <= DAC_ALARM_CLR_4;
//                    end
//                end
//
//                DAC_ALARM_CLR_4: begin
//                    if (dac_cmd.ack) begin
//                        dac_alarm_clr_delay_cntr <= dac_alarm_clr_delay_cntr + 1;
//                        dac_alarm_clr_fsm_state  <= DAC_ALARM_CLR_5;
//                    end
//                end
//
//                DAC_ALARM_CLR_5: begin
//                    dac_alarm_clr_delay_cntr <= dac_alarm_clr_delay_cntr + 1;
//
//                    if (dac_alarm_clr_delay_cntr == 0) begin
//                        dac_alarm_clr_fsm_state <= DAC_ALARM_CLR_0;
//                    end
//                end
//            endcase
//        end
//    end
