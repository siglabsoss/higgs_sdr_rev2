module eb2a #(
    parameter T_0_WIDTH = 8,
    parameter I_0_WIDTH = 8
) (
    input        [T_0_WIDTH-1:0] t0_data,
    input                        t0_valid,
    output wire                 t0_ready,

    output wire [I_0_WIDTH-1:0] i0_data,
    output wire                 i0_valid,
    input                        i0_ready,

    input clk, reset_n
);

wire en0, en1, sel;

eb2a_data #(
    .T_0_WIDTH(T_0_WIDTH),
    .I_0_WIDTH(I_0_WIDTH)
) udata (
    .t_0_data(t0_data),
    .i_0_data(i0_data),
    .en0(en0), .en1(en1), .sel(sel),
    .clk(clk), .reset_n(reset_n)
);

eb2a_ctrl uctrl (
    .t_0_valid(t0_valid), .t_0_ready(t0_ready),
    .i_0_valid(i0_valid), .i_0_ready(i0_ready),
    .en0(en0), .en1(en1), .sel(sel),
    .clk(clk), .reset_n(reset_n)
);

endmodule
