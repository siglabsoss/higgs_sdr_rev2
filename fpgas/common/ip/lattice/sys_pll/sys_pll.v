/* Verilog netlist generated by SCUBA Diamond (64-bit) 3.10.1.112 */
/* Module Version: 5.7 */
/* C:\lscc\diamond\3.10_x64\ispfpga\bin\nt64\scuba.exe -w -n sys_pll -lang verilog -synth synplify -bus_exp 7 -bb -arch sa5p00 -type pll -fin 125 -fclkop 125 -fclkop_tol 0.0 -fclkos 6 -fclkos_tol 0.0 -phases 0 -phase_cntl DYNAMIC -lock -fb_mode 1 -fdc C:/FPGA/higgs_sdr_rev2/fpgas/common/ip/lattice/sys_pll/sys_pll.fdc  */
/* Wed Jan 10 21:44:53 2018 */


`timescale 1 ns / 1 ps
module sys_pll (CLKI, PHASESEL, PHASEDIR, PHASESTEP, PHASELOADREG, CLKOP, 
    CLKOS, LOCK)/* synthesis NGD_DRC_MASK=1 */;
    input wire CLKI;
    input wire [1:0] PHASESEL;
    input wire PHASEDIR;
    input wire PHASESTEP;
    input wire PHASELOADREG;
    output wire CLKOP;
    output wire CLKOS;
    output wire LOCK;

    wire REFCLK;
    wire CLKOS_t;
    wire CLKOP_t;
    wire scuba_vhi;
    wire scuba_vlo;

    VHI scuba_vhi_inst (.Z(scuba_vhi));

    VLO scuba_vlo_inst (.Z(scuba_vlo));

    defparam PLLInst_0.PLLRST_ENA = "DISABLED" ;
    defparam PLLInst_0.INTFB_WAKE = "DISABLED" ;
    defparam PLLInst_0.STDBY_ENABLE = "DISABLED" ;
    defparam PLLInst_0.DPHASE_SOURCE = "ENABLED" ;
    defparam PLLInst_0.CLKOS3_FPHASE = 0 ;
    defparam PLLInst_0.CLKOS3_CPHASE = 0 ;
    defparam PLLInst_0.CLKOS2_FPHASE = 0 ;
    defparam PLLInst_0.CLKOS2_CPHASE = 0 ;
    defparam PLLInst_0.CLKOS_FPHASE = 0 ;
    defparam PLLInst_0.CLKOS_CPHASE = 124 ;
    defparam PLLInst_0.CLKOP_FPHASE = 0 ;
    defparam PLLInst_0.CLKOP_CPHASE = 5 ;
    defparam PLLInst_0.PLL_LOCK_MODE = 0 ;
    defparam PLLInst_0.CLKOS_TRIM_DELAY = 0 ;
    defparam PLLInst_0.CLKOS_TRIM_POL = "FALLING" ;
    defparam PLLInst_0.CLKOP_TRIM_DELAY = 0 ;
    defparam PLLInst_0.CLKOP_TRIM_POL = "FALLING" ;
    defparam PLLInst_0.OUTDIVIDER_MUXD = "DIVD" ;
    defparam PLLInst_0.CLKOS3_ENABLE = "DISABLED" ;
    defparam PLLInst_0.OUTDIVIDER_MUXC = "DIVC" ;
    defparam PLLInst_0.CLKOS2_ENABLE = "DISABLED" ;
    defparam PLLInst_0.OUTDIVIDER_MUXB = "DIVB" ;
    defparam PLLInst_0.CLKOS_ENABLE = "ENABLED" ;
    defparam PLLInst_0.OUTDIVIDER_MUXA = "DIVA" ;
    defparam PLLInst_0.CLKOP_ENABLE = "ENABLED" ;
    defparam PLLInst_0.CLKOS3_DIV = 1 ;
    defparam PLLInst_0.CLKOS2_DIV = 1 ;
    defparam PLLInst_0.CLKOS_DIV = 125 ;
    defparam PLLInst_0.CLKOP_DIV = 6 ;
    defparam PLLInst_0.CLKFB_DIV = 1 ;
    defparam PLLInst_0.CLKI_DIV = 1 ;
    defparam PLLInst_0.FEEDBK_PATH = "CLKOP" ;
    EHXPLLL PLLInst_0 (.CLKI(CLKI), .CLKFB(CLKOP_t), .PHASESEL1(PHASESEL[1]), 
        .PHASESEL0(PHASESEL[0]), .PHASEDIR(PHASEDIR), .PHASESTEP(PHASESTEP), 
        .PHASELOADREG(PHASELOADREG), .STDBY(scuba_vlo), .PLLWAKESYNC(scuba_vlo), 
        .RST(scuba_vlo), .ENCLKOP(scuba_vlo), .ENCLKOS(scuba_vlo), .ENCLKOS2(scuba_vlo), 
        .ENCLKOS3(scuba_vlo), .CLKOP(CLKOP_t), .CLKOS(CLKOS_t), .CLKOS2(), 
        .CLKOS3(), .LOCK(LOCK), .INTLOCK(), .REFCLK(REFCLK), .CLKINTFB())
             /* synthesis FREQUENCY_PIN_CLKOS="6.000000" */
             /* synthesis FREQUENCY_PIN_CLKOP="125.000000" */
             /* synthesis FREQUENCY_PIN_CLKI="125.000000" */
             /* synthesis ICP_CURRENT="9" */
             /* synthesis LPF_RESISTOR="72" */;

    assign CLKOS = CLKOS_t;
    assign CLKOP = CLKOP_t;


    // exemplar begin
    // exemplar attribute PLLInst_0 FREQUENCY_PIN_CLKOS 6.000000
    // exemplar attribute PLLInst_0 FREQUENCY_PIN_CLKOP 125.000000
    // exemplar attribute PLLInst_0 FREQUENCY_PIN_CLKI 125.000000
    // exemplar attribute PLLInst_0 ICP_CURRENT 9
    // exemplar attribute PLLInst_0 LPF_RESISTOR 72
    // exemplar end

endmodule