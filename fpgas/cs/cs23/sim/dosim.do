vsim -c -log sim.log +access +r -PL pmi_work -L ovi_ecp5u -lib work tb_cs23_top
trace -rec *
run -all
# asdb2vcd wave.asdb wave.vcd
exit
