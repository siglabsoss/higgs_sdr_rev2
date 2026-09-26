#include "xbaseband.h"
#include "vmem.h"
#include "csr_control.h"
#include "dma.h"
#include "ringbus.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"


int main(void)
{
    for(int i = 0; i < 8*100; i++) {
        STALL(50);
    }
    // ring_block_send_eth(0xdeadbeef);
    // ring_block_send_eth(0xffffffff);
    // ring_block_send_eth(0x00000000);
    // ring_block_send_eth(0x11223344);
    // for(int i = 0; i < 10; i++) {
    //     ring_block_send_eth(0xde000000 | i);
    // }
}

