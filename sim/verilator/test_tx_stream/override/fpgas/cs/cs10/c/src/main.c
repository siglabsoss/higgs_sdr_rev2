#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "ringbus.h"
#include "symbol.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS10
#include "ringbus2_post.h"

#include "unit_test_ring.h"


void report_test_results(unsigned int data)
{
  // put the value of pass_fail_0 into the
  // vector memory at a high address
  unsigned int occupancy;

  while(1)
  {
   CSR_READ(RINGBUS_SCHEDULE_OCCUPANCY, occupancy);
   if(occupancy < RINGBUS_SCHEDULE_DEPTH)
   {
    break;
   }
  }

  CSR_WRITE(RINGBUS_WRITE_ADDR, RING_ADDR_PC);
  CSR_WRITE(RINGBUS_WRITE_DATA, data);
  CSR_WRITE(RINGBUS_WRITE_EN, 0);
}


void dma_push_block_schedule(unsigned int dma_ptr, unsigned int word_count) {
  unsigned int occupancy;
  while(1) {
    CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
    if( occupancy < DMA_1_SCHEDULE_DEPTH) {
      break;
    }
  }
  CSR_WRITE(DMA_1_START_ADDR, dma_ptr);
  CSR_WRITE(DMA_1_LENGTH, word_count);
  CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);  // start right away
  CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0); // any value
}

#define PLAYBACK_FRAMES (2)

void playback_once()
{
  unsigned int frames = PLAYBACK_FRAMES;
  unsigned int frame_size = 1024;
  for(unsigned int i = 0; i < frames; i++) {
    // instead of CP, just double it for now
    dma_push_block_schedule(i*frame_size, frame_size);
    dma_push_block_schedule(i*frame_size, frame_size);
  }
}

unsigned int program_mode;

void cs10_mode_cb(unsigned int data) {
    // static int times_called = 0;
    // report_test_results(++times_called);
    program_mode = data;
    // register volatile unsigned int x3 asm("x3") __attribute__((unused));  // for debug
    // register volatile unsigned int x4 asm("x4") __attribute__((unused));  // for debug
    // x4 = data;

    if(program_mode == 3 || program_mode == 4) {
      CSR_WRITE(DMA_0_FLUSH_SCHEDULE,  0); // flush when enter
    }
}

int main(void)
{
  CSR_WRITE(DMA_0_FLUSH_SCHEDULE,  0);
  CSR_WRITE(DMA_1_FLUSH_SCHEDULE,  0);

  // clear int
  CSR_WRITE(DMA_0_INTERRUPT_CLEAR, 0);
  CSR_WRITE(DMA_1_INTERRUPT_CLEAR, 0);

  CSR_WRITE(GPIO_WRITE_EN, LED_GPIO_BIT);
  CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);
  // required for vmalloc to work
 
  Ringbus ringbus;

  program_mode = 4;
  ring_register_callback(&cs10_mode_cb, CS10_PLAYBACK);

  report_test_results(OUR_RING_ENUM);

  int pm1trig = 0;
  int helper;

  while(1)  {
    // blink and accept bootload / commands
    if(program_mode == 0) {
      for(unsigned int i = 0; i < 10; i++) {
        CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);
          check_ring(&ringbus);
        for(int j = 0; j < 100000; j++) {  // 10000
          check_ring(&ringbus);
        }
        CSR_WRITE(GPIO_WRITE, 0);
        for(int j = 0; j < 1000000; j++) {  // 1000000
          check_ring(&ringbus);
        }
      }
    } else if(program_mode == 1) {
      for(unsigned int i = 0; i < 10; i++) {
        CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);
          check_ring(&ringbus);
        for(int j = 0; j < 100000; j++) {  // 10000
          check_ring(&ringbus);
        }
        CSR_WRITE(GPIO_WRITE, 0);
        for(int j = 0; j < 10000000; j++) {  // 1000000
          check_ring(&ringbus);
        }
      }
    } else if(program_mode == 2) {
      for(unsigned int i = 0; i < 10; i++) {
        CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);
          check_ring(&ringbus);
        for(int j = 0; j < 10000; j++) {  // 10000
          check_ring(&ringbus);
        }
        CSR_WRITE(GPIO_WRITE, 0);
        for(int j = 0; j < 1000000; j++) {  // 1000000
          check_ring(&ringbus);
        }
      }
    } else if(program_mode == 3) {
      // drain dma 1 by 1 and report to host
      if( pm1trig == 0) {
        CSR_WRITE(DMA_0_START_ADDR, 0);
        CSR_WRITE(DMA_0_LENGTH, 1);
        CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff);  // start right away
        CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0); // any value
        pm1trig = 1;
      }

      if( pm1trig == 1 ) {
        CSR_READ(mip, helper);
        if(helper & DMA_0_ENABLE_BIT) {
          CSR_WRITE(DMA_0_INTERRUPT_CLEAR, 0);
          report_test_results(vector_memory[0]);
          pm1trig = 0;
        }
      }
      check_ring(&ringbus);

    } else if(program_mode == 4) {
      // set this to different values to determine how much memory to fill
      // the "play out" is still 
      unsigned int full_mem = 0x10000;
      full_mem = 1024*PLAYBACK_FRAMES;

      // setup intput dma
      CSR_WRITE(DMA_0_START_ADDR, 0);
      CSR_WRITE(DMA_0_LENGTH, full_mem);
      CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff);  // start right away
      CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0); // any value

      // wait till int (meaning input DMA is finished, (all of vmem is full))
      
      while(1) {
        CSR_READ(mip, helper);
        if(helper & DMA_0_ENABLE_BIT) {
          CSR_WRITE(DMA_0_INTERRUPT_CLEAR, 0);
          break;
        }
        check_ring(&ringbus);
      }

      report_test_results(0xcafe0000);

      for(unsigned int i = 0; i < full_mem; i++) {
        report_test_results(vector_memory[i]);
      }


      while(1) {
        playback_once();
        check_ring(&ringbus);
      }
    }
  }
}



int main2(void)
{
  CSR_WRITE(DMA_0_FLUSH_SCHEDULE,  0);
  CSR_WRITE(DMA_1_FLUSH_SCHEDULE,  0);

  CSR_WRITE(GPIO_WRITE_EN, LED_GPIO_BIT);
  CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);
  Ringbus ringbus;

  report_test_results(OUR_RING_ENUM);

  while(1) {
    CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);
      check_ring(&ringbus);
    for(int j = 0; j < 100000; j++) {  // 10000
      check_ring(&ringbus);
    }
    CSR_WRITE(GPIO_WRITE, 0);
    for(int j = 0; j < 1000000; j++) {  // 1000000
      check_ring(&ringbus);
    }
  }
  
}




int main3(void)
{
  Ringbus ringbus;
  register volatile unsigned int x3 asm("x3");
  register volatile unsigned int x4 asm("x4");

  unsigned int i;
  for(i = 0; i < 1024; i++)
  {
    vector_memory[i] = dmem_sin[i];
    // vector_memory[i] = 0x7000;
  }

  report_test_results(0x00101234);

  x3 = 0;

  unsigned int start_addr;
  unsigned int data_len;
  unsigned int start_time;

  start_addr = 0;
  data_len = 1024;
  start_time = 0xffffffff;

  CSR_WRITE(GPIO_WRITE_EN, LED_GPIO_BIT);
  CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);
  int led = 1;
  check_ring(&ringbus);

  unsigned int occupancy;
  unsigned int h1;

  CSR_WRITE(DMA_1_FLUSH_SCHEDULE, 0);
  while(1) {
    CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
    check_ring(&ringbus);
    if(occupancy < 2)
    {
      CSR_WRITE(DMA_1_START_ADDR, start_addr);
      CSR_WRITE(DMA_1_LENGTH, data_len);
      CSR_WRITE(DMA_1_TIMER_VAL, start_time);
      CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0);
      x3++;
      check_ring(&ringbus);
    }

    CSR_READ(mip, h1);
    if(h1 & DMA_1_ENABLE_BIT) {
      CSR_WRITE(DMA_1_INTERRUPT_CLEAR, 0);
      check_ring(&ringbus);
    }

    CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);
  }
}