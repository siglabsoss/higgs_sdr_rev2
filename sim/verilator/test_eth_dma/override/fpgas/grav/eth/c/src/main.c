#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_ETH
#include "ringbus2_post.h"


int main(void)
{
  // pre_delay();
  register volatile unsigned int x3 asm("x3");
  register volatile unsigned int x4 asm("x3");

  x3 = 0xf0f0f0f0;
  vector_memory[0] = 1;
  vector_memory[1] = 2;
  vector_memory[2] = 3;
  vector_memory[3] = 4;
  CSR_WRITE(DMA_1_START_ADDR, 0);
  CSR_WRITE(DMA_1_LENGTH, 4);
  CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);  // start right away
  CSR_WRITE_ZERO(DMA_1_PUSH_SCHEDULE);
  x3 = 0xaaaaaaaa;
}
