
# all must be set before include
Q_ENGINE_REPO=../../../libs/q-engine
IP_LIBRARY_REPO=../../../libs/ip-library
RISCV_BASEBAND_REPO=../../../libs/riscv-baseband
DATAPATH_REPO=../../../libs/datapath
HIGGS_ROOT=../../..

HIGGS_TB_TOP=eth_frame_router

# required to set this before including this file
HIGGS_TEST_DIR=.
# include $(HIGGS_ROOT)/scripts/make_include/makefile_sim_verialtor.mk


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

VER_TOP=--top-module $(HIGGS_TB_TOP) +define+VERILATE  +define+LOAD_VMEM


VER_SOURCES=\
$(IP_LIBRARY_REPO)/lattice_support/gbit_mac/modules/eth_frame_router/hdl/eth_frame_router.sv \
$(IP_LIBRARY_REPO)/fwft_fifos/sc_fifo/hdl/fwft_sc_fifo.v \
$(IP_LIBRARY_REPO)/lattice_support/fifos/pmi_fifo_sc_fwft_v1_0/hdl/pmi_fifo_sc_fwft_v1_0.sv


THIS_FILE=Makefile
CPP_TB_FILES= tb.cpp


# this path is relative to where this makefile is because it gets expanded here and
# passed to verilator
CPP_TB_FILES+= $(wildcard $(VER_CPP_INCLUDE_PATH)/*.cpp)

C_SRC = ./c/src
VER_BINARY = Veth_frame_router



VER_FLAGS = -Wno-PINMISSING --trace  
# VER_FLAGS = --trace  -Wall 

# VER_FLAGS += -pvalue+CBUF_MAX_ETH_FRAME_DEPTH=6 \
# -pvalue+IPV4_PKT_FIFO_PKT_DEPTH=4 \
# -pvalue+ARP_PKT_FIFO_PKT_DEPTH=2



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


all: cleanall verilate compile trun

test: cleanall verilate compile run

vall: compilehex


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
	$(CPP_TB_FILES)


compile:
	make  -j  -C obj_dir/ -f Veth_frame_router.mk Veth_frame_router

update_mif:
	cd ../../../ && $(MAKE) vallcs

clean: clean_verilator_parse_syms
	rm -rf obj_dir
	rm -rf c/build/*
	rm -f wave_dump.vcd wave_dump.vcd.idx
	$(call invoke_c_override_clean)
	rm -f cs10.out cs20.out cs30.out cs10_out.hex cs20_out.hex output.txt t1.txt

cleanall: clean

show:
	gtkwave wave_dump.vcd --fastload dma_out.gtkw &

quick: compilehex compile trun
quickt: compilehex compile run


# these allow for parsing of the vmem names
PARSE_VERILATOR_OBJ_DIR=obj_dir
include $(HIGGS_ROOT)/scripts/make_include/verilator_vmem.mk

# these allow bootloading from this folder
BOOTLOAD_ROOT=override
include $(HIGGS_ROOT)/scripts/make_include/bootloader.mk

.PHONY: check
