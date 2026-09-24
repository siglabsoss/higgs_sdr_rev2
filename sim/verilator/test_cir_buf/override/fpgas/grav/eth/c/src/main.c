
#include "xbaseband.h"
#include "vmem.h"
#include "csr_control.h"
#include "bootloader.h"
#include "pass_fail.h"
#include "circular_buffer.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_ETH
#include "ringbus2_post.h"


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

circular_buf_t dma_inbuf;
unsigned int dma_inbuf_storage[DMA_IN_COUNT+1];
unsigned int* dma_in_ptr[DMA_IN_COUNT];

void setup_dma_in(void) {
  dma_inbuf.size = DMA_IN_COUNT+1;
  dma_inbuf.buffer = dma_inbuf_storage;
  circular_buf_reset(&dma_inbuf);
}

#define MY_ASSERT(x) if(!(x)) {return __LINE__;}

// insert 4 things, remove directly, test flags along the way, and values as they come out
int test0() {

    setup_dma_in();

    int error = 0;
    int empty = 0;
    int full = 0;

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 1);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf_put(&dma_inbuf, 2);
    // report_test_results(dma_inbuf.occupancy);
    report_test_results(error);
    MY_ASSERT(error == 0);

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf_put(&dma_inbuf, 4);
    // report_test_results(dma_inbuf.occupancy);
    MY_ASSERT(error == 0);

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf_put(&dma_inbuf, 6);
    // report_test_results(dma_inbuf.occupancy);
    MY_ASSERT(error == 0);

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf_put(&dma_inbuf, 8);
    // report_test_results(dma_inbuf.occupancy);
    MY_ASSERT(error == 0);

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 1);

    unsigned int data;

    error = circular_buf_get(&dma_inbuf, &data);
    MY_ASSERT(error == 0);
    MY_ASSERT(data == 2);

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf_get(&dma_inbuf, &data);
    MY_ASSERT(error == 0);
    MY_ASSERT(data == 4);

    error = circular_buf_get(&dma_inbuf, &data);
    MY_ASSERT(error == 0);
    MY_ASSERT(data == 6);

    error = circular_buf_get(&dma_inbuf, &data);
    MY_ASSERT(error == 0);
    MY_ASSERT(data == 8);

    error = circular_buf_get(&dma_inbuf, &data);
    MY_ASSERT(error != 0);

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 1);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);
    // MY_ASSERT(data == 8);


    // circular_buf_put(&dma_inbuf, 0);
    // report_test_results(dma_inbuf.occupancy);

    // circular_buf_put(&dma_inbuf, 0);
    // report_test_results(dma_inbuf.occupancy);

  return 0;
}


// test add 3, remove 2, add 2, remove 3
int test1() {

    setup_dma_in();

    int error = 0;
    unsigned int data, empty, full;

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 1);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf_put(&dma_inbuf, 2); MY_ASSERT(error == 0);
    error = circular_buf_put(&dma_inbuf, 4); MY_ASSERT(error == 0);
    error = circular_buf_put(&dma_inbuf, 6); MY_ASSERT(error == 0);

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);


    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 2);

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 4);

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf_put(&dma_inbuf, 20); MY_ASSERT(error == 0);
    error = circular_buf_put(&dma_inbuf, 22); MY_ASSERT(error == 0);


    // ((dma_inbuf.head + 1) % dma_inbuf.size) == dma_inbuf.tail;

    // cap should be 3 here

    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 6);

    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 20);

    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 22);

    // should be empty
    error = circular_buf_get(&dma_inbuf, &data);
    MY_ASSERT(error != 0);

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 1);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);

  return 0;
}


#define REPORT_CIRBUF4(yy) \
report_test_results(0xcafe); \
report_test_results(yy.head); \
report_test_results(yy.tail); \
report_test_results(circular_buf_full(&yy)); \
report_test_results(circular_buf_empty(&yy)); \
report_test_results(circular_buf_occupancy(&yy)); \
report_test_results(yy.buffer[0]); \
report_test_results(yy.buffer[1]); \
report_test_results(yy.buffer[2]); \
report_test_results(yy.buffer[3]); \
report_test_results(yy.buffer[4]);

#define REPORT_CIRBUF(yy) \
report_test_results(0xcafe); \
report_test_results(yy.head); \
report_test_results(yy.tail); \
report_test_results(((yy.head + 1) % yy.size)); \
report_test_results(circular_buf_full(&yy)); \
report_test_results(circular_buf_empty(&yy)); \
report_test_results(circular_buf_occupancy(&yy));

#define REP() REPORT_CIRBUF(dma_inbuf)

int test_debug() {

    setup_dma_in();

    int error = 0;
    unsigned int data;

    REP();
    error = circular_buf_put(&dma_inbuf, 2); MY_ASSERT(error == 0);
    REP();
    error = circular_buf_put(&dma_inbuf, 4); MY_ASSERT(error == 0);
    REP();
    error = circular_buf_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
    REP();
    error = circular_buf_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
    REP();
    error = circular_buf_put(&dma_inbuf, 6); MY_ASSERT(error != 0);
    REP();
    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    // REP();
    error = circular_buf_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
    REP();
    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    // REP();
    error = circular_buf_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
    REP();
    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    // REP();
    error = circular_buf_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
    REP();
    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    // REP();
    error = circular_buf_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
    REP();
    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    // REP();
    error = circular_buf_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
    REP();

#ifdef asdf


    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 2);

    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 4);

    error = circular_buf_put(&dma_inbuf, 20); MY_ASSERT(error == 0);
    error = circular_buf_put(&dma_inbuf, 22); MY_ASSERT(error == 0);


    // ((dma_inbuf.head + 1) % dma_inbuf.size) == dma_inbuf.tail;

    // cap should be 3 here

    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 6);

    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 20);

    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 22);

    // should be empty
    error = circular_buf_get(&dma_inbuf, &data);
    MY_ASSERT(error != 0);
#endif


  return 0;
}

int test2() {

    setup_dma_in();

    int error = 0;
    unsigned int data, empty, full, occupancy;

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 1);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);
    error = circular_buf_peek(&dma_inbuf, &data); MY_ASSERT(error == -1);

    error = circular_buf_put(&dma_inbuf, 2); MY_ASSERT(error == 0);
    error = circular_buf_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 2);

    error = circular_buf_put(&dma_inbuf, 4); MY_ASSERT(error == 0);
    error = circular_buf_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 2);

    error = circular_buf_put(&dma_inbuf, 6); MY_ASSERT(error == 0);
    error = circular_buf_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 2);



    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);


    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 2);
    error = circular_buf_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 4);


    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);


    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 4);
    error = circular_buf_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 6);

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 0);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);

    error = circular_buf_put(&dma_inbuf, 20); MY_ASSERT(error == 0);
    error = circular_buf_put(&dma_inbuf, 22); MY_ASSERT(error == 0);

    error = circular_buf_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 6);

    // ((dma_inbuf.head + 1) % dma_inbuf.size) == dma_inbuf.tail;

    // cap should be 3 here

    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 6);
    error = circular_buf_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 20);

    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 20);
    error = circular_buf_peek(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 22);

    error = circular_buf_get(&dma_inbuf, &data); MY_ASSERT(error == 0);
    MY_ASSERT(data == 22);

    // peek should error when empty
    error = circular_buf_peek(&dma_inbuf, &data); MY_ASSERT(error != 0);

    // should be empty
    error = circular_buf_get(&dma_inbuf, &data);
    MY_ASSERT(error != 0);

    empty = circular_buf_empty(&dma_inbuf); MY_ASSERT(empty == 1);
    full = circular_buf_full(&dma_inbuf); MY_ASSERT(full == 0);

  return 0;
}

// torture test circular_buf_occupancy
int test3() {
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
          occupy = circular_buf_occupancy(&dma_inbuf);
          MY_ASSERT(occupy == 0);

          // add pre items
          for(unsigned int i = 0; i < pre; i++) {
            circular_buf_put(&dma_inbuf, i);
          }

          // check if we are at the correct fill
          occupy = circular_buf_occupancy(&dma_inbuf);
          MY_ASSERT(occupy == pre);

          // add `add` more items
          for(unsigned int i = 0; i < add; i++) {
            circular_buf_put(&dma_inbuf, i);

            occupy = circular_buf_occupancy(&dma_inbuf);
            MY_ASSERT(occupy == (pre+i+1)); // checking after i runs
          }

          // check outside loop
          occupy = circular_buf_occupancy(&dma_inbuf);
          MY_ASSERT(occupy == (pre+add));

          // sub `sub` items
          for(int i = 0; i < sub; i++) {
            circular_buf_get(&dma_inbuf, &data);

            occupy = circular_buf_occupancy(&dma_inbuf);
            MY_ASSERT(occupy == (pre+add-i-1)); // checking after i runs
          }

        }
      }
    }


  return 0;
}


// can hold 4 things
#define DMA_IN_QUEUE_SIZE (5)
circular_buf_t dma_in_queue;
unsigned int dma_in_queue_storage[DMA_IN_QUEUE_SIZE];

unsigned int dma_trig_next = 0;

// test I added when I realized filling a size 4 buf returns occupancy 3;
int test4() {
   circular_buf_initialize(&dma_in_queue, dma_in_queue_storage, DMA_IN_QUEUE_SIZE);

   // REPORT_CIRBUF(dma_in_queue);

   // report_test_results(0xea1);

  int error;
  unsigned int data;
  unsigned int dma_occupancy;
  unsigned int filled;

  filled = circular_buf_occupancy(&dma_in_queue);
  MY_ASSERT(filled == 0);


  circular_buf_put(&dma_in_queue, 0);
  filled = circular_buf_occupancy(&dma_in_queue);
  MY_ASSERT(filled == 1);


  circular_buf_put(&dma_in_queue, 1);
  filled = circular_buf_occupancy(&dma_in_queue);
  MY_ASSERT(filled == 2);


  circular_buf_put(&dma_in_queue, 2);
  filled = circular_buf_occupancy(&dma_in_queue);
  MY_ASSERT(filled == 3);


  circular_buf_put(&dma_in_queue, 3);
  filled = circular_buf_occupancy(&dma_in_queue);
  MY_ASSERT(filled == 4);


  circular_buf_put(&dma_in_queue, 4); // should get rejected
  filled = circular_buf_occupancy(&dma_in_queue);
  MY_ASSERT(filled == 4);


  error = circular_buf_get(&dma_in_queue, &data); //  report_test_results(data); report_test_results(error);

  MY_ASSERT(data == 0);

  error = circular_buf_get(&dma_in_queue, &data); //  report_test_results(data); report_test_results(error);

  MY_ASSERT(data == 1);

  error = circular_buf_get(&dma_in_queue, &data); //  report_test_results(data); report_test_results(error);
  MY_ASSERT(data == 2);

  error = circular_buf_get(&dma_in_queue, &data); //  report_test_results(data); report_test_results(error);
  MY_ASSERT(data == 3);


  filled = circular_buf_occupancy(&dma_in_queue);
  MY_ASSERT(filled == 0);

  return 0;
}


int main(void)
{
  unsigned int all_results = 0;
  unsigned int test = 0;

  report_test_results(0xdeadbeef);
 
  test = test0();
  all_results |= (test==0)<<0;

  test = test1();
  all_results |= (test==0)<<1;

  test = test2();
  all_results |= (test==0)<<2;

  test = test3();
  all_results |= (test==0)<<3;

  test = test4();
  all_results |= (test==0)<<4;


  report_test_results(0x70000000 | test);
  report_test_results(0x80000000 | all_results);

  return 0;
}
