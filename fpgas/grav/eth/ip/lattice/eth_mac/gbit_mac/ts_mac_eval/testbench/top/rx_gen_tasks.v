// ===========================================================================
// Verilog module generated by IPexpress
// Filename: rx_gen_tasks.v  
// Copyright 2005 (c) Lattice Semiconductor Corporation. All rights reserved.
// ===========================================================================

   task rx_frmgen;
 /*( gben,
                 des_addr,
                 frm_len,
                 num_premb,
                 num_ipg,
                 badcrc,         //1'b0 is for good crc, 1'b1 is for bad crc 
                 norm_vlan_paus, //2'b00 is for normal frame, 2'b01 is for VLAN tagged frame,
                                 //2'b10 is for pause frame, 
                                 //2'b11 is for pause frame with customized des_addr
                 pause_timer,    //valid only when norm_vlan_paus is set to 2'b10
                 bad_pcode,      //1'b0 is for good pause op-code, 1'b1 is for bad pause op-code
                 len_type,       //1'b0 is for length field, 1'b1 is for type field
                 len_chkerr,     //1'b0 is for no length check error, 
                                 //1'b1 is for generating length check error
                 badsfd,         //1'b0 is for good sfd, 1'b1 is for bad sfd
                 runt_frmid,
                 frm_patn       //4'd0 is a normal frame without any following condition
                                 //4'd1 is for assert one clock long rx_er during frame     
                                 //4'd2 is for non-padded frame which is less than 64 bytes
                                 //4'd14 is for non-padded frame which is less than 64 with dribble nibble
                                 //4'd15 is for dribble nibble frame
                 );*/
      input        gben;              //bit 0 in frm_info
      input [47:0] des_addr;          //bit 48:1 in frm_info
      input [13:0] frm_len;           //bit 62:49 in frm_info
      input [3:0]  num_premb;         //bit 66:63 in frm_info
      input [4:0]  num_ipg;           //bit 71:67 in frm_info
      input        badcrc;            //bit 72 in frm_info
      input [1:0]  norm_vlan_paus;    //bit 74:73 in frm_info
      input [15:0] pause_timer;       //bit 90:75 in frm_info
      input        bad_pcode;         //bit 91 in frm_info
      input        len_type;          //bit 92 in frm_info  
      input        len_chkerr;        //bit 93 in frm_info 
      input        badsfd;            //bit 94 in frm_info
      input [2:0]  runt_frmid;        //bit 97:95 in frm_info
      input [3:0]  frm_patn;          //bit 101:98 in frm_info

      integer i;
      reg [13:0] reg_i;
      reg [13:0] data_len;
      
      reg [7:0] strt_rxd;
      reg [31:0] crc_reg;
      reg [101:0] frm_info;

      reg [7:0] rxd_reg;
      //reg       rxdv;
      //reg       rxer;
      //reg [7:0] rxd;
      event frm_data;
      
      
      parameter PREAMBLE_NIB = 4'h5;
      parameter SFD_NIB1 = 4'hd;
      parameter GOODSFD_NIB2 = 4'h5;
      parameter BADSFD_NIB2 = 4'h3;
      parameter RNDM_FRM_TYPE1 = 4'h5;
      parameter RNDM_FRM_TYPE2 = 4'hd;
      parameter CRC_INIT_VALUE = 32'hffff_ffff;
      parameter TAG_CTRL_NIB1 = 4'd1;
      parameter TAG_CTRL_NIB2 = 4'd2;
      parameter TAG_CTRL_NIB3 = 4'd3;
      parameter TAG_CTRL_NIB4 = 4'd4;
      parameter PAUSE_LENTYPE1 = 4'h8; //length/type field for pause frame is 16'h8808
      parameter PAUSE_LENTYPE2 = 4'h0; //reversed version of it is 16'h1110 
      parameter PAUSE_LENTYPE3 = 4'h8;
      parameter PAUSE_LENTYPE4 = 4'h8;
      
      parameter PAUSE_DES_NIB0 = 4'h1; //destination address for pause frame is 48'h0180_c200_0001
      parameter PAUSE_DES_NIB1 = 4'h0;
      parameter PAUSE_DES_NIB2 = 4'h0;
      parameter PAUSE_DES_NIB3 = 4'h0;
      parameter PAUSE_DES_NIB4 = 4'h0;
      parameter PAUSE_DES_NIB5 = 4'h0;
      parameter PAUSE_DES_NIB6 = 4'h2;
      parameter PAUSE_DES_NIB7 = 4'hc;
      parameter PAUSE_DES_NIB8 = 4'h0;
      parameter PAUSE_DES_NIB9 = 4'h8;
      parameter PAUSE_DES_NIB10 = 4'h1;
      parameter PAUSE_DES_NIB11 = 4'h0;

//      parameter PAUSE_OP_NIB1 = 4'h0;
//      parameter PAUSE_OP_NIB2 = 4'h0;
//      parameter PAUSE_OP_NIB3 = 4'h8;
//      parameter PAUSE_OP_NIB4 = 4'h0;

      parameter PAUSE_D_NIB1 = 4'h0;
      parameter PAUSE_D_NIB2 = 4'h0;
  
      //runt frame ID
      //8'd0 do not generate runt frame
      //8'd1 runt frame with preamble only
      //8'd2 runt frame with preamble and sfd
      //8'd3 runt frame with preamble, sfd and destination address 
      //8'd4 runt frame with preamble, sfd, destination address and source address 
`define PAUSE_OP_NIB4  4'h0
`define PAUSE_OP_NIB3  4'h0
`define PAUSE_OP_NIB2  4'h0
`define PAUSE_OP_NIB1  4'h1
//`define PAUSE_OP_NIB4 test_ts_mac.U1_ts_mac_top.U1_ts_mac_core.U1_cpu_if.pause_opcode[15:12]   
//`define PAUSE_OP_NIB3 test_ts_mac.U1_ts_mac_top.U1_ts_mac_core.U1_cpu_if.pause_opcode[11:8]   
//`define PAUSE_OP_NIB2 test_ts_mac.U1_ts_mac_top.U1_ts_mac_core.U1_cpu_if.pause_opcode[7:4]   
//`define PAUSE_OP_NIB1 test_ts_mac.U1_ts_mac_top.U1_ts_mac_core.U1_cpu_if.pause_opcode[3:0]   

//`include "rx_gen_tasks.inc.v"
      
   begin
      frm_info = {frm_patn, runt_frmid, badsfd, len_chkerr, len_type, bad_pcode, pause_timer,
                  norm_vlan_paus, badcrc, num_ipg, num_premb, frm_len, des_addr, gben};

      if (num_ipg == 0) begin  
        rxdv = 1'b0;
        rxer = 1'b1;
        rxd  = 8'h0f; // carrier extend
      end 
      else begin
        rxdv = 1'b0;
        rxer = 1'b0;
        rxd  = 8'b0;
      end 

      $display("INFO:%t New Frame is generated:", $time);
      if(!norm_vlan_paus[0] && !norm_vlan_paus[1]) 
        $display("\t\tIt is a normal frame;");
      else if(norm_vlan_paus[0] && !norm_vlan_paus[1])
        $display("\t\tIt is a VLAN tagged frame;");
      else if(norm_vlan_paus[1])
        $display("\t\tIt is a Pause frame;");
      $display("\t\tData length is %d(DEC);", frm_len);
      $display("\t\tDestination address is %h(HEX);", des_addr);
      $display("\t\t%d bytes of preamble;", num_premb);
      if(badsfd)
        $display("\t\tBad SFD will be appended;");
      $display("\t\tIPG number is %d(DEC);", num_ipg);
      if(len_chkerr)
        $display("\t\tLength check err will be generated;");
      if(badcrc)
        $display("\t\tBad CRC will be generated.\n");
      else
        $display("\t\tGood CRC will be generated.\n");
        
      if(num_ipg == 0)
        $display("\t\t6 clocks of carrier extension will be generated.\n");

      //wait for proper IPG or 6 clks of Carrier Extend
      if (num_ipg == 0) begin  
         repeat (6) begin
            @(negedge rxmac_clk);
         end
      end 
      else begin
         repeat (num_ipg-1) begin
            @(negedge rxmac_clk);
         end
      end 

      crc_reg = 32'hffff_ffff;
      
      //Preamble generation
      if(num_premb > 4'd0) begin
         repeat (num_premb) begin
	    @(negedge rxmac_clk)
            rxdv = 1'b1;
	    
            if (num_ipg == 0) begin  // carrier extend
              rxer = 1'b0;
            end
	    
            rxd_reg = {PREAMBLE_NIB, PREAMBLE_NIB};
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
	    //$display ("%t rxd_reg is %h\n", $time, rxd_reg);
         end // repeat (num_premb)
      end // if (num_premb > 4'd0)


      //SFD generation
      if(runt_frmid !== 8'd1) begin
         if(!badsfd) begin
	    @(negedge rxmac_clk)
            rxdv = 1'b1;
            rxd_reg = {SFD_NIB1, GOODSFD_NIB2};
         end // if (!badsfd)
         else begin
            @(negedge rxmac_clk)
            rxdv = 1'b1;
            rxd_reg = {SFD_NIB1, BADSFD_NIB2};
         end
      end
      if (gben == 1) begin
	 rxd[7:0] = rxd_reg[7:0];
      end else begin
	 rxd[3:0] = rxd_reg[3:0];
         @(posedge rxmac_clk)
	 rxd[3:0] = rxd_reg[7:4];
      end
      
      //destination address generation
      if (norm_vlan_paus == 2'b10) begin //pause frame
         @(negedge rxmac_clk)
         rxd_reg = {PAUSE_DES_NIB11, PAUSE_DES_NIB10};
         crc_reg = nextCRC32_D8(rxd_reg, CRC_INIT_VALUE);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxd_reg = {PAUSE_DES_NIB9, PAUSE_DES_NIB8};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxd_reg = {PAUSE_DES_NIB7, PAUSE_DES_NIB6};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxd_reg = {PAUSE_DES_NIB5, PAUSE_DES_NIB4};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxd_reg = {PAUSE_DES_NIB3, PAUSE_DES_NIB2};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxd_reg = {PAUSE_DES_NIB1, PAUSE_DES_NIB0};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
      end // if (norm_vlan_paus == 2'b10)
      else begin
         if( (runt_frmid !== 8'd1) && (runt_frmid !== 8'd2) ) begin
            @(negedge rxmac_clk)
            rxd_reg = des_addr[47:40];
            crc_reg = nextCRC32_D8(rxd_reg, CRC_INIT_VALUE);
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
            @(negedge rxmac_clk)
            rxd_reg = des_addr[39:32];
            crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
            @(negedge rxmac_clk)
            rxd_reg = des_addr[31:24];
            crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
            @(negedge rxmac_clk)
            rxd_reg = des_addr[23:16];
            crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
            @(negedge rxmac_clk)
            rxd_reg = des_addr[15:8];
            crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
            @(negedge rxmac_clk)
            rxd_reg = des_addr[7:0];
            crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
         end // if ( (runt_frmid !== 8'd1) && (runt_frmid !== 8'd2) )
      end // else: !if(norm_vlan_paus == 2'b10)
         
      //source address generation
      if( (runt_frmid !== 8'd1) && (runt_frmid !== 8'd2) && (runt_frmid !== 8'd3) ) begin
         @(negedge rxmac_clk)
         rxd_reg = {`SRC_NIB11, `SRC_NIB10};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxd_reg = {`SRC_NIB9, `SRC_NIB8};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxd_reg = {`SRC_NIB7, `SRC_NIB6};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)             
         rxd_reg = {`SRC_NIB5, `SRC_NIB4};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxd_reg = {`SRC_NIB3, `SRC_NIB2};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxd_reg = {`SRC_NIB1, `SRC_NIB0};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
      end // if ( (runt_frmid !== 8'd1) && (runt_frmid !== 8'd2) )

         //VLAN tagged field
      if(norm_vlan_paus == 2'b01) begin
         @(negedge rxmac_clk)
         rxd_reg = 8'b1000_0001;
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxd_reg = 8'b0000_0000;
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxd_reg = {TAG_CTRL_NIB4, TAG_CTRL_NIB3};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxd_reg = {TAG_CTRL_NIB2, TAG_CTRL_NIB1};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
      end // if (norm_vlan_paus == 2'b01)

         //length/type field generation
      if((norm_vlan_paus == 2'b10) || (norm_vlan_paus == 2'b11)) begin //pause frame
         @(negedge rxmac_clk)
         rxd_reg = {PAUSE_LENTYPE4, PAUSE_LENTYPE3};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxd_reg = {PAUSE_LENTYPE2, PAUSE_LENTYPE1};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
      end // if (norm_vlan_paus[1] == 1'b1)
      else begin
         if( (runt_frmid != 8'd1) && (runt_frmid != 8'd2) && (runt_frmid != 8'd3)
             && (runt_frmid != 8'd4) ) begin     
           if(!len_type) begin
              if(!len_chkerr) begin
                 @(negedge rxmac_clk)
                 rxd_reg = {5'b00000,frm_len[10:8]};
                 crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
                 if (gben == 1) begin
	            rxd[7:0] = rxd_reg[7:0];
                 end else begin
	            rxd[3:0] = rxd_reg[3:0];
                    @(posedge rxmac_clk)
	            rxd[3:0] = rxd_reg[7:4];
                 end
                 @(negedge rxmac_clk)
                 rxd_reg = frm_len[7:0];
                 crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
                 if (gben == 1) begin
	            rxd[7:0] = rxd_reg[7:0];
                 end else begin
	            rxd[3:0] = rxd_reg[3:0];
                    @(posedge rxmac_clk)
	            rxd[3:0] = rxd_reg[7:4];
                 end
              end // if (!len_chkerr)
              else begin
                 @(negedge rxmac_clk)
                 rxd_reg = {5'b00000, frm_len[10:8]};
                 crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
                 if (gben == 1) begin
	            rxd[7:0] = rxd_reg[7:0];
                 end else begin
	            rxd[3:0] = rxd_reg[3:0];
                    @(posedge rxmac_clk)
	            rxd[3:0] = rxd_reg[7:4];
                 end
                 @(negedge rxmac_clk)
                 rxd_reg = frm_len[7:0] + 8'h01;
                 crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
                 if (gben == 1) begin
	            rxd[7:0] = rxd_reg[7:0];
                 end else begin
	            rxd[3:0] = rxd_reg[3:0];
                    @(posedge rxmac_clk)
	            rxd[3:0] = rxd_reg[7:4];
                 end
              end // else: !if(!len_chkerr)
           end // if (!len_type)
           else begin
              @(negedge rxmac_clk)
              rxd_reg = {RNDM_FRM_TYPE2, RNDM_FRM_TYPE1};
              crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
              if (gben == 1) begin
	         rxd[7:0] = rxd_reg[7:0];
              end else begin
	         rxd[3:0] = rxd_reg[3:0];
                 @(posedge rxmac_clk)
	         rxd[3:0] = rxd_reg[7:4];
              end
              @(negedge rxmac_clk)
              rxd_reg = 8'h00;
              crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
              if (gben == 1) begin
	         rxd[7:0] = rxd_reg[7:0];
              end else begin
	         rxd[3:0] = rxd_reg[3:0];
                 @(posedge rxmac_clk)
	         rxd[3:0] = rxd_reg[7:4];
              end
           end // else: !if(!len_type)
         end // if ( (runt_frmid != 8'd1) && (runt_frmid != 8'd2) && (runt_frmid != 8'd3)...
      end // else: !if(norm_vlan_paus == 2'b10)
      
         //MAC control op-code and parameter, frame data for pause frame
      if((norm_vlan_paus == 2'b11) || (norm_vlan_paus == 2'b10)) begin //pause frame
         @(negedge rxmac_clk)
         if(bad_pcode)
            rxd_reg = ~{`PAUSE_OP_NIB4, `PAUSE_OP_NIB3};
         else
            rxd_reg = {`PAUSE_OP_NIB4, `PAUSE_OP_NIB3};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxer = (frm_patn === 4'd1); //generating rx_er during pause frame
         rxd_reg = {`PAUSE_OP_NIB2, `PAUSE_OP_NIB1};
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxer = 1'b0;                //de-assert rx_er
         rxd_reg = pause_timer[15:8];
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         @(negedge rxmac_clk)
         rxd_reg = pause_timer[7:0];
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         for (i=0; i<42; i=i+1) begin
            @(negedge rxmac_clk)
            rxd_reg = {PAUSE_D_NIB2, PAUSE_D_NIB1};
            crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
         end
      end // if (norm_vlan_paus[1] == 1'b1)
      //else if(runt_frmid == 4'd0) begin
      else if(runt_frmid == 4'd0 || (runt_frmid == 4'd2 && frm_patn == 4'd3) ) begin  // N.G change
         
         //frame data generation
         strt_rxd = $random;
         rxer = (frm_patn === 4'd1);
         @(negedge rxmac_clk)    
         rxd_reg = strt_rxd[7:0];
         crc_reg = nextCRC32_D8(rxd_reg, crc_reg);
         if (gben == 1) begin
	    rxd[7:0] = rxd_reg[7:0];
         end else begin
	    rxd[3:0] = rxd_reg[3:0];
            @(posedge rxmac_clk)
	    rxd[3:0] = rxd_reg[7:4];
         end
         rxer = 1'b0;
         //for (i=(frm_len-18-1);i>=1;i=i-1) begin
         if( !norm_vlan_paus[0] && !norm_vlan_paus[1] ) begin
            //if ( (frm_len < 14'd46) && ((frm_patn != 4'd2) && (frm_patn != 4'd14)) ) begin
            if ( (frm_len < 14'd46) && ((frm_patn != 4'd2) && (frm_patn != 4'd14) && (frm_patn != 4'd3)) ) begin
              data_len = 14'd46;
            end
            else begin
               data_len = frm_len;
            end
         end
         else if( norm_vlan_paus[0] && !norm_vlan_paus[1] ) begin
            if( (frm_len < 14'd42) && (frm_patn != 4'd2) ) begin
              data_len = 14'd42;
            end
            else begin
               //data_len = frm_len - 4;
               data_len = frm_len;
            end
         end
         else 
           data_len = frm_len;
         //for (i=(frm_len-1);i>=1;i=i-1) begin
         for (i=(data_len-1);i>=1;i=i-1) begin
            @(negedge rxmac_clk)

             rxd_reg = rxd_reg+8'd1;

	     /*
	    if (i == 0) begin
               rxd_reg = 8'h24;
	    end else begin
               rxd_reg = 8'h55;
            end
	    */

            crc_reg = nextCRC32_D8(rxd_reg, crc_reg);                
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
         end // for (i=frm_len-1;i<1;i=i-1)
      end // else: !if(norm_vlan_paus[1] == 1'b1)
        
      //FCS generation
      if(runt_frmid === 3'd0) begin        
         if(!badcrc) begin
            crc_reg = crc_rev(crc_reg);
	    @(negedge rxmac_clk)
            rxd_reg = crc_reg[7:0];
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
            @(negedge rxmac_clk)
            rxd_reg = crc_reg[15:8];
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
            @(negedge rxmac_clk)
            rxd_reg = crc_reg[23:16];
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
            @(negedge rxmac_clk)
            rxd_reg = crc_reg[31:24];
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
         end // if (badfcs)
         else begin
            @(negedge rxmac_clk)
            rxd_reg = crc_reg[7:0];
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
            @(negedge rxmac_clk)
            rxd_reg = crc_reg[15:8];
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
            @(negedge rxmac_clk)
            rxd_reg = crc_reg[23:16];
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
            @(negedge rxmac_clk)
            rxd_reg = crc_reg[31:24];
            if (gben == 1) begin
	       rxd[7:0] = rxd_reg[7:0];
            end else begin
	       rxd[3:0] = rxd_reg[3:0];
               @(posedge rxmac_clk)
	       rxd[3:0] = rxd_reg[7:4];
            end
         end // else: !if(badfcs)
      end // if (runt_frmid === 8'd0)

      if (gben == 1) begin
         @(negedge rxmac_clk)
         rxdv = 1'b0;
      end else begin
     	 if ((frm_patn == 4'd15) || (frm_patn == 4'd14)) begin
            @(negedge rxmac_clk)
              rxd_reg = $random;
	      rxd[3:0] = rxd_reg[3:0];
              @(posedge rxmac_clk)
              rxdv = 1'b0;
         end else begin
	    @(negedge rxmac_clk)
            rxdv = 1'b0;
	 end
      end
   end
endtask     

function [31:0] nextCRC32_D8;

      input [7:0] Data;
      input [31:0] CRC;
      
      reg [7:0] D;
      reg [31:0] C;
      reg [31:0] NewCRC;

  begin
     D[0] = Data[7];
     D[1] = Data[6];
     D[2] = Data[5];
     D[3] = Data[4];
     D[4] = Data[3];
     D[5] = Data[2];
     D[6] = Data[1];
     D[7] = Data[0];
     C = CRC;
     
     NewCRC[0] = D[6] ^ D[0] ^ C[24] ^ C[30];
     NewCRC[1] = D[7] ^ D[6] ^ D[1] ^ D[0] ^ C[24] ^ C[25] ^ C[30] ^ 
                 C[31];
     NewCRC[2] = D[7] ^ D[6] ^ D[2] ^ D[1] ^ D[0] ^ C[24] ^ C[25] ^ 
                 C[26] ^ C[30] ^ C[31];
     NewCRC[3] = D[7] ^ D[3] ^ D[2] ^ D[1] ^ C[25] ^ C[26] ^ C[27] ^ 
                 C[31];
     NewCRC[4] = D[6] ^ D[4] ^ D[3] ^ D[2] ^ D[0] ^ C[24] ^ C[26] ^ 
                 C[27] ^ C[28] ^ C[30];
     NewCRC[5] = D[7] ^ D[6] ^ D[5] ^ D[4] ^ D[3] ^ D[1] ^ D[0] ^ C[24] ^ 
                C[25] ^ C[27] ^ C[28] ^ C[29] ^ C[30] ^ C[31];
     NewCRC[6] = D[7] ^ D[6] ^ D[5] ^ D[4] ^ D[2] ^ D[1] ^ C[25] ^ C[26] ^ 
                C[28] ^ C[29] ^ C[30] ^ C[31];
     NewCRC[7] = D[7] ^ D[5] ^ D[3] ^ D[2] ^ D[0] ^ C[24] ^ C[26] ^ 
                 C[27] ^ C[29] ^ C[31];
     NewCRC[8] = D[4] ^ D[3] ^ D[1] ^ D[0] ^ C[0] ^ C[24] ^ C[25] ^ 
                 C[27] ^ C[28];
     NewCRC[9] = D[5] ^ D[4] ^ D[2] ^ D[1] ^ C[1] ^ C[25] ^ C[26] ^ 
                 C[28] ^ C[29];
     NewCRC[10] = D[5] ^ D[3] ^ D[2] ^ D[0] ^ C[2] ^ C[24] ^ C[26] ^ 
                  C[27] ^ C[29];
     NewCRC[11] = D[4] ^ D[3] ^ D[1] ^ D[0] ^ C[3] ^ C[24] ^ C[25] ^ 
                 C[27] ^ C[28];
     NewCRC[12] = D[6] ^ D[5] ^ D[4] ^ D[2] ^ D[1] ^ D[0] ^ C[4] ^ C[24] ^ 
                  C[25] ^ C[26] ^ C[28] ^ C[29] ^ C[30];
     NewCRC[13] = D[7] ^ D[6] ^ D[5] ^ D[3] ^ D[2] ^ D[1] ^ C[5] ^ C[25] ^ 
                  C[26] ^ C[27] ^ C[29] ^ C[30] ^ C[31];
     NewCRC[14] = D[7] ^ D[6] ^ D[4] ^ D[3] ^ D[2] ^ C[6] ^ C[26] ^ C[27] ^ 
                  C[28] ^ C[30] ^ C[31];
     NewCRC[15] = D[7] ^ D[5] ^ D[4] ^ D[3] ^ C[7] ^ C[27] ^ C[28] ^ 
                  C[29] ^ C[31];
     NewCRC[16] = D[5] ^ D[4] ^ D[0] ^ C[8] ^ C[24] ^ C[28] ^ C[29];
     NewCRC[17] = D[6] ^ D[5] ^ D[1] ^ C[9] ^ C[25] ^ C[29] ^ C[30];
     NewCRC[18] = D[7] ^ D[6] ^ D[2] ^ C[10] ^ C[26] ^ C[30] ^ C[31];
     NewCRC[19] = D[7] ^ D[3] ^ C[11] ^ C[27] ^ C[31];
     NewCRC[20] = D[4] ^ C[12] ^ C[28];
     NewCRC[21] = D[5] ^ C[13] ^ C[29];
     NewCRC[22] = D[0] ^ C[14] ^ C[24];
     NewCRC[23] = D[6] ^ D[1] ^ D[0] ^ C[15] ^ C[24] ^ C[25] ^ C[30];
     NewCRC[24] = D[7] ^ D[2] ^ D[1] ^ C[16] ^ C[25] ^ C[26] ^ C[31];
     NewCRC[25] = D[3] ^ D[2] ^ C[17] ^ C[26] ^ C[27];
     NewCRC[26] = D[6] ^ D[4] ^ D[3] ^ D[0] ^ C[18] ^ C[24] ^ C[27] ^ 
                  C[28] ^ C[30];
     NewCRC[27] = D[7] ^ D[5] ^ D[4] ^ D[1] ^ C[19] ^ C[25] ^ C[28] ^ 
                  C[29] ^ C[31];
     NewCRC[28] = D[6] ^ D[5] ^ D[2] ^ C[20] ^ C[26] ^ C[29] ^ C[30];
     NewCRC[29] = D[7] ^ D[6] ^ D[3] ^ C[21] ^ C[27] ^ C[30] ^ C[31];
     NewCRC[30] = D[7] ^ D[4] ^ C[22] ^ C[28] ^ C[31];
     NewCRC[31] = D[5] ^ C[23] ^ C[29];
     
     nextCRC32_D8 = NewCRC;

  end

endfunction


function [31:0] crc_rev;
      
      input [31:0] data_in;
      integer crc_i;
   begin    
      for (crc_i = 0; crc_i < 32; crc_i = crc_i+1) begin
         crc_rev[crc_i] = ~data_in[31 - crc_i]; 
      end
   end
endfunction

      
