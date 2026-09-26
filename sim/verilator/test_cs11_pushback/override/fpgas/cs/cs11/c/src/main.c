

#include "test_dma_torture.h"



int main(void)
{
    CSR_WRITE(DMA_0_FLUSH_SCHEDULE, 0);
    CSR_WRITE(DMA_1_FLUSH_SCHEDULE, 0);

    // simple_random_seed(0x1f29ab);
    // run_receiver();

    // don't seed for this one
    // seed will be set by timer value of first dma word
    run_reciver_random_seeded(0, 0x1f6C);

    Ringbus ringbus;
    while(1) {
        check_ring(&ringbus);
    }
}
