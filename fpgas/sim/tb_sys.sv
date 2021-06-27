//
// Template Test Bench Module
//

`timescale 1ns / 10ps

`default_nettype none

module tb_sys;

localparam CMD_ACK_TIMEOUT_CLKS      = 32;


logic            i_srst;             
logic 	         CLK = 0;
logic [22:0]     CS00_LS_EAST;
logic [47:0]     CS00_HS_EAST;                    
logic            CS00_mib_start;     
logic            CS00_mib_rd_wr_n;   
logic            CS00_mib_slave_ack;
// 
logic [22:0]     CS01_LS_EAST;
logic [47:0]     CS01_HS_EAST; 
logic [22:0]     CS01_LS_NORTH;
logic [47:0]     CS01_HS_NORTH;    
logic            CS01_mib_start;     
logic            CS01_mib_rd_wr_n;   
logic            CS01_mib_slave_ack;
//
logic [22:0]     CS02_LS_WEST;
logic [47:0]     CS02_HS_WEST;  
logic [22:0]     CS02_LS_NORTH;
logic [47:0]     CS02_HS_NORTH;   
logic            CS02_mib_start;     
logic            CS02_mib_rd_wr_n;   
logic            CS02_mib_slave_ack;
//
//
logic [22:0]     CS12_LS_NORTH;
logic [47:0]     CS12_HS_NORTH;
logic [22:0]     CS12_LS_SOUTH;
logic [47:0]     CS12_HS_SOUTH;
//  
logic [22:0]     CS20_LS_EAST;
logic [47:0]     CS20_HS_EAST; 
logic            CS20_mib_start;     
logic            CS20_mib_rd_wr_n;   
logic            CS20_mib_slave_ack;

//
logic [22:0]     CS21_LS_WEST;
logic [47:0]     CS21_HS_WEST;
logic [22:0]     CS21_LS_EAST;
logic [47:0]     CS21_HS_EAST;
//
logic [22:0]     CS22_LS_NORTH;
logic [47:0]     CS22_HS_NORTH;
logic [22:0]     CS22_LS_WEST;
logic [47:0]     CS22_HS_WEST;
;
//
logic 	         CS30_led_check;     
logic            CSCFG_mib_start;     
logic            CSCFG_mib_rd_wr_n;   
wire             CSCFG_mib_slave_ack;  
wire    [15:0]   mib_dabus; 

logic            CS12_mib_start;
logic            CS12_mib_rd_wr_n;
wire             CS12_mib_slave_ack;
logic            CS22_mib_start;
logic            CS22_mib_rd_wr_n;
wire             CS22_mib_slave_ack;
logic            CS21_mib_start;
logic            CS21_mib_rd_wr_n;
wire             CS21_mib_slave_ack;
   

always #5 CLK = !CLK;

assign CS00_mib_start      = CSCFG_mib_start;  
assign CS00_mib_rd_wr_n    = CSCFG_mib_rd_wr_n;
assign CSCFG_mib_slave_ack = CS00_mib_slave_ack;

assign CS01_LS_NORTH       = CS00_LS_EAST;
assign CS01_HS_NORTH       = CS00_HS_EAST;
assign CS02_LS_NORTH       = CS01_LS_EAST;
assign CS02_HS_NORTH       = CS01_HS_EAST;
assign CS12_LS_NORTH       = CS02_LS_WEST;
assign CS12_HS_NORTH       = CS02_HS_WEST;
assign CS22_LS_NORTH       = CS12_LS_SOUTH; 
assign CS22_HS_NORTH       = CS12_HS_SOUTH;
assign CS21_LS_EAST        = CS22_LS_WEST; 
assign CS21_HS_EAST        = CS22_HS_WEST;
assign CS20_LS_EAST        = CS21_LS_WEST;
assign CS20_HS_EAST        = CS21_HS_WEST;  

assign CS01_mib_start      = CSCFG_mib_start;  
assign CS01_mib_rd_wr_n    = CSCFG_mib_rd_wr_n;
assign CSCFG_mib_slave_ack = CS01_mib_slave_ack;

assign CS12_mib_start      = CSCFG_mib_start;  
assign CS12_mib_rd_wr_n    = CSCFG_mib_rd_wr_n;
assign CSCFG_mib_slave_ack = CS12_mib_slave_ack;

assign CS22_mib_start      = CSCFG_mib_start;  
assign CS22_mib_rd_wr_n    = CSCFG_mib_rd_wr_n;
assign CSCFG_mib_slave_ack = CS22_mib_slave_ack;

assign CS21_mib_start      = CSCFG_mib_start;  
assign CS21_mib_rd_wr_n    = CSCFG_mib_rd_wr_n;
assign CSCFG_mib_slave_ack = CS21_mib_slave_ack;

assign CS02_mib_start      = CSCFG_mib_start;  
assign CS02_mib_rd_wr_n    = CSCFG_mib_rd_wr_n;
assign CSCFG_mib_slave_ack = CS02_mib_slave_ack;

assign CS20_mib_start      = CSCFG_mib_start;  
assign CS20_mib_rd_wr_n    = CSCFG_mib_rd_wr_n;
assign CSCFG_mib_slave_ack = CS20_mib_slave_ack;

fmc_cmd #(25,16) fmc_tb (i_srst);
fmc_stim _fmc_stim (fmc_tb);

cs00_top #(
    .P_CMD_ACK_TIMEOUT_CLKS  (CMD_ACK_TIMEOUT_CLKS)  
)                                                        
_cs00_top
(                                                         
    .i_srst                  (i_srst              ),      
	.CLK                     (CLK                 ),
    .led_check               (                    ),
    .LS_EAST                 (CS00_LS_EAST        ),
    .HS_EAST                 (CS00_HS_EAST        ),
    .i_mib_start             (CS00_mib_start      ),      
    .i_mib_rd_wr_n           (CS00_mib_rd_wr_n    ),      
    .o_mib_slave_ack         (CS00_mib_slave_ack  ),      
    .mib_dabus               (mib_dabus           )       
	);
    
cs01_top #(
    .P_CMD_ACK_TIMEOUT_CLKS  (CMD_ACK_TIMEOUT_CLKS)  
)                                                        
_cs01_top
(                                                         
    .i_srst                  (i_srst              ),      
	.CLK                     (CLK                 ),
    .led_check               (                    ),
    .LS_NORTH                (CS01_LS_NORTH       ),        
    .HS_NORTH                (CS01_HS_NORTH       ),    
    .LS_EAST                 (CS01_LS_EAST        ),
    .HS_EAST                 (CS01_HS_EAST        ),
    .i_mib_start             (CS01_mib_start      ),      
    .i_mib_rd_wr_n           (CS01_mib_rd_wr_n    ),      
    .o_mib_slave_ack         (CS01_mib_slave_ack  ),      
    .mib_dabus               (mib_dabus           )       
	);
    
cs02_top #(
    .P_CMD_ACK_TIMEOUT_CLKS  (CMD_ACK_TIMEOUT_CLKS)  
)                                                        
_cs02_top
(                                                         
    .i_srst                  (i_srst              ),      
	.CLK                     (CLK                 ),
    .led_check               (                    ),
    .LS_NORTH                (CS02_LS_NORTH       ),        
    .HS_NORTH                (CS02_HS_NORTH       ), 
    .LS_WEST                 (CS02_LS_WEST        ),        
    .HS_WEST                 (CS02_HS_WEST        ),      
    .i_mib_start             (CS02_mib_start      ),      
    .i_mib_rd_wr_n           (CS02_mib_rd_wr_n    ),      
    .o_mib_slave_ack         (CS02_mib_slave_ack  ),      
    .mib_dabus               (mib_dabus           )       
	);

cs12_top #(
    .P_CMD_ACK_TIMEOUT_CLKS  (CMD_ACK_TIMEOUT_CLKS)  
)                                                        
_cs12_top
(                                                         
    .i_srst                  (i_srst              ),      
	.CLK                     (CLK                 ),
    .led_check               (                    ),    
	.LS_NORTH                (CS12_LS_NORTH       ),
	.HS_NORTH                (CS12_HS_NORTH       ),
	.LS_WEST                 (CS12_LS_SOUTH       ),
	.HS_WEST                 (CS12_HS_SOUTH       ),   
    .i_mib_start             (CS12_mib_start      ),      
    .i_mib_rd_wr_n           (CS12_mib_rd_wr_n    ),      
    .o_mib_slave_ack         (CS12_mib_slave_ack  ),      
    .mib_dabus               (mib_dabus           )       
	);
    
cs22_top #(
    .P_CMD_ACK_TIMEOUT_CLKS (CMD_ACK_TIMEOUT_CLKS)    // This is how many clocks to wait for a CMD ACK before concluding that no ACK will ever come (SHOULD MAKE SMALLER THAN MIB BUS ACK TIMEOUT!)
)
_cs22_top
(
    .i_srst                 (i_srst              ),
	.CLK                    (CLK                 ),
    .led_check              (                    ),
	.LS_NORTH               (CS22_LS_NORTH       ),
	.HS_NORTH               (CS22_HS_NORTH       ),
	.LS_WEST                (CS22_LS_WEST        ),
	.HS_WEST                (CS22_HS_WEST        ),
    .i_mib_start            (CS22_mib_start      ), 
    .i_mib_rd_wr_n          (CS22_mib_rd_wr_n    ), 
    .o_mib_slave_ack        (CS22_mib_slave_ack  ),
    .mib_dabus              (mib_dabus           )
	);     
    
 
cs21_top #(
    .P_CMD_ACK_TIMEOUT_CLKS (CMD_ACK_TIMEOUT_CLKS)    // This is how many clocks to wait for a CMD ACK before concluding that no ACK will ever come (SHOULD MAKE SMALLER THAN MIB BUS ACK TIMEOUT!)
)
_cs21_top
(
    .i_srst                 (i_srst              ),
	.CLK                    (CLK                 ),
    .led_check              (                    ),
	.LS_EAST                (CS21_LS_EAST        ),
	.HS_EAST                (CS21_HS_EAST        ),
	.LS_WEST                (CS21_LS_WEST        ),
	.HS_WEST                (CS21_HS_WEST        ),
    .i_mib_start            (CS21_mib_start      ), 
    .i_mib_rd_wr_n          (CS21_mib_rd_wr_n    ), 
    .o_mib_slave_ack        (CS21_mib_slave_ack  ),
    .mib_dabus              (mib_dabus           )
	);
 
cs20_top #(
    .P_CMD_ACK_TIMEOUT_CLKS  (CMD_ACK_TIMEOUT_CLKS)  
)                                                        
_cs20_top
(                                                         
    .i_srst                  (i_srst              ),      
	.CLK                     (CLK                 ),
    .led_check               (                    ),    
	.LS_EAST                 (CS20_LS_EAST        ),
	.HS_EAST                 (CS20_HS_EAST        ),   
    .i_mib_start             (CS20_mib_start      ),      
    .i_mib_rd_wr_n           (CS20_mib_rd_wr_n    ),      
    .o_mib_slave_ack         (CS20_mib_slave_ack  ),      
    .mib_dabus               (mib_dabus           )       
	);    
    

cs30_top #(
    .P_CMD_ACK_TIMEOUT_CLKS (CMD_ACK_TIMEOUT_CLKS     )
)
_cs30_top
(
    .i_srst                (i_srst             ),
	.CLK                   (CLK                )
    //.o_mib_start           (CSCFG_mib_start     ), 
    //.o_mib_rd_wr_n         (CSCFG_mib_rd_wr_n   ), 
    //.i_mib_slave_ack       (CSCFG_mib_slave_ack ),
    //.mib_dabus             (mib_dabus          )
	);
    
cscfg_top #(
        .SIM_MODE (1)
)
_cscfg_top
(
    .clk54_ext_a             (fmc_tb.clk),          
    .CLK                     (CLK),
    .ext_mcu_arst            (i_srst),
    .fmc_a                   (fmc_tb.fmc_a    ),
    .fmc_d                   (fmc_tb.fmc_d    ),
    .fmc_ne1                 (fmc_tb.fmc_ne1  ),
    .fmc_noe                 (fmc_tb.fmc_noe  ),
    .fmc_nwe                 (fmc_tb.fmc_nwe  ),
    .fmc_nwait               (fmc_tb.fmc_nwait),
    .cfg_mib_d               (mib_dabus),
    .cfg_mib_start           (CSCFG_mib_start),
    .cfg_mib_rd_wr_n         (CSCFG_mib_rd_wr_n),
    .cfg_mib_slave_ack       (CSCFG_mib_slave_ack),
    .o_clk_out               (),
    .o_rst_out               (),
    .PROG_N                  ()
	);
    
initial begin
    i_srst = 1'b1;
    repeat(20) @(posedge CLK);
    i_srst = 1'b0;
end

endmodule

`default_nettype wire