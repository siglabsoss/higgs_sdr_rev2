#include "xbaseband.h"
#include "csr_control.h"
#include "dma.h"
#include "ringbus.h"
#include "vmem.h"
#include "random.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "feedback_bus_parse.h"



// make enable or disable our debug SET_REG calls
// #undef SET_REG
// #define SET_REG(x,y) ;


#include "vmalloc.h"
// declare as global
VMalloc mgr;

void handle_vector_type_ff(unsigned int* data, unsigned int vector_length) {
    
    SET_REG(x3, 0xdeadbeef);
    for(unsigned int i = 0; i < vector_length; i++) {
        SET_REG(x3, data[i]);
    }
}

void handle_vector_type_0(unsigned int* data, unsigned int vector_length) {
    SET_REG(x4, 0xdeadbeef);
    SET_REG(x4, 0xcafe);
    static unsigned int total_length = 0;

    total_length += vector_length;
    ring_block_send_eth(total_length);
}

void fb_vector_type_callback(unsigned int *header, unsigned int *body, unsigned int vector_length, unsigned int vtype) {
    switch(vtype) {
        case 0xff:
            handle_vector_type_ff(body, vector_length);
            // debug
            break;
        case 0:
            // something else
            handle_vector_type_0(body, vector_length);
            break;
        case FEEDBACK_VEC_STATUS_REPLY:
            ring_block_send_eth(FEEDBACK_ALIVE);
            break;
        default:
            fb_vector_default();
            break;
    }
}

void fb_stream_type_callback(unsigned int *header, unsigned int *body, unsigned int stream_length, unsigned int stype) {

    // mapper mover

    // I think for now I will ignore the stream type and treat them all the same
    unsigned int res = xorshift32(0, body, stream_length);

    ring_block_send_eth(res);

}

unsigned int release_pointer;
unsigned int release_count = 0;
unsigned int user_callbacks = 0;
unsigned int start_pause = 0;

void user_data_callback( 
    unsigned int *cpu_header,
    unsigned int dma_body,
    unsigned int this_chunk_length,
    unsigned int chunk_index,
    unsigned int total_chunks) {

    ring_block_send_eth(DEBUG_1_PCCMD | user_callbacks);

    unsigned int* cpu_body = REVERSE_VMEM_DMA_ADDRESS(dma_body);

    SET_REG(x3, 0xdeadbeff);
    SET_REG(x3, chunk_index);
    SET_REG(x3, total_chunks);
    // for(unsigned int i = 0; i < this_chunk_length; i++) {
    //     SET_REG(x4, cpu_body[i]);
    // }

    // if(user_callbacks == 1) {
    //     CSR_READ(TIMER_VALUE, start_pause);
    //     start_pause += 1000 + (simple_random()%7000);
    //     fb_pause_mapmov();
    //     SET_REG(x3, 0xaa000000);
    // }


    release_count++;
    release_pointer = dma_body;
    user_callbacks++;
}

void do_release() {
    if( release_count > 1 ) {
        // error we were given 2 pointers too fast and only recorded 1
    }

    if( release_count == 1 ) {
        release_count--;
        fb_release_userdata_dma_pointer(release_pointer);
    }
}

void seed_callback(unsigned int data) {
    simple_random_seed(data);
}

int main(void) {
    ring_block_send_eth(0xdeadbeef);
    // ring_block_send_eth((unsigned int)buf0_header);
    // ring_block_send_eth((unsigned int)buf0_body);

    release_count = 0;

    STALL(1000);

    fb_parse_setup();

    fb_register_vector_callback(&fb_vector_type_callback);
    fb_register_stream_callback(&fb_stream_type_callback);
    fb_register_mapmov_callback(&user_data_callback);

    ring_register_callback(&seed_callback, SEED_RANDOM_CMD);

    Ringbus r;

    unsigned int timer;

    while(1) {
        SET_REG(x3, 0x0000);
        pet_fb_parse();
        SET_REG(x3, 0x0000);
        do_release();
        check_ring(&r);

        CSR_READ(TIMER_VALUE, timer);

        // if( start_pause != 0 && timer > start_pause) {
        //     start_pause = 0;
        //     fb_unpause_mapmov();
        //     SET_REG(x3, 0xaa000001);
        // }

        // if the test is running long, always unpause
        if(timer < 0x13800)
        {
            unsigned int pull = simple_random() & 0xff;
            if( pull < 25 ) {
                unsigned int pull2 = simple_random() & 0xf;
                if( pull2 < 0x4) {
                    fb_unpause_mapmov();
                } else {
                    fb_pause_mapmov();
                }
            }
        } else {
            fb_unpause_mapmov();
        }
    }
}

