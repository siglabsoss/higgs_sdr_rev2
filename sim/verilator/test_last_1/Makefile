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
# TB_USE_ADC=1

# This option removes the RISCV processor, but leaves the ringbus
# For example this could allow a simulation with only eth,cs20,cs10
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



clean: cleanall
	rm -f cs10.out cs20.out cs30.out cs10_out.hex cs20_out.hex output.txt t1.txt

bootload:
	make vall
	make bootload_cs30 && sleep 0.2
	make bootload_cs31 && sleep 0.2
	make bootload_cs21 && sleep 0.2
	make bootload_cs11 && sleep 0.2
	make bootload_cs01 && sleep 0.2
	make bootload_cs00

plot:
	python $(HIGGS_ROOT)/scripts/plot_file.py -csv ./cs00_out.csv -scr 17 18 19 20 21 22 63 64 65 66 67

plot2:
	python $(HIGGS_ROOT)/scripts/plot_file.py -csv ./cs00_out.csv
