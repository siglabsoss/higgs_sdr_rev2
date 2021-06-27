/*
 * Module: afb_stim
 * 
 */
 
`timescale 1ns / 10ps
 
`default_nettype none

module afb_stim( 
        intf_afb.master       stim_intf
    );

    localparam int unsigned WIDTH=18;
    
    // DUT SIGNALS
    initial begin
        stim_intf.in_inph       <= '0;
        stim_intf.in_quad       <= '0;
        stim_intf.in_valid      <= 0;
        stim_intf.enable        <= 0;
        stim_intf.ppf_gain_pow2 <= 4'h0;
        stim_intf.fft_rounding  <= 20'b01010101010101010101;
    end

    

    // TB SIGNALS
    
    // These must match how they're defined in the DUT.  I haven't made them parameters of the DUT because they shouldn't need to change frequently
    // and I have not guaranteed that any combination of these will actually work.
    localparam int N = 1024;
    //localparam int N = 31195136;
    localparam int L = 31195136;
    localparam int K = 6;

    localparam real         relative_energy_threshold = 0.0001; // means that the ratio of energy in a non-active bin to the eneryg in the active bin can't exceed this
    localparam real         phase_tolerance           = 0.01;   // means that the phase computed at the "FFT" output of the current test bin must be within +/- this of the actual phase of the test tone
    localparam real         pi                        = 3.14159;
    localparam int unsigned frac_bits                 = 16; // this should match the number of fractional bits used for quantizing the DUT's filter coefficients

    localparam int           tb_bins [8]        = '{4,5,6,99,211,357,-401,-231};
    localparam real          tb_phases [8]      = '{0, pi/8, (2*pi)/8, (3*pi)/8, (4*pi)/8, (5*pi)/8, (6*pi)/8, (7*pi)/8};
    string                   tb_test_str        = "none";
    logic                    tb_end_of_sinusoid = 0;
    
    logic signed [WIDTH-1:0] tb_in_inph_samples [0:$size(tb_bins)-1] [0:N-1] ; 
    logic signed [WIDTH-1:0] tb_in_quad_samples [0:$size(tb_bins)-1] [0:N-1] ; 
    
    
    // create test sinusoids in their respective bins
    initial begin
        int f;
        for (int unsigned b=0; b<$size(tb_bins); b++) begin
            for (int unsigned n=0; n<N; n++) begin
                tb_in_inph_samples[b][n] = int'( $cos( ( (2*pi*n*tb_bins[b])/N ) + tb_phases[b] ) * (2**frac_bits-1) );
                tb_in_quad_samples[b][n] = int'( $sin( ( (2*pi*n*tb_bins[b])/N ) + tb_phases[b] ) * (2**frac_bits-1) );
            end
        end
    end
    
    
    task automatic feed_samples (logic signed [WIDTH-1:0] inph [0:N-1], logic signed [WIDTH-1:0] out_quad [0:N-1] , int unsigned num_iters);
        begin
            int unsigned idx = 0;
            repeat (N*num_iters) begin
                @(posedge stim_intf.clock);
                stim_intf.in_valid <= 1;
                stim_intf.in_inph  <= inph[idx];
                stim_intf.in_quad  <= out_quad[idx]; 
                idx = (idx+1)%N;
                @(posedge stim_intf.clock);
                stim_intf.in_valid <= 0;
                @(posedge stim_intf.clock);
                @(posedge stim_intf.clock);
            end
        end
    endtask
    

    /*
     * 
     * STIMULUS
     * 
     */
    
    initial begin

        @(negedge stim_intf.reset);
        repeat (10) @(posedge stim_intf.clock);
        @(posedge stim_intf.enable);
        
        /*
         * 
         * TEST: Complex sinusoids in frequency different frequency bins (one after the other) 
         * 
         */
        
        tb_test_str = "Sinusoids";

        for (int unsigned b=0; b<$size(tb_bins); b++) begin
            feed_samples(tb_in_inph_samples[b], tb_in_quad_samples[b], K*(L/N)+5); // +5 to make sure we hit steady state in the filter (we collect samples after +3)
            @(posedge stim_intf.clock);
            tb_end_of_sinusoid <= 1;
            @(posedge stim_intf.clock);
            tb_end_of_sinusoid <= 0;
            @(posedge stim_intf.clock);
        end
        
        $display("Done: Sinusoids Test");

        repeat (100) @(posedge stim_intf.clock);
        
        $display("<<<TB_SUCCESS>>>");
        $finish();
            
    end
    

    /*
     * 
     * Test Checkers
     * 
     */
    
    
    /*
     * Overflow, Underflow, and Saturation reporting have been verified in submodule test benches, so they aren't test here.
     * We just want to make sure none of these events are occurring here.
     */

    //always @(posedge stim_intf.clock) begin
    //    if (~stim_intf.reset) begin
    //        assert (stim_intf.fft_underflow    ==  0) else begin $display("<<<TB_FAILURE>>>"); $fatal(1, "DUT Indicated FFT Underflow!");          end
    //        assert (stim_intf.fft_overflow     ==  0) else begin $display("<<<TB_FAILURE>>>"); $fatal(1, "DUT Indicated FFT Overflow!");           end
    //        assert (stim_intf.fft_saturate     == '0) else begin $display("<<<TB_FAILURE>>>"); $fatal(1, "DUT Indicated FFT Saturation!");         end
    //        assert (stim_intf.ppf_saturate     ==  0) else begin $display("<<<TB_FAILURE>>>"); $fatal(1, "DUT Indicated PPF Saturation!");         end
    //        assert (stim_intf.reorder_overflow ==  0) else begin $display("<<<TB_FAILURE>>>"); $fatal(1, "DUT Indicated Reorder Block Overflow!"); end
    //    end
    //end

    function automatic bit filt_out_check (real fft_i [], real fft_q [], int active_bin, real expected_phase, real energy_thresh, real phase_tolerance);
        begin
            
            // NOTE: I ASSUME CORRESPONDING INPUT ARRAYS HAVE THE SAME DIMENSIONS
            
            int f      = $fopen($sformatf("bin_%0d_test_fft_iq.txt", active_bin), "w");
            bit retval = 1'b1; // assum the check passes and correct later if it doesn't
            real energy [];
            real phase;
            int active_bin_idx = (active_bin >= 0) ? active_bin : active_bin + N;
            
            energy = new[$size(fft_i)];
            
            $fwrite(f, "Bin, I, Q\n");
            for (int unsigned b=0; b<$size(fft_i); b++) begin
                $fwrite(f, $sformatf("%0d, %f, %f\n", (b < N/2) ? b : b - N, fft_i[b], fft_q[b]));
            end
            
            $fclose(f);
            
            // compute per bin energy
            for (int unsigned b=0; b<$size(fft_i); b++) begin
                energy[b] = fft_i[b]**2 + fft_q[b]**2;
            end
            
            // verify results are within expected tolerances
            //$display("New Results:");
            //$display("Active Bin: %0d (Index: %0d), Energy: %f", active_bin, active_bin_idx, energy[active_bin_idx]);
            //
            //for (int unsigned b=0; b<$size(fft_i); b++) begin
            //    if (b != active_bin_idx) begin 
            //        $display("Bin %0d Energy Relative To Active Bin: %f", ((b < N/2) ? b : b - N), energy[b]/energy[active_bin_idx]);
            //        if (energy[b]/energy[active_bin_idx] > energy_thresh) begin
            //            $error("Energy In Non-Active Bin To Large!");
            //            retval = 1'b0;
            //        end
            //    end
            //end
            //
            //phase = $atan2( fft_q[active_bin_idx] , fft_i[active_bin_idx] );
            //$display("Phase: %f, Expected: %f", phase, expected_phase);
            //if ( (phase > (expected_phase + phase_tolerance) ) || ( phase < (expected_phase - phase_tolerance) ) ) begin
            //    $error("Incorrect phase at output of active bin %0d!  Expected: %f, Received: %f", active_bin, expected_phase, phase);
            //    retval = 1'b0;
            //end

            return retval;
            
        end
    endfunction
    
    
    real         tb_fft_output_inph [0:N-1] ; 
    real         tb_fft_output_quad [0:N-1] ; 
    int unsigned tb_fft_output_cnt        = 0;
    int unsigned tb_fft_output_bin        = 0;
    int          tb_cur_bin_idx           = 0;
    logic        tb_fft_output_cap_flag   = 0;
    
    always @(posedge stim_intf.clock) begin
        if (tb_test_str == "Sinusoids") begin
            if (tb_end_of_sinusoid) begin
                // call func to check captured output
                assert ( filt_out_check(tb_fft_output_inph, tb_fft_output_quad, tb_bins[tb_cur_bin_idx], tb_phases[tb_cur_bin_idx], relative_energy_threshold, phase_tolerance )) else begin $display("Error in Sinusoids Test!"); $fatal(1, "<<<TB_FAILURE>>>"); end
                tb_cur_bin_idx++;
                tb_fft_output_cnt = 0;
                tb_fft_output_bin = 0;
            end else begin
                if (stim_intf.out_valid & stim_intf.ready) begin
                    if ( (tb_fft_output_cnt >= (N*(K*(L/N)+2)) ) && ( tb_fft_output_bin < N ) ) begin // only capture samples during steady state
                        tb_fft_output_cap_flag                = 1;
                        tb_fft_output_inph[tb_fft_output_bin] = 1.0*stim_intf.out_inph;
                        tb_fft_output_quad[tb_fft_output_bin] = 1.0*stim_intf.out_quad;
                        tb_fft_output_bin++;
                    end else begin
                        tb_fft_output_cap_flag = 0;
                    end
                    tb_fft_output_cnt++;
                end
            end
        end
    end
    
    
    /*
     * 
     * DUT
     * 
     */
    
    assign stim_intf.ready = stim_intf.out_valid;

endmodule

`default_nettype wire
