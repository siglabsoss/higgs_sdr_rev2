vlib work
vmap work work

vlog -work work -sv ../../libs/ip-library/mib_bus/hdl/mib_master.sv
vlog -work work -sv ../../libs/ip-library/mib_bus/hdl/mib_slave.sv
vlog -work work -sv ../../libs/ip-library/mib_bus/hdl/mib_slave_wrapper.sv
vlog -work work -sv ../../libs/ip-library/mib_bus/hdl/mib_master_wrapper.sv
vlog -work work -sv ../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
vlog -work work -sv ../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
vlog -work work -sv ../../libs/ip-library/lattice_support/fifos/pmi_fifo_dc_fwft_v1_0/hdl/pmi_fifo_dc_fwft_v1_0.sv
vlog -work work -sv ../../libs/ip-library/test/lfsr.sv
vlog -work work -sv ../../libs/ip-library/test/validator.sv
vlog -work work -sv ../../libs/ip-library/fmc_slave/hdl/fmc_bridge.sv
vlog -work work -sv ../../libs/ip-library/fmc_slave/hdl/fmc_slave.sv 

vlog -work work     ../cs/cs30/ip/cs30_pll/cs30_pll.v
vlog -work work -sv ../cs/cs30/hdl/cs30_cmd.sv
vlog -work work -sv ../cs/cs30/hdl/resetter.sv
vlog -work work -sv ../cs/cs30/hdl/cs30_top.sv
vlog -work work     ../cs/cs00/ip/cs00_pll/cs00_pll.v
vlog -work work -sv ../cs/cs00/hdl/cs00_cmd.sv
vlog -work work -sv ../cs/cs00/hdl/cs00_top.sv
vlog -work work     ../cs/cs01/ip/cs01_pll/cs01_pll.v
vlog -work work -sv ../cs/cs01/hdl/cs01_cmd.sv
vlog -work work -sv ../cs/cs01/hdl/cs01_top.sv

vlog -work work     ../cs/cs02/ip/cs02_pll/cs02_pll.v
vlog -work work -sv ../cs/cs02/hdl/cs02_cmd.sv
vlog -work work -sv ../cs/cs02/hdl/cs02_top.sv

vlog -work work     ../cs/cs12/ip/cs12_pll/cs12_pll.v
vlog -work work -sv ../cs/cs12/hdl/cs12_cmd.sv
vlog -work work -sv ../cs/cs12/hdl/cs12_top.sv

vlog -work work     ../cs/cs22/ip/cs22_pll/cs22_pll.v
vlog -work work -sv ../cs/cs22/hdl/cs22_cmd.sv
vlog -work work -sv ../cs/cs22/hdl/cs22_top.sv

vlog -work work     ../cs/cs21/ip/cs21_pll/cs21_pll.v
vlog -work work -sv ../cs/cs21/hdl/cs21_cmd.sv
vlog -work work -sv ../cs/cs21/hdl/cs21_top.sv

vlog -work work     ../cs/cs20/ip/cs20_pll/cs20_pll.v
vlog -work work -sv ../cs/cs20/hdl/cs20_cmd.sv
vlog -work work -sv ../cs/cs20/hdl/cs20_top.sv

vlog -work work -sv ../cs/cscfg/hdl/fmc_slave_memory.sv
vlog -work work -sv ../cs/cscfg/hdl/cfg_fpga_comm.sv 
vlog -work work     ../cs/cscfg/ip/cscfg_pll/cscfg_pll.v 
vlog -work work -sv ../cs/cscfg/hdl/cscfg_top.sv 
vlog -work work -sv ../cs/cscfg/hdl/mib_cdc.sv 

vlog -work work -sv fmc_cmd.sv
vlog -work work -sv fmc_stim.sv
vlog -work work -sv tb_sys.sv

vsim -t 1ps -L pmi_work -L ecp5u_vlg -voptargs=+acc tb_sys

log -r /*

do wave.do

run -all