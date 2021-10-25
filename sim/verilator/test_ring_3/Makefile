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
# TX FPGAs
OVERRIDE_CS11_C=1
# OVERRIDE_CS12_C=1
# OVERRIDE_CS02_C=1
# OVERRIDE_CS01_C=1
# RX FPGAs
# OVERRIDE_CS31_C=1
# OVERRIDE_CS32_C=1
# OVERRIDE_CS22_C=1
# OVERRIDE_CS21_C=1
OVERRIDE_CS20_C=1
# OVERRIDE_ETH_C=1


# set as many of these as you want, setting to 0 does not disable, you must comment
# TX FPGAs
TB_USE_CS11=1
TB_USE_CS12=1
TB_USE_CS02=1
TB_USE_CS01=1
# TB_USE_DAC=1
# RX FPGAs
TB_USE_CS31=1
TB_USE_CS32=1
TB_USE_CS22=1
TB_USE_CS21=1
TB_USE_CS20=1

# This option removes the RISCV processor, but leaves the ringbus
# For example this could allow a simulation with only ETH, CS11, CS12, CS02,
# CS01 but still allow CS01 to send ringbus messages back to ETH
# TX FPGAs
# CS11_NO_RISCV=1
CS12_NO_RISCV=1
CS02_NO_RISCV=1
CS01_NO_RISCV=1
# RX FPGAs
CS31_NO_RISCV=1
CS32_NO_RISCV=1
CS22_NO_RISCV=1
CS21_NO_RISCV=1
# CS20_NO_RISCV=1

EXTRA_CPP_FILES=$(RISCV_BASEBAND_REPO)/c/inc/feedback_bus.c \
$(SMODEM_REPO)/soapy/src/common/FileUtils.cpp \
$(SMODEM_REPO)/soapy/src/common/GenericOperator.cpp


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
	make -C override/fpgas/cs/cs01/c all
	make -C override/fpgas/cs/cs02/c all
	make -C override/fpgas/cs/cs12/c all
	make -C override/fpgas/cs/cs11/c all
	make bootload_cs01 && sleep 1
	make bootload_cs02 && sleep 1
	make bootload_cs12 && sleep 1
	make bootload_cs11 && sleep 1


clean: cleanall
	rm -f cs01.out cs02.out cs12.out cs11.out cs01_out.hex cs02_out.hex cs12_out.hex cs11_out.hex output.txt t1.txt


test: test_dependencies
	TEST_SELECT=8 make run



test7: no_clean_test_dependencies
	TEST_SELECT=7 make quickt
	@echo ""
	@echo "expected"
	@md5sum test7_cs11_in.hex
	@echo ""
	@echo "got"
	@md5sum cs11_in.hex
	@echo ""
	@echo ""
