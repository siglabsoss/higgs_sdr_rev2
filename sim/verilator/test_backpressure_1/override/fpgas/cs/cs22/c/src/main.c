#include "vmem.h"
#include "csr_control.h"
#include "dma.h"
#include "vmem_counter_8k.h"
#include "tb_debug.h"

#define VMEM_SIZE (1024 * 8)
#define PACKET_SIZE (1024)

unsigned int out_buffer_index = 0;
/**
 *
 */
void enable_output_dma();

/**
 *
 */
void update_output_buffer();

int main(void) {
    STALL(200);

    enable_output_dma();

    return 0;
};

void enable_output_dma() {
    dma_block_send(VMEM_DMA_ADDRESS(vmem_counter_8k) + out_buffer_index,
                   VMEM_SIZE);
}
