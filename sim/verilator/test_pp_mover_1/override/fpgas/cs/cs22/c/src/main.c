#include "dma.h"
#include "vmem.h"
#include "ringbus.h"
#include "xvcordic.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "check_bootload.h"
#include "random.h"
#include "stall2.h"

#include "debug_320_1.h"

// VMEM_SECTION unsigned int zeros[1040];

// #define MASK_PULL (0x1ff)
#define MASK_PULL (0x1ff)

unsigned have_seed = 0;

void got_seed(const unsigned seed) {
    simple_random_seed(seed);
    
    have_seed = 1;
}


int main(void) {

    ring_register_callback( &got_seed, EDGE_EDGE_IN);
    while(!have_seed) {
        check_ring(0);
    }

    unsigned dma_ptr = VMEM_DMA_ADDRESS(cooked);

   // ring_register_callback(fine_sync_callback, SYNCHRONIZATION_CMD);
    unsigned sz = ARRAY_SIZE(cooked);


    unsigned int sent = 0;
    unsigned int pull;

    // this code does not handle the case where the test runs for longer than the size of the input
    // in put is 64k words, test runs for about 33k
    while(1) {
        pull = simple_random() & MASK_PULL;

        dma_block_send(dma_ptr + sent, pull);
        sent += pull;

        // STALL(300);
        stall2(150);

    }
}


