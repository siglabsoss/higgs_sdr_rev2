#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
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
#include "schedule.h"
#include <string.h>

schedule_t _schedule;
schedule_t* schedule = &_schedule;

int main(void)
{
    Ringbus ringbus;
    default_schedule(schedule);
    schedule->offset = 0xf000;
    schedule->offset = 512 - 30 + (512*29);

    schedule->epoc_time = 1536193523;

    ring_block_send_eth(0xdead0000 | OUR_RING_ENUM);

    uint32_t start, end, progress, accumulated_progress, timeslot, can_tx, epoc;

    uint32_t lifetime_32;

    uint32_t cb = 0xf000;
    // cb = 0;

    lifetime_32 = cb;

    uint32_t run_time = 1000;

    for(uint32_t i = 0; i < run_time; i++) {

        schedule_get_timeslot2(schedule,
                            lifetime_32,
                            &progress,
                            &accumulated_progress,
                            &timeslot,
                            &epoc,
                            &can_tx);

        ring_block_send_eth(lifetime_32);
        ring_block_send_eth(progress);
        ring_block_send_eth(accumulated_progress);
        ring_block_send_eth(timeslot);
        ring_block_send_eth(epoc);
        ring_block_send_eth(0xf0000000 | can_tx);

        lifetime_32++;
    }

}

// int main(void)
// {
//     Ringbus ringbus;
//     default_schedule(schedule);
//     schedule->offset = 0xf000;
//     schedule->offset = 0x1;
//     // schedule->offset = 0;

//     ring_block_send_eth(0xdead0000 | OUR_RING_ENUM);

//     uint32_t start, end, progress, timeslot, can_tx;

//     uint32_t lifetime_32;

//     uint32_t cb = 0xf000;
//     cb = 0;

//     for(lifetime_32 = 0+cb; lifetime_32 < (1000+cb); lifetime_32++) {

//         schedule_get_timeslot(schedule,
//                             lifetime_32,
//                             &start,
//                             &end,
//                             &progress,
//                             &timeslot,
//                             &can_tx);

//         ring_block_send_eth(lifetime_32);
//         ring_block_send_eth(start);
//         ring_block_send_eth(end);
//         ring_block_send_eth(progress);
//         ring_block_send_eth(timeslot);
//         ring_block_send_eth(0xf0000000 | can_tx);
//     }

// }