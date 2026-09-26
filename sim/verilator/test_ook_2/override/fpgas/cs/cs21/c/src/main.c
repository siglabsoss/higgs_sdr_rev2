#include "xbaseband.h"
#include "csr_control.h"
#include "dma.h"
#include "vmem.h"
#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "random.h"

// #define ENABLE_TB_DEBUG

#ifndef ENABLE_TB_DEBUG
#define DISABLE_TB_DEBUG
#endif
#include "tb_debug.h"

// #define OOK_USE_DEBUG

#include "ook_modem.h"

static VMEM_SECTION unsigned int fft_frame[32];

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
}

// const int32_t baseline_noise_tol = 20*0x000001e0/2;
int32_t baseline_noise_tol = 4*0x000001e0;
// const int32_t baseline_noise_tol = 40;
// const int32_t baseline_noise_tol = 80;
uint32_t add_noise(const uint32_t in) {
    int32_t noise_re = (simple_random() % (baseline_noise_tol)) - (baseline_noise_tol/2) ;
    int32_t noise_im = (simple_random() % (baseline_noise_tol)) - (baseline_noise_tol/2) ;

    int32_t re,im;
    re =  (int16_t)(in & 0xffff);
    im =  (int16_t)((in>>16) & 0xffff);

    re += noise_re;
    im += noise_im;

    // const uint32_t assembled = ((im << 16) & 0xffff) | (re&0xffff); // real only noise im is not used
    const uint32_t assembled = ((im << 16) & 0xffff0000) | (re&0xffff);

    return assembled;
}

// uint32_t add_gain(uint32_t in, float g) {
//     int32_t re,im;
//     re =  (int16_t)(in & 0xffff);
//     im =  (int16_t)((in>>16) & 0xffff);

//     re *= g;
//     im *= g;

//     const uint32_t assembled = ((im << 16) & 0xffff0000) | (re&0xffff);

//     return assembled;
// }





void mm(uint32_t* v, uint32_t mmin, uint32_t mmax) {
    *v = (simple_random() % (mmax-mmin)) + mmin;
}

void pick2(int32_t* baseline_noise_tol, uint32_t* baseline_noise, uint32_t* pilot) {

    mm((uint32_t*)baseline_noise_tol, 1920, 0x7fff);
    *baseline_noise = 0;
    mm(pilot, 1000, 0x7fff);

    // int32_t delta = *pilot - (*baseline_noise + *baseline_noise_tol);
    // cout << "delta: " << delta << "\n";

    // float f = 1.42;
    // f = 1.21;


    // if( 
    //     ( (((int32_t)*baseline_noise_tol)*f) > ((int32_t)*pilot))
    //   )
    //  // 
    // {
    //     // re-roll
    //     pick(baseline_noise_tol, baseline_noise, pilot);
    // }
}

void pick(int32_t* baseline_noise_tol, uint32_t* baseline_noise, uint32_t* pilot) {
    float f;
    f = 1.21; // ok 
    // f = 1.15; // fail
    do {
        pick2(baseline_noise_tol, baseline_noise, pilot);
    }while(( (((int32_t)*baseline_noise_tol)*f) > ((int32_t)*pilot)));
}




int main(void) {

    simple_random_seed(0x2af8712c);

    setup_debug();

    unsigned leadin = 12;

    const uint32_t baseline_noise = 74;
    // const uint32_t baseline_noise = 0x000001e0;


    OOKMessage message;
    ook_prep_outbound(&message);


    // OOKMessage demod;
    // ook_prep_outbound(&demod);
    // uint32_t prog = 0;

    // message.data[0] = 0xfade0000;
    // message.data[1] = 0xeeeeaaaa;

    // message.data[0] = 0xac93ac01;
    // message.data[1] = 0x00000000;

    // message.data[0] = 0xf500000f;
    // message.data[1] = 0x00000000;

    // message.data[0] = 0xffffffff;
    // message.data[1] = 0x00000000;



    uint32_t outword;


    for(unsigned j = 0; j < 10; j++) {
        bool done = false;
        ook_prep_outbound(&message);

        message.data[0] = 0xf500000f + j;
        message.data[1] = 0x5000 | j | (j << 4) | (j << 8);

        for(unsigned i = 0; i < (leadin+OOK_TOTAL_LENGTH) ; i++) {

            // if( !done ) {
            //     ook_modulate_next(&message, &outword, &done);
            //     send_one(outword);

            //     // unload_bit(demod.data, outword, prog);
            //     // prog++;
            // } else {
            //     // send_one(demod.data[0]);
            // }

            if( i < leadin ) {
                send_one(add_noise(baseline_noise));
                // send_one(baseline_noise);
            }

            if( i >= leadin ) {
                if( !done ) {
                    ook_modulate_next(&message, &outword, &done);
                    // send_one(add_noise(outword));
                    // send_one(add_gain(outword,0.3));
                    send_one(add_noise(add_gain(outword,0.3f)));

                    // we can corrupt the hash by taking advantage of the fact
                    // that the modulator will calculate the has on the first 
                    // call to ook_modulate_next
                    // if we modify the body of the message afterwards we can fool it
                    if( j == 1 && message.count == 3 ) {
                        message.data[0] ^= 0x00100000;
                    }

                    // send_one(outword);

                    // unload_bit(demod.data, outword, prog);
                    // prog++;
                }
            }


        }
    }


    // ring_block_send_eth(0xde);

}
