vsim -c -log sim.log +access +r -lib work -L pmi_work tb_eth_dsp_counters
trace -rec *
trace -rec uut/*
run -all
# asdb2vcd wave.asdb wave.vcd
exit
