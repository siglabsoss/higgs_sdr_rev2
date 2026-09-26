#include "ringbus.h"
#include "coarse_sync.h"
#include "atan.h"
#include "nco_data.h"
#include "performance.h"
#include "xvcordic.h"



#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "debug_last_signal.h"



int main(void) {
    Ringbus ringbus;

    setup_debug_last(OUR_RING_ENUM);

    while(1) {
        pet_debug_last();
        check_ring(&ringbus);
    }
}