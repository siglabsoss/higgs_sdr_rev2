
#include "xbaseband.h"
#include "vmem.h"
#include "csr_control.h"
#include "bootloader.h"
#include "pass_fail.h"
#include "circular_buffer_pow2.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"











#define REPORT_CIRBUF4(yy) \
report_test_results(0xcafe); \
report_test_results(yy.head); \
report_test_results(yy.tail); \
report_test_results(circular_buf2_full(&yy)); \
report_test_results(circular_buf2_empty(&yy)); \
report_test_results(circular_buf2_occupancy(&yy)); \
report_test_results(yy.buffer[0]); \
report_test_results(yy.buffer[1]); \
report_test_results(yy.buffer[2]); \
report_test_results(yy.buffer[3]); \
report_test_results(yy.buffer[4]);

#define REPORT_CIRBUF(yy) \
report_test_results(0xcafe); \
report_test_results(yy.head); \
report_test_results(yy.tail); \
report_test_results(circular_buf2_full(&yy)); \
report_test_results(circular_buf2_empty(&yy)); \
report_test_results(circular_buf2_occupancy(&yy));

#define REP() REPORT_CIRBUF(dma_inbuf)











// call this ONLY ONE TIME per project to offset the beginning of vmem allocation
VMEM_SECTION_OFFSET_WORDS(0);

// we need a dma location to send stuff out to the testbench
#define VECTOR_REPORT_ADDRESS (4096-16)

void report_test_results(unsigned int data)
{
  static int offset = 0;
  // put the value of pass_fail_0 into the
  // vector memory at a high address
  vector_memory[VECTOR_REPORT_ADDRESS+offset] = data;
  // vector_report[0] = data;

  CSR_WRITE(DMA_1_START_ADDR, VECTOR_REPORT_ADDRESS+offset);
  CSR_WRITE(DMA_1_LENGTH, 1);
  CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);  // start right away
  CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0); // any values

  offset = (offset+1)&0x3;
}

#define DMA_IN_COUNT (4)

// circular_buf_t dma_inbuf;
// unsigned int dma_inbuf_storage[DMA_IN_COUNT+1];


// unsigned int* dma_in_ptr[DMA_IN_COUNT];

// this is a half macro which creates a hidden storage array
// the result is that this line actually makes 2 variables
// this will hang forever (if non pow2 is used)
// (it hands later in CIRBUF_POW2_RUNTIME_INITIALIZE )
// passing 0 as second argument will also assert-hang
circular_buf_pow2_t dma_inbuf = CIRBUF_POW2_STATIC_CONSTRUCTOR(dma_inbuf, 4);
// probably never use this one:
// circular_buf_pow2_t dma_inbuf = CIRBUF_POW2_STATIC_CONSTRUCTOR_UNSAFE(dma_inbuf, 4);


void setup_dma_in(void) {
    CIRBUF_POW2_RUNTIME_INITIALIZE(dma_inbuf);
}

#define MY_ASSERT(x) if(!(x)) {return __LINE__;}


int test5(void) {

    unsigned int res;

    res = circular_buf2_get_pow2(1);  MY_ASSERT(res == 0);
    
    res = circular_buf2_get_pow2(2);  MY_ASSERT(res == 1);

    res = circular_buf2_get_pow2(349);  MY_ASSERT(res == 0xffffffff);

    res = circular_buf2_get_pow2(1024);  MY_ASSERT(res == 10);


    return 0;

}


circular_buf_pow2_t __dma_schedule_in = CIRBUF_POW2_STATIC_CONSTRUCTOR(__dma_schedule_in, 4);
circular_buf_pow2_t* dma_schedule_in = &__dma_schedule_in;

int test6(void) {

    CIRBUF_POW2_RUNTIME_INITIALIZE(__dma_schedule_in);


    // REPORT_CIRBUF(__dma_schedule_in);

    // put zero
    circular_buf2_put(dma_schedule_in, 0);

    // REPORT_CIRBUF(__dma_schedule_in); 

    int error;

    unsigned int data;

    // get it
    error = circular_buf2_get(&dma_inbuf, &data);
    MY_ASSERT(error == 0);
    MY_ASSERT(data == 0);

    // REPORT_CIRBUF(__dma_schedule_in);

    // put a 2
    circular_buf2_put(dma_schedule_in, 2);


    // REPORT_CIRBUF(__dma_schedule_in);

    // get it
    error = circular_buf2_get(&dma_inbuf, &data);
    // MY_ASSERT(error == 0);
    // MY_ASSERT(data == 2);


    // REPORT_CIRBUF(__dma_schedule_in);


    return 0;

}

// insert 4 things, remove directly, (does a remove at bottom to verify fail), test flags along the way, and values as they come out
// at the end once empty, quicky fills.
// does an add when full, checks error, trains to verfy over add didn't go
int test0(void) {

    int res;

    // CIRBUF_POW2_RUNTIME_INITIALIZE(dma_inbuf);
    setup_dma_in();

    int error = 0;
    int empty = 0;
    int full = 0;

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 1);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf2_put(&dma_inbuf, 2);
    // report_test_results(dma_inbuf.occupancy);
    // report_test_results(error);
    MY_ASSERT(error == 0);

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf2_put(&dma_inbuf, 4);
    // report_test_results(dma_inbuf.occupancy);
    MY_ASSERT(error == 0);

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf2_put(&dma_inbuf, 6);
    // report_test_results(dma_inbuf.occupancy);
    MY_ASSERT(error == 0);

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf2_put(&dma_inbuf, 8);
    // report_test_results(dma_inbuf.occupancy);
    MY_ASSERT(error == 0);

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 1);

    unsigned int data;

    error = circular_buf2_get(&dma_inbuf, &data);
    MY_ASSERT(error == 0);
    MY_ASSERT(data == 2);

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf2_get(&dma_inbuf, &data);
    MY_ASSERT(error == 0);
    MY_ASSERT(data == 4);

    error = circular_buf2_get(&dma_inbuf, &data);
    MY_ASSERT(error == 0);
    MY_ASSERT(data == 6);

    error = circular_buf2_get(&dma_inbuf, &data);
    MY_ASSERT(error == 0);
    MY_ASSERT(data == 8);

    // get past what we put in
    error = circular_buf2_get(&dma_inbuf, &data);
    MY_ASSERT(error != 0);

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 1);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);
    // MY_ASSERT(data == 8);

    // at this point the buffer is EMPTY.  we want it full quickly

    circular_buf2_put(&dma_inbuf, 9);
    report_test_results(dma_inbuf.occupancy);

    circular_buf2_put(&dma_inbuf, 10);
    report_test_results(dma_inbuf.occupancy);

    circular_buf2_put(&dma_inbuf, 11);
    report_test_results(dma_inbuf.occupancy);

    circular_buf2_put(&dma_inbuf, 12);
    report_test_results(dma_inbuf.occupancy);

    // make sure it's full
    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 1);

    MY_ASSERT(dma_inbuf.occupancy == 4);

    // test some adds after it's full for error codes

    // should fail
    error = circular_buf2_put(&dma_inbuf, 0xffff); MY_ASSERT(error != 0);

    // should still be at 4
    MY_ASSERT(dma_inbuf.occupancy == 4);

    // check that our failed insert didn't actually go

    error = circular_buf2_get(&dma_inbuf, &data);
    MY_ASSERT(error == 0);
    MY_ASSERT(data == 9);

    error = circular_buf2_get(&dma_inbuf, &data);
    MY_ASSERT(error == 0);
    MY_ASSERT(data == 10);

    error = circular_buf2_get(&dma_inbuf, &data);
    MY_ASSERT(error == 0);
    MY_ASSERT(data == 11);

    error = circular_buf2_get(&dma_inbuf, &data);
    MY_ASSERT(error == 0);
    MY_ASSERT(data == 12);

    // final empty/full
    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 1);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);


  return 0;
}


// test add 3, remove 2, add 2, remove 3
int test1(void) {

    setup_dma_in();

    int error = 0;
    unsigned int data, empty, full;

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 1);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf2_put(&dma_inbuf, 2); MY_ASSERT(error == 0);
    error = circular_buf2_put(&dma_inbuf, 4); MY_ASSERT(error == 0);
    error = circular_buf2_put(&dma_inbuf, 6); MY_ASSERT(error == 0);

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);


    error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 2);

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 4);

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf2_put(&dma_inbuf, 20); MY_ASSERT(error == 0);
    error = circular_buf2_put(&dma_inbuf, 22); MY_ASSERT(error == 0);


    // ((dma_inbuf.head + 1) % dma_inbuf.size) == dma_inbuf.tail;

    // cap should be 3 here

    error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 6);

    error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 20);

    error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 22);

    // should be empty
    error = circular_buf2_get(&dma_inbuf, &data);
    MY_ASSERT(error != 0);

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 1);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);

  return 0;
}



// int test_debug() {

//     setup_dma_in();

//     int error = 0;
//     unsigned int data;

//     REP();
//     error = circular_buf2_put(&dma_inbuf, 2); MY_ASSERT(error == 0);
//     REP();
//     error = circular_buf2_put(&dma_inbuf, 4); MY_ASSERT(error == 0);
//     REP();
//     error = circular_buf2_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
//     REP();
//     error = circular_buf2_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
//     REP();
//     error = circular_buf2_put(&dma_inbuf, 6); MY_ASSERT(error != 0);
//     REP();
//     error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
//     // REP();
//     error = circular_buf2_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
//     REP();
//     error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
//     // REP();
//     error = circular_buf2_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
//     REP();
//     error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
//     // REP();
//     error = circular_buf2_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
//     REP();
//     error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
//     // REP();
//     error = circular_buf2_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
//     REP();
//     error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
//     // REP();
//     error = circular_buf2_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
//     REP();

// #ifdef asdf


//     error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
//     MY_ASSERT(data == 2);

//     error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
//     MY_ASSERT(data == 4);

//     error = circular_buf2_put(&dma_inbuf, 20); MY_ASSERT(error == 0);
//     error = circular_buf2_put(&dma_inbuf, 22); MY_ASSERT(error == 0);


//     // ((dma_inbuf.head + 1) % dma_inbuf.size) == dma_inbuf.tail;

//     // cap should be 3 here

//     error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
//     MY_ASSERT(data == 6);

//     error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
//     MY_ASSERT(data == 20);

//     error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
//     MY_ASSERT(data == 22);

//     // should be empty
//     error = circular_buf2_get(&dma_inbuf, &data);
//     MY_ASSERT(error != 0);
// #endif


//   return 0;
// }

int test2(void) {

    setup_dma_in();

    int error = 0;
    unsigned int data, empty, full, occupancy;

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 1);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);
    error = circular_buf2_peek(&dma_inbuf, &data); MY_ASSERT(error == -1);

    error = circular_buf2_put(&dma_inbuf, 2); MY_ASSERT(error == 0);
    error = circular_buf2_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 2);

    error = circular_buf2_put(&dma_inbuf, 4); MY_ASSERT(error == 0);
    error = circular_buf2_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 2);

    error = circular_buf2_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
    error = circular_buf2_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 2);



    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);


    error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 2);
    error = circular_buf2_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 4);


    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);


    error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 4);
    error = circular_buf2_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 6);

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf2_put(&dma_inbuf, 20); MY_ASSERT(error == 0);
    error = circular_buf2_put(&dma_inbuf, 22); MY_ASSERT(error == 0);

    error = circular_buf2_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 6);

    // ((dma_inbuf.head + 1) % dma_inbuf.size) == dma_inbuf.tail;

    // cap should be 3 here

    error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 6);
    error = circular_buf2_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 20);

    error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 20);
    error = circular_buf2_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 22);

    error = circular_buf2_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 22);

    // peek should error when empty
    error = circular_buf2_peek(&dma_inbuf, &data); MY_ASSERT(error != 0);

    // should be empty
    error = circular_buf2_get(&dma_inbuf, &data);
    MY_ASSERT(error != 0);

    empty = circular_buf2_empty(&dma_inbuf); MY_ASSERT(empty == 1);
    full = circular_buf2_full(&dma_inbuf); MY_ASSERT(full == 0);

  return 0;
}

// torture test circular_buf2_occupancy
int test3(void) {
    int error = 0;
    unsigned int data, empty, full, occupy;

    unsigned int cap = DMA_IN_COUNT;
    // pre, how many to add to start with
    // add, how many to add and then check
    // sub, how many to then sub, and then check
    //
    // pre, add are the numbers we will use for the test
    // they are run in further loops below
    for(unsigned int pre = 0; pre < 3; pre++) {
      for(unsigned int add = 0; add < (cap - pre); add++) {
        for(int sub = -1; sub < (signed)(pre+add); sub++) {

          // report_test_results(0xa0000000 | (pre << 16) | (add<<8) | (sub&0xff));

          setup_dma_in();

          // check empty
          occupy = circular_buf2_occupancy(&dma_inbuf);
          MY_ASSERT(occupy == 0);

          // add pre items
          for(unsigned int i = 0; i < pre; i++) {
            circular_buf2_put(&dma_inbuf, i);
          }

          // check if we are at the correct fill
          occupy = circular_buf2_occupancy(&dma_inbuf);
          MY_ASSERT(occupy == pre);

          // add `add` more items
          for(unsigned int i = 0; i < add; i++) {
            circular_buf2_put(&dma_inbuf, i);

            occupy = circular_buf2_occupancy(&dma_inbuf);
            MY_ASSERT(occupy == (pre+i+1)); // checking after i runs
          }

          // check outside loop
          occupy = circular_buf2_occupancy(&dma_inbuf);
          MY_ASSERT(occupy == (pre+add));

          // sub `sub` items
          for(int i = 0; i < sub; i++) {
            circular_buf2_get(&dma_inbuf, &data);

            occupy = circular_buf2_occupancy(&dma_inbuf);
            MY_ASSERT(occupy == (pre+add-i-1)); // checking after i runs
          }

        }
      }
    }


  return 0;
}


circular_buf_pow2_t dma_in_queue = CIRBUF_POW2_STATIC_CONSTRUCTOR(dma_in_queue, 4);

int test4(void) {
   CIRBUF_POW2_RUNTIME_INITIALIZE(dma_in_queue);

   // REPORT_CIRBUF(dma_in_queue);

   // report_test_results(0xea1);

  int error;
  unsigned int data;
  unsigned int dma_occupancy;
  unsigned int filled;

  filled = circular_buf2_occupancy(&dma_in_queue);
  MY_ASSERT(filled == 0);


  circular_buf2_put(&dma_in_queue, 0);
  filled = circular_buf2_occupancy(&dma_in_queue);
  MY_ASSERT(filled == 1);


  circular_buf2_put(&dma_in_queue, 1);
  filled = circular_buf2_occupancy(&dma_in_queue);
  MY_ASSERT(filled == 2);


  circular_buf2_put(&dma_in_queue, 2);
  filled = circular_buf2_occupancy(&dma_in_queue);
  MY_ASSERT(filled == 3);


  circular_buf2_put(&dma_in_queue, 3);
  filled = circular_buf2_occupancy(&dma_in_queue);
  MY_ASSERT(filled == 4);


  circular_buf2_put(&dma_in_queue, 4); // should get rejected
  filled = circular_buf2_occupancy(&dma_in_queue);
  MY_ASSERT(filled == 4);


  error = circular_buf2_get(&dma_in_queue, &data); //  report_test_results(data); report_test_results(error);

  MY_ASSERT(data == 0);

  error = circular_buf2_get(&dma_in_queue, &data); //  report_test_results(data); report_test_results(error);

  MY_ASSERT(data == 1);

  error = circular_buf2_get(&dma_in_queue, &data); //  report_test_results(data); report_test_results(error);
  MY_ASSERT(data == 2);

  error = circular_buf2_get(&dma_in_queue, &data); //  report_test_results(data); report_test_results(error);
  MY_ASSERT(data == 3);


  filled = circular_buf2_occupancy(&dma_in_queue);
  MY_ASSERT(filled == 0);

  return 0;
}

static VMEM_SECTION unsigned junk_mem_a[32];

// should be in it's own file but that's ok
int test_vmem_util(void) {

    unsigned dma_addr = 64;
    unsigned row_addr = 27;


    unsigned a0 = VMEM_ADDRESS((junk_mem_a+32));
    unsigned b0 = VMEM_ADDRESS(junk_mem_a+32);
    MY_ASSERT(a0 == b0);

    unsigned a1 = VMEM_ROW_ADDRESS((junk_mem_a+32));
    unsigned b1 = VMEM_ROW_ADDRESS(junk_mem_a+32);
    MY_ASSERT(a1 == b1);

    unsigned a2 = REVERSE_VMEM_ADDRESS((junk_mem_a+32));
    unsigned b2 = REVERSE_VMEM_ADDRESS(junk_mem_a+32);
    MY_ASSERT(a2 == b2);

    unsigned a3 = REVERSE_VMEM_ROW_ADDRESS((junk_mem_a+32));
    unsigned b3 = REVERSE_VMEM_ROW_ADDRESS(junk_mem_a+32);
    MY_ASSERT(a3 == b3);

    unsigned a4 = VMEM_DMA_ADDRESS((junk_mem_a+32));
    unsigned b4 = VMEM_DMA_ADDRESS(junk_mem_a+32);
    MY_ASSERT(a4 == b4);

    unsigned a5 = REVERSE_VMEM_DMA_ADDRESS((dma_addr+32));
    unsigned b5 = REVERSE_VMEM_DMA_ADDRESS(dma_addr+32);
    MY_ASSERT(a5 == b5);

    unsigned a6 = VMEM_DMA_ADDRESS_TO_ROW((dma_addr+32));
    unsigned b6 = VMEM_DMA_ADDRESS_TO_ROW(dma_addr+32);
    MY_ASSERT(a6 == b6);

    unsigned a7 = VMEM_ROW_ADDRESS_TO_DMA((dma_addr+32));
    unsigned b7 = VMEM_ROW_ADDRESS_TO_DMA(dma_addr+32);
    MY_ASSERT(a7 == b7);


    return 0;
}



int main(void)
{
    unsigned int all_results = 0;
    unsigned int test = 0;

    report_test_results(0xdeadbeef);

    int (*all_tests[32])(void) = {
         &test0
        ,&test1
        ,&test2
        ,&test3
        ,&test4
        ,&test5
        ,&test6
        ,&test_vmem_util
    };

    unsigned count_valid = 0;
    for(unsigned i = 0; i < 32; i++) {
        if( all_tests[i] == 0) {
            count_valid = i;
            break;
        }
    }

    report_test_results(0x60000000 | count_valid);

    for(unsigned i = 0; i < count_valid; i++) {
        test = all_tests[i]();
        all_results |= (test==0)<<i;
    }

    // test = test0();
    // all_results |= (test==0)<<0;

    // test = test1();
    // all_results |= (test==0)<<1;

    // test = test2();
    // all_results |= (test==0)<<2;

    // test = test3();
    // all_results |= (test==0)<<3;

    // test = test4();
    // all_results |= (test==0)<<4;

    // test = test5();
    // all_results |= (test==0)<<5;

    // test = test6();
    // all_results |= (test==0)<<6;


    report_test_results(0x70000000 | test);
    report_test_results(0x80000000 | all_results);

    return 0;
}
