##########################
#
#  This Makefile produces the .hex for each FPGA
#
#  Please note that this Makefile is written correctly to only build with changes
#  to source.  Additional dependency files that trigger rebuilds are added to $(SENSATIVE_SRCS)
#

# normally outputs are left in build/
# however we copy them to ../build/tmp and then do further processing there
# this means the files will be copied however that shouldn't matter too much


# Automatically calculate variable FPGA_NAME
FPGA_NAME=$(shell pwd | tr '/' '\n' | tail -2 | head -1)

# Automatically calculate folder we are in
COMPILE_FOLDER_NAME=$(shell pwd | tr '/' '\n' | tail -6 | head -1)/$(FPGA_NAME)

PROJ_NAME=$(FPGA_NAME)_top

# our include folder
INC_FOLDER=$(RISCV_BASEBAND_REPO)/c/inc


RISCV_PATH = $(RISCV)
RISCV_PATH ?= /opt/riscv/
# before we were using:
# -march=rv32ia
# now with hardware multiplier we are using:
# -march=rv32ima

# -fdata-sections -ffunction-sections
# allows entire functions to be optimized out and save imem/dmem
# see https://stackoverflow.com/questions/6687630/how-to-remove-unused-c-c-symbols-with-gcc-and-ld
# we could add  -Wpedantic
# -Wduplicated-branches  -Wpadded
# -funroll-loops
CFLAGS += -march=rv32ima   -mabi=ilp32 -O3 -fdata-sections -ffunction-sections \
-Wall -Wextra \
-Wno-missing-prototypes -Wno-unused-function \
-Werror=return-type -Werror=array-bounds -Wshadow -Werror=shift-count-overflow \
-Werror=overflow -Wunused-but-set-variable -Wcomment -Wvolatile-register-var \
-Wunused-variable -Wduplicated-cond -Wnull-dereference \
-Wdouble-promotion -Wstack-usage=256 -Wbad-function-cast -Wdangling-else \
-Waddress -Wstrict-prototypes -Wnested-externs -Winline \
-Wint-in-bool-context -Wvla -Wunsuffixed-float-constants \
-Wold-style-definition -Wold-style-declaration -Winit-self \
-Wpointer-arith -Wcast-qual -Werror=incompatible-pointer-types \
-Werror=strict-prototypes -Werror=uninitialized \
-Wlogical-op -Werror=logical-op -Werror=null-dereference -Wjump-misses-init \
-Werror=jump-misses-init -Werror=sequence-point \
-Werror=missing-braces -Werror=write-strings -Werror=address -Werror=array-bounds \
-Werror=char-subscripts -Werror=enum-compare -Werror=implicit-int \
-Werror=empty-body -Werror=main -Werror=nonnull -Werror=parentheses \
-Werror=pointer-sign -Werror=ignored-qualifiers \
-Werror=missing-parameter-type -Werror=unused-value \
-Wtrampolines


RISCV_NAME = riscv32-unknown-elf
RISCV_OBJCOPY = $(RISCV_PATH)/bin/$(RISCV_NAME)-objcopy
RISCV_OBJDUMP = $(RISCV_PATH)/bin/$(RISCV_NAME)-objdump
RISCV_CLIB=$(RISCV_PATH)$(RISCV_NAME)/lib/
RISCV_CC=$(RISCV_PATH)/bin/$(RISCV_NAME)-gcc

# define LDSCRIPT from outside this when invoking make to point to a custom ld_script.
# NOTE: do not put the full path, just put the file name which must be under libs/riscv-baseband/c/inc
ifndef LDSCRIPT
LDSCRIPT=$(INC_FOLDER)/ld_standard
else
LDSCRIPT:=$(INC_FOLDER)/$(LDSCRIPT)
endif




SRCS = 	$(wildcard src/*.c) \
        $(wildcard src/*.S)

SRCS += $(wildcard $(INC_FOLDER)/*.c) \
        $(wildcard $(INC_FOLDER)/*.S)

# add any .h files or other files that should trigger rebuilds but are not directly
# passed to gcc
SENSATIVE_SRCS = $(wildcard $(INC_FOLDER)/*.h) $(wildcard src/*.h) $(LDSCRIPT) $(wildcard $(IP_LIBRARY_REPO)/../datapath/symbol.h)

# bjm 20180608
# adding -flto to both compile and link flags
# without this I was not able to inline function between different .c files
# see https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#index-flto
# see https://gcc.gnu.org/wiki/LinkTimeOptimization

# For description of -Wl see
# https://stackoverflow.com/questions/6562403/i-dont-understand-wl-rpath-wl
# removing  -flto    for now
CFLAGS += -static -g -I$(INC_FOLDER) -I$(IP_LIBRARY_REPO)/../datapath -Winline
# -Wl,--gc-sections allows entire functions to be optimized out
# removing  -Wl,-flto    for now
LDFLAGS += -e_start -T $(LDSCRIPT) -nostartfiles  -Wl,-Map,$(OBJDIR)/$(PROJ_NAME).map -Wl,--print-memory-usage -Wl,--gc-sections
OBJDIR = build/build/build/build/build/build/build/build/build
OBJDIR2 = build
OBJS := $(SRCS)
OBJS := $(OBJS:.c=.o)
OBJS := $(OBJS:.cpp=.o)
OBJS := $(OBJS:.S=.o)
OBJS := $(addprefix $(OBJDIR)/,$(OBJS))





ifeq ($(FPGA_NAME),cs01)
ifneq ("${CS01_EXTRA_IMEM}","")
LDFLAGS += -Wl,'--defsym=EXTRA_IMEM=${CS01_EXTRA_IMEM}'
endif
endif

ifeq ($(FPGA_NAME),cs11)
ifneq ("${CS11_EXTRA_IMEM}","")
LDFLAGS += -Wl,'--defsym=EXTRA_IMEM=${CS11_EXTRA_IMEM}'
endif
endif

ifeq ($(FPGA_NAME),cs20)
ifneq ("${CS20_EXTRA_IMEM}","")
LDFLAGS += -Wl,'--defsym=EXTRA_IMEM=${CS20_EXTRA_IMEM}'
endif
endif

ifeq ($(FPGA_NAME),cs21)
ifneq ("${CS21_EXTRA_IMEM}","")
LDFLAGS += -Wl,'--defsym=EXTRA_IMEM=${CS21_EXTRA_IMEM}'
endif
endif

ifeq ($(FPGA_NAME),cs31)
ifneq ("${CS31_EXTRA_IMEM}","")
LDFLAGS += -Wl,'--defsym=EXTRA_IMEM=${CS31_EXTRA_IMEM}'
endif
endif

ifeq ($(FPGA_NAME),eth)
ifneq ("${ETH_EXTRA_IMEM}","")
LDFLAGS += -Wl,'--defsym=EXTRA_IMEM=${ETH_EXTRA_IMEM}'
endif
endif

ifeq ($(FPGA_NAME),cs02)
ifneq ("${CS02_EXTRA_IMEM}","")
LDFLAGS += -Wl,'--defsym=EXTRA_IMEM=${CS02_EXTRA_IMEM}'
endif
endif

ifeq ($(FPGA_NAME),cs12)
ifneq ("${CS12_EXTRA_IMEM}","")
LDFLAGS += -Wl,'--defsym=EXTRA_IMEM=${CS12_EXTRA_IMEM}'
endif
endif

ifeq ($(FPGA_NAME),cs22)
ifneq ("${CS22_EXTRA_IMEM}","")
LDFLAGS += -Wl,'--defsym=EXTRA_IMEM=${CS22_EXTRA_IMEM}'
endif
endif

ifeq ($(FPGA_NAME),cs32)
ifneq ("${CS32_EXTRA_IMEM}","")
LDFLAGS += -Wl,'--defsym=EXTRA_IMEM=${CS32_EXTRA_IMEM}'
endif
endif






ifeq ($(FPGA_NAME),cs01)
CFLAGS += -DOUR_RING_ENUM=3
endif

ifeq ($(FPGA_NAME),cs11)
CFLAGS += -DOUR_RING_ENUM=2
endif

ifeq ($(FPGA_NAME),cs20)
CFLAGS += -DOUR_RING_ENUM=10
endif

ifeq ($(FPGA_NAME),cs21)
CFLAGS += -DOUR_RING_ENUM=9
endif

ifeq ($(FPGA_NAME),cs31)
CFLAGS += -DOUR_RING_ENUM=8
endif

ifeq ($(FPGA_NAME),eth)
CFLAGS += -DOUR_RING_ENUM=1
endif

ifeq ($(FPGA_NAME),cs02)
CFLAGS += -DOUR_RING_ENUM=4
endif

ifeq ($(FPGA_NAME),cs12)
CFLAGS += -DOUR_RING_ENUM=5
endif

ifeq ($(FPGA_NAME),cs22)
CFLAGS += -DOUR_RING_ENUM=6
endif

# ifeq ($(FPGA_NAME),cs23)
# CFLAGS += -DOUR_RING_ENUM=7
# endif

# ifeq ($(FPGA_NAME),cs33)
# CFLAGS += -DOUR_RING_ENUM=8
# endif

ifeq ($(FPGA_NAME),cs32)
CFLAGS += -DOUR_RING_ENUM=7
endif

CFLAGS += -DCOMPILE_FOLDER=\"$(COMPILE_FOLDER_NAME)\"

# final output is in the tmp folder
TMP_OUTPUT=../build/tmp


C_HEX_PATH=$(TMP_OUTPUT)/$(PROJ_NAME).hex

MIF_OUT_TARGETS=$(TMP_OUTPUT)/scalar0.mif $(TMP_OUTPUT)/scalar1.mif $(TMP_OUTPUT)/scalar2.mif $(TMP_OUTPUT)/scalar3.mif

VMEM_MIF_OUT_TARGETS=$(TMP_OUTPUT)/vmem0.mif $(TMP_OUTPUT)/vmem1.mif $(TMP_OUTPUT)/vmem2.mif $(TMP_OUTPUT)/vmem3.mif $(TMP_OUTPUT)/vmem4.mif $(TMP_OUTPUT)/vmem5.mif $(TMP_OUTPUT)/vmem6.mif $(TMP_OUTPUT)/vmem7.mif $(TMP_OUTPUT)/vmem8.mif $(TMP_OUTPUT)/vmem9.mif $(TMP_OUTPUT)/vmem10.mif $(TMP_OUTPUT)/vmem11.mif $(TMP_OUTPUT)/vmem12.mif $(TMP_OUTPUT)/vmem13.mif $(TMP_OUTPUT)/vmem14.mif $(TMP_OUTPUT)/vmem15.mif 

all: sensative_clean elf_hex_asm $(TMP_OUTPUT)/scalar0.mif


$(MIF_OUT_TARGETS): $(C_HEX_PATH)
	@mkdir -p $(TMP_OUTPUT)
	@python $(RISCV_BASEBAND_REPO)/scripts/hex2mif.py -i $(C_HEX_PATH) -o0 $(TMP_OUTPUT)/scalar0.mif -o1 $(TMP_OUTPUT)/scalar1.mif -o2 $(TMP_OUTPUT)/scalar2.mif -o3 $(TMP_OUTPUT)/scalar3.mif
	@python $(RISCV_BASEBAND_REPO)/scripts/hex2mif_vmem.py -i $(C_HEX_PATH) \
	-o0 $(TMP_OUTPUT)/vmem0.mif \
	-o1 $(TMP_OUTPUT)/vmem1.mif \
	-o2 $(TMP_OUTPUT)/vmem2.mif \
	-o3 $(TMP_OUTPUT)/vmem3.mif \
	-o4 $(TMP_OUTPUT)/vmem4.mif \
	-o5 $(TMP_OUTPUT)/vmem5.mif \
	-o6 $(TMP_OUTPUT)/vmem6.mif \
	-o7 $(TMP_OUTPUT)/vmem7.mif \
	-o8 $(TMP_OUTPUT)/vmem8.mif \
	-o9 $(TMP_OUTPUT)/vmem9.mif \
	-o10 $(TMP_OUTPUT)/vmem10.mif \
	-o11 $(TMP_OUTPUT)/vmem11.mif \
	-o12 $(TMP_OUTPUT)/vmem12.mif \
	-o13 $(TMP_OUTPUT)/vmem13.mif \
	-o14 $(TMP_OUTPUT)/vmem14.mif \
	-o15 $(TMP_OUTPUT)/vmem15.mif


mif:
	stat $(OBJDIR)/$(PROJ_NAME).hex

# this is a special target that will detect changes in non .c files which
# mean we need to recompile, it forces a clean
# you must run this first and not last
sensative_clean: $(OBJDIR)/.sensative_cookie

$(OBJDIR)/.sensative_cookie: $(SENSATIVE_SRCS)
	mkdir -p $(OBJDIR)
	make clean > $(OBJDIR)/.sensative_cookie

elf_hex_asm: $(OBJDIR)/$(PROJ_NAME).elf $(OBJDIR)/$(PROJ_NAME).hex $(OBJDIR)/$(PROJ_NAME).asm $(OBJDIR)/$(PROJ_NAME)_symbols.txt
	@echo "done: $(PROJ_NAME).hex"

# see https://stackoverflow.com/a/12300798/836450 for the | character
$(OBJDIR)/%.elf: $(OBJS) | $(OBJDIR)
	$(RISCV_CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

$(C_HEX_PATH): $(OBJDIR)/$(PROJ_NAME).hex

%.hex: %.elf
	$(RISCV_OBJCOPY) -O ihex $^ $@
	mkdir -p $(TMP_OUTPUT)
	cp $@ $(TMP_OUTPUT)/

%.bin: %.elf
	$(RISCV_OBJCOPY) -O binary $^ $@

%.asm: %.elf
	$(RISCV_OBJDUMP) -S -d $^ > $@
	mkdir -p $(TMP_OUTPUT)
	cp $@ $(TMP_OUTPUT)/

%_symbols.txt:  %.elf
	$(RISCV_OBJDUMP) -t $^ > $@
	mkdir -p $(TMP_OUTPUT)
	cp $@ $(TMP_OUTPUT)/

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(RISCV_CC) -c $(CFLAGS)  $(INC) -o $@ $^

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(RISCV_CC) -c $(CFLAGS)  $(INC) -o $@ $^

$(OBJDIR)/%.o: %.S
	@mkdir -p $(dir $@)
	@$(RISCV_CC) -c $(CFLAGS) -o $@ $^ -D__ASSEMBLY__=1

$(OBJDIR):
	mkdir -p $@


#OMG do not add 	rm -f $(OBJDIR)/.sensative_cookie
# below! (Causes build everytime)
clean:
	rm -f $(OBJDIR)/$(PROJ_NAME).elf
	rm -f $(OBJDIR)/$(PROJ_NAME).hex
	rm -f $(OBJDIR)/$(PROJ_NAME).map
	rm -f $(OBJDIR)/$(PROJ_NAME).v
	rm -f $(OBJDIR)/$(PROJ_NAME).asm
	rm -f $(OBJDIR)/$(PROJ_NAME)_symbols.txt
	rm -f $(TMP_OUTPUT)/$(PROJ_NAME).asm
	rm -f $(TMP_OUTPUT)/$(PROJ_NAME)_symbols.txt
	rm -f $(TMP_OUTPUT)/$(PROJ_NAME).hex
	rm -f $(MIF_OUT_TARGETS)
	rm -f $(VMEM_MIF_OUT_TARGETS)

	find $(OBJDIR2) -type f -name '*.o' -print0 | xargs -0 -r rm

# see https://www.gnu.org/software/make/manual/html_node/Special-Targets.html
.SECONDARY: $(OBJS)
