#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "ringbus.h"

void clear_idma(unsigned int data_len, unsigned int start_time){
    CSR_WRITE(DMA_0_START_ADDR, 0);
    CSR_WRITE(DMA_0_LENGTH, data_len);
    CSR_WRITE(DMA_0_TIMER_VAL, start_time);
    CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);

    for(unsigned int i = 0; i < 15; i++){
            asm("nop");
    }
    CSR_WRITE(DMA_0_FLUSH_SCHEDULE, 0);
}

int main(void)
{
    Ringbus ringbus;
    check_ring(&ringbus);

    clear_idma(10, START_IMMED_TIME);
    CSR_WRITE(GPIO_WRITE_EN, LED_GPIO_BIT);
    CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);

    while(1) {
        CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);
            check_ring(&ringbus);
        for(int j = 0; j < 100000; j++) {
            check_ring(&ringbus);
        }
        CSR_WRITE(GPIO_WRITE, 0);
        for(int j = 0; j < 1000000; j++) {
            check_ring(&ringbus);
        }
    }
}