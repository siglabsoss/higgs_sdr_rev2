`include "configurable_mif_paths.v"


module tb_higgs_top (
		input wire 			clk,
		input wire 			MIB_MASTER_RESET,
		
		
		
		input wire [31:0] 	i_data_adc,
		input wire 			i_data_valid_adc,
		
		output wire [31:0] 	o_data_dac,
		output wire 		o_data_valid_dac,
		
		
		
		input wire 			i_o_ready_dac,

		// port 30000
		input wire [31:0] tx_turnstile_data_in,
    	input wire        tx_turnstile_data_valid,

    	// port 20000
    	input wire [31:0] ringbus_in_data,
    	input wire        ringbus_in_data_vld,

    	// port 10000
    	output wire [31:0] ringbus_out_data,
    	output wire        ringbus_out_data_vld,

    	input wire        ring_bus_i0_ready,

    	output wire [31:0] cs20_out_data,
    	output wire        cs20_o_data_vld,
		input wire		   cs20_i_ready,

    	// 
    	output wire [31:0] 	o_data_eth,
		output wire 		o_data_valid_eth,

		output  	      DAC_CTRL_SDIO, // technically an inout, but we currently don't support reads from DAC SIF
    	output  	      DAC_CTRL_SDENN,
    	output  	     DAC_CTRL_SCLK,
    	output  	     DAC_CTRL_RESETN,

    	input logic cs20_i_ringbus,

    	// snap to edge
    	output logic snap_cs20_o_ringbus,
    	output logic snap_cs10_o_ringbus,

    	input logic eth_rst

		);


parameter VERILATE = 1'b1;

	logic [31:0] cs20_out_data;
	logic		 cs20_o_data_vld;
	logic 		 cs20_i_ready;
	logic 		 cs20_o_ringbus;
	
	logic 		 cs10_o_ringbus;
		
	logic [31:0] cs00_out_data;
	logic		 cs00_o_data_vld;
	logic 		 cs00_i_ready;
	logic 		 cs00_o_ringbus;
	
	logic [31:0] cs01_out_data;
	logic		 cs01_o_data_vld;
	logic 		 cs01_i_ready;
	logic 		 cs01_o_ringbus;
	
	logic [31:0] cs11_out_data;
	logic		 cs11_o_data_vld;
	logic 		 cs11_i_ready;
	logic 		 cs11_o_ringbus;
	
	logic [31:0] cs21_out_data;
	logic		 cs21_o_data_vld;
	logic 		 cs21_i_ready;
	logic 		 cs21_o_ringbus;
	
	logic [31:0] cs31_out_data;
	logic		 cs31_o_data_vld;
	logic 		 cs31_i_ready;
	logic 		 cs31_o_ringbus;
	
	logic [31:0] 	i_data_eth;
	logic 			i_data_valid_eth;
	logic 			o_i_ready_eth;

	// logic [31:0] 	o_data_eth;
	// logic 		o_data_valid_eth;

	logic 			i_o_ready_eth;

	logic 			eth_rst;

	logic 	[20:0] MIB_AD;

	logic o_ringbus;
	// logic i_ringbus;

	// logic snap_cs20_o_ringbus;
	assign snap_cs20_o_ringbus = cs20_o_ringbus;
	assign snap_cs10_o_ringbus = cs10_o_ringbus;


`ifdef TB_USE_CS20
// assign eth_rst = MIB_MASTER_RESET;
	cs20_top #(.VERILATE (VERILATE),
			   .SCALAR_MEM_0 (`CS20_SCALAR_0),
       		   .SCALAR_MEM_1 (`CS20_SCALAR_1),
       		   .SCALAR_MEM_2 (`CS20_SCALAR_2),
       		   .SCALAR_MEM_3 (`CS20_SCALAR_3))
		cs20_top (
			.CLK				(clk),
			.FPGA_LED			(),
			.HS_NORTH_OUT 		({cs20_o_data_vld,cs20_out_data}), // data valid + 32 bit data
			.HS_NORTH_IN		(cs20_i_ready), // ready
			.LS_WEST_IN			(eth_rst), // reset coming from eth
			.HS_WEST_in			(cs20_i_ringbus), // i_ringbus
			.HS_NORTH_out		(cs20_o_ringbus), // o_ringbus
			.HS_WEST_IN			({i_data_valid_eth,i_data_eth}), // data valid + 32 bit data
			.HS_WEST_OUT		(o_i_ready_eth),	// ready
			.MIB_MASTER_RESET	(MIB_MASTER_RESET)
		);
`endif
	
`ifdef TB_USE_CS10
	cs10_top #(.VERILATE (VERILATE),
				.SCALAR_MEM_0 (`CS10_SCALAR_0),
       		    .SCALAR_MEM_1 (`CS10_SCALAR_1),
       		    .SCALAR_MEM_2 (`CS10_SCALAR_2),
       		    .SCALAR_MEM_3 (`CS10_SCALAR_3))
		cs10_top (
			.CLK				(clk),
			.FPGA_LED			(),
			.HS_WEST_OUT 		({o_data_valid_dac,o_data_dac}), // data valid + 32 bit data
			.HS_WEST_IN			(i_o_ready_dac), // ready
			.HS_SOUTH_in		(cs20_o_ringbus), // i_ringbus
			.HS_NORTH_out		(cs10_o_ringbus), // o_ringbus
			.HS_SOUTH_IN		({cs20_o_data_vld,cs20_out_data}), // data valid + 32 bit data
			.HS_SOUTH_OUT		(),	// ready cs20_i_ready
			.MIB_MASTER_RESET	(MIB_MASTER_RESET)
		);
`endif
`ifdef TB_USE_CS00
	cs00_top #(.VERILATE (VERILATE),
			.SCALAR_MEM_0 (`CS00_SCALAR_0),
			.SCALAR_MEM_1 (`CS00_SCALAR_1),
			.SCALAR_MEM_2 (`CS00_SCALAR_2),
			.SCALAR_MEM_3 (`CS00_SCALAR_3))
		cs00_top (
			.CLK				(clk),
			.FPGA_LED			(),
			.HS_EAST_OUT 		({cs00_o_data_vld,cs00_out_data}), // data valid + 32 bit data
			.HS_EAST_IN			(cs00_i_ready), // ready
			.HS_SOUTH_in		(cs10_o_ringbus), // i_ringbus
			.HS_EAST_out		(cs00_o_ringbus), // o_ringbus
			.HS_WEST_IN			({i_data_valid_adc,i_data_valid_adc,i_data_adc}), // data valid + 32 bit data
			.MIB_MASTER_RESET	(MIB_MASTER_RESET)
		);
`endif
`ifdef TB_USE_CS01
	cs01_top #(.VERILATE (VERILATE),
			.SCALAR_MEM_0 (`CS01_SCALAR_0),
			.SCALAR_MEM_1 (`CS01_SCALAR_1),
			.SCALAR_MEM_2 (`CS01_SCALAR_2),
			.SCALAR_MEM_3 (`CS01_SCALAR_3))
		cs01_top (
			.CLK				(clk),
			.FPGA_LED			(),
			.HS_SOUTH_OUT 		({cs01_o_data_vld,cs01_out_data}), // data valid + 32 bit data
			.HS_SOUTH_IN		(cs01_i_ready), // ready
			.HS_WEST_in			(cs00_o_ringbus), // i_ringbus
			.HS_SOUTH_out		(cs01_o_ringbus), // o_ringbus
			.HS_WEST_IN			({cs00_o_data_vld,cs00_out_data}), // data valid + 32 bit data
			.HS_WEST_OUT		(cs00_i_ready),	// ready
			.MIB_MASTER_RESET	(MIB_MASTER_RESET)
		);
`endif
`ifdef TB_USE_CS11
	cs11_top #(.VERILATE (VERILATE),
			.SCALAR_MEM_0 (`CS11_SCALAR_0),
			.SCALAR_MEM_1 (`CS11_SCALAR_1),
			.SCALAR_MEM_2 (`CS11_SCALAR_2),
			.SCALAR_MEM_3 (`CS11_SCALAR_3))
		cs11_top (
			.CLK				(clk),
			.FPGA_LED			(),
			.HS_SOUTH_OUT 		({cs11_o_data_vld,cs11_out_data}), // data valid + 32 bit data
			.HS_SOUTH_IN		(cs11_i_ready), // ready
			.HS_NORTH_in		(cs01_o_ringbus), // i_ringbus
			.HS_SOUTH_out		(cs11_o_ringbus), // o_ringbus
			.HS_NORTH_IN		({cs01_o_data_vld,cs01_out_data}), // data valid + 32 bit data
			.HS_NORTH_OUT		(cs01_i_ready),	// ready
			.MIB_MASTER_RESET	(MIB_MASTER_RESET)
		);
`endif
`ifdef TB_USE_CS21
	cs21_top #(.VERILATE (VERILATE),
			.SCALAR_MEM_0 (`CS21_SCALAR_0),
			.SCALAR_MEM_1 (`CS21_SCALAR_1),
			.SCALAR_MEM_2 (`CS21_SCALAR_2),
			.SCALAR_MEM_3 (`CS21_SCALAR_3))
		cs21_top (
			.CLK				(clk),
			.FPGA_LED			(),
			.HS_SOUTH_OUT 		({cs21_o_data_vld,cs21_out_data}), // data valid + 32 bit data
			.HS_SOUTH_IN		(cs21_i_ready), // ready
			.HS_NORTH_in		(cs11_o_ringbus), // i_ringbus
			.HS_SOUTH_out		(cs21_o_ringbus), // o_ringbus
			.HS_NORTH_IN		({cs11_o_data_vld,cs11_out_data}), // data valid + 32 bit data
			.HS_NORTH_OUT		(cs11_i_ready),	// ready
			.MIB_MASTER_RESET	(MIB_MASTER_RESET)
		);
`endif
`ifdef TB_USE_CS31
	cs31_top #(.VERILATE (VERILATE),
			.SCALAR_MEM_0 (`CS31_SCALAR_0),
			.SCALAR_MEM_1 (`CS31_SCALAR_1),
			.SCALAR_MEM_2 (`CS31_SCALAR_2),
			.SCALAR_MEM_3 (`CS31_SCALAR_3))
		cs31_top (
			.CLK				(clk),
			.FPGA_LED			(),
			.HS_WEST_OUT 		({cs31_o_data_vld,cs31_out_data}), // data valid + 32 bit data
			.HS_WEST_IN			(cs31_i_ready), // ready
			.HS_NORTH_in		(cs21_o_ringbus), // i_ringbus
			.HS_WEST_out		(cs31_o_ringbus), // o_ringbus
			.HS_NORTH_IN		({cs21_o_data_vld,cs21_out_data}), // data valid + 32 bit data
			.HS_NORTH_OUT		(cs21_i_ready),	// ready
			.MIB_MASTER_RESET	(MIB_MASTER_RESET)
		);
`endif
`ifdef TB_USE_CS30
	cs30_top #(.VERILATE (VERILATE),
			.SCALAR_MEM_0 (`CS30_SCALAR_0),
			.SCALAR_MEM_1 (`CS30_SCALAR_1),
			.SCALAR_MEM_2 (`CS30_SCALAR_2),
			.SCALAR_MEM_3 (`CS30_SCALAR_3))
		cs30_top (
			.CLK				(clk),
			.FPGA_LED			(),
			.HS_WEST_OUT 		({o_data_valid_eth,o_data_eth}), // data valid + 32 bit data
			.HS_WEST_IN			(i_o_ready_eth), // ready
			.HS_EAST_in			(cs31_o_ringbus), // i_ringbus
			.HS_WEST_out		(o_ringbus), // o_ringbus
			.HS_EAST_IN			({cs31_o_data_vld,cs31_out_data}), // data valid + 32 bit data
			.HS_EAST_OUT		(cs31_i_ready),	// ready
			.MIB_MASTER_RESET	(MIB_MASTER_RESET)
		);
`endif

	// eth_top #(.VERILATE (VERILATE),
	// 		.SCALAR_MEM_0 (`ETH_SCALAR_0),
	// 		.SCALAR_MEM_1 (`ETH_SCALAR_1),
	// 		.SCALAR_MEM_2 (`ETH_SCALAR_2),
	// 		.SCALAR_MEM_3 (`ETH_SCALAR_3))
	// 	eth_top (
	// 		.CLK				(clk),

	// 		.LED_D4				(),
	// 		.LED_D12			(),
	// 		.LED_D13			(),

	// 		.P1A_DDR_IN      ({o_data_valid_eth,o_data_eth}),
	// 		.P1A_DDR_OUT     (i_o_ready_eth),x4

	// 		.P1B_DDR_OUT     ({i_data_valid_eth,i_data_eth}),
	// 		.P1B_DDR_IN      (o_i_ready_eth),

	// 		.P1B_SDR_OUT     (eth_rst),

	// 		.P1A_DDR_in      (o_ringbus),
	// 		.P1B_DDR_out     (i_ringbus),

	// 		.MIB_AD          (MIB_AD),

	// 		.MIB_MASTER_RESET(MIB_MASTER_RESET),

	// 		// port 30000
	// 		.tx_turnstile_data_in     (tx_turnstile_data_in),
	// 		.tx_turnstile_data_valid  (tx_turnstile_data_valid),

	// 		// port 20000
	// 		.ringbus_in_data     	  (ringbus_in_data),
	// 		.ringbus_in_data_vld      (ringbus_in_data_vld),

	// 		// port 10000
	// 		.ringbus_out_data         (ringbus_out_data),
	// 		.ringbus_out_data_vld     (ringbus_out_data_vld),

	// 		.ring_bus_i0_ready        (ring_bus_i0_ready)

	// 		);


`ifdef TB_USE_CFG
	cfg_top #(.VERILATE (VERILATE))
		 cfg_top (
		 	.CFG_CLK          (clk),
		 	.MIB_AD           (MIB_AD[20:17]),
		 	.DAC_CTRL_SDIO    (DAC_CTRL_SDIO),
		 	.DAC_CTRL_SDENN   (DAC_CTRL_SDENN),
		 	.DAC_CTRL_SCLK    (DAC_CTRL_SCLK),
		 	.DAC_CTRL_RESETN  (DAC_CTRL_RESETN)

		 	);
`endif
	
	
endmodule