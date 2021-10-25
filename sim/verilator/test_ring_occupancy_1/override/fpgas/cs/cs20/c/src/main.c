#include "xbaseband.h"
#include "csr_control.h"
#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "stall2.h"

unsigned got = 0;
unsigned correct = 0;

void rbcb(const unsigned int data) {
    SET_REG(x3, 0x0);
    // SET_REG(x3, 0xffffffff);
    SET_REG(x3, data);

    const unsigned expected = 0xC001 + got;

    if( data == expected) {
        correct++;
    }

    got++;
}


int main(void) {
    int sent_results = 0;
    Ringbus ringbus;
    ring_register_callback(&rbcb, EDGE_EDGE_IN);
    stall2(1550);
    while(1) {
        STALL(1000);
        STALL(1000);
        STALL(1000);
        check_ring(&ringbus);

        if( correct >= 16 && !sent_results ) {
            ring_block_send_eth(EDGE_EDGE_OUT | 1);
            sent_results = 1;
        }
    }
}

