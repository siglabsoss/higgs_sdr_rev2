// ===========================================================================
// Verilog module generated by IPexpress
// Filename: test_1.v  
// Copyright 2005 (c) Lattice Semiconductor Corporation. All rights reserved.
// ===========================================================================

reg   [7:0]  dummy_data;


initial begin
   $display(" .") ;
   $display(" ==============================================================") ;
   $display(" INFO : EVAL SIMULATION") ;
   $display(" ==============================================================") ;
   $display(" INFO : NOTE: This simulation includes the TSMAC IP Core,") ;
   $display(" INFO : instantiated in an FPGA top level that consists of ") ;
   $display(" INFO : test logic (MAC client side loop back logic, PLLs, ") ;
   $display(" INFO : and registers with Read/Write Intf). This FPGA top ") ;
   $display(" INFO : is instantiated in an eval testbench that configures ") ;
   $display(" INFO : FPGA test logic and TSMAC IP core registers and sources") ;
   $display(" INFO : ethernet packets (testcase.v)") ;
   $display(" ==============================================================") ;
   $display(" .") ;
   $display(" .") ;
   $display(" INFO : testcase STARTING") ;
   repeat (10) @(posedge clk_125) ;
   reset_n   <= 1'b0 ;
   repeat (10) @(posedge clk_125) ;
   reset_n   <= 1'b1 ;

// delay to wait for initialization
   repeat (10) @(posedge clk_125 );

	// Test logic registers
        orc_write(18'h08006, 8'hC1); // fifo AFL 
        orc_write(18'h08007, 8'h01); //  fifo AFH
        orc_write(18'h08008, 8'h05); //  fifo AEL
        orc_write(18'h08001, 8'h07); // tstcntl
        orc_read(18'h08000, dummy_data); // verid
        orc_read(18'h08006, dummy_data); // fifo AFL
        orc_read(18'h08007, dummy_data); // fifo AFH
        orc_read(18'h08008, dummy_data); // fifo AEL

	// MDIO registers
       `ifdef MIIM_MODULE
        orc_write(18'h00016, 8'h00); // to  - MDIO DATA reg 
        orc_write(18'h00017, 8'h80); // to  - MDIO DATA reg 
        orc_write(18'h00014, 8'h00); // to  - MDIO ACCESS CTL reg 
        orc_write(18'h00015, 8'h21); // to  - MDIO ACCESS CTL reg
	#20000 
        orc_read(18'h00014, dummy_data); // to  - MDIO ACCESS CTL reg 
        orc_read(18'h00015, dummy_data); // to  - MDIO ACCESS CTL reg 
       `endif

	// MAC registers
        orc_write(18'h0000a, 8'hcd);           // MAC Addr reg 0
        orc_write(18'h0000b, 8'haa);           // MAC Addr reg 0
        orc_write(18'h0000c, 8'h12);           // MAC Addr reg 1
        orc_write(18'h0000d, 8'hef);           // MAC Addr reg 1
        orc_write(18'h0000e, 8'h56);           // MAC Addr reg 2
        orc_write(18'h0000f, 8'h34);           // MAC Addr reg 2

        orc_write(18'h00002, 8'h9a); // to host bus - TX_RX_CTL reg 
        orc_read(18'h00002, dummy_data); // to host bus  - TX_RX_CTL reg

        orc_write(18'h00008, 8'h48); // to host bus - IPG reg 
        orc_read(18'h00008, dummy_data); // to host bus  - IPG reg
        orc_read(18'h00009, dummy_data); // to host bus  - IPG reg

    // Default to simulate with 1000M mode
        orc_write(18'h00000, 8'h0f); // to host bus - mode reg 
    // To simulate the 100M mode, use the following mode reg
    //  orc_write(18'h00000, 8'h0e); // to host bus - mode reg 
        orc_read(18'h00000, dummy_data); // to host bus  - mode reg


    // FRAME TEMPLATE
    // rx_frmgen (gben(1), des_addr(48), frm_len(14), num_premb(4), num_ipg(5), badcrc(1), norm_vlan_paus(2),
    // pause_timer(16), bad_pcode(1), len_type(1), len_chkerr(1), badsfd(1), runt_frmid(3), frm_patn(4));

    rx_frmgen(1'b1, 48'haacdef123456, 14'd75, 4'd7, 5'd26, `RX_GOOD_CRC, `RX_NORM_FRM, 16'h0, `RX_GOOD_OPCODE,
             `RX_LEN_FIELD, `RX_LENCHK_NOER, `RX_GOOD_SFD, `RX_RUNT_NOT, `RX_FRMPTN_NOER); 

    rx_frmgen(1'b1, 48'haacdef123456, 14'd70, 4'd7, 5'd26, `RX_GOOD_CRC, `RX_NORM_FRM, 16'h0, `RX_GOOD_OPCODE,
             `RX_LEN_FIELD, `RX_LENCHK_NOER, `RX_GOOD_SFD, `RX_RUNT_NOT, `RX_FRMPTN_NOER); 

    rx_frmgen(1'b1, 48'haacdef123456, 14'd66, 4'd7, 5'd26, `RX_GOOD_CRC, `RX_NORM_FRM, 16'h0, `RX_GOOD_OPCODE,
             `RX_LEN_FIELD, `RX_LENCHK_NOER, `RX_GOOD_SFD, `RX_RUNT_NOT, `RX_FRMPTN_NOER); 

    rx_frmgen(1'b1, 48'haacdef123456, 14'd64, 4'd7, 5'd26, `RX_GOOD_CRC, `RX_NORM_FRM, 16'h0, `RX_GOOD_OPCODE,
             `RX_LEN_FIELD, `RX_LENCHK_NOER, `RX_GOOD_SFD, `RX_RUNT_NOT, `RX_FRMPTN_NOER); 

    rx_frmgen(1'b1, 48'haacdef123456, 14'd32, 4'd7, 5'd26, `RX_GOOD_CRC, `RX_NORM_FRM, 16'h0, `RX_GOOD_OPCODE,
             `RX_LEN_FIELD, `RX_LENCHK_NOER, `RX_GOOD_SFD, `RX_RUNT_NOT, `RX_FRMPTN_NOER); 

    /* For 100M Classic mode, use following rx_frmgen
    rx_frmgen(1'b0, 48'haacdef123456, 14'd75, 4'd7, 5'd26, `RX_GOOD_CRC, `RX_NORM_FRM, 16'h0, `RX_GOOD_OPCODE,
             `RX_LEN_FIELD, `RX_LENCHK_NOER, `RX_GOOD_SFD, `RX_RUNT_NOT, `RX_FRMPTN_NOER); 

    rx_frmgen(1'b0, 48'haacdef123456, 14'd70, 4'd7, 5'd26, `RX_GOOD_CRC, `RX_NORM_FRM, 16'h0, `RX_GOOD_OPCODE,
             `RX_LEN_FIELD, `RX_LENCHK_NOER, `RX_GOOD_SFD, `RX_RUNT_NOT, `RX_FRMPTN_NOER); 

    rx_frmgen(1'b0, 48'haacdef123456, 14'd66, 4'd7, 5'd26, `RX_GOOD_CRC, `RX_NORM_FRM, 16'h0, `RX_GOOD_OPCODE,
             `RX_LEN_FIELD, `RX_LENCHK_NOER, `RX_GOOD_SFD, `RX_RUNT_NOT, `RX_FRMPTN_NOER); 

    rx_frmgen(1'b0, 48'haacdef123456, 14'd64, 4'd7, 5'd26, `RX_GOOD_CRC, `RX_NORM_FRM, 16'h0, `RX_GOOD_OPCODE,
             `RX_LEN_FIELD, `RX_LENCHK_NOER, `RX_GOOD_SFD, `RX_RUNT_NOT, `RX_FRMPTN_NOER); 

    rx_frmgen(1'b0, 48'haacdef123456, 14'd32, 4'd7, 5'd26, `RX_GOOD_CRC, `RX_NORM_FRM, 16'h0, `RX_GOOD_OPCODE,
             `RX_LEN_FIELD, `RX_LENCHK_NOER, `RX_GOOD_SFD, `RX_RUNT_NOT, `RX_FRMPTN_NOER); 
   */

   repeat (500) @(posedge clk_125 );
   // To simulate the 100M mode, will need long simulation time and should use following one:
   //repeat (2000) @(posedge clk_125 );

   $stop ;
end

// =============================================================================
