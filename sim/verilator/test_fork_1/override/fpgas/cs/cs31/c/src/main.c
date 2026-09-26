#include "test_dma_torture.h"
#include "dma.h"
#include "tb_inject_mem.h"
#include <stdint.h>

int main(void)
{
    // CSR_WRITE(DMA_0_FLUSH_SCHEDULE, 0);
    // CSR_WRITE(DMA_1_FLUSH_SCHEDULE, 0);

    // simple_random_seed(0x9a529c2e);

    const uint32_t test_seed = get_tb_seed(); // grab seed from test-bench

    simple_random_seed(test_seed);

    const bool is_verilator = get_is_verilator();
    const uint32_t higgs_id = get_higgs_id();

    // ring_block_send_eth(0x10000000 | higgs_id);

    // test_seed is the same for each fpga, this loop changes the random pool per-fpga
    for(unsigned i = 0; i < OUR_RING_ENUM; i++) {
        const unsigned burn = simple_random();
        (void)burn;
    }


    if( higgs_id == 0 ) {
        dma_torture_input_type = 1;
    }

    run_receiver();


    dma_block_send(VMEM_DMA_ADDRESS(vmem_input), 4096);

    while(1) {
        check_ring(0);
    }
}
