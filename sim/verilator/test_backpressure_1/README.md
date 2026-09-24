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


