#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "dma.h"
#include "fill.h"
#include "mover.h"
#include "mapper.h"
#include "ringbus.h"
#include "vmem.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS20
#include "ringbus2_post.h"

#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "bootloader.h"

#define nco_length 32768

void enable_machine_interrupts(void)
{
    // Enable RiscV Interrupts (was in crt.S)
    CSR_WRITE(mie, RINGBUS_ENABLE_BIT|DMA_0_ENABLE_BIT|DMA_1_ENABLE_BIT);
    CSR_WRITE(mip, RINGBUS_ENABLE_BIT|DMA_0_ENABLE_BIT|DMA_1_ENABLE_BIT);
    CSR_WRITE(mstatus, 0x008);
}

void nco_example(void)
{
    enable_machine_interrupts();

    CSR_WRITE(NCO_START_ANGLE, 0xbfffffff);  //pi = 0x7fffffff
    CSR_WRITE(NCO_LENGTH, nco_length);
    CSR_WRITE(NCO_DELTA, 1<<20); // delta=freq*131.072
    CSR_WRITE(NCO_PUSH_SCHEDULE, 0); // any value

    CSR_WRITE(DMA_2_START_ADDR, 0);
    CSR_WRITE(DMA_2_LENGTH, nco_length);
    CSR_WRITE(DMA_2_TIMER_VAL, 0xffffffff);  // start right away
    CSR_WRITE(DMA_2_PUSH_SCHEDULE, 0); // any value

    // Block until done
    unsigned int occupancy;
    while(1) {
        CSR_READ(DMA_2_SCHEDULE_OCCUPANCY, occupancy);
        if(occupancy == 0) {//DMA_2_SCHEDULE_DEPTH
            break;
        }
    }

}

int main(void)
{
    nco_example();


    CSR_WRITE(DMA_1_START_ADDR, 0);
    CSR_WRITE(DMA_1_LENGTH, nco_length);
    CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);
    CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0);
}
