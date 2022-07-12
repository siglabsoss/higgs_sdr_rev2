# Purpose

Found that an ofdm frame is being repeated every 512 frames during user data transmission.

This happens due to contention from how buffers are selected between frame sync and the other one.

cs20 has junk ideas to fix this problem that didn't make it into tx_7/cs20/main.c