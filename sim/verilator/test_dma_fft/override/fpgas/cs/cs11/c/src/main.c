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
#define OUTPUT_FRAME_COUNT (32)
#define FFT_SIZE (1024)

#define OUR_RING_ENUM RING_ENUM_CS20
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


#define MY_ASSERT(x) if(!(x)) { ring_block_send_eth(0xe0000000|__LINE__);}



register volatile unsigned int x3 asm("x3");
register volatile unsigned int x4 asm("x4");


/*111:row 6, column 15
122:row 7, column 10
133:row 8, column 5
144:row 9, column 0
866:row 54, column 2
877:row 54, column 13
888:row 55, column 8
899:row 56, column 3*/


Table qpsk_table;
Table bpsk_table;

#define MOVER_SRC_ROW          (mapper_output_row)
#define MAPPER_DEST_ROW        (mapper_output_row)

unsigned int mapper_output_row;

unsigned int qpsk_table_dma;
unsigned int bpsk_table_dma;

void setup_mapper(void) {
  qpsk_table_dma = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));
  qpsk_table = mapper_qpsk_table(qpsk_table_dma);
  
  bpsk_table_dma = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));
  bpsk_table = mapper_bpsk_table(bpsk_table_dma);

  // output of mapper, input to mover
  mapper_output_row = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
}


Schedule sch1 [NSLICES];
Schedule sch2 [NSLICES];

#define GARBAGE_ROW       (garbage_row)
#define SCRATCH_DMA       (scratch_dma_ptr)
#define DST_ROW           (VMEM_ROW_ADDRESS(mover_output))

unsigned int garbage_row;

// output of the mapper needs 16 * 1024 words of memory in a row
// currently vmalloc is not great for large blocks of memory, static allocaiton
// will work for now
VMEM_SECTION unsigned int mover_output[FFT_SIZE*OUTPUT_FRAME_COUNT] = {};

void setup_mover(void) {
  garbage_row = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));

  sch1[0] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0x3 << 12) | (DST_ROW+9), 128};    //vector column 0
  sch1[1] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0x0 << 12) | (GARBAGE_ROW), 0};  //vector column 1    
  sch1[2] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0x2 << 12) | (DST_ROW+54), 128};   //vector column 2  
  sch1[3] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0x4 << 12) | (DST_ROW+56), 128};   //vector column 3  
  sch1[4] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0x0 << 12) | (GARBAGE_ROW), 0};  //vector column 4
  sch1[5] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0xd << 12) | (DST_ROW+8), 128};    //vector column 5
  sch1[6] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0x0 << 12) | (GARBAGE_ROW), 0};  //vector column 6 
  sch1[7] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0x0 << 12) | (GARBAGE_ROW), 0};  //vector column 7 
  sch1[8] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0xe << 12) | (DST_ROW+55), 128};   //vector column 8 
  sch1[9] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0x0 << 12) | (GARBAGE_ROW), 0};  //vector column 9 
  sch1[10] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0x7 << 12) | (DST_ROW+7), 128};    //vector column 10 
  sch1[11] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0x0 << 12) | (GARBAGE_ROW), 0};  //vector column 11
  sch1[12] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0x0 << 12) | (GARBAGE_ROW), 0};  //vector column 12 
  sch1[13] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0x8 << 12) | (DST_ROW+54), 128};   //vector column 13
  sch1[14] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0x0 << 12) | (GARBAGE_ROW), 0};  //vector column 14
  sch1[15] =    (Schedule) {MOVER_SRC_ROW, 0x1, (0x1 << 12) | (DST_ROW+6), 128};


  sch2[0] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((11) << 12) | (64+DST_ROW+9), 128};    //vector column 0
  sch2[1] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((0) << 12) | (GARBAGE_ROW), 0};  //vector column 1    
  sch2[2] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((10) << 12) | (64+DST_ROW+54), 128};   //vector column 2  
  sch2[3] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((12) << 12) | (64+DST_ROW+56), 128};   //vector column 3  
  sch2[4] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((0) << 12) | (GARBAGE_ROW), 0};  //vector column 4
  sch2[5] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((5) << 12) | (64+DST_ROW+8), 128};    //vector column 5
  sch2[6] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((0) << 12) | (GARBAGE_ROW), 0};  //vector column 6 
  sch2[7] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((0) << 12) | (GARBAGE_ROW), 0};  //vector column 7 
  sch2[8] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((6) << 12) | (64+DST_ROW+55), 128};   //vector column 8 
  sch2[9] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((0) << 12) | (GARBAGE_ROW), 0};  //vector column 9 
  sch2[10] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((15) << 12) | (64+DST_ROW+7), 128};    //vector column 10 
  sch2[11] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((0) << 12) | (GARBAGE_ROW), 0};  //vector column 11
  sch2[12] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((0) << 12) | (GARBAGE_ROW), 0};  //vector column 12 
  sch2[13] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((0) << 12) | (64+DST_ROW+54), 128};   //vector column 13
  sch2[14] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((0) << 12) | (GARBAGE_ROW), 0};  //vector column 14
  sch2[15] =    (Schedule) {MOVER_SRC_ROW, 0x1, ((9) << 12) | (64+DST_ROW+6), 128};
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

#define DMA_SCHEDULE_OUT_SIZE (4+1)
circular_buf_t dma_schedule_out;
unsigned int dma_schedule_out_storage[DMA_SCHEDULE_OUT_SIZE];

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

void setup_dma_out(void) {
  circular_buf_initialize(&dma_schedule_out, dma_schedule_out_storage, DMA_SCHEDULE_OUT_SIZE);
}

// unsigned int fake_work_todo = 0;

void pet_dma_inqueue(void) {
  int error;
  unsigned int just_finished_idx;
  unsigned int dma_occupancy;

  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, dma_occupancy);
  unsigned int filled = circular_buf_occupancy(&dma_schedule_in);

  // for(unsigned int i = 0; i < 100; i++) {
  //   STALL(20);
  // }


  if( dma_occupancy != filled ) {
    // just_finished_idx is the index of the dma that just finished
    error = circular_buf_get(&dma_schedule_in, &just_finished_idx); MY_ASSERT(error == 0);

    // now that dma is done with this chunk, we add it to the next
    // circular buffer which signals the program that there is fresh data to be processed
    circular_buf_put(&dma_in_buffer, just_finished_idx);

    trig_dma_in_next();
  }

}


void pet_dma_in(void) {
  pet_dma_inqueue();
}


#define FILL_UNDERFLOW  (0x1 << 16)
#define FILL_OVERFLOW   (0x1 << 17)

#define LESS_THAN_UNDERFLOW (1)
#define GREATER_THAN_OVERFLOW (62)

#define FILL_LEVEL_REPORT_TIME (200000000)

unsigned int report_timer, report_timer_check, fill_flags;

void setup_fill_level(void) {
  // fill level update timer, ringbus takes about 500 counter ticks to write
  // so this update should be much slower then that
  CSR_READ(TIMER_VALUE, report_timer);
  fill_flags = 0;
}

void pet_fill_level(void) {
  Ringbus ringbus;

  unsigned int fill;

  CSR_READ(TIMER_VALUE, report_timer_check);

  fill = circular_buf_occupancy(&dma_in_buffer);

  // check for under/over every time
  if(fill <= LESS_THAN_UNDERFLOW) {
    fill_flags |= FILL_UNDERFLOW;
  }

  if(fill >= GREATER_THAN_OVERFLOW) {
    fill_flags |= FILL_OVERFLOW;
  }

  // report flags slowly
  if( report_timer_check >= (report_timer+FILL_LEVEL_REPORT_TIME) ) {
    ring_block_send_eth(0x0b000000 | fill_flags | fill);
    report_timer = report_timer_check;

    check_ring(&ringbus); // check bootloader at same rate we report

    fill_flags = 0;
  }
}


int main(void)
{
  unsigned int rtn;

  // setup vmalloc
  init_VMalloc(&mgr);


  unsigned int* input_cpu_ptr =  vmalloc_single(&mgr);
  unsigned int input_dma_ptr = VMEM_DMA_ADDRESS(input_cpu_ptr);

  unsigned int* scrach_cpu_ptr = vmalloc_single(&mgr);
  unsigned int scratch_dma_ptr = VMEM_DMA_ADDRESS(scrach_cpu_ptr);

  unsigned int* debug_cpu_ptr =  vmalloc_single(&mgr);
  unsigned int debug_dma_ptr = VMEM_DMA_ADDRESS(debug_cpu_ptr);

  for(unsigned int i = 0; i < 16; i++) {
    debug_cpu_ptr[i] = 0xa0000000 | i;
  }


  // dump our input and reset
  CSR_WRITE(DMA_0_START_ADDR, dma_idx_to_ptr(0));
  CSR_WRITE(DMA_0_LENGTH, 1024);
  CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff); // start right away
  CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);
  CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);

  for(unsigned int i = 0; i < 10000; i++) {
    STALL(1);
  }

  CSR_WRITE(DMA_0_FLUSH_SCHEDULE,  0);
  CSR_WRITE(DMA_1_FLUSH_SCHEDULE,  0);




  // for(unsigned int i = 0; i < 1024; i++) {
  //   vector_memory[i+input_dma_ptr] = 0xf000d000 + i;
  // }

  // setup mapper
  setup_mapper();

  setup_mover();

  setup_dma_in();
  // setup_dma_out();
  setup_fill_level();

  ring_block_send_eth(0xdead); // boot

  ring_block_send_eth(input_dma_ptr);
  ring_block_send_eth(scratch_dma_ptr);
  ring_block_send_eth(debug_dma_ptr);

  // debug ringbus out row addresses of all memory for easy lookup into cs20.out (row+1 = linenumber)
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
      error = circular_buf_get(&dma_in_buffer, &dma_idx_just_finished); //MY_ASSERT(error == 0);

      needs_mapping_dma_ptr = dma_idx_to_ptr(dma_idx_just_finished);

      //Got 16 words, lets map bpsk
      // this stage takes an input from a changing address (dependin on which chunk we are doing)
      // however the output always goes to the same address (a single buffer)
      for (unsigned int i = 0; i < 16; i++) {
          // 2nd argument goes by stride of 16
          // 3rd argument inches along 1 by 1

          // 3rd argument is a dma pointer to input memory
          // mapper_load_qpsk(&qpsk_table, ( (i*NSLICES) + (MAPPER_DEST_ROW*NSLICES) ), i+needs_mapping_dma_ptr);
          mapper_load_qpsk(&qpsk_table, ( (i*NSLICES) + (MAPPER_DEST_ROW*NSLICES) ), i+debug_dma_ptr);
      }
      CSR_WRITE(GPIO_WRITE, (0x200000) | 1);

    }

    if (fsm_state==MOVE_STATE) {

      // spin until previous output dma is done (one by one)
      while(1) {
        CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, outgoing_dma_occupancy);
        if(outgoing_dma_occupancy == 0) {
          break;
        } else {
          pet_dma_in();
          pet_fill_level();
        }
      }


      mover_schedule(sch1, SCRATCH_DMA);
      mover_roll(16);

      mover_schedule(sch2, SCRATCH_DMA);
      mover_roll(16);
      CSR_WRITE(GPIO_WRITE, (0x200000) | 2);
    
    }

    if (fsm_state==TX_STATE) {
      // // spin until output dma is free
      // while(1) {
      //   CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, outgoing_dma_occupancy);
      //   if(occupancy < RINGBUS_SCHEDULE_DEPTH) {
      //     break;
      //   }
      // }
      // trigger output
      dma_out_set(DST_ROW*NSLICES, FFT_SIZE*OUTPUT_FRAME_COUNT);
      // if( ben_counter == 10000) {
      //   dma_out_set(DST_ROW*NSLICES, 1);
      // ben_counter=0;
      // }
      // ben_counter++;
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
            // incomming_occupancy = circular_buf_occupancy(&dma_in_buffer);

            // if(incomming_occupancy != 0) {
               fsm_state = MAP_STATE;
             // }
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
            fsm_state = WAITING_STATE;
           // } 
           break;

       default: /* Optional */
           fsm_state = INIT_STATE;
           break;
    }

    x4 = fsm_state;

  }

}

