`default_nettype none

 // capacity units is 32bit words

module width_32_8 #(parameter CAPACITY = 32)
(
    input clk,
    input reset,

    input wire [31:0]  t0_data,
    input wire         t0_valid,
    output wire        t0_ready,


    output reg [7:0]   i0_data,
    output wire        i0_valid,
    input wire         i0_ready,

    output wire [31:0] fillcount
);


wire [31:0] m_data;
wire        m_valid;
wire        m_ready;


// -3 was chosen through experimental verification
fwft_sc_fifo #(
    .DEPTH        (CAPACITY),
    .WIDTH        (32), // address width

    .ALMOST_FULL  (CAPACITY-3) // number of locations for afull to be active
) wrap_buffer (
    .clk          (clk),
    .rst          (reset),
    .wren         (t0_valid && t0_ready),
    .wdata        (t0_data),
    .o_afull_n    (t0_ready),
    .rden         (m_ready),
    .rdata        (m_data),
    .rdata_vld    (m_valid),
    .fillcount    (fillcount)
    );




reg [1:0]         cnt;
assign i0_data = (cnt == 0) ? m_data[7:0]:
                 (cnt == 1) ? m_data[15:8]:
                 (cnt == 2) ? m_data[23:16]:
m_data[31:24];
assign i0_valid = m_valid;
assign m_ready = (cnt == 3) && i0_ready? 1:0;
always @(posedge clk) begin
    if(reset) begin
        cnt <= 0;
    end
    else begin
    if(i0_valid & i0_ready)
        cnt <= cnt + 1;
    end
end



endmodule


`default_nettype wire
