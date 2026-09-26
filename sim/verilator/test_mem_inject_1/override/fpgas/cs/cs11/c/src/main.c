#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "unit_test_ring.h"
 
volatile unsigned int d;
volatile unsigned int e; 



int main(void)
{

    // *pass_fail_0 = 0xdeadfeed;

    ring_block_send_eth(*pass_fail_0);
    ring_block_send_eth(*pass_fail_1);

    while(1) {
        STALL(1);
    }

}
