`timescale 10 ns / 10 ps

`default_nettype none

module tb_eth_dsp_counters;

logic dac_data_valid_reg = 1'b0;
logic fifo_dc_wren = 1'b0;
logic cmd_clock;
logic cmd_sreset;
logic dsp_clock;
logic dsp_sreset;
intf_cmd cmd();

eth_dsp_counters uut (.*);

localparam DSP_PERIOD = 10;
localparam CMD_PERIOD = 50;

always begin
    #(DSP_PERIOD/2) dsp_clock <= 1'b1;
    #(DSP_PERIOD - DSP_PERIOD/2) dsp_clock <= 1'b0;
end

always begin
    #(CMD_PERIOD/2) cmd_clock <= 1'b1;
    #(CMD_PERIOD - CMD_PERIOD/2) cmd_clock <= 1'b0;
end

logic dac_enable;
logic adc_enable;

integer dac_period_cnt = 0;
integer adc_period_cnt = 3;
always_ff @(posedge dsp_clock) begin
    if (dac_enable == 1'b1) begin
        if (dac_period_cnt == 10) begin
            dac_period_cnt <= 0;
            dac_data_valid_reg <= 1'b1;
        end else begin
            dac_period_cnt <= dac_period_cnt + 1;
            dac_data_valid_reg <= 1'b0;
        end
    end

    if (adc_enable == 1'b1) begin
        if (adc_period_cnt == 10) begin
            adc_period_cnt <= 0;
            fifo_dc_wren <= 1'b1;
        end else begin
            adc_period_cnt <= adc_period_cnt + 1;
            fifo_dc_wren <= 1'b0;
        end
    end
end

integer local_errors = 0;

initial begin
    // Perform reset
    cmd_sreset = 1'b1;
    dsp_sreset = 1'b1;
    dac_enable = 1'b0;
    adc_enable = 1'b0;

    #10000;

    @(negedge dsp_clock) begin
        cmd_sreset = 1'b0;
        dsp_sreset = 1'b0;
    end

    #10000;

    // Enable adc count
    @(negedge dsp_clock) begin
        adc_enable = 1'b1;
    end

    #10000;

    // Enable dac count
    @(negedge dsp_clock) begin
        dac_enable = 1'b1;
    end

    #100000;

    if ((local_errors > 0) || (write_acked == 1'b0) || (read_acked == 1'b0)) begin
        $display("Test failed!");
        $display("There were %d errors...", local_errors + 1'(~write_acked) + 1'(~read_acked));
    end else begin
        $display("<<TB_SUCCESS>>");
    end
    $finish;

end

logic write_requested = 1'b1;
logic write_acked = 1'b1;
logic read_requested = 1'b0;
logic read_acked = 1'b0;

always_ff @ (posedge cmd_clock) begin
    if (cmd_sreset == 1'b1) begin
        cmd.sel <= '0;
        cmd.rd_wr_n <= '0;
        cmd.byte_addr <= '0;
        cmd.wdata <= '0;
    end else if ((cmd_sreset == 1'b0) && (dsp_sreset == 1'b0) && (dac_enable == 1'b1) && (adc_enable == 1'b1)) begin
        if (write_requested == 1'b0) begin
            cmd.sel <= 1'b1;
            cmd.rd_wr_n <= 1'b0;
            cmd.wdata <= 32'h1234;
            cmd.byte_addr <= 0;
            read_requested <= 1'b1;
        end else if (write_acked == 1'b0) begin
            if (cmd.ack == 1'b1) begin
                write_acked <= 1'b1;
            end
        end else if (read_requested == 1'b0) begin
            cmd.sel <= 1'b1;
            cmd.rd_wr_n <= 1'b1;
            cmd.byte_addr <= 0;
            read_requested <= 1'b1;
        end else if (read_acked == 1'b0) begin
            if (cmd.ack == 1'b1) begin
                read_acked <= 1'b1;
            end
        end else begin
            cmd.sel <= 1'b0;
            cmd.rd_wr_n <= 1'b0;
            cmd.byte_addr <= 0;
            cmd.wdata <= '0;
        end
    end
end

endmodule: tb_eth_dsp_counters

`default_nettype wire
