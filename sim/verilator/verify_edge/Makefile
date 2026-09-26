HIGGS_ROOT=../../..
###############
#
#    Per FPGA compiled c/asm/hex override section
#    This tb allows for overriding the compiled code of any FPGA
#    simply set as many of (OVERRIDE_CS20_C, ...) to a value and
#    create the corresponding folder in the ./override folder
#    include makefile_sim_verialtor.mk afterwards and it's all good to go
#
#    the override folder mirrors the structure of higgs_sdr_rev2
#

# set as many of these as you want, setting to 0 does not disable, you must comment
# If you are NOT overriding a particular FPGA, the QENGINE_LITE setting below must match
# what the FPGA has set inside fpga_top.sv
OVERRIDE_CS20_C=1
OVERRIDE_CS10_C=1
OVERRIDE_CS00_C=1
OVERRIDE_CS01_C=1
OVERRIDE_CS11_C=1
OVERRIDE_CS21_C=1
OVERRIDE_CS31_C=1
OVERRIDE_CS30_C=1
# OVERRIDE_ETH_C=1

# set as many of these as you want, setting to 0 does not disable, you must comment
TB_USE_CS20=1
TB_USE_CS10=1
TB_USE_CS00=1
TB_USE_CS01=1
TB_USE_CS11=1
TB_USE_CS21=1
TB_USE_CS31=1
TB_USE_CS30=1
# TB_USE_DAC=1

# This option removes the RISCV processor, but leaves the ringbus
# For example this would allow a simulation with only eth,cs20,cs10
# but still allow cs10 to send ringbus messages back to eth
# CS20_NO_RISCV=1
# CS10_NO_RISCV=1
# CS00_NO_RISCV=1
# CS01_NO_RISCV=1
# CS11_NO_RISCV=1
# CS21_NO_RISCV=1
# CS31_NO_RISCV=1
# CS30_NO_RISCV=1

include $(HIGGS_ROOT)/scripts/make_include/tb_common.mk

# any custom test or clean targets must come after tb_common.mk or they
# will not override correctly


show:
	gtkwave wave_dump.vcd --giga -S higgs.tcl &


.PHONY: edge_edge_test edge_edge_reset edge1 edge2 edge3 edge4 edge5 edge6 edge7 edge_test edge_bootload edge_do_test

edge_edge_reset:
	python scripts/edge_to_edge_reset.py

edge_test:
	make edge_bootload
	make edge_edge_test

edge_bootload:
	make vall
	make bootload_cs00 bootload_cs01 bootload_cs10 bootload_cs11
	make bootload_cs20 bootload_cs21 bootload_cs30 bootload_cs31


edge_edge_test:
	make edge1
	make edge2
	make edge3
	make edge4
	make edge5
	make edge6
	make edge7
	@echo ""
	@echo ""
	@echo "------------------------------------------------"
	@echo "-                                              -"
	@echo "-          all edge-edge tests passed          -"
	@echo "-                                              -"
	@echo "------------------------------------------------"
	@echo ""

edge1:
	python ../../../scripts/edge_edge_test.py cs00 cs01

edge2:
	python ../../../scripts/edge_edge_test.py cs01 cs11

edge3:
	python ../../../scripts/edge_edge_test.py cs11 cs21

edge4:
	python ../../../scripts/edge_edge_test.py cs21 cs31

edge5:
	python ../../../scripts/edge_edge_test.py cs31 cs30

edge6:
	python ../../../scripts/edge_edge_test.py cs20 cs10

edge7:
	python ../../../scripts/edge_edge_test.py cs30 eth
