#include "ringbus.h"
#include "random.h"

// #define ENABLE_TB_DEBUG

#ifndef ENABLE_TB_DEBUG
#define DISABLE_TB_DEBUG
#endif
#include "tb_debug.h"

#define PING_PONG_DISABLE_RUN_TILL_LAST
#include "ping_pong_driver.h"

#define VECTOR_INITIAL_VALUE 0x2
#include "vmem_vector_1k.h"


unsigned seed_set = 0;

void seed_callback(unsigned int data) {
    simple_random_seed(data);
    seed_set = 1;
}

unsigned transform(const unsigned int index,
               const unsigned int* const cpu_in,
               unsigned int* const cpu_out);

int main(void) {
    ping_pong_cb_t transform_callback = transform;
    setup_debug();

    ring_register_callback(&seed_callback, SEED_RANDOM_CMD);

    Ringbus ringbus;
    while(seed_set == 0) {
        check_ring(&ringbus);
    }

    unsigned int random_int;
    unsigned int stall_max = 3500;
    unsigned int stall_min = 0;
    unsigned int mod_stall = (simple_random() % (stall_max - stall_min)) + stall_min;

    ping_pong_set_callback(transform_callback);
    setup_ping_pong();
    while (1) {
        execute_ping_pong();

        random_int = simple_random() % mod_stall;
        for (unsigned int i = 0; i < random_int; i++) {
            STALL(1);
        }
    }

    return 0;
}

unsigned transform(
                const unsigned int index,
                const unsigned int* const cpu_in,
                      unsigned int* const cpu_out
               ) {

    const unsigned input_row_addr = VMEM_ROW_ADDRESS(cpu_in);
    const unsigned output_row_addr = VMEM_ROW_ADDRESS(cpu_out);
    vector_add_1024(
                    input_row_addr,
                    VMEM_ROW_ADDRESS(vmem_values),
                    output_row_addr);
    STALL(50);

    return 1;
}
