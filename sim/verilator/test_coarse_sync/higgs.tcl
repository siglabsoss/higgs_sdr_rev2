lappend system "clk"
lappend system "reset"
lappend system "TOP.tb_higgs_top.eth_top.cs20_srst"

set num_added [ gtkwave::addSignalsFromList $system ]





gtkwave::/Edit/Insert_Comment "--- Port 30000 ---"

lappend bundle07 "TOP.tx_turnstile_data_in"
lappend bundle07 "TOP.tx_turnstile_data_valid"

set num_added [ gtkwave::addSignalsFromList $bundle07 ]









gtkwave::/Edit/Insert_Comment "--- Port 20000 ---"

lappend bundle08 "TOP.ringbus_in_data"
lappend bundle08 "TOP.ringbus_in_data_vld"

set num_added [ gtkwave::addSignalsFromList $bundle08 ]






gtkwave::/Edit/Insert_Comment "--- Port 10000 ---"

lappend bundle09 "TOP.ringbus_out_data"
lappend bundle09 "TOP.ringbus_out_data_vld"

set num_added [ gtkwave::addSignalsFromList $bundle09 ]




gtkwave::/Edit/Insert_Comment "--- Eth: FPGA ---"


gtkwave::/Edit/Insert_Comment "--- Eth: PC ---"

lappend writeback "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.system_cpu.writeBack_PC"

set num_added [ gtkwave::addSignalsFromList $writeback ]


gtkwave::/Edit/Insert_Comment "--- Eth: REGS ---"
# omit zero
# lappend bundle4 "TOP.q_engine.VexRiscv.VexRiscv.RegFilePlugin_regFile(0)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(1)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(2)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(3)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(4)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(5)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(6)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(7)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(8)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(9)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(10)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(11)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(12)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(13)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(14)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(15)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(16)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(17)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(18)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(19)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(20)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(21)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(22)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(23)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(24)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(25)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(26)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(27)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(28)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(29)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(30)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.VexRiscv.RegFilePlugin_regFile(31)"
set num_added [ gtkwave::addSignalsFromList $bundle4 ]


gtkwave::/Edit/Insert_Comment "--- ETH: DMA in---"
# lappend bundle06 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_0_busy"
lappend bundle06 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_in_0.i0_addr"
lappend bundle06 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_in_0.t0_data"
lappend bundle06 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_in_0.t0_ready"
lappend bundle06 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_in_0.t0_valid"
lappend bundle06 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_in_0.strobe_complete"
lappend bundle06 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_0_interrupt"
lappend bundle06 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_0_interrupt_clear"
set num_added [ gtkwave::addSignalsFromList $bundle06 ]

gtkwave::/Edit/Insert_Comment "--- ETH: DMA out---"
# lappend bundle11 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_1_busy"
lappend bundle11 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_out_0.i0_addr"
lappend bundle11 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_out_0.t0_data"
lappend bundle11 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_out_0.t0_ready"
lappend bundle11 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_out_0.t0_valid"
lappend bundle06 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_out_0.strobe_complete"
lappend bundle11 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_1_interrupt"
lappend bundle11 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_1_interrupt_clear"
set num_added [ gtkwave::addSignalsFromList $bundle11 ]


gtkwave::/Edit/Insert_Comment "--- CS20: PC---"
# lappend bundle11 "TOP.tb_higgs_top.eth_top.q_engine_inst.dma_1_busy"
lappend cs20pc "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.VexRiscv.system_cpu.writeBack_PC"
set num_added [ gtkwave::addSignalsFromList $cs20pc ]




gtkwave::/Edit/Insert_Comment "--- X3 X4 ---"
gtkwave::/Edit/Insert_Comment "eth cs20 cs10 cs00 cs01 cs11 cs21 cs31 cs30"
lappend x3bundle "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.VexRiscv.VexRiscv.system_cpu.RegFilePlugin_regFile(3)"
lappend x3bundle "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.VexRiscv.VexRiscv.system_cpu.RegFilePlugin_regFile(4)"
set num_added [ gtkwave::addSignalsFromList $x3bundle ]

gtkwave::/Edit/Insert_Comment "--- V0 ---"

lappend v0 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_0.vreg(0)"
lappend v0 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_1.vreg(0)"
lappend v0 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_2.vreg(0)"
lappend v0 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_3.vreg(0)"
lappend v0 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_4.vreg(0)"
lappend v0 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_5.vreg(0)"
lappend v0 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_6.vreg(0)"
lappend v0 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_7.vreg(0)"
lappend v0 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_8.vreg(0)"

set num_added [ gtkwave::addSignalsFromList $v0 ]

gtkwave::/Edit/Insert_Comment "--- V1 ---"

lappend v1 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_0.vreg(1)"
lappend v1 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_1.vreg(1)"
lappend v1 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_2.vreg(1)"
lappend v1 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_3.vreg(1)"
lappend v1 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_4.vreg(1)"
lappend v1 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_5.vreg(1)"
lappend v1 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_6.vreg(1)"
lappend v1 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_7.vreg(1)"
lappend v1 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_8.vreg(1)"

set num_added [ gtkwave::addSignalsFromList $v1 ]

gtkwave::/Edit/Insert_Comment "--- V2 ---"

lappend v2 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_0.vreg(2)"
lappend v2 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_1.vreg(2)"
lappend v2 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_2.vreg(2)"
lappend v2 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_3.vreg(2)"
lappend v2 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_4.vreg(2)"
lappend v2 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_5.vreg(2)"
lappend v2 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_6.vreg(2)"
lappend v2 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_7.vreg(2)"
lappend v2 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_8.vreg(2)"

set num_added [ gtkwave::addSignalsFromList $v2 ]

gtkwave::/Edit/Insert_Comment "--- V3 ---"

lappend v3 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_0.vreg(3)"
lappend v3 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_1.vreg(3)"
lappend v3 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_2.vreg(3)"
lappend v3 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_3.vreg(3)"
lappend v3 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_4.vreg(3)"
lappend v3 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_5.vreg(3)"
lappend v3 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_6.vreg(3)"
lappend v3 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_7.vreg(3)"
lappend v3 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_8.vreg(3)"

set num_added [ gtkwave::addSignalsFromList $v3 ]

gtkwave::/Edit/Insert_Comment "--- i_k15 ---"

lappend i_k15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_0.i_k15_data"
lappend i_k15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_1.i_k15_data"
lappend i_k15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_2.i_k15_data"
lappend i_k15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_3.i_k15_data"
lappend i_k15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_4.i_k15_data"
lappend i_k15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_5.i_k15_data"
lappend i_k15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_6.i_k15_data"
lappend i_k15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_7.i_k15_data"
lappend i_k15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.unode13.vector_slice_8.i_k15_data"

set num_added [ gtkwave::addSignalsFromList $i_k15 ]

gtkwave::/Edit/Insert_Comment "--- K15 ---"

lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.i_ovs_dat"
lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.dat26_nxt"
lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.dat26_r"
lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.dat29"
lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.sel29"
lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.dat29_r0"
lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.dat29_r1"
lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.dat29_nxt"
lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.en29_0"
lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.ivs_ready15"
lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.ivs_valid15"
lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.en28"
lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.dat28"
lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.dat28_r"
lappend sk15 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.piston_inst.dat28_nxt"
set num_added [ gtkwave::addSignalsFromList $sk15 ]



gtkwave::/Edit/Insert_Comment "--- CS10: PC---"
lappend cs10pc "TOP.tb_higgs_top.cs10_top.vex_machine_top_inst.q_engine_inst.VexRiscv.VexRiscv.system_cpu.writeBack_PC"
set num_added [ gtkwave::addSignalsFromList $cs10pc ]


gtkwave::/Edit/Insert_Comment "--- CS30 OUT ---"
lappend cs30dmaout "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.i0_valid"
lappend cs30dmaout "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.i0_ready"
lappend cs30dmaout "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.i0_data"
set num_added [ gtkwave::addSignalsFromList $cs30dmaout ]

gtkwave::/Edit/Insert_Comment "--- CS30 IN ---"
lappend cs30dmain "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.t0_valid"
lappend cs30dmain "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.t0_ready"
lappend cs30dmain "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.t0_data"
set num_added [ gtkwave::addSignalsFromList $cs30dmain ]

gtkwave::/Edit/Insert_Comment "--- CS20 IN ---"
lappend cs20dmain "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.t0_valid"
lappend cs20dmain "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.t0_ready"
lappend cs20dmain "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.t0_data"
set num_added [ gtkwave::addSignalsFromList $cs20dmain ]


gtkwave::/Edit/Insert_Comment "--- CS10 IN ---"
lappend cs10dmain "TOP.tb_higgs_top.cs10_top.vex_machine_top_inst.t0_valid"
lappend cs10dmain "TOP.tb_higgs_top.cs10_top.vex_machine_top_inst.t0_ready"
lappend cs10dmain "TOP.tb_higgs_top.cs10_top.vex_machine_top_inst.t0_data"
set num_added [ gtkwave::addSignalsFromList $cs10dmain ]

gtkwave::/Edit/Insert_Comment "--- CS10 OUT ---"
lappend cs10dmaout "TOP.tb_higgs_top.cs10_top.vex_machine_top_inst.i0_valid"
lappend cs10dmaout "TOP.tb_higgs_top.cs10_top.vex_machine_top_inst.i0_ready"
lappend cs10dmaout "TOP.tb_higgs_top.cs10_top.vex_machine_top_inst.i0_data"
set num_added [ gtkwave::addSignalsFromList $cs10dmaout ]




gtkwave::/Edit/Insert_Comment "--- Rings ---"
gtkwave::/Edit/Insert_Comment "--- Rings Eth ---"
lappend bundle10 "TOP.tb_higgs_top.eth_top.q_engine_inst.ring_bus_inst.o_done_wr"
lappend bundle10 "TOP.tb_higgs_top.eth_top.i_ringbus"
lappend bundle10 "TOP.tb_higgs_top.eth_top.o_ringbus"
set num_added [ gtkwave::addSignalsFromList $bundle10 ]


gtkwave::/Edit/Insert_Comment "--- Rings 20 ---"
# lappend ringcs20 "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.i_ringbus"
lappend ringcs20 "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.o_ringbus"
set num_added [ gtkwave::addSignalsFromList $ringcs20 ]

gtkwave::/Edit/Insert_Comment "--- Rings 10 ---"
# lappend ringcs10 "TOP.tb_higgs_top.cs10_top.vex_machine_top_inst.i_ringbus"
lappend ringcs10 "TOP.tb_higgs_top.cs10_top.vex_machine_top_inst.o_ringbus"
set num_added [ gtkwave::addSignalsFromList $ringcs10 ]


gtkwave::/Edit/Insert_Comment "--- Rings 00 ---"
# lappend ringcs00 "TOP.tb_higgs_top.cs00_top.vex_machine_top_inst.i_ringbus"
lappend ringcs00 "TOP.tb_higgs_top.cs00_top.vex_machine_top_inst.o_ringbus"
set num_added [ gtkwave::addSignalsFromList $ringcs00 ]

gtkwave::/Edit/Insert_Comment "--- Rings 01 ---"
# lappend ringcs01 "TOP.tb_higgs_top.cs01_top.vex_machine_top_inst.i_ringbus"
lappend ringcs01 "TOP.tb_higgs_top.cs01_top.vex_machine_top_inst.o_ringbus"
set num_added [ gtkwave::addSignalsFromList $ringcs01 ]

gtkwave::/Edit/Insert_Comment "--- Rings 11 ---"
# lappend ringcs11 "TOP.tb_higgs_top.cs11_top.vex_machine_top_inst.i_ringbus"
lappend ringcs11 "TOP.tb_higgs_top.cs11_top.vex_machine_top_inst.o_ringbus"
set num_added [ gtkwave::addSignalsFromList $ringcs11 ]

gtkwave::/Edit/Insert_Comment "--- Rings 21 ---"
# lappend ringcs21 "TOP.tb_higgs_top.cs21_top.vex_machine_top_inst.i_ringbus"
lappend ringcs21 "TOP.tb_higgs_top.cs21_top.vex_machine_top_inst.o_ringbus"
set num_added [ gtkwave::addSignalsFromList $ringcs21 ]

gtkwave::/Edit/Insert_Comment "--- Rings 31 ---"
# lappend ringcs31 "TOP.tb_higgs_top.cs31_top.vex_machine_top_inst.i_ringbus"
lappend ringcs31 "TOP.tb_higgs_top.cs31_top.vex_machine_top_inst.o_ringbus"
set num_added [ gtkwave::addSignalsFromList $ringcs31 ]

gtkwave::/Edit/Insert_Comment "--- Rings 30 ---"
# lappend ringcs30 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.i_ringbus"
lappend ringcs30 "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.o_ringbus"
set num_added [ gtkwave::addSignalsFromList $ringcs30 ]
