#include "xbaseband.h"
#include "vmem.h"
#include "csr_control.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_ETH
#include "ringbus2_post.h"

VMEM_SECTION unsigned int vmem_load0[16] = {0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF};

int main(void)
{
  register volatile unsigned int x3 asm("x3");
  register volatile unsigned int x4 asm("x3");

  x3 = 0xf0f0f0f0;
  CSR_WRITE(DMA_1_START_ADDR, VMEM_DMA_ADDRESS(vmem_load0));
  CSR_WRITE(DMA_1_LENGTH, 16);
  CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);  // start right away
  CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0); // any value
  x3 = 0xaaaaaaaa;
}
