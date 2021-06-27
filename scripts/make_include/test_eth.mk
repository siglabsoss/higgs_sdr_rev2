test_eth:
	python $(HIGGS_ROOT)scripts/test_eth.py -p $(HIGGS_TX_CMD_PORT)\
	                           -rp $(HIGGS_RX_CMD_PORT)\
	                           -ht $(HIGGS_HOST)\
	                           -pd $(BOOTLOADING_PACKET_DELAY)