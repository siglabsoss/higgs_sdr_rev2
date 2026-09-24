// #include "fill.h"
#include "xbaseband.h"
// #include "apb_bus.h"
#include "csr_control.h"
#include "vmem.h"
// #include "pass_fail.h"

#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"

// #include "unit_test_ring.h"

#include "auto_barrelshift.h"
 
// volatile unsigned int d;
// volatile unsigned int e; 

#include "config_word_conj_eq_0f.h"


VMEM_SECTION unsigned short variable_eq_applied[32];

VMEM_SECTION unsigned int input1[16] = {
 0x00000401-2
,0x00000401-2
,0x00000401-2
,0x00000401
,0x00000401
,0x00000401
,0x00000401
,0x00000401
,0x00000401
,0x00000401
,0x00000401
,0x00000401
,0x00000401
,0x00000401
,0x00000401
,0x00000401
};



int main2(void)
{



    auto_bs_t auto_initial_eq = (auto_bs_t){
        {0, 1, 2}
        ,config_word_conj_eq_0f
        ,variable_eq_applied
        ,1024
        ,0
        ,0
    };

    int shift;
    shift = run_auto_bs_core(&auto_initial_eq, input1);
    ring_block_send_eth(shift);

    auto_initial_eq.idx[0] = 3;
    auto_initial_eq.idx[1] = 4;
    auto_initial_eq.idx[2] = 5;

    shift = run_auto_bs_core(&auto_initial_eq, input1);
    ring_block_send_eth(shift);

    auto_initial_eq.idx[0] = 0;
    auto_initial_eq.idx[1] = 4;
    auto_initial_eq.idx[2] = 2;

    shift = run_auto_bs_core(&auto_initial_eq, input1);
    ring_block_send_eth(shift);

    return 0;
}

int main(void) {

    auto_bs_t auto_initial_eq = (auto_bs_t){
        {0, 1, 2}
        ,config_word_conj_eq_0f
        ,variable_eq_applied
        ,4
        ,5
        ,0
    };

    ring_block_send_eth(config_word_conj_eq_0f[10]);
    run_auto_bs(&auto_initial_eq, input1);
    ring_block_send_eth(variable_eq_applied[10]);

    auto_initial_eq.idx[0] = 3;
    auto_initial_eq.idx[1] = 4;
    auto_initial_eq.idx[2] = 5;

    for(unsigned i = 0; i < 10; i++ ) {
        run_auto_bs(&auto_initial_eq, input1);
        ring_block_send_eth(variable_eq_applied[10]);
    }


    // copy_set_barrel(config_word_conj_eq_0f, variable_eq_applied, 7);
    // copy_set_barrel(config_word_conj_eq_0f, variable_eq_applied, 8);
    // ring_block_send_eth(variable_eq_applied[10]);

    int shift;
    // shift = run_auto_bs_core(&auto_initial_eq, input1);
    // ring_block_send_eth(variable_eq_applied[0]);
    // ring_block_send_eth(shift);


    return 0;
}

