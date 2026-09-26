# Purpose



# FPGAs
* CS00 - power detection, symbol sync, hand-tuned symbol advance, Sfo/cfo adjustments
* CS01 - Sends 1024 + 16 extra words.  These are scheduled as a signle DMA
* CS11 - CS01 will send 1024 + 16 extra words.  these 16 words contain sync data.  This is a single dma schedule
* CS21 - reverse mover, tagges data, every N frames we send every subcarrier

5/30/18 latest work between Ben/Zhen.  Can feedback rx phase and keep alignment relativly well over a cable.