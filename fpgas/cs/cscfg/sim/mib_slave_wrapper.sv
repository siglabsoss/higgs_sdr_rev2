
`default_nettype none

module mib_slave_wrapper
#( 
    parameter bit [3:0] P_SLAVE_MIB_ADDR_MSN   = 4'h0, // each slave on the MIB bus gets a unique MIB Address Most Significant Nibble  
    parameter int       P_CMD_ACK_TIMEOUT_CLKS = 16    // This is how many clocks to wait for a CMD ACK before concluding that no ACK will ever come (SHOULD MAKE SMALLER THAN MIB BUS ACK TIMEOUT!)
)
(
    input  wire             i_sysclk, 
    input  wire             i_srst,
    input  wire             i_mib_start,            // from master, starts a mib transaction
    input  wire             i_mib_rd_wr_n,          // 1 = read, 0 = write
    output reg              o_mib_slave_ack,        // slave drives this during read data phases (if it's the target) (MAKE SURE THIS IS PULLED LOW AT MASTER)
    inout  wire             [15:0] mib_dabus,       // driven by slave during read data phases (if needed)
    intf_cmd.master         cmd_master
);

logic                        mib_ack_int;
logic                        mib_ack_int_high_z;
logic [15:0]                 mib_ad_int;
logic                        mib_ad_int_high_z;
logic                        mib_ack_int_reg;
logic                        mib_ack_int_high_z_reg;
logic [15:0]                 mib_ad_int_reg        /* synthesis syn_noprune=1 */;
logic                        mib_ad_int_high_z_reg /* synthesis syn_noprune=1 */;                            
logic [15:0]                 mib_ad_in_reg;
logic                        mib_start_in_reg;
logic                        mib_rd_wr_n_in_reg;

always_ff @(posedge i_sysclk) begin
    mib_ad_in_reg      <= mib_dabus;    
    mib_start_in_reg   <= i_mib_start;  
    mib_rd_wr_n_in_reg <= i_mib_rd_wr_n;
end


mib_slave #(
    .P_SLAVE_MIB_ADDR_MSN   (P_SLAVE_MIB_ADDR_MSN   ),
    .P_CMD_ACK_TIMEOUT_CLKS (P_CMD_ACK_TIMEOUT_CLKS ) // div by 2 to make sure master command bus timeout is less than MIB bus timeout
) _mib_slave (
    .i_sysclk               (i_sysclk          ), 
    .i_srst                 (i_srst             ),
    .cmd_master             (cmd_master         ),
    .i_mib_start            (mib_start_in_reg   ),
    .i_mib_rd_wr_n          (mib_rd_wr_n_in_reg ),
    .o_mib_slave_ack        (mib_ack_int        ),
    .o_mib_slave_ack_high_z (mib_ack_int_high_z ),
    .o_mib_ad               (mib_ad_int         ),
    .i_mib_ad               (mib_ad_in_reg      ),
    .o_mib_ad_high_z        (mib_ad_int_high_z  )

);

always_ff @(posedge i_sysclk) begin
    mib_ack_int_reg        <= mib_ack_int;
    mib_ack_int_high_z_reg <= mib_ack_int_high_z;
    mib_ad_int_reg         <= mib_ad_int;
    mib_ad_int_high_z_reg  <= mib_ad_int_high_z;
end

assign o_mib_slave_ack = (mib_ack_int_high_z_reg) ? 1'bz                    : mib_ack_int_reg;  //o_mib_slave_ack output, mib_ack
assign mib_dabus       = (mib_ad_int_high_z_reg)  ? 16'bzzzz_zzzz_zzzz_zzzz : mib_ad_int_reg;   //mib_dabus   output, mib_ad

endmodule

`default_nettype wire