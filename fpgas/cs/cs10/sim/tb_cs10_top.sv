//
// Template Test Bench Module
//

`timescale 10ps / 10ps

`default_nettype none

module tb_cs10_top;

localparam logic [3:0] TB_SLAVE_0_MIB_MSN         = 4'h0; // MSN = Most Significant Nibble
localparam int unsigned ADDR_BITS                 = 24;
localparam int unsigned DATA_BITS                 = 32;

logic                   sys_clk = 1'b0;
logic                   clk25;                        
//////   
logic                   i_srst;
logic  	                CLK;
logic    [70:0]         IL07;
logic    [70:0]         IL68;
logic    [70:0]         IL12;
logic    [70:0]         IL34;
logic    [3:0]          error_dect_50;
logic    [3:0]          error_dect_125;
logic 	                led_check;
logic                   i_mib_start;             
logic                   i_mib_rd_wr_n;           
logic                   o_mib_slave_ack;
wire    [15:0]          mib_dabus;
/////
logic                   pll_lock;


intf_cmd #(ADDR_BITS, DATA_BITS) tb_master(); // this is the command bus that actually goes to all the modules

always #5 sys_clk = !sys_clk;
assign CLK = sys_clk;

cs10_pll _sys_pll (.CLKI(CLK), .CLKOP(clk25), .CLKOS(), .CLKOS2(), .LOCK(pll_lock));

cs10_top 
#(  .P_CMD_ACK_TIMEOUT_CLKS (16  ))
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
    mib_ad_in_reg      <= mib_dabus;        
    mib_slave_ack_reg  <= o_mib_slave_ack; 
    i_mib_start        <= mib_start_int;   
    i_mib_rd_wr_n      <= mib_rd_wr_n_int; 
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

assign mib_dabus   = (mib_ad_int_high_z_reg)  ? 'bz : mib_ad_int_reg;   

task cmd_bus_write (input [23:0] waddr, input [31:0] wdata);

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
            $display("MIB BUS WRITE TIMEOUT!");
            break;
        end
    end

    tb_master.sel = 0;

    $display("CMD BUS WRITE: ADDR = 0x%x, DATA = 0x%X", waddr, wdata);
    
endtask

task cmd_bus_read (input [23:0] raddr);

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
            $display("MIB BUS READ TIMEOUT!");
            break;
        end
    end

    tb_master.sel = 0;

    $display("CMD BUS READ: ADDR = 0x%x, DATA = 0x%X", raddr, tb_master.rdata);

endtask

logic [31:0] err_cnt = 0;
initial begin
    i_srst = 1'b1;
    @(posedge clk25);
    i_srst = 1'b0;
    @(posedge pll_lock);
    @(posedge clk25);

    /* write-to-write */

    $display("MIB S0 WRITE-TO-WRITE");
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h00004}, 32'h01010202);
    cmd_bus_read({TB_SLAVE_0_MIB_MSN, 20'h00000});
    repeat(4) @(posedge clk25);
    cmd_bus_read({TB_SLAVE_0_MIB_MSN, 20'h00004});
    cmd_bus_read({TB_SLAVE_0_MIB_MSN, 20'h00008});
    $finish;
end
    
endmodule