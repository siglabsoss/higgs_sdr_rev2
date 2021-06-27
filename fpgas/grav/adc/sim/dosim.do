vsim -c -log sim.log +access +r -lib work tb_adc_dsp
trace -rec *
run -all
asdb2vcd wave.asdb wave.vcd
exit
