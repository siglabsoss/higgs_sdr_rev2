#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "turnstile.h"
#include "dma.h"
// #include "symbol.h"
#include "ringbus.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS10
#include "ringbus2_post.h"

#include "unit_test_ring.h"

VMEM_SECTION unsigned int zeros[1024] = {};
VMEM_SECTION unsigned int junk[1024];

int main(void)
{
  Ringbus ringbus;
  register volatile unsigned int x3 asm("x3");
  register volatile unsigned int x4 asm("x4");

  // unsigned int i;
  // for(i = 0; i < 1024; i++)
  // {
  //   vector_memory[i] = dmem_sin[i];
  //   // vector_memory[i] = 0x7000;
  // }

  // x3 = 0;

  unsigned int start_addr;
  unsigned int data_len;
  unsigned int start_time;

  start_addr = VMEM_DMA_ADDRESS(zeros);
  data_len = 1024;
  start_time = 0xffffffff;

  CSR_WRITE(GPIO_WRITE_EN, LED_GPIO_BIT);
  CSR_SET_BITS(GPIO_WRITE, LED_GPIO_BIT);
  int led = 1;
  check_ring(&ringbus);

  unsigned int occupancy;
  unsigned int h1;

  int counter = 0;

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

      // prevent upstream from locking up
      dma_in_set(VMEM_DMA_ADDRESS(junk), data_len);
    }

    CSR_READ(mip, h1);
    if(h1 & DMA_1_ENABLE_BIT) {
      CSR_WRITE(DMA_1_INTERRUPT_CLEAR, 0);
      check_ring(&ringbus);
    }

    counter++;
    if(counter == 100000) {
      CSR_SET_BITS(GPIO_WRITE, LED_GPIO_BIT);
    }

    if(counter == 1000000) {
      counter = 0;
      CSR_CLEAR_BITS(GPIO_WRITE, LED_GPIO_BIT);
    }

  }
}