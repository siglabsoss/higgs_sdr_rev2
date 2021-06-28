`default_nettype none

 // capacity units is 32bit words

module width_8_32 #(parameter CAPACITY = 32)
(
    input clk,
    input reset,

    input wire [7:0]  t0_data,
    input wire         t0_valid,
    output wire        t0_ready,


    output reg [31:0]   i0_data,
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
    .WIDTH        (8), // address width

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

reg [31:0]         data;
reg [1:0]          cnt;

assign i0_data = {m_data, data[23:0]};
assign i0_valid = (cnt == 3) ? m_valid:0;
assign m_ready = (cnt == 3) ? i0_ready:1;
always @(posedge clk) begin
    if(reset) begin
        cnt <= 0;
        data <= 0;
    end
    else begin
        if(m_valid & m_ready) begin
            cnt <= cnt + 1;
            case(cnt)
                0: data[7:0] <= m_data;
                1: data[15:8] <= m_data;
                2: data[23:16] <= m_data;
                3: data[31:23] <= m_data;
            endcase
        end
    end
end



endmodule


`default_nettype wire
