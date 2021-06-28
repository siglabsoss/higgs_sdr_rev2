#include "csr_control.h"
#include "ringbus.h"

void listen_cmd(Ringbus *ringbus){
    CSR_WRITE(GPIO_WRITE_EN, LED_GPIO_BIT);
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
    Ringbus ringbus;
    listen_cmd(&ringbus);
}