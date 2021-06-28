//
// Template Test Bench Module
//

`timescale 1ns / 10ps

`default_nettype none

module tb_cs01_top;


localparam logic [3:0] TB_SLAVE_0_MIB_MSN         = 4'h1; // MSN = Most Significant Nibble
localparam int unsigned ADDR_BITS                 = 24;
localparam int unsigned CMD_DATA_BITS             = 32;
localparam int unsigned DATA_WIDTH                = 18;
localparam int unsigned LOG2LEN                   = 10;



logic                   sys_clk = 1'b0;
logic                   clk25;                        
logic                   clk125;                        
//////   
logic                   i_srst;
logic 	                CLK;
logic  [47:10]          HS_NORTH_IN;    
logic  [47:10]          HS_EAST_OUT;
logic	                FPGA_LED;
logic                   MIB_START;             // from master; starts a mib transaction
logic                   MIB_RD_WR_N;           // 1 = read; 0 = write
logic                   MIB_ACK;
wire    [15:0]          MIB_AD;   

logic                   pll_lock;

assign HS_NORTH_IN[18:11] = tb_afb.in_inph;
assign HS_NORTH_IN[46:29] = tb_afb.in_quad;
assign HS_NORTH_IN[47:47] = tb_afb.in_valid;

intf_cmd #(ADDR_BITS, CMD_DATA_BITS) tb_master(); // this is the command bus that actually goes to all the modules
intf_afb #(DATA_WIDTH, LOG2LEN) tb_afb(.clock(clk125),.reset(i_srst));

always #5 sys_clk = !sys_clk;
assign CLK = sys_clk;

cs01_pll _sys_pll (.CLKI(CLK), .CLKOP(clk25), .CLKOS(), .CLKOS2(clk125), .LOCK(pll_lock));

afb_stim afb_stim(tb_afb);

cs01_top 
#(
    .P_CMD_ACK_TIMEOUT_CLKS (16  ),
    .SIM_MODE               (1))
UUT(.*);

logic                 tb_m_cmd_mib_timeout;
logic  [15:0]         mib_ad_in_reg;
logic                 mib_slave_ack_reg;    
logic                 mib_rd_wr_n_reg;  
logic  [15:0]         o_mib_ad;
logic  [15:0]         mib_ad_int_reg;
logic                 mib_ad_high_z_int;             
logic                 mib_ad_int_high_z_reg;
logic                 mib_start_int;    
logic                 mib_rd_wr_n_int;               

always_ff @(posedge clk25) begin
    mib_ad_in_reg      <= MIB_AD;        
    mib_slave_ack_reg  <= MIB_ACK; 
    MIB_START        <= mib_start_int;   
    MIB_RD_WR_N      <= mib_rd_wr_n_int; 
end 

 
mib_master #(
    .P_MIB_ACK_TIMEOUT_CLKS(32)  // This is how many clocks to wait for a slave ACK before concluding that no ACK will ever come :(
)sim_mib(
    .i_sysclk             (clk25               ),      // currently cmd and mib interface are synchronous to this clock
    .i_srst               (i_srst              ), 
	.cmd_slave            (tb_master           ), 
    .i_mib_ad             (mib_ad_in_reg       ),      // driven by slave during read data phases (if needed)    
    .i_mib_slave_ack      (mib_slave_ack_reg   ),      // slave drives this during write phase to ack write data or during read data phase to signal valid read data 
    .o_cmd_mib_timeout    (tb_m_cmd_mib_timeout), 
    .o_mib_start          (mib_start_int       ),      // master drives this for one clock during address phase 1 
    .o_mib_rd_wr_n        (mib_rd_wr_n_int     ),      // 1 = read  0 = write
    .o_mib_ad_high_z      (mib_ad_high_z_int   ),      // 1 = tri-state b_mib_ad at top level  0 = drive it
    .o_mib_ad             (o_mib_ad            )       // driven by master during address phases and write data phases (if needed)
);

always_ff @(posedge clk25) begin
    mib_ad_int_reg         <= o_mib_ad;
    mib_ad_int_high_z_reg  <= mib_ad_high_z_int;
end

assign MIB_AD   = (mib_ad_int_high_z_reg)  ? 'bz : mib_ad_int_reg;   

task cmd_bus_write (input [23:0] waddr, input [31:0] wdata, string str);

    @(posedge clk25);

    tb_master.sel       = 1;
    tb_master.rd_wr_n   = 0;
    tb_master.byte_addr = waddr;
    tb_master.wdata     = wdata;

    while (1) begin
        @(posedge clk25);
        tb_master.sel       = 0;
        if (tb_master.ack) begin
            break;
        end
        else if (tb_m_cmd_mib_timeout) begin
            $display("CMD BUS WRITE: ADDR = 0x%x WRITE TIMEOUT!", waddr);
            break;
        end
    end

    tb_master.sel = 0;

    $display("CMD BUS WRITE: ADDR = 0x%x, DATA = 0x%X: %s", waddr, wdata, str);
    
endtask

task cmd_bus_read (input [23:0] raddr, string str);

    @(posedge clk25);

    tb_master.sel       = 1;
    tb_master.rd_wr_n   = 1;
    tb_master.byte_addr = raddr;

    while (1) begin
        @(posedge clk25);
        tb_master.sel       = 0;
        if (tb_master.ack) begin
            break;
        end
        else if (tb_m_cmd_mib_timeout) begin
            $display("CMD BUS READ: ADDR = 0x%x READ TIMEOUT!", raddr);
            break;
        end
    end

    tb_master.sel = 0;

    $display("CMD BUS READ: ADDR = 0x%x, DATA = 0x%X: %s", raddr, tb_master.rdata, str);

endtask

task poll ();
    cmd_bus_read({TB_SLAVE_0_MIB_MSN, 20'h10004}, "rxt_aligned_sample_counter");    
    cmd_bus_read({TB_SLAVE_0_MIB_MSN, 20'h10008}, "rxt_aligned_frame_counter");
    cmd_bus_read({TB_SLAVE_0_MIB_MSN, 20'h1000c}, "rxt_unaligned_sample_counter");
    cmd_bus_read({TB_SLAVE_0_MIB_MSN, 20'h10010}, "rxt_unaligned_frame_counter"); 
endtask

logic [31:0] err_cnt = 0;
initial begin
    tb_afb.enable <= 0;
    i_srst = 1'b1;
    @(posedge clk25);
    i_srst = 1'b0;
    @(posedge pll_lock);
    repeat(1000) @(posedge clk25);

    /* write-to-write */
    //  Register: rf1.rxt_window_target              Address: 0x10000     External: false
    //  Register: rf1.rxt_aligned_sample_counter     Address: 0x10004     External: false
    //  Register: rf1.rxt_aligned_frame_counter      Address: 0x10008     External: false
    //  Register: rf1.rxt_unaligned_sample_counter   Address: 0x1000c     External: false
    //  Register: rf1.rxt_unaligned_frame_counter    Address: 0x10010     External: false
    //  Register: rf1.rxt_trigger_target_frame       Address: 0x10014     External: false
    //  Register: rf1.rxt_trigger_target_sample      Address: 0x10018     External: false    
    $display("MIB S0 WRITE-TO-WRITE");
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h00004}, 32'h01010202, "Scratchpad");
    cmd_bus_read({TB_SLAVE_0_MIB_MSN, 20'h00000}, "fpga_uid");
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10000}, 32'h00000064, "window target"); 
    poll();
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10014}, 32'h00000000, "rxt_trigger_target_frame"); 
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10018}, 32'h000001DC, "rxt_trigger_target_sample");   
    repeat(4) @(posedge clk25);
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10014}, 32'h00000001, "rxt_trigger_target_frame");    
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10018}, 32'h000001DC, "rxt_trigger_target_sample");    
    repeat(4) @(posedge clk25);  
    @(posedge clk125);
    tb_afb.enable <= 1;
    repeat (1000) @(posedge clk25);
    poll();
end
    
endmodule