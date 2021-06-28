package pkg_adc_regmap;

localparam ADDR_BITS = 26;
localparam CHIP_BITS = 10;
localparam MOD_BITS = 4;

// This is the 8-bit prefix for the ADC chip
localparam CHIP_PREFIX = CHIP_BITS'('h223);

// This is the 4-bit prefix indicating the module on the chip
localparam DSP_PREFIX = MOD_BITS'('h0);

// This is a macro used for the addresses
`define DSP_PREFIXER(x) (ADDR_BITS'(x) | (ADDR_BITS'(CHIP_PREFIX) << 16) | (ADDR_BITS'(DSP_PREFIX)))

// DSP Register Map
localparam bit [15:0] REG_DSP_RESET   = 16'h0000; //`DSP_PREFIXER('h0000);
localparam bit [15:0] REG_DSP_ERROR   = 16'h0004; //`DSP_PREFIXER('h0004);
localparam bit [15:0] REG_DSP_CHANNEL = 16'h0008; //`DSP_PREFIXER('h0008);
//localparam REG_DSP_NBGAIN = GAIN_PREFIXER('h000C);
//localparam REG_DSP_WBGAIN = GAIN_PREFIXER('h0010);

function test_regs();
    $display("REG_DSP_RESET   = %x", REG_DSP_RESET);
    $display("REG_DSP_ERROR   = %x", REG_DSP_ERROR);
    $display("REG_DSP_CHANNEL = %x", REG_DSP_CHANNEL);
    return REG_DSP_RESET;
endfunction: test_regs

endpackage: pkg_adc_regmap