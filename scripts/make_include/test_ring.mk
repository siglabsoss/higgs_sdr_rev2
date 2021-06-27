# (default) the last argument will be set to empty string
# run
#    make test_ring TEST_RING_FLAGS=-e
# to exit a non zero code per failure


test_ring:
	python $(HIGGS_ROOT)/scripts/test_ring.py -a\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)\
	                                          ${TEST_RING_FLAGS}


test_cs20_ring:
	python $(HIGGS_ROOT)/scripts/test_ring.py -f cs20\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)

test_cs10_ring:
	python $(HIGGS_ROOT)/scripts/test_ring.py -f cs10\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)

test_cs00_ring:
	python $(HIGGS_ROOT)/scripts/test_ring.py -f cs00\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)

test_cs01_ring:
	python $(HIGGS_ROOT)/scripts/test_ring.py -f cs01\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)

test_cs11_ring:
	python $(HIGGS_ROOT)/scripts/test_ring.py -f cs11\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)

test_cs21_ring:
	python $(HIGGS_ROOT)/scripts/test_ring.py -f cs21\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)

test_cs31_ring:
	python $(HIGGS_ROOT)/scripts/test_ring.py -f cs31\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)

test_cs30_ring:
	python $(HIGGS_ROOT)/scripts/test_ring.py -f cs30\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)

test_cs32_ring:
	python $(HIGGS_ROOT)/scripts/test_ring.py -f cs32\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)

test_cs12_ring:
	python $(HIGGS_ROOT)/scripts/test_ring.py -f cs12\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)

test_cs02_ring:
	python $(HIGGS_ROOT)/scripts/test_ring.py -f cs02\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)

test_cs22_ring:
	python $(HIGGS_ROOT)/scripts/test_ring.py -f cs22\
	                                          -ht $(HIGGS_HOST)\
	                                          -tcp $(HIGGS_TX_CMD_PORT)\
	                                          -rcp $(HIGGS_RX_CMD_PORT)