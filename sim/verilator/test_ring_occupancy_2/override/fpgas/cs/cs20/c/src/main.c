#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "unit_test_ring.h"


void rbcb(const unsigned int data) {
    unsigned int data2 = (~data) & 0xffffff;

    ring_block_send_eth(EDGE_EDGE_IN | data2);
}



void getmem(const unsigned int data) {
    unsigned int* memp = (unsigned int*)data;

    unsigned int readword = *memp;

    const unsigned int high = (readword>>16) & 0xffff;
    const unsigned int low  = readword & 0xffff;

    ring_block_send_eth(EDGE_EDGE_OUT | 0x00000 | low);
    ring_block_send_eth(EDGE_EDGE_OUT | 0x10000 | high);
}




int main(void) {
    Ringbus ringbus;
    ring_register_callback(&rbcb, EDGE_EDGE_IN);
    ring_register_callback(&getmem, EDGE_EDGE_OUT);
    while(1) {
        check_ring(&ringbus);
    }
}

