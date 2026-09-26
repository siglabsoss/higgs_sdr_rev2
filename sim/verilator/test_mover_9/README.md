# Purpose



# FPGAs
* RX0
  * power detection,
  * symbol sync,
  * hand-tuned symbol advance,
  * Sfo/cfo adjustments (not usually used here on rx side, probably still work)
* RX1 - Sends 1024 + 16 extra words.  These are scheduled as a signle DMA
* RX2 
  * will send 1024 + 16 extra words.  these 16 words contain sync data.  This is a single dma schedule
  * 1/x Magnitude compensation
* RX3 - reverse mover, tagges data, every N frames we send every subcarrier
* RX4 - Forwarding only

# Naming
* RX0 - CS31
* RX1 - CS32
* RX2 - CS22
* RX3 - CS21
* RX4 - CS20


# Test 7
* Using mapmov_320_qpsk_1_fft_1_randomized.hex
  * we do this so that the values calculated and put in the trunk by cs32 will be different
  * I'm afaid if data is put in before cs31 with no fft, these values will be the same every frame
  * hiding any off by 1 frame issues
* we mess with the code of cs32 to try and get execution time down
* starting at 0x1194
  * I was able to get it to 0xbb1, with the old non ping_pong driver
* this is because adding the ping pong driver pushes us over
