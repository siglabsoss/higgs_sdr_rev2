#####################
#
#  This file generates verilator command line arguemnts
#  which let the user override paths of .mif files
#
# HIGGS_TEST_DIR must be defined before including this file
#
# This file sets both VERILATOR_C_OVERRIDE_DEFINES, VERILATOR_TB_INCLUDE_DEFINES
# however these variables are passed to the same command line, and could be merged
#
# ADDCFLAGS is set and is used to define c macros
#


VERILATOR_C_OVERRIDE_DEFINES=
VERILATOR_C_SUBMAKE_PATHS=
VERILATOR_TB_INCLUDE_DEFINES=
CS_FPGA=CS31 CS32 CS22 CS21 CS20 CS11 CS12 CS02 CS01
ETH_FPGA=ETH
RF_FPGA=ADC DAC
ALL_FPGA=$(CS_FPGA) $(ETH_FPGA) $(RF_FPGA)

# USE_LARGE_IMEM is now DISABLED and won't work

# please note that the trailing newline is important to tell make
# that these commands need their own shell
define override_c_submake
$(2) make -C $(1)

endef # don't remove whitespace before this line



define override_c_submake_clean
cd $(1) && make clean

endef # don't remove whitespace before this line



define verilator_c_override =
ifdef OVERRIDE_$(1)_C
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_SCALAR_0=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/scalar0.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_SCALAR_1=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/scalar1.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_SCALAR_2=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/scalar2.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_SCALAR_3=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/scalar3.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM0=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem0.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM1=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem1.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM2=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem2.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM3=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem3.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM4=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem4.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM5=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem5.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM6=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem6.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM7=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem7.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM8=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem8.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM9=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem9.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM10=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem10.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM11=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem11.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM12=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem12.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM13=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem13.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM14=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem14.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+$(1)_VMEM15=\"$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/build/tmp/vmem15.mif\"
ifndef $(1)_NO_RISCV
VERILATOR_C_SUBMAKE_PATHS+=$(HIGGS_TEST_DIR)/override/fpgas/$(if $(filter-out ETH,$(1)),cs,grav)/$(shell echo $(1) | tr A-Z a-z)/c/
endif
endif
endef

define verilator_tb_include =
ifdef TB_USE_$(1)
VERILATOR_TB_INCLUDE_DEFINES+=+define+TB_USE_$(1)=1
ADDCFLAGS += -CFLAGS -DTB_USE_$(1)
endif
ifdef $(1)_QENGINE_LITE
VERILATOR_TB_INCLUDE_DEFINES+=+define+$(1)_QENGINE_LITE=1
endif
ifdef $(1)_NO_RISCV
VERILATOR_TB_INCLUDE_DEFINES+=+define+$(1)_NO_RISCV=1
ADDCFLAGS += -CFLAGS -D$(1)_NO_RISCV
endif
endef

$(foreach fpga,$(CS_FPGA),$(eval $(call verilator_c_override,$(fpga))))
$(foreach fpga,$(ETH_FPGA),$(eval $(call verilator_c_override,$(fpga))))
$(foreach fpga,$(CS_FPGA),$(eval $(call verilator_tb_include,$(fpga))))
$(foreach fpga,$(RF_FPGA),$(eval $(call verilator_tb_include,$(fpga))))




ifdef CS01_EXTRA_IMEM
EVERY_EXTRA_IMEM+=CS01_EXTRA_IMEM=$(CS01_EXTRA_IMEM)
endif

ifdef CS11_EXTRA_IMEM
EVERY_EXTRA_IMEM+=CS11_EXTRA_IMEM=$(CS11_EXTRA_IMEM)
endif

ifdef CS20_EXTRA_IMEM
EVERY_EXTRA_IMEM+=CS20_EXTRA_IMEM=$(CS20_EXTRA_IMEM)
endif

ifdef CS21_EXTRA_IMEM
EVERY_EXTRA_IMEM+=CS21_EXTRA_IMEM=$(CS21_EXTRA_IMEM)
endif

ifdef CS31_EXTRA_IMEM
EVERY_EXTRA_IMEM+=CS31_EXTRA_IMEM=$(CS31_EXTRA_IMEM)
endif

ifdef ETH_EXTRA_IMEM
EVERY_EXTRA_IMEM+=ETH_EXTRA_IMEM=$(ETH_EXTRA_IMEM)
endif

ifdef CS02_EXTRA_IMEM
EVERY_EXTRA_IMEM+=CS02_EXTRA_IMEM=$(CS02_EXTRA_IMEM)
endif

ifdef CS12_EXTRA_IMEM
EVERY_EXTRA_IMEM+=CS12_EXTRA_IMEM=$(CS12_EXTRA_IMEM)
endif

ifdef CS22_EXTRA_IMEM
EVERY_EXTRA_IMEM+=CS22_EXTRA_IMEM=$(CS22_EXTRA_IMEM)
endif

ifdef CS32_EXTRA_IMEM
EVERY_EXTRA_IMEM+=CS32_EXTRA_IMEM=$(CS32_EXTRA_IMEM)
endif


# hijack this, and attach the LDSCRIPT here as well
ifdef LDSCRIPT
EVERY_EXTRA_IMEM+=LDSCRIPT=$(LDSCRIPT)
endif



# https://www.gnu.org/software/make/manual/html_node/Eval-Function.html
define invoke_c_override_submake
$(foreach path,$(VERILATOR_C_SUBMAKE_PATHS),$(call override_c_submake,$(path),$(EVERY_EXTRA_IMEM)))
endef

define invoke_c_override_clean
$(foreach path,$(VERILATOR_C_SUBMAKE_PATHS),$(call override_c_submake_clean,$(path)))
endef

VERILATOR_C_OVERRIDE_DEFINES+=+define+QAM8_MIF=\"$(HIGGS_TEST_DIR)/../../../fpgas/common/modules/mapper_mover/qam8.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+QAM16_MIF=\"$(HIGGS_TEST_DIR)/../../../fpgas/common/modules/mapper_mover/qam16.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+QAM32_MIF=\"$(HIGGS_TEST_DIR)/../../../fpgas/common/modules/mapper_mover/qam32.mif\"
VERILATOR_C_OVERRIDE_DEFINES+=+define+QAM64_MIF=\"$(HIGGS_TEST_DIR)/../../../fpgas/common/modules/mapper_mover/qam64.mif\"


# FOR UPCONVERTER
# add MIF file paths here
VERILATOR_TB_INCLUDE_DEFINES+=+define+DUC_SINCOS_PATH=\"$(IP_LIBRARY_REPO)/upconverter/hdl/duc_sincos.mif\"


# FOR DOWNCONVERTER
VERILATOR_TB_INCLUDE_DEFINES+=+define+DDC_SINCOS_PATH=\"$(IP_LIBRARY_REPO)/downconverter/hdl/ddc_sincos.mif\"


# FOR ETH
ifdef ETH_USE_MEGA_WRAPPER
VERILATOR_TB_INCLUDE_DEFINES+=+define+ETH_USE_MEGA_WRAPPER=1
ADDCFLAGS += -CFLAGS -DETH_USE_MEGA_WRAPPER
endif


# For dual higgs forked verilator
ifdef VERILATE_MULTIPLE_HIGGS
VERILATOR_TB_INCLUDE_DEFINES+=+define+VERILATE_MULTIPLE_HIGGS=1
ADDCFLAGS += -CFLAGS -DVERILATE_MULTIPLE_HIGGS
endif
