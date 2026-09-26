#include "dma.h"
#include "mover.h"
#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS20
#include "ringbus2_post.h"

int main(void)
{
    ring_block_send_eth(0xdeadbeef);

    vmem_fill_low(0x0, 8, 0xdead);
    vmem_fill_low(0x8, 1, 0xcafe);

    ring_block_send_eth(0x0);
}
