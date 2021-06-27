.PHONY: status
# grep for lines in Syms.h file
# only writes typedef lines for things it finds

#status:
#	$(if $(TB_USE_CS20), $(if $(OVERRIDE_CS20_C), $(if $(CS20_NO_RISCV), $(warning "All CS20"), $(warning "no RISCV CS20")), $(warning "no override CS20")),$(warning "no USE CS20"))
#	$(if $(TB_USE_CS10), $(if $(OVERRIDE_CS10_C), $(if $(CS10_NO_RISCV), $(warning "All CS10"), $(warning "no RISCV CS10")), $(warning "no override CS10")),$(warning "no USE CS10"))
#	$(if $(TB_USE_CS00), $(if $(OVERRIDE_CS00_C), $(if $(CS00_NO_RISCV), $(warning "All CS00"), $(warning "no RISCV CS00")), $(warning "no override CS00")),$(warning "no USE CS00"))
#	$(if $(TB_USE_CS01), $(if $(OVERRIDE_CS01_C), $(if $(CS01_NO_RISCV), $(warning "All CS01"), $(warning "no RISCV CS01")), $(warning "no override CS01")),$(warning "no USE CS01"))
#	$(if $(TB_USE_CS11), $(if $(OVERRIDE_CS11_C), $(if $(CS11_NO_RISCV), $(warning "All CS11"), $(warning "no RISCV CS11")), $(warning "no override CS11")),$(warning "no USE CS11"))
#	$(if $(TB_USE_CS21), $(if $(OVERRIDE_CS21_C), $(if $(CS21_NO_RISCV), $(warning "All CS21"), $(warning "no RISCV CS21")), $(warning "no override CS21")),$(warning "no USE CS21"))
#	$(if $(TB_USE_CS31), $(if $(OVERRIDE_CS31_C), $(if $(CS31_NO_RISCV), $(warning "All CS31"), $(warning "no RISCV CS31")), $(warning "no override CS31")),$(warning "no USE CS31"))
#	$(if $(TB_USE_CS30), $(if $(OVERRIDE_CS30_C), $(if $(CS30_NO_RISCV), $(warning "All CS30"), $(warning "no RISCV CS30")), $(warning "no override CS30")),$(warning "no USE CS30"))
#	$(if  $(TB_USE_ETH),  $(if $(OVERRIDE_ETH_C),  $(if $(ETH_NO_RISCV), $(warning "All ETH"), $(warning "no RISCV ETH")), $(warning "no override ETH")),$(warning "no USE ETH"))




status:
	python $(HIGGS_ROOT)/scripts/test_status.py --orbit=$(OVERRIDE_CS20_C)