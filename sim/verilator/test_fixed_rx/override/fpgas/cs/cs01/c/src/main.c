#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "vmalloc.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS01
#include "ringbus2_post.h"

#include "unit_test_ring.h"
#include "dma.h"

VMalloc mgr;

#define FFT_CP_SAMPLES (256) // works (1/4)
#define DMA_IN_CHUNK (1024) + FFT_CP_SAMPLES

unsigned int dma_in_ptr[2];

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

VMEM_SECTION unsigned int dma_buffer_a[1024+256];
VMEM_SECTION unsigned int dma_buffer_b[1024+256];

void setup_dma_in(void) {
  dma_in_ptr[0] = dma_buffer_a;
  dma_in_ptr[1] = dma_buffer_b;


  trig_dma_in(0, 0xffffffff);
  trig_dma_in(1, 0xffffffff);
}


int main(void)
{
	 init_VMalloc(&mgr);
	// unsigned int burn = 16;
	// for(unsigned int i = 0)

  	// CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);

	setup_dma_in();
  	// setup_dma_out();

  	dma_in_set(0, 4096*16);  // take whatever is left
}
