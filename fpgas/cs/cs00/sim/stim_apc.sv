//
// Template Test Bench Module
//

`timescale 10ps / 10ps

`default_nettype none

module stim_apc (
    intf_apc_stim.master         int_apc
);

localparam integer INPUT_WIDTH     = 16;
localparam integer INPUT_FRAC      = 15;
localparam integer LAG_DELAY       = 100;
localparam integer MAVG_DELAY      = 100;
localparam integer OUTPUT_WIDTH    = 39;

logic                           i_clk;
logic                           i_rst_n;
logic signed [OUTPUT_WIDTH-1:0] o_acorr_re;
logic signed [OUTPUT_WIDTH-1:0] o_acorr_im;
logic        [OUTPUT_WIDTH-1:0] o_power;
logic                           o_valid;

assign i_clk = int_apc.clk;
assign i_rst_n = ~int_apc.rst;

// debug variable declarations
logic [31:0] glbl_err_count = 0;
logic [31:0] local_err_count = 0;
logic [31:0] run_count = 0;

task reset_all;
    int_apc.data_re = '0;
    int_apc.data_im = '0;
    int_apc.valid = 1'b0;
endtask: reset_all

initial begin: stimulus
    reset_all();
    @(posedge i_rst_n)

    // Test 1: No data in = no data out.
    $display("Test 1 Started!");
    reset_all();
    repeat(1500) @(posedge i_clk);
    if (run_count > 0) begin
        $display("Error: Test 1 failed! No data input, but data output received.");
        glbl_err_count++;
    end
    repeat(10) @(posedge i_clk);
    $display("Test 1 Done!");

    // Test 2: No data in = no data out.
    $display("Test 2 Started!");
    reset_all();
    repeat(10) @(posedge i_clk);
    for (int idx = 0; idx < 10000; idx++) begin
        #1;
        int_apc.data_re <= $rtoi($itor(1<<14) * $cos(0.1 * 8.0 * $atan(1.0) * $itor(idx)));
        int_apc.data_im <= $rtoi($itor(1<<14) * $sin(0.1 * 8.0 * $atan(1.0) * $itor(idx)));
        int_apc.valid <= 1'b1;
        repeat(1) @(posedge i_clk);
        // int_apc.data_re <= '0;
        // int_apc.data_im <= '0;
        #1 int_apc.valid <= 1'b0;
        repeat(6) @(posedge i_clk);
    end
    repeat(100) @(posedge i_clk);
    if (run_count != 10000) begin
        $display("Error: Test 2 failed! Expected 10000 samples, received %d.", run_count);
        glbl_err_count++;
    end
    repeat(10) @(posedge i_clk);
    $display("Test 2 Done!");

    // Finished
    repeat(100) @(posedge i_clk);
    glbl_err_count = local_err_count + glbl_err_count;
    repeat(1) @(posedge i_clk);
    $display("Simulation done!");
    if (glbl_err_count != 0) begin
        $display("Tests failed %d times.", glbl_err_count);
    end else begin
        $display("<<TB_SUCCESS>>");
    end
    $finish();

end

logic        [OUTPUT_WIDTH-1:0] pow_real_reg;
logic signed [OUTPUT_WIDTH-1:0] ac_real_reg;
logic signed [OUTPUT_WIDTH-1:0] ac_imag_reg;
logic                           o_valid_and_nonzero_acorr = 1'b0;
logic                           o_valid_and_nonzero_power = 1'b0;
logic                           acorr_before_power = 1'b0;
logic                           unequal_ac_to_pow = 1'b0;

always @ (posedge i_clk) begin
    if (i_rst_n == 1'b0) begin
        run_count <= '0;
        o_valid_and_nonzero_acorr <= 1'b0;
        o_valid_and_nonzero_power <= 1'b0;
        acorr_before_power <= 1'b0;
    end else if (o_valid == 1'b1) begin
        run_count <= run_count + 1;
        if (o_power != '0) begin
            o_valid_and_nonzero_power <= 1'b1;
        end
        if ((o_acorr_re != '0) || (o_acorr_im != '0)) begin
            o_valid_and_nonzero_acorr <= 1'b1;
        end
        if (o_valid_and_nonzero_acorr == 1'b1) begin
            if (o_valid_and_nonzero_power == 1'b0) begin
                if (acorr_before_power == 1'b0) begin
                    $display("Error detected... Autocorrelator output occurred before power output.");
                    local_err_count++;
                end
            end
        end
    end

    if (o_valid == 1'b1) begin
        ac_real_reg <= o_acorr_re;
        ac_imag_reg <= o_acorr_im;
        pow_real_reg <= o_power;

        if ((ac_real_reg != '0) && (pow_real_reg != '0)) begin
            if ((ac_real_reg == o_acorr_re) && (ac_imag_reg == o_acorr_im)) begin
                if ((ac_real_reg != pow_real_reg) && (unequal_ac_to_pow == 1'b0)) begin
                    $display("Error detected... Noiseless AC output and Power output are not equal.");
                    $display("       AC: (%d, %d)", ac_real_reg, ac_imag_reg);
                    $display("       AC: (%d, 0.0)", pow_real_reg);
                    local_err_count++;
                end
            end
        end
    end
end

endmodule: stim_apc

`default_nettype wire
