#include "vmem.h"
#include "csr_control.h"
#include "dma.h"
#include "vmem_counter_8k.h"
#include "tb_debug.h"
#include "random.h"
#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"



#define VMEM_SIZE (1024 * 8)
#define CHUNK_SIZE (512)
#define CHUNKS (8*2)


unsigned seed_set = 0;

void seed_callback(unsigned int data) {
    simple_random_seed(data);
    seed_set = 1;
}


/**
 *
 */
void enable_output_dma(void);

/**
 *
 */
void update_output_buffer(void);

int main(void) {
    STALL(200);

    enable_output_dma();

    return 0;
};

void enable_output_dma(void) {
    unsigned int random_int;
    unsigned int mod_stall;
    unsigned int stall_max = 400;
    unsigned int stall_min = 1;

    ring_register_callback(&seed_callback, SEED_RANDOM_CMD);
    Ringbus ringbus;
    while(seed_set == 0) {
        check_ring(&ringbus);
    }

    mod_stall = (simple_random() % (stall_max - stall_min)) + stall_min;

    for(unsigned i = 0; i < CHUNKS; i++) {
        const unsigned offset = i*CHUNK_SIZE;
        dma_block_send(VMEM_DMA_ADDRESS(vmem_counter_8k) + offset, CHUNK_SIZE);
        

        random_int = simple_random() % mod_stall;
        for (unsigned int i = 0; i < random_int; i++) {
            STALL(1);
        }

    }
}
