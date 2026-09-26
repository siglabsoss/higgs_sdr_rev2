#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "vmalloc.h"
#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include <stdint.h>



#define RUN_SIZE (1024*2)

VMEM_SECTION unsigned int dma_input[RUN_SIZE] = {0};

int needs_validate = 0;

void check_in(void) {
    unsigned int in_occupancy;
    CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, in_occupancy);

    if( in_occupancy == 0 && needs_validate == 0) {
        needs_validate = 1;
        ring_block_send_eth(DEBUG_1_PCCMD);
    }
}

void validate(void) {
    if( needs_validate == 1 ) {
        needs_validate = 2;

        int result = 0;

        for(int i = 0; i < RUN_SIZE; i++) {
            uint32_t expected = i;

            if( dma_input[i] != expected ) {
                result = i+1;
                break;
            }
        }

        ring_block_send_eth(DEBUG_3_PCCMD | result);
    }
}

int main(void) {
    ring_block_send_eth(DEBUG_2_PCCMD);
    // read in full memory
    dma_in_set(VMEM_DMA_ADDRESS(dma_input), RUN_SIZE);


    Ringbus ringbus;

    while(1) {
        check_in();
        validate();
        check_ring(&ringbus);
    }

}
