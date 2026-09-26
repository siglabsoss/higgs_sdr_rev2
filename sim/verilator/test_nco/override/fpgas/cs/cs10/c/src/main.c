#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "vmalloc.h"
#include "circular_buffer.h"
#include "fill.h"
#include "ringbus.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS10
#include "ringbus2_post.h"


int main(void) {

    while(1) {
        dma_in_set(0, 65536);
    }

} 