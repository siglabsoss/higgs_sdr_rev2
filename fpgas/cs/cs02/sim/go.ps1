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
Compile-Verilog ../../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
Compile-Verilog ../../../../libs/ip-library/interfaces/cmd_interface/hdl/cmd_master_example.sv
Compile-Verilog ../../../../libs/ip-library/test/lfsr.sv
Compile-Verilog ../../../../libs/ip-library/test/validator.sv
Compile-Verilog ../../../../libs/ip-library/subcarrier_selector/hdl/subcarrier_selector.sv
vlog -work work -sv2k12 -dbg  ../../../../libs/ip-library/lattice_support/gbit_mac/modules/udp_packetizer/hdl/udp_packetizer.sv +incdir+../../../../libs/ip-library/lattice_support/gbit_mac/packages/
Compile-Verilog ../../../../libs/ip-library/lattice_support/fifos/pmi_fifo_dc_fwft_v1_0/hdl/pmi_fifo_dc_fwft_v1_0.sv
Compile-Verilog ../../../../libs/ip-library/lattice_support/fifos/pmi_fifo_sc_fwft_v1_0/hdl/pmi_fifo_sc_fwft_v1_0.sv
Compile-Verilog ../ip/cs02_pll/cs02_pll.v
Compile-Verilog ../hdl/cs02_cmd.sv
Compile-Verilog ../hdl/resetter.sv
Compile-Verilog ../hdl/piper.sv
Compile-Verilog ../hdl/REGS_02_pio.sv
Compile-Verilog ../hdl/mib_cdc.sv
Compile-Verilog tb_cs02_top.sv
Compile-Verilog ../hdl/cs02_top.sv

# Execute the Simulation
vsimsa -do dosim.do
