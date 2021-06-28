/*
 * Module: tb_dac_top
 * 
 * TODO: Add module documentation
 */
 
`timescale 1ns/1ps

module tb_dac_top;
    
    logic        clk125 = 0;
    logic        led_d6;
    logic        dac_dclk;
    logic        dac_sync;
    logic        dac_frame;
    logic        dac_parity;
    logic [15:0] dac_d;
    
    initial begin
        forever #4 clk125 = ~clk125;
    end
    
    initial begin
        #100000;
        $finish();
    end

    dac_top DUT (
        .FPGA0_CLK   (clk125), 
        .LED_D6      (led_d6     ), 
        .DAC_DCLK    (dac_dclk   ), 
        .DAC_SYNC    (dac_sync   ), 
        .DAC_FRAME   (dac_frame  ), 
        .DAC_PARITY  (dac_parity ), 
        .DAC_D       (dac_d      ));

endmodule


