#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "vmalloc.h"
#include "circular_buffer.h"
#include "fill.h"
#include "ringbus.h"
#include "coarse_sync.h"
#include "atan.h"
#include "xvcordic.h"


#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "nco_data.h"
#include "corrupt_dma.h"
#include "check_bootload.h"
#include "subtract_timers.h"
#include "dma.h"


#define NORMAL_OPERATION

#ifdef NORMAL_OPERATION



// #include "cooked_1.h"     // randomzied 39
#include "cooked_2.h"     // perfect (39) data, randomzied (38, 42) clocks
#define COOKED_FRAMES (63)

///////////////////////////////////////
//// for fine sync
VMEM_SECTION unsigned int trunk[16]={0};




#define FFT_CP_SAMPLES (0) // works (1/4)

#define DMA_IN_CHUNK (1024) + FFT_CP_SAMPLES

#define DMA_OUT_CHUNK_ATTACHMENT 16

#define DMA_OUT_CHUNK (1024+DMA_OUT_CHUNK_ATTACHMENT)

// must be power of two, must change next as well
#define DMA_IN_COUNT (4)
#define DMA_IN_COUNT_MASK 0x3



unsigned int working = 0;

void trig_dma() {

    unsigned int occupancy;

    CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
    if((occupancy+1) < DMA_1_SCHEDULE_DEPTH) {
          // break; // should break on first go
    } else {
          // stuck, can report this with ringbus
          // ring_block_send_eth(0xd0000000);
        return;
    }

    unsigned int dma_addr = VMEM_DMA_ADDRESS(cooked_data) + (working*1024);


    dma_block_send(dma_addr, 1024);
    dma_block_send_finalized(VMEM_DMA_ADDRESS(trunk), 16, 1);

    working++;
    if( working >= COOKED_FRAMES ) {
        working -= COOKED_FRAMES;
    }

}

int main(void) {

  Ringbus ringbus;
  int counter = 0;
  int ring_flag = 0;

  while(1) {
    trig_dma();
    

    if(counter == 100) {  // please use 100 as the ringbus interval
      check_ring(&ringbus);
      // unsigned int mem_free = vmalloc_available(&mgr);
      // ring_block_send_eth(0xc0000000 | mem_free);
      ring_flag = 1;
      counter = 0;
    }
    else
    {
        ring_flag = 0;
    }

    counter++;
    // ring_block_send_eth(counter);
  }
  

} 












#else

#endif  