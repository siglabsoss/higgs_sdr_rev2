
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

#include <stdint.h>
#include <stdbool.h>

// VMEM_SECTION_OFFSET_WORDS(16);


static VMEM_SECTION unsigned test_report_vmem[16];
static VMEM_SECTION unsigned vmem_buffer[16*12];


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

#define MY_ASSERT(x) if(!(x)) {return __LINE__;}


// insert 4 things, remove directly, (does a remove at bottom to verify fail), test flags along the way, and values as they come out
// at the end once empty, quicky fills.
// does an add when full, checks error, trains to verfy over add didn't go
int test777(void) {

    for(unsigned i = 0; i < vmem_len; i++) {
        vmem_buffer[i] = i+1;
        // SET_REG(x3,i);
    }

    for(unsigned i = 0; i < vmem_len; i++) {
        MY_ASSERT(vmem_buffer[i] == i+1);
    }


    return 0;
}

static VMEM_SECTION unsigned fill_buf[16*3];

int test1(void) {
    unsigned row = VMEM_ROW_ADDRESS(fill_buf);

    unsigned len = ARRAY_SIZE(fill_buf);

    unsigned value = 0x3203;

    vmem_fill_low(row, 3, value);

    for(unsigned i = 0; i < len; i++) {
        MY_ASSERT(fill_buf[i] == value);
    }

    return 0;
}

static VMEM_SECTION unsigned fill_buf2[16*4];
static VMEM_SECTION unsigned fill_work[16*3];

int test2(void) {
    unsigned row = VMEM_ROW_ADDRESS(fill_buf2);
    unsigned work_row = VMEM_ROW_ADDRESS(fill_work);

    unsigned len = ARRAY_SIZE(fill_buf2);

    unsigned value = 0xf13203;

    // unsigned start;
    // unsigned end;


    // CSR_READ(TIMER_VALUE, start);
    vmem_fill_prep(work_row, value);
    vmem_fill_run(work_row, row, 2);

    // CSR_READ(TIMER_VALUE, end);

    // report_test_results(0x5);
    // report_test_results(0x5);
    // report_test_results(end-start);


    // for(unsigned i = 0; i < len; i++) {
    //     report_test_results(fill_buf2[i]);
    // //     MY_ASSERT(fill_buf2[i] == value);
    // }

    for(unsigned i = 0; i < 16*2; i++) {
        MY_ASSERT(fill_buf2[i] == value);
    }
    for(unsigned i = 16*2; i < 16*3; i++) {
        MY_ASSERT(fill_buf2[i] == 0);
    }

    return 0;
}

static VMEM_SECTION unsigned t3_buf[1024+16];

#define INCLUDE_COUNTER_AS t3_counter
#define INCLUDE_COUNTER_BASE 0xff000000
#include "vmem_counter_1k.h"

int test3(void) {

    // takes 40 us
    // for(unsigned i = 0; i < 1024; i++) {
    //     MY_ASSERT(t3_buf[i] == 0);
    // }

    for(unsigned i = 0; i < 1024; i++) {
        MY_ASSERT(t3_counter[i] == 0xff000000+i);
    }


    vmem_copy_rows(
        VMEM_ROW_ADDRESS(t3_counter),
        VMEM_ROW_ADDRESS(t3_buf),
        32
        );

    // check copy went ok
    for(unsigned i = 0; i < 512; i++) {
        MY_ASSERT(t3_buf[i] == 0xff000000+i);
    }

    // check copy didn't run over
    for(unsigned i = 512; i < 512+16; i++) {
        MY_ASSERT(t3_buf[i] == 0);
    }

    return 0;
}

#define INCLUDE_COUNTER_AS t4_counter
#define INCLUDE_COUNTER_BASE 0xfd000000
#include "vmem_counter_1k.h"

static VMEM_SECTION unsigned t4_buf[512+16];

int test4(void) {

    unsigned s0 = 1+simple_random()%14;
    unsigned s1 = 1+simple_random()%17;
    unsigned s2 = 1+simple_random()%27;

    vmem_copy_words_slow(
        VMEM_DMA_ADDRESS(t4_counter) + s0,
        VMEM_DMA_ADDRESS(t4_buf) + s1,
        s2
        );

    // for( unsigned i = s1-1; i < s1+s2+1; i++) {
    //     report_test_results(t4_buf[i]);
    // }

    for( unsigned i = s1; i < s1+s2; i++) {
        // report_test_results(0xfd000000+i+7-2);
        MY_ASSERT(t4_buf[i] == (0xfd000000+i+s0-s1) );
        // report_test_results(t4_buf[i]);
    }

    // report_test_results(0);
    // report_test_results(simple_random());
    // report_test_results(0);
    return 0;
}


#define INCLUDE_COUNTER_AS t5_counter
#define INCLUDE_COUNTER_BASE 0xfc000000
#include "vmem_counter_1k.h"

#define INCLUDE_VECTOR_AS t5_buf
#define INCLUDE_FIXED_VALUE 7
#include "vmem_fixed_1k.h"
// static VMEM_SECTION unsigned t5_buf[512+16];

VMEM_SECTION unsigned t5_garbage[16];

/**********************************************************************************
 **********************************************************************************
 **********************************************************************************
 **********************************************************************************
 **********************************************************************************
 **********************************************************************************
 *
 **********************************************************************************/

unsigned compare_s0 = 7;
unsigned compare_s1 = 39;
unsigned compare_s2 = 45;


int test5(void) {

    const unsigned int garbage_row = VMEM_ROW_ADDRESS(t5_garbage);

    // shift source
    unsigned s0 = compare_s0; //1+simple_random()%14;
    // shift dest
    unsigned s1 = compare_s1; //1+simple_random()%17;

    // length
    unsigned s2 = compare_s2; //1+simple_random()%27;

    bool ok = vmem_copy_words(
        VMEM_DMA_ADDRESS(t5_counter) + s0,
        VMEM_DMA_ADDRESS(t5_buf) + s1,
        s2,
        garbage_row
    );


    report_test_results(1);
    for( unsigned i = 0; i < s1+s2+16; i++) {
        report_test_results(t5_buf[i]);
    }
    report_test_results(1);

    // for( unsigned i = s1; i < s1+s2; i++) {
    //     MY_ASSERT(t5_buf[i] == (0xfc000000+i+s0-s1) );
    // }

    MY_ASSERT(ok);

    return 0;
}

#define INCLUDE_COUNTER_AS t5_counter_s
#define INCLUDE_COUNTER_BASE 0xfc000000
#include "vmem_counter_1k.h"

#define INCLUDE_VECTOR_AS t5_buf_s
#define INCLUDE_FIXED_VALUE 7
#include "vmem_fixed_1k.h"

// same as test five
int test9_slow(void) {
    unsigned s0 = compare_s0; //1+simple_random()%14;
    // shift dest
    unsigned s1 = compare_s1; //1+simple_random()%17;

    // length
    unsigned s2 = compare_s2; //1+simple_random()%27;

    vmem_copy_words_slow(
        VMEM_DMA_ADDRESS(t5_counter_s) + s0,
        VMEM_DMA_ADDRESS(t5_buf_s) + s1,
        s2
    );

    report_test_results(1);
    for( unsigned i = 0; i < s1+s2+16; i++) {
        report_test_results(t5_buf_s[i]);
    }
    report_test_results(1);

    return 0;

}



#define INCLUDE_COUNTER_AS t6_counter
#define INCLUDE_COUNTER_BASE 0xf3000000
#include "vmem_counter_1k.h"

#define INCLUDE_VECTOR_AS t6_buf_s
#define INCLUDE_FIXED_VALUE 77
#include "vmem_fixed_1k.h"


#define INCLUDE_VECTOR_AS t6_buf_f
#define INCLUDE_FIXED_VALUE 77
#include "vmem_fixed_1k.h"
// static VMEM_SECTION unsigned t5_buf[512+16];

VMEM_SECTION unsigned t6_garbage[16];


int test6_base(unsigned s0, unsigned s1, unsigned s2) {

    const unsigned int garbage_row = VMEM_ROW_ADDRESS(t6_garbage);

    vmem_fill_low(garbage_row, 64, 0);
    vmem_fill_low(VMEM_ROW_ADDRESS(t6_buf_s), 64, 77);
    vmem_fill_low(VMEM_ROW_ADDRESS(t6_buf_f), 64, 77);


    bool ok = vmem_copy_words(
        VMEM_DMA_ADDRESS(t6_counter) + s0,
        VMEM_DMA_ADDRESS(t6_buf_f) + s1,
        s2,
        garbage_row
    );

    vmem_copy_words_slow(
        VMEM_DMA_ADDRESS(t6_counter) + s0,
        VMEM_DMA_ADDRESS(t6_buf_s) + s1,
        s2
    );


    for( unsigned i = 0; i < s1+s2+48; i++) {
        MY_ASSERT(t6_buf_s[i] == t6_buf_f[i]);
    }



    return 0;
}

int test6_1(void) {

    // shift source
    unsigned s0 = 0; //1+simple_random()%14;
    // shift dest
    unsigned s1 = 15; //1+simple_random()%17;

    // length
    unsigned s2 = 19+16; //1+simple_random()%27;
    (void)s0;
    (void)s1;
    (void)s2;

    return test6_base(0,15,35);
}

int test6_2(void) {
    return test6_base(0,15,32);
}

int test6_random(void) {

    unsigned s0 = 0; //1+simple_random()%14;
    // shift dest
    unsigned s1 = 15; //1+simple_random()%17;

    // length
    unsigned s2 = 19+16; //1+simple_random()%27;

    unsigned runs = 4*6+2;

    // runs = 4;

    int error = 0;

    for(unsigned i = 0; i < runs; i++) {
        // shift source
        s0 = simple_random()%34;

        // shift dest
        s1 = simple_random()%55;

        // length
        s2 = simple_random()%65;


        // s0 = 7;
        // s1 = 39;
        // s2 = 45;


        

        error = test6_base(s0, s1, s2);
        if( error ) {
            report_test_results(0x1337);
            report_test_results(i);
            report_test_results(0x1338);
            report_test_results(s0);
            report_test_results(s1);
            report_test_results(s2);
            return error;
        }
    }

    return error;
}



#define INCLUDE_COUNTER_AS t7_counter
#define INCLUDE_COUNTER_BASE 0xcc000000
#include "vmem_counter_1k.h"

static VMEM_SECTION unsigned t7_buf[512+16];

int test7(void) {

    const unsigned int input_rows = VMEM_ROW_ADDRESS(t7_counter);
    const unsigned int output_rows = VMEM_ROW_ADDRESS(t7_buf);
    const unsigned int rows = 1;


    const uint16_t l_perm = 0x0000  & 0xf000;
    const uint16_t s_perm = 0x1000  & 0xf000;

    MVXV_KNOP(V7, l_perm);
    MVXV_KNOP(V8, s_perm);

    MVXV_KNOP(V2, input_rows);
    MVXV_KNOP(V3, output_rows);
    MVXV_KNOP(V4, 0x1);

    ADD_KNOP(V2, V2, V7);
    ADD_KNOP(V3, V3, V8);


    for (unsigned int i = 0; i < rows; i++){
        ADD_LK13(V2, V2, V4, 0x0);
        ADD_SK13(V3, V3, V4, 0x0);
    }

    report_test_results(0);
    for( unsigned i = 0; i < 16; i++) {
        report_test_results(t7_buf[i]);
    }
    report_test_results(0);

    return 0;
}


int test8(void) {

    const uint16_t v = 0xffef;
    
    MY_ASSERT(rol16(v,4) == 0xfeff);

    const uint16_t v2 = 0xffef;
    
    MY_ASSERT(ror16(v2,4) == 0xfffe);

    report_test_results(ror16(v2,4));

    const uint16_t v3 = 0xff0f;

    MY_ASSERT(ror16(v3,1) == 0xff87);

    MY_ASSERT(rol16(ror16(v3,1),2) == 0xfe1f);

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
        // ,&test2
        // ,&test3
        // ,&test4
        // &test5
        // &test9_slow
        // &test6_1
        // ,&test6_2
        &test6_random
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
