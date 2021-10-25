#include "xbaseband.h"
#include "csr_control.h"
#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"



int main(void)
{
    Ringbus ringbus;
    int counter = 0;
    CSR_WRITE(RINGBUS_WRITE_ADDR, RING_ADDR_CS20);
    while(1) {
        check_ring(&ringbus);

        if(counter == 5) {
            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc001);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc002);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc003);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);
        // }
        // if(counter == 20) {
            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc004);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc005);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc006);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc007);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc008);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc009);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc00a);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc00b);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc00c);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc00d);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc00e);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc00f);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc010);
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            // CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc011);
            // CSR_WRITE_ZERO(RINGBUS_WRITE_EN);
        }

        // if(counter == 130) {
        //     CSR_WRITE(RINGBUS_WRITE_DATA, EDGE_EDGE_IN | 0xc012);
        //     CSR_WRITE_ZERO(RINGBUS_WRITE_EN);
        // }

        counter++;
    }
}
