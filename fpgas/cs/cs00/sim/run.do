vlib work
vmap work work

vlog -work work -sv ../../../../libs/ip-library/mib_bus/hdl/mib_master.sv
vlog -work work -sv ../../../../libs/ip-library/mib_bus/hdl/mib_slave.sv
vlog -work work -sv ../../../../libs/ip-library/mib_bus/hdl/mib_slave_wrapper.sv
vlog -work work -sv ../../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
vlog -work work -sv ../../../../libs/ip-library/interfaces/cmd_interface/hdl/cmd_master_example.sv
vlog -work work -sv ../../../../libs/ip-library/apc/hdl/apc.sv
vlog -work work -sv ../../../../libs/ip-library/apc/hdl/apc_abs2.sv
vlog -work work -sv ../../../../libs/ip-library/apc/hdl/apc_cmulv2.sv
vlog -work work -sv ../../../../libs/ip-library/apc/hdl/apc_moving_average.sv
vlog -work work -sv ../../../../libs/ip-library/apc/hdl/apc_preproc.sv
vlog -work work -sv ../../../../libs/ip-library/apc/hdl/apc_ram.sv
vlog -work work -sv ../../../../libs/ip-library/argmax/hdl/argmax.sv
vlog -work work -sv ../../../../libs/ip-library/lattice_support/fifos/pmi_fifo_dc_fwft_v1_0/hdl/pmi_fifo_dc_fwft_v1_0.sv
vlog -work work -sv ../../../../libs/ip-library/lattice_support/fifos/pmi_fifo_sc_fwft_v1_0/hdl/pmi_fifo_sc_fwft_v1_0.sv
vlog -work work     ../ip/cs00_pll/cs00_pll.v
vlog -work work -sv ../hdl/REGS_00_pio.sv
vlog -work work -sv ../hdl/cs00_cmd.sv
vlog -work work -sv ../hdl/resetter.sv
vlog -work work -sv ../hdl/piper.sv
vlog -work work -sv ../hdl/mib_cdc.sv
vlog -work work ../hdl/lattice_mult_add_sum_v2_8.v
vlog -work work -sv ../hdl/apc_argmax_wrapper.sv
vlog -work work -sv intf_apc_stim.sv
vlog -work work -sv stim_apc.sv
vlog -work work -sv tb_cs00_top.sv
vlog -work work -sv ../hdl/cs00_top.sv

vsim -t 1ps -L pmi_work -L ecp5u_vlg -voptargs=+acc tb_cs00_top

log -r /*

do wave.do

run -all