#include "csr_control.h"
#include "dma.h"
#include "ringbus.h"


#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "readback_hash.h"



void debug_callback(unsigned int data) {

}


int main(void)
{
    Ringbus ringbus;

    ring_register_callback(&debug_callback, EDGE_EDGE_IN);

    while(1) {

        check_ring(&ringbus);
    }

}
