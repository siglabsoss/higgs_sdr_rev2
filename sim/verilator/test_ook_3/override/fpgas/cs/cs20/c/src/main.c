#include "xbaseband.h"
#include "csr_control.h"
#include "dma.h"
#include "vmem.h"
#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "random.h"


#define ENABLE_TB_DEBUG

#ifndef ENABLE_TB_DEBUG
#define DISABLE_TB_DEBUG
#endif
#include "tb_debug.h"

// #define OOK_USE_DEBUG



#include "ook_modem.h"

static uint32_t lifetime_32 = 0xface;

static VMEM_SECTION unsigned int fft_frame[32];

uint32_t get_one(void) {
    dma_block_get(VMEM_DMA_ADDRESS(fft_frame), 1);

    unsigned occupancy;

    while(1) {
        CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
        if( occupancy == 0) {
            break;
        }
    }

    return fft_frame[0];
}

void got_ook_message(const OOKMessage* const message) {
    _printf("Got message 0x%x 0x%x    0x%x\n", message->data[0], message->data[1], lifetime_32);

    uint32_t delta = lifetime_32 - message->data[0] - OOK_DELTA_FRAMES;

    delta &= 0xffffff;

    ring_block_send_eth(DEBUG_0_PCCMD | delta);
    // ring_block_send_eth(message->data[1]);
}

OOKDemod demod;


int main(void) {
    simple_random_seed(0x2af8712c);

    setup_debug();

    // _printf("hi\n");

    ook_register_callback(&got_ook_message);


    // ook_sync[0] = simple_random();

    uint32_t word;

    ook_prep_demod(&demod);
    // OOKMessage message;
    // ook_prep_outbound(&message);

    uint32_t ta,tb;

        STALL(20);

    while(1) {
        word = get_one();
        CSR_READ(TIMER_VALUE, ta);
        ook_demodulate(&demod, word);
        CSR_READ(TIMER_VALUE, tb);

        // _printf("%x\n", word);
        
        _printf("time: %d\n", tb-ta);
        lifetime_32++;
        // ring_block_send_eth(word);
    }

    // ring_block_send_eth(fft_frame[0]);
    // ring_block_send_eth(0xde);
}
