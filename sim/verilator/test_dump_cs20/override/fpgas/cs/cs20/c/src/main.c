#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "dma.h"
#include "fill.h"
#include "mover.h"
#include "mapper.h"
#include "ringbus.h"
#include "coarse_sync.h"


#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "feedback_bus_parse.h"
#include "vector_multiply.h"
#include "vmem_copy.h"

#include "nco_data.h"

#include "config_word_cmul_eq_0f.h"
#include "config_word_conj_eq_0f.h"
#include "config_word_add_eq_00.h"
// #include "performance.h"


#include "schedule.h"
#include "check_bootload.h"
#include "random.h"


void readout(unsigned int* data) {
    for(int i = 0; i < 16; i++) {
        // unsigned int* ptr = data;
        unsigned int word = data[i];
        ring_block_send_eth(CS20_USERDATA_ERROR | ((word & 0xffff0000)>>8) | 0x16 );
        ring_block_send_eth(CS20_USERDATA_ERROR | ((word & 0xffff)<<8)     | 0x17 );
    }
}



VMEM_SECTION unsigned int junk[16];
VMEM_SECTION unsigned int data[16];

int main(void)
{
    Ringbus ringbus;

    // SET_REG(x3, 0x00000000 | OUR_RING_ENUM);

    CSR_WRITE_ZERO(DMA_1_FLUSH_SCHEDULE);
    CSR_WRITE_ZERO(DMA_2_FLUSH_SCHEDULE);
    // flush_input_dma(VMEM_DMA_ADDRESS(junk), 16, 8192);



    uint32_t counter2 = 0;
    uint32_t occupancy;
    uint32_t have_data = 0;

    while(1) {

        SET_REG(x3, 0x0);
        CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
        SET_REG(x3, 0x1);

        if( occupancy == 0 ) {
            SET_REG(x3, 0x2);
            dma_in_set(VMEM_DMA_ADDRESS(data), 16);
            SET_REG(x3, 0x3);

            do {
                CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
                SET_REG(x3, 0x4);
                check_ring(&ringbus);
            } while (occupancy != 0);
            SET_REG(x3, 0x5);


            // readout(data);

            for(int i = 0; i < 50000000; i++) {
                STALL(2);
            }

            SET_REG(x3, 0x6);
        }


        check_ring(&ringbus);



    }

}
