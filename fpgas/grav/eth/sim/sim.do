
# CREATE A LIBRARY CALLED WORK

vlib work


# COMPILE VERILOG AND SYSTEM VERILOG SOURCES 

vlog +define+SIM_MODE -work work ../hdl/eth_top.sv
vlog +define+SIM_MODE -work work ../hdl/pmi_fifo_dc_fwft.sv
vlog -work work ../ip/lattice/sys_pll/sys_pll.v
vlog -work work ../../../libs/ip-library/lattice_gbit_mac_support/modules/udp_packetizer/hdl/*.sv
vlog -work work ../../../libs/ip-library/lattice_gbit_mac_support/modules/udp_packetizer/ip/lattice/udp_pkt_fifo/udp_pkt_fifo.v
vlog -work work ./*.sv


# SIMULATE LOGGING TO SIM.LOG, ALLOWING READ ACCESS TO SIGNALS, USING LIBRARY WORK, AND WITH TEST BENCH XXX

vsim -log sim.log +access +r -lib work -L pmi_work -L ovi_ecp5u tb_eth_top


# LOG ALL SIGNALS IN DESIGN

trace -decl *
trace -decl DUT/*
#trace -decl dac_fifo/*


# RUN UNTIL NO MORE EVENTS TO SIMULATE

run -all


# LAUNCH ACTIVE-HDL AND VIEW WAVEFORM
#
# Use -do "open -asdb wave.asdb" if you don't have a wave.do file you'd like to use.
#
# Use -do "view wave; waveconnect wave.asdb; do wave.do; wavezoom -fit" if you do.

runexe avhdl.exe -nosplash -do "open -asdb wave.asdb" "view wave; waveconnect wave.asdb" 
#runexe avhdl.exe -nosplash -do "view wave; waveconnect wave.asdb; do wave.do; wavezoom -fit" 


# PEACE OUT

bye
