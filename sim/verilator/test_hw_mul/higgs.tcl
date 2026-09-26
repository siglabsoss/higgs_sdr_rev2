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

lappend writeback "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.writeBack_PC"

set num_added [ gtkwave::addSignalsFromList $writeback ]


gtkwave::/Edit/Insert_Comment "--- Eth: REGS ---"
# omit zero
# lappend bundle4 "TOP.q_engine.VexRiscv.RegFilePlugin_regFile(0)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(1)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(2)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(3)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(4)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(5)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(6)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(7)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(8)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(9)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(10)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(11)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(12)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(13)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(14)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(15)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(16)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(17)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(18)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(19)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(20)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(21)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(22)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(23)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(24)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(25)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(26)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(27)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(28)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(29)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(30)"
lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(31)"
set num_added [ gtkwave::addSignalsFromList $bundle4 ]


# gtkwave::/Edit/Insert_Comment "--- CS20: REGS ---"
# # omit zero
# # lappend bundle4 "TOP.q_engine.VexRiscv.RegFilePlugin_regFile(0)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(1)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(2)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(3)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(4)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(5)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(6)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(7)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(8)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(9)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(10)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(11)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(12)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(13)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(14)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(15)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(16)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(17)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(18)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(19)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(20)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(21)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(22)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(23)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(24)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(25)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(26)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(27)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(28)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(29)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(30)"
# lappend cs20regs "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(31)"
# set num_added [ gtkwave::addSignalsFromList $cs20regs ]

# gtkwave::/Edit/Insert_Comment "--- Eth: REGS ---"
# # omit zero
# # lappend bundle4 "TOP.q_engine.VexRiscv.RegFilePlugin_regFile(0)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(1)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(2)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(3)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(4)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(5)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(6)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(7)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(8)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(9)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(10)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(11)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(12)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(13)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(14)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(15)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(16)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(17)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(18)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(19)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(20)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(21)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(22)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(23)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(24)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(25)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(26)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(27)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(28)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(29)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(30)"
# lappend bundle4 "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(31)"
# set num_added [ gtkwave::addSignalsFromList $bundle4 ]


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





gtkwave::/Edit/Insert_Comment "--- X3 ---"
gtkwave::/Edit/Insert_Comment "eth cs20 cs10 cs00 cs01 cs11 cs21 cs31 cs30"
lappend x3bundle "TOP.tb_higgs_top.eth_top.q_engine_inst.VexRiscv.RegFilePlugin_regFile(3)"
lappend x3bundle "TOP.tb_higgs_top.cs20_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(3)"
lappend x3bundle "TOP.tb_higgs_top.cs10_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(3)"
lappend x3bundle "TOP.tb_higgs_top.cs00_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(3)"
lappend x3bundle "TOP.tb_higgs_top.cs01_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(3)"
lappend x3bundle "TOP.tb_higgs_top.cs11_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(3)"
lappend x3bundle "TOP.tb_higgs_top.cs21_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(3)"
lappend x3bundle "TOP.tb_higgs_top.cs31_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(3)"
lappend x3bundle "TOP.tb_higgs_top.cs30_top.vex_machine_top_inst.q_engine_inst.VexRiscv.RegFilePlugin_regFile(3)"
set num_added [ gtkwave::addSignalsFromList $x3bundle ]




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






gtkwave::/Edit/Insert_Comment "--- VMEM ---"

lappend bundle5 "TOP.q_engine.i_mem_data(1)"
lappend bundle5 "TOP.q_engine.i_mem_data(2)"
lappend bundle5 "TOP.q_engine.i_mem_data(3)"
lappend bundle5 "TOP.q_engine.i_mem_data(4)"
lappend bundle5 "TOP.q_engine.i_mem_data(5)"
lappend bundle5 "TOP.q_engine.i_mem_data(6)"
lappend bundle5 "TOP.q_engine.i_mem_data(7)"
lappend bundle5 "TOP.q_engine.i_mem_data(8)"
lappend bundle5 "TOP.q_engine.i_mem_data(9)"
lappend bundle5 "TOP.q_engine.i_mem_data(10)"
lappend bundle5 "TOP.q_engine.i_mem_data(11)"
lappend bundle5 "TOP.q_engine.i_mem_data(12)"
lappend bundle5 "TOP.q_engine.i_mem_data(13)"
lappend bundle5 "TOP.q_engine.i_mem_data(14)"
lappend bundle5 "TOP.q_engine.i_mem_data(15)"
lappend bundle5 "TOP.q_engine.i_mem_data(16)"
lappend bundle5 "TOP.q_engine.i_mem_data(17)"
lappend bundle5 "TOP.q_engine.i_mem_data(18)"
lappend bundle5 "TOP.q_engine.i_mem_data(19)"
lappend bundle5 "TOP.q_engine.i_mem_data(20)"
lappend bundle5 "TOP.q_engine.i_mem_data(21)"
lappend bundle5 "TOP.q_engine.i_mem_data(22)"
lappend bundle5 "TOP.q_engine.i_mem_data(23)"
lappend bundle5 "TOP.q_engine.i_mem_data(24)"
lappend bundle5 "TOP.q_engine.i_mem_data(25)"
lappend bundle5 "TOP.q_engine.i_mem_data(26)"
lappend bundle5 "TOP.q_engine.i_mem_data(27)"
lappend bundle5 "TOP.q_engine.i_mem_data(28)"
lappend bundle5 "TOP.q_engine.i_mem_data(29)"
lappend bundle5 "TOP.q_engine.i_mem_data(30)"
lappend bundle5 "TOP.q_engine.i_mem_data(31)"
set num_added [ gtkwave::addSignalsFromList $bundle5 ]

gtkwave::setZoomFactor -4

