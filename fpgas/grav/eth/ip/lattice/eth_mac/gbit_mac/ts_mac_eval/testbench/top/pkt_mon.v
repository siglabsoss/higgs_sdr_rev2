/*================================================================
-- Copyright (c) 2004 - Lattice Semiconductor Corporation
--                    - NetCom IP
--
-- This program is controlled by a written license agreement.
-- Unauthorized Reproduction or Use is Expressly Prohibited.
-- ================================================================*/

// file name:	pkt_mon.v
// version:	1.0
// date:	may 13, 2008 
// 
// code type:	Behavioral Level
//
// Overview:    This module monitors ethernet packets, and checks
//              their crc and then prints them to a file.	
//
// Rev:		0.0 - Initial Ver N.G
/*=================================================================*/

`timescale 1ns/100ps


// DEFINES


module pkt_mon(
		reset_n,
		gbit_en,
    		tx_clk,
		`ifdef SGMII_TSMAC
    		  tx_clk_en,
		`endif
         	tx_en,
         	tx_er,
    		txd
		);



        // ethernet ports
	input [7:0] txd;
	input tx_er;
	input tx_en;

        // general ports
        input reset_n;                      // active low global reset
        input gbit_en;                      // GbE mode enable 
        input tx_clk;                       // tx clock

	`ifdef SGMII_TSMAC
    	  input tx_clk_en;
	`endif

        integer out_file;                   // pointer to output file - ethernet_pkts_sink 

        wire nib_enb;                        //
        wire nib_enb_0;                      // nib_0 enable select (3:0) 
        wire nib_enb_1;                      // nib_0 enable select (7:4) 
	
	// ------------------------------
	// signals related to monitor FSM
	// ------------------------------
        reg [1:0] m_st;
	reg [11:0] byt_cnt;
	reg [15:0] pkt_cnt;
	reg [7:0] txd_fe ;
	reg [7:0] txd_fe_B ;
	reg byte_valid_fe ;
	reg tx_en_1d ;
	reg tx_er_1d ;
	reg tx_en_2d ;
	reg tx_er_2d ;
	reg [7:0] txd_1d ;
        reg nib_cnt;                          // nibble counter - toggles 

	reg [7:0] len_B0 ;
	reg [15:0] len_B0B1 ;
        reg [31:0] crc_reg;
        reg [31:0] crc_reg_latched;
	reg [7:0] Rx_CRC_B0 ;
	reg [7:0] Rx_CRC_B1 ;
	reg [7:0] Rx_CRC_B2 ;
	reg [31:0] Rx_CRC ;
	reg tx_error_latched ;
	reg control_pkt ;
	reg pause_pkt ;
	reg vlan_pkt ;
	reg [7:0] op_code_B0 ;
	reg [7:0] vlan_len_B0 ;

	
        parameter[1:0] 
                  IDLE = 0,
                  PREMB = 1,
                  PKT_BODY = 2;
	  
        parameter CRC_INIT_VALUE = 32'hffff_ffff;

	// ------------------------------



// Initializations
initial
begin
	out_file = $fopen("ethernet_pkts_sink");
	m_st = 0;
	byt_cnt = 0;
	pkt_cnt = 0;
	len_B0 = 0;
	len_B0B1 = 0;
	Rx_CRC_B0 = 0;
	Rx_CRC_B1 = 0;
	Rx_CRC_B2 = 0;
	Rx_CRC = 0;
end


  // wire assignments
   assign nib_enb = nib_cnt;
   assign nib_enb_0 = ( (nib_enb == 1'd0) && (tx_en == 1'b1) ) ? 1:0;
   assign nib_enb_1 = ( (nib_enb == 1'd1) && (tx_en == 1'b1) ) ? 1:0;

   
  // ---------------------------------------------------------------------------------
  // 10/100 nib to bytes 
  // ---------------------------------------------------------------------------------
   always @(posedge tx_clk or negedge reset_n) begin
        if (~reset_n) begin
           nib_cnt <= 1'b0;
           txd_fe[7:0]  <= 8'b0;
           byte_valid_fe  <= 1'b0;
           tx_en_1d  <= 1'b0;
           tx_en_2d  <= 1'b0;
           tx_er_1d  <= 1'b0;
           tx_er_2d  <= 1'b0;
           txd_1d  <= 8'b0;
           txd_fe_B  <= 8'b0;
	end
	else begin

           txd_1d  <= txd;
           tx_en_1d  <= tx_en;
           tx_en_2d  <= tx_en_1d;
           tx_er_1d  <= tx_er;
           tx_er_2d  <= tx_er_1d;
	   
           byte_valid_fe  <= nib_enb_1 & tx_en_1d;
	   
	   if (tx_en && nib_cnt == 1'b0) begin
             nib_cnt <= 1'b1;
           end
	   else begin
             nib_cnt <= 1'b0;
           end

	   if (nib_enb_0) begin
              txd_fe[3:0] <= txd[3:0];
	   end
	   if (nib_enb_1) begin
              txd_fe_B[7:0] <= {txd[3:0],txd_fe[3:0]};
	   end
	end
   end // always 

  
   
// -------------------------------------------------------------------------------------------------------
// Ethernet PACKETS SINK MONITOR  
// -------------------------------------------------------------------------------------------------------
always@(posedge tx_clk) begin
        case (m_st)

             IDLE:
                begin
		  byt_cnt = 0;
                  crc_reg = 32'hffff_ffff;
                  crc_reg_latched = 32'hffff_ffff;
                  tx_error_latched = 1'b0;
                  control_pkt <= 1'b0;
                  pause_pkt <= 1'b0;
                  vlan_pkt <= 1'b0;
                  op_code_B0 <= 8'd0;
                  vlan_len_B0 <= 8'd0;

		    if (gbit_en) begin
                     `ifdef SGMII_TSMAC
                       if (tx_en == 1 && byt_cnt == 0) begin
		   	   byt_cnt <= byt_cnt + 1;
			   pkt_cnt <= pkt_cnt + 1;
                           $fdisplay(out_file, "-----------------------------------------------");
                           $fdisplay(out_file, "PKT %d \t BYTE \t DATA \t TX_EN \t TX_ER", pkt_cnt);
                           $fdisplay(out_file, "-----------------------------------------------");
                           m_st = PREMB;
		       end
	             `else // CLASSIC_TSMAC OR GBE_MAC
                       if (tx_en == 1 && byt_cnt == 0) begin
		  	   byt_cnt <= byt_cnt + 1;
			   pkt_cnt <= pkt_cnt + 1;
                           $fdisplay(out_file, "-----------------------------------------------");
                           $fdisplay(out_file, "PKT %d \t BYTE \t DATA \t TX_EN \t TX_ER", pkt_cnt);
                           $fdisplay(out_file, "-----------------------------------------------");
                           m_st = PREMB;
		       end
		     `endif

                    end
		    else begin
		  
                       if (tx_en_1d == 1 && byt_cnt == 0) begin
		 	  byt_cnt <= byt_cnt + 1;
			  pkt_cnt <= pkt_cnt + 1;
                          $fdisplay(out_file, "-----------------------------------------------");
                          $fdisplay(out_file, "PKT %d \t BYTE \t DATA \t TX_EN \t TX_ER", pkt_cnt);
                          $fdisplay(out_file, "-----------------------------------------------");
                          m_st = PREMB;
		       end

                    end
                end
             PREMB:
                begin

		    if (gbit_en) begin

                     `ifdef SGMII_TSMAC
                       if (tx_en_1d && txd_1d[7:0] != 8'hd5 && tx_clk_en == 1) begin
		           byt_cnt <= byt_cnt + 1;
                           $fdisplay(out_file, "preamble \t %d \t %h \t %b \t %b",byt_cnt,txd_1d,tx_en_1d,tx_er_1d);
                           m_st = PREMB;
                       end
		       else if (tx_en_1d && txd_1d[7:0] == 8'hd5 && tx_clk_en == 1) begin
		           byt_cnt <= 1;
                           $fdisplay(out_file, "SFD \t \t \t %h \t %b \t %b",txd_1d,tx_en_1d,tx_er_1d);
                           m_st = PKT_BODY;
                       end
	             `else // CLASSIC_TSMAC OR GBE_MAC
                       if (tx_en_1d && txd_1d[7:0] != 8'hd5) begin
		           byt_cnt <= byt_cnt + 1;
                           $fdisplay(out_file, "preamble \t %d \t %h \t %b \t %b",byt_cnt,txd_1d,tx_en_1d,tx_er_1d);
                           m_st = PREMB;
                       end
		       else if (tx_en_1d && txd_1d[7:0] == 8'hd5) begin
		           byt_cnt <= 1;
                           $fdisplay(out_file, "SFD \t \t \t %h \t %b \t %b",txd_1d,tx_en_1d,tx_er_1d);
                           m_st = PKT_BODY;
                       end
		     `endif

                    end
		    else begin
			    
                      if (tx_en_2d && txd_fe_B[7:0] != 8'hd5) begin
                        if (byte_valid_fe) begin
		          byt_cnt <= byt_cnt + 1;
                          $fdisplay(out_file, "preamble \t %d \t %h \t %b \t %b",byt_cnt,txd_fe_B,tx_en_2d,tx_er_2d);
                          m_st = PREMB;
                        end
                      end
		      else if (tx_en_2d && txd_fe_B[7:0] == 8'hd5) begin
                        if (byte_valid_fe) begin
		          byt_cnt <= 1;
                          $fdisplay(out_file, "SFD \t \t \t %h \t %b \t %b",txd_fe_B,tx_en_2d,tx_er_2d);
                          m_st = PKT_BODY;
                        end
                      end

                    end

                end
             PKT_BODY:
                begin

		    if (gbit_en) begin

                     `ifdef SGMII_TSMAC
		       if (tx_en_1d && tx_clk_en == 1) begin	
	             `else // CLASSIC_TSMAC OR GBE_MAC
                       if (tx_en_1d) begin
	             `endif
		           byt_cnt <= byt_cnt + 1;
                           m_st = PKT_BODY;
		           
			   if (byt_cnt == 12'd1) begin
                             crc_reg = nextCRC32_D8(txd_1d, CRC_INIT_VALUE);
                           end
			   else if (byt_cnt == (len_B0B1 + 12'd15)) begin
                              crc_reg_latched = crc_rev(crc_reg);
                           end
			   else begin
                             crc_reg = nextCRC32_D8(txd_1d, crc_reg);
                           end
			   
			   if (byt_cnt == 12'd13) begin
				  len_B0 <= txd_1d;
			   end 
			   if (byt_cnt == 12'd15 && control_pkt) begin
				  op_code_B0 <= txd_1d;
			   end 
			   if (byt_cnt == 12'd17 && vlan_pkt) begin
				  vlan_len_B0 <= txd_1d;
			   end 
			   if (byt_cnt == 12'd14) begin
				   if ({len_B0,txd_1d} < 16'd46) begin // short pkt 
				     len_B0B1 <= 16'd46;
			           end
				   else if ({len_B0,txd_1d} == 16'h8808) begin // control pkt 
				     control_pkt <= 1'b1;
			           end
				   else if ({len_B0,txd_1d} == 16'h8100) begin // vlan tagged pkt 
				     vlan_pkt <= 1'b1;
			           end
				   else begin
				     len_B0B1 <= {len_B0,txd_1d};
			           end
			   end
			   else if (byt_cnt == 12'd16 && control_pkt) begin
				   if ({op_code_B0,txd_1d} == 16'h0001) begin // pause pkt
				     pause_pkt <= 1'b1; 
				     len_B0B1 <= 16'd46;
			           end
			   end
			   else if (byt_cnt == 12'd18 && vlan_pkt) begin
				   len_B0B1 <= {vlan_len_B0,txd_1d} + 16'd4;
			   end
			   
			   if (byt_cnt == (len_B0B1 + 12'd15)) begin
				  Rx_CRC_B0 <= txd_1d;
			   end 
			   if (byt_cnt == (len_B0B1 + 12'd16)) begin
				  Rx_CRC_B1 <= txd_1d;
			   end
			   if (byt_cnt == (len_B0B1 + 12'd17)) begin
				  Rx_CRC_B2 <= txd_1d;
			   end
			   if (byt_cnt == (len_B0B1 + 12'd18)) begin
				  Rx_CRC <= {txd_1d,Rx_CRC_B2,Rx_CRC_B1,Rx_CRC_B0};
			   end
			   
			   if (byt_cnt >= 12'd1 && byt_cnt <= 12'd6 ) begin
                              $fdisplay(out_file, "DA \t \t %d \t %h \t %b \t %b",byt_cnt,txd_1d,tx_en_1d,tx_er_1d);
			   end
			   else if (byt_cnt >= 12'd7 && byt_cnt <= 12'd12 ) begin
                              $fdisplay(out_file, "SA \t \t %d \t %h \t %b \t %b",byt_cnt,txd_1d,tx_en_1d,tx_er_1d);
			   end
			   else if (byt_cnt >= 12'd13 && byt_cnt <= 12'd14 ) begin
			      if ((byt_cnt == 12'd13 && txd_1d == 8'h81) 
			           || (byt_cnt == 12'd14 && {len_B0,txd_1d} == 16'h8100)) begin
                                $fdisplay(out_file, "TAG \t \t %d \t %h \t %b \t %b",byt_cnt,txd_1d,tx_en_1d,tx_er_1d);
			      end
			      else begin
                                $fdisplay(out_file, "LT \t \t %d \t %h \t %b \t %b",byt_cnt,txd_1d,tx_en_1d,tx_er_1d);
			      end
			   end
			   else if ((byt_cnt >= 12'd15 && byt_cnt <= 12'd16) && vlan_pkt ) begin
                              $fdisplay(out_file, "TAGC \t \t %d \t %h \t %b \t %b",byt_cnt,txd_1d,tx_en_1d,tx_er_1d);
			   end
			   else if ((byt_cnt >= 12'd17 && byt_cnt <= 12'd18) && vlan_pkt ) begin
                              $fdisplay(out_file, "LT \t \t %d \t %h \t %b \t %b",byt_cnt,txd_1d,tx_en_1d,tx_er_1d);
			   end
			   else if (byt_cnt >= (len_B0B1 + 12'd15) && byt_cnt <= (len_B0B1 + 12'd18) ) begin
                              $fdisplay(out_file, "CRC \t \t %d \t %h \t %b \t %b",byt_cnt,txd_1d,tx_en_1d,tx_er_1d);
			   end
			   else begin
                              $fdisplay(out_file, "Data \t \t %d \t %h \t %b \t %b",byt_cnt,txd_1d,tx_en_1d,tx_er_1d);
			   end
		      
			   if (tx_er_1d == 1'b1) begin
				  tx_error_latched <= 1'b1;
			   end
			   
                       end
                     `ifdef SGMII_TSMAC
		       else if (tx_en_1d == 0 && tx_clk_en == 1) begin	
	             `else // CLASSIC_TSMAC OR GBE_MAC
		       else if (tx_en_1d == 0) begin
		     `endif
		           byt_cnt <= 0;
                           $fdisplay(out_file, "-----------------------------------------------");
                           $fdisplay(out_file, "RECEIVED CRC \t \t %h ",Rx_CRC);
                           $fdisplay(out_file, "EXPECTED CRC \t \t %h ",crc_reg_latched);
			   if ( Rx_CRC == crc_reg_latched && !tx_error_latched) begin
                                  $fdisplay(out_file, "GOOD PKT");
			   end
			   else begin
                                  $fdisplay(out_file, "BAD PKT");
			   end
			   if (pause_pkt) begin
                                  $fdisplay(out_file, "THIS IS A PAUSE PACKET");
			   end
			   else if (vlan_pkt) begin
                                  $fdisplay(out_file, "THIS IS A VLAN TAGGED PACKET");
			   end
                           $fdisplay(out_file, " ");
                           $fdisplay(out_file, " ");
                           $fdisplay(out_file, " ");
                           m_st = IDLE;
                       end
			
                    end // (gbit_en)
		    else begin  // gbit_en == 0 (10/100 mode)
                      
		        if (tx_en_2d) begin
			      
			   if (byte_valid_fe) begin
				   
		              byt_cnt <= byt_cnt + 1;
                              m_st = PKT_BODY;
		           
			      if (byt_cnt == 12'd1) begin
                                crc_reg = nextCRC32_D8(txd_fe_B, CRC_INIT_VALUE);
                              end
			      else if (byt_cnt == (len_B0B1 + 12'd15)) begin
                                crc_reg_latched = crc_rev(crc_reg);
                              end
			      else begin
                                crc_reg = nextCRC32_D8(txd_fe_B, crc_reg);
                              end
			   

                             if (byt_cnt == 12'd13) begin
				  len_B0 <= txd_fe_B;
			     end 
			     if (byt_cnt == 12'd15 && control_pkt) begin
			 	    op_code_B0 <= txd_1d;
			     end 
                             if (byt_cnt == 12'd17 && vlan_pkt) begin
				  vlan_len_B0 <= txd_fe_B;
			     end 
			     if (byt_cnt == 12'd14) begin
				     if ({len_B0,txd_fe_B} < 16'd46) begin // short pkt 
				       len_B0B1 <= 16'd46;
			             end
				     else if ({len_B0,txd_fe_B} == 16'h8808) begin // control pkt 
				       control_pkt <= 1'b1;
			             end
				     else if ({len_B0,txd_fe_B} == 16'h8100) begin // vlan tagged pkt 
				       vlan_pkt <= 1'b1;
			             end
				     else begin
				       len_B0B1 <= {len_B0,txd_fe_B};
			             end
			     end
			     else if (byt_cnt == 12'd16 && control_pkt) begin
				     if ({op_code_B0,txd_fe_B} == 16'h0001) begin // pause pkt
				       pause_pkt <= 1'b1; 
				       len_B0B1 <= 16'd46;
			             end
			     end
                             else if (byt_cnt == 12'd18 && vlan_pkt) begin
				   len_B0B1 <= {vlan_len_B0,txd_fe_B} + 16'd4;
			     end

			      
			      if (byt_cnt == (len_B0B1 + 12'd15)) begin
				  Rx_CRC_B0 <= txd_fe_B;
			      end 
			      if (byt_cnt == (len_B0B1 + 12'd16)) begin
				  Rx_CRC_B1 <= txd_fe_B;
			      end
			      if (byt_cnt == (len_B0B1 + 12'd17)) begin
				  Rx_CRC_B2 <= txd_fe_B;
			      end
			      if (byt_cnt == (len_B0B1 + 12'd18)) begin
				  Rx_CRC <= {txd_fe_B,Rx_CRC_B2,Rx_CRC_B1,Rx_CRC_B0};
			      end
			   
			      if (byt_cnt >= 12'd1 && byt_cnt <= 12'd6 ) begin
                                 $fdisplay(out_file, "DA \t \t %d \t %h \t %b \t %b",byt_cnt,txd_fe_B,tx_en_2d,tx_er_2d);
			      end
			      else if (byt_cnt >= 12'd7 && byt_cnt <= 12'd12 ) begin
                                 $fdisplay(out_file, "SA \t \t %d \t %h \t %b \t %b",byt_cnt,txd_fe_B,tx_en_2d,tx_er_2d);
			      end
			      else if (byt_cnt >= 12'd13 && byt_cnt <= 12'd14 ) begin
			         if ((byt_cnt == 12'd13 && txd_fe_B == 8'h81) 
			           || (byt_cnt == 12'd14 && {len_B0,txd_fe_B} == 16'h8100)) begin
                                   $fdisplay(out_file, "TAG \t \t %d \t %h \t %b \t %b",byt_cnt,txd_fe_B,tx_en_2d,tx_er_2d);
			         end
			         else begin
                                   $fdisplay(out_file, "LT \t \t %d \t %h \t %b \t %b",byt_cnt,txd_fe_B,tx_en_2d,tx_er_2d);
			         end
			      end
			      else if ((byt_cnt >= 12'd15 && byt_cnt <= 12'd16) && vlan_pkt ) begin
                                 $fdisplay(out_file, "TAGC \t \t %d \t %h \t %b \t %b",byt_cnt,txd_fe_B,tx_en_2d,tx_er_2d);
			      end
			      else if ((byt_cnt >= 12'd17 && byt_cnt <= 12'd18) && vlan_pkt ) begin
                                 $fdisplay(out_file, "LT \t \t %d \t %h \t %b \t %b",byt_cnt,txd_fe_B,tx_en_2d,tx_er_2d);
			      end
			      else if (byt_cnt >= (len_B0B1 + 12'd15) && byt_cnt <= (len_B0B1 + 12'd18) ) begin
                                 $fdisplay(out_file, "CRC \t \t %d \t %h \t %b \t %b",byt_cnt,txd_fe_B,tx_en_2d,tx_er_2d);
			      end
			      else begin
                                 $fdisplay(out_file, "Data \t \t %d \t %h \t %b \t %b",byt_cnt,txd_fe_B,tx_en_2d,tx_er_2d);
			      end
		     
			      if (tx_er_2d == 1'b1) begin
				  tx_error_latched <= 1'b1;
			      end
			      
		           end // if (byte_valid_fe)
			   
		        end //  if (tx_en_2d)
			else if (tx_en_2d == 0) begin
                           byt_cnt <= 0;
                           $fdisplay(out_file, "-----------------------------------------------");
                           $fdisplay(out_file, "RECEIVED CRC \t \t %h ",Rx_CRC);
                           $fdisplay(out_file, "EXPECTED CRC \t \t %h ",crc_reg_latched);
			   if ( Rx_CRC == crc_reg_latched && !tx_error_latched) begin
                                  $fdisplay(out_file, "GOOD PKT");
			   end
			   else begin
                                  $fdisplay(out_file, "BAD PKT");
			   end
			   if (pause_pkt) begin
                                  $fdisplay(out_file, "THIS IS A PAUSE PACKET");
			   end
			   else if (vlan_pkt) begin
                                  $fdisplay(out_file, "THIS IS A VLAN TAGGED PACKET");
			   end
                           $fdisplay(out_file, " ");
                           $fdisplay(out_file, " ");
                           $fdisplay(out_file, " ");
                           m_st = IDLE;
		        end //  if (tx_en_2d == 0)
			
                    end // gbit_en == 0 (10/100 mode)
		   
                end // PKT_BODY
        endcase //case (m_st)

end //always@( posedge tx_clk) begin


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
	
endmodule

