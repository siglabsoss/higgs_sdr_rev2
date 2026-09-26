
#include "xbaseband.h"
#include "vmem.h"
#include "csr_control.h"
#include "bootloader.h"
#include "pass_fail.h"
#include "ringbus.h"
#include "linker_symbols.h"


#include "sig_utils.h"

#include "stack_test.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"

volatile unsigned int external;
volatile unsigned int external2;
volatile unsigned int external3;


unsigned int bloat[1024];

// void fill_stack();


// #define CSR_READ(addr,valout)  asm volatile("csrr  %0, " _XSTR(addr) : "=r" (valout) : )


unsigned get_stack_remaining() {
    int sp0;
    GET_REG(x2, sp0);

    int sp_end = (unsigned int) &__sp_start - (unsigned int) &__stack_size;
    int sp_remaining = sp0 - sp_end;
    return sp_remaining;
}


int recurse(int depth, int *val) {
    int a[4];
    a[3] = depth;
    a[2] = depth-1;
    a[1] = (int)val;

    // int sp0;
    // GET_REG(x2, sp0);
    // SET_REG(x3, sp0);
    if( depth > 10 ) {
        ring_block_send_eth(get_stack_remaining());
    }

    // fill_stack();

    if( depth == 0 || external != 0) {
        return 0xfeed;
    } else {
        int res = recurse(depth-1, &a[3]);

        // if( a[3] != depth ) {
        //     return 1;
        // } else {
            return res;
        // }

    }
}

void fill_bloat() {
    for(int i = 0; i < ARRAY_SIZE(bloat); i++ ) {
        bloat[i] = 0xffefffff;
    }
}




void main()
{
    SET_REG(x3, 0xdeadbeef);
    Ringbus ringbus;

    // for(int i = 0; i < 800; i++) {
    //     STALL(40);
    // }

    fill_bloat();
    fill_stack();

    int timer;

    do {
        STALL(40);
        CSR_READ(TIMER_VALUE, timer);
    }while(timer < 0x9e38); // sending ringbus before this time will not come out correctly



    SET_REG(x3, 0xffffffff);
    int timer_done;
    CSR_READ(TIMER_VALUE, timer_done);


    int sp0;
    GET_REG(x2, sp0);



    int sp_end = (unsigned int) &__sp_start - (unsigned int) &__stack_size;
    int sp_remaining = sp0 - sp_end;

    external = 0;
    external2 = 50;
    int s1 = 4;
    int res = recurse(external2, &s1);

    int untouched = stack_untouched();




    ring_block_send_eth(0xffffffff);
    ring_block_send_eth((unsigned int) &__stack_size);
    ring_block_send_eth((unsigned int) &__sp_start);
    ring_block_send_eth(sp_remaining);
    // ring_block_send_eth(sp0);


    ring_block_send_eth(res);
    // ring_block_send_eth(timer_done);
    ring_block_send_eth(untouched);

    ring_block_send_eth(0xdeadbeee);

}




