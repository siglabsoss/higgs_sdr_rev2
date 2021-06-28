vlib work
vmap work work

vlog -work work -sv ../../../../libs/ip-library/mib_bus/hdl/mib_master.sv
vlog -work work -sv ../../../../libs/ip-library/mib_bus/hdl/mib_slave.sv
vlog -work work -sv ../../../../libs/ip-library/mib_bus/hdl/mib_slave_wrapper.sv
vlog -work work -sv ../../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
vlog -work work -sv ../../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv
vlog -work work -sv ../../../../libs/ip-library/interfaces/cmd_interface/hdl/cmd_master_example.sv
vlog -work work -sv ../../../../libs/ip-library/test/lfsr.sv
vlog -work work -sv ../../../../libs/ip-library/test/validator.sv
vlog -work work -sv ../../../../libs/ip-library/subcarrier_selector/hdl/subcarrier_selector.sv
vlog -work work -sv ../../../../libs/ip-library/lattice_support/gbit_mac/modules/udp_packetizer/hdl/udp_packetizer.sv +incdir+../../../../libs/ip-library/lattice_support/gbit_mac/packages/
vlog -work work -sv ../../../../libs/ip-library/lattice_support/fifos/pmi_fifo_dc_fwft_v1_0/hdl/pmi_fifo_dc_fwft_v1_0.sv
vlog -work work -sv ../../../../libs/ip-library/lattice_support/fifos/pmi_fifo_sc_fwft_v1_0/hdl/pmi_fifo_sc_fwft_v1_0.sv
vlog -work work     ../ip/cs02_pll/cs02_pll.v
vlog -work work -sv ../hdl/cs02_cmd.sv
vlog -work work -sv ../hdl/resetter.sv
vlog -work work -sv ../hdl/piper.sv
vlog -work work -sv ../hdl/REGS_02_pio.sv
vlog -work work -sv ../hdl/mib_cdc.sv
vlog -work work -sv tb_cs02_top.sv
vlog -work work -sv ../hdl/cs02_top.sv

vsim -t 1ps -L pmi_work -L ecp5u_vlg -novopt tb_cs02_top

log -r /*

do wave.do

run -all