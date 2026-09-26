# Purpose

Extensive randomized test for do_forward.  This re-uses components from `test_dma_torture`.  This is designed to have force pushback by slowing down sample consumption in `cs02`.

# Flow
* `cs11` - Runs dma_torture sender
* `cs12` - Runs do_forward
* `cs02` - Runs dma_torture receiver
* Testbench sends a seed to cs11
* Testbench sends to cs02:
  * a random delay value
  * a random delay base value
  * a random seed
* A 4k counter is sent in random chunks from cs11
* cs12 forwards in chunks of 256 (but could be changed to any number that can evenly divide 4k)
* cs02 catches input dma in random chunks and then checks each value of the counter for correctness

