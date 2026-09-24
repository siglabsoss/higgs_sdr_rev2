#define STREAM_CHUNK (64)
#define MOD_STALL (100)
#define MOD_SIZE (300)

#include "dma.h"
#include "nco_data.h"
#include "random.h"
#include "xbaseband.h"
#include "csr_control.h"
#include "check_bootload.h"
#include "tb_debug.h"
#include "random.h"
#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"

unsigned seed_set = 0;

void seed_callback(unsigned int data) {
    simple_random_seed(data);
    seed_set = 1;
}

int main(void)
{
    unsigned int occupancy;
    unsigned int random_int;
    unsigned int input_size;
    unsigned int mod_stall;
    unsigned int mod_size;
    unsigned int stall_max = 200;
    unsigned int stall_min = 1;
    unsigned int size_max = 600;
    unsigned int size_min = 15;
    setup_debug();

    ring_register_callback(&seed_callback, SEED_RANDOM_CMD);

    Ringbus ringbus;
    while(seed_set == 0) {
        check_ring(&ringbus);
    }

    mod_stall = (simple_random() % (stall_max - stall_min)) + stall_min;
    mod_size = (simple_random() % (size_max - size_min)) + size_min;
    while (1) {
        while(1) {
            CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
            if( occupancy < DMA_0_SCHEDULE_DEPTH) break;
        }
        random_int = simple_random() % mod_stall;
        input_size = simple_random() % mod_size;
        // input_size = 64;
        for (unsigned int i = 0; i < random_int; i++) {
            STALL(1);
        }
        CSR_WRITE(DMA_0_START_ADDR, 0);
        CSR_WRITE(DMA_0_LENGTH, input_size);
        CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff);
        CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);
    }
    // no_exit_stream();
}
