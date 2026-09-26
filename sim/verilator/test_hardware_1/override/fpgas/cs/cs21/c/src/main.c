#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
// #include "pass_fail.h"
#include "dma.h"
// #include "fill.h"
// #include "mover.h"
// #include "mapper.h"
#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"
// #include "circular_buffer.h"
// #include "feedback_bus.h"
#include "trunk_types.h"
#include "vmem_copy.h"
#include "handle_generic_op.h"
#include "stall2.h"
#include "subtract_timers.h"

#include "../../../cs11/c/src/cooked_data_unrotated.h"


uint32_t send_delay = 970;

VMEM_SECTION unsigned int trunk[TRUNK_LENGTH];

uint32_t lifetime_32 = 0;
uint32_t early_exit = 0;
uint32_t spam_ring_delay = 0;
uint32_t last_spam = 0;
uint32_t spam_count = 0;


uint32_t* pointer_for_generic_op(const uint32_t sel) {
    uint32_t *p = 0;
    switch(sel) {
        case 0:
            p = (uint32_t*) &lifetime_32;
            break;
        case 1:
            p = (uint32_t*) &send_delay;
            break;
        case 2:
            p = (uint32_t*) &early_exit;
            break;
        case 3:
            p = (uint32_t*) &spam_ring_delay;
            break;
        case 4:
            p = (uint32_t*) &last_spam;
            break;
        case 5:
            p = (uint32_t*) &spam_count;
            break;
        default:
            break;
            // return; // unknown selector
    }
    return p;
}

uint32_t frame_phase = 0;
void send_one(void) {

    unsigned int occupancy;
    CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
    if( occupancy >= DMA_1_SCHEDULE_DEPTH) {
        early_exit++;
        return;
    }

    uint32_t dma = VMEM_DMA_ADDRESS(vmem_counter) + (frame_phase*1024);

    dma_block_send(dma, 1024);

    // // write trunk
    trunk[TRUNK_FRAME_COUNTER] = lifetime_32;

    dma_block_send(VMEM_DMA_ADDRESS(trunk), TRUNK_LENGTH);


    frame_phase = (frame_phase+1) % 5;
    lifetime_32++;

    stall2(send_delay);
}



void do_spam_ring(void) {

    uint32_t now;

    CSR_READ(TIMER_VALUE, now);
    const unsigned delta = subtract_timers(now, last_spam);

    if( delta < spam_ring_delay ) {
        return;
    }


    ring_block_send_eth(DEBUG_18_PCCMD | (spam_count&0xffffff));

    spam_count++;
    last_spam = now;
    // spam_ring_delay = 0;
}



int main(void) {
    Ringbus ringbus;

    ring_register_callback(&handle_generic_callback_original, GENERIC_OPERATOR_CMD);
    handle_generic_register_get_pointer(&pointer_for_generic_op);
    handle_generic_register_ring(&ring_block_send_eth);


    while(1) {
        check_ring(&ringbus);
        send_one();

        if( spam_ring_delay ) {
            do_spam_ring();
        }

        // SET_REG(x3, 0x0);
    }

    return 0;
}


/*

I can make it crash with:
sjs.sendZerosToHiggs(1000)



sjs.dsp.op(hc.RING_ADDR_CS21, "set", 3, 1000)





*/

