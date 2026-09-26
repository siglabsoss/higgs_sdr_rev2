#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "vmalloc.h"
#include "circular_buffer.h"
#include "fill.h"
#include "ringbus.h"

#include "flush_config_word_data.h"
#include "fft_1024_3914.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS10
#include "ringbus2_post.h"


#define MY_ASSERT(x) if(!(x)) { ring_block_send_eth(0xe0000000|__LINE__);}


void fft_accept_new(unsigned int dma_ptr);
void dma_out_set_safe(unsigned int dma_ptr, unsigned int size);

// declare as global
VMalloc mgr;

#define DMA_IN_CHUNK 1024

// #define FFT_CP_SAMPLES (384) // works (3/8)
// #define FFT_CP_SAMPLES (320) // works (5/16)
// #define FFT_CP_SAMPLES (288) // works (9/32)
#define FFT_CP_SAMPLES (256) // works (1/4)
// #define FFT_CP_SAMPLES (128) // does not work


// must be power of two, must change next as well
#define DMA_IN_COUNT (4)
#define DMA_IN_COUNT_MASK 0x3


// 0 a
// 1 b
int dma_state = 0;
int dma_in_valid = -1;
unsigned int dma_in_ptr[2];
int fft_a_empty;
int fft_b_empty;

int fft_ready = -1;
int fft_valid = -1;
unsigned int fft_ptr[2];


int dma_out_valid = -1;
int dma_out_ready = -1;

#define DMA_OUT_CIRBUF_SIZE (4)
circular_buf_t dma_out_started;
unsigned int dma_out_started_storage[DMA_OUT_CIRBUF_SIZE+1];

// 
void trig_dma_in(unsigned int idx, unsigned int timer_start) {
  // dma_in_set(VMEM_DMA_ADDRESS(dma_in_ptr[idx]), DMA_IN_CHUNK);

  // static unsigned int timer_start = 4096;

  CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx]));
  CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK);
  CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
  CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value

  // timer_start += 4096;
}

void setup_dma_in(void) {
  dma_in_ptr[0] = vmalloc_single(&mgr);
  dma_in_ptr[1] = vmalloc_single(&mgr);


  trig_dma_in(0, 0xffffffff);
  trig_dma_in(1, 0xffffffff);
}

fft1024_t active_plan;

void setup_fft(void) {
  fft_ptr[0] = vmalloc_single(&mgr);
  fft_ptr[1] = vmalloc_single(&mgr);
  fft_a_empty = 1;
  fft_b_empty = 1;

  active_plan = get_fft1024_plan(0, 0);
}

void setup_dma_out(void) {

  dma_out_started.size = DMA_OUT_CIRBUF_SIZE+1;
  dma_out_started.buffer = dma_out_started_storage;
  circular_buf_reset(&dma_out_started);
}

void pet_dma_in(void) {
  unsigned int occupancy;
  int error;
  unsigned int data;
  unsigned int helper;
  unsigned int set_pace = 0;
  static unsigned int pace;

  CSR_READ(mip, helper);

  if(helper & DMA_0_ENABLE_BIT) {
    CSR_WRITE(DMA_0_INTERRUPT_CLEAR, 0);
    CSR_WRITE(GPIO_WRITE, (1<<8) | dma_state) ;

    dma_in_valid = dma_state; // signal a buffer index downstream

    dma_state = (dma_state+1)&0x1;
  }

  if(fft_ready != -1) {
    // if(set_pace == 0) {
    //   CSR_READ(TIMER_VALUE, pace);
    //   pace += 8192; // in the future with a slightly shorter buffer
    //   set_pace = 1;
    // } else {
    //   pace += 8192;
    // }

    trig_dma_in(fft_ready, 0xffffffff); // pace
    CSR_WRITE(GPIO_WRITE, (2<<8) | fft_ready);
    fft_ready = -1;

  }


}

void pre_pet_fft() {
  if(dma_out_ready != -1) {
    // Output dma is telling us that a buffer has finished going out

    // could check for error here if it's already empty
    // fixme what is a cleaner way to do this?
    if(dma_out_ready == 0) {
      fft_a_empty = 1;
    }
    if(dma_out_ready == 1) {
      fft_b_empty = 1;
    }

    CSR_WRITE(GPIO_WRITE, (0x4a << 8) | dma_out_ready);
    CSR_WRITE(GPIO_WRITE, (0x4b << 8) | (fft_a_empty << 1) | fft_b_empty);

    dma_out_ready = -1;

  }
}


void pet_fft() {
  // example only starts to work once we have 2 items in the queue

  int error;
  static unsigned updates = 1;

  // deals with dma telling us we are done
  pre_pet_fft();

  unsigned int output_blocked = 0;


  // deals with dma telling us that we have new data to consume
  if( dma_in_valid != -1 )
  {
    ///////
    //
    // when set to 0, it means buffer A
    // buffer A goes into our buffer C
    //
    int consume_idx = dma_in_valid;

    if(dma_in_valid == 0) {
      if( fft_a_empty == 0) {
        // this means incoming data came too soon, we were still working
        // ring_block_send_eth(0x2a000000);
        return; // early
        // error
      }
      fft_a_empty = 0;
    }
    if(dma_in_valid == 1) {
      if( fft_b_empty == 0) {
        // error
        // ring_block_send_eth(0x2b000000);
        return; // early
      }
      fft_b_empty = 0;
    }

    CSR_WRITE(GPIO_WRITE, (3 << 8) | (fft_a_empty << 1) | fft_b_empty);

    unsigned int* cpu_ptr_from_dma = dma_in_ptr[consume_idx];
    unsigned int* cpu_ptr_fft = fft_ptr[consume_idx];


    active_plan.data_location   = VMEM_ROW_ADDRESS(cpu_ptr_from_dma);  //input
    active_plan.data_location_0 = VMEM_ROW_ADDRESS(cpu_ptr_fft);       //output 

    // CSR_WRITE(GPIO_WRITE, 0x30);

    fft_1024_run(&active_plan);

    CSR_WRITE(GPIO_WRITE, (0x3a<<8) | consume_idx );

    // reset signal we consumed
    dma_in_valid = -1;
    // signal back, this releases our reliance on the input that dma gave us, meaning dma is free to erase it
    fft_ready = consume_idx; // Release A to be over written

    fft_valid = consume_idx; // send C on to be DMA'd out
  }

  

}



// similar to just calling dma_out_set
void dma_out_set_safe(unsigned int dma_ptr, unsigned int size) {

 
  unsigned int occupancy;
  // unsigned int occupancy_busy;
  // unsigned int occupancy_combined1, occupancy_combined2;
  while(1) {
    CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
    if(occupancy < DMA_1_SCHEDULE_DEPTH) {
      break; // should break on first go
    } else {
      // stuck, can report this with ringbus
      ring_block_send_eth(0xd0000000);
    }
  }
  
  dma_out_set(dma_ptr, size);
}

#define IS_SECOND_DMA (0x2)
#define DMA_A_B_MASK (0x1)

unsigned int dma_out_extra = 0;

void pet_dma_out(void) {
  int helper;
  unsigned int occupancy;
  int error;
  unsigned int data;
  unsigned int output_blocked = 0;
  unsigned int dma_occupancy_combined;
  unsigned int dma_occupancy;
  unsigned int dma_occupancy_status;

  // we can tell by the size of our cirbuf if we have room 
  // for instance we get a 3 here after 0x500, 0x501, 0x500 meaning that ...
  if(fft_valid != -1) {
    occupancy = circular_buf_occupancy(&dma_out_started);
    CSR_WRITE(GPIO_WRITE, (7<<8) | occupancy );
    if( occupancy > 2 ) {
      output_blocked = 1;
    } else {
      output_blocked = 0;
    }
  }

  // handles setting dma's
  if( (!output_blocked) && (fft_valid != -1) ) {
    ///////
    //
    // when set to 0, it means buffer C is being given to us by FFT
    // we fire off output dma and then wait
    //
    int consume_idx = fft_valid;

    CSR_WRITE(GPIO_WRITE, (0x5<<8) | consume_idx);

    unsigned int* cpu_ptr_from_fft = fft_ptr[consume_idx];

    // remember which dma and in which order is running
    // write two entries into cirbuf, when we pull out the 2nd one (downbelow)
    // we will know that incoming "C" buffer is free

    error = circular_buf_put(&dma_out_started, consume_idx); MY_ASSERT(error == 0);
    error = circular_buf_put(&dma_out_started, consume_idx | IS_SECOND_DMA); MY_ASSERT(error == 0);



    // schedule the CP
    dma_out_set_safe(VMEM_DMA_ADDRESS(cpu_ptr_from_fft)+(DMA_IN_CHUNK-FFT_CP_SAMPLES), FFT_CP_SAMPLES);

    // schedule the full FFT
    dma_out_set_safe(VMEM_DMA_ADDRESS(cpu_ptr_from_fft), DMA_IN_CHUNK);


    fft_valid = -1; // set this to -1 so we don't get caught
  }


  // handles when a dma is finished
  // we will get two interrupts for a given buffer because we scheduled twice (CP)
  // each time we pull a value from the cirbuf letting us know what just finished
  // if the value has the IS_SECOND_DMA set, then we know the buffer is not needed again
  // CSR_READ(mip, helper);
  CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, dma_occupancy);
  unsigned int filled = circular_buf_occupancy(&dma_out_started);

  if( dma_occupancy != filled ) {

    CSR_WRITE(DMA_1_INTERRUPT_CLEAR, 0);
    // using a cirbuf we remember which dma was put first
    error = circular_buf_get(&dma_out_started, &data); MY_ASSERT(error == 0);
    CSR_WRITE(GPIO_WRITE, (6<<8) | data );

    if(error == 0) {
      if( data & IS_SECOND_DMA ) {
        // signal back to fft that output dma is done
        // only runs one for every 2 interrupts
        dma_out_ready = data & DMA_A_B_MASK; 

        pre_pet_fft();
      }
    } else {
      // oh boy
      // not exactly sure how we are here but we are checking extra times for dma out being done
      // we can avoid this with better magic math above
    }

  }

}



int main(void) {
  init_VMalloc(&mgr);
  // unsigned int burn = 16;
  // for(unsigned int i = 0)

  CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);

  

  setup_dma_in();
  setup_fft();
  setup_dma_out();

  unsigned int counter = 0;
  CSR_WRITE(GPIO_WRITE, 0xdeadbeef);

  // ring_block_send_eth(dma_in_ptr[0]);
  // ring_block_send_eth(dma_in_ptr[1]);
  // ring_block_send_eth(fft_ptr[0]);
  // ring_block_send_eth(fft_ptr[1]);

  Ringbus ringbus;

  while(1) {
    pet_dma_in();
    pet_fft();
    pet_dma_out();
    pet_dma_out();
    pet_dma_out();
    pet_dma_out();

    if(counter == 2000) {
      check_ring(&ringbus);
      // unsigned int mem_free = vmalloc_available(&mgr);
      // ring_block_send_eth(0xc0000000 | mem_free);
      counter = 0;
    }
    counter++;
  }
  


  // unsigned int dma_in_index;

  // unsigned int* in_a = vmalloc_single(&mgr);
  // unsigned int* in_b = vmalloc_single(&mgr);




} 