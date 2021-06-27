// adc_dsp.sv
//
// Top level for the DSP operations in the ADC FPGA.
//

`timescale 10ps / 10ps

`default_nettype none

import pkg_adc_regmap::*;

module adc_dsp # (
    parameter integer WIDTH = 16,
    parameter integer ACTIVE_CHANNEL = 16
) (
    input  wire logic [WIDTH-1:0] i_inph_data,
    input  wire logic [WIDTH-1:0] i_inph_delay_data,
    input  wire logic             i_valid,
    output wire logic [WIDTH-1:0] o_inph,
    output wire logic [WIDTH-1:0] o_quad,
    output wire logic             o_valid,
    
    output wire logic [WIDTH-1:0] o_downconv_inph,
    output wire logic [WIDTH-1:0] o_downconv_quad,
    output wire logic             o_downconv_valid,
    input  wire logic             i_clock,
    input  wire logic             i_reset,
    input  wire logic             i_phase_inc_update,
    input  wire logic [11:0]      i_phase_inc_new,
    input  wire logic             i_sw_rst_strb,
    output wire logic             o_local_rst_status,
    output wire logic             o_flow_problem
);

// Register signals
// --> Phase Accumulator Increment
logic [12-1:0] reg_phase_inc = ACTIVE_CHANNEL;
logic          reg_phase_inc_valid;
// --> Local Reset
logic          local_reset;

logic [15:0] dcout_inph;
logic [15:0] dcout_quad;
logic        dcout_valid;

downconverter #(
    .WIDTH(WIDTH))
downconverter_inst (
    .i_inph_data      (i_inph_data      ),
    .i_inph_delay_data(i_inph_delay_data),
    .i_valid          (i_valid          ),
    .o_inph_data      (dcout_inph       ),
    .o_quad_data      (dcout_quad       ),
    .o_valid          (dcout_valid      ),
    .i_clock          (i_clock          ),
    .i_reset          (local_reset      ));
    
assign o_downconv_inph = dcout_inph;
assign o_downconv_quad = dcout_quad;
assign o_downconv_valid = dcout_valid;

logic [15:0] bb_inph;
logic [15:0] bb_quad;
logic        bb_inph_oflow;
logic        bb_quad_oflow;
logic        bb_valid;
logic        bb_ready; // blocks ADC samples until

logic phase_inc_valid;
assign phase_inc_valid = reg_phase_inc_valid | local_reset;

rx_channel_modulator #(
    .WIDTH(WIDTH))
rx_channel_modulator_inst (
    .i_inph           (dcout_inph     ),
    .i_quad           (dcout_quad     ),
    .i_valid          (dcout_valid    ),
    .i_phase_inc      (reg_phase_inc  ),
    .i_phase_inc_valid(phase_inc_valid),
    .o_inph           (bb_inph        ),
    .o_quad           (bb_quad        ),
    .o_inph_oflow     (               ),
    .o_quad_oflow     (               ),
    .o_valid          (bb_valid       ),
    .i_clock          (i_clock        ),
    .i_reset          (local_reset    ));

cic_decimator #(
    .WIDTH(WIDTH))
cic_decimator_inst (
    .i_inph              (bb_inph    ),
    .i_quad              (bb_quad    ),
    .i_valid             (bb_valid   ),
    .o_ready             (bb_ready   ), // For synch only, ignored by upstream (real-time modules)
    .o_inph              (o_inph     ),
    .o_quad              (o_quad     ),
    .o_valid             (o_valid    ),
    .o_inph_pos_oflow    (           ),
    .o_inph_neg_oflow    (           ),
    .o_quad_pos_oflow    (           ),
    .o_quad_neg_oflow    (           ),
    .o_cic_inph_pos_oflow(           ),
    .o_cic_inph_neg_oflow(           ),
    .o_cic_quad_pos_oflow(           ),
    .o_cic_quad_neg_oflow(           ),
    .i_clock             (i_clock    ),
    .i_reset             (local_reset));

assign o_flow_problem = bb_valid & (~bb_ready);

logic [3:0] local_reset_cnt;

// channel change update
always_ff @(posedge i_clock) begin
    if (i_reset == 1'b1) begin
        reg_phase_inc_valid <= 1'b1;
    end else begin
        //Default Values
        reg_phase_inc_valid <= 1'b0;
        if (i_phase_inc_update) begin
            reg_phase_inc_valid <= 1'b1;
            reg_phase_inc       <= i_phase_inc_new;
        end
    end
end

assign o_local_rst_status = local_reset;

// local reset 
always_ff @(posedge i_clock) begin
    if (i_reset == 1'b1) begin
        local_reset <= 1'b1;
        local_reset_cnt <= 4'b1111;
    end else begin
        // Count down local reset (hold for 15 clock cycles)
        if (local_reset_cnt != 0) begin
            local_reset_cnt <= local_reset_cnt - 1;
            local_reset <= 1'b1;
        end else begin
            local_reset <= 1'b0;
        end
        
        if (i_sw_rst_strb) begin
            local_reset_cnt <= 4'b1111;
        end
    end
end

endmodule: adc_dsp

`default_nettype wire
