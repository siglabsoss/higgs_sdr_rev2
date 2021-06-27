
.PHONY: clean_verilator_parse_syms verilator_parse_syms

PV_OUTFILE=$(PARSE_VERILATOR_OBJ_DIR)/vmem_types.h

verilator_parse_syms: $(PARSE_VERILATOR_OBJ_DIR)/vmem_types.h


# grep for lines in Syms.h file
# only writes typedef lines for things it finds
define write_vmem_h_line
cat $(PARSE_VERILATOR_OBJ_DIR)/Vtb_higgs_top__Syms.h | grep dat | grep -v include | sed -e 's/^    \(\w*\) \(\w*$(1)\w*\);/MATCH\1/g' | grep MATCH | sed -e 's/MATCH\(\w*\)/\1/' | tr -d '\n' | grep V; \
if [ $$? -eq 0 ] ; \
then \
echo -n "typedef " >> $(PV_OUTFILE); \
cat $(PARSE_VERILATOR_OBJ_DIR)/*Syms.h | grep dat | grep -v include | sed -e 's/^    \(\w*\) \(\w*$(1)\w*\);/MATCH\1/g' | grep MATCH | sed -e 's/MATCH\(\w*\)/\1/' | tr -d '\n' >> $(PV_OUTFILE); \
echo " $(1)_node_t;" >> $(PV_OUTFILE); \
else \
echo "path no" ; \
fi
endef

$(PV_OUTFILE): $(PARSE_VERILATOR_OBJ_DIR)/Vtb_higgs_top__Syms.h
	@echo "#ifndef __VMEM_TYPES_H__" >  $(PV_OUTFILE)
	@echo "#define __VMEM_TYPES_H__" >> $(PV_OUTFILE)

	$(call write_vmem_h_line,cs00)
	$(call write_vmem_h_line,cs01)
	$(call write_vmem_h_line,cs10)
	$(call write_vmem_h_line,cs11)
	$(call write_vmem_h_line,cs20)
	$(call write_vmem_h_line,cs21)
	$(call write_vmem_h_line,cs30)
	$(call write_vmem_h_line,cs31)
	$(call write_vmem_h_line,eth)
	
	@echo "#endif" >> $(PV_OUTFILE)

clean_verilator_parse_syms:
	rm -f $(PV_OUTFILE)
