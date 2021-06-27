`timescale 10 ps / 10 ps

`default_nettype none

import pkg_adc_regmap::*;

module tb_adc_dsp;

localparam integer WIDTH = 16;
localparam integer ACTIVE_CHANNEL = 1;

logic [WIDTH-1:0] i_inph_data;
logic [WIDTH-1:0] i_inph_delay_data;
logic             i_valid;
logic [WIDTH-1:0] o_inph;
logic [WIDTH-1:0] o_quad;
logic             o_valid;
logic             i_clock;
logic             i_reset;
logic             o_flow_problem;
intf_cmd          cmd();

adc_dsp #(
    .WIDTH(WIDTH),
    .ACTIVE_CHANNEL(ACTIVE_CHANNEL))
uut (.*);

initial begin
    i_clock = 1'b0;
    while (1) begin
        #5 i_clock = 1'b0;
        #5 i_clock = 1'b1;
    end
end


logic [31:0] err_cnt = 0;
initial begin
    i_reset = 1'b1;
    i_inph_data = '0;
    i_inph_delay_data = '0;
    i_valid = 1'b0;
    cmd.sel = 1'b0;
    cmd.rd_wr_n = 1'b0;
    cmd.wdata = 32'h0;
    cmd.byte_addr = 0;
    #100;
    @(negedge i_clock) begin
        i_reset = 1'b0;
        i_inph_data = 16'h4000;
        i_inph_delay_data = 16'h4000;
        i_valid = 1'b1;
    end
    #10000;
    // Attempt to read register
    @(negedge i_clock) begin
        cmd.sel = 1'b1;
        cmd.rd_wr_n = 1'b1;
        cmd.byte_addr = REG_DSP_CHANNEL;
    end
    #10;
    cmd.sel = 1'b0;
    cmd.rd_wr_n = 1'b0;
    cmd.byte_addr = 0;
    for (int lind = 0; lind < 1000; lind++) begin
        if (cmd.ack == 1'b1) begin
            break;
        end
        #10;
    end
    if (cmd.ack != 1'b1) begin
        $display("Error reading from register, no ack!");
        err_cnt++;
    end else if (cmd.rdata != ACTIVE_CHANNEL) begin
        $display("Error reading from register, value expected = %d, value read = %d.", ACTIVE_CHANNEL, cmd.rdata);
        err_cnt++;
    end
    #20;
    // Attempt to write register
    @(negedge i_clock) begin
        cmd.sel = 1'b1;
        cmd.rd_wr_n = 1'b0;
        cmd.wdata = 32'h100;
        cmd.byte_addr = REG_DSP_CHANNEL;
    end
    #10;
    cmd.sel = 1'b0;
    cmd.rd_wr_n = 1'b0;
    cmd.wdata = 32'h0;
    cmd.byte_addr = 0;
    for (int lind = 0; lind < 1000; lind++) begin
        if (cmd.ack == 1'b1) begin
            break;
        end
        #10;
    end
    if (cmd.ack != 1'b1) begin
        $display("Error writing to register, no ack!");
        err_cnt++;
    end
    #20;
    // Attempt to read register
    @(negedge i_clock) begin
        cmd.sel = 1'b1;
        cmd.rd_wr_n = 1'b1;
        cmd.byte_addr = REG_DSP_CHANNEL;
    end
    #10;
    cmd.sel = 1'b0;
    cmd.rd_wr_n = 1'b0;
    cmd.byte_addr = 0;
    for (int lind = 0; lind < 1000; lind++) begin
        if (cmd.ack == 1'b1) begin
            break;
        end
        #10;
    end
    if (cmd.ack != 1'b1) begin
        $display("Error reading from register, no ack!");
        err_cnt++;
    end else if (cmd.rdata != 32'h100) begin
        $display("Error reading from register, value expected = %d, value read = %d.", ACTIVE_CHANNEL, cmd.rdata);
        err_cnt++;
    end
    #1000;
    $finish;
end

endmodule: tb_adc_dsp

`default_nettype wire
