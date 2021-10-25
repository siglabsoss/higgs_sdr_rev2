#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "dma.h"
#include "nco_data.h"


int main(void)
{
    int sent = 0;
    int base = 500;
    int jump = 1500;
    int next = base;
    unsigned int now;

    while(1) {
        CSR_READ(TIMER_VALUE, now);

        if( now > next ) {
            ring_block_send_eth(DEBUG_0_PCCMD | sent);

            sent++;

            next = base+ (sent*jump);
        }

    }
}

