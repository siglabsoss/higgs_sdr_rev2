/****************************************************************************
 * core_reset.sv
 ****************************************************************************/

/*
 * Module: core_reset
 * 
 * Synchronizes an asynchronous external reset to a given clock domain and provides a user configurable number of output sync_resets each with its own release delay.
 * 
 */
 
`default_nettype none

module core_reset #(
    parameter int NUM_OUTPUTS                          = 1,
    parameter VERILATE                                 = 1'b0,
    parameter int EXTRA_RESET_CLOCKS [0:NUM_OUTPUTS-1] = 0 // allows you to specify how many additional clocks to hold a particular output in reset after the external asynchronous reset has been released
)(
    input  wire logic i_ext_arst,                      // external asynchronous reset 
    input  wire logic i_clk,                           // clock to synchronize the external reset to
    input  wire logic i_clk_pll_unlocked,              // enables this module, tie this to the inverted version of the lock signal of the PLL providing i_clk
    `ifdef VERILATE_DEF
        output wire logic o_sync_resets /*[0:NUM_OUTPUTS-1]*/
    `else
        output wire logic o_sync_resets [0:NUM_OUTPUTS-1]
    `endif
);

    
    logic [1:0] ext_arst_sync_flops;

    always_ff @(posedge i_clk or posedge i_ext_arst) begin
        if (i_ext_arst) begin
            ext_arst_sync_flops <= '1; 
        end else begin
            ext_arst_sync_flops <= {ext_arst_sync_flops[0], 1'b0};    
        end
    end
   
    `ifdef VERILATE_DEF


            logic       sync_resets;
            logic       reset_holds;
            int         delay_cnts; 

                always_ff @(posedge i_clk or posedge i_ext_arst) begin
                    if (i_ext_arst) begin
                        sync_resets <= 1'b1;
                    end else begin
                        sync_resets <= ext_arst_sync_flops[1] | reset_holds;
                    end
                end

                // note: delay counter is synchronously reset
                always_ff @(posedge i_clk) begin
                    if (ext_arst_sync_flops[1]) begin
                        delay_cnts  <= 0;
                        reset_holds <= 1'b0;
                    end else begin
                        if (delay_cnts == 1) begin
                            delay_cnts  <= delay_cnts;
                            reset_holds <= 0;
                        end else begin
                            delay_cnts  <= delay_cnts + 1;
                            reset_holds <= 1;
                        end
                    end
                end
                
                assign o_sync_resets = sync_resets;
    `else 
        generate
            genvar idx;


                
            logic       sync_resets [0:NUM_OUTPUTS-1];
            logic       reset_holds [0:NUM_OUTPUTS-1];
            int         delay_cnts  [0:NUM_OUTPUTS-1]; 

            for (idx=0; idx < NUM_OUTPUTS; idx++) begin

                always_ff @(posedge i_clk or posedge i_ext_arst) begin
                    if (i_ext_arst) begin
                        sync_resets[idx] <= 1'b1;
                    end else begin
                        sync_resets[idx] <= ext_arst_sync_flops[1] | reset_holds[idx];
                    end
                end
                
                // note: delay counter is synchronously reset
                always_ff @(posedge i_clk) begin
                    if (ext_arst_sync_flops[1]) begin
                        delay_cnts[idx]  <= 0;
                        reset_holds[idx] <= 1'b0;
                    end else begin
                        if (delay_cnts[idx] == EXTRA_RESET_CLOCKS[idx]) begin
                            delay_cnts[idx]  <= delay_cnts[idx];
                            reset_holds[idx] <= 0;
                        end else begin
                            delay_cnts[idx]  <= delay_cnts[idx] + 1;
                            reset_holds[idx] <= 1;
                        end
                    end
                end
                
                assign o_sync_resets[idx] = sync_resets[idx];
                
            end
           
        endgenerate
    `endif

endmodule

`default_nettype wire
