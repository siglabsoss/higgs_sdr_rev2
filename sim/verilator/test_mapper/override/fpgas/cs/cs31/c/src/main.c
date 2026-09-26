#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS31
#include "ringbus2_post.h"



//void trig_out()
//{
//  unsigned int dma_out_next = DMA_OUT_NEXT_INDEX();
//
//  CSR_WRITE(DMA_1_START_ADDR, DMA_OUT_START(dma_out_next));
//  CSR_WRITE(DMA_1_LENGTH, DMA_OUT_LEN);
//  CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);  // start right away
//  CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0); // any value
//
//  dma_out_last = dma_out_next;
//}

#define OUTPUT_BUF_START (0)
#define DMA_LEN (1500)

int main(void)
{
	// CSR_WRITE(RINGBUS_WRITE_ADDR, RING_ADDR_CS30);

	// for(unsigned int i = 0; i < 1000; i++ )
	// {

	// }
	// CSR_WRITE(RINGBUS_WRITE_ADDR, 0);
 //    CSR_WRITE(RINGBUS_WRITE_DATA, 0x04cafebb);
 //    CSR_WRITE(RINGBUS_WRITE_EN, 0);
	// for(int i = OUTPUT_BUF_START; i < OUTPUT_BUF_START+DMA_LEN; i++) {
	// 	vector_memory[i] = i;
	// }

	vector_memory[0] = 0x2;

	CSR_WRITE(DMA_1_START_ADDR, OUTPUT_BUF_START);
	CSR_WRITE(DMA_1_LENGTH, DMA_LEN);
	CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);  // start right away
	CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0); // any value



}
