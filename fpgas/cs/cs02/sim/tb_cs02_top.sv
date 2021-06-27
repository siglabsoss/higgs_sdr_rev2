//
// Template Test Bench Module
//

`timescale 1ns / 10ps

`default_nettype none

module tb_cs02_top;

localparam logic [3:0] TB_SLAVE_0_MIB_MSN         = 4'h2; // MSN = Most Significant Nibble
localparam int unsigned ADDR_BITS                 = 24;
localparam int unsigned DATA_BITS                 = 32;

logic                   sys_clk = 1'b0;
logic                   clk25, clk125;                        
logic                   clk125_srst;                        

/////

logic                   i_srst;
logic 	                CLK;
logic   [47:10]         HS_NORTH_IN;    
logic   [47:36]         HS_WEST_OUT;
logic	                led_check; ;
logic                   MIB_START;             // from master; starts a mib transaction
logic                   MIB_RD_WR_N;           // 1 = read; 0 = write
logic                   MIB_ACK;
wire    [15:0]          MIB_AD;   

logic                   pll_lock;
logic [17:0]            i_sc_inph;
logic [17:0]            i_sc_quad;
logic                   i_valid;  
logic                   set;  


intf_cmd #(ADDR_BITS, DATA_BITS) tb_master(); // this is the command bus that actually goes to all the modules

always #5 sys_clk = !sys_clk;
assign CLK = sys_clk;
assign HS_NORTH_IN[10]    = ^(i_sc_inph) ^ (^(i_sc_quad));
assign HS_NORTH_IN[28:11] = i_sc_inph; 
assign HS_NORTH_IN[46:29] = i_sc_quad;
assign HS_NORTH_IN[47]    = i_valid;  

cs02_pll sys_pll (.CLKI(CLK), .CLKOP(clk125), .CLKOS(clk25), .LOCK(pll_lock));

cs02_top 
#(  .P_CMD_ACK_TIMEOUT_CLKS (16  ),
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
    i_srst      = 1'b1;
    clk125_srst = 1'b1;
    @(posedge clk25);
    i_srst = 1'b0;
    @(posedge pll_lock);
    repeat(1000) @(posedge clk25);
    @(posedge clk125);
    clk125_srst = 1'b0;
    /* write-to-write */
    @(posedge clk25);
    $display("MIB S0 READ-TO-READ");
    cmd_bus_read({TB_SLAVE_0_MIB_MSN, 20'h00000});    
    $display("MIB S0 WRITE-TO-WRITE");
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h00004}, 32'h01010202);
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10000}, 32'h00000000);
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10004}, 32'hFFFFFFFF);
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10008}, 32'hFFFFFFFF);
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h1000C}, 32'hFFFFFFFF);
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10010}, 32'hFFFFFFFF);
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10014}, 32'hFFFFFFFF);
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10018}, 32'hFFFFFFFF);
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h1001C}, 32'hFFFFFFFF);    
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10020}, 32'hFFFFFFFF);    
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10024}, 32'hFFFFFFFF);    
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10028}, 32'hFFFFFFFF);    
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h1002C}, 32'hFFFFFFFF);    
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10030}, 32'hFFFFFFFF); 
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10034}, 32'hFFFFFFFF); 
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10038}, 32'hFFFFFFFF); 
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h1003C}, 32'hFFFFFFFF); 
    cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10040}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10044}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10048}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h1004C}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10050}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10054}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10058}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h1005C}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10060}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10064}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10068}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h1006C}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10070}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10074}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10078}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h1007C}, 32'hFFFFFFFF); 
    //cmd_bus_write({TB_SLAVE_0_MIB_MSN, 20'h10080}, 32'hFFFFFFFF); 
    repeat(4) @(posedge clk25);
    $display("MIB S0 READ-TO-READ");
    cmd_bus_read({TB_SLAVE_0_MIB_MSN, 20'h00004});
    $display("SEND STIM DATA");
    for (int index = 0; index < 100000; index++) begin
        repeat(4) @(posedge clk125);
        #1 i_sc_inph = index;
        #1 i_sc_quad = -index;
        #1 i_valid   = 1'b1;
        @(posedge clk125);
        #1 i_valid   = 1'b0;
    end 
    $finish;
end
    
endmodule