`timescale 10 ns / 10 ps

`default_nettype none

module eth_dsp_counters #(
    parameter logic [15:0] TX_MAGIC_INPH = 16'h8001,
    parameter logic [15:0] TX_MAGIC_QUAD = 16'h8001,
    parameter logic [15:0] RX_MAGIC_INPH = 16'h7FFF,
    parameter logic [15:0] RX_MAGIC_QUAD = 16'h7FFF
)(
    input wire logic        dac_data_valid_reg,
    input wire logic [15:0] dac_data_inph_reg,
    input wire logic [15:0] dac_data_quad_reg,
    input wire logic        fifo_dc_wren,
    input wire logic        cmd_clock,
    input wire logic        cmd_sreset,
    input wire logic        dsp_clock,
    input wire logic        dsp_sreset,
    intf_cmd.slave          cmd
);

logic [3:0]  update_counter;
logic [31:0] dac_counter;
logic [31:0] adc_counter;
logic        counts_valid;

always_ff @(posedge dsp_clock) begin
    if (dsp_sreset == 1'b1) begin
        update_counter <= 0;
        dac_counter <= 0;
        adc_counter <= 0;
    end else begin
        // Signal FIFO to grab counts every once in a while.
        if (update_counter == 0) begin
            counts_valid = 1'b1;
        end else begin
            counts_valid = 1'b0;
        end
        // Increment update counter
        update_counter <= update_counter + 1;
        // Increment dac counter when requested
        if (dac_data_valid_reg == 1'b1) begin
            case ({dac_data_inph_reg, dac_data_quad_reg})
                {TX_MAGIC_INPH, TX_MAGIC_QUAD}: dac_counter <= dac_counter;
                {RX_MAGIC_INPH, RX_MAGIC_QUAD}: dac_counter <= dac_counter;
                default: dac_counter <= dac_counter + 1;
            endcase
        end
        // Increment adc counter when requested
        if (fifo_dc_wren == 1'b1) begin
            adc_counter <= adc_counter + 1;
        end
    end
end

logic [63:0] cmd_counts;
logic        cmd_counts_valid;

pmi_fifo_dc_fwft_v1_0 #(
    .WR_DEPTH        (64     ),
    .WR_DEPTH_AFULL  (32     ),
    .WR_WIDTH        (64     ),
    .RD_WIDTH        (64     ),
    .FAMILY          ("ECP5U"),
    .IMPLEMENTATION  ("EBR"  ),
    .RESET_MODE      ("sync" ),
    .WORD_SWAP       (0      ),
`ifndef SIM_MODE
    .SIM_MODE        (0      )
`else
    .SIM_MODE        (1      )
`endif
) fifo_counts (
    .wrclk           (dsp_clock                 ),
    .wrclk_rst       (dsp_sreset                ),
    .rdclk           (cmd_clock                 ),
    .rdclk_rst       (cmd_sreset                ),
    .wren            (counts_valid              ),
    .wdata           ({adc_counter, dac_counter}),
    .full            (                          ),
    .afull           (                          ),
    .rden            (1'b1                      ),
    .rdata           (cmd_counts                ),
    .rdata_vld       (cmd_counts_valid          ));

logic [31:0] cmd_dac_count;
//logic [31:0] cmd_adc_count;
logic [31:0] cmd_dac_count_reg;
logic [31:0] cmd_adc_count_reg;
logic        cmd_counts_frozen;

always_ff @(posedge cmd_clock) begin
    if (cmd_sreset) begin
        cmd_dac_count     <= '0;
        cmd_dac_count_reg <= '0;
        cmd_adc_count_reg <= '0;
        cmd_counts_frozen <= 1'b0;
        cmd.ack           <= 1'b0;
    end else begin
        // Update count shadow regs
        if (cmd_counts_valid == 1'b1) begin
            cmd_dac_count_reg <= cmd_counts[31:0];
            cmd_adc_count_reg <= cmd_counts[63:32];
        end
        // Only update if we are not in the middle of a read
        if (cmd_counts_frozen == 1'b0) begin
            cmd_dac_count <= cmd_dac_count_reg;
        end
        // If the bus is selected for reading, then we do something
        if ((cmd.sel == 1'b1) && (cmd.rd_wr_n == 1'b1)) begin
            case (cmd.byte_addr)
            0: begin
                cmd.rdata <= cmd_adc_count_reg;
                cmd_counts_frozen <= 1'b1;
                cmd.ack <= 1'b1;
            end
            4: begin
                cmd.rdata <= cmd_dac_count;
                cmd_counts_frozen <= 1'b0;
                cmd.ack <= 1'b1;
            end
            default: begin
                cmd_counts_frozen <= 1'b0;
                cmd.ack <= 1'b1; // Acknowledge to not freeze caller.
            end
            endcase
        end else begin
            cmd.ack <= 1'b0;
        end
    end
end

endmodule: eth_dsp_counters

`default_nettype wire
