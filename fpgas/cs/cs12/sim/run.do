vlib work
vmap work work

vlog -work work -sv ../../../../libs/ip-library/mib_bus/hdl/mib_master.sv
vlog -work work -sv ../../../../libs/ip-library/mib_bus/hdl/mib_slave.sv
vlog -work work -sv ../../../../libs/ip-library/mib_bus/hdl/mib_slave_wrapper.sv
vlog -work work -sv ../../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
vlog -work work -sv ../../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
vlog -work work -sv ../../../../libs/ip-library/test/lfsr.sv
vlog -work work -sv ../../../../libs/ip-library/test/validator.sv
vlog -work work     ../ip/cs12_pll/cs12_pll.v
vlog -work work -sv ../hdl/piper.sv
vlog -work work -sv ../hdl/REGS_12_pio.sv
vlog -work work -sv ../hdl/resetter.sv
vlog -work work -sv tb_cs12_top.sv
vlog -work work -sv ../hdl/cs12_top.sv

vsim -t 1ps -L pmi_work -L ecp5u_vlg -voptargs=+acc tb_cs12_top

log -r /*

do wave.do

run -all