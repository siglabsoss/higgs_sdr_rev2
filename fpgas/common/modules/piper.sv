//---------------------------------------------------------
// Design Name : piper
// File Name   : piper.sv
// Authors     : Daniel Lomeli
// Modified    : 
// Function    : 
//-----------------------------------------------------

`default_nettype none

module piper #(
    parameter int   WIDTH       =  16,
    parameter int   DELAYS      =   0,
    parameter logic RST_ENA     = 1'b1
) (
    input wire                            i_clk,
    input wire                            i_rst,
    input wire   [WIDTH-1:0]              i_input,
    output logic [WIDTH-1:0]              o_output
);

    generate 
        if (DELAYS == 0) begin
        
            assign o_output = i_input;
        
        end 
    endgenerate
    
    generate
        if (DELAYS > 0 && RST_ENA == 1) begin
            logic [DELAYS-1:0][WIDTH-1:0] inline;
                    
            always_ff @(posedge i_clk) begin
                if (i_rst) begin
                    inline    <=  1'b0;
                end else begin
                    inline    <= { inline[$left(inline)-1:0], i_input};
                end
            end
    
            assign o_output = inline[$left(inline)];
        end
    endgenerate
        
    generate
        if (DELAYS > 0 && RST_ENA == 0) begin
            logic [DELAYS-1:0][WIDTH-1:0] inline;
            
            always_ff @(posedge i_clk) begin
                inline    <= { inline[$left(inline)-1:0], i_input}; 
            end 
            
            assign o_output = inline[$left(inline)];
        end
    endgenerate
    
    
endmodule