function Compile-Verilog
{
	param( [string]$Filename)
	vlog -work work +cdefine+SIM_MODE -sv2k12 -dbg $Filename

	if ($LastExitCode -ne 0) {
		echo "                                                                "
		echo "    ############################################################"
		$msg = "      Compilation of " + $Filename + " failed! "
		echo $msg
		echo "    ############################################################"
		echo "                                                                "
		exit
	}
}

# Create the Work Library
vlib work

$srcdir = "..\..\hdl"
$intdir = "..\..\..\..\libs\ip-library\interfaces\cmd_interface\hdl"
$fifodir = "..\..\..\..\libs\ip-library\lattice_support\fifos\pmi_fifo_dc_fwft_v1_0\hdl"

# Compile the Verilog Files
Compile-Verilog tb_eth_dsp_counters.sv
Compile-Verilog $srcdir\eth_dsp_counters.sv
Compile-Verilog $intdir\intf_cmd.sv
Compile-Verilog $fifodir\pmi_fifo_dc_fwft_v1_0.sv

# # Remove output files
# Remove-Item test3.txt -force -ErrorAction silentlycontinue
# Remove-Item test4.txt -force -ErrorAction silentlycontinue
# Remove-Item test5.txt -force -ErrorAction silentlycontinue

# Execute the Simulation
vsimsa -do dosim.do
