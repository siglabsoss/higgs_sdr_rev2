# on tx side
tx_index = 49

# on rx side
def tx_rx(sc):
	if(sc<512):
		return 1024-sc;
	return None

# on rx side after mover_mappr
def subcarrierForBufferIndex(i):
    expected_subcarrier = 945 + i*2;
    return expected_subcarrier

def bufferIndexForSc(idx):
	return (idx - 945) / 2.0

rx_index = tx_rx(tx_index)
rx_array_idx = bufferIndexForSc(rx_index)


print "on tx side: ", tx_index
print "on rx side: ", rx_index
print "subcarrier array index on rx: ", rx_array_idx