#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "vmalloc.h"
#include "circular_buffer.h"
#include "fill.h"
#include "ringbus.h"
#include "coarse_sync.h"
#include "atan.h"
#include "xvcordic.h"

#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "nco_data.h"
#include "check_bootload.h"
#include "subtract_timers.h"
#include "trunk_types.h"
#include "copy_config.h"
#include "vmem_copy.h"
#include "eq_random_rotation.h"
#include "get_timer.h"

#include <stdint.h>
#include <stdbool.h>


#include "dengine_test_vector.h"



VMEM_SECTION unsigned int trunk[TRUNK_LENGTH];


unsigned total_data_size = ARRAY_SIZE(dengine_test_vector);


unsigned test_data_run = 0;


void load_test_data(void) {

    unsigned data_dma = VMEM_DMA_ADDRESS(dengine_test_vector);
    unsigned trunk_dma = VMEM_DMA_ADDRESS(trunk);

    if( 1 ) {
        // index into data
        // send first data twice
        unsigned data_index = (test_data_run == 0) ? (0) : (test_data_run - 1);

        // number of data words already sent
        unsigned data_offset = data_index * 1024;

        if( data_offset > total_data_size ) {
            while(1) {}; // stop sending
        }

        // send your data
        dma_block_send(data_dma + data_offset, 1024);

        // load trunk
        trunk[TRUNK_FRAME_COUNTER] = test_data_run;
        // send trunk
        dma_block_send_finalized(trunk_dma, TRUNK_LENGTH, 1);
    }


    test_data_run++;
}



int main2(void);
int main(void)
{
    self_sync_block_boot();
    main2();
    return 0;
}
int main2(void) {

    Ringbus ringbus;

    while(1) {
        load_test_data();
        check_ring(&ringbus);
    }

    return 0;
}


