###########################
#
#   Top Level Higgs Makefile
#
# This has aliases to all of the "make cs20 cs30" (which invoke a sub-make)
# See:
#   scripts/make_include/higgs_core.defs
#   scripts/make_include/user_settings.defs
#   scripts/make_include/makefile_fpga.defs
#


# must come before higgs_core.defs
# because this_fpga doesn't define any targets, we can put this "all" target after it
all: help
.PHONY: all help buildstatus list program_launch_message

# Configurate DAC input parameters
CONFIG_DAC_DELAY = 0.2
# Configurate ADC input parameters
# Channel A: 0 <= DSA_ATTENUATION <= 30 (Steps of 2)
# Channel B: 0 <= DSA_ATTENUATION <= 31 (Steps of 1)
DSA_ATTENUATION = 30
VGA_ATTENUATION = 0
RX_CHANNEL = B
# Transmit signal input parameters
TX_CMD_DELAY = 0.005
TX_DATA_DELAY = 0.005
TX_PACKET_SIZE = 256
# Receive signal input parameters
RX_BUFFER_SIZE = 1472
RX_SOC_TIMEOUT = 1.0
RX_PACKET_COUNT = 100
SAVE_DATA_FILE_PATH = receive_data.csv
# these come first because included scripts use them
CS_FPGAS := cs00 cs01 cs02 cs03 cs10 cs11 cs12 cs13 cs20 cs21 cs22 cs23 cs30 cs31 cs32 cs33 cscfg
GRAV_FPGAS := adc cfg dac eth
ALL_FPGAS := $(CS_FPGAS) $(GRAV_FPGAS)

GOLD_ITERATIONS ?= 20

BUILD_ITER ?= 1


# in order to include this file, we must set HIGGS_ROOT
HIGGS_ROOT = .
include scripts/make_include/higgs_core.defs
include scripts/make_include/connection_configs.defs
include scripts/make_include/jenkins_shared.defs
include scripts/make_include/jenkins_root.defs
include scripts/make_include/ascii_docs.mk

.PHONY: jenkins_automated_build jenkins_final_target

# looking for (jenkins_automated_build, jenkins_final_target)  ?? see this file
include jenkins.defs

# looking for enabledfpga enabledfpgas ? see this file
include enabledfpgas.defs

# this is how we can grab official bitfiles
# use optional include (see inside the file for reason)
-include scripts/make_include/sigdata_fetch.defs

# this is how we flash from the command line
include scripts/make_include/flashing_jtag.defs

BOOTLOAD_ROOT=.
include scripts/make_include/bootloader.mk
include scripts/make_include/test_ring.mk
include scripts/make_include/test_bootload.mk
include scripts/make_include/adc_commands.mk
include scripts/make_include/dac_commands.mk

help:
	@$(PRINTF) "\n Welcome to Higgs Makefile!\n\nYou can:\n  make list    - Show all targets in this Makefile\n  make buildstatus\n  make clean\n  make help    - This message\n\n\n In general this should build fpgas, see status of previous builds, save build outputs, restore build outputs\n"

.deps_ok:
	$(TOUCH) .deps_ok


.PHONY: checksystem check_system checklattice check_lattice check_activehdl_path check_lattice_path

# Legacy targets that were made for windows.  Leaving these here incase I forgot a reference somewhere
checksystem:
check_system:
scripts/checksystem.sh:
checklattice:
check_lattice:

###############
# 
# complicated setup but it goes like this
# 
# line 1:
#    if bin is not found, error code 1, which flips into 0, which causes error to print.
#    if bin is found, error code 0, which flips to a 1, which prints nothing.
#       above exit codes are sqashed by ;true
# line 2:
#    if bin is not found, error code 0
#    if bin is found< error code 1  (note that we use 2>&1 to suppress all output, we just want exit code)
#

check_activehdl_path:
	@ ! which vsimsa && $(ECHO) "" && $(ECHO) "" && $(ECHO) "  vsimsa not found, is your PATH setup correctly?"; true
	@ which vsimsa > /dev/null 2>&1

check_lattice_path:
	@ ! which pnmainc && $(ECHO) "" && $(ECHO) "" && ! $(ECHO) "  pnmainc not found, is your PATH setup correctly?"; true
	@ which pnmainc > /dev/null 2>&1


# Sugar targets for building.  jenkins uses allfpga

.PHONY: allcs allgrav allfpga adc cfg dac eth cs00 cs01 cs02 cs03 cs10 cs11 cs12 cs13 cs20 cs21 cs22 cs23 cs30 cs31 cs32 cs33 cscfg



allcs: $(CS_FPGAS)

allgrav: $(GRAV_FPGAS)

allfpga: allcs allgrav

allfpgas: allfpga


# looking for enabledfpg enabledfpgs see the enabledfpgas.defs file
# enabledfpga: cs00 cs01


cs00:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs01:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs02:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs03:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs10:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs11:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs12:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs13:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs20:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs21:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs22:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs23:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs30:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs31:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs32:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cs33:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)
cscfg:
	cd fpgas/cs/$(@)/build && make itr=$(BUILD_ITER)

################ GRAV ################
adc:
	cd fpgas/grav/$(@)/build && make itr=$(BUILD_ITER)
cfg:
	cd fpgas/grav/$(@)/build && make itr=$(BUILD_ITER)
dac:
	cd fpgas/grav/$(@)/build && make itr=$(BUILD_ITER)
eth:
	cd fpgas/grav/$(@)/build && make itr=$(BUILD_ITER)


cs00gold:
	cd fpgas/cs/cs00/build && make itr=$(GOLD_ITERATIONS)
cs01gold:
	cd fpgas/cs/cs01/build && make itr=$(GOLD_ITERATIONS)
cs02gold:
	cd fpgas/cs/cs02/build && make itr=$(GOLD_ITERATIONS)
cs03gold:
	cd fpgas/cs/cs03/build && make itr=$(GOLD_ITERATIONS)
cs10gold:
	cd fpgas/cs/cs10/build && make itr=$(GOLD_ITERATIONS)
cs11gold:
	cd fpgas/cs/cs11/build && make itr=$(GOLD_ITERATIONS)
cs12gold:
	cd fpgas/cs/cs12/build && make itr=$(GOLD_ITERATIONS)
cs13gold:
	cd fpgas/cs/cs13/build && make itr=$(GOLD_ITERATIONS)
cs20gold:
	cd fpgas/cs/cs20/build && make itr=$(GOLD_ITERATIONS)
cs21gold:
	cd fpgas/cs/cs21/build && make itr=$(GOLD_ITERATIONS)
cs22gold:
	cd fpgas/cs/cs22/build && make itr=$(GOLD_ITERATIONS)
cs23gold:
	cd fpgas/cs/cs23/build && make itr=$(GOLD_ITERATIONS)
cs30gold:
	cd fpgas/cs/cs30/build && make itr=$(GOLD_ITERATIONS)
cs31gold:
	cd fpgas/cs/cs31/build && make itr=$(GOLD_ITERATIONS)
cs32gold:
	cd fpgas/cs/cs32/build && make itr=$(GOLD_ITERATIONS)
cs33gold:
	cd fpgas/cs/cs33/build && make itr=$(GOLD_ITERATIONS)
cscfggold:
	cd fpgas/cs/cscfg/build && make itr=$(GOLD_ITERATIONS)

################ GRAV ################
adcgold:
	cd fpgas/grav/adc/build && make itr=$(GOLD_ITERATIONS)
cfggold:
	cd fpgas/grav/cfg/build && make itr=$(GOLD_ITERATIONS)
dacgold:
	cd fpgas/grav/dac/build && make itr=$(GOLD_ITERATIONS)
ethgold:
	cd fpgas/grav/eth/build && make itr=$(GOLD_ITERATIONS)


rallfpga: rcs00 rcs01 rcs02 rcs03 rcs10 rcs11 rcs12 rcs13 rcs20 rcs21 rcs22 rcs23 rcs30 rcs31 rcs32 rcs33 rcscfg radc rcfg rdac reth

rallfpgas: rallfpga

rcs00:
	cd fpgas/cs/cs00/build && $(MAKE) jenkins_build
rcs01:
	cd fpgas/cs/cs01/build && $(MAKE) jenkins_build
rcs02:
	cd fpgas/cs/cs02/build && $(MAKE) jenkins_build
rcs03:
	cd fpgas/cs/cs03/build && $(MAKE) jenkins_build
rcs10:
	cd fpgas/cs/cs10/build && $(MAKE) jenkins_build
rcs11:
	cd fpgas/cs/cs11/build && $(MAKE) jenkins_build
rcs12:
	cd fpgas/cs/cs12/build && $(MAKE) jenkins_build
rcs13:
	cd fpgas/cs/cs13/build && $(MAKE) jenkins_build
rcs20:
	cd fpgas/cs/cs20/build && $(MAKE) jenkins_build
rcs21:
	cd fpgas/cs/cs21/build && $(MAKE) jenkins_build
rcs22:
	cd fpgas/cs/cs22/build && $(MAKE) jenkins_build
rcs23:
	cd fpgas/cs/cs23/build && $(MAKE) jenkins_build
rcs30:
	cd fpgas/cs/cs30/build && $(MAKE) jenkins_build
rcs31:
	cd fpgas/cs/cs31/build && $(MAKE) jenkins_build
rcs32:
	cd fpgas/cs/cs32/build && $(MAKE) jenkins_build
rcs33:
	cd fpgas/cs/cs33/build && $(MAKE) jenkins_build
rcscfg:
	cd fpgas/cs/cscfg/build && $(MAKE) jenkins_build
radc:
	cd fpgas/grav/adc/build && $(MAKE) jenkins_build
rcfg:
	cd fpgas/grav/cfg/build && $(MAKE) jenkins_build
rdac:
	cd fpgas/grav/dac/build && $(MAKE) jenkins_build
reth:
	cd fpgas/grav/eth/build && $(MAKE) jenkins_build


################ all RDL for subdirs ################

.PHONY: allrdl rdlall rdl cleanrdl

rdl: allrdl

rdlall: allrdl

allrdl:
	cd fpgas/cs/cs01/build && $(MAKE) rdl
	cd fpgas/cs/cs02/build && $(MAKE) rdl
	cd fpgas/cs/cs03/build && $(MAKE) rdl
	cd fpgas/cs/cs11/build && $(MAKE) rdl
	cd fpgas/cs/cs12/build && $(MAKE) rdl
	cd fpgas/cs/cs13/build && $(MAKE) rdl
	cd fpgas/cs/cs20/build && $(MAKE) rdl
	cd fpgas/cs/cs21/build && $(MAKE) rdl
	cd fpgas/cs/cs22/build && $(MAKE) rdl
	cd fpgas/cs/cs23/build && $(MAKE) rdl
	cd fpgas/cs/cs31/build && $(MAKE) rdl
	cd fpgas/cs/cs32/build && $(MAKE) rdl
	cd fpgas/cs/cs33/build && $(MAKE) rdl
	cd fpgas/cs/cscfg/build && $(MAKE) rdl
	cd fpgas/grav/adc/build && $(MAKE) rdl
	cd fpgas/grav/cfg/build && $(MAKE) rdl
	cd fpgas/grav/dac/build && $(MAKE) rdl
	cd fpgas/grav/eth/build && $(MAKE) rdl

cleanrdl:
	cd fpgas/cs/cs00/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs01/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs02/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs03/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs10/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs11/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs12/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs13/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs20/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs21/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs22/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs23/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs30/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs31/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs32/build && $(MAKE) cleanrdl
	cd fpgas/cs/cs33/build && $(MAKE) cleanrdl
	cd fpgas/cs/cscfg/build && $(MAKE) cleanrdl
	cd fpgas/grav/adc/build && $(MAKE) cleanrdl
	cd fpgas/grav/cfg/build && $(MAKE) cleanrdl
	cd fpgas/grav/dac/build && $(MAKE) cleanrdl
	cd fpgas/grav/eth/build && $(MAKE) cleanrdl

.PHONY: vallfpga vall vallgrav vallcs veth vcs00 vcs01 vcs10 vcs11 vcs20 vcs21 vcs30 vcs31
.PHONY: bootload_eth bootload_cs20 bootload_cs10 bootload_cs00 bootload_cs01 bootload_cs11 bootload_cs21 bootload_cs31 bootload_cs30

vallfpga: vall
vall: vallgrav vallcs

# right now there is only one FPGA with a q-engine on graviton
vallgrav: veth

vallcs: vcs01 vcs02 vcs11 vcs20 vcs21 vcs22 vcs31 vcs32 vcs12


veth:
	make -C fpgas/grav/eth/c
vcs01:
	make -C fpgas/cs/cs01/c
vcs02:
	make -C fpgas/cs/cs02/c
vcs11:
	make -C fpgas/cs/cs11/c
vcs12:
	make -C fpgas/cs/cs12/c
vcs20:
	make -C fpgas/cs/cs20/c
vcs21:
	make -C fpgas/cs/cs21/c
vcs22:
	make -C fpgas/cs/cs22/c
vcs31:
	make -C fpgas/cs/cs31/c
vcs32:
	make -C fpgas/cs/cs32/c

vclean:
	cd fpgas/grav/eth/c && $(MAKE) clean
	cd fpgas/cs/cs00/c && $(MAKE) clean
	cd fpgas/cs/cs01/c && $(MAKE) clean
	cd fpgas/cs/cs10/c && $(MAKE) clean
	cd fpgas/cs/cs11/c && $(MAKE) clean
	cd fpgas/cs/cs20/c && $(MAKE) clean
	cd fpgas/cs/cs21/c && $(MAKE) clean
	cd fpgas/cs/cs30/c && $(MAKE) clean
	cd fpgas/cs/cs31/c && $(MAKE) clean

config_dac:
	python scripts/dac_module.py -dm -p $(HIGGS_TX_CMD_PORT)\
	                                 -ht $(HIGGS_HOST)\
	                                 -pd $(CONFIG_DAC_DELAY)

tx_channel:
	python scripts/dac_module.py -tx $(tx)\
	                             -p $(HIGGS_TX_CMD_PORT)\
	                             -ht $(HIGGS_HOST)\
	                             -pd $(CONFIG_DAC_DELAY)

disable_tx:
	python scripts/dac_module.py -dis \
	                             -p $(HIGGS_TX_CMD_PORT)\
	                             -ht $(HIGGS_HOST)\
	                             -pd $(CONFIG_DAC_DELAY)

transmit: 
	$(MAKE) datafile
	python scripts/transmit_data.py -tx -ht $(HIGGS_HOST)\
	                                    -tcp $(HIGGS_TX_CMD_PORT)\
	                                    -tdp $(HIGGS_TX_DATA_PORT)\
	                                    -dd $(TX_DATA_DELAY)\
	                                    -cd $(TX_CMD_DELAY)\
	                                    -ps $(TX_PACKET_SIZE)

capture_samples:
	python scripts/receive_data.py -rp $(packet_count)\
	                               -ht $(HIGGS_HOST)\
	                               -oht $(OUR_HOST)\
	                               -rdp $(HIGGS_RX_DATA_PORT)\
	                               -s $(SAVE_DATA_FILE_PATH)\
	                               -bs $(RX_BUFFER_SIZE)\
	                               -t $(RX_SOC_TIMEOUT)

datafile:
	cd libs/datapath && ./bin/gen-symbol.js

.PHONY: test_eth

test_eth:
	python scripts/test_eth.py -p $(HIGGS_TX_CMD_PORT)\
	                           -rp $(HIGGS_RX_CMD_PORT)\
	                           -ht $(HIGGS_HOST)\
	                           -pd $(BOOTLOADING_PACKET_DELAY)

test_dump:
	python scripts/dump_ringbus.py -p $(HIGGS_PORT)\
	                               -rp $(HIGGS_RX_CMD_PORT)\
	                               -ht $(HIGGS_HOST)\
	                               -pd $(BOOTLOADING_PACKET_DELAY)

.PHONY: cleansystem clean cleanall vclean

# cleans all buildstem stuff
# DOES NOT CLEAN FPGAS
cleansystem:
	@echo "Cleaning build system, FPGA bitfiles were left alone"
	$(RM) .deps_ok
	$(RM) scripts/buildstatus.sh
	$(RM) scripts/checksystem.sh

# for now clean = cleansystem
clean: cleansystem cleanxcf vclean


cleanall: vclean
	@echo "Cleans all fpga build related stuff"
	cd fpgas/cs/cs01/build && $(MAKE) clean
	cd fpgas/cs/cs02/build && $(MAKE) clean
	cd fpgas/cs/cs03/build && $(MAKE) clean
	cd fpgas/cs/cs11/build && $(MAKE) clean
	cd fpgas/cs/cs12/build && $(MAKE) clean
	cd fpgas/cs/cs13/build && $(MAKE) clean
	cd fpgas/cs/cs20/build && $(MAKE) clean
	cd fpgas/cs/cs21/build && $(MAKE) clean
	cd fpgas/cs/cs22/build && $(MAKE) clean
	cd fpgas/cs/cs23/build && $(MAKE) clean
	cd fpgas/cs/cs31/build && $(MAKE) clean
	cd fpgas/cs/cs32/build && $(MAKE) clean
	cd fpgas/cs/cs33/build && $(MAKE) clean
	cd fpgas/cs/cscfg/build && $(MAKE) clean
	cd fpgas/grav/adc/build && $(MAKE) clean
	cd fpgas/grav/cfg/build && $(MAKE) clean
	cd fpgas/grav/dac/build && $(MAKE) clean
	cd fpgas/grav/eth/build && $(MAKE) clean





.PHONY: terminate_builds idle_priority kill_cableserver

# looking for these targets? see the jekins.defs file
# jenkins_automated_build:
# jenkins_final_target:

terminate_builds:
	pgrep -x diamond | xargs kill; true
	pgrep -x pnmainc | xargs kill; true
	ps -efW | grep par | tr -s ' ' | cut -d' ' -f3 | xargs -I '{}' taskkill /F /PID {}
	ps -efW | grep m_gen_lattice | tr -s ' ' | cut -d' ' -f3 | xargs -I '{}' taskkill /F /PID {}

# run this at your desk to set the priority of the build to low
# this would allow your explorer.exe to have a quicker response time
idle_priority:
	cmd /c "scripts\\set_windows_build_process_priority.bat"

kill_cableserver:
	cmd /c "scripts\\kill_cableserver_windows.bat"
