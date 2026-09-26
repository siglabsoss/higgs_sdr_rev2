#include "test_dma_torture.h"


unsigned seed_set = 0;

void seed_callback(unsigned int data) {
    simple_random_seed(data);
    seed_set = 1;
}



int main(void)
{
    r_delay = 40;
    r_delay_base = 200;

    // simple_random_seed(0x1e24bc);
    ring_register_callback(&seed_callback, SEED_RANDOM_CMD);

    Ringbus ringbus;
    while(seed_set == 0) {
        check_ring(&ringbus);
    }

    run_receiver();

    // Ringbus ringbus;
    while(1) {
        check_ring(&ringbus);
    }
}
