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
TB_USE_CS10=1
TB_USE_CS00=1
TB_USE_CS01=1
TB_USE_CS11=1
TB_USE_CS21=1
TB_USE_CS31=1
TB_USE_CS30=1
# TB_USE_DAC=1

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

EXTRA_CPP_FILES=$(RISCV_BASEBAND_REPO)/c/inc/feedback_bus.c

include $(HIGGS_ROOT)/scripts/make_include/tb_common.mk


clean: cleanall
	rm -f cs10.out cs20.out cs30.out cs10_out.hex cs20_out.hex cs30_out.hex output.txt t1.txt


bootload:
	make vall
	make bootload_cs30
	make bootload_cs31
	make bootload_cs21
	make bootload_cs11
	make bootload_cs01
	make bootload_cs00

SEED_RESULTS=seed_results.txt

# assumes pre_work or similar has already been done
bad_seeds:
	make pre_work
	rm -f ${SEED_RESULTS}

	# newly found
	TEST_SEED=1536021521 make run; echo $$? >> ${SEED_RESULTS}; true
	TEST_SEED=1536021538 make run; echo $$? >> ${SEED_RESULTS}; true
	TEST_SEED=1536025417 make run; echo $$? >> ${SEED_RESULTS}; true
	TEST_SEED=1536026565 make run; echo $$? >> ${SEED_RESULTS}; true

	# existing
	TEST_SEED=1528427758 make run; echo $$? >> ${SEED_RESULTS}; true
	TEST_SEED=1528540978 make run; echo $$? >> ${SEED_RESULTS}; true
	TEST_SEED=1529277192 make run; echo $$? >> ${SEED_RESULTS}; true
	TEST_SEED=1529290884 make run; echo $$? >> ${SEED_RESULTS}; true
	TEST_SEED=1529300722 make run; echo $$? >> ${SEED_RESULTS}; true
	TEST_SEED=1529320397 make run; echo $$? >> ${SEED_RESULTS}; true
	TEST_SEED=1529335532 make run; echo $$? >> ${SEED_RESULTS}; true



	TEST_SEED=1535864277 make run; echo $$? >> ${SEED_RESULTS}; true
	TEST_SEED=1535864278 make run; echo $$? >> ${SEED_RESULTS}; true
	TEST_SEED=1535864356 make run; echo $$? >> ${SEED_RESULTS}; true
	TEST_SEED=1535865991 make run; echo $$? >> ${SEED_RESULTS}; true
	TEST_SEED=1535863916 make run; echo $$? >> ${SEED_RESULTS}; true
	TEST_SEED=1535936626 make run; echo $$? >> ${SEED_RESULTS}; true
	cat ${SEED_RESULTS}