/*
 * This module should look like this
 * 
 *  intf_mcu <-> FMC_FSM <-> Internal Memory Look up  
 *                       <-> MIB MASTER for CS          ->
 *                       <-> MIB MASTER for graviton    ->
 */


module cfg_fpga_comm #(
    parameter bit          SIM_MODE       = 0
)
    (
   // --- system inputs ---
    input clk54_ext_a,    // Clock
    input CLK,
    input ext_mcu_arst,    // System reset.

    // FMC Interface
    input [24:0]  fmc_a,     // address bus (we don't use FMC_A[0] because we only need the address from the first write or read cycle)
    inout [15:0]  fmc_d,     // data bus
    input         fmc_ne1,   // chip select (active low), doesn't toggle between consecutive accesses
    input         fmc_noe,   // output enable (active low)
    input         fmc_nwe,   // write enable (active low)
    output        fmc_nwait,  // wait (active low)

    // internal interface - memory
    // Not needed, we'll wire them up

    
    // internal interface - MIB master copper suicide
    // Not needed, we'll wire them up


    // --- MIB Master Interface CS EXT ---
    // It's not necessary that cfg_mib_d be 16 bit. Hwever the overall mib bus
    // width is 30 bits, we need to make sure that we use the remaining 
    // 30 - (16+5) = 9 bits in the future. 
    //output            cfg_mib_timeout,
    inout [15:0]      cfg_mib_d,   // The bus
    output            cfg_mib_start, // mib transaction initiator
    output            cfg_mib_rd_wr_n,
    input             cfg_mib_slave_ack,
    //input             cfg_mib_slave,
    //output            cfg_mib_ad_high_z,
    // clk out for slaves making it source synchronous.
    output            o_clk_out,
    output            o_rst_out

    
);

/* UPDATE THESE TO MATCH ACTUAL TEST SETUP!!! */
    localparam  real    SYS_CLK_T_SECS         = 9.23e-9; // period of the system clock (seconds) 
    localparam  real    STM32F_HCLK_T_SECS     = 62.5e-9; // period of stm32f microcontroller's hclk (seconds)
    localparam  integer FMC_ADDR_SETUP_HCLKS   = 15;    // number of hclks used for FMC address setup
    localparam  integer FMC_DATA_SETUP_HCLKS   = 15;    // number of hclks used for FMC data setup
    localparam  integer FMC_BUS_TURN_HCLKS     = 15;    // number of hclks used for bus turnaround time during asynchronous read
    localparam  integer P_MIB_ACK_TIMEOUT_CLKS = 64;    // TIMEOUT value for MIB

    localparam NUM_OUTPUT_BRIDGES    =  4;
    localparam HOST_ADDRESS_BITS     = 26;
    localparam TARGETS_ADRESS_BITS   = 24;
    localparam HOST_DATA_BITS        = 32;

    localparam DATA_BITS  =           32; // Data bus size.
    localparam ADDR_BITS  =           26; // in terms of bits.

    //localparam integer P_MIB_ACK_TIMEOUT_CLKS = 32; 

wire fmc_d_high_z;
// PLL related signals
wire sys_clk;
wire sys_rst;
wire sys_clk_pll_locked;
wire [15:0]  fmc_out_d;     // data bus
reg [3:0]   mcu_arst_sync_regs = 3'b0;

intf_cmd #(HOST_ADDRESS_BITS, HOST_DATA_BITS) fmc_bridge ();
intf_cmd #(TARGETS_ADRESS_BITS, HOST_DATA_BITS) bridge_mib_csu[3:0] ();
intf_cmd #(TARGETS_ADRESS_BITS, HOST_DATA_BITS) mig_csu_1 ();

// mcu async reset sync 
always @ (posedge sys_clk) begin
    mcu_arst_sync_regs <= {mcu_arst_sync_regs[2:0], ext_mcu_arst};
end

assign sys_rst = &mcu_arst_sync_regs;


cscfg_pll cscfg_pll_i (
            .CLKI(CLK), .CLKOP(sys_clk), .LOCK(sys_clk_pll_locked)); 

assign o_clk_out = sys_clk;
assign o_rst_out = sys_rst;
assign fmc_d = (fmc_d_high_z) ? 'bz : fmc_out_d ; 

fmc_slave #(
    .SYS_CLK_T_SECS(SYS_CLK_T_SECS),
    .STM32F_HCLK_T_SECS(STM32F_HCLK_T_SECS),
    .FMC_ADDR_SETUP_HCLKS(FMC_ADDR_SETUP_HCLKS),
    .FMC_DATA_SETUP_HCLKS(FMC_DATA_SETUP_HCLKS),
    .FMC_BUS_TURN_HCLKS(FMC_BUS_TURN_HCLKS)
    ) _fmc_slave (
        .i_sys_clk(clk54_ext_a),
        .i_sys_rst(sys_rst),
        .o_fmc_d_high_z(fmc_d_high_z),
        .o_cmd_timeout(),
        // interface
        // TODO: change it intf_cmd type
        .i_cmd_ack(fmc_bridge.ack),
        .i_cmd_rdata(fmc_bridge.rdata),
        .o_cmd_sel(fmc_bridge.sel),
        .o_cmd_rd_wr_n(fmc_bridge.rd_wr_n),
        .o_cmd_byte_addr(fmc_bridge.byte_addr),
        .o_cmd_wdata(fmc_bridge.wdata),

        // FMC connections
        .i_fmc_a(fmc_a),
        .i_fmc_d(fmc_d),
        .o_fmc_d(fmc_out_d),
        //.o_fmc_d(fmc_d),
        .i_fmc_ne1(fmc_ne1),

        .i_fmc_noe(fmc_noe),
        .i_fmc_nwe(fmc_nwe),
        .o_fmc_nwait(fmc_nwait)


    );

fmc_bridge #(
    .NUM_OUTPUT_BRIDGES(NUM_OUTPUT_BRIDGES),    
    .HOST_ADDRESS_BITS(HOST_ADDRESS_BITS),     
    .TARGETS_ADRESS_BITS(TARGETS_ADRESS_BITS),   
    .HOST_DATA_BITS(HOST_DATA_BITS)       

    ) _fmc_bridge (
        .i_sys_clk(clk54_ext_a),
        .i_sys_rst(sys_rst),

        .i_fmc(fmc_bridge),
        .o_ext(bridge_mib_csu)

    );

fmc_slave_memory #(
        .ADDR_BITS(TARGETS_ADRESS_BITS),
        .DATA_BITS(HOST_DATA_BITS)
    ) _fmc_slave_memory (
        .i_sys_clk(clk54_ext_a),
        .i_sys_rst(sys_rst),

        .mem_cmd(bridge_mib_csu[0])

    );
    
mib_master_wrapper
#( 
    .P_MIB_ACK_TIMEOUT_CLKS(32)    
)
_mib_master_wrapper
(
    .i_sysclk           (sys_clk),
    .i_srst             (sys_rst),
    .o_mib_start        (cfg_mib_start), 
    .o_mib_rd_wr_n      (cfg_mib_rd_wr_n), 
    .o_cmd_mib_timeout  (), 
    .i_mib_slave_ack    (cfg_mib_slave_ack), 
    .mib_dabus          (cfg_mib_d), 
    .cmd_slave          (mig_csu_1)
);

mib_cdc #(
    .ADDR_BITS   (TARGETS_ADRESS_BITS),
    .DATA_BITS   (HOST_DATA_BITS),
    .SIM_MODE    (SIM_MODE)
)
_mib_cdc
(        

    .i_mib_clk    (clk54_ext_a), 
    .i_mib_srst   (sys_rst),
    .i_sys_clk    (sys_clk),
    .i_sys_srst   (sys_rst),
    .cmd_mib      (bridge_mib_csu[1]),
    .cmd_sys      (mig_csu_1)
);

endmodule