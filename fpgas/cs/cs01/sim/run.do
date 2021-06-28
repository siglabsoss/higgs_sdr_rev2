vlib work
vmap work work

if {![file isdirectory twidroms]} {
  mkdir twidroms
} 

file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_001_1024.mif  ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_002_1024.mif  ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_004_1024.mif  ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_008_1024.mif  ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_016_1024.mif  ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_032_1024.mif  ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_064_1024.mif  ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_128_1024.mif  ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_256_1024.mif  ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_512_1024.mif  ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_1024_1024.mif ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_001_1024.mif    ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_002_1024.mif    ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_004_1024.mif    ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_008_1024.mif    ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_016_1024.mif    ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_032_1024.mif    ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_064_1024.mif    ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_128_1024.mif    ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_256_1024.mif    ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_512_1024.mif    ./twidroms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_1024_1024.mif   ./twidroms/

if {![file isdirectory coeff_roms]} {
  mkdir coeff_roms
} 

file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom00.txt   ./coeff_roms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom01.txt   ./coeff_roms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom02.txt   ./coeff_roms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom03.txt   ./coeff_roms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom04.txt   ./coeff_roms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom05.txt   ./coeff_roms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom06.txt   ./coeff_roms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom07.txt   ./coeff_roms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom08.txt   ./coeff_roms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom09.txt   ./coeff_roms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom10.txt   ./coeff_roms/
file copy -force ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom11.txt   ./coeff_roms/

vlog -work work -sv ../../../../libs/ip-library/mib_bus/hdl/mib_master.sv
vlog -work work -sv ../../../../libs/ip-library/mib_bus/hdl/mib_slave.sv
vlog -work work -sv ../../../../libs/ip-library/mib_bus/hdl/mib_slave_wrapper.sv
vlog -work work -sv ../../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
vlog -work work -sv ../../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
vlog -work work -sv ../../../../libs/ip-library/interfaces/cmd_interface/hdl/cmd_master_example.sv
vlog -work work -sv ../../../../libs/ip-library/test/lfsr.sv
vlog -work work -sv ../../../../libs/ip-library/test/validator.sv
vlog -work work -sv ../../../../libs/ip-library/rx_turnstile/hdl/rx_aligned_counter.sv
vlog -work work -sv ../../../../libs/ip-library/rx_turnstile/hdl/rx_scheduling_counters.sv
vlog -work work -sv ../../../../libs/ip-library/rx_turnstile/hdl/rx_turnstile.sv
vlog -work work -sv ../../../../libs/ip-library/rx_turnstile/hdl/rx_turnstile_fifo.sv
vlog -work work -sv ../../../../libs/ip-library/lattice_support/fifos/pmi_fifo_dc_fwft_v1_0/hdl/pmi_fifo_dc_fwft_v1_0.sv
vlog -work work -sv ../../../../libs/ip-library/lattice_support/fifos/pmi_fifo_sc_fwft_v1_0/hdl/pmi_fifo_sc_fwft_v1_0.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/hdl/afb_ppf_coeff_rom.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/hdl/afb_ppf_dl.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/hdl/afb_ppf_mult.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/hdl/afb_ppf_round_half_up.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/hdl/afb_ppf_sum.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/hdl/afb_ppf.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/afb_ffto_butterfly.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/afb_ffto_complex_multiply.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/afb_ffto_multiplexer.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/afb_ffto_shift_regs.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/afb_ffto_single_stage.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/afb_ffto_twiddler.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/afb_fft.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb_reorderer/hdl/afb_reorderer.sv
vlog -work work -sv ../../../../libs/ip-library/analysis_filter_bank/afb/hdl/afb.sv
vlog -work work     ../ip/cs01_pll/cs01_pll.v
vlog -work work -sv ../hdl/cs01_cmd.sv
vlog -work work -sv ../hdl/resetter.sv
vlog -work work -sv ../hdl/REGS_01_pio.sv
vlog -work work -sv ../hdl/piper.sv
vlog -work work -sv ../hdl/mib_cdc.sv
vlog -work work -sv intf_afb.sv
vlog -work work -sv afb_stim.sv
vlog -work work -sv tb_cs01_top.sv
vlog -work work -sv ../hdl/cs01_top.sv

vsim -t 1ps -L pmi_work -L ecp5u_vlg -voptargs=+acc tb_cs01_top

log -r /*

do wave.do

run -all