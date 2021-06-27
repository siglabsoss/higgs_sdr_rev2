###################
#
#  You must set:
#    HIGGS_ROOT
#    BOOTLOAD_ROOT
#
#  Before including this file
#


# Higgs connection configuration
include $(HIGGS_ROOT)/scripts/make_include/connection_configs.defs
# Bootloader input parameters
BOOTLOADING_PACKET_DELAY = 0.0009
BOOTLOADING_FILETYPE = hex

ifeq ($(UNAME_P),aarch64)
# Sigcarrier / Artick specifc stuff
BOOTLOADING_PACKET_DELAY = 0.001
endif


define bootload_python
python $(HIGGS_ROOT)/scripts/bootload.py $(1) -p $(HIGGS_TX_CMD_PORT)\
-ht $(HIGGS_HOST)\
-ft $(BOOTLOADING_FILETYPE)\
-pd $(BOOTLOADING_PACKET_DELAY)\
-fp $(2) \
$(EXTRA_BL_FLAG)
endef

define bootload_python_force_quiet
python $(HIGGS_ROOT)/scripts/bootload.py $(1) -p $(HIGGS_TX_CMD_PORT)\
-ht $(HIGGS_HOST)\
-ft $(BOOTLOADING_FILETYPE)\
-pd $(BOOTLOADING_PACKET_DELAY)\
-fp $(2) \
-q
endef

.PHONY: bootload_eth bootload_cs20 bootload_cs01 bootload_cs11 bootload_cs21 bootload_cs31 bootload_cs02 bootload_cs12 bootload_cs22 bootload_cs32 bootload_cs23 bootload_cs33

bootload_eth: $(BOOTLOAD_ROOT)/fpgas/grav/eth/build/tmp/eth_top.hex
	$(call bootload_python, eth, $(BOOTLOAD_ROOT)/fpgas/grav/eth/build/tmp/eth_top.hex)
bootload_cs20: $(BOOTLOAD_ROOT)/fpgas/cs/cs20/build/tmp/cs20_top.hex
	$(call bootload_python, cs20, $(BOOTLOAD_ROOT)/fpgas/cs/cs20/build/tmp/cs20_top.hex)
bootload_cs01: $(BOOTLOAD_ROOT)/fpgas/cs/cs01/build/tmp/cs01_top.hex
	$(call bootload_python, cs01, $(BOOTLOAD_ROOT)/fpgas/cs/cs01/build/tmp/cs01_top.hex)
bootload_cs11: $(BOOTLOAD_ROOT)/fpgas/cs/cs11/build/tmp/cs11_top.hex
	$(call bootload_python, cs11, $(BOOTLOAD_ROOT)/fpgas/cs/cs11/build/tmp/cs11_top.hex)
bootload_cs21: $(BOOTLOAD_ROOT)/fpgas/cs/cs21/build/tmp/cs21_top.hex
	$(call bootload_python, cs21, $(BOOTLOAD_ROOT)/fpgas/cs/cs21/build/tmp/cs21_top.hex)
bootload_cs31: $(BOOTLOAD_ROOT)/fpgas/cs/cs31/build/tmp/cs31_top.hex
	$(call bootload_python, cs31, $(BOOTLOAD_ROOT)/fpgas/cs/cs31/build/tmp/cs31_top.hex)
bootload_cs02: $(BOOTLOAD_ROOT)/fpgas/cs/cs02/build/tmp/cs02_top.hex
	$(call bootload_python, cs02, $(BOOTLOAD_ROOT)/fpgas/cs/cs02/build/tmp/cs02_top.hex)
bootload_cs12: $(BOOTLOAD_ROOT)/fpgas/cs/cs12/build/tmp/cs12_top.hex
	$(call bootload_python, cs12, $(BOOTLOAD_ROOT)/fpgas/cs/cs12/build/tmp/cs12_top.hex)
bootload_cs22: $(BOOTLOAD_ROOT)/fpgas/cs/cs22/build/tmp/cs22_top.hex
	$(call bootload_python, cs22, $(BOOTLOAD_ROOT)/fpgas/cs/cs22/build/tmp/cs22_top.hex)
bootload_cs32: $(BOOTLOAD_ROOT)/fpgas/cs/cs32/build/tmp/cs32_top.hex
	$(call bootload_python, cs32, $(BOOTLOAD_ROOT)/fpgas/cs/cs32/build/tmp/cs32_top.hex)
# bootload_cs23: $(BOOTLOAD_ROOT)/fpgas/cs/cs23/build/tmp/cs23_top.hex
# 	$(call bootload_python, cs23, $(BOOTLOAD_ROOT)/fpgas/cs/cs23/build/tmp/cs23_top.hex)
# bootload_cs33: $(BOOTLOAD_ROOT)/fpgas/cs/cs33/build/tmp/cs33_top.hex
# 	$(call bootload_python, cs33, $(BOOTLOAD_ROOT)/fpgas/cs/cs33/build/tmp/cs33_top.hex)

bootload_eth_quiet: $(BOOTLOAD_ROOT)/fpgas/grav/eth/build/tmp/eth_top.hex
	$(call bootload_python_force_quiet, eth, $(BOOTLOAD_ROOT)/fpgas/grav/eth/build/tmp/eth_top.hex)