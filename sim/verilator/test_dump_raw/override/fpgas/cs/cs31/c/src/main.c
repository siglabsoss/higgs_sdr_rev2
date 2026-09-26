#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "fill.h"
#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "check_bootload.h"


VMEM_SECTION unsigned int dump_raw[63*1024] = {0};


int main(void) 
{
	Ringbus ringbus;    
	int i, occupancy;
   while(1) {
       check_ring(&ringbus);
       dma_in_set(VMEM_DMA_ADDRESS(dump_raw), 1024*63);
       while(1) {
           check_ring(&ringbus);
           CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
           if( occupancy == 0)  {
               break;
           }
       }
       for(i = 0; i < 63; i++) {
           check_ring(&ringbus);
           dma_block_send(VMEM_DMA_ADDRESS(dump_raw) + (i*1024), 1024);
           for(i = 0; i< 10000; i++) {
               STALL(1);
           }
       }
       while(1) {
           check_ring(&ringbus);
           CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
           if( occupancy == 0) {
                 break;
           }
         }
   }
}