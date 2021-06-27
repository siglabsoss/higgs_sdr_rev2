`timescale 10 ps / 10 ps

interface intf_afb #(
    parameter WIDTH     = 32,
    parameter LOG2LEN   = 10
) (input logic clock, reset);

    logic [WIDTH-1:0]     in_inph;          // Input in-phase data
    logic [WIDTH-1:0]     in_quad;          // Input quadrature data
    logic                 enable;           // Controls if this module does anything at all
    logic                 in_valid;         // Indicates valid data on i_inph and i_quad (ASSERT NO MORE OFTEN THAN EVERY OTHER CLOCK CYCLE)
    logic [3:0]           ppf_gain_pow2;    // Controls gain of the polyphase filter output 
    logic [LOG2LEN*2-1:0] fft_rounding;     // Controls rounding at various stages of the FFT block
    logic                 ready;            // Asserted by downstream module to indicate it's ready to receive the next sample on o_inph and o_quad
    logic                 out_valid;        // Indicates valid data on o_inph and o_quad 
    logic [WIDTH-1:0]     out_inph;         // Output in-phase data
    logic [WIDTH-1:0]     out_quad;         // Output quadrature data
    logic                 ppf_saturate;     // Output of polyphase filter saturated
    logic                 reorder_overflow; // Indicates that the FFT output reorder block was full when new FFT outputs were written into it
    logic                 fft_underflow;    // Indicates that FFT asserted o_ready and polyphase filter was not asserting o_valid
    logic                 fft_overflow;     // Indicates that polyphase filter asserted o_valid when FFT was not asserting o_ready
    logic [LOG2LEN-1:0]   fft_saturate;     // Output of FFT saturated

    modport master (
        input   clock,            // Clock
        input   reset,            // Reset
        output  in_inph,          // Input in-phase data
        output  in_quad,          // Input quadrature data
        output  enable,           // Controls if this module does anything at all
        output  in_valid,         // Indicates valid data on i_inph and i_quad (ASSERT NO MORE OFTEN THAN EVERY OTHER CLOCK CYCLE)
        output  ppf_gain_pow2,    // Controls gain of the polyphase filter output 
        output  fft_rounding,     // Controls rounding at various stages of the FFT block
        input   ready,            // Asserted by downstream module to indicate it's ready to receive the next sample on o_inph and o_quad
        input   out_valid,        // Indicates valid data on o_inph and o_quad 
        input   out_inph,         // Output in-phase data
        input   out_quad,         // Output quadrature data
        input   ppf_saturate,     // Output of polyphase filter saturated
        input   reorder_overflow, // Indicates that the FFT output reorder block was full when new FFT outputs were written into it
        input   fft_underflow,    // Indicates that FFT asserted o_ready and polyphase filter was not asserting o_valid
        input   fft_overflow,     // Indicates that polyphase filter asserted o_valid when FFT was not asserting o_ready
        input   fft_saturate      // Output of FFT saturated  
    );

    modport slave (
        input   clock,            // Clock
        input   reset,            // Reset
        input   in_inph,          // Input in-phase data
        input   in_quad,          // Input quadrature data
        input   enable,           // Controls if this module does anything at all
        input   in_valid,         // Indicates valid data on i_inph and i_quad (ASSERT NO MORE OFTEN THAN EVERY OTHER CLOCK CYCLE)
        input   ppf_gain_pow2,    // Controls gain of the polyphase filter output 
        input   fft_rounding,     // Controls rounding at various stages of the FFT block
        output  ready,            // Asserted by downstream module to indicate it's ready to receive the next sample on o_inph and o_quad
        output  out_valid,        // Indicates valid data on o_inph and o_quad 
        output  out_inph,         // Output in-phase data
        output  out_quad,         // Output quadrature data
        output  ppf_saturate,     // Output of polyphase filter saturated
        output  reorder_overflow, // Indicates that the FFT output reorder block was full when new FFT outputs were written into it
        output  fft_underflow,    // Indicates that FFT asserted o_ready and polyphase filter was not asserting o_valid
        output  fft_overflow,     // Indicates that polyphase filter asserted o_valid when FFT was not asserting o_ready
        output  fft_saturate      // Output of FFT saturated  
    
    );

endinterface: intf_afb