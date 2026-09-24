#include "vmem.h"
#include "csr_control.h"
#include "dma.h"
#include "vmem_counter_8k.h"
#include "tb_debug.h"
#include "random.h"
#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#define INCLUDE_VECTOR_AS final_flush
#define VECTOR_INITIAL_VALUE 0xffffffff
#include "vmem_vector_1k.h"


// #define VMEM_SIZE (1024 * 8)
#define CHUNK_SIZE (1024)
#define CHUNKS (8)


unsigned seed_set = 0;

void seed_callback(unsigned int data) {
    SET_REG(x3, 0);
    SET_REG(x3, 1);
    SET_REG(x3, data);
    simple_random_seed(data);
    seed_set = 1;
}


/**
 *
 */
void enable_output_dma(void);

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

    unsigned int short_chunk = 1 + (simple_random()%6);
    unsigned int short_run = 10 + (simple_random()%1000);

    mod_stall = (simple_random() % (stall_max - stall_min)) + stall_min;

    for(unsigned i = 0; i < CHUNKS; i++) {
        const unsigned offset = i*CHUNK_SIZE;

        if( i == short_chunk ) {

            if( 0 ) {
                // this version will corrupt this frame now, however the dma actually
                // only detects an error when a last comes early 
                dma_block_send(VMEM_DMA_ADDRESS(vmem_counter_8k) + offset, short_run);
            } else {
                // this version will corrupt now, and dma will detect it now
                // this may be less realistic, but this makes it easier to debug
                dma_block_send_finalized(VMEM_DMA_ADDRESS(vmem_counter_8k) + offset, short_run, 1);
            }
        } else {
            dma_block_send_finalized(VMEM_DMA_ADDRESS(vmem_counter_8k) + offset, CHUNK_SIZE, 1);
        }

        

        random_int = simple_random() % mod_stall;

        SET_REG(x3, 2);
        SET_REG(x3, random_int);


        for (unsigned int j = 0; j < random_int; j++) {
            STALL(1);
        }

    }

    dma_block_send_finalized(VMEM_DMA_ADDRESS(final_flush), 1024, 1);
}
