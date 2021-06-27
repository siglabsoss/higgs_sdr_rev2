# (default) the last argument will be set to empty string
# run
#    make test_ring TEST_RING_FLAGS=-e
# to exit a non zero code per failure


test_bootload_tx:
	python $(HIGGS_ROOT)/scripts/test_bootload.py -tx\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)\
	                                          -e

test_bootload_rx:
	python $(HIGGS_ROOT)/scripts/test_bootload.py -rx\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)\
	                                          -e
