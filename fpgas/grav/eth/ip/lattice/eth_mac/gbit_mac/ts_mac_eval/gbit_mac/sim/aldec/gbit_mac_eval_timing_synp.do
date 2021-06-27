cd "C:/FPGA/gravitinolink/experiments/tempy_temp/eth_mac/gbit_mac/ts_mac_eval/gbit_mac/sim/aldec"
workspace create tsmac_test
design create tsmac_test . 
design open tsmac_test     
waveformmode asdb
cd "C:/FPGA/gravitinolink/experiments/tempy_temp/eth_mac/gbit_mac/ts_mac_eval/gbit_mac/sim/aldec"
#==== compile
vlog  -incr \
+incdir+../../src/params \
+incdir+../../../testbench/top \
+incdir+../../../testbench/tests \
-y ../../../models/ecp5u +libext+.v \
../../impl/synplify/impl/gbit_mac_reference_eval_impl_vo.vo \
../../src/params/ts_mac_defines.v \
../../../testbench/top/pkt_mon.v \
../../../testbench/top/orcastra_drv.v \
../../../testbench/top/test_ts_mac.v

#==== run the simulation
vsim  -o5 +access +r -lib tsmac_test -L ovi_ecp5u -L pmi_work \
      -multisource_delay max +transport_int_delays +transport_path_delays +notimingchecks \
      tsmac_test.test_ts_mac

view wave
do wave_sdf.do
run -all
