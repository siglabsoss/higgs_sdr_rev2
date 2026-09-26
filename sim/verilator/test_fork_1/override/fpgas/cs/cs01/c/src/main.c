#include "test_dma_torture.h"
#include "dma.h"
#include "tb_inject_mem.h"
#include "pass_fail.h"
#include <stdint.h>

// static VMEM_SECTION unsigned int vmem_zeros[1024];

int main(void)
{
    // CSR_WRITE(DMA_0_FLUSH_SCHEDULE, 0);
    // CSR_WRITE(DMA_1_FLUSH_SCHEDULE, 0);

    const uint32_t test_seed = get_tb_seed(); // grab seed from test-bench

    simple_random_seed(test_seed);

    const bool is_verilator = get_is_verilator();
    const uint32_t higgs_id = get_higgs_id();

    // ring_block_send_eth(0x10000000 | higgs_id);
    // ring_block_send_eth(*pass_fail_1);

    // test_seed is the same for each fpga, this loop changes the random pool per-fpga
    for(unsigned i = 0; i < OUR_RING_ENUM; i++) {
        const unsigned burn = simple_random();
        (void)burn;
    }

    // ring_block_send_eth(test_seed);

    if( higgs_id == 1 ) {
        dma_torture_output_type = 1;
    }

    run_sender();


    // dma_block_send(VMEM_DMA_ADDRESS(vmem_zeros), 1024);

    Ringbus ringbus;
    while(1) {
        check_ring(&ringbus);
    }
}
