#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "circular_buffer_pow2.h"
#include "fill.h"
#include "ringbus.h"
#include "coarse_sync.h"
#include "atan.h"
#include "nco_data.h"
// #include "performance.h"
#include "xvcordic.h"
#include "corrupt_dma.h"
#include "subtract_timers.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "deprecate_all_ringbus.h"
#include "check_bootload.h"
#include "vmem_copy.h"
#include "trunk_types.h"
#include "get_timer.h"
#include "self_sync.h"
#include "copy_config.h"
#include "handle_generic_op.h"
#include "duplex_schedule.h"

#include <stdint.h>
#include <stdbool.h>
#include "tb_inject_mem.h"

bool is_verilator = false;



#include "soft_ringbus.h"
// ALLOCATE_SOFT_RINGBUS(64);
circular_buf_pow2_t __soft_ring_queue = CIRBUF_POW2_STATIC_CONSTRUCTOR(__soft_ring_queue, 32);
circular_buf_pow2_t* soft_ring_queue = &__soft_ring_queue;





// #define FSM_INIT (0)
#define FSM_FLUSHING (1)
#define FSM_DO_SYNC (4)
#define FSM_PING_PONG (3)

// #define COARSE_SYNC_OFDM_NUM (25600)
// #define COARSE_SYNC_OFDM_NUM (50)

unsigned global_coarse_sync_num = 50;
uint32_t lifetime_32 = 0;
static uint32_t duplex_progress;
static uint32_t duplex_mode;
static uint32_t duplex_mode_rx;

duplex_timeslot_t duplex;
duplex_timeslot_t duplex_rx;
static unsigned cooked_data_type = 0;
// #define FORCE_POWER_FIRST_FRAME

#define POWER_WAIT_FOR_RING

#define ENABLE_DBGPOWER





void turnstile_advance_callback(unsigned int data);
void mark_both_fft_empty(void);

// Global vars, updated by ringbus
static unsigned int enable_output_stream;
static unsigned int pending_input_advance;
static unsigned int sync_finished = 0;
static unsigned int sync_finished_hold = 0;
static unsigned int self_adjust_after_sync = 0;
static unsigned int coarse_sync_result = 0;
// static unsigned int then = 0;


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

#define DMA_IN_CHUNK_CS 1024

#include "flush_config_word_data.h"
#include "fft_1024_3914_modular.h"

#include "config_word_cmul_rx4_0f.h"
#include "config_word_cmul_rx4_00.h"
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

#define PWR_BOOT 0
#define PWR_IDLE 1
#define PWR_CAPTURE 2
#define PWR_ANALYZE 3
#define PWR_JUDGE 4
#define PWR_DISABLE 5

#define FFT_SIZE (1024)

#define FFT_CP_SAMPLES (256) // works (1/4)

#define DMA_IN_CHUNK ((1024) + FFT_CP_SAMPLES)
#define DMA_OUT_CHUNK (1024)
// must be power of two, must change next as well
#define DMA_IN_COUNT (4)
#define DMA_IN_COUNT_MASK 0x3


#define BOTH_HOLDOVER_MULTIPLIER (2)
#define BOTH_HOLDOVER_SIZE (1280*BOTH_HOLDOVER_MULTIPLIER)

static unsigned int power_est_state = 0;


static unsigned int needs_holdover = 0;

static unsigned int coarse_sync_zero_output_counter = 0;

// static VMEM_SECTION unsigned int zero_buffer[1280] = {0};

// This is a junk area, use it for whatever
static VMEM_SECTION unsigned int holdover_data[BOTH_HOLDOVER_SIZE] = {0};

static VMEM_SECTION unsigned int input_data[1280*4]={0};

static VMEM_SECTION unsigned int output_conj_multi[1024]  = {0};
static VMEM_SECTION unsigned int output_onetone_calculation[1024] = {0};
static VMEM_SECTION unsigned int output_sum_complex[16] = {0};
// static VMEM_SECTION unsigned int output_coarse_sync[16*16] = {0};

static VMEM_SECTION unsigned int garbage_vmem[16];
static VMEM_SECTION unsigned int zeros[16];
unsigned int garbage_row;

#include "coarse_sync_vectors.h"


// the fft only needs 1024 samples, however we want to directly write the trunk
// at the end of this buffer after the fft is done, and then ship out via DMA
// so we add the size here. previously this was only 1024 and caused a memory
// corruption bug
static VMEM_SECTION unsigned int fft_mem_a[DMA_IN_CHUNK+TRUNK_LENGTH];
static VMEM_SECTION unsigned int fft_mem_b[DMA_IN_CHUNK+TRUNK_LENGTH];


// When RX radio sends us a P frame, only 1/10 frames are valud
// We use a composite frame here
static VMEM_SECTION unsigned int duplex_composite_ul_pilot[FFT_SIZE];

unsigned int duplex_composite_ul_pilot_row;
unsigned int duplex_composite_ul_pilot_dma;
uint32_t enable_composite_ul_pilot = 0;
uint32_t enable_freeze_wide_ul_pilot = 0;



// when triggered, we copy memory from one of the ping-pong buffers to this buffer
// so that power can have it to slowly look at it
static VMEM_SECTION unsigned int power_data_copy[1024] = {0};


static VMEM_SECTION unsigned short variable_coarse_mul[32];
static VMEM_SECTION unsigned short variable_coarse_fft[32];
static VMEM_SECTION unsigned short variable_coarse_add[32];


// static VMEM_SECTION unsigned int cfo_nco_data[1280] = {0};

// we write to these using copy_config
static VMEM_SECTION unsigned short cfg_cfo_mul[32];
static VMEM_SECTION unsigned short cfg_cfo_mul_conj[32];

static unsigned int variable_coarse_mul_row;
static unsigned int variable_coarse_fft_row;
static unsigned int variable_coarse_add_row;

uint32_t times_nco_driver_failed = 0;
uint32_t invalid_sfo_cfo = 0;


#define MAX_TURNSTILE_ADVANCE (1024+256)

VMEM_SECTION unsigned int dma_buffer_a[1024+256+MAX_TURNSTILE_ADVANCE];
VMEM_SECTION unsigned int dma_buffer_b[1024+256+MAX_TURNSTILE_ADVANCE];


// //////////////////////////////////////////////////////////////

static unsigned int fsm_state;

static unsigned int last_error_report;


#define MY_ASSERT(x) if(!(x)) { ring_block_send_eth(0xe0000000|__LINE__);}


void fft_accept_new(unsigned int dma_ptr);
void dma_out_set_safe(unsigned int dma_ptr, unsigned int size);

//for cfo purpose
static unsigned int rbus_theta_shift = 0;
static unsigned int next_angle = 0;
static unsigned int rbus_omega_sign = 0;
static unsigned int rbus_omega = 0;
static unsigned int delta_low  = 0;

#define NCO_BUFFER_SIZE  (1024)
#include "nco_driver_sfo.h"


static unsigned int cfg_cmulti_location;
static unsigned int cfg_cmulco_location;


// how many ofdm symbols go by before we adjust
static unsigned int packet_num_SFO_adjustment_period = 0; //71111;

static unsigned int packet_counter_SFO_adjustment = 0; // how many ofdm symbols we have, seen since last adjustment
static unsigned int packet_SFO_adjustment_nco_freq = 0;
// determines the direction we adjust, and disables when zero
// 0 is no adjustment
// 1 delete
// 2 add
static unsigned int packet_SFO_adjustment_direction = 0;

// how many samples to + / - each period
// ALWAYS positive, as sign is encoded into  packet_SFO_adjustment_direction
static unsigned int packet_SFO_adjustment_step = 0; 

static unsigned int packet_STO_adjustment_step = 0;

static unsigned int packet_STO_adjustment_neg_step = 0;


static unsigned int ringbus_sfo_adjustment_temp = 0;

// Call this first
void sfo_adjustment_callback(const unsigned int data) {
    ringbus_sfo_adjustment_temp = data;
    // ring_block_send_eth(data);
}

// Call this second, settings are not applied till this is called
// 0 is disable sfo adjustment
// 1 is delete (1 sample)
// 2 is add (1 sample)
void __attribute__((optimize("Os"))) sfo_sign_callback(const unsigned int data) {
    
    

    if( data == 0 )  {
        packet_num_SFO_adjustment_period = ringbus_sfo_adjustment_temp;
        packet_SFO_adjustment_direction = data;
        packet_SFO_adjustment_step = 0;
        packet_counter_SFO_adjustment = 0;
    } else if (data == 1 || data == 2) {
        packet_num_SFO_adjustment_period = ringbus_sfo_adjustment_temp;
        packet_SFO_adjustment_direction = data;
        packet_SFO_adjustment_step = 1;

        packet_SFO_adjustment_nco_freq=(unsigned int)(4194304.0f/(packet_num_SFO_adjustment_period*1.0f));
        //packet_counter_SFO_adjustment = 0;
    } else if (data == 3) {
        packet_STO_adjustment_step = ringbus_sfo_adjustment_temp;
    } else if (data == 4) {
        packet_STO_adjustment_neg_step = ringbus_sfo_adjustment_temp;
    }

    
    // ring_block_send_eth(0x87000000|ringbus_sfo_adjustment_temp);
    // ring_block_send_eth(0x86000000|data);

}

// 0 a
// 1 b
static int dma_state = 0;
static int dma_in_valid = -1;
static unsigned int dma_in_ptr[2];
static int fft_a_empty;
static int fft_b_empty;

static int fft_ready = -1;
static int fft_valid = -1;
static unsigned int fft_ptr[2];


static int dma_out_valid = -1;
static int dma_out_ready = -1;

static unsigned int input_expected_occupany = 0;


void post_sync_prime_ping_pong(void) {
    dma_out_ready = -1;
    dma_out_valid = -1;
    dma_in_valid = -1;
    fft_ready = -1;
    fft_valid = -1;
    dma_out_valid = -1;
    dma_out_ready = -1;
    mark_both_fft_empty();
}

#define DMA_OUT_CIRBUF_SIZE (4)
circular_buf_pow2_t dma_out_started = CIRBUF_POW2_STATIC_CONSTRUCTOR(dma_out_started, DMA_OUT_CIRBUF_SIZE);
// circular_buf_t dma_out_started;
// unsigned int dma_out_started_storage[DMA_OUT_CIRBUF_SIZE+1];

// trigger the next DMA in
// if advance is non zero, we will do a longer DMA, but it will never get read
// this means buffer a/b must be 1024 + cp + largest advance
// if largest advance is simply 1024+cp, the buffers need to be 2*(1024+cp)
void trig_dma_in(unsigned int idx, unsigned int timer_start, unsigned int advance) {
    // dma_in_set(VMEM_DMA_ADDRESS(dma_in_ptr[idx]), DMA_IN_CHUNK);


    // CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx]));
    // CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance);
    // CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
    // CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value


    // static unsigned int timer_start = 4096;

    // if direction is disabled, OR it is enabled and we are not at the period
    // if( packet_SFO_adjustment_direction == 0 || 
    //     (packet_counter_SFO_adjustment != packet_num_SFO_adjustment_period &&
    //         packet_SFO_adjustment_direction == 1)
    //     ) {
    // if( i am 0, always true.  i am 1 or 2, true when not counter) {}
    if( packet_SFO_adjustment_direction == 0) {
        CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx])+packet_STO_adjustment_neg_step);
        CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance-packet_STO_adjustment_neg_step+packet_STO_adjustment_step);
        CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
        CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value
        
        
        // if((packet_STO_adjustment_neg_step!=0)||(packet_STO_adjustment_step!=0))
        // {
        //     ring_block_send_eth(0x85000000|packet_STO_adjustment_neg_step);
        //     ring_block_send_eth(0x84000000|packet_STO_adjustment_step);
        // }
        

       
        packet_STO_adjustment_neg_step = 0;
        packet_STO_adjustment_step = 0;
        // SET_REG(x3, 0xf0000000);

    } else {


        if(packet_counter_SFO_adjustment < (packet_num_SFO_adjustment_period-1)) {
            CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx])+packet_STO_adjustment_neg_step);
            CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance-packet_STO_adjustment_neg_step+packet_STO_adjustment_step);
            CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
            CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value
            packet_STO_adjustment_neg_step = 0;
            packet_STO_adjustment_step = 0;
            packet_counter_SFO_adjustment++;
        } else {
            // mode 1 delete 
            // SET_REG(x3, 0xd0000000);
            // SET_REG(x4, packet_SFO_adjustment_direction);
            if( packet_SFO_adjustment_direction == 1 ) {
                // SET_REG(x3, 0xc0000000);
                CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx]));
                CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance+packet_SFO_adjustment_step);
                CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
                CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value
            } else if (packet_SFO_adjustment_direction == 2) {
                // SET_REG(x3, 0xb0000000);
                // mode 2 add
                CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx]));
                CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance-packet_SFO_adjustment_step);
                CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
                CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);

                // in add, we just copy
                if(packet_SFO_adjustment_step!=0)
                {
                    // SET_REG(x3, 0xa0000000);
                    // FIXME: only valid when packet_SFO_adjustment_step = 1.
                    vector_memory[VMEM_DMA_ADDRESS(dma_in_ptr[idx])+1279] = vector_memory[VMEM_DMA_ADDRESS(dma_in_ptr[idx])+1278];
                }
                
            }
            // if we hit the counter
            packet_counter_SFO_adjustment = 0;
            // ring_block_send_eth(0xff000000);
        
        }
    }
}


// FIXME: this function is big for some reason
// test in hardware and add
// __attribute__((optimize("Os")))
void  setup_dma_in(void) {
  dma_in_ptr[0] = (unsigned int) dma_buffer_a;
  dma_in_ptr[1] = (unsigned int) dma_buffer_b;


  trig_dma_in(0, 0xffffffff, 0);
  trig_dma_in(1, 0xffffffff, 0);

  // trigger_update_nco(0);
  // trigger_update_nco(1);

  input_expected_occupany = 2;
}

void mark_both_fft_empty(void) {
    fft_a_empty = 1;
    fft_b_empty = 1;
}

fft1024_cfg_t active_plan;

void setup_fft(void) {
  fft_ptr[0] = (unsigned int)fft_mem_a;
  fft_ptr[1] = (unsigned int)fft_mem_b;
  mark_both_fft_empty();

  active_plan = get_fft1024_plan(0, 0);
}


// an array in imem same length as number of stages in fft1024_cfg_t
unsigned pending_bs[5] = {0,0,0,0,0};

void __attribute__((optimize("Os"))) fft_barrel_shift_save(unsigned int data) {
    unsigned int stage = ((data & 0x00FF0000) >> 16);
    unsigned int shift = ((data & 0xffff));

    if( stage > 4 ) {
        ring_block_send_eth(APP_ASSERT_PCCMD | 0xf00001);
        return;
    }

    pending_bs[stage] = shift;
}

void __attribute__((optimize("Os"))) fft_barrel_shift_write_out(void) {
    int error;
    int error_save = 0;

    for(int i = 0; i < 5; i++) {
        // return 1 on failure
        error = fft_1024_set_bs(&active_plan, i, pending_bs[i]);

        // save first error only
        // but keep going
        // this hopefully puts us in a less bad state vs stopping
        if( error_save == 0) {
            error_save = error;
        }
    }



    // prove to test bench we got the right values
    // ring_block_send_eth(stage);
    // ring_block_send_eth(shift);

    if( error_save ) {
        ring_block_send_eth(APP_ASSERT_PCCMD | 0xf00000);
    }
}

// remember ringbus.c shaves off the upper 8 bits for us
//  1) send in 5 ringbus with a state and shift
//  2) always send the 4th stage last, it will latch the values in
void __attribute__((optimize("Os"))) fft_barrel_shift_callback(unsigned int data) {
    unsigned int stage = ((data & 0x00FF0000) >> 16);
    
    fft_barrel_shift_save(data);

    if(stage == 4) {
        fft_barrel_shift_write_out();
    }
}


void fft_barrel_shift_defaults(const unsigned int data) {
    (void)data;

    const unsigned default_shift[FFT_STAGES] = {
         0x0f
        ,0x0f
        ,0x0f
        ,0x0f
        ,0x0f
        };

    for(unsigned i = 0; i < 5; i++ ) {
        // does the same as and,
        // allows us to skip fft_barrel_shift_save()
        pending_bs[i] = default_shift[i];
    }
    fft_barrel_shift_write_out();
}



bool periodic_self_sync_send(void);


unsigned pending_self_sync_adjust_state = 0;
unsigned pending_self_sync_adjust_amount = 0;
unsigned pending_self_sync_adjust_counter = 0;



/// This function does what I was manually doing in stoBoth() in modem_main.js to adjust our own
/// tx and rx chains
void apply_self_sync_after_coarse(void) {
    const unsigned coarse_tweak = 192;
    const unsigned amount = coarse_tweak + 1280 - coarse_sync_result;

    SET_REG(x3, 0x20000000);
    SET_REG(x3, 0x00000000);
    SET_REG(x3, amount);
    SET_REG(x3, 0x00000000);


    // cs31CoarseSync()
    sfo_adjustment_callback(amount);   // SFO_PERIODIC_ADJ_CMD
    sfo_sign_callback(3);  // SFO_PERIODIC_SIGN_CMD

    pending_self_sync_adjust_state = 1;
    pending_self_sync_adjust_amount = amount;
}

// setPartnerSfoAdvance(1, y, 3);
void periodic_self_sync_adjust_tx_chain(void) {
    if( pending_self_sync_adjust_state == 0 ) {
        return;
    }
    SET_REG(x3, 0x30000000);
    SET_REG(x3, pending_self_sync_adjust_state);
    SET_REG(x3, 0x00000000);


    const unsigned fdelay = 4; // how many frames to wait before adjusting tx chain

    if( pending_self_sync_adjust_state == 1 ) {
        pending_self_sync_adjust_counter = lifetime_32 + fdelay;
        pending_self_sync_adjust_state = 2;
        return;
    }

    if( pending_self_sync_adjust_state == 2 ) {
        if( lifetime_32 >= pending_self_sync_adjust_counter ) {
            SET_REG(x3, 0x40000000);
            SET_REG(x3, 0x00000000);
            bool done = periodic_self_sync_send();

            if( !done ) {
                pending_self_sync_adjust_counter = lifetime_32 + fdelay;
            } else {
                pending_self_sync_adjust_counter = 0;
                pending_self_sync_adjust_amount = 0;
                pending_self_sync_adjust_state = 0;
            }
        }
    }
}

// returns true if done, false if more updates need to run
bool periodic_self_sync_send(void) {
    const unsigned cap = 128;
    // pending_self_sync_adjust_amount


    if( pending_self_sync_adjust_amount ) {
        const unsigned amount_chunk = MIN(cap, pending_self_sync_adjust_amount);

        SET_REG(x3, 0x50000000);
        SET_REG(x3, 0x00000000);
        SET_REG(x3, amount_chunk);
        SET_REG(x3, 0x00000000);

        Ringbus ring;
        ring.addr = RING_ADDR_TX_TIME_DOMAIN;

        ring.data = SFO_PERIODIC_ADJ_CMD | amount_chunk;
        send_cmd(&ring);

        ring.data = SFO_PERIODIC_SIGN_CMD | 3;
        send_cmd(&ring);

        pending_self_sync_adjust_amount -= amount_chunk;
    }

    // do not combine this as else to the above if
    if( pending_self_sync_adjust_amount == 0 ) {
        return true; // true for done
    }

    return false; // false for more work needed
}



void setup_dma_out(void) {
    CIRBUF_POW2_RUNTIME_INITIALIZE(dma_out_started);
}



void setup_fsm(void) {
    fsm_state = FSM_PING_PONG;
}


unsigned pending_sync_symbol_timing = 0;
uint32_t occupancy_before_coarse = 0;

void pet_fsm(void) {

    unsigned int next_state = fsm_state;
    unsigned int dma_out_occupancy;
    unsigned int occupancy;

    switch(fsm_state) {
        case FSM_FLUSHING:
            // stay in flushing until these flags are cleared
            dma_out_occupancy = circular_buf2_occupancy(&dma_out_started);
            if( fft_a_empty == 1 && fft_b_empty == 1 && dma_out_occupancy == 0 ) {
                sync_finished = 0;  // set this flag 0, after the fn runs it will be set to 1
                next_state = FSM_DO_SYNC;
            }
            break;
        case FSM_PING_PONG:
            if(pending_sync_symbol_timing) {
                CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);

                // only click over into at the best time
                // 2 here is for ping pong
                if( (occupancy == 2 && input_expected_occupany == 2) || (occupancy == 1 && input_expected_occupany == 1)  ) {
                    pending_sync_symbol_timing = 0; // reset flag
                    next_state = FSM_FLUSHING;
                    needs_holdover = 1; // set flag so input dma will holdover buffer before sync
                    coarse_sync_zero_output_counter = 0; // set to zero, 
                    occupancy_before_coarse = occupancy;
                }
            }
            break;
        case FSM_DO_SYNC:
            if(sync_finished) {
                next_state = FSM_PING_PONG;
                sync_finished = 0;

                sync_finished_hold = 1;

                post_sync_prime_ping_pong();

                setup_dma_in(); // re prime the pump, other setups don't need to be called so far

                if( self_adjust_after_sync ) {
                    SET_REG(x3, 0x10000000);
                    SET_REG(x3, 0x00000000);
                    apply_self_sync_after_coarse();
                    self_adjust_after_sync = 0;


                }
            }
        break;
    }

    fsm_state = next_state;
}

const unsigned power_idle_calls = 2; // value may be 2 larger than recorded here
unsigned times_power_idle = 0;

void debug_save_memory(void) {
    int next_state = power_est_state;

#ifdef FORCE_POWER_FIRST_FRAME
    if( power_est_state == 0) {
        power_est_state = PWR_CAPTURE;
    }
#endif

    if( power_est_state == PWR_BOOT ) {
        times_power_idle = 0;
        next_state = PWR_IDLE;
    }
    if( power_est_state == PWR_IDLE ) {
        if( times_power_idle == power_idle_calls ) {
            next_state = PWR_CAPTURE;
        } else {
            times_power_idle++;
        }
    }

    if( power_est_state == PWR_CAPTURE ) {
        
        int consume_idx = dma_state;
        unsigned int* cpu_ptr_from_dma = (unsigned int*) dma_in_ptr[consume_idx];
        unsigned int input_row = VMEM_ROW_ADDRESS(cpu_ptr_from_dma);

        // copy a second time

        unsigned int power_row = VMEM_ROW_ADDRESS(power_data_copy);
        vmem_copy_rows(input_row, power_row, 1024/16);

        // SET_REG(x3, 0x10000000);
        // SET_REG(x3, power_data_copy[0]);
        // SET_REG(x3, power_data_copy[1]);


        next_state = PWR_ANALYZE;
    }

    power_est_state = next_state;
}

/////////////
//
//  Pet incoming dma
//  
//  This fn watches for input dma to trigger.
//  Each trigger is assumed to be the other DMA buffer
//  because of this, no circular buffer is used.
void pet_dma_in(void) {
  unsigned int occupancy;
  // int error;
  // unsigned int data;
  // unsigned int helper;
  // unsigned int set_pace = 0;
  // static unsigned int pace;

  // CSR_READ(mip, helper);
  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);



  if(occupancy < input_expected_occupany) {
    // CSR_WRITE(DMA_0_INTERRUPT_CLEAR, 0);
    // SET_REG(x3, (1<<8) | dma_state);

    // during flushing or other states, we eat this signal
    // we do not tell FFT there is anything
    // we DO update dma_state so the rest keeps going
    if( fsm_state == FSM_PING_PONG ) {
        dma_in_valid = dma_state; // signal a buffer index downstream
    } // What happens if we go out of the state? we never update valud?

    input_expected_occupany--;

#ifdef ENABLE_DBGPOWER
    debug_save_memory();
#endif


#ifdef DEBUG_DUMP_FIRST_FRAME
    if( dump_first == 0 ) {
        

        unsigned int* p = (unsigned int*) dma_in_ptr[dma_state];

        for(int i = 0; i < 800; i++) {
            STALL(40);
        }
        
        ring_block_send_eth(p[0]);
        ring_block_send_eth(p[1]);
        ring_block_send_eth(p[2]);
        ring_block_send_eth(p[3]);

    }
    dump_first--;
#endif

    dma_state = (dma_state+1)&0x1;
  }

  if(fft_ready != -1) {

    // during flushing, we:
    //  * ignore any value in pending_input_advance
    //  * do not trigger further input dma's
    //  * we DO clear fft_ready as normal
    if( fsm_state == FSM_PING_PONG ) {
        if( pending_input_advance == 0 ) {
            // normal trigger
            trig_dma_in(fft_ready, 0xffffffff, 0);
            input_expected_occupany++;
        } else {
            // ringbus has instructed us to advance the turnstile
            // simply run this once, and then set back to 0
            trig_dma_in(fft_ready, 0xffffffff, pending_input_advance);
            input_expected_occupany++;
            // SET_REG(x4, 0xa0000000 | pending_input_advance);
            ring_block_send_eth(DEBUG_13_PCCMD | (pending_input_advance&0xffffff) );
            pending_input_advance = 0;
        }

        // fire off one for every input
        //trigger_update_nco(fft_ready);
    } else if( fsm_state == FSM_FLUSHING ) {

        // one time trigger a flushing buffer between ping-pong and sync
        if( needs_holdover ) {
            needs_holdover = 0;
            dma_in_set(VMEM_DMA_ADDRESS(holdover_data), BOTH_HOLDOVER_SIZE);
        }

    }


    // SET_REG(x3, (2<<8) | fft_ready);
    fft_ready = -1;

  }
}

// pass the index of the NCO buffer you wish to trigger into
// void trigger_update_nco(unsigned int idx) {
//     make_nco(VMEM_DMA_ADDRESS(nco_in_ptr[idx]), 1024, nco_angle, nco_delta);
//     nco_angle = (nco_angle + nco_angle_delta);
// }

// at some point the fft needed to respond to signals without going into the full pet_fft
// this was created to call more frequently to update flags
void pet_fft_respond_signals(void) {
  if(dma_out_ready != -1) {
    // Output dma is telling us that a buffer has finished going out

    // could check for error here if it's already empty
    // fixme what is a cleaner way to do this?
    if(dma_out_ready == 0) {
      fft_a_empty = 1;
    }
    if(dma_out_ready == 1) {
      fft_b_empty = 1;
    }

    // SET_REG(x3, (0x4a << 8) | dma_out_ready);
    // SET_REG(x3, (0x4b << 8) | (fft_a_empty << 1) | fft_b_empty);

    dma_out_ready = -1;

  }
}

void run_cfo(
    const unsigned int is_conj,
    const unsigned int input_data_fft,
    const unsigned int input_data_nco,
    const unsigned int output_data_location) {
    if (is_conj == 0x1) {
        MVXV_KNOP(V0, cfg_cmulco_location);
    } else {
        MVXV_KNOP(V0, cfg_cmulti_location);
    }
    
    VNOP_LK14(V0); //k14 configuration word

    MVXV_KNOP(V1, input_data_fft);
    MVXV_KNOP(V2, input_data_nco);
    MVXV_KNOP(V3, output_data_location);
    MVXV_KNOP(V4, 0x1);

    for (unsigned int i = 0; i < 4; i++){
        ADD_LK8(V1, V1, V4, 0x0);
        ADD_LK9(V2, V2, V4, 0x0);
    }


    for (unsigned int i = 0; i < 60; i++){
        ADD_LK8(V1, V1, V4, 0x0);
        ADD_LK9(V2, V2, V4, 0x0);
        ADD_SK1(V3, V3, V4, 0x0);
    }
    for (unsigned int i = 0; i < 4; i++){
        ADD_SK1(V3, V3, V4, 0x0);
    }
}

void handle_freeze_pilot(const unsigned int* const cpu_ptr_fft) {
    if( unlikely(cooked_data_type != 2) ) {
        return;
    }

    // only for tx
    if( duplex.role == DUPLEX_ROLE_RX ) {
        return;
    }

    const int32_t partial_pilots_frame = duplex_ul_pilot_phase(duplex_progress);

    if( likely(partial_pilots_frame == -1) ) {
        return;
    }

    const unsigned fft_row = VMEM_ROW_ADDRESS(cpu_ptr_fft);
    const unsigned fft_dma = VMEM_DMA_ADDRESS(cpu_ptr_fft);
    (void)fft_dma;

    const unsigned phase = (duplex_rx.lt_phase);// + (unsigned)partial_pilots_frame;

    if( phase == 0 ) {

        // copy everything to our saved frame
        vmem_copy_rows(
            fft_row,
            duplex_composite_ul_pilot_row,
            64
        );

        // leave fft_row alone, we will use it without modifying

    } else {

        // write "tone 2"  to our saved buffer
        duplex_composite_ul_pilot[2] = cpu_ptr_fft[2];

        // copy our saved buffer over fft
        vmem_copy_rows(
            duplex_composite_ul_pilot_row,
            fft_row,
            64
        );
    }
}

// holds a composite frame for the pilots
// copies it back over the cpu_fft pointer when done
void handle_composite_pilot(const unsigned int* const cpu_ptr_fft) {
    if( unlikely(cooked_data_type != 2) ) {
        return;
    }

    // only for tx
    if( duplex.role == DUPLEX_ROLE_RX ) {
        return;
    }

    if( unlikely(!enable_composite_ul_pilot) ) {
        return;
    }

    const int32_t partial_pilots_frame = duplex_ul_pilot_phase(duplex_progress);

    if( likely(partial_pilots_frame == -1) ) {
        return;
    }

    const unsigned fft_row = VMEM_ROW_ADDRESS(cpu_ptr_fft);
    const unsigned fft_dma = VMEM_DMA_ADDRESS(cpu_ptr_fft);

    const unsigned phase = (duplex_rx.lt_phase);// + (unsigned)partial_pilots_frame;


    // *2 here is to compensate for the table structure, not 5 vs 10 phase
    const unsigned fresh_dma = duplex_partial_pilot_rx[(phase*2)];
    const unsigned fresh_len = duplex_partial_pilot_rx[(phase*2)+1];

    // copy input from fft row pointer
    // write output to our saved buffer

    vmem_copy_words(
        fft_dma+fresh_dma,
        duplex_composite_ul_pilot_dma+fresh_dma,
        fresh_len,
        garbage_row
    );

    // write "tone 2"
    duplex_composite_ul_pilot[2] = cpu_ptr_fft[2];

    // copy everything back
    vmem_copy_rows(
        duplex_composite_ul_pilot_row,
        fft_row,
        64
    );
}

void pet_fft(void) {
  // example only starts to work once we have 2 items in the queue

  // int error;
  // static unsigned updates = 1;

  // deals with dma telling us we are done
  pet_fft_respond_signals();

  // unsigned int output_blocked = 0;


  // deals with dma telling us that we have new data to consume
  if( dma_in_valid != -1 )
  {
    ///////
    //
    // when set to 0, it means buffer A
    // buffer A goes into our buffer C
    //
    int consume_idx = dma_in_valid;

    if(dma_in_valid == 0) {
      if( fft_a_empty == 0) {
        // this means incoming data came too soon, we were still working
        // ring_block_send_eth(0x2a000000);
        return; // early
        // error
      }
      fft_a_empty = 0; // mark fft A full (set empty to false)
    }
    if(dma_in_valid == 1) {
      if( fft_b_empty == 0) {
        // error
        // ring_block_send_eth(0x2b000000);
        return; // early
      }
      fft_b_empty = 0;  // mark fft B full (set empty to false)
    }

    // SET_REG(x3, (3 << 8) | (fft_a_empty << 1) | fft_b_empty);

    unsigned int* cpu_ptr_from_dma = (unsigned int*) dma_in_ptr[consume_idx];  // input
    unsigned int* cpu_ptr_fft      = (unsigned int*) fft_ptr[consume_idx];     // output



    ///////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////// for cfo correction   this is a sequence method////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // if(sync_finished_hold == 1)
    // {
    //     const unsigned int current_angle = next_angle + rbus_theta_shift;

    //     rbus_theta_shift = 0;

    //     make_nco(VMEM_DMA_ADDRESS(cfo_nco_data), nco_length, current_angle, rbus_omega);

    //     next_angle = current_angle + nco_length*rbus_omega; 


    //     run_cfo(
    //     rbus_omega_sign,
    //     VMEM_ROW_ADDRESS(cpu_ptr_from_dma),
    //     VMEM_ROW_ADDRESS(cfo_nco_data),
    //     VMEM_ROW_ADDRESS(cpu_ptr_from_dma));
    // }
    /////////////////////////////////////////////////////////////////////////////////////////









    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // debug, read out first 4 values so we can track
    // SET_REG(x4, 0xfeedfeed);
    // SET_REG(x4, cpu_ptr_from_dma[0]);
    // SET_REG(x4, cpu_ptr_from_dma[1]);
    // SET_REG(x4, cpu_ptr_from_dma[2]);
    // SET_REG(x4, cpu_ptr_from_dma[3]);


    active_plan.data_location   = VMEM_ROW_ADDRESS(cpu_ptr_from_dma);  //input
    active_plan.data_location_0 = VMEM_ROW_ADDRESS(cpu_ptr_fft);       //output 


    // if(cfo_compensation_direction == 1)
    // {
    //     xbb_conj_multi(VMEM_ADDRESS(config_word_conj_eq_0f), VMEM_ADDRESS(cpu_ptr_from_dma), VMEM_ADDRESS(nco_in_ptr[consume_idx]), VMEM_ADDRESS(cpu_ptr_from_dma));
      
    // }
    // else if (cfo_compensation_direction == 0)
    // {
    //     xbb_conj_multi(VMEM_ADDRESS(config_word_cmul_eq_0f), VMEM_ADDRESS(cpu_ptr_from_dma), VMEM_ADDRESS(nco_in_ptr[consume_idx]), VMEM_ADDRESS(cpu_ptr_from_dma));
    // }

    // CSR_WRITE(GPIO_WRITE, 0x30);

	if( is_verilator ) {
		vmem_copy_rows(VMEM_ROW_ADDRESS(cpu_ptr_from_dma), VMEM_ROW_ADDRESS(cpu_ptr_fft), 64);
	} else {
    	fft_1024_run(&active_plan);
	}

    duplex_progress = lifetime_32 % DUPLEX_FRAMES;
    duplex_mode_rx = get_duplex_mode(&duplex_rx, duplex_progress, lifetime_32);
    duplex_mode = get_duplex_mode(&duplex, duplex_progress, lifetime_32);



    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    //////////////////////  phase correction //////////////////////////////////////
    if(sync_finished_hold == 1)
   // if(1)
    {

        const unsigned int current_angle = next_angle + rbus_theta_shift;

        rbus_theta_shift = 0;

        unsigned int counter_ahead = 0;

        if(packet_counter_SFO_adjustment == 0)
        {
            counter_ahead = packet_num_SFO_adjustment_period-1;
        }
        else
        {
            counter_ahead = packet_counter_SFO_adjustment - 1;
        }



        unsigned int flag = nco_schedule_next_sfo(counter_ahead, packet_SFO_adjustment_nco_freq, current_angle);
        if(flag == 1)
        {
            next_angle = current_angle + rbus_omega;
        }
        else if(flag == 0)
        {
            next_angle = current_angle;
        }

        unsigned cpu_pointer = 0;

        int nco_ready_indi = nco_ready_sfo(&cpu_pointer);


        if((nco_ready_indi == 1) || (nco_ready_indi == 0))
        {
            
            if(
                   ((rbus_omega_sign == 0) && (packet_SFO_adjustment_direction == 1))
                || ((rbus_omega_sign == 1) && (packet_SFO_adjustment_direction == 2))
                ) {
                run_cfo(rbus_omega_sign, VMEM_ROW_ADDRESS(cpu_ptr_fft), VMEM_ROW_ADDRESS(cpu_pointer), VMEM_ROW_ADDRESS(cpu_ptr_fft));
                nco_done_sfo(nco_ready_indi);
            } else {
                invalid_sfo_cfo++;
                nco_done_sfo(nco_ready_indi);
            }
            // else if((packet_SFO_adjustment_direction == 1) &&(rbus_omega_sign == 1))
            // {
            //     ring_block_send_eth(0xff000000|rbus_omega_sign);
            //     ring_block_send_eth(0xff000000|packet_SFO_adjustment_direction);
            //     nco_done_sfo(nco_ready_indi);
            // }
            // else if((packet_SFO_adjustment_direction == 2) &&(rbus_omega_sign == 0))
            // {
            //     ring_block_send_eth(0xff000000|rbus_omega_sign);
            //     ring_block_send_eth(0xff000000|packet_SFO_adjustment_direction);
            //     nco_done_sfo(nco_ready_indi);
            // }

        } else {
            times_nco_driver_failed++;
        }

    }

    vector_memory[VMEM_DMA_ADDRESS(cpu_ptr_fft) +
                  DMA_OUT_CHUNK +
                  TRUNK_FRAME_COUNTER] = lifetime_32;


    // if( enable_freeze_wide_ul_pilot ) {
    //     handle_freeze_pilot(cpu_ptr_fft);
    // } else {
    //     handle_composite_pilot(cpu_ptr_fft);
    // }


     /////////////////////////// handle copy 4rows //////////////////
    // if(cooked_data_type == 2){

    //     if(((duplex.role == DUPLEX_ROLE_RX) && ((lifetime_32%DUPLEX_FRAMES == 0) || (lifetime_32%DUPLEX_FRAMES == 1)))){
    //     // ((duplex.role == DUPLEX_ROLE_TX_0) && (lifetime_32%DUPLEX_FRAMES == DUPLEX_ULFB_START)) || 
    //     // ((duplex.role == DUPLEX_ROLE_TX_1) && (lifetime_32%DUPLEX_FRAMES == DUPLEX_ULFB_START+1))){
    //     //if(duplex_narrowband_move_cs31(duplex, lifetime_32)){
    //         const unsigned input_dma = VMEM_ROW_ADDRESS_TO_DMA((VMEM_ROW_ADDRESS(cpu_ptr_fft)+59))+1;
    //         const unsigned output_dma = VMEM_ROW_ADDRESS_TO_DMA(VMEM_ROW_ADDRESS(cpu_ptr_fft)+1);
    //         const unsigned words = 64;
    //         vmem_copy_words(input_dma, output_dma, words, garbage_row);
    //         ////reset zeros
    //         vmem_copy_rows(VMEM_ROW_ADDRESS(zeros), VMEM_ROW_ADDRESS(cpu_ptr_fft)+59, 1);
    //         vmem_copy_rows(VMEM_ROW_ADDRESS(zeros), VMEM_ROW_ADDRESS(cpu_ptr_fft)+60, 1);
    //         vmem_copy_rows(VMEM_ROW_ADDRESS(zeros), VMEM_ROW_ADDRESS(cpu_ptr_fft)+61, 1);
    //         vmem_copy_rows(VMEM_ROW_ADDRESS(zeros), VMEM_ROW_ADDRESS(cpu_ptr_fft)+62, 1);
    //         vmem_copy_rows(VMEM_ROW_ADDRESS(zeros), VMEM_ROW_ADDRESS(cpu_ptr_fft)+63, 1);
    //         //vector_memory[VMEM_DMA_ADDRESS(cpu_ptr_fft)+1022] = 0x2000; ///////// for tone-2 0x2000 may be changed later and tone-2 may be updated later in cs11
    //     }

    // }


    lifetime_32++;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    SET_REG(x3, (0x3a<<8) | consume_idx );

    // reset signal we consumed
    dma_in_valid = -1;
    // signal back, this releases our reliance on the input that dma gave us, meaning dma is free to erase it
    fft_ready = consume_idx; // Release A to be over written

    fft_valid = consume_idx; // send C on to be DMA'd out
  }

  

}



// similar to just calling dma_out_set
void dma_out_set_safe(unsigned int dma_ptr, unsigned int size) {

  unsigned int now;
  unsigned int occupancy;
  unsigned int rb_occupancy;
  // unsigned int occupancy_busy;
  // unsigned int occupancy_combined1, occupancy_combined2;
  while(1) {
    CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
    if(occupancy < DMA_1_SCHEDULE_DEPTH) {
      break; // should break on first go
    } else {
    CSR_READ(RINGBUS_SCHEDULE_OCCUPANCY, rb_occupancy);
        // this IF blocks us from blocking
        if(rb_occupancy < RINGBUS_SCHEDULE_DEPTH-2) {
        // stuck, can report this with ringbus
           
           CSR_READ(TIMER_VALUE, now);
             if( (now - last_error_report) > 125000000 ) {
                last_error_report = now;
               ring_block_send_eth(RX_ERROR_PCCMD);
            }
        }
    }
  }
  
  dma_send_finalized(dma_ptr, size, 1);
}

#define IS_SECOND_DMA (0x2)
#define DMA_A_B_MASK (0x1)

unsigned int dma_out_extra = 0;

void pet_dma_out(void) {
  // int helper;
  unsigned int occupancy;
  int error;
  unsigned int data;
  unsigned int output_blocked = 0;
  // unsigned int dma_occupancy_combined;
  unsigned int dma_occupancy;
  // unsigned int dma_occupancy_status;

  // we can tell by the size of our cirbuf if we have room 
  // for instance we get a 3 here after 0x500, 0x501, 0x500 meaning that ...
  // if fft is signaling to us that it just finished a buffer
  if(fft_valid != -1) {
    occupancy = circular_buf2_occupancy(&dma_out_started);
    // SET_REG(x3, (7<<8) | occupancy );
    if( occupancy > 2 ) {
      output_blocked = 1;
    } else {
      output_blocked = 0;
    }
  }

  // handles setting dma's
  if( (!output_blocked) && (fft_valid != -1) ) {
    ///////
    //
    // when set to 0, it means buffer C is being given to us by FFT
    // we fire off output dma and then wait
    //
    int consume_idx = fft_valid;

    // SET_REG(x3, (0x5<<8) | consume_idx);

    unsigned int* cpu_ptr_from_fft = (unsigned int*) fft_ptr[consume_idx];

    if( enable_output_stream == 1 ) {
        error = circular_buf2_put(&dma_out_started, consume_idx); MY_ASSERT(error == 0);



        // schedule the CP
        // dma_out_set_safe(VMEM_DMA_ADDRESS(cpu_ptr_from_fft)+(DMA_IN_CHUNK-FFT_CP_SAMPLES), FFT_CP_SAMPLES);

        // schedule the full FFT
        const unsigned dma_out_ptr = VMEM_DMA_ADDRESS(cpu_ptr_from_fft);
        const unsigned dma_out_sz = DMA_OUT_CHUNK + TRUNK_LENGTH;

        dma_out_set_safe(dma_out_ptr, dma_out_sz);



    } else {
        // output stream disabled

        // instantly free buffer for upstream.
        // unsure how to handle situation when changing from enabled to disabled and reverse
        
        dma_out_ready = consume_idx;
        pet_fft_respond_signals();
    }

    // _perf_work(); // triggers when we SEND and not complete dma but that's ok

    fft_valid = -1; // set this to -1 so we don't get caught
  }


  // handles when a dma is finished
  // we will get two interrupts for a given buffer because we scheduled twice (CP)
  // each time we pull a value from the cirbuf letting us know what just finished
  // if the value has the IS_SECOND_DMA set, then we know the buffer is not needed again
  // CSR_READ(mip, helper);
  CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, dma_occupancy);
  unsigned int filled = circular_buf2_occupancy(&dma_out_started);

  if( enable_output_stream == 1 && dma_occupancy != filled ) {

    CSR_WRITE(DMA_1_INTERRUPT_CLEAR, 0);
    // using a cirbuf we remember which dma was put first
    error = circular_buf2_get(&dma_out_started, &data); //MY_ASSERT(error == 0);
    // SET_REG(x3, (6<<8) | data );

    if(error == 0) {
      if( 1 ) { // always true, there is only one output dma
        // signal back to fft that output dma is done
        // only runs one for every 2 interrupts
        dma_out_ready = data & DMA_A_B_MASK; 

        pet_fft_respond_signals();
      }
    } else {
      // oh boy
      // not exactly sure how we are here but we are checking extra times for dma out being done
      // we can avoid this with better magic math above
    }

  }

}

// internal function which will advance the timing
void turnstile_advance(unsigned int samples) {
    // limit advance
    // FIXME, this could be done without % by masking the
    // next highest bitmaxk and doing a single subtract if greater than
    samples = samples % MAX_TURNSTILE_ADVANCE;

    pending_input_advance = samples;
}


// callback from ringbus
void turnstile_advance_callback(const unsigned int data) {
    turnstile_advance(data);
}

// data is already masked and has upper 8 bits removed at this point
void sync_callback_original(const unsigned int data) {
    // round down to make adjustment a multiple of 5
    unsigned amount = data - (data%5);

    const unsigned max_value = (25600*2);

    if(amount > max_value ) {
        amount = max_value;
    }
    
    // only run if we are given a large enough value
    if(amount >= 5) {
        pending_sync_symbol_timing = 1;
        global_coarse_sync_num = amount;
    }
    self_adjust_after_sync = 0;
}


// see apply_self_sync_after_coarse()
//     periodic_self_sync_adjust_tx_chain()
void sync_callback_duplex(const unsigned int data) {
    sync_callback_original(data);
    self_adjust_after_sync = 1;

    // send ringbus to cs32 with duplicate value
    CSR_WRITE(RINGBUS_WRITE_ADDR, RING_ADDR_RX_FINE_SYNC);
    CSR_WRITE(RINGBUS_WRITE_DATA, (DUPLEX_SYNCHRONIZATION_CMD|data));
    CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

}

void setup_cfo(void) {

  copy_set_barrel(config_word_cmul_eq_0f, cfg_cfo_mul,      0x0f);
  copy_set_barrel(config_word_conj_eq_0f, cfg_cfo_mul_conj, 0x0f);

  cfg_cmulti_location=VMEM_ROW_ADDRESS(cfg_cfo_mul);
  cfg_cmulco_location=VMEM_ROW_ADDRESS(cfg_cfo_mul_conj);

}




// pass a pointer to a result struct
// pass a number of iterations to run sync `coarse_sync_number`
void xbb_coarse_sync(const unsigned int coarse_sync_number) 
{
  
  // Ringbus ringbus;
  
  // auto_gain_ctrl(&ringbus);

  // for(int index=0;index<1000; index++)
  // {
  //   asm("nop");
  // }
  // ring_block_send_eth(0x12345678);

  ////////////////////////////////////////////
  //unsigned int coarse_sync_result_array[COARSE_SYNC_OFDM_NUM];
  //for(int index = 0; index<COARSE_SYNC_OFDM_NUM; index++)
  //{
  //  coarse_sync_result_array[index] = 0;
  //}

  //////////////////////////////////////////////////////////////////////////////
    //////// enable nco for cfo at the first time
    const unsigned int current_angle = next_angle + rbus_theta_shift;

    rbus_theta_shift = 0;

    unsigned int flag = nco_schedule_next_sfo(packet_counter_SFO_adjustment, packet_SFO_adjustment_nco_freq, current_angle);
    if(flag == 1)
    {
        next_angle = current_angle + rbus_omega;
        //ring_block_send_eth(0x88888888);
    }
    else if(flag == 0)
    {
        next_angle = current_angle;
        //ring_block_send_eth(0x77777777);
    }
    /////////////////////////////////////////////////////////////////////////////////


  unsigned int occupancy;


  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
  // SET_REG(x4, 0xe0000000 | occupancy);

  // unsigned int start_clk, end_clk;

   

  const unsigned int input_data_location = VMEM_ROW_ADDRESS(input_data);
  unsigned int dma_in_address = VMEM_DMA_ADDRESS(input_data);


  //const unsigned int exp_data_1280_ext_location=VMEM_ROW_ADDRESS(exp_data_1280_ext_14) + 80;
  unsigned int exp_data_1280_ext_location=VMEM_ROW_ADDRESS(exp_data_1280_ext_15) + 80;
    
  // const unsigned int output_coarse_sync_location = VMEM_ROW_ADDRESS(output_coarse_sync);
  const unsigned int output_sum_complex_location = VMEM_ROW_ADDRESS(output_sum_complex);
  const unsigned int output_onetone_calculation_location = VMEM_ROW_ADDRESS(output_onetone_calculation);
  const unsigned int output_conj_multi_location = VMEM_ROW_ADDRESS(output_conj_multi);
  // const unsigned int benchmark_coarse_sync_location=VMEM_ROW_ADDRESS(benchmark_coarse_sync);


  const unsigned int all_one_location=VMEM_ROW_ADDRESS(all_one);
  const unsigned int bank_address_onetone_calculation_location = VMEM_ROW_ADDRESS(bank_address_onetone_calculation);
  const unsigned int permutation_address_onetone_calculation_location = VMEM_ROW_ADDRESS(permutation_address_onetone_calculation);
  

  unsigned int input_conj_multi_location_0;
  unsigned int input_conj_multi_location_1;
  unsigned int exp_data_location;

  unsigned int dma_in_enable_indicator=0;
  unsigned int coarse_sync_indicator=0;
  unsigned int exp_data_indicator=0;

  // unsigned int offset_pre = 0;

  const unsigned int expected_occupancy=2;

  unsigned int flag_temp_complex = 1;
  unsigned int temp_complex;
  unsigned int temp_complex_pre = 0;
  // unsigned int index_done;
  // unsigned int index_done_pre = 0;
  // unsigned int index_final;
  unsigned int recorded_data_flag = 0; // did we ever overflow and record data into the histogram?


  unsigned int temp_angle;
  unsigned int temp_offset;
  // unsigned int temp_offset_benchmark;

  MVXV_KNOP(V11, output_sum_complex_location);
  MVXV_KNOP(V10, output_onetone_calculation_location);

  // SET_REG(x3, 0xdead0002);

  // use dma_block_get here so we will automatically not overflow input dma
  for(int index=0; index<4; index++)
  {
    // SET_REG(x4, 0xa0000000 | index);
    dma_block_get(dma_in_address, DMA_IN_CHUNK_CS);
    if (dma_in_enable_indicator!=4)
    {
        dma_in_address += DMA_IN_CHUNK_CS;
        dma_in_enable_indicator++;
    }
    else if(dma_in_enable_indicator==4)
    {
      dma_in_address=VMEM_DMA_ADDRESS(input_data);
      dma_in_enable_indicator=0;
    }
  }

  // SET_REG(x4, 0xb0000000);
   

  // we have downtime will the occupancy drops to 2
  // so do this now
  for(unsigned int i = 0; i < 16; i++) {
    output_sum_complex[i] = 0;
  }
  STALL(100);


  
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  // this could be moved off the stack
  unsigned int coarse_sync_estimate_array[(1280>>6)] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  // soft_queue_ring_eth(0x66666666);
  unsigned int index_counter = 0;

  unsigned int temp_offset_average = 0;
  unsigned int temp_offset_counter = 0;
  // for each iteration
  for(unsigned index=0; index<coarse_sync_number; index++)
  {

    // SET_REG(x4, 0xc0000000 | index);
    
    while(1)
    {
     CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
     // SET_REG(x4, 0xd0000000 | occupancy);
     if(occupancy == expected_occupancy) 
     {
      break;
     }

    }

    // CSR_READ(TIMER_VALUE, start_clk);

    //////// do coarse sync
    input_conj_multi_location_0=input_data_location+coarse_sync_indicator*64;
    coarse_sync_indicator = (coarse_sync_indicator+1);
    if (coarse_sync_indicator == 5) {
        coarse_sync_indicator = 0;
    }
    
    input_conj_multi_location_1=input_data_location+(coarse_sync_indicator)*64;

    // see https://beta.observablehq.com/@drom/ofdm-symbol-synchronisation/2

    // CSR_READ(TIMER_VALUE, start_clk);
    xbb_conj_multi(
        variable_coarse_mul_row, 
        input_conj_multi_location_0, // previous 1024 samples X_m-1
        input_conj_multi_location_1, // current 1024 samples  X_m
        output_conj_multi_location // output Y_m
        );
    // CSR_READ(TIMER_VALUE, end_clk);   
    exp_data_location=exp_data_1280_ext_location;

    xbb_onetone_calculation(
        variable_coarse_fft_row,
        output_conj_multi_location,        // input to this function, which is the output of the previous Y_m
        exp_data_location,                 // start address of the exponential sin wave (1024 samples)
        output_onetone_calculation_location,      // this is a 1024 vector of output, but only [0] has valid data
        bank_address_onetone_calculation_location, // data for internals of xbb_onetone_calculation
        permutation_address_onetone_calculation_location, // data for internals
        all_one_location // data for internals
        );

    if(exp_data_indicator != 4)
    {
        exp_data_indicator++;
        exp_data_1280_ext_location -= 16;
    }
    else if (exp_data_indicator == 4)
    {
        exp_data_indicator = 0;
        exp_data_1280_ext_location += 64;

    }

    
    STALL(30);
   
    // MVXV_KNOP(V12, 1);
    // VNOP_LK15(V10); 
    // ADD_SK15(V11, V11, V12, 0);
    // STALL(10);
    // STALL(10);
    // STALL(10);
    // temp_complex = vector_memory[(output_coarse_sync_location+index)*16];

    ////////////////////////////////////
    //
    //  see above (outside the loop) where we have
    //    MVXV_KNOP(V11, output_sum_complex_location);
    //    MVXV_KNOP(V10, output_onetone_calculation_location);
    //
    // The following XBB commands add output_onetone_calculation_location[0] to output_sum_complex_location[0]
    // with every iteration of the loop.  the result is acumulated over all loop runs.
    // this term is a_m

    MVXV_KNOP(V0, variable_coarse_add_row);

    VNOP_LK14(V0);

    FLUSH_CONFIG_WORD(V15);

    VNOP_LK8(V10);
    VNOP_LK9(V11);
    VNOP_SK1(V11);
    STALL(50);

    temp_complex = vector_memory[(output_sum_complex_location)*16];

    // index_final = index;

    // flag_temp_complex 0 means overflow
    //                   1 means ok

    // check for overflow
    if (((temp_complex & 0xffff) == 0x7fff) || ((temp_complex & 0xffff) == 0x8000))
    {
        flag_temp_complex = 0;
    }
    if ((((temp_complex>>16) & 0xffff) == 0x7fff) || (((temp_complex>>16) & 0xffff) == 0x8000))
    {
        flag_temp_complex = 0;
    }

    if(flag_temp_complex == 1)
    {
        // index_done = index;

        // if we didn't overflow, save into temp_complex_pre
        temp_complex_pre = temp_complex;
    }

    // is this the last loop?
    const unsigned last_loop = (index == (coarse_sync_number-1));

    // we force enter data into the histogram if this is the last loop iteration
    // and so far we have not recorded anything.  this protects against small ofdm numbers
    // returning 0
    const unsigned save_last_loop = last_loop && (recorded_data_flag==0);

    // once we overflow, we don't use overflown (current) data, instead we use 
    // temp_complex_pre, which was the last good value before overflow
    // this only checks every 5 dma
     if( ( (flag_temp_complex == 0) && (index_counter == 4) ) || save_last_loop ) {
        recorded_data_flag = 1;

        ATAN(temp_angle,temp_complex_pre,15);
        temp_angle = temp_angle&0xffff;
        temp_offset = ((((temp_angle+6553)&0xffff)*1280)>>16);

        temp_offset_average = temp_offset_average + temp_offset;
        temp_offset_counter = temp_offset_counter + 1;

        if(temp_offset == 1280) {
            temp_offset = 0;
        }
        
        // warning, this is always true
        // if( (index_done - index_done_pre) >= 0 ) {
        if( 1 ) {
            coarse_sync_estimate_array[temp_offset>>6] = coarse_sync_estimate_array[temp_offset>>6]+1;
            //ring_block_send_eth(index_done-index_done_pre);
            // ring_block_send_eth(temp_offset);
        }

        vector_memory[(output_sum_complex_location)*16] = 0;
        STALL(50);

        flag_temp_complex = 1;
        // index_done_pre = index+1;

       

     }

     index_counter = index_counter + 1;
     if(index_counter == 5) {
        index_counter = 0;
     }




    // if (((temp_complex & 0xffff) == 0x7fff) || ((temp_complex & 0xffff) == 0x8000))
    // {
    //     ring_block_send_eth(index);
    //     ring_block_send_eth(temp_complex);
    // }

    //  if ((((temp_complex>>16) & 0xffff) == 0x7fff) || (((temp_complex>>16) & 0xffff) == 0x8000))
    // {
    //     ring_block_send_eth(index);
    //     ring_block_send_eth(temp_complex);
    // }

    

    //coarse_sync_result_array[index] = temp_offset;

    // offset_pre = temp_offset;
    // ring_block_send_eth(index);
    // //ring_block_send_eth(temp_angle);
    // ring_block_send_eth(temp_offset);

    // CSR_READ(TIMER_VALUE, end_clk);

    CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
    // SET_REG(x4, 0xd0000000 | occupancy);
    dma_in_set(dma_in_address,  DMA_IN_CHUNK_CS);
    if (dma_in_enable_indicator!=4)
    {
        dma_in_address += DMA_IN_CHUNK_CS;
        dma_in_enable_indicator++;
    }
    else if(dma_in_enable_indicator==4)
    {
        dma_in_address=VMEM_DMA_ADDRESS(input_data);
        dma_in_enable_indicator=0;
    }


    
     
    // performance check
    // vector_memory[(output_coarse_sync_location+index)*16] = temp_offset;
    // temp_offset_benchmark=vector_memory[(benchmark_coarse_sync_location)*16+index];
    // ring_block_send_eth(index);
    // ring_block_send_eth(temp_offset);
    // ring_block_send_eth(temp_offset_benchmark);
    // ring_block_send_eth(end_clk-start_clk);

    // hand_tune_coarse_sync_zeros();
    // hand_tune_coarse_sync_zeros();
    // hand_tune_coarse_sync_zeros();
    // hand_tune_coarse_sync_zeros();

  } //// for index < coarse_sync_number




  // ring_block_send_eth(0x88888888);
  // ring_block_send_eth(temp_offset);
  temp_offset = (unsigned int)(temp_offset_average*1.0f)/(temp_offset_counter *1.0f);
  // ring_block_send_eth(temp_offset);

  unsigned int max_index = 0;
  for(unsigned int index = 0; index<(1280>>6); index++)
  {
    if(coarse_sync_estimate_array[index]>coarse_sync_estimate_array[max_index])
        max_index = index;
  }

  temp_offset = max_index*64;

  // at this point we are done with the sync, CFO comes next

  // performance check
  // SET_REG(x3, 0x88888888);
  // ring_block_send_eth(0x88888888);
  // int flag = result_check(benchmark_coarse_sync_location, output_coarse_sync_location, coarse_sync_number, 4);
  // if (flag==1)
  //   ring_block_send_eth(0xffffffff);
  // else
  //   ring_block_send_eth(0x00000000);

  // temp_offset = offset_pre;
  // ring_block_send_eth(temp_complex);
  // ring_block_send_eth(offset_pre);
  // ring_block_send_eth(COARSE_SYNC_PCCMD | (temp_offset&DATA_MASK));

  // CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
  // ring_block_send_eth(occupancy);

  //  CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
  //  ring_block_send_eth(occupancy);

  // while(1)
  // {
  //   CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
  //   if(occupancy==expected_occupancy)
  //   {
  //       break;
  //   }
  // }
  
  // dma_block_get(dma_in_address,  DMA_IN_CHUNK_CS);
  // bump_xbb_samples_consumed(DMA_IN_CHUNK_CS);
  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
  // SET_REG(x4, 0xd0000000 | occupancy);
  dma_in_set(dma_in_address,  DMA_IN_CHUNK_CS);
  if (dma_in_enable_indicator!=4)
  {
      dma_in_address += DMA_IN_CHUNK_CS;
      dma_in_enable_indicator++;
  }
  else if(dma_in_enable_indicator==4)
  {
      dma_in_address=VMEM_DMA_ADDRESS(input_data);
      dma_in_enable_indicator=0;
  }

  // ring_block_send_eth(0xdeaddead);
  // CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
  // ring_block_send_eth(occupancy);
  // ring_block_send_eth(coarse_sync_indicator);
  // ring_block_send_eth(dma_in_enable_indicator);
  // ring_block_send_eth(0xbeefbeef);


  while(1)
  {
    CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
    if(occupancy==expected_occupancy)
    {
        break;
    }
  }

  SET_REG(x3, 0xe0000000);
  SET_REG(x3, 0x00000000);

  dma_in_set(VMEM_DMA_ADDRESS(holdover_data), BOTH_HOLDOVER_SIZE);

  // CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
  // ring_block_send_eth(occupancy);

  // report value to pc
  ring_block_send_eth(COARSE_SYNC_PCCMD | (temp_offset&DATA_MASK));
  coarse_sync_result = temp_offset;

}

void pet_symbol_sync(void) {

    if( fsm_state != FSM_DO_SYNC ) {
        return;
    }

    xbb_coarse_sync(global_coarse_sync_num);
    packet_counter_SFO_adjustment = 0;
    sync_finished = 1;

    // fudge of 9 here is hand calculated and accounts for both holdover
    // plus 1 I can't quite account for??
    const unsigned holdover_counter_fudge = 9;

    lifetime_32 += (global_coarse_sync_num*4/5) + holdover_counter_fudge;

    ring_block_send_eth(DEBUG_COARSE_COUNTER_PCCMD | occupancy_before_coarse);
    occupancy_before_coarse = 0;

}

#include "calculate_power.h"



unsigned int _get_sat_ratio(void) {
    unsigned int samples_saturated;

    // Clear saturation counter
    CSR_READ(SATDETECT, samples_saturated);
    for(unsigned int i = 0; i < 204; i++){
        asm("nop");
    }
    CSR_READ(SATDETECT, samples_saturated);
    samples_saturated = samples_saturated>>16;

    return samples_saturated;
}


static unsigned int underflow_then;

void reset_underflow_counter(void) {
    CSR_WRITE(CS_CONTROL, 0x1);
    CSR_WRITE(CS_CONTROL, 0x0);
}

unsigned grab_underflow_counter(void) {
    unsigned int riscv_status;
    CSR_READ(CS_STATUS, riscv_status);
    return riscv_status;
}


void ringbus_send_counter(unsigned counter) {
    ring_block_send_eth(TX_UNDERFLOW|0x070000|(counter&0xffff));
    ring_block_send_eth(TX_UNDERFLOW|0x080000|((counter>>16)&0xffff));
}



static unsigned pending_underflow_report = 0;


void request_underflow_report_callback(const unsigned int data) {
    (void)data;
    pending_underflow_report = 1;
}





#define UNDERFLOW_BECAME_OK  (0x050000)
#define UNDERFLOW_BECAME_BAD (0x060000)


// 0 is ok 
// 1 is underflowing

static unsigned underflow_state = 1; // start in underflowing state

void check_error_counter(void) {
    // CSR_WRITE(CS_CONTROL, err_state);

    unsigned int now;
    CSR_READ(TIMER_VALUE, now);

    unsigned counter_delta = subtract_timers(now,underflow_then);

    if(counter_delta > (125000000/10)) {
        underflow_then = now;

        unsigned int counter;
        counter = grab_underflow_counter();

        unsigned underflow_now = counter!=0;

        unsigned int transition = ((underflow_state & 0x1) << 1) | (underflow_now);

        if( pending_underflow_report ) {
            pending_underflow_report = 0;

            // if we are requesting a report
            // set the "then" bit to the opposite of the "now" bit
            // which will force a report

            transition = (transition & (~0x2)); // clear bit
            if(!underflow_now) {
                transition |= 0x2; // set bit if now is unset, otherwise, leave unset
            }
        }

        // bit 1           |    bit 0
        // was underflow   |    currently underflow

        switch(transition) {

            // was not underflowing, currently not underflowing
            case 0x0:
                // do nothing
                break;

            // was not underflowing, currently underflowing
            case 0x1:
                ring_block_send_eth(TX_UNDERFLOW|UNDERFLOW_BECAME_BAD);
                ringbus_send_counter(counter);
                underflow_state = 1;
                break;

            // was underflowing, currently not underflowing
            case 0x2:
                ring_block_send_eth(TX_UNDERFLOW|UNDERFLOW_BECAME_OK);
                underflow_state = 0;
                break;

            // was underflowing, currently underflowing
            case 0x3:
                // do nothing
                break;
            default:
                // illegal!
                break;
        }

        // ring_block_send_eth_debug(riscv_status);
        // ring_block_send_eth(riscv_status);
        // ring_block_send_eth(TX_UNDERFLOW|(riscv_status & 0xffffff));
        reset_underflow_counter();
        
    }
}



#ifdef ENABLE_DBGPOWER

unsigned current_shift = 18;

void setup_debug_pwr(void) {

#ifdef POWER_WAIT_FOR_RING
    power_est_state = PWR_DISABLE;
#endif
    
}


uint64_t signal_power_sum = 0;

void debug_power(void) {
    SET_REG(x3, 0x21000000);
    unsigned now;
    CSR_READ(TIMER_VALUE, now);

    int next_state = power_est_state;

    // && now > 0x8000
    if( power_est_state == PWR_ANALYZE ) {
        SET_REG(x3, 0x30000000);
        next_state = PWR_JUDGE;

        power_smart_shift(current_shift);

        

        uint32_t get_power_from_row = VMEM_ROW_ADDRESS(power_data_copy);


        signal_power_sum = calculate_power(get_power_from_row);

        SET_REG(x3, 0x50000000);


        // this will already have upper most bit set for saturation
        const uint64_t shift_flag = ((uint64_t)current_shift & 0xff) << 48;
        const uint64_t ringbus_back = signal_power_sum | shift_flag;


        // ring_block_send_eth(power_est_state);
        // STALL(100);
        soft_queue_ring_eth(POWER_RESULT_PCCMD | (ringbus_back & 0xffff) );
        
        soft_queue_ring_eth(POWER_RESULT_PCCMD | 0x00010000 | ((ringbus_back>>16) & 0xffff));
        

        soft_queue_ring_eth(POWER_RESULT_PCCMD | 0x00020000 | ((ringbus_back>>32) & 0xffff));
        
        soft_queue_ring_eth(POWER_RESULT_PCCMD | 0x00030000 | ((ringbus_back>>48) & 0xffff));
        

        // soft_queue_ring_eth(DEBUG_0_PCCMD | calculate_power_runtime_0() );
        
        // soft_queue_ring_eth(DEBUG_1_PCCMD | calculate_power_runtime_1() );


        SET_REG(x3, 0x60000000);
    }

    if( power_est_state == PWR_JUDGE ) {
        char advice;
        int direction;
        judge_power_results(signal_power_sum, current_shift, &advice, &direction);
        // advice will return 'o' for ok
        // or 'c' for change, in change we just add the value
        if( advice == 'c' ) {
            current_shift += direction;
            next_state = PWR_ANALYZE;
        } else {
            next_state = PWR_DISABLE; // debug park 
        }

    }

    power_est_state = next_state;

}


void trigger_power_callback(unsigned int data) {
    if( data == 0 ) {
        if( power_est_state == PWR_DISABLE ) {
            power_est_state = PWR_BOOT;
        }
    }
}

#endif


void setup_barrel_shift(void) {
    variable_coarse_mul_row = VMEM_ROW_ADDRESS(variable_coarse_mul);
    variable_coarse_fft_row = VMEM_ROW_ADDRESS(variable_coarse_fft);
    variable_coarse_add_row = VMEM_ROW_ADDRESS(variable_coarse_add);
}

// VMEM_ROW_ADDRESS(config_word_conj_eq_0f), 
// VMEM_ROW_ADDRESS(config_word_cmul_rx4_0f),
// MVXV_KNOP(V0, VMEM_ROW_ADDRESS(config_word_add_eq_00));

void app_barrel_shift_callback(const unsigned int data) {
    // APP_BARREL_SHIFT_CMD
    const unsigned int stage = ((data & 0x00FF0000) >> 16);
    const unsigned int shift = ((data & 0xffff));

    unsigned short* cpu_source;
    unsigned short* cpu_dest;

    // we need to set conj and non conj the same here
    switch(stage) {
        case 0:
            cpu_source = config_word_conj_eq_0f;
            cpu_dest   = variable_coarse_mul;
            break;
        case 1:
            cpu_source = config_word_cmul_rx4_0f;
            cpu_dest   = variable_coarse_fft;
            break;
        case 2:
            cpu_source = config_word_add_eq_00;
            cpu_dest   = variable_coarse_add;
            break;
        default:
            return; // exit the function
            break;
    }

    copy_set_barrel(cpu_source, cpu_dest, shift);
}

void default_barrel_shift_callback(const unsigned int data) {
    app_barrel_shift_callback(0x000000 | 0x0f);
    app_barrel_shift_callback(0x010000 | 0x0f);
    app_barrel_shift_callback(0x020000 | 0x00);

    fft_barrel_shift_defaults(data);
}



// run first
void lower_callback(const unsigned int data) {
    // check_x4 = data;
    delta_low = (data & 0xffff);

    
}

// run to trigger
// void upper_callback(unsigned int data) {
//     // check_x4 = data;
//     rbus_omega = (((data & 0xffff) << 16) | delta_low);
// }

// set upper last, copies and clears lower
void upper_callback(unsigned int data) {

    

    const unsigned int cmd = ((data & 0x00FF0000) >> 16);
    data = ((data & 0xffff) << 16);
    


    if (cmd == 0x00){
        rbus_omega = (data | delta_low);
        rbus_omega_sign = 0;
    } else if (cmd == 0x01){
        rbus_omega = (data | delta_low);
        rbus_omega_sign = 1;
    } else if (cmd == 0x02){
        rbus_theta_shift = (data | delta_low);
    }
    delta_low = 0;
    // check_x3 = 0xbabe;

    ring_block_send_eth(0x81000000|rbus_omega);
}

void cfo_turnoff_callback(const unsigned int data) {
    (void)data;
    sync_finished_hold = 0;
    ring_block_send_eth(0x83aaaaaa);
}

void cooked_data_type_callback(const unsigned int data) {
    cooked_data_type = data;
}



uint32_t* pointer_for_generic_op(const uint32_t sel) {
    uint32_t *p = 0;
    switch(sel) {
        case 0:
            p = &lifetime_32;
            break;
        case 1:
            p = (uint32_t*)&sync_finished_hold;
            break;
        case 2:
            p = (uint32_t*)&rbus_omega_sign;
            break;
        case 3:
            p = (uint32_t*)&packet_SFO_adjustment_direction;
            break;
        case 4:
            p = (uint32_t*)&times_nco_driver_failed;
            break;
        case 10:
            p = (uint32_t*) &duplex.role;
            break;
        case 12:
            p = (uint32_t*)&enable_composite_ul_pilot;
            break;
        case 13:
            p = (uint32_t*)&enable_freeze_wide_ul_pilot;
            break;
        // case 1:
        //     p = &demod.state;
        //     break;
        // case 2:
        //     // most significant
        //     p = ((uint32_t*)&demod.off_filter_state) + 1;
        //     break;
        // case 3:
        //     // least significant
        //     p = (uint32_t*)&demod.off_filter_state;
        //     break;
        // case 4:
        //     // most significant
        //     p = ((uint32_t*)&demod.on_filter_state) + 1;
        //     break;
        // case 5:
        //     // least significant
        //     p = (uint32_t*)&demod.on_filter_state;
        //     break;
        // case 6:
        //     p = &demod.filter_gain_off;
        //     break;
        // case 7:
        //     p = &demod.filter_gain_on;
        //     break;
        default:
            break;
    }
    return p;
}

void generic_op_finished(const uint32_t sel, const uint32_t op, const uint32_t value ) {
    (void)op;
    (void)value;
    switch(sel) {
        case 10:
            update_duplex_role(&duplex);
            break;
    }
}


uint32_t prev_sfo_cfo_report = 0;

void slow_report_sfo_cfo(void) {
    uint32_t delta = invalid_sfo_cfo - prev_sfo_cfo_report;
    if( delta > 8000 ) {
        prev_sfo_cfo_report = invalid_sfo_cfo;
        ring_block_send_eth( INVALID_SFO_CFO_PCCMD | (invalid_sfo_cfo&0xffffff));
        // INVALID_SFO_CFO_PCCMD
    }
}

void setup_duplex(void) {
    init_duplex(&duplex, DUPLEX_ROLE_RX);
    init_duplex(&duplex_rx, DUPLEX_ROLE_RX);
}

void setup_pointers(void) {
    duplex_composite_ul_pilot_row = VMEM_ROW_ADDRESS(duplex_composite_ul_pilot);
    duplex_composite_ul_pilot_dma = VMEM_DMA_ADDRESS(duplex_composite_ul_pilot);
    garbage_row = VMEM_ROW_ADDRESS(garbage_vmem);
}


int main2(void);
int main(void)
{
    self_sync_block_boot();
    main2();
}
int main2(void) {
    // put this first so we can have a more "determinstic" wakeup
    setup_dma_in();

    is_verilator = get_is_verilator();


#ifdef STREAM_DEFAULT_TO_ON
    enable_output_stream = 1;
#else
    enable_output_stream = 0;
#endif
    pending_input_advance = 0;

    // int occupancy;

    CSR_READ(TIMER_VALUE, last_error_report);


    init_nco_sfo();


    setup_calculate_power();

    CIRBUF_POW2_RUNTIME_INITIALIZE(__soft_ring_queue);
    setup_soft_ring(2, 1);

#ifdef ENABLE_DBGPOWER
    setup_debug_pwr();
#endif

    CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);


    ring_register_callback(&sfo_adjustment_callback, SFO_PERIODIC_ADJ_CMD);
    ring_register_callback(&sfo_sign_callback, SFO_PERIODIC_SIGN_CMD);



    ring_register_callback(&turnstile_advance_callback, TURNSTILE_CMD);
    ring_register_callback(&sync_callback_original, SYNCHRONIZATION_CMD);
    ring_register_callback(&sync_callback_duplex, DUPLEX_SYNCHRONIZATION_CMD);
    ring_register_callback(&corrupt_dma_callback, CORRUPT_DMA_OUT_CMD);
    ring_register_callback(&check_bootload_status, CHECK_BOOTLOAD_CMD);
    ring_register_callback(&request_underflow_report_callback, REQUEST_UNDERFLOW_REPORT_CMD);
    ring_register_callback(&fft_barrel_shift_callback, FFT_BARREL_SHIFT_CMD);
    ring_register_callback(&trigger_power_callback, POWER_ESTIMATION_CMD);
    ring_register_callback(&app_barrel_shift_callback, APP_BARREL_SHIFT_CMD);
    ring_register_callback(&default_barrel_shift_callback, DEFAULT_APP_BARREL_SHIFT_CMD);
    ring_register_callback(&readback_timer_callback, GET_TIMER_CMD);

    ring_register_callback(&lower_callback, TX_CFO_LOWER_CMD);
    ring_register_callback(&upper_callback, TX_CFO_UPPER_CMD);


    ring_register_callback(&cfo_turnoff_callback, CFO_TURN_OFF_CMD);
    ring_register_callback(&handle_generic_callback_original, GENERIC_OPERATOR_CMD);
    ring_register_callback(&cooked_data_type_callback, COOKED_DATA_TYPE_CMD);

    handle_generic_register_post_callback(&generic_op_finished);
    handle_generic_register_get_pointer(&pointer_for_generic_op);
    handle_generic_register_ring(&ring_block_send_eth);

    setup_barrel_shift();
    default_barrel_shift_callback(0);

    setup_duplex();
    setup_pointers();


    // this can allow manual setting of barrel stages at boot
    // fft_1024_set_bs(&active_plan, 4, 0x0f);

    setup_fsm();
    setup_fft();
    setup_dma_out();
    setup_cfo();
    // unsigned int counter = 0;
    // SET_REG(x3, 0xdead0000);

    // ring_block_send_eth(0xdead0000);
    // ring_block_send_eth(VMEM_ROW_ADDRESS(dma_buffer_a));
    // ring_block_send_eth(VMEM_ROW_ADDRESS(dma_buffer_b));

    // _setup_perf();
    // ring_block_send_eth(dma_in_ptr[0]);
    // ring_block_send_eth(dma_in_ptr[1]);
    // ring_block_send_eth(fft_ptr[0]);
    // ring_block_send_eth(fft_ptr[1]);

    // wait till zhen's finish triggering

    Ringbus ringbus;
    // unsigned int helper;


    int power_counter = 0;


  while(1) {
    pet_fsm();
    pet_dma_in();
    pet_fft();
    pet_dma_out();
    pet_dma_out();
    pet_dma_out();
    pet_dma_out();
    pet_symbol_sync();
    check_ring(&ringbus);

#ifdef ENABLE_DBGPOWER
    if( power_counter >= 4 ) {
        debug_power();
        power_counter = 0;
    } else {
        power_counter++;
    }
#endif

    pet_soft_ring();
    check_error_counter();
    periodic_self_sync_adjust_tx_chain();
    slow_report_sfo_cfo();

    // if(counter == 100000) {
        // ring_block_send_eth(ringbus_sfo_adjustment_temp);
        // ring_block_send_eth(packet_SFO_adjustment_direction);
        // counter = 0;
    // }
    // counter++;
    // unsigned int mem_free = vmalloc_available(&mgr);
    // ring_block_send_eth(0xc0000000 | mem_free);
  }
  
}
