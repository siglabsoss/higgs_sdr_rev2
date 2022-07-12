#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "dma.h"
#include "fill.h"
#include "mover.h"
#include "mapper.h"
#include "ringbus.h"
#include "circular_buffer.h"
#include "vmem.h"


#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS10
#include "ringbus2_post.h"

// VMalloc mgr;

// programs with large EMPTY DMEM arrays take awhile to start due to crt.S filling zeros at boot
// putting something in causes bss to NOT zero? weird
unsigned int ring_storage[1024+256] = {1};
unsigned int ring_used = 0;

VMEM_SECTION unsigned int junk_data[1024] = {1};

void cb_storage(unsigned int data) {
    ring_storage[ring_used] = data;
    ring_used++;
    SET_REG(x3, data);
    // ring_block_send_eth(data);
}

void cb_dump_storage(unsigned int data) {
    ring_block_send_eth(ring_used);
    for(unsigned int i = 0; i < ring_used; i++) {
        ring_block_send_eth(ring_storage[i]);
    }
}


int main(void) {
    Ringbus r;

    SET_REG(x3, 0xdeadbeef);

    ring_register_callback(&cb_storage, EDGE_EDGE_IN);
    ring_register_callback(&cb_dump_storage, EDGE_EDGE_OUT);

    unsigned int occupancy;

    while(1) {
        check_ring(&r);

        CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);

        // forever run the input DMA over the same memory
        // we can send data from cs20 in the test, and then look at 
        if(occupancy < DMA_0_SCHEDULE_DEPTH) {
            dma_block_get(VMEM_DMA_ADDRESS(junk_data),1024);
        }

    }

}