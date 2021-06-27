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

Compile-Verilog ../../libs/ip-library/mib_bus/hdl/mib_master.sv
Compile-Verilog ../../libs/ip-library/mib_bus/hdl/mib_slave.sv
Compile-Verilog ../../libs/ip-library/mib_bus/hdl/mib_slave_wrapper.sv
Compile-Verilog ../../libs/ip-library/mib_bus/hdl/mib_master_wrapper.sv
Compile-Verilog ../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
Compile-Verilog ../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
Compile-Verilog ../../libs/ip-library/test/lfsr.sv
Compile-Verilog ../../libs/ip-library/test/validator.sv
Compile-Verilog ../cs/cs30/ip/cs30_pll/cs30_pll.v
Compile-Verilog ../cs/cs30/hdl/cs30_cmd.sv
Compile-Verilog ../cs/cs30/hdl/resetter.sv
Compile-Verilog ../cs/cs30/hdl/cs30_top.sv
Compile-Verilog ../cs/cs00/ip/cs00_pll/cs00_pll.v
Compile-Verilog ../cs/cs00/hdl/cs00_cmd.sv
Compile-Verilog ../cs/cs00/hdl/cs00_top.sv
Compile-Verilog ../cs/cs01/ip/cs01_pll/cs01_pll.v
Compile-Verilog ../cs/cs01/hdl/cs01_cmd.sv
Compile-Verilog ../cs/cs01/hdl/cs01_top.sv
Compile-Verilog ../cs/cs20/ip/cs20_pll/cs20_pll.v
Compile-Verilog ../cs/cs20/hdl/cs20_cmd.sv
Compile-Verilog ../cs/cs20/hdl/cs20_top.sv
Compile-Verilog ../cs/cs02/ip/cs02_pll/cs02_pll.v
Compile-Verilog ../cs/cs02/hdl/cs02_cmd.sv
Compile-Verilog ../cs/cs02/hdl/cs02_top.sv
Compile-Verilog tb_sys.sv

# Execute the Simulation
vsimsa -do dosim.do
