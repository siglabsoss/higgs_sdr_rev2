#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "ringbus.h"
#include "symbol.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS00
#include "ringbus2_post.h"

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

void _saturation_ratio(Ringbus *ringbus, unsigned int gain){
    unsigned int delay[7] = {3, 6, 13, 26, 51, 102, 204};
    unsigned int samples_size = gain&0xF;
    unsigned int delay_value = delay[samples_size];
    unsigned int gain_value = gain&0xFFFFF0;
    unsigned int samples_saturated;
    
    ringbus->addr = 6;
    ringbus->data = DSA_GAIN_CMD|gain_value;
    send_cmd(ringbus);
    SET_HALF_REG_VAR(x4, 0xdeadbeef);
    for(unsigned int i = 0; i < 1000; i++){
        asm("nop");
    }
    SET_HALF_REG_VAR(x4, 0xdeadcafe);
    // Clear saturation counter
    CSR_READ(SATDETECT, samples_saturated);
    for(unsigned int i = 0; i < delay_value; i++){
        asm("nop");
    }
    CSR_READ(SATDETECT, samples_saturated);
    SET_HALF_REG_VAR(x4, samples_saturated);
    ringbus->addr = 5;
    ringbus->data = samples_saturated;
    send_cmd(ringbus);

}

void test_saturation_ratio(unsigned int data){
    Ringbus ringbus;
    _saturation_ratio(&ringbus, data);
}

int main(void)
{
    unsigned int ringbus_occupancy;
    unsigned int saturation_detect;
    Ringbus ringbus;
    
    ring_register_callback(&test_saturation_ratio, SATURATION_RATIO_CMD);
    listen_cmd(&ringbus);
}