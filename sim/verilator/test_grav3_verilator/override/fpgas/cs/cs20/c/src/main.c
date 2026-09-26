#include "csr_control.h"
#include "ringbus.h"

void send_ringbus(Ringbus *ringbus, unsigned int delay,
                  unsigned int addr, unsigned int data) {
    for (unsigned int i = 0; i < delay; i++) {
        STALL(40);
    }
    ringbus->addr = addr;
    ringbus->data = data;
    send_cmd(ringbus);
}

void dma_in(unsigned int data_len) {
    CSR_WRITE(DMA_0_START_ADDR, 0);
    CSR_WRITE(DMA_0_LENGTH, data_len);
    CSR_WRITE(DMA_0_TIMER_VAL, START_IMMED_TIME);
    CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);
    INTERRUPT_WAIT_CLEAR_DMA_0();
}

int main(void)
{
    Ringbus ringbus;
    SET_REG(x3, 0x20000000);

    dma_in(1024);

    for (unsigned int i = 0; i < 100; i++) {
        STALL(40);
    }

    send_ringbus(&ringbus, 900, 0, 0x20000000);
    send_ringbus(&ringbus, 100, 1, 0x20000010);
    send_ringbus(&ringbus, 100, 2, 0x20000011);
    send_ringbus(&ringbus, 100, 3, 0x20000001);
    send_ringbus(&ringbus, 100, 4, 0x20000002);
    send_ringbus(&ringbus, 100, 5, 0x20000012);
    send_ringbus(&ringbus, 100, 6, 0x20000022);
    send_ringbus(&ringbus, 100, 7, 0x20000032);
    send_ringbus(&ringbus, 100, 8, 0x20000031);
    send_ringbus(&ringbus, 100, 9, 0x20000020);

    return 0;
}