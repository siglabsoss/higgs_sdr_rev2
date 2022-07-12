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
        case FEEDBACK_VEC_SCHEDULE:
            ring_block_send_eth(0xdeadfeed);
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

void user_data_callback(
    unsigned int *cpu_header,
    unsigned int dma_body,
    unsigned int this_chunk_length,
    unsigned int chunk_index,
    unsigned int total_chunks) {

    fb_release_userdata_dma_pointer(dma_body);

}

int main(void) {
    ring_block_send_eth(0xdeadbeef);
    // ring_block_send_eth((unsigned int)buf0_header);
    // ring_block_send_eth((unsigned int)buf0_body);

    STALL(1000);

    fb_parse_setup();

    fb_register_vector_callback(&fb_vector_type_callback);
    fb_register_stream_callback(&fb_stream_type_callback);
    fb_register_mapmov_callback(&user_data_callback);

    Ringbus r;

    while(1) {
        SET_REG(x3, 0x0000);
        pet_fb_parse();
        SET_REG(x3, 0x0000);
        check_ring(&r);

    }

}

