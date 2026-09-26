#include "csr_control.h"
#include "vmem.h"

void dma_out(unsigned int data_len) {
    CSR_WRITE(DMA_1_START_ADDR, 0);
    CSR_WRITE(DMA_1_LENGTH, data_len);
    CSR_WRITE(DMA_1_TIMER_VAL, START_IMMED_TIME);
    CSR_WRITE_ZERO(DMA_1_PUSH_SCHEDULE);
    INTERRUPT_WAIT_CLEAR_DMA_1();
}

int main(void)
{   
    SET_REG(x3, 0x31000000);

    for (unsigned int i = 0; i < 1024; i++) {
        vector_memory[i] = i;
    }

    dma_out(1024);

    return 0;
}