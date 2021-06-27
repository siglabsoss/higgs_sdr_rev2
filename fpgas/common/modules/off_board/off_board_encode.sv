`default_nettype none

 // capacity units is 32bit words

module off_board_encode #(parameter CAPACITY = 32)
(
    input clk,
    input reset,

    input wire [31:0]  t0_data,
    input wire         t0_valid,
    output logic       t0_ready,


    output reg [31:0]  i0_data,
    output logic        i0_valid,
    input wire         i0_ready

    // output wire [31:0] fillcount
);

logic [31:0]  data_storage;


// // -3 was chosen through experimental verification
// fwft_sc_fifo #(
//     .DEPTH        (CAPACITY),
//     .WIDTH        (32), // address width

//     .ALMOST_FULL  (CAPACITY-3) // number of locations for afull to be active
// ) wrap_buffer (
//     .clk          (clk),
//     .rst          (reset),
//     .wren         (t0_valid && t0_ready),
//     .wdata        (t0_data),
//     .o_afull_n    (t0_ready),
//     .rden         (m_ready),
//     .rdata        (m_data),
//     .rdata_vld    (m_valid),
//     .fillcount    (fillcount)
//     );


reg [2:0]         cnt;



always @(posedge clk) begin
    if(reset) begin
        cnt <= 0;
        t0_ready <= 0;
    end else begin

        if( cnt == 0 & t0_valid)
            cnt <= 1;
        else if (cnt == 1 & t0_valid)
            cnt <= 2;
        else if (cnt == 2)
            cnt <= 3;
        else if( cnt == 3 && i0_ready ) begin
            cnt <= 4;
        end else if( cnt == 4 && i0_ready ) begin
            cnt <= 0;
        end


        if( cnt == 1 ) begin
            t0_ready <= 1;
        end
        if( cnt == 2 ) begin
            data_storage <= t0_data;
            t0_ready <= 0;
            i0_data <= data_storage;
        end
        if( cnt == 3 | cnt == 4 ) begin
            // if( i0_ready ) begin
                i0_valid <= 1;
                i0_data <= data_storage;
            // end
        end

        if( cnt == 0 ) begin
            i0_valid <= 0;
        end

    // if( t0_valid & t0_ready ) begin

    // end

    // if(i0_valid & i0_ready)
    //     cnt <= cnt + 1;
    end
end



endmodule


`default_nettype wire
