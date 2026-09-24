#define STREAM_CHUNK (128)

// #define DO_FORWARD_BACKPRESSURE
// #define MOD_STALL_UPPER (340)
// #define DO_FORWARD_LUCK_CHANCE 2
// #define DO_FORWARD_LUCK_STALL 1000

#include "dma.h"
#include "do_forward_stream.h"
#include "nco_data.h"

int main(void) {

    simple_random_seed(0x9b4fabe3);


    no_exit_stream();
}
