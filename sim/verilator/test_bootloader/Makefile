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
# OVERRIDE_CS11_C=1
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
CS11_NO_RISCV=1
CS12_NO_RISCV=1
CS02_NO_RISCV=1
CS01_NO_RISCV=1
# RX FPGAs
CS31_NO_RISCV=1
CS32_NO_RISCV=1
CS22_NO_RISCV=1
CS21_NO_RISCV=1
# CS20_NO_RISCV=1

# required to set this before including this file
include $(HIGGS_ROOT)/scripts/make_include/tb_common.mk


# show:
# 	gtkwave wave_dump.vcd --giga -S higgs.tcl &


