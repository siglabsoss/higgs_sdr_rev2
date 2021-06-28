#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "ringbus.h"


int main(void)
{
    Ringbus ringbus;
    // Ringbus ring_to_eth;
    check_ring(&ringbus);

    CSR_WRITE(GPIO_WRITE_EN, LED_GPIO_BIT);
    CSR_SET_BITS(GPIO_WRITE, LED_GPIO_BIT);

    while(1) {
        CSR_SET_BITS(GPIO_WRITE, LED_GPIO_BIT);
        check_ring(&ringbus);
        for(int j = 0; j < 100000; j++) {
            check_ring(&ringbus);
        }
        CSR_CLEAR_BITS(GPIO_WRITE, LED_GPIO_BIT);
        for(int j = 0; j < 10000000; j++) {
            check_ring(&ringbus);
        }
    }

}
