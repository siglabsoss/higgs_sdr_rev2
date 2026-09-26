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


/// also bumps lifetime_32 because logic below would be easy
/// to mess up radio between loops/counter/and send_one
/// this way they are all the same
void send_one(uint32_t w) {
    fft_frame[0] = w;
    dma_block_send(VMEM_DMA_ADDRESS(fft_frame), 1);

    unsigned occupancy;
    while(1) {
        CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
        if( occupancy == 0) {
            break;
        }
    }

    lifetime_32++;
}

// const int32_t baseline_noise_tol = 20*0x000001e0/2;
// const int32_t baseline_noise_tol = 4*0x000001e0;
// const int32_t baseline_noise_tol = 40;
const int32_t baseline_noise_tol = 80;
uint32_t add_noise(const uint32_t in) {
    int32_t noise_re = (simple_random() % (baseline_noise_tol)) - (baseline_noise_tol/2) ;
    int32_t noise_im = (simple_random() % (baseline_noise_tol)) - (baseline_noise_tol/2) ;

    int32_t re,im;
    re =  (int16_t)(in & 0xffff);
    im =  (int16_t)((in>>16) & 0xffff);

    re += noise_re;
    im += noise_im;

    const uint32_t assembled = ((im << 16) & 0xffff) | (re&0xffff); // real only noise im is not used
    // const uint32_t assembled = ((im << 16) & 0xffff0000) | (re&0xffff);

    return assembled;
}

uint32_t add_gain(uint32_t in, float g) {
    int32_t re,im;
    re =  (int16_t)(in & 0xffff);
    im =  (int16_t)((in>>16) & 0xffff);

    re *= g;
    im *= g;

    const uint32_t assembled = ((im << 16) & 0xffff0000) | (re&0xffff);

    return assembled;
}

int main(void) {

    simple_random_seed(0x2af8712c);

    setup_debug();

    unsigned leadin = 12;

    const uint32_t baseline_noise = 74;
    // const uint32_t baseline_noise = 0x000001e0;


    OOKMessage message;
    ook_prep_outbound(&message);


    uint32_t outword;


    for(unsigned j = 0; j < 10; j++) {
        bool done = false;

        for(unsigned i = 0; i < (leadin+OOK_TOTAL_LENGTH) ; i++) {

            if( i >= leadin ) {
                if( !done ) {

                    if( i == leadin )  {
                        ook_prep_outbound(&message);

                        message.data[0] = lifetime_32;
                        message.data[1] = 0;
                        _printf("Sending at %x\n", lifetime_32);
                    }

                    ook_modulate_next(&message, &outword, &done);
                    send_one(add_noise(outword));
                }
            }
        }
    }
}
