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
# OVERRIDE_CS00_C=1
# OVERRIDE_CS01_C=1
# OVERRIDE_CS11_C=1
# OVERRIDE_CS21_C=1
# OVERRIDE_CS31_C=1
# OVERRIDE_CS30_C=1
# OVERRIDE_ETH_C=1

# set as many of these as you want, setting to 0 does not disable, you must comment
TB_USE_CS20=1
#TB_USE_CS10=1
#TB_USE_CS00=1
#TB_USE_CS01=1
#TB_USE_CS11=1
#TB_USE_CS21=1
#TB_USE_CS31=1
#TB_USE_CS30=1
# TB_USE_DAC=1

# THESE DONT WORK
# These are only respected when under verilator.  If OVERRIDE above is not set this
# setting must match what the fpga_top.sv fpga uses for QENGINE_LITE
#CS20_QENGINE_LITE=1
# CS10_QENGINE_LITE=1
# CS00_QENGINE_LITE=1
# CS01_QENGINE_LITE=1
# CS11_QENGINE_LITE=1
# CS21_QENGINE_LITE=1
# CS31_QENGINE_LITE=1
# CS30_QENGINE_LITE=1
# ETH_QENGINE_LITE=1

# This option removes the RISCV processor, but leaves the ringbus
# For example this could allow a simulation with only eth,cs20,cs10
# but still allow cs10 to send ringbus messages back to eth
# CS20_NO_RISCV=1
# CS10_NO_RISCV=1
CS00_NO_RISCV=1
CS01_NO_RISCV=1
CS11_NO_RISCV=1
CS21_NO_RISCV=1
CS31_NO_RISCV=1
CS30_NO_RISCV=1

EXTRA_CPP_FILES=$(RISCV_BASEBAND_REPO)/c/inc/feedback_bus.c\
$(SMODEM_REPO)/soapy/src/common/ParseReveal.cpp



# required to set this before including this file
# HIGGS_TEST_DIR=.
# include $(HIGGS_ROOT)/scripts/make_include/makefile_sim_verialtor.mk

# required to set this before including this file
HIGGS_TEST_DIR=.
include $(HIGGS_ROOT)/scripts/make_include/tb_common.mk

.PHONY: bootload

bootload:
	make -C ../../../ vall
	make -C ../../../ bootload_eth && sleep 1
	make -C override/fpgas/cs/cs10/c all
	make -C override/fpgas/cs/cs20/c all
	make bootload_cs10 && sleep 1
	make bootload_cs20 && sleep 1


clean: cleanall
	rm -f cs10.out cs20.out cs30.out cs10_out.hex cs20_out.hex output.txt t1.txt


