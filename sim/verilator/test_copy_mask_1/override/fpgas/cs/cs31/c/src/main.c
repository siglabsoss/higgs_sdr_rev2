#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "vmalloc.h"
#include "circular_buffer_pow2.h"
#include "fill.h"
#include "ringbus.h"
#include "coarse_sync.h"
#include "atan.h"
#include "nco_data.h"
#include "performance.h"
#include "xvcordic.h"
#include "corrupt_dma.h"
#include "subtract_timers.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "check_bootload.h"
#include "vmem_copy.h"


#include "soft_ringbus.h"
// ALLOCATE_SOFT_RINGBUS(64);
circular_buf_pow2_t __soft_ring_queue = CIRBUF_POW2_STATIC_CONSTRUCTOR(__soft_ring_queue, 32);
circular_buf_pow2_t* soft_ring_queue = &__soft_ring_queue;


#define MEMORY_MANAGER_SIZE 1024
#define MEMORY_MANAGER_CHUNKS (8)
#include "memory_manager.h"





// #define FSM_INIT (0)
#define FSM_FLUSHING (1)
#define FSM_DO_SYNC (4)
#define FSM_PING_PONG (3)

// #define COARSE_SYNC_OFDM_NUM (25600)
// #define COARSE_SYNC_OFDM_NUM (50)

unsigned global_coarse_sync_num = 50;

// #define FORCE_POWER_FIRST_FRAME

// #define POWER_WAIT_FOR_RING

#define ENABLE_DBGPOWER





void turnstile_advance_callback(unsigned int data);
void stream_callback(unsigned int data);
void trigger_update_nco();
void mark_both_fft_empty();

// Global vars, updated by ringbus
static unsigned int enable_output_stream;
static unsigned int pending_input_advance;
static unsigned int sync_finished = 0;
static unsigned int then = 0;


//////////////////////////////////
//
//  #define macro options

// define, and the FPGA will start streaming at boot.
// without this, a ringbus command must come to start output
#define STREAM_DEFAULT_TO_ON

// previous way of doing a sync call before my code starts
// #define HACK_SYNC

// Check status/control registers for underflow / overflow conditions
// #define CHECK_ERROR_COUNTERS

// #define DEBUG_DUMP_FIRST_FRAME

#define NORMAL_OPERATION

#ifdef NORMAL_OPERATION

#define DMA_IN_CHUNK_CS 1024

#include "flush_config_word_data.h"
#include "fft_1024_3914_modular.h"


// #include "config_word_cmul_rx4_0f.h"
// #include "config_word_cmul_rx4_00.h"
#include "config_word_cmul_eq_0f.h"
#include "config_word_conj_eq_0f.h"
#include "config_word_conj_eq_0b.h"
#include "config_word_conj_rx4_0f.h"
#include "config_word_add_eq_00.h"
#include "config_word_add_rx_01.h"
#include "config_word_add_rx4_01.h"
#include "config_word_add_rx4_02.h"
#include "config_word_sub_eq_00.h"
#include "config_word_magsquare_eq_00.h"
#include "config_word_magsquare_rx4_00.h"
#include "config_word_magsquare_rx4_01.h"
#include "config_word_magsquare_rx4_15.h"

#ifdef DEBUG_DUMP_FIRST_FRAME
unsigned int dump_first = 1;
#endif



VMEM_SECTION unsigned int mema[16] = {
0x00500050,
0x00500050,
0x00500050,
0x00500050,
0x00500050,
0x00500050,
0x00500050,
0x00500050,
0x00500050,
0x00500050,
0x00500050,
0x00500050,
0x00500050,
0x00500050,
0x00500050,
0x00500050
};
VMEM_SECTION unsigned int memb[16] = {
0x00007fff,
0x00007fff,
0x00007fff,
0x00007fff,
0x00007fff,
0x00007fff,
0x00007fff,
0x00007fff,
0x00007fff,
0x00007fff,
0x00007fff,
0x00007fff,
0x00007fff,
0x00007fff,
0x00007fff,
0x00007fff
};
VMEM_SECTION unsigned int memc[1024] = {0};








#ifdef ENABLE_DBGPOWER

unsigned current_shift = 18;

void setup_debug() {
    // for(int i = 0; i < 1000; i++) {
    //     STALL(10);
    // }
    // int a = _mgr.chunk_dma_address[0];
    // int b = _mgr.chunk_dma_address[1];
    // ring_block_send_eth(DEBUG_0_PCCMD | a);
    // ring_block_send_eth(DEBUG_0_PCCMD | b);
    // int mem = memory_manger_get();
#ifdef POWER_WAIT_FOR_RING
    power_est_state = PWR_DISABLE;
#endif
    
}


uint64_t signal_power_sum = 0;



extern uint32_t pwr_output_0[512];
extern uint32_t pwr_output_1[128];
extern uint32_t pwr_output_2[32];
extern uint32_t pwr_output_3[16];
extern unsigned short variable_cfg_stage_0[32];

uint32_t slow_report_state = 0;
uint32_t slow_report_next = 200;

void __attribute__((optimize("Os"))) slow_report() {

    if( slow_report_state == 0xffffffff) {
        return;
    }

    // int sent = 0;
    int base = 200;
    int jump = 0x1c00;


    unsigned int now;

    CSR_READ(TIMER_VALUE, now);

    if( now > slow_report_next ) {

        uint32_t ptr = 0;
        switch(slow_report_state) {
            case 0:
                ptr = VMEM_DMA_ADDRESS( mema );
                break;
            case 1:
                ptr = VMEM_DMA_ADDRESS( memb );
                break;
            case 2:
                ptr = VMEM_DMA_ADDRESS( memc );
                break;
            default:
                slow_report_state = 0xffffffff;
                return;
                break;
        }


        soft_queue_ring_eth(DEBUG_2_PCCMD | (ptr&0xffffff) );


        slow_report_state++;

        slow_report_next = base+ (slow_report_state*jump);
    }

}

void trigger_power_callback(unsigned int data) {
    // if( data == 0 ) {
    //     if( power_est_state == PWR_DISABLE ) {
    //         power_est_state = PWR_BOOT;
    //     }
    // }
}

#endif



// interleave copy repeat
void vmem_copy_rows_m(const unsigned int input_rows, const unsigned int input_rowsb, const unsigned int output_rows, const unsigned int rows, const unsigned int predicate){
    MVXV_KNOP(V2, input_rows);
    MVXV_KNOP(V5, input_rowsb);

    if( input_rowsb > input_rows ) {
        MVXV_KNOP(V6, input_rowsb-input_rows);

        ADD_KNOP(V2, V2, V6, predicate);
    } else {
        MVXV_KNOP(V6, input_rows-input_rowsb);

        SUB_KNOP(V2, V2, V6, predicate);
    }


    MVXV_KNOP(V3, output_rows);
    MVXV_KNOP(V4, 0x1);

    for (unsigned int i = 0; i < rows; i++){
        VNOP_LK13(V2);
        VNOP_SK13(V3);
        // ADD_KNOP(V2, V2, V4, 0x0);
        ADD_KNOP(V3, V3, V4, 0x0);
    }
}





void debug_mask() {


    // arg 0 is rotation
    // arg 1 is do not rotate
    // arg2 is output
    vmem_copy_rows_m(VMEM_ROW_ADDRESS(mema), VMEM_ROW_ADDRESS(memb), VMEM_ROW_ADDRESS(memc), 64, 0xaaaa);
    // vmem_copy_rows_m(VMEM_ROW_ADDRESS(memb), VMEM_ROW_ADDRESS(mema), VMEM_ROW_ADDRESS(memc)+32, 32, 0x5555);

}

int main(void) {

#ifdef STREAM_DEFAULT_TO_ON
    enable_output_stream = 1;
#else
    enable_output_stream = 0;
#endif
    pending_input_advance = 0;

    int occupancy;

    // CSR_READ(TIMER_VALUE, last_error_report);


    // init_VMalloc(&mgr);
    // init_memory_manager();
    // unsigned int burn = 16;
    // for(unsigned int i = 0) 

    // setup_calculate_power();

    CIRBUF_POW2_RUNTIME_INITIALIZE(__soft_ring_queue);
    setup_soft_ring(2, 1);

#ifdef ENABLE_DBGPOWER
    setup_debug();
#endif

    CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);


    Ringbus ringbus;
    unsigned int helper;

    int counter = 0;

    int power_counter = 0;

  while(1) {
    check_ring(&ringbus);

#ifdef ENABLE_DBGPOWER
    if( power_counter >= 4 ) {
        // debug_power();
        power_counter = 0;
    } else {
        power_counter++;
    }
    slow_report();
#endif


    counter++;

    if( counter == 10 ) {
        SET_REG(x3, 0xffff);
        SET_REG(x3, 0x0);
        debug_mask();
    }

    pet_soft_ring();


  }
  
}

#else


#endif


