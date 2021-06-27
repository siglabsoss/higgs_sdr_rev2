set_vga:
	python $(HIGGS_ROOT)/scripts/set_adc.py -vga $(VGA_ATTENUATION)\
	                                        -c $(RX_CHANNEL)\
	                                        -ht $(HIGGS_HOST)\
	                                        -tcp $(HIGGS_TX_CMD_PORT)

set_dsa:
	python $(HIGGS_ROOT)/scripts/set_adc.py -dsa $(DSA_ATTENUATION)\
	                                        -c $(RX_CHANNEL)\
	                                        -ht $(HIGGS_HOST)\
	                                        -tcp $(HIGGS_TX_CMD_PORT)
saturation_map:
	python $(HIGGS_ROOT)/scripts/set_adc.py -map \
	                                        -ht $(HIGGS_HOST)\
	                                        -tcp $(HIGGS_TX_CMD_PORT)

block_size_map:
	python $(HIGGS_ROOT)/scripts/set_adc.py -bs \
	                                        -ht $(HIGGS_HOST)\
	                                        -tcp $(HIGGS_TX_CMD_PORT)

clipping_var_map:
	python $(HIGGS_ROOT)/scripts/set_adc.py -var $(iterations)\
	                                        -ht $(HIGGS_HOST)\
	                                        -tcp $(HIGGS_TX_CMD_PORT)

get_adc_samples:
	python $(HIGGS_ROOT)/scripts/receive_data.py -c $(samples)\
	                                             -ht $(HIGGS_HOST)\
	                                             -oht $(OUR_HOST)\
	                                             -rdp $(HIGGS_RX_DATA_PORT)

pass_adc_samples:
	python $(HIGGS_ROOT)/scripts/receive_data.py -pd $(samples)\
	                                             -ht $(HIGGS_HOST)\
	                                             -oht $(OUR_HOST)\
	                                             -rdp $(HIGGS_RX_DATA_PORT)

get_pass_samples:
	python $(HIGGS_ROOT)/scripts/receive_data.py -c $(samples)\
	                                             -pd $(samples)\
	                                             -ht $(HIGGS_HOST)\
	                                             -oht $(OUR_HOST)\
	                                             -rdp $(HIGGS_RX_DATA_PORT)

disable_adc_counter:
	python $(HIGGS_ROOT)/scripts/receive_data.py -dc\
	                                             -ht $(HIGGS_HOST)\
	                                             -oht $(OUR_HOST)\
	                                             -rdp $(HIGGS_RX_DATA_PORT)
