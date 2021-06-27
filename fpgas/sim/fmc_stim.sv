`timescale 1s / 100ps // delay value specified in seconds with 100 ps resolution

`default_nettype none

module fmc_stim (
        fmc_cmd.master  fmc_tb  
    );

    localparam logic [3:0] P_SLAVE_MIB_ADDR_MSN   = 0; // each slave on the MIB bus gets a unique MIB Address Most Significant Nibble  
    localparam int         P_CMD_ACK_TIMEOUT_CLKS = 16;    // This is how many clocks to wait for a CMD ACK before concluding that no ACK will ever come (SHOULD MAKE SMALLER THAN MIB BUS ACK TIMEOUT!)

    localparam real    SYS_CLK_T_SECS   = 9.23e-9; // system clock period (seconds)
    localparam real    HCLK_T_SECS      = 62.5e-9;  // STM32F microcontroller clock period (seconds)
    localparam integer ADDR_SETUP_HCLKS = 15;    // number of hclks to use for fmc address setup time (valid range = 0 to 15)
    localparam integer DATA_SETUP_HCLKS = 15;    // number of hclks to use for fmc data setup time (valid range = 4 to 256 (4 is min to allow for nwait detection by stm32f))
    localparam integer BUS_TURN_HCLKS   = 15;    // number of hclks to use for fmc bus turnaround time (valid range = 0 to 15)

    localparam real ADDR_SETUP_TIME         = HCLK_T_SECS * ADDR_SETUP_HCLKS;
    localparam real DATA_SETUP_TIME         = HCLK_T_SECS * DATA_SETUP_HCLKS;
    localparam real BUS_TURN_TIME           = HCLK_T_SECS * BUS_TURN_HCLKS;

    // These localparam values were determined from logic analyzer observation of the Nucleo-F767ZI eval board's FMC interface.  Not sure how universal they are.
    localparam real FMC_TRANS_TO_TRANS_SECS = 1e-6;   // duration between FMC bus transactions 
    localparam real OUT_EN_PULSE_SECS       = 50e-9;  // duration that FMC de-asserts output enable before re-asserting it for second read data phase 
    localparam real WR_EN_PULSE_SECS        = 100e-9; // duration that FMC de-asserts write enable before re-asserting it for second write data phase
    
        // TODO: THESE LOCALPARAMS SHOULD COME FROM A PACKAGE
    localparam int unsigned ADDR_BITS             = 24;
    localparam int unsigned DATA_BITS             = 32;   
    localparam int unsigned CMD_DATA_BITS         = 32; 
    


    reg         tb_sys_clk = 1'b0;
    reg         tb_cmd_ack = 1'b0;
    reg  [31:0] tb_cmd_rdata = {32{1'b0}};
    wire        tb_cmd_vld;   
    wire        tb_cmd_rd_wr_n;
    wire [25:0] tb_cmd_addr;   
    wire [31:0] tb_cmd_wdata;   
    reg  [31:0] tb_cmd_dummy_reg;

    reg  [15:0] tb_fmc_master_d_reg = {16{1'b0}};
    wire [15:0] tb_fmc_slave_d;
    wire        tb_fmc_slave_d_high_z;
    reg         tb_fmc_master_oe = 1'b0;

    
    wire [15:0] mib_dabus;
    wire        i_mib_start;
    wire        i_mib_rd_wr_n;
    wire        o_mib_slave_ack;
    wire        clk_out;
    wire        rst_out;
    wire        rst_out_n;
    reg [32*8-1:0] tb_test_stage_str;

    initial begin
    fmc_tb.fmc_a     = 'b0;
    fmc_tb.fmc_ne1   = 1'b1;
    fmc_tb.fmc_noe   = 1'b1;
    fmc_tb.fmc_nwe   = 1'b1; 
    end
    
    task fmc_write (input [25:0] waddr, input [31:0] wdata); 

        // Since the fmc interface is wired for 16-bit data it takes 2 fmc write transactions per 32-bit write.
        // The FMC interface does NOT de-assert the chip select/enable in between write transactions, but rather de-asserts and re-asserts the write enable signal. 

        $display("FMC WRITE (ADDR = 0x%h, DATA = 0x%h)", waddr, wdata);

        /* cycle 1 */
        fmc_tb.fmc_a = waddr[25:1]; fmc_tb.fmc_ne1 = 1'b0; fmc_tb.fmc_noe = 1'b1;
        #ADDR_SETUP_TIME;
        tb_fmc_master_oe = 1'b1; fmc_tb.fmc_nwe = 1'b0; tb_fmc_master_d_reg = wdata[15:0]; // little endian
        #DATA_SETUP_TIME;
        if (fmc_tb.fmc_nwait == 1'b0) begin
            wait (fmc_tb.fmc_nwait); // delay extra time in slave is asserting nwait
            #(4*HCLK_T_SECS);
        end
        fmc_tb.fmc_nwe = 1'b1; tb_fmc_master_oe = 1'b0; // keep chip select/enable asserted between write data phases and only pulse write enable
        #WR_EN_PULSE_SECS;

        /* cycle 2 */
        fmc_tb.fmc_a = waddr[25:1] + 1; fmc_tb.fmc_ne1 = 1'b0; fmc_tb.fmc_noe = 1'b1;
        #ADDR_SETUP_TIME;
        tb_fmc_master_oe = 1'b1; fmc_tb.fmc_nwe = 1'b0; tb_fmc_master_d_reg = wdata[31:16]; // little endian
        #DATA_SETUP_TIME;
        if (fmc_tb.fmc_nwait == 1'b0) begin
            wait (fmc_tb.fmc_nwait); // delay extra time in slave is asserting nwait
            #(4*HCLK_T_SECS);
        end
        fmc_tb.fmc_ne1 = 1'b1; fmc_tb.fmc_nwe = 1'b1; tb_fmc_master_oe = 1'b0;
        #FMC_TRANS_TO_TRANS_SECS;


    endtask

    task fmc_read (input  [25:0] raddr); //, input nwait, output [25:0] a, output ne1, output noe, output nwe, input [15:0] d);

        reg [31:0] rdata;

        // Since the fmc interface is wired for 16-bit data it takes 2 fmc read transactions per 32-bit read.
        // The FMC interface does NOT de-assert the chip select/enable in between read transactions, but rather de-asserts and re-asserts the output enable signal. 

        /* cycle 1 */
        fmc_tb.fmc_a = raddr[25:1]; fmc_tb.fmc_ne1 = 1'b0; fmc_tb.fmc_nwe = 1'b1; fmc_tb.fmc_noe = 1'b0; tb_fmc_master_oe = 1'b0;
        #ADDR_SETUP_TIME;
        #DATA_SETUP_TIME;
        if (fmc_tb.fmc_nwait == 1'b0) begin
            wait (fmc_tb.fmc_nwait); // delay extra time in slave is asserting nwait
            #(4*HCLK_T_SECS);
        end
        fmc_tb.fmc_noe = 1'b1; // only de-assert output enable between read phases, keep chip select/enable asserted
        rdata[15:0] = fmc_tb.fmc_d; // little endian
        #OUT_EN_PULSE_SECS;

        /* cycle 2 */
        fmc_tb.fmc_a = raddr[25:1] + 1; fmc_tb.fmc_ne1 = 1'b0; fmc_tb.fmc_nwe = 1'b1; fmc_tb.fmc_noe = 1'b0; tb_fmc_master_oe = 1'b0;
        #ADDR_SETUP_TIME;
        #DATA_SETUP_TIME;
        if (fmc_tb.fmc_nwait == 1'b0) begin
            wait (fmc_tb.fmc_nwait); // delay extra time in slave is asserting nwait
            #(4*HCLK_T_SECS);
        end
        fmc_tb.fmc_ne1 = 1'b1; fmc_tb.fmc_noe = 1'b1;
        rdata[31:16] = fmc_tb.fmc_d; // little endian

        $display("FMC READ (ADDR = 0x%h, RDATA = 0x%h)", raddr, rdata);

        #FMC_TRANS_TO_TRANS_SECS;

    endtask

    assign fmc_tb.fmc_d = (tb_fmc_master_oe)       ? tb_fmc_master_d_reg : 16'bz;

    // system clock generation
    initial begin
        forever #(SYS_CLK_T_SECS/2.0) tb_sys_clk = ~tb_sys_clk;
    end
    
    assign fmc_tb.clk = tb_sys_clk;
    /*
     *
     * FOUR TEST SCENARIOS:
     *     1. BACK-TO-BACK WRITES
     *     2. BACK-TO-BACK READS
     *     3. WRITE FOLLOWED BY READ
     *     4. READ FOLLOWED BY WRITE
     *
     */

    initial begin : STIMULUS
        @(negedge fmc_tb.reset);        
        repeat (10) @(posedge fmc_tb.clk);       
        // BACK-TO-BACK WRITES
        tb_test_stage_str = "WRITE-WRITE TEST";
        fmc_read(26'h1000000);  //0
        fmc_read(26'h1100000);  //1
        fmc_read(26'h1200000);  //2
        ////fmc_read(26'h1300000);  //3
        ////fmc_read(26'h1400000);  //4
        ////fmc_read(26'h1500000);  //5
        fmc_read(26'h1600000);  //6
        ////fmc_read(26'h1700000);  //7
        fmc_read(26'h1800000);  //8
        fmc_read(26'h1900000);  //9
        fmc_read(26'h1A00000);  //10
        ////fmc_read(26'h1B00000);  //11
        ////fmc_read(26'h1C00000);  //12
        ////fmc_read(26'h1D00000);  //13
        ////fmc_read(26'h1E00000);  //14
        ////fmc_read(26'h1F00000);  //15

        $finish;
    end
endmodule