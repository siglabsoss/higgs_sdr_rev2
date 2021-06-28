
/*

IMPORTANT NOTES:

A. SOFTWARE RESET COMMAND: 
Restores the device to initial power-up state. 
Since hardware reset is not conencted, the volatile freeze-bit and PPB lock bit if set cannot be cleared with software reset
the non-volatile bits still retain their previous state.
Block Protection bits BP[2:0] will only reset if they are configured as volatile via BPNV bit in CR[3] and freeze bit is set to 0.

B. MODE BIT RESET COMMAND:
Returns device from high performance read mode back to standby awaiting new command. 
Some devices lack hardware reset input or is not available.
It is recommended to use the MBR command after a system reset when the
RESET# signal is not available or, before sending a software reset, to ensure the device is released from continuous high
performance read mode.
The MBR command sends

C. For FL-S devices at 256 Mbits or higher density, the traditional SPI 3-byte addresses are unable to directly address all 
locations in the memory array. These device have a bank address register that is used with 3-byte address commands to supply
the high order address bits beyond the address from the host system. The default bank address is zero. Commands are provided
to load and read the bank address register. These devices may also be configured to take a 4-byte address from the host system 
with the traditional 3-byte address commands. The 4-byte address mode for traditional commands is activated by setting the 
External Address (EXTADD) bit in the bank address register to 1.

*/

module s25fl_qspi_reader #(
    parameter int unsigned P_256MBIT_OR_LARGER = 0 // 0 for flash densities <= 128Mbit flash and 1 for flashes densities >= 256Mbit
)(
    /* USER INTERFACE */
    
    input                                         i_clk,
    input                                         i_srst,               // synchronous reset
    input                                         i_rd_start,           // 1 = start reading
    input      [(24+(8*P_256MBIT_OR_LARGER))-1:0] i_rd_start_byte_addr, // byte address to start reading bytes from
    input      [(24+(8*P_256MBIT_OR_LARGER))-1:0] i_rd_num_bytes,       // number of bytes to read 
    input                                         i_rd_pause,           // 1 = hold off reading of additional bytes (i.e. tie to FIFO full)
    output reg                                    o_rd_busy,
    output reg                                    o_rd_byte_vld,
    output reg [7:0]                              o_rd_byte,

    /* QSPI INTERFACE */

    output reg                                    o_qspi_clk,
    output reg                                    o_qspi_sel_n,
    output reg                                    o_qspi_d0_high_z,
    input                                         i_qspi_d3,
    input                                         i_qspi_d2,
    input                                         i_qspi_d1,
    input                                         i_qspi_d0,
    output                                        o_qspi_d0

);
    
    /* PARAMETER RANGE CHECKS */
    
    initial begin
        assert ((P_256MBIT_OR_LARGER == 0) || (P_256MBIT_OR_LARGER == 1)) else $fatal(0, "Error! P_256MBIT_OR_LARGER MUST BE EITHER 0 (128Mbit FLASH OR SMALLER) OR 1 (256Mbit FLASH OR LARGER)");
    end


    /****** LOCAL PARAMETERS ******/

    localparam SPI_ADDR_BITS           = (P_256MBIT_OR_LARGER) ? 32 : 24;
    localparam SPI_CMD_BITS            = 8;
    localparam SPI_CMD_DATA_BITS       = 8;
    localparam SPI_DOUT_MAX_BITS       = SPI_CMD_BITS + SPI_CMD_DATA_BITS + SPI_ADDR_BITS;
    localparam SPI_DOUT_CNTR_BITS      = $clog2(SPI_DOUT_MAX_BITS);
    localparam SPI_CLK_CYCLE_CNTR_BITS = 4; // see datasheet, max count = "1111" binary

    /* FLASH CHIP COMMANDS */
    localparam logic [SPI_CMD_BITS - 1:0] MODE_BIT_RESET_CMD              = 8'hFF; // returns device to standby mode and recommended in case hardware reset is not available
    localparam logic [SPI_CMD_BITS - 1:0] SOFTWARE_RESET_CMD              = 8'hF0; // puts volatile registers in default state except freeze and PBB lock bits in configuration register
    localparam logic [SPI_CMD_BITS - 1:0] WRITE_ENABLE_CMD                = 8'h06; // Sets WEL bit in SR1 to 1
    localparam logic [SPI_CMD_BITS - 1:0] WRITE_CONFIG_REGISTER_CMD       = 8'h01; // Write configuration registers
    localparam logic [SPI_CMD_BITS - 1:0] WRITE_BANK_ADDR_REG             = 8'h17;
    localparam logic [SPI_CMD_BITS - 1:0] QUAD_OUTPUT_FAST_READ_CMD       = 8'h6b;
    localparam logic [SPI_CMD_BITS - 1:0] QUAD_OUTPUT_FAST_4BYTE_READ_CMD = 8'h6c; // use instead of QUAD_OUTPUT_FAST_READ_CMD when flash density > 128Mbit


    /* 
     * These values for configuration registers
     * should put the device into its default state regardless of how the non-volatile
     * counterpart registers were set.
     *
     */ 

    localparam logic [7:0] CONFIG_REGISTER_VAL  = 8'b11_0_0_0_0_1_0;  // Latency Code:11(Quad I/O Read <=50 MHz) , Block Protection, RFU, BPNV, RFU, Quad, FREEZE
    localparam logic [7:0] STATUS_REG1_VAL      = 8'b0_0_0_000_0_0;   // * need to write SR1 in order to write to config registers hence keeping the status registers at default state
                                                                      // * the WIP & WEL bit are not affected by Write Register command
                                                                      // * Pg 71 of the datasheet
    localparam logic [7:0] BANK_REG_ADDR_VAL    = 8'b1_00000_0_0;     // 4-byte addressing from command[7], RFU[6:2], BA[1:0] as default
                                                                                  
    /****** SIGNALS ******/


    logic [SPI_ADDR_BITS - 1:0] rd_byte_addr_reg;
    logic [SPI_ADDR_BITS - 1:0] rd_num_bytes_reg;

    logic rd_start_reg;   // used to create rising edge pulse
    logic rd_start_pulse;

    typedef enum {
        IDLE, 
        MODE_BIT_RESET, 
        SOFTWARE_RESET, 
        ISSUE_WRITE_ENABLE_CMD,             // must be done prior to writing the volatile config reg, enhanced volatile config reg, and entering 4-byte address mode
        NON_READ_DESELECT_DELAY,
        SOFTWARE_RESET_RECOVERY_DELAY,
        WRITE_CONFIG_REG,
        //WRITE_EVOLATILE_CONFIG_REG,         // enhanced volatile config reg
        ISSUE_ENTER_4BYTE_ADDRESS_MODE_CMD, // only needed when the flash density is > 128Mbit (write enable command must be issued prior to issuing this command)
        ISSUE_QUAD_OUTPUT_FAST_RD_CMD,
        ISSUE_DUMMY_CYCLE_CLOCKS,
        READ_HIGH_NIBBLE,
        READ_LOW_NIBBLE,
        POST_READ_DESELECT_DELAY
    } flash_rd_fsm_states_t;

    flash_rd_fsm_states_t flash_rd_fsm_state;
    flash_rd_fsm_states_t flash_rd_fsm_next_state;
    flash_rd_fsm_states_t flash_rd_fsm_next_next_state;

    logic [3:0] qspi_din_regs;


    // for flash read fsm
    logic [SPI_DOUT_MAX_BITS       - 1:0] flash_dout_reg;
    logic [SPI_DOUT_MAX_BITS       - 1:0] flash_dout_next_reg;
    logic [SPI_DOUT_MAX_BITS       - 1:0] flash_dout_next_next_reg;
    logic [SPI_DOUT_CNTR_BITS      - 1:0] flash_dout_cntr;
    logic [SPI_ADDR_BITS           - 1:0] flash_byte_cntr; 
    logic [SPI_CLK_CYCLE_CNTR_BITS - 1:0] flash_clk_cycle_cntr;
    logic                               die_crossing_flag; 

 
    /****** COMBINATIONAL LOGIC ******/


    assign rd_start_pulse = i_rd_start & ~rd_start_reg;

    assign o_qspi_d0 = flash_dout_reg[SPI_DOUT_MAX_BITS - 1];


    /****** SEQUENTIAL LOGIC ******/


    always_ff @ (posedge i_clk) begin
        rd_start_reg <= i_rd_start;
    end

    always_ff @ (posedge i_clk) begin
        qspi_din_regs <= {i_qspi_d3, i_qspi_d2, i_qspi_d1, i_qspi_d0};
    end


    /*
     * FSM Operation:
     *
     * 1. Wait for read start command
     * 2. Reset the flash
     * 3. Issue Write Enable command
     * 4. Write volatile and enhanced volatile configuration registers to put flash into known state
     * 5. Issue quad output fast read command (4-byte when flash density > 128Mbit, otherwise 3-byte)
     * 6. Read out requested number of bytes, presenting each one to the user, until a 256Mbit die boundary is hit or all bytes are read.  If a 256Mbit die boundary is hit then go back to step 5.
     *
     */
    
    always_ff @ (posedge i_clk) begin : FLASH_RD_FSM

        if (i_srst) begin
            o_rd_byte_vld      <= 0;
            o_rd_byte          <= 8'h0;
            o_rd_busy          <= 0;
            o_qspi_clk         <= 0;
            o_qspi_sel_n       <= 1;
            o_qspi_d0_high_z   <= 1;
            die_crossing_flag  <= 0;
            flash_rd_fsm_state <= IDLE;
        
        end else begin

            case (flash_rd_fsm_state)

                IDLE: begin

                    o_rd_busy       <= 0;
                    o_qspi_sel_n    <= 1;
                    flash_byte_cntr <= '0;
            
                    if ( (rd_start_pulse == 1) && (i_rd_num_bytes != 0) ) begin

                        rd_byte_addr_reg    <= i_rd_start_byte_addr;
                        rd_num_bytes_reg    <= i_rd_num_bytes;
                        o_rd_busy           <= 1;
                        o_qspi_sel_n        <= 0;
                        o_qspi_d0_high_z    <= 0;
                        flash_dout_reg      <= {MODE_BIT_RESET_CMD, {(SPI_DOUT_MAX_BITS-SPI_CMD_BITS){1'b0}}};
                        flash_dout_cntr     <= '0;
                        flash_rd_fsm_state  <= MODE_BIT_RESET;

                    end
                end


                /* sends the reset enable command to the flash */
                MODE_BIT_RESET: begin

                    if (o_qspi_clk) begin

                        o_qspi_clk      <= 0;
                        flash_dout_reg  <= flash_dout_reg << 1;
                        flash_dout_cntr <= flash_dout_cntr + 1;

                        if (flash_dout_cntr == (SPI_CMD_BITS - 1)) begin
                            o_qspi_sel_n            <= 1;
                            flash_clk_cycle_cntr    <= '0;
                            flash_dout_next_reg     <= {SOFTWARE_RESET_CMD, {(SPI_DOUT_MAX_BITS - SPI_CMD_BITS){1'b0}}};
                            flash_rd_fsm_state      <= NON_READ_DESELECT_DELAY;
                            flash_rd_fsm_next_state <= SOFTWARE_RESET;
                        end

                    end 

                    else begin
                        o_qspi_clk <= 1;
                    end
                end


                /* 
                 * Ensures that the minimum deselect time after nonRead command (tshsl2) is met.
                 *
                 * NOTE: ASSUMES THAT INPUT CLOCK IS RESTRICTED ACCORDING TO COMMENTS AT TOP OF FILE
                 *
                 */
                NON_READ_DESELECT_DELAY: begin

                    flash_clk_cycle_cntr <= flash_clk_cycle_cntr + 1;
                    flash_dout_cntr      <= '0;
                    flash_dout_reg       <= flash_dout_next_reg;

                    if (flash_clk_cycle_cntr == {SPI_CLK_CYCLE_CNTR_BITS{1'b1}}) begin
                        o_qspi_sel_n       <= 0;
                        flash_rd_fsm_state <= flash_rd_fsm_next_state; 
                    end
                end


                /* sends the reset memory command to the flash */
                SOFTWARE_RESET: begin

                    if (o_qspi_clk) begin

                        o_qspi_clk      <= 0;
                        flash_dout_reg  <= flash_dout_reg << 1;
                        flash_dout_cntr <= flash_dout_cntr + 1;

                        if (flash_dout_cntr == (SPI_CMD_BITS - 1)) begin
                            o_qspi_sel_n         <= 1;
                            flash_clk_cycle_cntr <= {SPI_CLK_CYCLE_CNTR_BITS{1'b0}};
                            flash_rd_fsm_state   <= SOFTWARE_RESET_RECOVERY_DELAY;
                        end

                    end else begin
                        o_qspi_clk <= 1;
                    end
                end


                /* 
                 * Ensures that the minimum software reset recovery time (tshsl3) is met.
                 *
                 * NOTE: ASSUMES THAT INPUT CLOCK IS RESTRICTED ACCORDING TO COMMENTS AT TOP OF FILE
                 *
                 */
                SOFTWARE_RESET_RECOVERY_DELAY: begin

                    flash_clk_cycle_cntr <= flash_clk_cycle_cntr + 1;

                    if (flash_clk_cycle_cntr == {SPI_CLK_CYCLE_CNTR_BITS{1'b1}}) begin
                        o_qspi_sel_n                 <=  0;
                        flash_dout_cntr              <= '0;
                        flash_dout_next_next_reg     <= {WRITE_CONFIG_REGISTER_CMD, STATUS_REG1_VAL, CONFIG_REGISTER_VAL, {(SPI_DOUT_MAX_BITS - SPI_CMD_BITS - SPI_CMD_DATA_BITS -SPI_CMD_DATA_BITS){1'b0}}};
                        flash_rd_fsm_next_next_state <= WRITE_CONFIG_REG;
                        flash_dout_reg               <= {WRITE_ENABLE_CMD, {(SPI_DOUT_MAX_BITS - SPI_CMD_BITS){1'b0}}};
                        flash_rd_fsm_state           <= ISSUE_WRITE_ENABLE_CMD; 
                    end
                end

 
                /* sends the write enable command to the flash */
                ISSUE_WRITE_ENABLE_CMD: begin

                    if (o_qspi_clk) begin

                        o_qspi_clk      <= 0;
                        flash_dout_reg  <= flash_dout_reg << 1;
                        flash_dout_cntr <= flash_dout_cntr + 1;

                        if (flash_dout_cntr == (SPI_CMD_BITS - 1)) begin
                            o_qspi_sel_n            <=  1;
                            flash_clk_cycle_cntr    <= '0;
                            flash_dout_next_reg     <= flash_dout_next_next_reg;
                            flash_rd_fsm_next_state <= flash_rd_fsm_next_next_state;
                            flash_rd_fsm_state      <= NON_READ_DESELECT_DELAY;
                        end

                    end else begin
                        o_qspi_clk <= 1;
                    end
                end


                /* writes the volatile configuration register of the flash */
                WRITE_CONFIG_REG: begin

                    if (o_qspi_clk) begin

                        o_qspi_clk      <= 0;
                        flash_dout_reg  <= flash_dout_reg << 1;
                        flash_dout_cntr <= flash_dout_cntr + 1;

                        if (flash_dout_cntr == (SPI_CMD_BITS + SPI_CMD_DATA_BITS + SPI_CMD_DATA_BITS - 1)) begin // wait for 16 clock cycles to allow status register & config register write.
                            o_qspi_sel_n            <= 1;
                            flash_clk_cycle_cntr    <= '0; 
                            flash_dout_next_reg     <= {QUAD_OUTPUT_FAST_READ_CMD, rd_byte_addr_reg, {(SPI_DOUT_MAX_BITS - SPI_CMD_BITS - SPI_ADDR_BITS){1'b0}}}; // 128Mbit or smaller flash so issue regular 3-byte quad output fast read command
                            flash_rd_fsm_next_state <= ISSUE_QUAD_OUTPUT_FAST_RD_CMD;
                            flash_rd_fsm_state      <= NON_READ_DESELECT_DELAY;
                        end
                        if(P_256MBIT_OR_LARGER) begin
                            flash_dout_next_next_reg     <= {WRITE_BANK_ADDR_REG, BANK_REG_ADDR_VAL {(SPI_DOUT_MAX_BITS-SPI_CMD_BITS-SPI_CMD_DATA_BITS){1'b0}}}; // command to set 4-byte addressing
                            flash_dout_next_reg          <= {WRITE_ENABLE_CMD, {(SPI_DOUT_MAX_BITS - SPI_CMD_BITS){1'b0}}};
                            flash_rd_fsm_next_next_state <= ISSUE_ENTER_4BYTE_ADDRESS_MODE_CMD;
                            flash_rd_fsm_next_state      <= ISSUE_WRITE_ENABLE_CMD;


                    end else begin
                        o_qspi_clk <= 1;
                    end
                end                
                
                /* issues the enter 4-byte address mode command to the flash (only needed when the flash density is > 128Mbit) */
                ISSUE_ENTER_4BYTE_ADDRESS_MODE_CMD: begin

                    if (o_qspi_clk) begin

                        o_qspi_clk      <= 0;
                        flash_dout_reg  <= flash_dout_reg << 1;
                        flash_dout_cntr <= flash_dout_cntr + 1;

                        if (flash_dout_cntr == (SPI_CMD_BITS - 1)) begin
                            o_qspi_sel_n            <= 1;
                            flash_clk_cycle_cntr    <= '0;
                            flash_dout_next_reg     <= {QUAD_OUTPUT_FAST_4BYTE_READ_CMD, rd_byte_addr_reg, {(SPI_DOUT_MAX_BITS - SPI_CMD_BITS - SPI_ADDR_BITS){1'b0}}}; // 256Mbit or larger flash so issue 4-byte quad output fast read command
                            flash_rd_fsm_next_state <= ISSUE_QUAD_OUTPUT_FAST_RD_CMD;
                            flash_rd_fsm_state      <= NON_READ_DESELECT_DELAY;
                        end

                    end else begin
                        o_qspi_clk <= 1;
                    end
                end
                    

                /* issues the quad output fast read command + address to the flash */
                ISSUE_QUAD_OUTPUT_FAST_RD_CMD: begin
                    
                    die_crossing_flag <= 0;

                    if (o_qspi_clk) begin

                        o_qspi_clk      <= 0;
                        flash_dout_reg  <= flash_dout_reg << 1;
                        flash_dout_cntr <= flash_dout_cntr + 1;

                        if (flash_dout_cntr == (SPI_CMD_BITS + SPI_ADDR_BITS - 1)) begin
                            o_qspi_d0_high_z     <= 1;
                            flash_clk_cycle_cntr <= '0;
                            flash_rd_fsm_state   <= ISSUE_DUMMY_CYCLE_CLOCKS;
                        end

                    end else begin
                        o_qspi_clk <= 1;
                    end
                end

                /* issues dummy cycle clocks to the flash */
                ISSUE_DUMMY_CYCLE_CLOCKS: begin

                    if (o_qspi_clk) begin

                        o_qspi_clk <= 0;

                        /* see datasheet Quad Output Fast and 4-Byte Quad Output Fast commands and their respective notes */
                        if (flash_clk_cycle_cntr == 4'h8) begin 
                            flash_rd_fsm_state <= READ_HIGH_NIBBLE;
                        end

                    end else begin
                        o_qspi_clk           <= 1;
                        flash_clk_cycle_cntr <= flash_clk_cycle_cntr + 1;
                    end
                end


                /*
                 * Captures the upper 4-bits of the incoming byte from the flash.
                 *
                 * NOTE: FLASH PRESENTS DATA ON FALLING EDGE OF SPI CLOCK (i.e. on the falling edge of the last dummy clock)
                 *
                 * SEE READ MEMORY OPERATIONS TIMING SECTION IN DATASHEET
                 *
                 */
                READ_HIGH_NIBBLE: begin

                    flash_clk_cycle_cntr <= '0; 

                    /* make sure that i_rd_pause and o_rd_byte_vld didn't simultaneously go high */
                    if ((~i_rd_pause) | (~o_rd_byte_vld)) begin 

                        o_rd_byte_vld <= 0;

                        if (o_qspi_clk) begin
                            o_qspi_clk         <= 0;
                            o_rd_byte[7:4]     <= qspi_din_regs; 
                            flash_rd_fsm_state <= READ_LOW_NIBBLE;
                        end else begin
                            
                            if (flash_byte_cntr == rd_num_bytes_reg) begin // done!
                                o_qspi_sel_n            <= 1;
                                flash_rd_fsm_next_state <= IDLE;
                                flash_rd_fsm_state      <= POST_READ_DESELECT_DELAY;
                                
                            // de-select the flash and issue another quad output fast read command because we've hit the end of the current 256Mbit die
                            end else if (die_crossing_flag) begin 
                                o_qspi_sel_n            <= 1;
                                flash_dout_next_reg     <= {QUAD_OUTPUT_FAST_4BYTE_READ_CMD, rd_byte_addr_reg, {(SPI_DOUT_MAX_BITS - SPI_CMD_BITS - SPI_ADDR_BITS){1'b0}}}; // 256Mbit or larger flash so issue 4-byte quad output fast read command
                                flash_rd_fsm_next_state <= ISSUE_QUAD_OUTPUT_FAST_RD_CMD;
                                flash_rd_fsm_state      <= POST_READ_DESELECT_DELAY;
                            end else begin
                                o_qspi_clk <= 1;
                            end

                        end
                    end
                end

                /* 
                 * captures the lower 4-bits of the incoming byte from the flash and
                 * presents the completed byte to the user.
                 */
                READ_LOW_NIBBLE: begin
                    
                    if (o_qspi_clk) begin

                        o_qspi_clk         <= 0;
                        o_rd_byte[3:0]     <= qspi_din_regs; 
                        o_rd_byte_vld      <= 1;
                        die_crossing_flag  <= 0;
                        flash_byte_cntr    <= flash_byte_cntr + 1;
                        rd_byte_addr_reg   <= rd_byte_addr_reg + 1;
                        flash_rd_fsm_state <= READ_HIGH_NIBBLE;

                        /* check for 256Mbit die crossing (i.e. we're reading the last byte of the current 256Mbit die) */
                        if (P_256MBIT_OR_LARGER) begin
                            if ( (rd_byte_addr_reg == 32'h01ffffff) || (rd_byte_addr_reg == 32'h03ffffff) || (rd_byte_addr_reg == 32'h05ffffff) ) begin
                                die_crossing_flag <= 1;
                            end
                        end

                    end else begin
                        o_qspi_clk <= 1;
                    end
                end

                /* 
                 * Lets the Flash cool off for a second after that blazing fast read.
                 *
                 * I found that the Micron N25Qxxx flash model wouldn't recognize the next Reset Enable command
                 * if the flash chip select wasn't deasserted for long enough after reading the last byte. 
                 *
                 * NOTE: ASSUMES THAT INPUT CLOCK IS RESTRICTED ACCORDING TO COMMENTS AT TOP OF FILE
                 *
                 */
                POST_READ_DESELECT_DELAY: begin

                    flash_clk_cycle_cntr <= flash_clk_cycle_cntr + 1;
                    flash_dout_cntr      <= '0;
                    flash_dout_reg       <= flash_dout_next_reg;

                    if (flash_clk_cycle_cntr == {SPI_CLK_CYCLE_CNTR_BITS{1'b1}}) begin
                        flash_rd_fsm_state <= flash_rd_fsm_next_state; 
                        
                        if (flash_rd_fsm_next_state == ISSUE_QUAD_OUTPUT_FAST_RD_CMD) begin
                            o_qspi_d0_high_z <= 0;
                            o_qspi_sel_n     <= 0;
                        end
                    end
                end

                
                default: begin
                    flash_rd_fsm_state <= IDLE; 
                end

            endcase
        end
    end

endmodule