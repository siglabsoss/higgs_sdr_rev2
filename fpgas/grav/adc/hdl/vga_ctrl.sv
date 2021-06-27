/*
 * Module: vga_ctrl
 * 
 * Allows for Software updates to the Variable Gain Amplifier (VGA) Gain control register.
 * 
 *
     ___________________________________________________________
    |ATTENUATION(dB)| GAIN (dB) |REGISTER SETTING (Address = 02h)|
    |    0             |    26         |    00h                  |
    |    1             |    25         |    01h                  |
    |    2             |    24         |    02h                  |
    |    3             |    23         |    03h                  |
    |    4             |    22         |    04h                  |
    |    5             |    21         |    05h                  |
    |    6             |    20         |    06h                  |
    |    7             |    19         |    07h                  |
    |    8             |    18         |    08h                  |
    |    9             |    17         |    09h                  |
    |    10            |    16         |    0Ah                  |
    |    11            |    15         |    0Bh                  |
    |    12            |    14         |    0Ch                  |
    |    13            |    13         |    0Dh                  |
    |    14            |    12         |    0Eh                  |
    |    15            |    11         |    0Fh                  |
    |    16            |    10         |    10h                  |
    |    17            |    9          |    11h                  |
    |    18            |    8          |    12h                  |
    |    19            |    7          |    13h                  |
    |    20            |    6          |    14h                  |
    |    21            |    5          |    15h                  |
    |    22            |    4          |    16h                  |
    |    23            |    3          |    17h                  |
    |    24            |    2          |    18h                  |
    |    25            |    1          |    19h                  |
    |    26            |    0          |    1Ah                  |
    |    27            |    –1       |    1Bh                  |
    |    28            |    –2       |    1Ch                  |
    |    29            |    –3       |    1Dh                  |
    |    30            |    –4       |    1Eh                  |
    |    31            |    –5       |    1Fh                  |
    |    32            |    –6       |    20h-3Fh              |
    -------------------------------------------------------------- 
*/
 
`default_nettype none

module vga_ctrl (
        
    input       i_clk125,
    input       i_srst,
    input       i_update_gain_ctrl, // pulse for one clock when new gain ctrl reg value ready
    input [6:0] i_gain_ctrl_val, // bit 6 = VGA power down
    
    output reg  o_cs_n,
    output reg  o_mosi,
    output reg  o_sclk,
    output reg  o_spi_busy
);
    
    logic [6:0] gain_ctrl_reg;
    logic [4:0] clk_div_cntr; // 5 bits = divide by 32 (divided by this much to try an get cleaner o_sclk and o_mosi edges)
    logic       clk_div_cntr_msb_reg;
    logic [5:0] counter;

    // second amplifier command bus (write only)
    always_ff @(posedge i_clk125) begin // NOTE: THIS USED TO RUN OFF OF 25MHz CLOCK, WHICH IS WHY I KNOW HAVE A 125MHz CLOCK DIVIDER
        
        if (i_srst) begin
            gain_ctrl_reg        <= 7'b0100000; // default from data sheet
            clk_div_cntr         <= '0;
            clk_div_cntr_msb_reg <= 0;
            counter              <= '0;
        end else begin

            // defaults
            clk_div_cntr         <= clk_div_cntr + 1;
            clk_div_cntr_msb_reg <= clk_div_cntr[$bits(clk_div_cntr)-1];
            counter              <= ( (counter != 6'd37) && (clk_div_cntr[$bits(clk_div_cntr)-1] == 1) && (clk_div_cntr_msb_reg == 0) ) ? counter + 1 : counter;
            
            // new write and we're not in the middle of currently updating the attenuation value
            if ( (i_update_gain_ctrl == 1) && (o_spi_busy == 0) ) begin
                gain_ctrl_reg <= i_gain_ctrl_val;
                clk_div_cntr  <= '0;
                counter       <= '0;
            end
        end
    end
    
    // NOTE: THIS USED TO RUN OFF OF THE NEGATIVE EDGE OF THE 25MHz CLOCK, NOW I DIVIDE DOWN THE 125MHz CLOCK BY 8 SO IT RUNS AT 15.625MHz 
    always_ff @(posedge i_clk125) begin
        
        if (i_srst) begin
            o_cs_n     <= 1;
            o_mosi     <= 0;
            o_sclk     <= 0;
            o_spi_busy <= 0;
        end else begin
            case (counter)
                0:  begin o_spi_busy <= 1; o_cs_n <= 1; o_sclk <= 0; o_mosi <= 0;                end // idle
                1:  begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= 0;                end // drive chip select and address bit A7 (not used so always 0) 
                2:  begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= 0;                end // clock in A7
                3:  begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= 0;                end // drive address bit A6 (first used address bit)
                4:  begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= 0;                end // clock in A6
                5:  begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= 0;                end // drive A5
                6:  begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= 0;                end // clock in A5
                7:  begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= 0;                end // drive A4 
                8:  begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= 0;                end // clock in A4
                9:  begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= 0;                end // drive A3 
                10: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= 0;                end // clock in A3
                11: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= 0;                end // drive A2
                12: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= 0;                end // clock in A2
                13: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= 1;                end // drive A1 
                14: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= 1;                end // clock in A1
                15: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= 0;                end // drive A0 (making the provided address = 8'h02
                16: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= 0;                end // clock in A0
                17: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= 0;                end // drive D7 (reserved, must be 1'b0)
                18: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= 0;                end // clock in D7 (reserved, must be 1'b0)
                19: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= gain_ctrl_reg[6]; end // drive D6
                20: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= gain_ctrl_reg[6]; end // clock in D6
                21: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= gain_ctrl_reg[5]; end // drive D5
                22: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= gain_ctrl_reg[5]; end // clock in D5
                23: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= gain_ctrl_reg[4]; end // drive D4
                24: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= gain_ctrl_reg[4]; end // clock in D4
                25: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= gain_ctrl_reg[3]; end // drive D3
                26: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= gain_ctrl_reg[3]; end // clock in D3
                27: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= gain_ctrl_reg[2]; end // drive D2
                28: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= gain_ctrl_reg[2]; end // clock in D2
                29: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= gain_ctrl_reg[1]; end // drive D1
                30: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= gain_ctrl_reg[1]; end // clock in D1
                31: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= gain_ctrl_reg[0]; end // drive D0
                32: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 1; o_mosi <= gain_ctrl_reg[0]; end // clock in D0
                33: begin o_spi_busy <= 1; o_cs_n <= 0; o_sclk <= 0; o_mosi <= 0;                end // last falling sclk edge
                34: begin o_spi_busy <= 1; o_cs_n <= 1; o_sclk <= 0; o_mosi <= 0;                end // done!
                35: begin o_spi_busy <= 1; o_cs_n <= 1; o_sclk <= 0; o_mosi <= 0;                end // inter-access gap time 
                36: begin o_spi_busy <= 1; o_cs_n <= 1; o_sclk <= 0; o_mosi <= 0;                end // inter-access gap time
                37: begin o_spi_busy <= 0; o_cs_n <= 1; o_sclk <= 0; o_mosi <= 0;                end
                default: begin o_spi_busy <= 0; o_cs_n <= 1; o_sclk <= 0; o_mosi <= 0;           end
            endcase
        end
    end 


endmodule

`default_nettype wire