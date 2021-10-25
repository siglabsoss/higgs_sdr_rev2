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
unsigned int dma_in_expected_occupancy;

// 4 is probably too big for output but we do not care
#define DMA_OUT_COUNT (4)
#define DMA_OUT_LEN (1)
// start dma at 8, but we give ourselves 1 bank wrap around of buffer (wasted space)
// the DMA banks should never be operating on the same memory (at time of writing)
#define DMA_OUT_OFFSET (8 + 32)
#define DMA_OUT_START(x) (DMA_OUT_OFFSET+(DMA_OUT_LEN*(x)))
#define DMA_OUT_NEXT_INDEX() ((dma_out_last+1) % DMA_OUT_COUNT)
unsigned int dma_out_last = DMA_OUT_COUNT;
// unsigned int dma_out_last_consumed = DMA_OUT_COUNT;

unsigned int ring_send_pending = 0;


void trig_in()
{
  register volatile unsigned int check_dma asm("x3");
  unsigned int dma_in_next = (dma_in_last+1) % DMA_IN_COUNT;


  check_dma = 0xa0000000 | DMA_IN_START(dma_in_next);

  CSR_WRITE(DMA_0_START_ADDR, DMA_IN_START(dma_in_next));
  CSR_WRITE(DMA_0_LENGTH, DMA_IN_LEN);
  CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff);  // start right away
  CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0); // any value

  dma_in_last = dma_in_next;
}

void trig_out()
{
  unsigned int dma_out_next = DMA_OUT_NEXT_INDEX();

  CSR_WRITE(DMA_1_START_ADDR, DMA_OUT_START(dma_out_next));
  CSR_WRITE(DMA_1_LENGTH, DMA_OUT_LEN);
  CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);  // start right away
  CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0); // any value

  dma_out_last = dma_out_next;
}

void to_ring(int index) {

  int mem_start = DMA_IN_START(index);

  unsigned int r1,r2;

  r1 = vector_memory[mem_start + 0] & 0xff;
  r2 = vector_memory[mem_start + 1];

  // unsigned int write_done;
  // while(1) {
  //     CSR_READ(RINGBUS_WRITE_DONE, write_done);
  //     if(write_done) {
  //       break;
  //     }
  //   }

  CSR_WRITE(RINGBUS_WRITE_ADDR, r1);
  CSR_WRITE(RINGBUS_WRITE_DATA, r2);
  CSR_WRITE(RINGBUS_WRITE_EN, 0);
}



void setup_dma_in(void) {
  trig_in();
  trig_in();
  trig_in();
  trig_in();
  // the core consumes a schedule when it starts running
  // so this number is 4-1
  dma_in_expected_occupancy = 3; 
}

void handle_single_dma_in(void)
{
  register volatile unsigned int check_dma asm("x3");
  unsigned int occupancy;
  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
  unsigned int i;
  // check_dma = occupancy;
  for(i=occupancy;i<dma_in_expected_occupancy;i++) {
    trig_in(); // this can be earler for better performance
    ring_send_pending++;
    check_dma = 0xd0000000 | ring_send_pending;
  }
}

void handle_pending_ring(void){
  register volatile unsigned int check_dma asm("x3");
  unsigned int occupancy;
  while(ring_send_pending) {

    check_dma = 5;
    CSR_READ(RINGBUS_SCHEDULE_OCCUPANCY, occupancy);
    if( occupancy < RINGBUS_SCHEDULE_DEPTH ) {
      check_dma = 6;
      dma_in_last_consumed = (dma_in_last_consumed+1) % DMA_IN_COUNT;
      to_ring(dma_in_last_consumed);
      ring_send_pending--;
    }
  }
}


void ring_send(unsigned char ttl, unsigned int data)
{
  CSR_WRITE(RINGBUS_WRITE_ADDR, ttl);
  CSR_WRITE(RINGBUS_WRITE_DATA, data);
  CSR_WRITE(RINGBUS_WRITE_EN, 0);
}



void handle_single_ring_in(void) {
  CSR_WRITE(RINGBUS_INTERRUPT_CLEAR, 0);
  int a;
  CSR_READ(RINGBUS_READ_DATA, a);
  vector_memory[DMA_OUT_START(DMA_OUT_NEXT_INDEX())] = a;
  trig_out();
}




void pre_delay() {
  int d1 = 10000; // 20 works, 10 fails
    for(int i = 0; i < d1; i++) {

    }
}

int main(void)
{
  register volatile unsigned int check_dma asm("x3");
  // turns out cs20 has a reset that goes longer than others
  // as a result of this, because we are the upstream from cs20, we need to burn cycles
  // before sending our first message or else the ringbus goes unstable

  // pre_delay();

  setup_dma_in();

  // blocking_run_dma_in();

  // blocking_run_ring_in();


  int h1;
  while(1) {
    CSR_READ(mip, h1);

    if(h1 == 0) {
      handle_pending_ring();
    }

    if(h1 & RINGBUS_ENABLE_BIT) {
      check_dma = 2;
      handle_single_ring_in();
    }

    handle_single_dma_in();

    // clear this bit right away when we see it
    // because we do not block on output dma because they are so short
    if(h1 & DMA_1_ENABLE_BIT) {
      CSR_WRITE(DMA_1_INTERRUPT_CLEAR, 0);
    }

    // clear because we are reading occupancy now
    if(h1 & DMA_0_ENABLE_BIT) {
      CSR_WRITE(DMA_0_INTERRUPT_CLEAR, 0);
    }

    // check_dma = 0x0f0f0f0f;
  }

}
