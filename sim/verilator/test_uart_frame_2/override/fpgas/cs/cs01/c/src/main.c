#include "csr_control.h"
#include "uart_driver.h"
#include "vmem.h"
#include "random.h"
#include "stall2.h"
#include <stdint.h>
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
    *uart_clock_divider = 0;

    ring_register_callback(&seed_callback, SEED_RANDOM_CMD);

    Ringbus ringbus;
    while(seed_set == 0) {
        check_ring(&ringbus);
    }


    unsigned stop = 512;
    // stop = 40;

    unsigned stalls[512];

    for(unsigned i = 0; i < stop; i++) {
        stalls[i] = (simple_random() % 3) + 3;
    }

    for(unsigned i = 0; i < stop; i++) {
        uint8_t c = (uint8_t)i;
        uart_put_char(c);
        stall2(stalls[i]);
    }

    return 0;
}
