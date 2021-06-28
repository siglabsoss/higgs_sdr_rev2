/*
 * Module: adc_data_rx
 * 
 * TODO: Add module documentation
 */
module adc_data_rx (
        
    input         i_adc_data_clk,
    input  [ 7:0] i_adc_chan_a_data,
    input  [ 7:0] i_adc_chan_b_data,

    output        o_sample_clk,
    output [15:0] o_chan_a_sample_0, 
    output [15:0] o_chan_a_sample_1, 
    output [15:0] o_chan_b_sample_0,
    output [15:0] o_chan_b_sample_1
);

    logic [ 7:0] adc_chan_a_data_delg;
    logic [ 7:0] adc_chan_b_data_delg;
    logic        adc_sclk            /* synthesis syn_keep=1 */;
    logic        adc_eclk;

    
    
    
    /* ADC DATA RX LOGIC */
    
    // see Lattice ECP5 High Speed IO Guide (TN1265) GDDRX2_RX.ECLK.Centered 

    ECLKSYNCB adc_eclk_syncb (
        .STOP  (1'b0), 
        .ECLKI (i_adc_data_clk), 
        .ECLKO (adc_eclk)
    );

    CLKDIVF adc_clkdivf (
        .RST     (1'b0),
        .ALIGNWD (1'b0),
        .CLKI    (adc_eclk),
        .CDIVX   (adc_sclk)
    );
    
    assign o_sample_clk = adc_sclk;
    
    genvar i;
    generate
        for (i=0; i < 8; i=i+1) begin
            
            /* Channel A */

            DELAYG #(
                .DEL_MODE ("ECLK_CENTERED")
            ) adc_chan_a_delg (
                .A (i_adc_chan_a_data[i]), 
                .Z (adc_chan_a_data_delg[i])
            );

            IDDRX2F adc_chan_a_iddr (
                .RST     (1'b0), 
                .ALIGNWD (1'b0), 
                .ECLK    (adc_eclk), 
                .SCLK    (adc_sclk), 
                .D       (adc_chan_a_data_delg[i]), 
                .Q0      (o_chan_a_sample_0[2*i]), 
                .Q1      (o_chan_a_sample_0[(2*i)+1]), 
                .Q2      (o_chan_a_sample_1[2*i]), 
                .Q3      (o_chan_a_sample_1[(2*i)+1])
            );
            
            /* Channel B */

            DELAYG #(
                .DEL_MODE ("ECLK_CENTERED")
            ) adc_chan_b_delg (
                .A (i_adc_chan_b_data[i]), 
                .Z (adc_chan_b_data_delg[i])
            );

            IDDRX2F adc_chan_b_iddr (
                .RST     (1'b0), 
                .ALIGNWD (1'b0), 
                .ECLK    (adc_eclk), 
                .SCLK    (adc_sclk), 
                .D       (adc_chan_b_data_delg[i]), 
                .Q0      (o_chan_b_sample_0[2*i]), 
                .Q1      (o_chan_b_sample_0[(2*i)+1]), 
                .Q2      (o_chan_b_sample_1[2*i]), 
                .Q3      (o_chan_b_sample_1[(2*i)+1])
            );

        end
    endgenerate    


endmodule


