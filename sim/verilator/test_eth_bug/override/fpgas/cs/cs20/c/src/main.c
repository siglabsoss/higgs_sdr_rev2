#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "unit_test_ring.h"

#include <stdint.h>
#include <stdbool.h>



uint32_t got[64];
uint32_t got_idx = 0;


// uint32_t expected[6] = {0xf0000000, 0xf0000001, 0xf0000002, 0xf0000003, 0xf0000004, 0xf0000005};

uint32_t expected_start = 0xf0000000;

int main(void)
{
    SET_REG(x3, 0xdeadbeef);
    unsigned int mip_read;
    unsigned int data;

    int expected_rbs = -1;


    while(1) {
        CSR_READ(mip, mip_read);
        if(mip_read & RINGBUS_ENABLE_BIT){
            CSR_WRITE(RINGBUS_INTERRUPT_CLEAR, 0);
            CSR_READ(RINGBUS_READ_DATA, data);
            SET_REG(x3, data);
            if( expected_rbs == -1 ){
                expected_rbs = data;
            } else {
                got[got_idx] = data;
                got_idx++;
            }
        } else {
            SET_REG(x3, 0);
        }

        if(expected_rbs != -1 && got_idx == expected_rbs) {
            break;
        }
    } // while


    // loop and check
    bool fail = false;
    for(uint32_t i = 0; i < expected_rbs; i++) {
        SET_REG(x4, i);
        if(got[i] != (expected_start + i))  {
            fail = true;
        }
    }


    if( fail ) {
        SET_REG(x4, 1);
        ring_block_send_eth(0xa0000001);
    } else {
        SET_REG(x4, 0);
        ring_block_send_eth(0xa0000000);
    }

}
