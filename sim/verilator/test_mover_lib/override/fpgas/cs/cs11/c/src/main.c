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
#include "subtract_timers.h"


#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "vmalloc.h"
// declare as global
VMalloc mgr;

////////////////////////////////
//
// output of the mapper needs x * 1024 words of memory in a row
// this number is calculated by: 
//   input_word_chunk = 16
//   bits_per_word = 32
//   bits_per_symbol = 2
//   enabled_subcarriers = 8
// formula:
//   input_word_chunk * bits_per_word / bits_per_symbol / enabled_subcarriers
//   16 * 32 / 2 / 8
//
#define OUTPUT_FRAME_COUNT_WORST_CASE (40)
#define FFT_SIZE (1024)

#define OUR_RING_ENUM RING_ENUM_CS11
#define ETH_RING_ENUM      (6)


#define TRUE               (0x1)
#define FALSE              (0x0)

#define INIT_STATE         (0x0)
#define RX_STATE           (0x1)
#define MAP_STATE          (0x2)
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
#define USE_FAKE_MOVER_INPUT

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



register volatile unsigned int x3 asm("x3");
register volatile unsigned int x4 asm("x4");


Table qpsk_table;
Table bpsk_table;

#define MOVER_SRC_ROW          (mapper_output_row)
#define MAPPER_DEST_ROW        (mapper_output_row)

unsigned int mapper_output_row;

unsigned int qpsk_table_dma;
unsigned int bpsk_table_dma;


void setup_mapper(void) {
  qpsk_table_dma = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));
#ifdef USE_FAKE_BPSK
  qpsk_table = mapper_debug_qpsk_table(qpsk_table_dma);
#else
  qpsk_table = mapper_qpsk_table(qpsk_table_dma);
#endif
  
  bpsk_table_dma = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));
  bpsk_table = mapper_bpsk_table(bpsk_table_dma);

  // output of mapper, input to mover
  mapper_output_row = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
}


int mover_working_on;

// FIXME lame way of doing this
#define MAX_SCHEDULE_COUNT 16

#define GARBAGE_ROW       (garbage_row)
#define SCRATCH_DMA       (scratch_dma_ptr)
#define DST_ROW           (VMEM_ROW_ADDRESS(mover_output))

unsigned int garbage_row;


#ifdef USE_OLD_MOVER_SCHEDULE_FORMAT
Schedule schedules[MAX_SCHEDULE_COUNT][NSLICES];
#else
// Directly create this in vmem (we could also load this at compile time with a compile time change to the value of DST_ROW)
VMEM_SECTION VmemSchedule vmem_schedules[MAX_SCHEDULE_COUNT];
#endif

// mover_output worst case (when we have the fewest enabled subcarriers)
// we map 256 at a time, so more subacciers means we have less output
// (higher mapped number means we pack inputs more densly in output)

VMEM_SECTION unsigned int mover_output[FFT_SIZE*OUTPUT_FRAME_COUNT_WORST_CASE] = {};

unsigned int enabled_subcarriers; // delcared here but SET BY OUTPUT FROM schedule_maker.py
unsigned int number_active_schedules; // same as previous

void setup_mover(void) {
  garbage_row = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
// Total Subcarrier #: 16
// Total Subcarriers: [0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255]
//   Bins:
// 00 (01): [0]
// 01 (01): [17]
// 02 (01): [34]
// 03 (01): [51]
// 04 (01): [68]
// 05 (01): [85]
// 06 (01): [102]
// 07 (01): [119]
// 08 (01): [136]
// 09 (01): [153]
// 10 (01): [170]
// 11 (01): [187]
// 12 (01): [204]
// 13 (01): [221]
// 14 (01): [238]
// 15 (01): [255]
// Longest bin: 1

// Schedule usage
// 00: 100.00 %
// Run: 0
// input_stride = 1
// output_stride = 64

// global constants
enabled_subcarriers = 16;
number_active_schedules = 1;

// Schedule for chunk(0): [0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255]
vmem_schedules[0] = (VmemSchedule) {
{ MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ (0x0 << 12) | (DST_ROW + 0), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 9), (0x0 << 12) | (DST_ROW + 10), (0x0 << 12) | (DST_ROW + 11), (0x0 << 12) | (DST_ROW + 12), (0x0 << 12) | (DST_ROW + 13), (0x0 << 12) | (DST_ROW + 14), (0x0 << 12) | (DST_ROW + 15) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};

}

unsigned int output_frame_count;

#ifdef USE_DOUBLE_BUFFER
unsigned int mover_output_increment_words;
unsigned int mover_output_increment_row;
#endif
// call after setup_mover
void setup_mover_post(void) {
  output_frame_count = (16 * 16) / enabled_subcarriers;

#ifdef USE_DOUBLE_BUFFER
  // bumps for our "a" / "b" buffers
  mover_output_increment_words = output_frame_count << 10; // times 1024
  mover_output_increment_row = mover_output_increment_words >> 4; // over 16
#endif
}
unsigned int dma_in_dma_ptr;

// in words
#define DMA_IN_SIZE (16)

// 64 is maximum here unless more memory is vmalloc'd
#define DMA_IN_CHUNKS (64)

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


void trig_dma_in_next(void) {
  trig_dma_in(dma_idx_to_ptr(dma_trig_next));

  circular_buf_put(&dma_schedule_in, dma_trig_next);

  dma_trig_next = (dma_trig_next+1) % DMA_IN_CHUNKS;
}
//////////////////////////////////////////
//
// We run 2 circular buffers
// the first buffer keeps track of outstanding input dma so they are always overlapping
// as these they dump into the 2nd circular buffer which is the "pending data" and also our fill level

void setup_dma_in(void) {
  dma_in_dma_ptr = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));

  circular_buf_initialize(&dma_schedule_in, dma_schedule_in_storage, DMA_SCHEDULE_IN_SIZE);
  circular_buf_initialize(&dma_in_buffer, dma_in_buffer_storage, DMA_IN_CIRBUF_SIZE);


  trig_dma_in_next();
  trig_dma_in_next();
  trig_dma_in_next();
  trig_dma_in_next();

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

void pet_dma_inqueue(void) {
  int error;
  unsigned int just_finished_idx;
  unsigned int dma_occupancy;

  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, dma_occupancy);
  unsigned int filled = circular_buf_occupancy(&dma_schedule_in);


  if( dma_occupancy != filled ) {
    // just_finished_idx is the index of the dma that just finished
    error = circular_buf_get(&dma_schedule_in, &just_finished_idx); MY_ASSERT(error == 0);

    // now that dma is done with this chunk, we add it to the next
    // circular buffer which signals the program that there is fresh data to be processed
    circular_buf_put(&dma_in_buffer, just_finished_idx);
    // ring_block_send_eth(dma_occupancy);
    // ring_block_send_eth(filled);

    // fake_work_todo += 10;


    // ring_block_send_eth(data);
    trig_dma_in_next();
  }

}


void pet_dma_in(void) {
  pet_dma_inqueue();
}


// #define FILL_UNDERFLOW  (0x1 << 16)
// #define FILL_OVERFLOW   (0x1 << 17)

#define LESS_THAN_UNDERFLOW (4)
#define GREATER_THAN_OVERFLOW (62)

#define FILL_LEVEL_REPORT_TIME (20000000)

unsigned int report_timer, report_timer_check, fill_flags;

void setup_fill_level(void) {
  // fill level update timer, ringbus takes about 500 counter ticks to write
  // so this update should be much slower then that
  CSR_READ(TIMER_VALUE, report_timer);
  fill_flags = 0;
}

#define FILL_L_SHIFT (0)
#define FILL_L_MASK (0xFF)

#define FILL_H_SHIFT (8)
#define FILL_H_MASK (0xFF)

#define FILL_UNDERFLOW_SHIFT (16)
#define FILL_UNDERFLOW_MASK (0x1)

#define FILL_OVERFLOW_SHIFT (17)
#define FILL_OVERFLOW_MASK (0x1)



void pet_fill_level(void) {
  Ringbus ringbus;

  unsigned int fill;
  static unsigned int fill_low = 0xffff;
  static unsigned int fill_high = 0x0000;

  CSR_READ(TIMER_VALUE, report_timer_check);

  fill = circular_buf_occupancy(&dma_in_buffer);

  // check for under/over every time
  if(fill <= LESS_THAN_UNDERFLOW) {
    fill_flags |= (1<<FILL_UNDERFLOW_SHIFT);
  }

  if(fill >= GREATER_THAN_OVERFLOW) {
    fill_flags |= (1<<FILL_OVERFLOW_SHIFT);
  }

  // set high/lows
  fill_low =  MIN(fill_low,  fill);
  fill_high = MAX(fill_high, fill);

  // report flags slowly
  // if( report_timer_check >= (report_timer+FILL_LEVEL_REPORT_TIME) ) {
  if( subtract_timers(report_timer_check, report_timer) >= FILL_LEVEL_REPORT_TIME) {
    ring_block_send_eth(0x0b000000 | 
      fill_flags | 
      ((fill_low & FILL_L_MASK) << FILL_L_SHIFT) |
      ((fill_high & FILL_H_MASK) << FILL_H_SHIFT)
      );
    // ring_block_send_eth(fill_low);
    // ring_block_send_eth(fill_high);

    report_timer = report_timer_check;
    

    fill_low = 0xffff;
    fill_high = 0x0000;
    fill_flags = 0;
  }

  check_ring(&ringbus); // check bootloader more often than we report
}

void debug_readout(unsigned int count) {
  unsigned int dma_idx_just_finished;
  unsigned int* dma_cpu_pointer;
  int error;
  for(unsigned i = 0; i < count; i++) {
    error = circular_buf_get(&dma_in_buffer, &dma_idx_just_finished); MY_ASSERT(error == 0);

    dma_cpu_pointer = REVERSE_VMEM_DMA_ADDRESS(dma_idx_to_ptr(dma_idx_just_finished));

    for(unsigned int j = 0; j < 16; j++) {
      ring_block_send_eth(dma_cpu_pointer[j]);
    }


  }
}

int main(void)
{
  unsigned int rtn;
  Ringbus ringbus;

  // setup vmalloc
  init_VMalloc(&mgr);


  unsigned int* input_cpu_ptr =  vmalloc_single(&mgr);
  unsigned int input_dma_ptr = VMEM_DMA_ADDRESS(input_cpu_ptr);

  unsigned int* scrach_cpu_ptr = vmalloc_single(&mgr);
  unsigned int scratch_dma_ptr = VMEM_DMA_ADDRESS(scrach_cpu_ptr);

  unsigned int* debug_cpu_ptr =  vmalloc_single(&mgr);
  unsigned int debug_dma_ptr = VMEM_DMA_ADDRESS(debug_cpu_ptr);

  mover_working_on = 0;

  unsigned int a, b, c, d;

  // for(unsigned int i = 0; i < 16; i++) {
  //   debug_cpu_ptr[i] = 0xa0000000 | i;
  // }


  // // dump our input and reset
  // CSR_WRITE(DMA_0_START_ADDR, dma_idx_to_ptr(0));
  // CSR_WRITE(DMA_0_LENGTH, 1024);
  // CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff); // start right away
  // CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);
  // CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);

  // for(unsigned int i = 0; i < 10000; i++) {
  //   STALL(1);
  // }

  CSR_WRITE(DMA_0_FLUSH_SCHEDULE,  0);
  CSR_WRITE(DMA_1_FLUSH_SCHEDULE,  0);




  // for(unsigned int i = 0; i < 1024; i++) {
  //   vector_memory[i+input_dma_ptr] = 0xf000d000 + i;
  // }

  // setup mapper
  setup_mapper();

  setup_mover();
  setup_mover_post(); // must be called afer previous

  setup_dma_in();
  // setup_dma_out();
  setup_fill_level();

  ring_block_send_eth(0xdead); // boot

  // debug ringbus out row addresses of all memory for easy lookup into cs20.out (row+1 = linenumber)
  ring_block_send_eth(input_dma_ptr);
  ring_block_send_eth(scratch_dma_ptr);
  ring_block_send_eth(debug_dma_ptr);
  ring_block_send_eth(MAPPER_DEST_ROW);
  ring_block_send_eth(VMEM_ROW_ADDRESS(mover_output));
  ring_block_send_eth(VMEM_ROW_ADDRESS(REVERSE_VMEM_DMA_ADDRESS(SCRATCH_DMA)));
  ring_block_send_eth(DST_ROW);
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

  unsigned int ben_counter = 0;

  int flushing = -1; // determines where the nextstate goes in FINISH_STATE

  unsigned int debug_counter = 0xf0000000;

  while(1) {
    pet_dma_in();
    pet_dma_in();
    pet_dma_in();
    pet_dma_in();
    // pet_dma_out();

    pet_fill_level(); // this can be called in other places, maybe where a blocking loop is


    // 0x300000 is sets two upper most bits of GPIO
    // CSR_WRITE(GPIO_WRITE, (0x300000) | fsm_state);

    //Want this to timeout if no dma is rcved. Do something else and try again. 

    //Send a heartbeat back to eth

    // if (fsm_state==INIT_STATE) {
    //   // initially set up a dma
    //   // dma_in_set(input_dma_ptr, 16);
    //   // CSR_WRITE(GPIO_WRITE, (0x200000) | 0);
    // }

    if (fsm_state==MAP_STATE) {

      // previous state (WAITING_STATE) guarantees that there is 1 or more item in this cirbuf
      // so we just grab it
      error = circular_buf_get(&dma_in_buffer, &dma_idx_just_finished); MY_ASSERT(error == 0);

      if( error == 0 ) {
        needs_mapping_dma_ptr = dma_idx_to_ptr(dma_idx_just_finished);




#ifndef USE_FAKE_MOVER_INPUT

        //Got 16 words, lets map bpsk
        // this stage takes an input from a changing address (dependin on which chunk we are doing)
        // however the output always goes to the same address (a single buffer)

        // CSR_READ(TIMER_VALUE, a);

        for (unsigned int i = 0; i < 16; i++) {  // 16 here is rows of mapped data.
            // 2nd argument goes by stride of 16
            // 3rd argument inches along 1 by 1

            // 3rd argument is a dma pointer to input memory
            mapper_load_qpsk(&qpsk_table, ( (i*NSLICES) + (MAPPER_DEST_ROW*NSLICES) ), i+needs_mapping_dma_ptr);
            // mapper_load_qpsk(&qpsk_table, ( (i*NSLICES) + (MAPPER_DEST_ROW*NSLICES) ), i+debug_dma_ptr);

            // mapper_load_qpsk(&qpsk_table, ( (i*NSLICES) + (MAPPER_DEST_ROW*NSLICES) ), debug_dma_ptr);
            // debug_counter++;
        }

        // CSR_READ(TIMER_VALUE, b);

        // ring_block_send_eth(0x0c000000 | b-a);

#else

        unsigned int mapper_output_dma_ptr = (MAPPER_DEST_ROW*NSLICES);
        for(unsigned int i = 0; i < NSLICES*DMA_IN_SIZE; i++) {
          vector_memory[i+mapper_output_dma_ptr] = debug_counter;
          debug_counter++;
        }
#endif


        CSR_WRITE(GPIO_WRITE, (0x200000) | 1);
      } else {
        CSR_WRITE(GPIO_WRITE, (0x200000) | 2); // error condition
      }

    }

    if (fsm_state==MOVE_STATE) {



      // spin until previous output dma is done (one by one)
      while(1) {
        CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, outgoing_dma_occupancy);
#ifdef USE_DOUBLE_BUFFER
        if(outgoing_dma_occupancy < 2) {
#else
        if(outgoing_dma_occupancy == 0) {
#endif
          break;
        } else {
          pet_dma_in();
          pet_fill_level();
        }
      }

#ifdef USE_OLD_MOVER_SCHEDULE_FORMAT
      for(unsigned int i = 0; i < number_active_schedules; i++) {
        mover_schedule(schedules[i], SCRATCH_DMA);
        mover_roll(output_frame_count);
      }

#ifdef USE_DOUBLE_BUFFER
#pragma GCC error "double buffer not supported when USE_OLD_MOVER_SCHEDULE_FORMAT is set"
#endif

#else

#ifdef USE_DOUBLE_BUFFER
      if( mover_working_on == 0) {
        
        // move into beginning of buffer
        for(unsigned int i = 0; i < number_active_schedules; i++) {
          mover_load_vmem(&(vmem_schedules[i]));
          mover_roll(output_frame_count);
        }
      } else {

        // move into offset of buffer
        for(unsigned int i = 0; i < number_active_schedules; i++) {
          mover_load_offset_output( &(vmem_schedules[i]), mover_output_increment_row);
          mover_roll(output_frame_count);
        }
      }
#else
      for(unsigned int i = 0; i < number_active_schedules; i++) {
          mover_load_vmem(&(vmem_schedules[i]));
          mover_roll(output_frame_count);
        }
#endif

#endif
      // exit2(0);

      // CSR_READ(TIMER_VALUE, d);

      // ring_block_send_eth(0x0d000000 | d-c);
      

      CSR_WRITE(GPIO_WRITE, (0x200000) | 2);
    
    }

    if (fsm_state==TX_STATE) {
      // trigger output

      // mover_output_increment_words
      // mover_output_increment_row


#ifdef USE_DOUBLE_BUFFER
      // schedule buffer "A" for output dma
      if( mover_working_on == 0 ) {
        dma_out_set(DST_ROW*NSLICES, FFT_SIZE*output_frame_count);
        mover_working_on = 1;
      } else {
        // buffer "B"
        dma_out_set(DST_ROW*NSLICES+mover_output_increment_words, FFT_SIZE*output_frame_count);
        mover_working_on = 0;
      }
#else
      dma_out_set(DST_ROW*NSLICES, FFT_SIZE*output_frame_count);
#endif





      CSR_WRITE(GPIO_WRITE, (0x200000) | 3);
    }

    if(fsm_state==FINISH_STATE) {
      
    }

    switch(fsm_state) {

       case INIT_STATE:
           fsm_state = WAITING_STATE;
           break;
           
           // we wait in this state, and check the size of the dma_in_buffer
           // as soon as there is something there, we start working on it
           // this value should never reach zero during "normal operation"
           // if it does, we are in an underflow condition
       case WAITING_STATE:
            incomming_occupancy = circular_buf_occupancy(&dma_in_buffer);

            if(incomming_occupancy > 0) {
               fsm_state = MAP_STATE;
              // debug_readout(16);
              flushing = 1;
             }
           break;
       
       case MAP_STATE:
           fsm_state = MOVE_STATE;
           break;

       case MOVE_STATE:
           fsm_state = TX_STATE;
           break;

       case TX_STATE:
           // CSR_WRITE(GPIO_WRITE, (0x200000) | 5);
           fsm_state = FINISH_STATE;
           break;     

       case FINISH_STATE:
           // CSR_WRITE(GPIO_WRITE, (0x200000) | 4);
           // rtn = dma_out_check(128);

           // if (rtn) {
             // CSR_WRITE(GPIO_WRITE, (0x200000) | 6);
            if(flushing >= 0) {
              flushing--;
              fsm_state = MAP_STATE;
            } else {
              flushing = -1; // disable
              fsm_state = WAITING_STATE;
            }
           // } 
           break;

       default: /* Optional */
           fsm_state = INIT_STATE;
           break;
    }

    x4 = fsm_state;

  }

}

