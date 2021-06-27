if {!0} {
    vlib work 
}
vmap work work
#==== compile
vlog -novopt -incr \
+incdir+../../src/params \
+incdir+../../../testbench/top \
+incdir+../../../testbench/tests \
-y C:/lscc/diamond/3.9_x64/cae_library/simulation/verilog/pmi +libext+.v \
-y C:/lscc/diamond/3.9_x64/cae_library/simulation/verilog/ecp5u +libext+.v \
-y ../../src/rtl/template +libext+.v \
-y ../../../models/ecp5u +libext+.v \
../../src/params/ts_mac_defines.v \
../../../../gbit_mac_beh.v \
../../src/rtl/top/ts_mac_top.v \
../../../testbench/top/pkt_mon.v \
../../../testbench/top/orcastra_drv.v \
../../../testbench/top/test_ts_mac.v

#==== run the simulation
vsim -novopt -L work work.test_ts_mac -l gbit_mac_eval.log 

view structure wave
do wave.do
run -all
