function Compile-Verilog
{
	param( [string]$Filename)
	vlog -work work -sv2k12 -dbg $Filename

	if ($LastExitCode -ne 0) {
		echo "                                                                "
		echo "    ############################################################"
		echo "      Compilation of " + $Filename + " failed! "
		echo "    ############################################################"
		echo "                                                                "
		exit
	}
}

# Create the Work Library
vlib work

# Compile the Verilog Files

if(![System.IO.File]::Exists('twidroms')){
    mkdir twidroms
}


Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_001_1024.mif  ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_002_1024.mif  ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_004_1024.mif  ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_008_1024.mif  ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_016_1024.mif  ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_032_1024.mif  ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_064_1024.mif  ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_128_1024.mif  ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_256_1024.mif  ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_512_1024.mif  ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_cosines_18_1024_1024.mif ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_001_1024.mif    ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_002_1024.mif    ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_004_1024.mif    ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_008_1024.mif    ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_016_1024.mif    ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_032_1024.mif    ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_064_1024.mif    ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_128_1024.mif    ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_256_1024.mif    ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_512_1024.mif    ./twidroms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/twidroms/afb_ffto_sines_18_1024_1024.mif   ./twidroms/

if(![System.IO.File]::Exists('coeff_roms')){
    mkdir coeff_roms
}

Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom00.txt   ./coeff_roms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom01.txt   ./coeff_roms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom02.txt   ./coeff_roms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom03.txt   ./coeff_roms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom04.txt   ./coeff_roms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom05.txt   ./coeff_roms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom06.txt   ./coeff_roms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom07.txt   ./coeff_roms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom08.txt   ./coeff_roms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom09.txt   ./coeff_roms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom10.txt   ./coeff_roms/
Copy-Item ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/coeff_roms/rom11.txt   ./coeff_roms/

Compile-Verilog ../../../../libs/ip-library/mib_bus/hdl/mib_master.sv
Compile-Verilog ../../../../libs/ip-library/mib_bus/hdl/mib_slave.sv
Compile-Verilog ../../../../libs/ip-library/mib_bus/hdl/mib_slave_wrapper.sv
Compile-Verilog ../../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
Compile-Verilog ../../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
Compile-Verilog ../../../../libs/ip-library/interfaces/cmd_interface/hdl/cmd_master_example.sv
Compile-Verilog ../../../../libs/ip-library/test/lfsr.sv
Compile-Verilog ../../../../libs/ip-library/test/validator.sv
Compile-Verilog ../../../../libs/ip-library/rx_turnstile/hdl/rx_aligned_counter.sv
Compile-Verilog ../../../../libs/ip-library/rx_turnstile/hdl/rx_scheduling_counters.sv
Compile-Verilog ../../../../libs/ip-library/rx_turnstile/hdl/rx_turnstile.sv
Compile-Verilog ../../../../libs/ip-library/rx_turnstile/hdl/rx_turnstile_fifo.sv
Compile-Verilog ../../../../libs/ip-library/lattice_support/fifos/pmi_fifo_dc_fwft_v1_0/hdl/pmi_fifo_dc_fwft_v1_0.sv
Compile-Verilog ../../../../libs/ip-library/lattice_support/fifos/pmi_fifo_sc_fwft_v1_0/hdl/pmi_fifo_sc_fwft_v1_0.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/hdl/afb_ppf_coeff_rom.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/hdl/afb_ppf_dl.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/hdl/afb_ppf_mult.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/hdl/afb_ppf_round_half_up.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/hdl/afb_ppf_sum.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb_ppf/hdl/afb_ppf.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/afb_ffto_butterfly.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/afb_ffto_complex_multiply.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/afb_ffto_multiplexer.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/afb_ffto_shift_regs.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/afb_ffto_single_stage.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/submodules/afb_ffto_twiddler.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb_fft/hdl/afb_fft.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb_reorderer/hdl/afb_reorderer.sv
Compile-Verilog ../../../../libs/ip-library/analysis_filter_bank/afb/hdl/afb.sv
Compile-Verilog ../ip/cs01_pll/cs01_pll.v
Compile-Verilog ../hdl/cs01_cmd.sv
Compile-Verilog ../hdl/resetter.sv
Compile-Verilog ../hdl/REGS_01_pio.sv
Compile-Verilog ../hdl/piper.sv
Compile-Verilog ../hdl/mib_cdc.sv
Compile-Verilog intf_afb.sv
Compile-Verilog afb_stim.sv
Compile-Verilog tb_cs01_top.sv
Compile-Verilog ../hdl/cs01_top.sv

# Execute the Simulation
vsimsa -do dosim.do
