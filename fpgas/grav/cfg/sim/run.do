vlib work
vmap work work

vlog -work work -sv ../hdl/fmc_slave_memory.sv 
vlog -work work -sv ../hdl/cfg_fpga_comm.sv 
vlog -work work -sv ../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv         
vlog -work work -sv ../../../libs/ip-library/fmc_slave/hdl/fmc_bridge.sv 
vlog -work work -sv ../../../libs/ip-library/fmc_slave/hdl/fmc_slave.sv 
vlog -work work -sv ../../../libs/ip-library/mib_bus/hdl/mib_master.sv           
vlog -work work -sv ../../../libs/ip-library/mib_bus/hdl/mib_master_wrapper.sv           
vlog -work work -sv ../../../libs/ip-library/mib_bus/hdl/mib_slave.sv           
vlog -work work -sv ../../../libs/ip-library/lattice_support/fifos/pmi_fifo_dc_fwft_v1_0/hdl/pmi_fifo_dc_fwft_v1_0.sv        
vlog -work work     ../ip/sys_clk_pll/sys_clk_pll.v
vlog -work work -sv ../hdl/cfg_top.sv
vlog -work work -sv ../hdl/mib_cdc.sv
vlog -work work -sv mib_slave_wrapper.sv
vlog -work work -sv sys_cmd.sv
vlog -work work -sv tb_cfg_top.sv

vsim -t 1ps -L pmi_work -L ecp5u_vlg -voptargs=+acc tb_cfg_top

log -r /*

do wave.do

run -all