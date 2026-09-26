#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "dma.h"
#include "fill.h"
#include "mover.h"
#include "mapper.h"
#include "ringbus.h"
#include "circular_buffer.h"


#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS10
#include "ringbus2_post.h"

#include "vmalloc.h"
// declare as global
VMalloc mgr;

// #define OUTPUT_FRAME_COUNT_WORST_CASE (40)
#define FFT_SIZE (1024)


#define TRUE               (0x1)
#define FALSE              (0x0)

#define INIT_STATE         (0x0)
#define RX_STATE           (0x1)
#define MOVE_STATE         (0x3)
#define TX_STATE           (0x4)
#define WAITING_STATE      (0x5)
#define FINISH_STATE       (0x6)

// normal operation
// accept words, map to bpsk, move to subcarriers, output

// modes
// accepts words, map to "debug bpsk" (values of 0,1,2,3), move to subcarriers, output
// #define USE_FAKE_BPSK

// ignore input, map counter values to subcarriers, output
// when this is set, USE_FAKE_BPSK, has no effect
// #define USE_FAKE_MOVER_INPUT

// controlls which style of schedule is consumed
// simply enabling this is not enough, setup_mover() should also be edited
// old means dmem schedule and mover_schedule()
// new means vmem schedule and mover_load_vmem()
// #define USE_OLD_MOVER_SCHEDULE_FORMAT


// might only be valid when USE_OLD_MOVER_SCHEDULE_FORMAT is not enabled
// disabling this means we will wait for every dma output to complete before scheduling
// the next
// #define USE_DOUBLE_BUFFER


#define MY_ASSERT(x) if(!(x)) { ring_block_send_eth(0xe0000000|__LINE__);}



// register volatile unsigned int x3 asm("x3");
// register volatile unsigned int x4 asm("x4");


Table qpsk_table;
Table bpsk_table;

// #define MOVER_SRC_ROW          (mapper_output_row)
// #define MAPPER_DEST_ROW        (mapper_output_row)

#define DST_ROW_REV (VMEM_ROW_ADDRESS(dst_mem))

unsigned int mapper_output_row;

// unsigned int qpsk_table_dma;
// unsigned int bpsk_table_dma;


// void setup_mapper(void) {
//   // qpsk_table_dma = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));
// #ifdef USE_FAKE_BPSK
//   // qpsk_table = mapper_debug_qpsk_table(qpsk_table_dma);
// #else
//   // qpsk_table = mapper_qpsk_table(qpsk_table_dma);
// #endif
  
//   // bpsk_table_dma = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));
//   // bpsk_table = mapper_bpsk_table(bpsk_table_dma);

//   // output of mapper, input to mover
//   mapper_output_row = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
// }


int mover_working_on;

// FIXME lame way of doing this
#define MAX_SCHEDULE_COUNT 16

#define GARBAGE_ROW       (garbage_row)
#define SCRATCH_DMA       (scratch_dma_ptr)

// in the reverse mover, DST_ROW is actually the source

#define SRC_ROW_REV           (VMEM_ROW_ADDRESS(input_dma))

unsigned int garbage_row;


#ifdef USE_OLD_MOVER_SCHEDULE_FORMAT
Schedule schedules[MAX_SCHEDULE_COUNT][NSLICES];
#else
// Directly create this in vmem (we could also load this at compile time with a compile time change to the value of DST_ROW)
VMEM_SECTION VmemSchedule vmem_schedules[MAX_SCHEDULE_COUNT];
#endif


// how many fft's to move at a time
#define FRAME_MOVE_CHUNK (2)

// the 2 is for ping/pong
VMEM_SECTION unsigned int input_dma[FFT_SIZE*FRAME_MOVE_CHUNK*2] = {};

// worst case 1024 enabled subcarriers
VMEM_SECTION unsigned int dst_mem[FFT_SIZE*FRAME_MOVE_CHUNK*2] = {};

unsigned int enabled_subcarriers; // delcared here but SET BY OUTPUT FROM schedule_maker.py
unsigned int number_active_schedules; // same as previous

unsigned int dma_in_dma_ptr;

void setup_mover(void) {
  garbage_row = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));

  dma_in_dma_ptr = VMEM_DMA_ADDRESS(input_dma);

//   // Total Subcarrier #: 32
// // Total Subcarriers: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31]
// //   Bins:
// // 00 (02): [0, 16]
// // 01 (02): [1, 17]
// // 02 (02): [2, 18]
// // 03 (02): [3, 19]
// // 04 (02): [4, 20]
// // 05 (02): [5, 21]
// // 06 (02): [6, 22]
// // 07 (02): [7, 23]
// // 08 (02): [8, 24]
// // 09 (02): [9, 25]
// // 10 (02): [10, 26]
// // 11 (02): [11, 27]
// // 12 (02): [12, 28]
// // 13 (02): [13, 29]
// // 14 (02): [14, 30]
// // 15 (02): [15, 31]
// // Longest bin: 2

// // Schedule usage
// // 00: 100.00 %
// // 01: 100.00 %
// // Run: 0
// // Run: 1
// // input_stride = 2
// // output_stride = 64

// // global constants
enabled_subcarriers = 32;
number_active_schedules = 2;

// //////////////////////////////////////////////////////////////////////////////////////////////////


// // Reverse for chunk(0): [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]
// schedules[0][0 ] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][1 ] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][2 ] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][3 ] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][4 ] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][5 ] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][6 ] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][7 ] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][8 ] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][9 ] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][10] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][11] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][12] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][13] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][14] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};
// schedules[0][15] = (Schedule) {SRC_ROW_REV+0, 64,(0x10 << 12) | (DST_ROW_REV + 0), 0x2};

// // Reverse for chunk(1): [16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31]
// schedules[1][0 ] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][1 ] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][2 ] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][3 ] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][4 ] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][5 ] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][6 ] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][7 ] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][8 ] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][9 ] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][10] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][11] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][12] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][13] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][14] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};
// schedules[1][15] = (Schedule) {SRC_ROW_REV+1, 64,(0x10 << 12) | (DST_ROW_REV + 1), 0x2};

vmem_schedules[0] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0) },
{ 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2 }
};

vmem_schedules[1] = (VmemSchedule) {
{ SRC_ROW_REV+1, SRC_ROW_REV+1, SRC_ROW_REV+1, SRC_ROW_REV+1, SRC_ROW_REV+1, SRC_ROW_REV+1, SRC_ROW_REV+1, SRC_ROW_REV+1, SRC_ROW_REV+1, SRC_ROW_REV+1, SRC_ROW_REV+1, SRC_ROW_REV+1, SRC_ROW_REV+1, SRC_ROW_REV+1, SRC_ROW_REV+1, SRC_ROW_REV+1 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1) },
{ 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2 }
};




}

unsigned int input_frame_count;

#ifdef USE_DOUBLE_BUFFER
unsigned int mover_output_increment_words;
unsigned int mover_output_increment_row;
#endif
// call after setup_mover
void setup_mover_post() {
  input_frame_count = FRAME_MOVE_CHUNK;

#ifdef USE_DOUBLE_BUFFER
  // bumps for our "a" / "b" buffers
  mover_output_increment_words = input_frame_count << 10; // times 1024
  mover_output_increment_row = mover_output_increment_words >> 4; // over 16
#endif
}

// in words
#define DMA_IN_SIZE (1024*2)

#define DMA_IN_CHUNKS (2)

#define DMA_IN_CIRBUF_SIZE (DMA_IN_CHUNKS+1)
circular_buf_t dma_in_buffer;
unsigned int dma_in_buffer_storage[DMA_IN_CIRBUF_SIZE];

// setting this to 5 means buffer can hold 4
#define DMA_SCHEDULE_IN_SIZE (4+1)
circular_buf_t dma_schedule_in;
unsigned int dma_schedule_in_storage[DMA_SCHEDULE_IN_SIZE];

// #define DMA_SCHEDULE_OUT_SIZE (4+1)
// circular_buf_t dma_schedule_out;
// unsigned int dma_schedule_out_storage[DMA_SCHEDULE_OUT_SIZE];

unsigned int dma_trig_next = 0;

// converts a dma index (used in the cirbufs) to a dma_ptr
// the dma index counts each block of memory
unsigned int dma_idx_to_ptr(unsigned int idx) {
  return dma_in_dma_ptr + (idx * DMA_IN_SIZE);
}

// convert a dma_ptr to an index
unsigned int dma_ptr_to_idx(unsigned int ptr) {
  return (ptr - dma_in_dma_ptr) / DMA_IN_SIZE;
}

void trig_dma_in(unsigned int dma_ptr) {
  // ring_block_send_eth(dma_ptr);
  CSR_WRITE(DMA_0_START_ADDR, dma_ptr);
  CSR_WRITE(DMA_0_LENGTH, DMA_IN_SIZE);
  CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff); // start right away
  CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);
}


// void trig_dma_out(unsigned int dma_ptr) {
//   CSR_WRITE(DMA_1_START_ADDR, dma_ptr);
//   CSR_WRITE(DMA_1_LENGTH, DMA_IN_SIZE);
//   CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff); // start right away
//   CSR_WRITE_ZERO(DMA_1_PUSH_SCHEDULE);
// }


void trig_dma_in_next() {
  trig_dma_in(dma_idx_to_ptr(dma_trig_next));

  circular_buf_put(&dma_schedule_in, dma_trig_next);

  dma_trig_next = (dma_trig_next+1) % DMA_IN_CHUNKS;
}
//////////////////////////////////////////
//
// We run 2 circular buffers2
// the first buffer keeps track of outstanding input dma so they are always overlapping
// as these they dump into the 2nd circular buffer which is the "pending data" and also our fill level

void setup_dma_in(void) {
  // dma_in_dma_ptr = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));

  circular_buf_initialize(&dma_schedule_in, dma_schedule_in_storage, DMA_SCHEDULE_IN_SIZE);
  circular_buf_initialize(&dma_in_buffer, dma_in_buffer_storage, DMA_IN_CIRBUF_SIZE);


  trig_dma_in_next();
  trig_dma_in_next();
  // trig_dma_in_next();
  // trig_dma_in_next();

  // how many chunks we get from a single vmalloc
  // unsigned int chunks = (VMALLOC_CHUNK_SIZE/4) / dma_in_size;

  // ring_block_send_eth(chunks);

  // trig_dma_in(0, 0xffffffff);
  // trig_dma_in(1, 0xffffffff);
}

// void setup_dma_out(void) {
//   circular_buf_initialize(&dma_schedule_out, dma_schedule_out_storage, DMA_SCHEDULE_OUT_SIZE);
// }

// unsigned int fake_work_todo = 0;

void pet_dma_inqueue() {
  int error;
  unsigned int just_finished_idx;
  unsigned int dma_occupancy;

  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, dma_occupancy);
  unsigned int filled = circular_buf_occupancy(&dma_schedule_in);

  unsigned int outgoing_buf_occupancy;


  if( dma_occupancy != filled ) {
    // just_finished_idx is the index of the dma that just finished
    error = circular_buf_get(&dma_schedule_in, &just_finished_idx); MY_ASSERT(error == 0);

    // now that dma is done with this chunk, we add it to the next
    // circular buffer which signals the program that there is fresh data to be processed
    circular_buf_put(&dma_in_buffer, just_finished_idx);
    // ring_block_send_eth(dma_occupancy);
    // ring_block_send_eth(filled);

    outgoing_buf_occupancy = circular_buf_occupancy(&dma_in_buffer);

    SET_REG(x3, 0xb0000000 | outgoing_buf_occupancy);

    // fake_work_todo += 10;


    // ring_block_send_eth(data);
    trig_dma_in_next();
  }

}


void pet_dma_in() {
  pet_dma_inqueue();
}


// unsigned int debug_readout(unsigned int count) {
//   unsigned int dma_idx_just_finished;
//   unsigned int* dma_cpu_pointer;
//   int error;
//   for(unsigned i = 0; i < count; i++) {
//     error = circular_buf_get(&dma_in_buffer, &dma_idx_just_finished); MY_ASSERT(error == 0);

//     dma_cpu_pointer = REVERSE_VMEM_DMA_ADDRESS(dma_idx_to_ptr(dma_idx_just_finished));

//     for(unsigned int j = 0; j < 16; j++) {
//       ring_block_send_eth(dma_cpu_pointer[j]);
//     }


//   }
// }

void pet_mover() {
  // SET_REG(x3, 0x2);

  unsigned int dma_idx_just_finished;
  unsigned int incomming_occupancy, outgoing_dma_occupancy;
  int error;

  incomming_occupancy = circular_buf_occupancy(&dma_in_buffer);
  CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, outgoing_dma_occupancy);

  if(incomming_occupancy > 0 && outgoing_dma_occupancy == 0) {
    error = circular_buf_get(&dma_in_buffer, &dma_idx_just_finished); MY_ASSERT(error == 0);

    SET_REG(x3, 0xa0000000 | dma_idx_just_finished);
    // SET_REG(x4, 0xcafe);
    // SET_REG(x4, dma_idx_just_finished);

    // input is 2 fft's at once
    unsigned int mover_input_increment_row = (dma_idx_just_finished * FFT_SIZE * FRAME_MOVE_CHUNK) / NSLICES;

    SET_REG(x4, mover_input_increment_row);

    for(unsigned int i = 0; i < number_active_schedules; i++) {
      mover_load_offset_input( &(vmem_schedules[i]), mover_input_increment_row);
      mover_roll(input_frame_count);
    }


    // if( mover_working_on == 0 ) {
    dma_out_set(DST_ROW_REV*NSLICES, enabled_subcarriers*FRAME_MOVE_CHUNK);
//         mover_working_on = 1;
//       } else {
//         // buffer "B"
        // dma_out_set(DST_ROW_REV*NSLICES+mover_output_increment_words, FFT_SIZE*input_frame_count);
//         mover_working_on = 0;
//       }

    // if( mover_working_on == 0) {
        
    //     // move into beginning of buffer
    //     for(unsigned int i = 0; i < number_active_schedules; i++) {
    //       mover_load_vmem(&(vmem_schedules[i]));
    //       mover_roll(input_frame_count);
    //     }
    //   } else {

    //     // move into offset of buffer
    //     for(unsigned int i = 0; i < number_active_schedules; i++) {
    //       mover_load_offset_output( &(vmem_schedules[i]), mover_output_increment_row);
    //       mover_roll(input_frame_count);
    //     }
    //   }

  }
  // SET_REG(x3, 0x4);
  // exit2(0);

  // CSR_READ(TIMER_VALUE, d);

  // ring_block_send_eth(0x0d000000 | d-c);


  CSR_WRITE(GPIO_WRITE, (0x200000) | 2);
    
}

int main(void)
{
  unsigned int rtn;
  Ringbus ringbus;

  // setup vmalloc
  init_VMalloc(&mgr);


  // unsigned int* input_cpu_ptr =  vmalloc_single(&mgr);
  // unsigned int input_dma_ptr = VMEM_DMA_ADDRESS(input_cpu_ptr);

  unsigned int* scrach_cpu_ptr = vmalloc_single(&mgr);
  unsigned int scratch_dma_ptr = VMEM_DMA_ADDRESS(scrach_cpu_ptr);

  unsigned int* debug_cpu_ptr =  vmalloc_single(&mgr);
  unsigned int debug_dma_ptr = VMEM_DMA_ADDRESS(debug_cpu_ptr);

  mover_working_on = 0;

  unsigned int a, b, c, d;


  CSR_WRITE(DMA_0_FLUSH_SCHEDULE,  0);
  CSR_WRITE(DMA_1_FLUSH_SCHEDULE,  0);




  // for(unsigned int i = 0; i < 1024; i++) {
  //   vector_memory[i+input_dma_ptr] = 0xf000d000 + i;
  // }

  // setup mapper
  // setup_mapper();

  setup_mover();
  setup_mover_post(); // must be called afer previous

  setup_dma_in();
  // setup_dma_out();
  // setup_fill_level();

  SET_REG(x3, 0xdeadbeef);
  SET_REG(x4, 0xdeadbeef);

  // ring_block_send_eth(0xdead); // boot

  // debug ringbus out row addresses of all memory for easy lookup into cs20.out (row+1 = linenumber)
  // ring_block_send_eth(input_dma_ptr);
  ring_block_send_eth(scratch_dma_ptr);
  ring_block_send_eth(debug_dma_ptr);
  ring_block_send_eth(VMEM_ROW_ADDRESS(input_dma));
  ring_block_send_eth(VMEM_ROW_ADDRESS(dst_mem));
  // ring_block_send_eth(MAPPER_DEST_ROW);
  // ring_block_send_eth(VMEM_ROW_ADDRESS(mover_output));
  // ring_block_send_eth(VMEM_ROW_ADDRESS(REVERSE_VMEM_DMA_ADDRESS(SCRATCH_DMA)));
  // ring_block_send_eth(DST_ROW);
  // ring_block_send_eth(DST_ROW+mover_output_increment_row);
  // ring_block_send_eth(VMEM_ROW_ADDRESS(&vs0));

  // ring_block_send_eth(VMEM_ROW_ADDRESS(input_cpu_ptr));
  // ring_block_send_eth(VMEM_ROW_ADDRESS(scrach_cpu_ptr));
  // ring_block_send_eth(GARBAGE_ROW);
  // ring_block_send_eth(mapper_output_row);
  // ring_block_send_eth(qpsk_table_dma/16);


  CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);
  unsigned int fsm_state = INIT_STATE;
  unsigned int return_time;



  unsigned int incomming_occupancy;
  unsigned int outgoing_dma_occupancy;
  unsigned int dma_idx_just_finished;
  unsigned int needs_mapping_dma_ptr;
  int error;

  while(1) {
    pet_dma_in();

    pet_mover();
  }

}

