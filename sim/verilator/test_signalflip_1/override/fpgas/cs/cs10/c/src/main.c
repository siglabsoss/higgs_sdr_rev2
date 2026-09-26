#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "vmalloc.h"
#include "circular_buffer_pow2.h"
#include "fill.h"
#include "ringbus.h"
#include "sig_utils.h"
#include "nco_data.h"

#include "flush_config_word_data.h"
#include "fft_1024_3914.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "config_word_cmul_eq_0f.h"
#include "config_word_conj_eq_0f.h"

VMEM_SECTION unsigned int data[1024];

int main(void)
{
    Ringbus ringbus;

    SET_REG(x3, 0x00000000 | OUR_RING_ENUM);

    //CSR_WRITE_ZERO(DMA_1_FLUSH_SCHEDULE);
    //CSR_WRITE_ZERO(DMA_2_FLUSH_SCHEDULE);
    //flush_input_dma(VMEM_DMA_ADDRESS(junk), 16, 8192);


    for(unsigned int i = 0; i < 500; i++) {
        // vector_memory[i] = 0xF0000000 | i;
        STALL(1);
    }

    SET_REG(x3, 0xdead0000 | OUR_RING_ENUM);

    //while(1) {`
    
    dma_block_get(VMEM_DMA_ADDRESS(data), 1024);
    unsigned int occupancy=1;
    while(occupancy != 0) {
	CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
    }
    
	//	dma_block_(VMEM_DMA_ADDRESS(data)+512, 512, 1);
	//}
    SET_REG(x3, 0xABCD1234);
    SET_REG(x3, data[0]);
    SET_REG(x3, data[1023]);
    unsigned int status=0;
    CSR_READ(DMA_0_STATUS, status);
    SET_REG(x3, status);
    if(status & 0x1 == 1) {
	dma_run_till_last();
	occupancy =1;
	while(occupancy != 0) {
	    CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
	}
	SET_REG(x3, 0x12345678);
	CSR_READ(DMA_0_STATUS, status);
	SET_REG(x3, status);
    }
	//	ring_block_send_eth(data[0]);
	//ring_block_send_eth(data[1023]);
}
