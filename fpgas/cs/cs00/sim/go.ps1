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
Compile-Verilog ../../../../libs/ip-library/mib_bus/hdl/mib_master.sv
Compile-Verilog ../../../../libs/ip-library/mib_bus/hdl/mib_slave.sv
Compile-Verilog ../../../../libs/ip-library/mib_bus/hdl/mib_slave_wrapper.sv
Compile-Verilog ../../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
Compile-Verilog ../../../../libs/ip-library/interfaces/cmd_interface/hdl/cmd_master_example.sv
Compile-Verilog ../../../../libs/ip-library/apc/hdl/apc.sv
Compile-Verilog ../../../../libs/ip-library/apc/hdl/apc_abs2.sv
Compile-Verilog ../../../../libs/ip-library/apc/hdl/apc_cmulv2.sv
Compile-Verilog ../../../../libs/ip-library/apc/hdl/apc_moving_average.sv
Compile-Verilog ../../../../libs/ip-library/apc/hdl/apc_preproc.sv
Compile-Verilog ../../../../libs/ip-library/apc/hdl/apc_ram.sv
Compile-Verilog ../../../../libs/ip-library/argmax/hdl/argmax.sv
Compile-Verilog ../../../../libs/ip-library/lattice_support/fifos/pmi_fifo_dc_fwft_v1_0/hdl/pmi_fifo_dc_fwft_v1_0.sv
Compile-Verilog ../../../../libs/ip-library/lattice_support/fifos/pmi_fifo_sc_fwft_v1_0/hdl/pmi_fifo_sc_fwft_v1_0.sv
Compile-Verilog ../ip/cs00_pll/cs00_pll.v
Compile-Verilog ../hdl/REGS_00_pio.sv
Compile-Verilog ../hdl/cs00_cmd.sv
Compile-Verilog ../hdl/resetter.sv
Compile-Verilog ../hdl/piper.sv
Compile-Verilog ../hdl/mib_cdc.sv
Compile-Verilog ../hdl/lattice_mult_add_sum_v2_8.v
Compile-Verilog ../hdl/apc_argmax_wrapper.sv
Compile-Verilog intf_apc_stim.sv
Compile-Verilog stim_apc.sv
Compile-Verilog tb_cs00_top.sv
Compile-Verilog ../hdl/cs00_top.sv

# Execute the Simulation
vsimsa -do dosim.do
