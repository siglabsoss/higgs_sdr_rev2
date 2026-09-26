# Test
Incomplete hand test of `CSR_WRITE(DMA_0_FLUSH_SCHEDULE, 1);`

# Flow
* Sets up a few short input DMA's
* As one finishes, we write to vector mem, but it's nut used I think (stolen from eth) 
* Then after a few loops we call flush
