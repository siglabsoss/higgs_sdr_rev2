#include "csr_control.h"

void dma_in(unsigned int data_len) {
    CSR_WRITE(DMA_0_START_ADDR, 0);
    CSR_WRITE(DMA_0_LENGTH, data_len);
    CSR_WRITE(DMA_0_TIMER_VAL, START_IMMED_TIME);
    CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);
    INTERRUPT_WAIT_CLEAR_DMA_0();
}

void dma_out(unsigned int data_len) {
    CSR_WRITE(DMA_1_START_ADDR, 0);
    CSR_WRITE(DMA_1_LENGTH, data_len);
    CSR_WRITE(DMA_1_TIMER_VAL, START_IMMED_TIME);
    CSR_WRITE_ZERO(DMA_1_PUSH_SCHEDULE);
    INTERRUPT_WAIT_CLEAR_DMA_1();
}

int main(void)
{
    SET_REG(x3, 0x12000000);

    dma_in(1024);

    for (unsigned int i = 0; i < 100; i++) {
        STALL(40);
    }

    dma_out(1024);

    return 0;
}