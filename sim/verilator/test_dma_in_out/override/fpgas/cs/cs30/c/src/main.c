#include "dma.h"
#include "mover.h"
#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "ringbus.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS30
#include "ringbus2_post.h"

VMEM_SECTION volatile unsigned int vmem_input [1024*32] = {};

#define MY_ASSERT(x) if(!(x)) {return __LINE__;}
#define MY_ASSERT_Y(x,y) if(!(x)) {return y;}

// int report_test_results(unsigned int data)
// {
//   // put the value of pass_fail_0 into the
//   // vector memory at a high address
//   unsigned int occupancy;

//   while(1)
//   {
//    CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
//    if(occupancy == 0)
//    {
//     break;
//    }
//   }


//   vector_memory[VECTOR_REPORT_ADDRESS] = data;
//   // vector_report[0] = data;

//   CSR_WRITE(DMA_1_START_ADDR, VECTOR_REPORT_ADDRESS);
//   CSR_WRITE(DMA_1_LENGTH, 1);
//   CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);  // start right away
//   CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0); // any values
// }

void trig_dma_in(unsigned int dma_ptr) {

  CSR_WRITE(DMA_0_START_ADDR, dma_ptr);
  CSR_WRITE(DMA_0_LENGTH, 1024);
  CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff);
  CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value

}

int report_output() {
  unsigned int check;
  for(unsigned int kblock = 0; kblock < 32; kblock++) {
    for(unsigned int i = 0; i < 1024;     i++ ) {
      check = (kblock*1024) + i; // index we are checking

      ring_block_send_eth(vmem_input[check]);
    }
  }
}

// will return 0 for pass
int check_input() {
  MY_ASSERT(vmem_input[0] == 0);
  STALL(4);
  MY_ASSERT(vmem_input[1] == 1);
  unsigned int check;
  unsigned int expected;
  unsigned int got;

  for(unsigned int kblock = 0; kblock < 32; kblock+=5) {
    for(unsigned int i = 0; i < 1024;      ) {
      check = (kblock*1024) + i; // index we are checking
      expected = check;
      got = vmem_input[check];

      CSR_WRITE(GPIO_WRITE, check );
      CSR_WRITE(GPIO_WRITE, 0x100000 | expected );
      CSR_WRITE(GPIO_WRITE, 0x300000 | got );

      // change this for a different pattern
      MY_ASSERT_Y(expected == got, check);
      STALL(4);

      // bump i by some random amount, loop does not have i++
      // equation cannot result to zero
      i += 7 + 256*i + kblock;
    }
  }
  return 0;
}


int main(void)
{
  unsigned int vmem_dma_ptr = VMEM_DMA_ADDRESS(vmem_input);
  unsigned int incoming_dma_occupancy;
  CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);

  ring_block_send_eth(0x02000000 | OUR_RING_ENUM);

  while(1) {
    // loop for 32 * 1024 blocks
    for(unsigned int i = 0; i < 32; i++) {
      trig_dma_in(vmem_dma_ptr+(i*1024));
      CSR_WRITE(GPIO_WRITE, 0x10000 | i );

      // slow down the loop as we go
      while(1) {
        CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, incoming_dma_occupancy);
        if(incoming_dma_occupancy < 4) {
          break;
        } else {

        }
      }
    } // done scheduling inputs

  CSR_WRITE(GPIO_WRITE, 0x20000 );
  // slow down, one last time after this we are 100% done
  while(1) {
    CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, incoming_dma_occupancy);
    if(incoming_dma_occupancy == 0) { // different condition that above
    break;
    } else {

    }
  }

  CSR_WRITE(GPIO_WRITE, 0x20001 );

  unsigned int fail; // non zero for fail, zero for pass
  fail = check_input();

  CSR_WRITE(GPIO_WRITE, 0x20002 );


  if( fail ) {
    ring_block_send_eth(STREAM_CMD | fail);
  } else {
    ring_block_send_eth(STREAM_CMD | fail); // pass (we could replace this with zero)
  }

  CSR_WRITE(GPIO_WRITE, 0x30000 | fail);

  } // while



}
