#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "dma.h"
#include "fill.h"
#include "mover.h"
#include "mapper.h"
#include "ringbus.h"
#include "circular_buffer.h"


#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS12
#include "ringbus2_post.h"

int main(void) {
    CSR_WRITE(DMA_0_START_ADDR, 0);
    CSR_WRITE(DMA_0_LENGTH, 1024*64);
    CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff); // start right away
    CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);
}
