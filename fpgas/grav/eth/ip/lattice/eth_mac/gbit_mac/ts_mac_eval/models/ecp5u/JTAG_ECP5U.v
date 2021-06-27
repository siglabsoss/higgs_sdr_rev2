module JTAG_ECP5U(
	grst_ni,
	tck,
	tms,
	tdi,
	tdo,
    	PC_Clk,
	PC_Data_In,
	PC_Ready,
	PC_Reset,
    	Cnt,
	PC_Data_Out,
	PC_Ack,
	PC_Error);

  input	    grst_ni;
  input	    tck;
  input	    tms;
  input	    tdi;
  output    tdo;
  input     PC_Data_Out;
  input     PC_Ack;
  input     PC_Error;
  output    PC_Clk;
  output    PC_Data_In;
  output    PC_Ready;
  output    PC_Reset;
  output    Cnt;
  wire      JTCK;
  wire      JTDI;
  wire      JSHIFT;
  wire      JRSTN;
  wire      JCE1;
  reg       JTDI_x;
  reg       JSHIFT_x;
  reg       JRSTN_x;
  reg       JCE1_x;
  reg       JTDO1;
  reg[5:0]  Count;
  reg[2:0]  State;
  reg       JTD01_a;
  reg       JTD01_b;
  reg       PC_Ready_i;
  reg       PC_Clk_a;
  reg       PC_Clk_b;
  reg       PC_Reset;
  reg       PC_Data_In;

JTAGG JTAGG(

	  // External Pad Interface
	.TCK		(tck), 
	.TMS		(tms), 
	.TDI		(tdi),
	.TDO		(tdo),


	.JTDO1		(JTDO1),
	.JTDO2		(1'b0),
	.JTDI		(JTDI),
	.JTCK		(JTCK),
	.JRTI1		(),
	.JRTI2		(),
	.JSHIFT		(JSHIFT),
	.JUPDATE	(),
	.JRSTN		(JRSTN),
	.JCE1		(JCE1),
	.JCE2		()

// Oringal XP I/O List
        // .JTDO1  (JTDO1),
	// .JTDO2  (0),
	// .JTCK   (JTCK),
	// .JRTI1  (),
	// .JRTI2  (),
	// .JTDI   (JTDI),
	// .JSHIFT (JSHIFT),
	// .JUPDATE(),
	// .JRSTN  (JRSTN),
	// .JCE1   (JCE1),
	// .JCE2   ()


);


initial
begin
//   PC_Clk_a	= 1'b0;
//   State = 3'd0;
//   Count = 6'd0;
end

  always @(negedge JTCK or negedge grst_ni)        // Delayed Data
  begin
    if (~grst_ni) begin
      JTDI_x    <= 1'b0;
      JSHIFT_x  <= 1'b0;
      JRSTN_x   <= 1'b0;
      JCE1_x    <= 1'b0;
    end else begin
      JTDI_x    <= JTDI;
      JSHIFT_x  <= JSHIFT;
      JRSTN_x   <= JRSTN;
      JCE1_x    <= JCE1;
    end
  end

  always @(posedge JTCK or negedge grst_ni) begin        // Count
    if (~grst_ni) begin
      Count    <= 6'b0;
    end else begin    
	    if (JCE1_x == 1) begin
		if (JSHIFT_x == 0) begin
		  Count <= 0;
	  end else begin
		  Count <= Count + 1;
	  	end
	  end
    end
  end

//* kes (011006): Only states 6, 5, and 1 are actualy used. First
//* three bits of tdi provide opcode and determine selective clock
//* assertion to target below. These are always required.
always @(posedge JTCK or negedge grst_ni) begin                                    
    if (~grst_ni) begin
	State <= 3'b0;
    end else begin
	if ((Count == 1) | (Count == 2) | (Count == 3)) begin       // 0 1 0 Ph2 of Write
        State <= {State[1:0], JTDI_x};                       // 0 1 1 (Don't Care)
      end                                                      // 1 0 0 (Don't Care)
    end
  end
                                                             // 1 0 1 Ph1 of Read
                                                             // 1 1 0 Ph1 of Write
                                                             // 1 1 1 (Don't Care)

  always @(posedge JTCK or negedge grst_ni) begin
    if (~grst_ni) begin
	JTDO1 <= 1'b0;
	JTD01_b <= 1'b0;
	JTD01_a <= 1'b0;
    end else begin
      if (Count == 4) begin
          JTDO1   <= PC_Ack;
          JTD01_b <= PC_Error;
      end else begin
          JTDO1   <= JTD01_b;
          JTD01_b <= JTD01_a;
      end
      JTD01_a     <= PC_Data_Out;
    end
  end

  always @(posedge JTCK or negedge grst_ni) begin
    if (~grst_ni) begin
      PC_Data_In    <= 1'b0;
      PC_Reset    <= 1'b0;
    end else begin
	    if (Count >  3) begin
        	PC_Data_In    <= JTDI_x;              // PC_Data_In
	    end
      PC_Reset <= JRSTN_x;             // PC_Reset
    end
  end

//* kes (011006): Writes and Phase 1 read operations require the full 55+
//* (actually ~58) cycles of tdi even though many tdi bits/fields are 
//* unused & not passed on to the target. Done to simplify I guess.
//* tdi bits are selectively passed by turning on and off the clock 
//* used by  the target. Phase 2 reads are shorter - see below.
  always @(posedge JTCK or negedge grst_ni) begin
    if (~grst_ni) begin
      PC_Clk_a    <= 1'b0;
    end else begin
      if (((State    == 'b001) & (Count >   3) & (Count <= 12)) |      // Read Data (+ extra clock at 12)
          ((State    == 'b110) & (Count >  16) & (Count <= 24)) |      // Write Data
          ((State[2] ==    1 ) & (Count >  30) & (Count <= 48)) |      // Address
	  ((State    == 'b010) & (Count == 12)                  )) begin    // (extra clock at 12 for writes)
	  if (PC_Clk_a == 1) begin
            PC_Clk_a    <= 0;
          end else begin
            PC_Clk_a    <= 1;
	  end
      end
    end
  end

  always @(negedge JTCK or negedge grst_ni)
    if (~grst_ni) begin
      PC_Clk_b    <= 1'b0;
    end else begin
      PC_Clk_b <= PC_Clk_a;
    end

//* kes (011006): Ready only generted for writes and phase 1 reads. Phase2
//* reads do not require additional work inside orcastra.v other than to
//* shift data back when this models selectively provides clocks as part
//* of the phase 2 operation. It can be shorter then than write & ph1 write 
  always @(posedge JTCK or negedge grst_ni) begin
   if (~grst_ni) begin
      PC_Ready_i    <= 1'b0;
   end else begin
	if ((Count == 55) & (State[2] == 1)) begin
            PC_Ready_i    <= 1;
        end else begin
	    if (Count == 3) begin
                PC_Ready_i    <= 0;
	    end
       end
   end
 end

  assign PC_Ready = PC_Ready_i;
  assign PC_Clk   = PC_Clk_a ^ PC_Clk_b;
  assign Cnt      = Count;

endmodule
