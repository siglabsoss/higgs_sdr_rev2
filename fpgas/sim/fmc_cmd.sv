`timescale 10 ps / 10 ps

interface fmc_cmd #(
    parameter ADDR_BITS = 26,
    parameter DATA_BITS = 32
) (input reset);

    logic                 clk;
    logic                 fmc_ne1;
    logic                 fmc_noe;
    logic                 fmc_nwe;
    logic                 fmc_nwait;
    logic [ADDR_BITS-1:0] fmc_a;
    wire  [DATA_BITS-1:0] fmc_d;


    modport master (
        output clk,
        input  reset,
        output fmc_a,
        inout  fmc_d,
        output fmc_ne1,
        output fmc_noe,
        output fmc_nwe,
        input  fmc_nwait
    );

    modport slave (
        input  clk,
        input  reset,
        input  fmc_a,
        inout  fmc_d,
        input  fmc_ne1,
        input  fmc_noe,
        input  fmc_nwe,
        output fmc_nwait
    );

endinterface: fmc_cmd