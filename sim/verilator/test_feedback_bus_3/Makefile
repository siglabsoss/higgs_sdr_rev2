
# all must be set before include
Q_ENGINE_REPO=../../../libs/q-engine
IP_LIBRARY_REPO=../../../libs/ip-library
RISCV_BASEBAND_REPO=../../../libs/riscv-baseband
DATAPATH_REPO=../../../libs/datapath
HIGGS_ROOT=../../..
include $(Q_ENGINE_REPO)/scripts/make_include/verilog_paths.mk
include $(HIGGS_ROOT)/scripts/make_include/verilog_paths.mk

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
# OVERRIDE_CS20_C=1
# OVERRIDE_CS10_C=1
# OVERRIDE_CS00_C=1
# OVERRIDE_CS01_C=1
# OVERRIDE_CS11_C=1
# OVERRIDE_CS21_C=1
# OVERRIDE_CS31_C=1
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

# THESE DONT WORK
# These are only respected when under verilator.  If OVERRIDE above is not set this
# setting must match what the fpga_top.sv fpga uses for QENGINE_LITE
# CS20_QENGINE_LITE=1
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
CS20_NO_RISCV=1
CS10_NO_RISCV=1
CS00_NO_RISCV=1
CS01_NO_RISCV=1
CS11_NO_RISCV=1
CS21_NO_RISCV=1
CS31_NO_RISCV=1
# CS30_NO_RISCV=1



# required to set this before including this file
HIGGS_TEST_DIR=.
include $(HIGGS_ROOT)/scripts/make_include/makefile_sim_verialtor.mk


# path should be relative to where this makefile is (repo root)
VER_CPP_INCLUDE_PATH=../inc

VER_HDL_PATH=../inc

VER_INCLUDE_DIRS=\
-I../../hdl  \
-I. \
-I$(VER_CPP_INCLUDE_PATH) \
-I$(HIGGS_ROOT)/fpgas/packages \
-I$(HIGGS_ROOT)/fpgas/common/modules/eb2a \
-I$(IP_LIBRARY_REPO)/lattice_support/gbit_mac/packages

VER_TOP=--top-module $(HIGGS_TB_TOP) +define+VERILATE +define+LOAD_VMEM


VER_SOURCES= $(HIGGS_TB_ALL_VERILOG) $(Q_ENGINE_ALL_VERILOG)


THIS_FILE=Makefile
CPP_TB_FILES= tb.cpp


# this path is relative to where this makefile is because it gets expanded here and
# passed to verilator
CPP_TB_FILES+= $(wildcard $(VER_CPP_INCLUDE_PATH)/*.cpp)

C_SRC = ./c/src
VER_BINARY = Vtb_higgs_top



VER_FLAGS = -Wno-PINMISSING --trace  
# VER_FLAGS = --trace  -Wall 


VER_FLAGS += -DVERILATE_DEF


ADDCFLAGS += -CFLAGS -pthread
ADDCFLAGS += -CFLAGS -g
ADDCFLAGS += -CFLAGS -std=c++11
ADDCFLAGS += -LDFLAGS -pthread

# path is relative to obj_dir (So some variables will have a ../ prepended)
ADDCFLAGS += -CFLAGS -I../$(VER_CPP_INCLUDE_PATH)
ADDCFLAGS += -CFLAGS -I../$(RISCV_BASEBAND_REPO)/verilator/inc
# it may seem weird but we can include the same file from both the TB and from the Riscv itself (this include path is for the TB's sake)
ADDCFLAGS += -CFLAGS -I../$(RISCV_BASEBAND_REPO)/c/inc
ADDCFLAGS += -CFLAGS -I/usr/local/share/verilator/include

.PHONY: all compilehex run_only run verilate compile clean cleanall show trun vall


all: cleanall compilehex verilate verilator_parse_syms compile trun

test: cleanall compilehex verilate verilator_parse_syms compile run

vall: compilehex

# please note that this will build all default .hex files as well as
# particular overrided ones.  The default compiles are wasted and the output is not used
# if they are overridden
compilehex:
	# cd $(HIGGS_ROOT) && make -j22 vallfpga
	cd $(HIGGS_ROOT) && make vallfpga
	$(call invoke_c_override_submake)
	

run_only:
	./obj_dir/$(VER_BINARY)

run: compile
	./obj_dir/$(VER_BINARY)

trun: compile
	./obj_dir/$(VER_BINARY) +trace

verilate:
	verilator $(VER_TOP) \
	$(VER_FLAGS) \
	$(VER_INCLUDE_DIRS) \
	-cc \
	$(VER_SOURCES) \
	-O3 \
	${ADDCFLAGS} \
	--gdbbt \
	${VERILATOR_ARGS} \
	-Wno-UNOPTFLAT -Wno-WIDTH \
	--x-assign unique \
	--exe \
	$(CPP_TB_FILES) \
	$(VERILATOR_C_OVERRIDE_DEFINES) \
	$(VERILATOR_TB_INCLUDE_DEFINES)


compile:
	make  -j  -C obj_dir/ -f Vtb_higgs_top.mk Vtb_higgs_top

update_mif:
	cd ../../../ && $(MAKE) vallcs

clean: clean_verilator_parse_syms
	rm -rf obj_dir
	rm -rf c/build/*
	rm -f wave_dump.vcd wave_dump.vcd.idx
	$(call invoke_c_override_clean)
	rm -f cs10.out cs20.out cs30.out cs10_out.hex cs20_out.hex cs30_out.hex output.txt t1.txt

cleanall: clean

show:
	gtkwave wave_dump.vcd --fastload dma_out.gtkw &

quick: compilehex verilator_parse_syms compile trun
quickt: compilehex verilator_parse_syms compile run


# these allow for parsing of the vmem names
PARSE_VERILATOR_OBJ_DIR=obj_dir
include $(HIGGS_ROOT)/scripts/make_include/verilator_vmem.mk

# these allow bootloading from this folder
BOOTLOAD_ROOT=override
include $(HIGGS_ROOT)/scripts/make_include/bootloader.mk

.PHONY: check

check:
	diff cs20_expected.hex cs20_out.hex

bootload:
	make vall
	make bootload_cs30
	make bootload_cs31
	make bootload_cs21
	make bootload_cs11
	make bootload_cs01
	make bootload_cs00


# cp ../../../fpgas/cs/cs00/c/src/main.c override/fpgas/cs/cs00/c/src/main.c
# cp ../../../fpgas/cs/cs01/c/src/main.c override/fpgas/cs/cs01/c/src/main.c
# cp ../../../fpgas/cs/cs02/c/src/main.c override/fpgas/cs/cs02/c/src/main.c
# cp ../../../fpgas/cs/cs03/c/src/main.c override/fpgas/cs/cs03/c/src/main.c
# cp ../../../fpgas/cs/cs10/c/src/main.c override/fpgas/cs/cs10/c/src/main.c
# cp ../../../fpgas/cs/cs11/c/src/main.c override/fpgas/cs/cs11/c/src/main.c
# cp ../../../fpgas/cs/cs12/c/src/main.c override/fpgas/cs/cs12/c/src/main.c
# cp ../../../fpgas/cs/cs13/c/src/main.c override/fpgas/cs/cs13/c/src/main.c
# cp ../../../fpgas/cs/cs20/c/src/main.c override/fpgas/cs/cs20/c/src/main.c
# cp ../../../fpgas/cs/cs21/c/src/main.c override/fpgas/cs/cs21/c/src/main.c
# cp ../../../fpgas/cs/cs22/c/src/main.c override/fpgas/cs/cs22/c/src/main.c
# cp ../../../fpgas/cs/cs23/c/src/main.c override/fpgas/cs/cs23/c/src/main.c
# cp ../../../fpgas/cs/cs30/c/src/main.c override/fpgas/cs/cs30/c/src/main.c
# cp ../../../fpgas/cs/cs31/c/src/main.c override/fpgas/cs/cs31/c/src/main.c
# cp ../../../fpgas/cs/cs32/c/src/main.c override/fpgas/cs/cs32/c/src/main.c
# cp ../../../fpgas/cs/cs33/c/src/main.c override/fpgas/cs/cs33/c/src/main.c
# cp ../../../fpgas/grav/adc/c/src/main.c override/fpgas/grav/adc/c/src/main.c
# cp ../../../fpgas/grav/cfg/c/src/main.c override/fpgas/grav/cfg/c/src/main.c
# cp ../../../fpgas/grav/dac/c/src/main.c override/fpgas/grav/dac/c/src/main.c
# cp ../../../fpgas/grav/eth/c/src/main.c override/fpgas/grav/eth/c/src/main.c


