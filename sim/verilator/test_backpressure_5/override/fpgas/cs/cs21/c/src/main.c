#include "ringbus.h"
#include "random.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#define ENABLE_TB_DEBUG

#ifndef ENABLE_TB_DEBUG
#define DISABLE_TB_DEBUG
#endif
#include "tb_debug.h"

// #define PING_PONG_ENABLE_PRINT

#define PING_PONG_DISABLE_RUN_TILL_LAST
#include "ping_pong_driver.h"

#define VECTOR_INITIAL_VALUE 0xf0000000
#include "vmem_vector_1k.h"


#define INCLUDE_VECTOR_AS vec1
#define VECTOR_INITIAL_VALUE 0x11111111
#include "vmem_vector_1k.h"

#define INCLUDE_VECTOR_AS vec2
#define VECTOR_INITIAL_VALUE 0x22222222
#include "vmem_vector_1k.h"

#define INCLUDE_VECTOR_AS vec3
#define VECTOR_INITIAL_VALUE 0x33333333
#include "vmem_vector_1k.h"



unsigned seed_set = 0;

void seed_callback(unsigned int data) {
    simple_random_seed(data);
    seed_set = 1;
}

unsigned transform(const unsigned int index,
               const unsigned int* const cpu_in,
               unsigned int* const cpu_out);

void ping_pong_did_pause(void);

unsigned pause_at = 0;
unsigned transforms = 0;
unsigned did_pause = 0;
unsigned pause_counter = 0;
unsigned send_when_paused = 0;

int main(void) {
    setup_debug();

    ring_register_callback(&seed_callback, SEED_RANDOM_CMD);

    Ringbus ringbus;
    while(seed_set == 0) {
        check_ring(&ringbus);
    }

    pause_at = (simple_random() % 4) + 1;
    // pause_at = 3;

    send_when_paused = (simple_random() % 4);
    // send_when_paused = 2;

    unsigned int pause_delay1 = 2 + (simple_random()%4);
    unsigned int pause_delay2 = pause_delay1 + 1 + (simple_random()%3);


    unsigned int random_int;
    unsigned int stall_max = 500;
    unsigned int stall_min = 0;
    unsigned int mod_stall = (simple_random() % (stall_max - stall_min)) + stall_min;


    ping_pong_set_callback(&transform);
    ping_pong_set_pause_callback(&ping_pong_did_pause);
    setup_ping_pong();
    while (1) {
        execute_ping_pong();

        random_int = simple_random() % mod_stall;
        for (unsigned int i = 0; i < random_int; i++) {
            STALL(1);
        }


        if( did_pause ) {
            pause_counter++;
        }

        if( pause_counter == pause_delay1 ) {
            if( send_when_paused >= 1) {
                dma_block_send(VMEM_DMA_ADDRESS(vec1), 1024);
            }
            if( send_when_paused >= 2) {
                dma_block_send(VMEM_DMA_ADDRESS(vec2), 1024);
            }
            if( send_when_paused >= 3) {
                dma_block_send(VMEM_DMA_ADDRESS(vec3), 1024);
            }
        }

        if( pause_counter > pause_delay2 ) {
            ping_pong_request_resume();
            did_pause = 0;
        }

    }

    return 0;
}


void ping_pong_did_pause(void) {
    ring_block_send_eth(EDGE_EDGE_OUT | ((transforms)&0xff) | ((send_when_paused&0xff)<<8) );
    _puts("did pause callback");
    did_pause = 1;
}

unsigned transform(
                const unsigned int index,
                const unsigned int* const cpu_in,
                      unsigned int* const cpu_out
               ) {

    _printf("%s%d\n", "transform ", transforms);
    // _printf("%s%d %d\n", "transform ", transforms, cpu_in[0]);

    const unsigned input_row_addr = VMEM_ROW_ADDRESS(cpu_in);
    const unsigned output_row_addr = VMEM_ROW_ADDRESS(cpu_out);
    vector_add_1024(
                    input_row_addr,
                    VMEM_ROW_ADDRESS(vmem_values),
                    output_row_addr);
    STALL(50);


    if( transforms == pause_at ) {
        _printf("%s%d\n", "requested pause at ", transforms);
        ping_pong_request_pause();
    }

    transforms++;

    return 1;
}
