/****************************************************************************
 * core_top.sv
 ****************************************************************************/

/**
 * Module: core_top
 * 
 * 
 * Encapsulates blocks that are found in and common to pretty much all FPGAs on Graviton and Copper Suicide 
 * (e.g. clocking, reset generation, and MIB slave)
 * 
 */
 

`include "udp_cmd_pkg.sv"
 
`default_nettype none

`define OMIT_MIB

module core_top #(
        
    parameter int       CLOCK_SHIFT_TRAINING_COUNTER_LIMIT                 = 100,    // (ask Anurag about what this does if confused)
    parameter int       NUM_SYS_CLK_SRSTS                                  = 1,      // sets the number of system clock synchronous reset created (the only reason to do this is if you want to release different modules from reset at different times)
    parameter int       SYS_CLK_SRSTS_EXTRA_CLOCKS [0:NUM_SYS_CLK_SRSTS-1] = {NUM_SYS_CLK_SRSTS{1'b0}},   // use this to specify how many extra system clocks to hold each system clock synchronous reset after the FPGA external reset (from pin) is released.  This allows different modules to be brought out of reset at different times.
    parameter int       NUM_MIB_CLK_SRSTS                                  = 1,      // same as NUM_SYS_CLK_SRSTS, but for MIB bus clock domain (probably will only ever need 1).
    parameter int       MIB_CLK_SRSTS_EXTRA_CLOCKS [0:NUM_MIB_CLK_SRSTS-1] = {NUM_SYS_CLK_SRSTS{1'b0}}, // same as SYS_CLK_SRSTS_EXTRA_CLOCKS, but for MIB bus clock domain (probably won't ever need to change from default).
    parameter int       INT_OSC_DIV_VAL                                    = 12,     // Divider value for Lattice FPGA internal oscillator (12 results in a 25.8MHz +/- 20% clock, see Lattice Documentation for more detail)
    parameter int       NUM_INT_OSC_SRST_CLOCKS                            = 128,    // number of internal oscillator clock cycles to assert o_int_osc_clk_srst for after FPGA configuration
    parameter bit [3:0] MIB_SLAVE_ADDR_MSN                                 = 4'h0,   // Set this to the MIB Most-Significant-Nibble used to uniquely identify the FPGA this module is in on the MIB bus
    parameter bit       MIB_CLOCK_DESKEW_ENABLE                            = 1'b1,   // Controls whether or not MIB clock deskewing should be done (YOU DEFINITELY SHOULD SET THIS TO 1'b1 IN THE FPGA CONTAINING THE MIB MASTER) 
    parameter bit       INCLUDE_MIB_SLAVE                                  = 1'b1,   // Set to 1'b0 in the FPGA containing the MIB Master
    parameter bit		VERILATE										   = 1'b0 	 // Set to 1'b1 during running verilator.   
) (

    input  wire logic        i_fpga_clk,           // FPGA clock (from FPGA pin)
    input  wire logic        i_fpga_ext_arst,      // external async reset (from FPGA pin)
    output wire logic        o_int_osc_clk,
    output wire logic        o_int_osc_clk_srst,
    output wire logic        o_sys_clk,
    output wire logic        o_sys_clk_srsts,// [0:NUM_SYS_CLK_SRSTS-1],
    output wire logic        o_mib_clk,
    output wire logic        o_mib_clk_srsts,// [0:NUM_MIB_CLK_SRSTS-1],
    output wire logic        o_mib_deskew_done,
    output wire logic        o_sys_pll_locked,
    input  wire logic        i_mib_tbit,          // MIB toggle bit (for MIB bus deskew)
    input  wire logic        i_mib_start,
    input  wire logic        i_mib_rd_wr_n,
    inout  wire logic [15:0] b_mib_ad,
    output wire logic        o_mib_slave_ack,
    intf_cmd.master          cmd_sys // connect this to your REGS_pio block created by the ORDT using the regs.rdl input (synchronous to o_sys_clk)

);

    /*
     * 
     * CLOCKING
     * 
     */
    
    
    localparam INT_OSC_CLK_SRST_CNTR_BITS = $clog2(NUM_INT_OSC_SRST_CLOCKS);

    logic [INT_OSC_CLK_SRST_CNTR_BITS-1:0] int_osc_clk_srst_cntr = '0;
    logic                                  int_osc_clk_srst      = 1'b1;
    logic                                  sys_pll_locked;
    logic                                  sys_pll_not_locked;
    logic                                  sys_pll_phase_dir;
    logic                                  sys_pll_phase_step;


    // FPGA internal oscillator (mainly needed in both Graviton and Copper Suicide CFG FPGAs to FPGA SPI programming of external ICs and FPGA slave serial programming)
    generate if (!VERILATE) begin

    OSCG #(
        .DIV(INT_OSC_DIV_VAL)
    ) oscg_inst (
        .OSC(o_int_osc_clk));
    end else begin
        assign o_int_osc_clk = i_fpga_clk; // just assign, only used on eth anyways
    end
endgenerate
    

    // post configuration reset
    always_ff @(posedge o_int_osc_clk) begin
        if (int_osc_clk_srst_cntr != (NUM_INT_OSC_SRST_CLOCKS-1)) begin
            int_osc_clk_srst <= 1;
            int_osc_clk_srst_cntr <= int_osc_clk_srst_cntr + 1'b1;
        end else begin
            int_osc_clk_srst <= 0;
        end
    end
    
    assign o_int_osc_clk_srst = int_osc_clk_srst;
    

    generate
    	if (VERILATE) begin
    		assign o_sys_clk = i_fpga_clk;
    		assign o_mib_clk = i_fpga_clk;
    		logic [3:0] counter = 0;
    		logic temp = 0;
    		always_ff @(posedge o_sys_clk) begin
    			counter <= counter + 1;
    			if (counter == {4{1'b1}}) begin
    				counter <= counter;
    				temp <= 1;
    			end
    		end
    		
    		assign sys_pll_locked = temp;
    	end else if (MIB_CLOCK_DESKEW_ENABLE == 1'b1) begin

            sys_pll sys_pll (
                .CLKI         (i_fpga_clk), 
                .CLKOP        (o_sys_clk), 
                .CLKOS        (o_mib_clk), 
                .PHASESEL     (2'b00), // selects clkos2 for phase shifting
                .PHASEDIR     (sys_pll_phase_dir), 
                .PHASESTEP    (sys_pll_phase_step), 
                .PHASELOADREG (1'b0), 
                .LOCK         (sys_pll_locked));
            
        end else begin

            sys_pll sys_pll (
                .CLKI         (i_fpga_clk), 
                .CLKOP        (o_sys_clk), 
`ifndef OMIT_MIB
                .CLKOS        (o_mib_clk), 
`endif
                .PHASESEL     (2'b00), // selects clkos2 for phase shifting
                .PHASEDIR     (1'b0), 
                .PHASESTEP    (1'b0), 
                .PHASELOADREG (1'b0), 
                .LOCK         (sys_pll_locked));
        end
    endgenerate

    assign sys_pll_not_locked = ~sys_pll_locked;
    assign o_sys_pll_locked   = sys_pll_locked;
    
    /*
     * 
     * RESETS
     * 
     */

    // DON'T FORGET TO UPDATE MAXDELAY CONSTRAINTS IN .lpf FILE IF YOU CHANGE THE NAMES OF THESE OR ADD MORE
   generate
    if (VERILATE) begin
        assign o_sys_clk_srsts = i_fpga_ext_arst;
    end else begin
    core_reset #(
    		.NUM_OUTPUTS        (NUM_SYS_CLK_SRSTS),
    		.EXTRA_RESET_CLOCKS (SYS_CLK_SRSTS_EXTRA_CLOCKS)
    	) sys_clk_resets (
    		.i_ext_arst         (i_fpga_ext_arst), 
    		.i_clk              (o_sys_clk),
    		.i_clk_pll_unlocked (sys_pll_not_locked),
    		.o_sync_resets      (o_sys_clk_srsts));
`ifndef OMIT_MIB
    core_reset #(
    		.NUM_OUTPUTS        (1),
    		.EXTRA_RESET_CLOCKS (SYS_CLK_SRSTS_EXTRA_CLOCKS)
    	) mib_clk_resets (
    		.i_ext_arst         (1'b0), // only reset once after sys_pll locks
    		.i_clk              (o_mib_clk),
    		.i_clk_pll_unlocked (sys_pll_not_locked),
    				.o_sync_resets      (o_mib_clk_srsts));
`endif
end 
endgenerate
    
    /*
     * 
     * MIB SLAVE
     * 
     */
    
    // clock deskew
    generate
        
        if (MIB_CLOCK_DESKEW_ENABLE == 1'b1) begin

            // used to deskew the clock used for the MIB Slave in this FPGA and the clock used for the MIB Master in some other FPGA
            clock_shift #(
                .TRAINING_COUNTER_LIMIT  (CLOCK_SHIFT_TRAINING_COUNTER_LIMIT)
                ) clock_shift (
                .i_training_bit          (i_mib_tbit), 
                .i_traininig_clk         (o_mib_clk), 
                .o_pll_PHASESTEP         (sys_pll_phase_step), 
                .o_pll_PHASEDIR          (sys_pll_phase_dir), 
                .i_pll_lock              (sys_pll_locked), 
                .i_start                 (1'b1), 
                .i_reset_p               (o_mib_clk_srsts), 
                .o_done                  (o_mib_deskew_done));
            
        end else begin

            assign o_mib_deskew_done = 1'b1;

        end

    endgenerate
    
    // mib slave
    generate 

    	if ((INCLUDE_MIB_SLAVE == 1'b1) && !VERILATE) begin

    		intf_cmd #(MIB_ADDR_BITS, CMD_DATA_BITS) cmd_mib(); // parameters specified in udp_cmd_pkg.sv

    		logic        mib_ack_int;
    		logic        mib_ack_int_high_z;
    		logic [15:0] mib_ad_int;
    		logic        mib_ad_int_high_z;
    		logic        mib_ack_int_reg;
    		logic        mib_ack_int_high_z_reg;
    		logic [15:0] mib_ad_int_reg;
    		logic        mib_ad_int_high_z_reg;
    		logic [15:0] mib_ad_in_reg;
    		logic        mib_start_in_reg;
    		logic        mib_rd_wr_n_in_reg;
    
    		// just to help with timing
    		always_ff @(posedge o_mib_clk) begin
    			mib_ad_in_reg      <= b_mib_ad;    
    			mib_start_in_reg   <= i_mib_start;  
    			mib_rd_wr_n_in_reg <= i_mib_rd_wr_n;
    		end

    		mib_slave #(
    				.P_SLAVE_MIB_ADDR_MSN   (MIB_SLAVE_ADDR_MSN   ),
    				.P_CMD_ACK_TIMEOUT_CLKS (CMD_MASTER_ACK_TIMEOUT_CLKS) // specified in udp_cmd_pkg.sv
    			) mib_slave (
    				.i_sysclk               (o_mib_clk          ), 
    				.i_srst                 (o_mib_clk_srsts ),
    				.cmd_master             (cmd_mib            ),
    				.i_mib_start            (mib_start_in_reg   ),
    				.i_mib_rd_wr_n          (mib_rd_wr_n_in_reg ),
    				.o_mib_slave_ack        (mib_ack_int        ),
    				.o_mib_slave_ack_high_z (mib_ack_int_high_z ),
    				.o_mib_ad               (mib_ad_int         ),
    				.i_mib_ad               (mib_ad_in_reg      ),
    				.o_mib_ad_high_z        (mib_ad_int_high_z  )
    
    			);
    
    		always_ff @(posedge o_mib_clk) begin
    			mib_ack_int_reg        <= mib_ack_int;
    			mib_ack_int_high_z_reg <= mib_ack_int_high_z;
    			mib_ad_int_reg         <= mib_ad_int;
    			mib_ad_int_high_z_reg  <= mib_ad_int_high_z;
    		end
    
    		assign o_mib_slave_ack = (mib_ack_int_high_z_reg) ? 1'bz                    : mib_ack_int_reg;
    		assign b_mib_ad        = (mib_ad_int_high_z_reg)  ? 16'bzzzz_zzzz_zzzz_zzzz : mib_ad_int_reg;

    		mib_cdc #(
    				.ADDR_BITS (MIB_ADDR_BITS),
    				.DATA_BITS (CMD_DATA_BITS),
    				.SIM_MODE (0),
    				.VERILATE (VERILATE)
    			) mib_cdc (        
    				.i_sys_clk  (o_sys_clk),
    				.i_sys_srst (o_sys_clk_srsts),
    				.i_mib_clk  (o_mib_clk),
    				.i_mib_srst (o_mib_clk_srsts),
    				.cmd_sys    (cmd_sys),
    				.cmd_mib    (cmd_mib));
                
        end
            
    endgenerate

endmodule

`default_nettype wire

