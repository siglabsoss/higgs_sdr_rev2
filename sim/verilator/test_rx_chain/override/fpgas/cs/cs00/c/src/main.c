#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "ringbus.h"
#include "symbol.h"


unsigned int START_ADDR = 0;
unsigned int CMD_DELAY = 0x64;
unsigned int START_TIME = START_IMMED_TIME;

void load_data(){
    for(unsigned int i = 0; i < global_dma_data_len; i++){
        vector_memory[i] = i;
    }
}

void output_data(Ringbus *ringbus){
    ringbus->addr = 0;
    ringbus->data = DMA_IN_CMD|global_dma_data_len;
    send_cmd(ringbus);
    CSR_WRITE(DMA_1_START_ADDR, START_ADDR);
    CSR_WRITE(DMA_1_LENGTH, global_dma_data_len);
    CSR_WRITE(DMA_1_TIMER_VAL, START_TIME);
    CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0);
    INTERRUPT_WAIT_CLEAR_DMA_0();
}

void listen_cmd(Ringbus *ringbus){
    CSR_WRITE(GPIO_WRITE_EN, LED_GPIO_BIT|RESET_ADC_COUNTER|ADC_COUNTER_BIT);
    while(1) {
        CSR_SET_BITS(GPIO_WRITE, LED_GPIO_BIT);
            check_ring(ringbus);
        for(int j = 0; j < 100000; j++) {
            check_ring(ringbus);
        }
        CSR_CLEAR_BITS(GPIO_WRITE, LED_GPIO_BIT);
        for(int j = 0; j < 1000000; j++) {
            check_ring(ringbus);
        }
    }
}

int main(void)
{
    unsigned int ringbus_occupancy;
    Ringbus ringbus;
    global_dma_data_len = 0x10000;
    load_data();
    listen_cmd(&ringbus);
    
}