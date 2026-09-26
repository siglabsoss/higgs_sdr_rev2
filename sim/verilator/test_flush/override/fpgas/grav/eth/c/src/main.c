#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_ETH
#include "ringbus2_post.h"

#define DMA_IN_COUNT (4)
#define DMA_IN_LEN (2)
#define DMA_IN_OFFSET (0)
#define DMA_IN_START(x) (DMA_IN_OFFSET+(DMA_IN_LEN*(x)))
unsigned int dma_in_last = DMA_IN_COUNT;
unsigned int dma_in_last_consumed = DMA_IN_COUNT;

// 4 is probably too big for output but we do not care
#define DMA_OUT_COUNT (4)
#define DMA_OUT_LEN (1)
// start dma at 8, but we give ourselves 1 bank wrap around of buffer (wasted space)
// the DMA banks should never be operating on the same memory (at time of writing)
#define DMA_OUT_OFFSET (8 + 32)
#define DMA_OUT_START(x) (DMA_OUT_OFFSET+(DMA_OUT_LEN*(x)))
#define DMA_OUT_NEXT_INDEX() ((dma_out_last+1) & 0x3)
// unsigned int dma_out_last = DMA_OUT_COUNT;
// unsigned int dma_out_last_consumed = DMA_OUT_COUNT;

unsigned int ring_send_pending;


void trig_in(unsigned int dma_in_next)
{

  CSR_WRITE(DMA_0_START_ADDR, DMA_IN_START(dma_in_next));
  CSR_WRITE(DMA_0_LENGTH, DMA_IN_LEN);
  CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff);  // start right away
  CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0); // any value
}


void setup_dma_in(void) {
  register volatile unsigned int x3 asm("x3");
  register volatile unsigned int x4 asm("x4");
  x4 = 0xf0;
  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, x3);
  trig_in(0);
  x4 = 0xf1;
  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, x3);
  trig_in(1);
  x4 = 0xf2;
  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, x3);
  trig_in(2);
  x4 = 0xf3;
  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, x3);
  trig_in(3);
  x4 = 0xf4;
  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, x3);
}

void handle_single_dma_in(void)
{
  unsigned int dma_in_expected_occupancy = 3;
  register volatile unsigned int check_dma asm("x3");
  unsigned int occupancy;
  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
  unsigned int i;
  // check_dma = occupancy;
  for(i=occupancy;i<dma_in_expected_occupancy;i++) {
    // trig_in(0); // this can be earler for better performance
    ring_send_pending++;
    // check_dma = 0xd0000000 | ring_send_pending;
  }
}



void pre_delay() {
  int d1 = 30; // 20 works, 10 fails
    for(int i = 0; i < d1; i++) {

    }
}

int main(void)
{
  register volatile unsigned int x3 asm("x3");
  register volatile unsigned int x4 asm("x4");
  // turns out cs20 has a reset that goes longer than others
  // as a result of this, because we are the upstream from cs20, we need to burn cycles
  // before sending our first message or else the ringbus goes unstable

  unsigned int total = 0;

  dma_in_last_consumed = 0;

  pre_delay();

  setup_dma_in();

  int h1;
  while(1) {
    CSR_READ(mip, h1);

    // handle_single_dma_in();

    CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, x3);

    if(h1 & DMA_0_ENABLE_BIT) {
      CSR_WRITE(DMA_0_INTERRUPT_CLEAR, 0);

      
      int mem_start = DMA_IN_START(dma_in_last_consumed);
      unsigned int addr;
      unsigned int data_packet;
      addr = vector_memory[mem_start + 0] & 0xff;
      data_packet = vector_memory[mem_start + 1];

      // x3 = addr;
      x4 = data_packet;

      ring_send_pending--;
      dma_in_last_consumed++;
      total++;
    }

    

    if(total == 2) {
      x3 = 0;
      x4 = 0;
      // reset of sorts

      // enabling or disabling this shows a "flaw" in DMA_0_SCHEDULE_OCCUPANCY
      if(1) {
        CSR_WRITE(DMA_0_FLUSH_SCHEDULE,  1);
        setup_dma_in();
        dma_in_last_consumed = 0;
      }


      total++; // not really accurate
    }

  }
}

