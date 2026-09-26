#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "vmem_counter_alpha.h"


int main(void)
{
    for(int i = 0; i < 800; i++) {
        STALL(40);
    }

    unsigned int addr = VMEM_DMA_ADDRESS(vmem_counter_alpha);

    // send same vector over and over, but with a different split point
    for( int j = 0; j < 900; j++ ) {
        unsigned int split = 22 + j;
        dma_block_send(addr, split);
        dma_block_send(addr+split, 1024-split);
    }

}


