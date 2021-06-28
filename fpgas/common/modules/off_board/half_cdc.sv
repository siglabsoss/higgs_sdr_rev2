module half_cdc #(parameter CAPACITY = 32)
(
    input clk,
    input half_clock,
    input reset,

    input wire [31:0]  t0_data,
    input wire         t0_valid,
    output logic       t0_ready,


    output reg [31:0]  i0_data,
    output logic       i0_valid,
    input wire         i0_ready
);




logic reg_half_clock;       // delay of half_clock, but in clk domain

logic [31:0] latch_data;    // in half_clock domain
logic latch_valid;          // in half_clock domain

logic latch_valid_p; // in clk domain

always @(posedge clk) begin
    reg_half_clock <= half_clock;
    if( !reg_half_clock && half_clock ) begin // unsure about phase here
        // negedge of half_clock
        latch_data <= t0_data;
        latch_valid <= t0_valid;
        t0_ready <= i0_ready;
    end

    if( reg_half_clock && !half_clock ) begin
        // posedge half_clock
        if( latch_valid ) begin
            i0_data <= latch_data;
            i0_valid <= 1'b1;
        end
    end else begin
        if( latch_valid_p ) begin
            i0_valid <= 1'b0;
        end
    end

    latch_valid_p <= latch_valid;
end



endmodule


`default_nettype wire
