#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "vmalloc.h"
#include "circular_buffer.h"
#include "fill.h"
#include "ringbus.h"
#include "coarse_sync.h"
#include "atan.h"

#include "flush_config_word_data.h"
#include "fft_1024_3914.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS01
#include "ringbus2_post.h"



#define NORMAL_OPERATION

#ifdef NORMAL_OPERATION




void xbb_atan()
{
	unsigned int data = 0x80008000;
    unsigned int angle = fxpt_atan2(((0x80008000>>16)&0xffff), (0x80008000 & 0xffff));
   
   	ring_block_send_eth(data);
   	ring_block_send_eth(angle);
 }
   	
   
   



int main(void) {

  int counter;
  Ringbus ringbus;

  while(1) {
    xbb_atan();

    

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


#endif