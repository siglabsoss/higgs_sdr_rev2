.PHONY: tx_cfo

tx_cfo:
	python scripts/tx_cfo.py -p $(HIGGS_TX_CMD_PORT)\
	                           -rp $(HIGGS_RX_CMD_PORT)\
	                           -ht $(HIGGS_HOST)\
	                           -pd $(BOOTLOADING_PACKET_DELAY)\
	                           -cfo $(cfo)

tx_phase:
	python scripts/tx_cfo.py -p $(HIGGS_TX_CMD_PORT)\
	                           -rp $(HIGGS_RX_CMD_PORT)\
	                           -ht $(HIGGS_HOST)\
	                           -pd $(BOOTLOADING_PACKET_DELAY)\
	                           -phase $(phase)
