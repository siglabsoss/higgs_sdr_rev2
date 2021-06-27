module half_cdc_tb #(parameter bit FOO = 0)(
    input clk,
    input rst
);

logic half_clock;
reg [2:0] half_cnt;
reg half_reset;

logic [31:0] test_counter;

always @(posedge clk) begin
    if (rst) begin
        half_clock <= 0;
    end else begin
        half_clock <= ~half_clock;
    end
end


always @(posedge clk) begin
    if( rst ) begin
        test_counter <= 0;
    end else begin
        test_counter <= test_counter + 1;
    end
end


logic [31:0] t0_data;
logic t0_valid;
logic t0_ready;

logic [31:0] i0_data;
logic i0_valid;
logic i0_ready;


half_cdc dut (
     .clk(clk)
    ,.half_clock(half_clock)
    ,.reset(rst)
    ,.t0_data(t0_data)
    ,.t0_valid(t0_valid)
    ,.t0_ready(t0_ready)
    ,.i0_data(i0_data)
    ,.i0_valid(i0_valid)
    ,.i0_ready(i0_ready)
    );


always @(posedge clk) begin
    case(test_counter)
        32'h10:  begin
            t0_data <= 32'h2;
            t0_valid <= 1;
            i0_ready <= 1;
        end
        32'h12:  begin
            t0_data <= 32'h20;
        end
        32'h14:  begin
            t0_data <= 32'h0;
        end
        32'h16:  begin
            t0_data <= 32'hdeadbeef;
        end
        32'h18:  begin
            t0_data <= 32'h7;
        end
        32'h1a:  begin
            t0_data <= 32'h0;
        end
        32'h1c:  begin
            t0_data <= 32'h0;
        end
        32'h1e:  begin
            t0_data <= 32'h0;
        end
        32'h20:  begin
            t0_data <= 32'h0;
        end
        32'h22:  begin
            t0_data <= 32'h0;
            i0_ready <= 0;
        end
        32'h24:  begin
            t0_data <= 32'h0;
            t0_valid <= 0;
            i0_ready <= 1;
        end
        32'h26:  begin
            t0_data <= 32'h0;
            t0_valid <= 1;
        end
        32'h28:  begin
            t0_data <= 32'h0;
            t0_valid <= 0;
        end

    endcase
end




endmodule