#include "test_dma_torture.h"


    // ring_register_callback(&calculate_epoc_callback, LIFETIME_TO_EPOC_CMD);

unsigned seed_set = 0;

void seed_callback(unsigned int data) {
    simple_random_seed(data);
    seed_set = 1;
}



int main(void)
{
    ring_register_callback(&seed_callback, SEED_RANDOM_CMD);

    Ringbus ringbus;
    while(seed_set == 0) {
        check_ring(&ringbus);
    }

    // simple_random_seed(0x1a20d9+234+43+122);

    run_sender();
}

    // while(1) {
    //     pet_out();

    //     check_ring(&ringbus);
    //     pet_epoc_readback();
    //     // SET_REG(x4, 0);
    //     // SET_REG(x4, schedule->id_mask);


    // }
