`timescale 10 ps / 10 ps

interface intf_apc_stim #(
    parameter DATA_BITS = 32
) (input logic clk, rst);

    logic [DATA_BITS-1:0] data_re;
    logic [DATA_BITS-1:0] data_im; 
    logic                 valid; 

    modport master (
        input  clk,
        input  rst,
        output data_re,
        output data_im,    
        output valid       
    );

    modport slave (
        input  clk,
        input  rst,
        input  data_re,
        input  data_im,    
        input  valid    
    
    );

endinterface: intf_apc_stim