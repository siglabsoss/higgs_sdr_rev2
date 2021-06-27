/*
 * Module: dac_data_tx
 *
 * TODO: Add module documentation
 */
module dac_data_tx (

    input         i_refclk,      // dac data rate clock
    input         i_refclk90,    // 90 degree phase shifted version of i_refclk

    /* DATA & SIGNALS TO Tx TO DAC */

    output        o_dac_sclk,    // make input data signals synchronous to this clock
    input         i_sync,
    input         i_frame,
    input         i_samples_vld, // when data isn't valid, we'll send zeros to the DAC
    input  [15:0] i_chan_a_sample_0,
    input  [15:0] i_chan_a_sample_1,
    input  [15:0] i_chan_b_sample_0,
    input  [15:0] i_chan_b_sample_1,

    /* FPGA TO DAC DATA INTERFACE */

    output        o_dac_dclk, // data clock to the DAC
    output        o_dac_sync,
    output        o_dac_frame,
    output        o_dac_parity,
    output [15:0] o_dac_data
);

    logic dac_eclk;
    logic dac_sclk;

    logic        sync_reg;
    logic        frame_reg;
    logic [15:0] chan_a_s0_reg;
    logic [15:0] chan_a_s1_reg;
    logic [15:0] chan_b_s0_reg;
    logic [15:0] chan_b_s1_reg;

    /* even parity */
    logic        chan_a_s0_parity;
    logic        chan_a_s1_parity;
    logic        chan_b_s0_parity;
    logic        chan_b_s1_parity;


    assign o_dac_sclk = dac_sclk;

    always_ff @(posedge dac_sclk) begin

        sync_reg  <= i_sync;
        frame_reg <= i_frame;

        if (i_samples_vld) begin

            chan_a_s0_reg    <= i_chan_a_sample_0;
            chan_a_s1_reg    <= i_chan_a_sample_1;
            chan_a_s0_parity <= ^i_chan_a_sample_0; // even parity
            chan_a_s1_parity <= ^i_chan_a_sample_1; // even parity

            chan_b_s0_reg    <= i_chan_b_sample_0;
            chan_b_s1_reg    <= i_chan_b_sample_1;
            chan_b_s0_parity <= ^i_chan_b_sample_0; // even parity
            chan_b_s1_parity <= ^i_chan_b_sample_1; // even parity

        end else begin

            chan_a_s0_reg    <= 16'h0000;
            chan_a_s1_reg    <= 16'h0000;
            chan_a_s0_parity <= 0;
            chan_a_s1_parity <= 0;

            chan_b_s0_reg    <= 16'h0000;
            chan_b_s1_reg    <= 16'h0000;
            chan_b_s0_parity <= 0;
            chan_b_s1_parity <= 0;

        end
    end


    /* DAC DATA TX LOGIC */

    // see Lattice ECP5 High Speed IO Guide (TN1265) GDDRX2_TX.ECLK.Centered


    ECLKSYNCB dac_eclksyncb (
        .STOP  (1'b0),
        .ECLKI (i_refclk),
        .ECLKO (dac_eclk)
    );


    CLKDIVF dac_clkdivf (
        .RST     (1'b0),
        .ALIGNWD (1'b0),
        .CLKI    (dac_eclk),
        .CDIVX   (dac_sclk)
    );


    ODDRX2F dac_clk_oddr (
        .RST  (1'b0),
        .ECLK (i_refclk90),
        .SCLK (dac_sclk),
        .D0   (1'b1),
        .D1   (1'b0),
        .D2   (1'b1),
        .D3   (1'b0),
        .Q    (o_dac_dclk)
    );

    ODDRX2F dac_sync_oddr (
        .RST  (1'b0),
        .ECLK (dac_eclk),
        .SCLK (dac_sclk),
        .D0   (sync_reg),
        .D1   (sync_reg),
        .D2   (sync_reg),
        .D3   (sync_reg),
        .Q    (o_dac_sync)
    );

    ODDRX2F dac_frame_oddr (
        .RST  (1'b0),
        .ECLK (dac_eclk),
        .SCLK (dac_sclk),
        .D0   (frame_reg),
        .D1   (frame_reg),
        .D2   (frame_reg),
        .D3   (frame_reg),
        .Q    (o_dac_frame)
    );

    ODDRX2F dac_parity_oddr (
        .RST  (1'b0),
        .ECLK (dac_eclk),
        .SCLK (dac_sclk),
        .D0   (chan_a_s0_parity),
        .D1   (chan_a_s1_parity),
        .D2   (chan_b_s0_parity),
        .D3   (chan_b_s1_parity),
        .Q    (o_dac_parity)
    );


    generate
        genvar i;

        for (i=0; i < 16; i++) begin
            ODDRX2F dac_data_oddr (
                .RST (1'b0),
                .ECLK(dac_eclk),
                .SCLK(dac_sclk),
                .D0  (chan_a_s0_reg[i]),
                .D1  (chan_a_s1_reg[i]),
                .D2  (chan_b_s0_reg[i]),
                .D3  (chan_b_s1_reg[i]),
                .Q   (o_dac_data[i])
            );
        end
    endgenerate

endmodule


