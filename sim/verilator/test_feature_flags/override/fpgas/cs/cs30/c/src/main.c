#include "xbaseband.h"
#include "vmem.h"
#include "csr_control.h"
#include "dma.h"

#include "ringbus.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "feature_flags.h"

#include <stdint.h>

# include <riscv_boost/preprocessor/slot/counter.hpp>



void pet_fake_app() {
    STALL(10);
}

volatile uint32_t external_conditions;

void setup_fake() {
    STALL(1);
    // never gets inside
    if( external_conditions == 30243919) {
        ring_block_send_eth(external_conditions+1);
    }
}


int main(void)
{
    Ringbus ringbus;
    external_conditions = 4;
    external_conditions++;

    setup_fake();

    uint32_t v1 = BOOST_PP_COUNTER;

#include BOOST_PP_UPDATE_COUNTER()

    uint32_t v2 = BOOST_PP_COUNTER;

    ring_block_send_eth(v1);
    ring_block_send_eth(v2);

    while(1) {
        pet_fake_app();
        check_ring(&ringbus);
    }

// no_exit_stream();
}
