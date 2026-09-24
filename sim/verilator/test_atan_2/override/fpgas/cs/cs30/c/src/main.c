#include "dma.h"
#include "ringbus.h"
#include "csr_control.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "random.h"
#include "xvcordic.h" // ATAN()


int main(void)
{
    for( int i = 0; i < 200; i++) {
        STALL(200);
    }

    for(int i = 0; i < 180; i++) {
        unsigned int input = simple_random();
        
        unsigned int output;
        ATAN(output,input,15);
        
        ring_block_send_eth(input);
        ring_block_send_eth(output);

    }
}
