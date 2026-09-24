
#include "xbaseband.h"
#include "vmem.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "circular_buffer_pow2.h"

#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"



#include "fill.h"
#include "vector_multiply.h"
#include "vmem_copy.h"
#include "random.h"
#include "circular_shift.h"
#include "handle_generic_op.h"

#include <stdint.h>
#include <stdbool.h>

#include "bootload_function.h"

// VMEM_SECTION_OFFSET_WORDS(16);


static VMEM_SECTION unsigned test_report_vmem[16];
static VMEM_SECTION unsigned vmem_buffer[16*12];


uint32_t duplex_mode = 0;
uint32_t duplex_progress = 0;
uint32_t tone_2_value = 0x00002000;

// length of our buffer, not all of vmem, in words
static uint32_t vmem_len = 16*12;


// we need a dma location to send stuff out to the testbench
#define VECTOR_REPORT_ADDRESS (VMEM_DMA_ADDRESS(test_report_vmem))

void setup_report(void) {
    CSR_WRITE(DMA_1_LENGTH, 1);
    CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);  // start right away
}

void report_test_results(const unsigned int data)
{
    static int offset = 0;
    // put the value of pass_fail_0 into the
    // vector memory at a high address
    vector_memory[VECTOR_REPORT_ADDRESS+offset] = data;
    // vector_report[0] = data;

    CSR_WRITE(DMA_1_START_ADDR, VECTOR_REPORT_ADDRESS+offset);
    CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0); // any values

    offset = (offset+1)&0xf;
}

typedef uint32_t (*bl_fun_t)(uint32_t, uint32_t);

BLFUN_VARIABLE_SECTION uint32_t bl_memory;

BLFUN_SECTION uint32_t blfun_zero(uint32_t a0, uint32_t a1) {
    (void)a1;
    if( a0 % 2 == 0) {
        return a0 * 4;
    }
    if( a0 == 3 ) {
        return 7;
    }

    return 0;
}

BLFUN_SECTION uint32_t blfun_one(uint32_t a0, uint32_t a1) {
    (void)a1;

    // if( a0 == bl_memory) {
    //     bl_memory++;
    // } else {
    bl_memory = a0;
    // }

    return 0;
}

BLFUN_SECTION uint32_t blfun_two(uint32_t a0, uint32_t a1) {
    (void)a0;
    (void)a1;
    return bl_memory;
}

BLFUN_SECTION uint32_t blfun_three(uint32_t a0, uint32_t a1) {
    (void)a0;
    (void)a1;

    uint32_t *p = 0;

    uint32_t sel = a0 & 0xf;
    uint32_t op = (a0 >> 8) & 0xf;

    switch(sel) {
        case 0:
            p = &duplex_mode;
            break;
        case 1:
            p = &duplex_progress;
            break;
        case 2:
            p = &tone_2_value;
            break;
    }

    if( p == 0 ) {
        return 0;
    }

    switch(op) { 
        case GENERIC_OP_SET:
            *p = a1;
            break;
        case GENERIC_OP_GET:
            return *p;
            break;
    }

    return 0;
}

// fixme add a table in normal memory

// this should live in normal memory
uint32_t blfun_entry(uint32_t sel, uint32_t a0, uint32_t a1) {


    bl_fun_t f0 = &blfun_zero;
    bl_fun_t f1 = &blfun_one;
    bl_fun_t f2 = &blfun_two;
    bl_fun_t f3 = &blfun_three;


    switch(sel) {
        case 0:
            return f0(a0, a1);
            break;
        case 1:
            return f1(a0, a1);
            break;
        case 2:
            return f2(a0, a1);
            break;
        case 3:
            return f3(a0, a1);
            break;
    }

    return 0;
}









#define MY_ASSERT(x) if(!(x)) {return __LINE__;}



int test1(void) {
    STALL(2);

    MY_ASSERT(4==4);


    MY_ASSERT(blfun_entry(0,0,0) == 0);
    MY_ASSERT(blfun_entry(0,3,0) == 7);

    MY_ASSERT(blfun_entry(0,15,0) == 0);

    return 0;
}

int test2(void) {

    MY_ASSERT(4==4);

    const uint32_t val = 234234;

    MY_ASSERT(blfun_entry(1,val,0) == 0);
    MY_ASSERT(blfun_entry(2,0,0) == val);

    return 0;
}


// glue two 8 bit numbers together
static uint32_t selop(uint32_t sel, uint32_t op) {
    return (sel&0xf) | ((op&0xf)<<8);
}


int test3(void) {


    duplex_mode = 2;
    duplex_progress = 0x2b;
    tone_2_value = 0x4000;



    uint32_t a = blfun_entry(0, selop(2, GENERIC_OP_GET), 0);

    report_test_results(a);




    return 0;
}







unsigned seed_set = 0;

void seed_callback(unsigned int data) {
    simple_random_seed(data);
    seed_set = 1;
}



#include "eth_unit_test_driver.h"


int main(void)
{
    unsigned int all_results = 0;
    unsigned int test = 0;

    setup_dma_in();

    setup_report();

    while(seed_set == 0) {
        // handle_pending_ring();
        handle_single_dma_in();
    }


    report_test_results(0xdeadbeef);

    unsigned run_times[32];


    int (*all_tests[32])(void) = {
         // &test1
        // &test2
        &test3
        // ,&test3
        // ,&test4
        // &test5
        // &test9_slow
        // &test6_1
        // ,&test6_2
        // &test6_random
        // ,&test7
        // &test8
    };

    unsigned start = 0;
    unsigned end = 0;

    unsigned count_valid = 0;
    for(unsigned i = 0; i < 32; i++) {
        if( all_tests[i] == 0) {
            count_valid = i;
            break;
        }
    }

    report_test_results(0x60000000 | count_valid);

    for(unsigned i = 0; i < count_valid; i++) {
        CSR_READ(TIMER_VALUE, start);
        test = all_tests[i]();
        CSR_READ(TIMER_VALUE, end);
        run_times[i] = end-start;
        all_results |= (test==0)<<i;
    }

    report_test_results(0x70000000 | test);
    report_test_results(0x80000000 | all_results);

    for(unsigned i = 0; i < count_valid; i++) {
        report_test_results(0x90000000 | run_times[i]);
    }


    return 0;
}
