#include "test_dma_torture.h"



    // ring_register_callback(&calculate_epoc_callback, LIFETIME_TO_EPOC_CMD);

int seed_set = 0;

void seed_callback(unsigned int data) {
    switch(seed_set) {
        case 0:
            r_delay = data;
            seed_set++;
            break;
        case 1:
            r_delay_base = data;
            seed_set++;
            break;
        case 2:
            simple_random_seed(data);
            seed_set++;
            break;
    }
}





// int main2(void)
// {
//     r_delay_base = 400;
//     r_delay = 2;

//     simple_random_seed(2 + 2342 + 49999 + 0xf0e030ff);

//     run_receiver();

//     Ringbus ringbus;
//     while(1) {
//         check_ring(&ringbus);
//     }
// }

int main(void) {
    ring_register_callback(&seed_callback, SEED_RANDOM_CMD);

    Ringbus ringbus;
    while(seed_set != 3) {
        check_ring(&ringbus);
    }

    run_receiver();
}