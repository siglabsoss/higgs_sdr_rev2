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
Compile-Verilog ../../../libs/ip-library/mib_bus/hdl/mib_master.sv
Compile-Verilog ../../../libs/ip-library/mib_bus/hdl/mib_slave.sv
Compile-Verilog ../../../libs/ip-library/mib_bus/hdl/mib_slave_wrapper.sv
Compile-Verilog ../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
Compile-Verilog ../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
Compile-Verilog ../../../libs/ip-library/test/lfsr.sv
Compile-Verilog ../../../libs/ip-library/test/validator.sv
Compile-Verilog ../ip/lattice/sys_pll/sys_pll.v
Compile-Verilog ../hdl/resetter.sv
Compile-Verilog ../hdl/cs30_top.sv
Compile-Verilog tb_cs30_top.sv

# Execute the Simulation
vsimsa -do dosim.do
