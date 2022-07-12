#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "dma.h"
#include "fill.h"
#include "mover.h"
#include "ringbus.h"
#include "coarse_sync.h"
#include "trunk_types.h"
#include "duplex_schedule.h"


#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "readback_hash.h"

#include "vector_multiply.h"
#include "vmem_copy.h"

#include "nco_data.h"

#include "handle_generic_op.h"

#include "config_word_cmul_eq_0f.h"
#include "config_word_conj_eq_0f.h"
#include "config_word_add_eq_00.h"

#include "eq_random_rotation.h"
#include "get_timer.h"
#include "self_sync.h"
#include "random.h"
#include "ook_modem.h"
#include "copy_config.h"
// #include "performance.h"

// #define READBACK_RINGBUS_HASH

// #define ENABLE_TB_DEBUG
// #define FBPARSE_USE_DEBUG

#define FEEDBACK_BUS_USER_POINTERS 36


#ifndef ENABLE_TB_DEBUG
#define DISABLE_TB_DEBUG
#endif
#include "tb_debug.h"

#ifndef ENABLE_TB_DEBUG
#undef FBPARSE_USE_DEBUG
#endif
#include "feedback_bus_parse.h"

#define READBACK_EQ_HASH
// #define READBACK_FULL_EQ

// #define MEASURE_STACK_USAGE

#ifdef MEASURE_STACK_USAGE
#include "stack_test.h"
#endif

#include "schedule.h"
#include "check_bootload.h"

#define INCLUDE_COUNTER_BASE (0xff000000)
#define INCLUDE_COUNTER_AS vmem_counter_1k
#include "vmem_counter_1k.h"

// If a Userdata packet comes this many ofdm frames early, we will drop it
// and report an error.  ~4 seconds
#define EARLY_UD_FRAMES_ERROR ((SCHEDULE_FRAMES)*4)


// difference between tx rx chain after self sync runs
// this value was hand tuned
#define TX_RX_CHAIN_FRAME_COUNTER_DELTA (5)

#define FFT_SIZE (1024)

#define DMA_OUT_CHUNK (FFT_SIZE+TRUNK_LENGTH)

// this fpga uses a 5-pong
// this fpga does not use the ping-pong driver
#define PONG_BUFFERS (5)


// the subcarrier which the tdma stuff overwrites
static unsigned ook_subcarrier = 21;
static unsigned int lifetime_32 = TX_RX_CHAIN_FRAME_COUNTER_DELTA;
static schedule_t _schedule;
static schedule_t* schedule = &_schedule;

static unsigned int lifetime_frame_counter = 0; // for sfo


// static unsigned int pending_epoc_update = 0;

static unsigned cooked_data_type = 0;

// tx side corrects it's EQ with this during duplex schedule
static VMEM_SECTION unsigned short variable_eq_correction_mul[32];

static VMEM_SECTION unsigned short variable_analog_eq_mul[32];


static uint32_t duplex_mode = 0;
static uint32_t duplex_progress = 0;
duplex_timeslot_t duplex;

uint32_t apply_eq_correction = 0;

static uint32_t duplex_stall = 0;

static unsigned tone_2_value = 0x00002000;

// #define HARDWARE_MODE


// buffer of allocated dma pointers
// FEEDBACK_BUS_USER_POINTERS is 8, how many buffers we can allocate out at once
circular_buf_pow2_t __full_dma_pointers = CIRBUF_POW2_STATIC_CONSTRUCTOR(__full_dma_pointers, FEEDBACK_BUS_USER_POINTERS_NEXT_POW2);
circular_buf_pow2_t* full_dma_pointers = &__full_dma_pointers;

// how many frames to read into our buffer ahead of time
#define USER_DATA_PAUSE_AFTER ((FEEDBACK_BUS_USER_POINTERS)-1)

static unsigned int output_frame_count = 0;

static unsigned int epoc_needs_latch = 0;

static unsigned int epoc_was_latched = 0;
static unsigned int epoc_latch_frame = 0;
static unsigned int epoc_latch_second = 0;

static unsigned int progress_report_remaining = 0;

#define MAXIMUM_MASK_SC (4)
static uint32_t mask_sc[MAXIMUM_MASK_SC];
static uint32_t mask_sc_enabled = 0;



/// 
/// States for userdata state machine
///
#define UD_NOT_ACTIVE (0)
#define UD_BUFFER (1)
#define UD_WAIT (2)
#define UD_SENDING (3)
#define UD_SENDING_TAIL (4)
#define UD_DUMPING (5)

#ifdef ENABLE_TB_DEBUG
static const char* state_string[] = {"UD_NOT_ACTIVE", "UD_BUFFER", "UD_WAIT", "UD_SENDING", "UD_SENDING_TAIL", "UD_DUMPING"};
#endif

static unsigned int user_callbacks = 0;
static unsigned int ud_state = UD_NOT_ACTIVE;
// requested timeslot, epoc
static unsigned int ud_lifetime = 0; // was ud_frame was ud_timeslot
// static unsigned int ud_epoc = 0;  // goes away
static unsigned int ud_total_chunks = 0;
static unsigned int ud_chunk_index_consumed = 0; // chunk index of the next thing we pull out of the fifo
static unsigned int ud_chunk_length;
static unsigned int ud_asked_for_pause;


static VMEM_SECTION unsigned int vmem_zeros[FFT_SIZE];
unsigned int vmem_zeros_row;

// #include "demod_output.h"
// #include "cooked_data.h" // vmem_counter
// #include "cooked_data_unrotated.h" // vmem_counter
#include "cooked_data_unrotated_320.h" // vmem_counter
#include "equalization_vectors.h" // eq_vector eq_vector_default eq_vector_corrected

#include "vmem_all_pilot_320.h"

#include "feedback_gain_vector.h" // feedback_gain_vector

static unsigned int vmem_all_pilots_row;


static VMEM_SECTION unsigned int nco_data[FFT_SIZE];


static VMEM_SECTION unsigned t0_pilot[FFT_SIZE];
static VMEM_SECTION unsigned t1_pilot[FFT_SIZE];
static VMEM_SECTION unsigned t2_pilot[FFT_SIZE];
static VMEM_SECTION unsigned t3_pilot[FFT_SIZE];

static unsigned* pilot_frame_cpu[4];
static unsigned pilot_frame_row[4];


static VMEM_SECTION unsigned partial_pilot_0[FFT_SIZE];
static VMEM_SECTION unsigned partial_pilot_1[FFT_SIZE];



// static VMEM_SECTION unsigned int input_copy_modify[1024] = {0};
static VMEM_SECTION unsigned int trunk[TRUNK_LENGTH*PONG_BUFFERS] = {};



// true if we are waiting to apply
static unsigned int pending_lifetime_update = 0;
// if pending is true, when we apply
static unsigned int incoming_future_lifetime_counter;


static unsigned int pending_packet_SFO_adjustment_nco_freq = 0;
static unsigned int pending_packet_num_SFO_adjustment_period;
static unsigned int pending_packet_SFO_adjustment_direction;

static unsigned int packet_SFO_adjustment_nco_freq;
static unsigned int packet_num_SFO_adjustment_period;
static unsigned int packet_SFO_adjustment_direction;
static unsigned int packet_counter_SFO_adjustment = 0;
static unsigned int ringbus_sfo_adjustment_temp = 0;

static unsigned int userdata_needs_latency_report = 0;

#ifdef READBACK_EQ_HASH
static unsigned int eq_need_hash = 0;
static unsigned int eq_hash_state = 0;
static unsigned int eq_hash_progress = 0;
#endif

static unsigned int stall_check = 0;

static uint32_t enable_composite_ul_pilot = 1;


// if a global flag is set
// we calculate an epoc value based on the current counter.near the end of the current schedule second
// void apply_epoc_update(schedule_t *o, const uint32_t accumulated_progress) {
//     if( !pending_epoc_update ) {
//         return;
//     }

//     if( accumulated_progress == (SCHEDULE_FRAMES-2) ) {
//         pending_epoc_update = 0;

//         o->epoc_time = lifetime_32 / SCHEDULE_FRAMES;
//     }
// }


static bool run_sfo = false;

// called after the last dma from this userdata has been queued
// SETS the state to UD_NOT_ACTIVE among other things
void user_data_finished(void) {
    ud_chunk_index_consumed = 0;
    fb_unpause_mapmov();
    ud_lifetime = 0;
    ud_chunk_length = 0;
    ud_asked_for_pause = 0;

    ud_state = UD_NOT_ACTIVE;

    // unsigned int fill_level = circular_buf2_occupancy(full_dma_pointers);

    // if(fill_level != 0) {
    //     ring_block_send_eth(TX_USERDATA_ERROR | 7);
    // }
}

void check_subcarrier(unsigned int* data) {
    uint64_t real;
    uint64_t imag;
    // float real;
    // float imag;

    const unsigned int sc = 1022;

    unsigned int mem = data[sc];

    real = (int16_t)(mem & 0xffff);
    imag = (int16_t)(mem>>16 & 0xffff);

    // real = int_real;
    // imag = int_imag;

    uint64_t mag2 = (real*real) + (imag*imag);

    const uint64_t tol = (0x1f00*0x1f00);

    if( mag2 < tol ) {
        ring_block_send_eth(TX_MAG_CHECK_PCCMD | 0x20000 | ((uint32_t)(real&0xffff)) );
        ring_block_send_eth(TX_MAG_CHECK_PCCMD | 0x30000 | ((uint32_t)(imag&0xffff)) );
    }
}

static unsigned int last_timeslot;
// static unsigned int last_epoc;

uint32_t ook_symbol_value = 0x2001;


/// user_data_callback is registered with 
/// 
void __attribute__((optimize("Os"))) user_data_callback(
    unsigned int *cpu_header,
    const unsigned int dma_body,
    const unsigned int this_chunk_length,
    const unsigned int chunk_index,
    const unsigned int total_chunks) {

    unsigned int insta_free;

    feedback_frame_vector_t* header = (feedback_frame_vector_t*) cpu_header;

    //check_subcarrier(REVERSE_VMEM_DMA_ADDRESS(dma_body));
    
    // ring_block_send_eth(DEBUG_1_PCCMD | user_callbacks | (header->seq<<16) );

    // save the body, we always have to do this, if we don't we drop data
    // this means we need to pause the feedback bus at any time that we can't accept
    // additional pointers
    int insert_error = circular_buf2_put(full_dma_pointers, dma_body);

    unsigned int fill_level = circular_buf2_occupancy(full_dma_pointers);

    if( chunk_index == 0 ) {
        dump_vmem_cpu(1, cpu_header, 16);
        dump_vmem_dma(2, dma_body, 1024);
    } else {
        dump_vmem_dma(2, dma_body, 1024);
    }

    switch(ud_state) {
        case UD_NOT_ACTIVE:
            // only pay for vmem->imem access cost one time, at the start
            // however the *cpu_header should be valid for every frame
            last_timeslot = ud_lifetime = header->seq;  // timeslot
            // last_epoc     = ud_epoc = header->seq2;     // epoc
            ud_chunk_length = this_chunk_length; // ASSUME ALL CHUNKS are same length...
            ud_total_chunks = total_chunks;
            userdata_needs_latency_report = 1;

            ud_state = UD_BUFFER;

            // ring_block_send_eth(0xdead0009);
            break;
        case UD_BUFFER:
            if( fill_level >= USER_DATA_PAUSE_AFTER ) {
                ud_state = UD_WAIT;
                fb_pause_mapmov();
            }
            break;
        case UD_WAIT:
            // probably should not be getting interrupts in the wait state...
            MY_ASSERT(0);
            break;

        case UD_SENDING_TAIL:
        case UD_SENDING:
            // buffer is fill, do not queue any more
            if(fill_level >= USER_DATA_PAUSE_AFTER) {
                fb_pause_mapmov();
            }

            if( chunk_index == (total_chunks-1) ) {
                // this was the last chunk we had
                // due to accounting issues. or not having global pointers that can overlap
                // we should just pause.  we may need to remove this if we want back to back timeslot streaming
                // the issue is that if we get a NEW header from the driver, but we are still dma-ing out the tail
                // of the old frame, we will corrupt or confuse global state.  Instead we just pause if we are on the last one
                // and wait for user_data_finished() to unpause us
                fb_pause_mapmov();
                ud_state = UD_SENDING_TAIL;

                // we set the state to _TAIL so that the other DMA will NOT unpause us like normal

            }
            break;
        case UD_DUMPING:

            // if and only if we are dumping, dump everything in the buffer so far
            // (including what we just now put in)
            // when we dump, we also free
            fill_level = circular_buf2_occupancy(full_dma_pointers);
            while(fill_level > 0) {
                circular_buf2_get(full_dma_pointers, &insta_free);
                fb_release_userdata_dma_pointer(insta_free);
                fill_level = circular_buf2_occupancy(full_dma_pointers);
            }

            // if we were dumping, and this is the last chunk, go back to normal
            if( chunk_index == (total_chunks-1) ) {
                user_data_finished();
            }

            break;
        default:
            // illegal state
            MY_ASSERT(0);
            break;
    }

    if(insert_error != 0) {
        fb_queue_ring_eth(TX_USERDATA_ERROR | 8);
    }

    if( chunk_index == 0 ) {
//         ud_timeslot
//         ud_epoc
        _printf("UD Scheduled for epoc: %d frame: %d\n", ud_epoc, ud_lifetime);
    } else {
        // _printf("%s", "ud\n");
    }




    // unsigned int* cpu_body = REVERSE_VMEM_DMA_ADDRESS(dma_body);

    // SET_REG(x3, 0xdeadbeff);
    // SET_REG(x3, chunk_index);
    // SET_REG(x3, total_chunks);
    // for(unsigned int i = 0; i < this_chunk_length; i++) {
    //     SET_REG(x4, cpu_body[i]);
    // }

    // if(user_callbacks == 1) {
    //     CSR_READ(TIMER_VALUE, start_pause);
    //     start_pause += 1000 + (simple_random()%7000);
    //     fb_pause_mapmov();
    //     SET_REG(x3, 0xaa000000);
    // }

    // release_count++;
    // release_pointer = dma_body;
    user_callbacks++;
}


void sfo_timer_callback(unsigned int data) {
    incoming_future_lifetime_counter = data;

    ring_block_send_eth(0x07000000|data);
    ring_block_send_eth(0x07000000|lifetime_frame_counter);
    
    if(incoming_future_lifetime_counter>0)
    {
        if(run_sfo==false)
        {
            //FIXME: remove +2 or re-calculate this
            incoming_future_lifetime_counter = incoming_future_lifetime_counter+0;
        }
    }
}

// Call this first
void sfo_adjustment_callback(unsigned int data) {
    ringbus_sfo_adjustment_temp = data;
}


// Call this second, settings are not applied till this is called
// 0 is disable sfo adjustment
// 1 is delete (1 sample)
// 2 is add (1 sample)
void sfo_sign_callback(unsigned int data) {
    
    // pick a frame value in the future
    // and send to cs20
    
    if (data == 1 || data == 2 || data == 0)
    {       
        pending_packet_num_SFO_adjustment_period = ringbus_sfo_adjustment_temp;
        pending_packet_SFO_adjustment_direction = data;
        
        pending_packet_SFO_adjustment_nco_freq=(unsigned int)(4194304.0f/(pending_packet_num_SFO_adjustment_period*1.0f));

        pending_lifetime_update = 1;

    } 

    if(data == 0)
    {
        // pending_lifetime_update = 0;
        // packet_counter_SFO_adjustment = 0;
        // run_sfo = false;
    }
    
    
    // SET_REG(x4, 0xdead0020);
    // SET_REG(x4, incoming_future_lifetime_counter);
    // SET_REG(x4, pending_packet_num_SFO_adjustment_period);
    // SET_REG(x4, pending_packet_SFO_adjustment_direction);


}

static unsigned int frame_counter = 0;

static VMEM_SECTION unsigned int work_area[FFT_SIZE*PONG_BUFFERS];


void sfo_phase_correction(unsigned int input_low, unsigned int sfo_counter, unsigned int sfo_nco_freq, unsigned int sfo_direction)
{


    unsigned int nco_delta = sfo_counter*sfo_nco_freq;
    unsigned int nco_angle = 0;
    make_nco(VMEM_DMA_ADDRESS(nco_data), 513, nco_angle, nco_delta);   // with sfo phase correction at tx
    //make_nco(VMEM_DMA_ADDRESS(nco_data), 513, 0, 0);   //without sfo phase correction at tx

    unsigned int occupancy;
    // while(1) {
    //     CSR_READ(DMA_2_SCHEDULE_OCCUPANCY, occupancy);
    //     if(occupancy == 0) {
    //         break;
    //     }
    // }

    nco_angle = 0x100000000-((511*nco_delta)&0xffffffff);
    // ring_block_send_eth(nco_angle);

    make_nco(VMEM_DMA_ADDRESS(nco_data)+513, 511, nco_angle, nco_delta); // with sfo phase correction at tx
    //make_nco(VMEM_DMA_ADDRESS(nco_data)+513, 511, 0, 0);  // without sfo phase correction at tx

    while(1) {
        CSR_READ(DMA_2_SCHEDULE_OCCUPANCY, occupancy);
        if(occupancy < 3) {
            break;
        }
    }


    ///
    /// This does an in-place multiply over the work_area pointer (as passed in as input_low)
    /// the 2st,3nd argument is input, the 4rd argument is output, which is same as input
    /// this is ok somehow

   if(sfo_direction == 2)
    {
        xbb_conj_multi(VMEM_ADDRESS(config_word_conj_eq_0f), input_low, VMEM_ADDRESS(nco_data), input_low);

    }
    else if (sfo_direction == 1)
    {
        xbb_conj_multi(VMEM_ADDRESS(config_word_cmul_eq_0f), input_low, VMEM_ADDRESS(nco_data), input_low);
    }
}


void mask_subcarriers(unsigned int dma_ptr) {
    unsigned mask;
    for(unsigned i = 0; i < mask_sc_enabled; i++) {
        mask = mask_sc[i];
        vector_memory[dma_ptr + mask] = 0;
    }
}


// this is run right before sfo_eq
void handle_cs10_cs20_sfo_sync(void) {
    // if we got a message from cs10, we can apply that here
    //
    // THIS IS NOT THE LIFETIME_32 COUNTER
    //
    if((lifetime_frame_counter == (incoming_future_lifetime_counter))&&(pending_lifetime_update==1))
    {
        pending_lifetime_update = 0;
        // packet_counter_SFO_adjustment = 0;

        // assign true in mode 1/2, false in mode 0
        run_sfo = (    pending_packet_SFO_adjustment_direction == 1
                    || pending_packet_SFO_adjustment_direction == 2 );


        packet_num_SFO_adjustment_period = pending_packet_num_SFO_adjustment_period;
        packet_SFO_adjustment_direction = pending_packet_SFO_adjustment_direction;

        packet_SFO_adjustment_nco_freq = pending_packet_SFO_adjustment_nco_freq;

        if(pending_packet_SFO_adjustment_direction == 0 ) {
            // also reset the counter
            packet_counter_SFO_adjustment = 0;
        }

        // SET_REG(x3, 0xdeaddead);
    }
}


/// checks on userdata timer and acts
/// sets us to the next stage of the FSM
/// makes associated calls to parse library when needed
/// can start sending
/// can dump
static inline void __attribute__((always_inline)) maybe_enter_ud(void
    // const uint32_t progress, 
    // const uint32_t timeslot,
    // const uint32_t can_tx,
    // const uint32_t epoc,
    // const uint32_t accumulated_progress
    ) {
    // (void)progress;
    // (void)timeslot;

    switch(ud_state) {
            // These first 2 states
            // this means we came in JUST EARLY ENOUGH
            // this should be a warning at least
            // possibly error condition

            // ring_block_send_eth(TX_USERDATA_ERROR | 1); // warning, technically might still be ok to run
            // NO BREAK
        case UD_BUFFER:
        case UD_WAIT:
            /// if requested epoc second is in the past
            if( likely(ud_lifetime < lifetime_32) ) {
                ud_state = UD_DUMPING;
                fb_queue_ring_eth(TX_USERDATA_ERROR | 2);
                _puts("Packet was scheduled in past");
            } else if( unlikely(ud_lifetime == lifetime_32) ) {
                // it is currently the exactly correct lifetime_32 frame
                
                // go time
                ud_state = UD_SENDING;
                fb_unpause_mapmov(); // we were either paused, or came in with super low buffer, so unpause
                // unpausing when already unpaused has no effect 
                // ring_block_send_eth(0xdead000a);
                _puts("Userdata Matched!!");

            } else if( unlikely((ud_lifetime - lifetime_32) > (EARLY_UD_FRAMES_ERROR)) ) {
                // if we are way too early, we get here
                // dump and error
                ud_state = UD_DUMPING;
                fb_queue_ring_eth(TX_USERDATA_ERROR | 3);
                _puts("Packet was scheduled more than 4s in the future");
            } else {
                // do nothing, epoc is in the future
                // SET_REG(x3, 0x00003001);
                // SET_REG(x3, ud_epoc);
                // SET_REG(x3, epoc);
                // SET_REG(x3, ud_frame);
                // SET_REG(x3, timeslot);
                // SET_REG(x3, progress);
            }
            break;

        case UD_DUMPING:
        case UD_SENDING:
        case UD_SENDING_TAIL:
        case UD_NOT_ACTIVE:
            // SET_REG(x3, 0x00003002);
            // SET_REG(x3, ud_epoc);
            // SET_REG(x3, epoc);
            // SET_REG(x3, ud_frame);
            // SET_REG(x3, timeslot);
            // SET_REG(x3, progress);
            break;
        default:
            break;
    }
                // SET_REG(x3, 0x00003000);
                // SET_REG(x3, ud_epoc);
                // SET_REG(x3, epoc);
                // SET_REG(x3, ud_frame);
                // SET_REG(x3, timeslot);
                // SET_REG(x3, progress);
}



// phase of previous partial pilot
static unsigned partial_pilot_p[2];

/// @param index - which buffer index should we work with
/// @param phase - what is the phase of the partial pilots
/// @return - row for the index provided with written data
///
/// We need to write a subset of pilots every frame (aka partial)
/// There are 10 different versions, all 64 + 1 subcarriers wide
/// Since the memory persists, we need to wipe the previous phase
/// of the revolve
/// We need to keep two previous phase's as we have two buffers
static inline uint32_t* __attribute__((always_inline)) prep_partial_pilot(const unsigned index, const unsigned phase) {

    unsigned int* pilot_cpu = partial_pilot_0;

    if(index != 0) {
        pilot_cpu = partial_pilot_1;
    }

    const unsigned int pilot_row = VMEM_ROW_ADDRESS(pilot_cpu);


    // write zeros first
    // use the previous phase to efficiently write zeros only where there were ones
    const unsigned prev_phase = partial_pilot_p[index];
    const unsigned prev_row = duplex_partial_pilot[(prev_phase*2)];
    const unsigned prev_len = duplex_partial_pilot[(prev_phase*2)+1];
    vmem_copy_rows(vmem_zeros_row, pilot_row+prev_row, prev_len);

    // inefficient, just wipe all zeros
    // vmem_copy_rows(vmem_zeros_row, pilot_row, 64);

    // write ones
    const unsigned ones_row = duplex_partial_pilot[(phase*2)];
    const unsigned ones_len = duplex_partial_pilot[(phase*2)+1];
    vmem_copy_rows(vmem_all_pilots_row+ones_row, pilot_row+ones_row, ones_len);

    // write "tone 2"
    pilot_cpu[1022] = tone_2_value;

    // update previous phase for next run
    partial_pilot_p[index] = phase;

    return (uint32_t*)pilot_cpu;
}

static inline bool __attribute__((always_inline)) send_cooked (
    const uint32_t frame_phase,
    uint32_t**  input_base,
    uint32_t**  output_base,
    uint32_t* input_dma_offset,
    unsigned* output_dma_offset
    ) {

    // which phase of the canned output are we on?
    // this resets to 0 at the start of a timeslot
    // input_dma_offset = (input_frame_phase*1024);
    *input_dma_offset  = (frame_phase*FFT_SIZE);
    *output_dma_offset = (frame_phase*FFT_SIZE);
    //   output_dma_offset = input_dma_offset = (frame_phase*1024);
    // output_offset_row = input_offset_row = input_dma_offset/16; 
    // these are cpu pointers to base of a 5 buffer long array
    *input_base  = (uint32_t*)vmem_counter;
    *output_base = (uint32_t*)work_area;

    bool sending_zeros = false;

    if( cooked_data_type == 1 ) {
        *input_base = (uint32_t*)vmem_all_pilots_row;
        *input_dma_offset = 0;
    } else if( cooked_data_type == 2 ) {

        bool eq_frame = false;
        bool cooked_data = false;
        int32_t FGHI_frame = -1;
        int32_t partial_pilots_frame = -1;

        switch(duplex_mode) {
            case DUPLEX_SEND_DL_PILOT_0:                   // p
                if( duplex.role == DUPLEX_ROLE_TX_0 ) {
                    eq_frame = true;
                }
                break;
            case DUPLEX_SEND_DL_PILOT_1:                   // q
                if( duplex.role == DUPLEX_ROLE_TX_1 ) {
                    eq_frame = true;
                }
                break;
            case DUPLEX_SEND_DL_PILOT_2:                   // r
                if( duplex.role == DUPLEX_ROLE_TX_2 ) {
                    eq_frame = true;
                }
                break;
            case DUPLEX_SEND_DL_PILOT_3:                   // s
                if( duplex.role == DUPLEX_ROLE_TX_3 ) {
                    eq_frame = true;
                }
                break;
            case DUPLEX_SEND_DL_DATA:                      // d
                cooked_data = true;
                break;
            case DUPLEX_SEND_UL_PILOT:                     // P
                partial_pilots_frame = duplex_ul_pilot_phase(duplex_progress);
                break;
            case DUPLEX_SEND_UL_FINESYNC:                  // S
                eq_frame = true;
                break;
            case DUPLEX_SEND_UL_DATA:                      // D
                cooked_data = true;
                break;
            case DUPLEX_SEND_DL_BEAM_PILOT:                // i
                eq_frame = true;
                break;
            case DUPLEX_SEND_UL_FEEDBACK_0:                // F
            case DUPLEX_SEND_UL_FEEDBACK_1:                // G
            case DUPLEX_SEND_UL_FEEDBACK_2:                // H
            case DUPLEX_SEND_UL_FEEDBACK_3:                // I
                FGHI_frame = duplex_ul_feedback_phase(duplex_progress);
                break;

            default:
            case DUPLEX_SEND_ZERO:
                eq_frame = false;
                break;
        }

        if( cooked_data ) {
            // we already setup the original cooked data pointers at the top
            // of this function.  so we just exit
            return false; // we are sending data, not zeros
        }

        if( FGHI_frame != -1 ) {
            // we are in FGHI
            *input_base = (uint32_t*)pilot_frame_cpu[FGHI_frame];

            // During FF of receiver, we make sure to have our original residue output
            (*input_base)[1022] = 0x2000;

        } else if( partial_pilots_frame != -1 ) {
            // We are in P

            if( likely(enable_composite_ul_pilot) ) {
                const unsigned index = (unsigned)partial_pilots_frame;
                const unsigned phase = duplex.lt_phase;
                *input_base = prep_partial_pilot(index, phase);
            } else {
                *input_base = (uint32_t*)vmem_all_pilots;
            }
            // SET_REG(x3, 0xf0f0);
            // SET_REG(x3, 0x0);
            // SET_REG(x3, index);
            // SET_REG(x3, 0x0);
            // SET_REG(x3, phase);
            // SET_REG(x3, 0x0);


        } else if( eq_frame ) {
            *input_base = (uint32_t*)vmem_all_pilots;
        } else {
            *input_base = (uint32_t*)vmem_zeros;
            sending_zeros = true;
        }
        
        
        *input_dma_offset = 0;

    } else if( cooked_data_type >= 3 ) {
        // bool we_are_rx = cooked_data_type == 2;

        bool eq_frame = false;

        switch(cooked_data_type) {
            case 3:
                if(duplex_progress < 4) {
                    eq_frame = true;
                }
                break;
            case 4:
                if((duplex_progress < 4) && (duplex_progress & 0x1) )  {
                    eq_frame = true;
                }
                break;
            case 5:
                if( duplex_progress == 0)  {
                    eq_frame = true;
                }
                break;
            case 6:
                if(duplex_progress >= 8 && duplex_progress < 16) {
                    eq_frame = true;
                }
                break;
            case 7:
                if(duplex_progress >= 4 && duplex_progress < 8) {
                    eq_frame = true;
                }
                break;
            case 8:
                if((duplex_progress < 4) && ((duplex_progress & 0x1)==0) )  {
                    eq_frame = true;
                }
                break;
            case 9:
                if(duplex_progress < 8) {
                    eq_frame = true;
                }
                break;
            case 10:
                if( duplex_progress == 1)  {
                    eq_frame = true;
                }
                break;
            case 11:
                if( duplex_progress == 2)  {
                    eq_frame = true;
                }
                break;
        }

        // if( cooked_data_type == 2 ) {

        // }

        


        if( eq_frame ) {
            *input_base = (uint32_t*)vmem_all_pilots;
        } else {
            *input_base = (uint32_t*)vmem_zeros;
            sending_zeros = true;
        }
        
        *input_dma_offset = 0;
    } // cooked_data_type over 3

    return sending_zeros;
}

// goal is to report how many frames using a fucked up format
// format is userdata - now
// static inline void __attribute__((always_inline)) report_userdata_latency(
//     const uint32_t epoc,
//     const uint32_t accumulated_progress,
//     const uint32_t ud_epoc,
//     const uint32_t ud_frame
//     ) {
//     userdata_needs_latency_report = 0;
//     int8_t e_delta = (int8_t)(ud_epoc - epoc);
//     int16_t frame_delta = (int16_t)((ud_frame) - accumulated_progress);
//     uint32_t masked = ((e_delta<<16)&0xff0000) | (frame_delta&0xffff);
//     fb_queue_ring_eth(TX_FILL_LEVEL_PCCMD | masked );
// }

// report how many frames delta the userdata came in on, new format, new rb
static inline void __attribute__((always_inline)) report_userdata_latency_duplex(void) {
    userdata_needs_latency_report = 0; // clear flag

    const uint32_t rb = schedule_report_delta_ringbus(ud_lifetime, lifetime_32);
    fb_queue_ring_eth(TX_UD_LATENCY_PCCMD | rb );
}



OOKMessage ook_message;
bool ook_running = false;



static inline void __attribute__((always_inline)) handle_ook(
    const uint32_t input_dma_start,
    const uint32_t accumulated_progress
    ) {
    ///
    /// Mask our OOK and all peer OOK subcarriers to ZERO
    /// this allows for OFDMA
    mask_subcarriers(input_dma_start);

    bool ook_done = false;

    // default for value we will be overwriting for the ook subcarrier
    uint32_t write = 0;

    // do not combine this with the following if
    if( !ook_running ) {
        // send this message every 1/4 of a second
        switch(accumulated_progress) {
            case (0 * SCHEDULE_FRAMES)/4:
            case (1 * SCHEDULE_FRAMES)/4:
            case (2 * SCHEDULE_FRAMES)/4:
            case (3 * SCHEDULE_FRAMES)/4:
                ook_prep_outbound_gain(&ook_message, ook_symbol_value);
                ook_message.data[0] = lifetime_32;
                ook_message.data[1] = FRAME_COUNTER_OOK;
                ook_running = true;
                break;

            default:
                break;
        }
    }

    if( ook_running ) {
        ook_modulate_next(&ook_message, &write, &ook_done);
        if( ook_done ) {
            ook_running = false;
        }
    }

    // actually does overwriting of subcarrier
    vector_memory[input_dma_start + ook_subcarrier] = write;
}



// Instead of ping/pong
// this operates with 5 input buffers, 5 output buffers
// each input is processed into it's matching output as fast as possible
// output is done with a _block_send() so we will never have more than 4 buffers
// out at once. this guarentees we never write to a buffer while output DMA is reading
// uint32_t pending_frame = 0;
uint32_t vmem_raw_dma_ptr;
void pet_eq_and_dma_out(void) {

    uint32_t input_dma_offset = 0;
    unsigned output_dma_offset;
    uint32_t input_offset_row;
    uint32_t output_offset_row;
    
    // uint32_t pilot_tone_output_row;
    uint32_t* input_base;
    uint32_t* output_base;
    // uint32_t* pilot_tone_output_base = pilot_tone_output;
    // choose which of our 5 frames to output,
    uint32_t timeslot, can_tx, epoc, accumulated_progress;

    // schedule, an object which this function modifies
    // lifetime_32 global counter owned by main.c
    // progress how may samples into the timeslot are we
    // accumulated_progress how many samples into the second are we
    // can_tx are we allowed to tx based on the schedule
    
    {
        uint32_t burn;
        // this is expensive to run, we can save output and all but progress would be able to be cached
        schedule_get_timeslot2(schedule,
                        lifetime_32,
                        &burn, // was progress
                        &accumulated_progress,
                        &timeslot,
                        &epoc,
                        &can_tx);
    }

    duplex_progress = lifetime_32 % DUPLEX_FRAMES;

    SET_REG(x3, 0x00003004);
    SET_REG(x3, duplex_progress);
    SET_REG(x3, 0x0);

    // this might glitch when entering / exiting a timeslot
    //   or when adjusting lifetime_32
    // due to output buffers not being rotated correctly
    // this number for now is still critical to the functioning of this block
    // we can be lazy about certain things because of our 5 buffer style.
    // that being said we have an expentive mod we should get rid of here
    uint32_t frame_phase = (accumulated_progress % PONG_BUFFERS);

    if(userdata_needs_latency_report) {
        // if flag, flag is reset inside function
        report_userdata_latency_duplex(); 
    }

#ifdef ENABLE_TB_DEBUG
    const char* as_string = state_string[ud_state];
    _printf("%s%d%s%d%s%d%s%s\n", "Epoc: ", epoc,  " Frame: ", accumulated_progress, " Timeslot: ", timeslot, " State: ", as_string);
#endif

    // _printf("Frame: %d state %d\n", accumulated_progress, ud_state);
    
    // unsigned ta;
    // unsigned tb;
    // CSR_READ(TIMER_VALUE, ta);
    // _printf("foo\n");
    // _printf("bar\n");
    // _printf("baz\n");
    // _printf("boop\n");
    // _printf("boop\n");
    // stall2(1000);
    // CSR_READ(TIMER_VALUE, tb);
    // _printf("stall took %d\n", (tb-ta));

    // updates userdata fsm
    // may pause/unpause feedback_bus_parse
    maybe_enter_ud();

    unsigned int do_free = 0; // INITIALIZE THIS
    unsigned int dma_pointer;

    // flag weather we underflowed our userdata buffer?
    unsigned int did_underflow_this_time = 0;

    duplex_mode = get_duplex_mode(&duplex, duplex_progress, lifetime_32);

    bool sending_zeros = false;

    // are we in D or d ?
    // pass false as this is TX Chain
    const bool ud_allowed = duplex_do_userdata(&duplex, duplex_progress, false);

    if(
           ud_allowed
        && (ud_state == UD_SENDING || ud_state == UD_SENDING_TAIL)
       ) {

        // unsigned int ud_will_consume = ud_chunk_index_consumed;
        int error = circular_buf2_get(full_dma_pointers, &dma_pointer);

        if( error != 0 ) {
            // we were in sending state but we UNDERFLOWED
            _puts("\nUnderflowed!!");

            fb_queue_ring_eth(
                TX_USERDATA_ERROR | 6 |
                ((ud_chunk_index_consumed << 8) & 0xff00)
                );

            did_underflow_this_time = 1;

            ///
            /// we were in a sending state
            /// but we are underflowed mid-way, so we need to switch to dumping state
            /// when we are in dumping, we will not get any new values in full_dma_pointers
            /// because the handle_userdata function will take it for us


            // go to the dumping state
            ud_state = UD_DUMPING;
            // unpause do drain input, even though, if we got to this state
            // the input has nothing
            fb_unpause_mapmov();

            // copied from below
            // these are the lines for the zero buffer

            output_dma_offset = (0*FFT_SIZE);
            output_offset_row = input_dma_offset/16;

            // zero buffer is only 1 long not 5, so always start at the beginning
            input_dma_offset = 0;
            input_offset_row = 0;

            input_base = (uint32_t*)vmem_zeros;
            output_base = (uint32_t*)work_area;


        } else {
            // free memory back to feedback bus later on
            do_free = 1;
        }

        output_dma_offset = (frame_phase*FFT_SIZE);
        input_dma_offset = 0;
        output_offset_row = output_dma_offset/16;
        input_offset_row = input_dma_offset/16;

        // annoying, but all code below expects that input_base is a cpu pointer
        input_base = (uint32_t*)REVERSE_VMEM_DMA_ADDRESS(dma_pointer);
        output_base = (uint32_t*)work_area;

        if(!did_underflow_this_time) {

            ud_chunk_index_consumed++; // post increment
            // but only if we didn't underflow

        }

    } else {
        // can tx, and not user data
        sending_zeros = send_cooked(
            frame_phase,
            &input_base,
            &output_base,
            &input_dma_offset,
            &output_dma_offset
            );

        input_offset_row  = VMEM_DMA_ADDRESS_TO_ROW(input_dma_offset);
        output_offset_row = VMEM_DMA_ADDRESS_TO_ROW(output_dma_offset);
    }

///////////////////////////////////////////////////////////////////////////////
///
/// Above this this line we choose what to send
/// Below this line we deal with eq and sfo only
///

    if( 0 ) {
        // code to send out counter
        input_base = (uint32_t*)vmem_counter_1k;
        input_dma_offset = 0;
        input_offset_row = 0;
    }

    // calculate a bunch row counters etc
    // calculate start dma/row for input
    const uint32_t input_dma_start = VMEM_DMA_ADDRESS(input_base) + input_dma_offset;
    const uint32_t input_start_row = VMEM_ROW_ADDRESS(input_base) + input_offset_row;


#ifdef asdff
    // Code below will modify the original cooked memory
    // so to prevent this we make ANOTHER copy, just so it can modify
    // that version and leave the original untouched
    vmem_copy_rows(input_start_row, VMEM_ROW_ADDRESS(input_copy_modify), 64);

    // now we set these input dma/row pointers to our copy_modify buffer
    input_dma_start = VMEM_DMA_ADDRESS(input_copy_modify);
    input_start_row = VMEM_ROW_ADDRESS(input_copy_modify);
#endif

    // input_copy_modify

    // SET_REG(x31, 0xABCDABCD);
    // SET_REG(x31, input_dma_start);
    // SET_REG(x31, input_start_row);
    // SET_REG(x31, mapper_count);
    // SET_REG(x31, pending_frame);

    // calculate start dma/row for output
    const uint32_t work_area_dma = VMEM_DMA_ADDRESS(output_base) + output_dma_offset;
    const uint32_t work_area_row = VMEM_ROW_ADDRESS(output_base) + output_offset_row;

    // SET_REG(x3, input_dma_start);
    // SET_REG(x3, input_start_row);
    // SET_REG(x3, work_area_dma);
    // SET_REG(x3, work_area_row);

    // never changes
    const uint32_t eq_row_corrected = VMEM_ROW_ADDRESS(eq_vector_corrected);
    const uint32_t eq_row           = VMEM_ROW_ADDRESS(eq_vector);


    // hanle OOK if we transmitting
    if( !sending_zeros && (cooked_data_type == 0) ) {
        handle_ook(input_dma_start, accumulated_progress);
    }

    if( 1 ) {
        for(unsigned i = 0; i < stall_check; i++) {
            STALL(5);
        }
    }

    // pilot_tone_output_row = VMEM_ROW_ADDRESS(pilot_tone_output_base);

    {
        unsigned int* input_as_cpu = REVERSE_VMEM_DMA_ADDRESS(input_dma_start);
        SET_REG(x3, 0xfeedfeed);
        SET_REG(x3, input_as_cpu[2]);
    }



    // multiply by eq
    // if we are in beamform data
    if( 
           (duplex_mode == DUPLEX_SEND_DL_DATA || duplex_mode == DUPLEX_SEND_DL_BEAM_PILOT )
        && (apply_eq_correction >= 1 ) 
        ) {
        vector_multiply_1024(0, input_start_row, eq_row_corrected, work_area_row );
    } else {
        // we are sending anything else, just use random rotation
        vector_multiply_1024(0, input_start_row, eq_row, work_area_row );
    }

    // handles:
    //  incoming_future_lifetime_counter
    //  lifetime_frame_counter
    // sets:
    //  run_sfo
    // these are just counters, no vector work
    handle_cs10_cs20_sfo_sync();

    // if we are not in sfo mode 0
    if(run_sfo) {
        // Pre-rotate each subcarrier a slightly different amount to cancel the effect of SFO
        sfo_phase_correction(work_area_row, packet_counter_SFO_adjustment, packet_SFO_adjustment_nco_freq, packet_SFO_adjustment_direction);
        
        packet_counter_SFO_adjustment++;
        
        if(packet_counter_SFO_adjustment >= packet_num_SFO_adjustment_period) {
            packet_counter_SFO_adjustment = 0;
        }
    }

    // for(unsigned i = 0; i < 1024; i++) {
    //     vector_memory[work_area_dma + i] = 0xdd000000 | i;
    // }
    // force tone 0 to be progress
    
    // vector_memory[work_area_dma + 16] = 0xff000000 | lifetime_32;

    // force tone 1 to be long term phase
    // vector_memory[work_area_dma + 17] = 0xfe000000 | duplex.lt_phase;



    const unsigned int trunk_start = VMEM_DMA_ADDRESS(trunk) + (TRUNK_LENGTH * frame_phase);

    vector_memory[trunk_start + TRUNK_FRAME_COUNTER] = lifetime_32;

    // dump_vmem_dma(0, work_area_dma, 1024);

    // before we used a limit of 4 for a 5-pong system
    // now we use a limit of 8 for a 5 pong system because we have 2 output occupancy per run
    dma_block_send_limit(work_area_dma, FFT_SIZE,     ((PONG_BUFFERS-1)*2));
    dma_block_send_limit(trunk_start,   TRUNK_LENGTH, ((PONG_BUFFERS-1)*2));

    {
        unsigned int occupancy;
        while(1) {
            CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
            if( occupancy <= 1) {
               break;
            }
        }
    }


    // technically we could free this earlier, (After eq runs)
    // but this will not take effect until this function exits
    // so we do it here so that we can start the above dma ASAP
    if(do_free) {
        fb_release_userdata_dma_pointer(dma_pointer);
        // if we did work, we can unpause, because we will guarenteed have at least 1 spot
        // to put a buffer in.

        // we unpause after every frame we send
        // UNLESS the state is in UD_SENDING_TAIL, this means we are not allowed to unpause
        // from this function
        if( ud_state == UD_SENDING ) {
            fb_unpause_mapmov();
        }
    }

    
    // _perf_work();


    if(
           (ud_state == UD_SENDING || ud_state == UD_SENDING_TAIL)
        && ud_chunk_index_consumed == ud_total_chunks
         ) {
        user_data_finished();
        SET_REG(x3, 0x00003003);
        // ring_block_send_eth(0xdead000b);
    }


    if(epoc_needs_latch) {
        epoc_needs_latch = 0;
        epoc_was_latched = 4;
        epoc_latch_frame = accumulated_progress;
        epoc_latch_second = epoc;
    }

    // fires 24 times per second
    if( (accumulated_progress & (1024-1)) == 0 ) {

        if( progress_report_remaining > 0 ) {
            if( accumulated_progress == 0 ) {
                ring_block_send_eth(TX_EPOC_REPORT_PCCMD | (epoc&0xffffff) );
            } else {
                ring_block_send_eth(TX_PROGRESS_REPORT_PCCMD | (accumulated_progress) );
            }
            progress_report_remaining--;
        }
    }



    // apply_epoc_update(schedule, accumulated_progress);

    if( duplex_progress == (DUPLEX_FRAMES-1)) {
        for(unsigned i = 0; i < duplex_stall; i++) {
            STALL(5);
        }
    }


    lifetime_frame_counter = (lifetime_frame_counter+1)&0xffffff;
    lifetime_32++; // runs at same rate as frame_counter but overflows less frequently

    frame_counter++;
    if(frame_counter == PONG_BUFFERS) {
        frame_counter = 0;
    }
}


// epoc_latch_second
// epoc_latch_frame

// returns if more data to be sent
unsigned int pet_epoc_readback2(void) {

    unsigned int base;

    switch(epoc_was_latched) {
        case 4:
            // lower
            base = (epoc_latch_frame & 0xffff);
            break;
        case 3: // upper
            base = ( (epoc_latch_frame>>16) & 0xffff);
            break;
        case 2:
            base = (epoc_latch_second & 0xffff);
            break;
        case 1:
            base = ( (epoc_latch_second>>16) & 0xffff);
            break;
        case 0:
        default:
            epoc_was_latched = 0;
            return 0;
            break;
    }
    // epoc_was_latched

    unsigned int dmode = epoc_was_latched << 16;
    // SET_REG(x3, 0xdeaddddd);
    // SET_REG(x3, epoc_was_latched);
    // SET_REG(x3, dmode);
    // SET_REG(x3, base);
    // SET_REG(x3, EPOC_REPLY_PCCMD | dmode | base);

    unsigned int occupancy;

    CSR_READ(RINGBUS_SCHEDULE_OCCUPANCY, occupancy);
    if(occupancy < RINGBUS_SCHEDULE_DEPTH)
    {
        ring_block_send_eth(EPOC_REPLY_PCCMD | dmode | base);
        epoc_was_latched--;
        return 1;
    } else {
        return 0; // do nothing, we will send it next time
    }
}

// runs above function untill all ringbus have been sent
void pet_epoc_readback(void) {
    while(pet_epoc_readback2()) {
    }
}

uint32_t times_analog_eq = 0;
uint32_t times_eq_corrected = 0;


// header and body are cpu pointers

uint32_t previous_eq_correction_counter = 0;
uint32_t report_timer_delta = 0;

void fb_vector_type_callback(unsigned int *header, unsigned int *body, unsigned int vector_length, unsigned int vtype) {
    (void)vector_length;
    const unsigned eq_row = VMEM_ROW_ADDRESS(eq_vector);
    const unsigned eq_row_corrected = VMEM_ROW_ADDRESS(eq_vector_corrected);
    const unsigned input_row = VMEM_ROW_ADDRESS(body);
    const feedback_frame_vector_t* as_vec = (feedback_frame_vector_t*)header;
    bool report = true;

    switch(vtype) {
        case FEEDBACK_VEC_TX_EQ:
            // copy twice
            vmem_copy_rows(input_row,        eq_row, 1024/16);
            vmem_copy_rows(input_row,        eq_row_corrected, 1024/16);
            // ring_block_send_eth(0xdeadfeed);
#ifdef READBACK_EQ_HASH
            eq_need_hash = 1;
#endif
            break;
        case FEEDBACK_VEC_STATUS_REPLY:
            break; // prevent default
        case FEEDBACK_VEC_EQ_ANALOG: {
                // we are RX. tx has sent us his "p" frame
                // we gain and load this into t0_pilot
                // so we can transmit it back in our "FF" section

                // uint32_t delta = 4;
                const uint32_t rx_seq = as_vec->seq;
                // const uint32_t got_progress = rx_seq % DUPLEX_FRAMES;
                const int32_t delta = lifetime_32 - rx_seq;

                if( report_timer_delta == 1 ) {
                    fb_queue_ring_eth(FEEDBACK_EQ_STATUS_PCCMD | (lifetime_32&0xffffff));
                    fb_queue_ring_eth(FEEDBACK_EQ_STATUS_PCCMD | (delta&0xffffff));
                    report_timer_delta = 0;
                }

                // based on the rx chain frame counter with cs20 received the frame
                // we save to our own row
                // we cap at 4, in the case of some incorrect behavior, it will just wrap
                uint32_t destination_row = pilot_frame_row[rx_seq%4];

                times_analog_eq++;

                // if( got_progress == 0 ) {
                //     destination_row = VMEM_ROW_ADDRESS(t0_pilot_1);
                // }

                // feedback_gain_vector is all 0x7fff
                // we multiply by this feedback_gain_vector, and then add a gain with bs
                // so that we can gain up the feedback

                if( true ) {
                    cfg_vector_multiply_1024(
                        VMEM_ROW_ADDRESS(variable_analog_eq_mul),
                        VMEM_ROW_ADDRESS(feedback_gain_vector),          // input
                        input_row,       // input
                        destination_row  // output
                    );
                } else {
                    vmem_copy_rows(input_row, destination_row, 1024/16);
                }
                report = false;
            }   
            break;
        case FEEDBACK_VEC_EQ_CORRECTION: {
                const uint32_t rx_seq = as_vec->seq;
                const int32_t delta = lifetime_32 - rx_seq;

                if( report_timer_delta == 2 ) {
                    fb_queue_ring_eth(FEEDBACK_EQ_STATUS_PCCMD | (lifetime_32&0xffffff));
                    fb_queue_ring_eth(FEEDBACK_EQ_STATUS_PCCMD | (delta&0xffffff));
                    report_timer_delta = 0;
                }

                // only grab the first correction by looking at the value of the previous
                // one and finding a large gap
                if(
                       (apply_eq_correction > 1)
                    && ((rx_seq - previous_eq_correction_counter) > (DUPLEX_FRAMES/2)) ) {

                    cfg_vector_multiply_1024(
                        VMEM_ROW_ADDRESS(variable_eq_correction_mul),
                        eq_row,          // input
                        input_row,       // input
                        eq_row_corrected // output
                    );

                    times_eq_corrected++;
                }

                if( apply_eq_correction > 1 && apply_eq_correction < 10000 ) {
                    apply_eq_correction--;
                }

                previous_eq_correction_counter = rx_seq;
                report = false;
            }
            break;
        default:
            fb_vector_default();
            break;
    }

    if( report ) {
        // fb_queue_ring_eth(0xdead0001);
        fb_queue_ring_eth(TX_PARSE_GOT_VECTOR_PCCMD | ((header[0]&0xff) << 8) | (header[4]&0xff) );
    }
}


void fb_stream_type_callback(unsigned int *header, unsigned int *body, unsigned int stream_length, unsigned int stype) {
    (void)header;
    (void)body;
    (void)stream_length;
    switch(stype) {
        default:
            fb_stream_default();
            break;
    }
}

void callback_eq_setting(unsigned int data) {
    if( data == 0 ) {
        // reset eq
        vmem_copy_rows(VMEM_ROW_ADDRESS(eq_vector_default), VMEM_ROW_ADDRESS(eq_vector), 1024/16);
        vmem_copy_rows(VMEM_ROW_ADDRESS(eq_vector_default), VMEM_ROW_ADDRESS(eq_vector_corrected), 1024/16);

        vmem_all_pilots[ook_subcarrier] = 0x00002000;
    }
}

// static uint32_t inspect_eq = 0;

static uint32_t debug_generic_op = 0x42;
uint32_t* pointer_for_generic_op(const uint32_t sel) {
    uint32_t *p = 0;
    switch(sel) {
        case 0:
            p = (uint32_t*) &lifetime_32;
            break;
        case 1:
            p = (uint32_t*) &frame_counter;
            break;
        case 3:
            p = (uint32_t*) &schedule->epoc_time;
            break;
        case 4:
            p = (uint32_t*) &ook_subcarrier;
            break;
        case 5:
            p = (uint32_t*) &times_eq_corrected;
            break;
        case 6:
            p = (uint32_t*) &lifetime_frame_counter;
            break;
        case 7:
            p = (uint32_t*) &packet_counter_SFO_adjustment;
            break;
        case 8:
            p = (uint32_t*) &report_timer_delta;
            break;
        case 9:
            p = (uint32_t*) &duplex_stall;
            break;
        case 10:
            p = (uint32_t*) &duplex.role;
            break;
        // case 11:
        //     p = (uint32_t*) &inspect_eq;
        //     break;
        case 12:
            p = (uint32_t*) &enable_composite_ul_pilot;
            break;
        case 13:
            p = (uint32_t*) &apply_eq_correction;
            break;
        case 14:
            p = (uint32_t*) &stall_check;
            break;
        case 15:
            p = (uint32_t*) &debug_generic_op;
            break;
        default:
            break;
            // return; // unknown selector
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



void epoc_was_requested_callback(const unsigned int data) {
    (void)data;
    // unsigned int dmode = (data >> 16) & 0xff;
    // unsigned int data_16 = data & 0xffff;
    epoc_needs_latch = 1;
    
}


// tricky, because we want to leave the lifetime counter alone
// so we bend over backwards to pick a schedule->offset such that it will be back to 0
// void schedule_reset_callback(unsigned int data) {

//     uint32_t now = lifetime_32 + schedule->offset;

//     uint32_t sample = now % SCHEDULE_FRAMES;

//     schedule->offset += SCHEDULE_FRAMES - sample;
// }

void setup_userdata(void) {
    CIRBUF_POW2_RUNTIME_INITIALIZE(__full_dma_pointers);

    ud_state = UD_NOT_ACTIVE;
    user_data_finished(); // a mini reset, call it here why not
}

// tries to do best to reset map/mov/feedback bus code
void eth_cs20_reset_callback(const unsigned int data) {
    (void)data;
    // dump but only if we are in the middle of a mapmov.

    switch(ud_state) {
        case UD_BUFFER:
        case UD_WAIT:
        case UD_SENDING:
        case UD_SENDING_TAIL:
            ud_state = UD_DUMPING;
            break;
        case UD_DUMPING:
        case UD_NOT_ACTIVE:
            // do nothing
            break;
    }
    fb_unpause_mapmov();
}

void cs20_control_progress_report(const unsigned int data) {
    progress_report_remaining = data;
}

void __attribute__((optimize("Os"))) cs20_dump_status(const unsigned int data) {
    (void)data;
    int inc = 0x00000000;

    // legacy, leaving so that handle_cs20_tx will be ok
    const unsigned int pending_data = 0;
    const unsigned int pending_timeslot = 0;
    const unsigned int pending_length = 0;

    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (frame_counter & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (lifetime_frame_counter & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (lifetime_32 & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | ((lifetime_32>>16) & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (pending_data & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (pending_timeslot & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (pending_length & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (output_frame_count & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (epoc_needs_latch & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (epoc_was_latched & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (progress_report_remaining & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (user_callbacks & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (ud_state & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (0 & 0x0000ffff)  ); inc += 0x00010000; // was ud_frame
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (0 & 0x0000ffff)  ); inc += 0x00010000; // was ud_epoc
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (ud_total_chunks & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (ud_chunk_index_consumed & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (ud_chunk_length & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (ud_asked_for_pause & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (pending_lifetime_update & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (incoming_future_lifetime_counter & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (userdata_needs_latency_report & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (run_sfo & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (0 & 0x0000ffff)  ); inc += 0x00010000; // FIXME
    fb_queue_ring_eth( TX_REPORT_STATUS_PCCMD | inc | (0 & 0x0000ffff)  ); inc += 0x00010000; // FIXME
}

#ifdef READBACK_EQ_HASH
void pet_eq_hash(void) {
//     unsigned int eq_need_hash = 0;
// unsigned int eq_hash_state = 0;
    // eq_vector

    bool go = eq_hash_progress != 0;

    if( eq_need_hash ) {
        eq_need_hash = 0;
        eq_hash_state = 1;
        eq_hash_progress = 0;
        go = true;
    }

    if( go ) {

#ifdef READBACK_FULL_EQ
        if( eq_hash_progress != 0 ) {
            if( fb_ring_queue->occupancy > 20 ) {
                return;
            }
        }

        for(int i = 0; i < 16; i++) {
            unsigned int* ptr = eq_vector + eq_hash_progress;
            unsigned int word = ptr[i];
            fb_queue_ring_eth(TX_USERDATA_ERROR | ((word & 0xffff0000)>>8) | 0x16 );
            fb_queue_ring_eth(TX_USERDATA_ERROR | ((word & 0xffff)<<8)     | 0x17 );
        }
#endif

        eq_hash_state = xorshift32(eq_hash_state, eq_vector + eq_hash_progress, 16);

        eq_hash_progress += 16;

        if( eq_hash_progress >= 1024 ) {
            eq_hash_progress = 0;

            fb_queue_ring_eth( FEEDBACK_HASH_PCCMD | 0x00000 | (eq_hash_state & 0x0000ffff)  );
            fb_queue_ring_eth( FEEDBACK_HASH_PCCMD | 0x10000 | (eq_hash_state>>16 & 0x0000ffff)  );
        }



    }

}
#endif

void set_ook_sc_callback(const unsigned int data) {
    if( data < 1024 ) {
        ook_subcarrier = data;
    } else {
        // error, do nothing
    }
}

// Mask subcarriers when tdma is <= 9
// tdma will get set above 9 during user data so this mask will not take effect then
// call with subcarrier in the bottom 16 bits and the array index into next 8 bits
// call with subcarrier value of zero to disable masking
// mask_sc_enabled is set so that only the first non-zero subcarrier indices are used
// technically this scheme disallows masking of subcarrier 0, but this is ok
void __attribute__((optimize("Os"))) set_mask_sc_callback(const unsigned int data) {
    uint32_t dmode = (data >> 16) & 0xff; // 8 bits
    uint32_t data_16 = data & 0xffff; // 16 bits

    if( data_16 >= 1024 ) {
        return; // do nothing
    }

    if( dmode >= MAXIMUM_MASK_SC ) {
        return; // do nothing
    }

    mask_sc[dmode] = data_16;

    // Set the enabled number to 4
    // loop over each entry and check for zero.  first zero value found
    // will set the enabled count to that index
    mask_sc_enabled = MAXIMUM_MASK_SC;
    for(unsigned i = 0; i < MAXIMUM_MASK_SC; i++) {
        if( !mask_sc[i] ) {
            mask_sc_enabled = i;
            break;
        }
    }
}

// void calculate_epoc_callback(const unsigned int data) {
//     if( data ) {
//         pending_epoc_update = 1;
//     } else {
//         pending_epoc_update = 0;
//     }
// }

void check_last_ud_callback(const unsigned int data) {
    (void)data;
    // last_timeslot
    // last_epoc
    // fb_queue_ring_eth( LAST_USERDATA_PCCMD |  (last_epoc&0xffffff) );
    fb_queue_ring_eth( LAST_USERDATA_PCCMD |  (last_timeslot&0xffffff) );
}

void cooked_data_type_callback(const unsigned int data) {
    cooked_data_type = data;
}

void eq_rotation_callback(const unsigned int data) {
    const unsigned eq_row = VMEM_ROW_ADDRESS(eq_vector);
    const unsigned random_row = VMEM_ROW_ADDRESS(eq_random_rotation);
    if( data == 1) {
        vmem_copy_rows(random_row, eq_row, 1024/16);
    }
}

void stall_check_callback(const unsigned int data) {
    stall_check = data;
}

void setup_duplex(void) {
    init_duplex(&duplex, DUPLEX_ROLE_RX);

    pilot_frame_cpu[0] = t0_pilot;
    pilot_frame_cpu[1] = t1_pilot;
    pilot_frame_cpu[2] = t2_pilot;
    pilot_frame_cpu[3] = t3_pilot;

    for(unsigned i = 0; i < 4; i++) {
        pilot_frame_row[i] = VMEM_ROW_ADDRESS(pilot_frame_cpu[i]);
    }
}

void setup_pointers(void) {
    vmem_all_pilots_row = VMEM_ROW_ADDRESS(vmem_all_pilots);
    vmem_zeros_row = VMEM_ROW_ADDRESS(vmem_zeros);
}

void app_barrel_shift_callback(const unsigned int data) {
    // APP_BARREL_SHIFT_CMD
    const unsigned int stage = ((data & 0x00FF0000) >> 16);
    const unsigned int shift = ((data & 0xffff));

    unsigned short* cpu_source;
    unsigned short* cpu_dest;

    // we need to set conj and non conj the same here
    switch(stage) {
        case 0:
            // reserved for EQ rotation
            return;
            // cpu_source = config_word_conj_eq_0f;
            // cpu_dest   = variable_coarse_mul;
            break;
        case 1:
            cpu_source = config_word_conj_eq_0f;
            cpu_dest   = variable_eq_correction_mul;
            break;
        case 2:
            cpu_source = config_word_cmul_eq_0f;
            cpu_dest   = variable_eq_correction_mul;
            break;
        case 3:
            cpu_source = config_word_conj_eq_0f;
            cpu_dest   = variable_analog_eq_mul;
            break;
        case 4:
            cpu_source = config_word_cmul_eq_0f;
            cpu_dest   = variable_analog_eq_mul;
            break;
        default:
            return; // exit the function
            break;
    }

    copy_set_barrel(cpu_source, cpu_dest, shift);
}

void default_barrel_shift_callback(const unsigned int data) {
    (void)data;
    // app_barrel_shift_callback(0x000000 | 0x0f);
    app_barrel_shift_callback(0x020000 | 0x0b);

    app_barrel_shift_callback(0x030000 | 0x0b);
}

void tone_2_value_callback(const unsigned int data) {
    const unsigned tone = 1022;
    const unsigned cooked_data_count = PONG_BUFFERS;
    tone_2_value = data & 0xffff; // global
    for(unsigned i = 0; i < cooked_data_count; i++) {
        vmem_counter[(1024*i) + tone] = tone_2_value;
    }
}

void ook_value_callback(const unsigned int data) {
    ook_symbol_value = data & 0xffff;
}

VMEM_SECTION unsigned int junk[16];

int main2(void);
int main(void)
{
    self_sync_block_boot();
    main2();
    return 0;
}
int main2(void) {
    Ringbus ringbus;

    SET_REG(x3, 0x00000000 | OUR_RING_ENUM);

    CSR_WRITE_ZERO(DMA_1_FLUSH_SCHEDULE);
    CSR_WRITE_ZERO(DMA_2_FLUSH_SCHEDULE);
    flush_input_dma(VMEM_DMA_ADDRESS(junk), 16, 8192);

#ifdef MEASURE_STACK_USAGE
    fill_stack();
#endif

    // Schedule an early DMA
    // this allows a more "determinstic" wakeup
    // dma_block_send(VMEM_DMA_ADDRESS(_memory_manager_chunks), DMA_OUT_CHUNK*2 );

    // let "check bootload" know we are out of sync
    // check_bootload_set_past_sync(1);


    // for(unsigned int i = 0; i < 500; i++) {
    //     // vector_memory[i] = 0xF0000000 | i;
    //     STALL(1);
    // }

    setup_debug();

    SET_REG(x3, 0xdead0000 | OUR_RING_ENUM);

    // ring_block_send_eth(0xdead0000 | OUR_RING_ENUM);

#ifdef HARDWARE_MODE
    for(unsigned int i = 0; i < 125000000; i++) {
        // vector_memory[i] = 0xF0000000 | i;
        STALL(3);
    }

    CSR_WRITE(RINGBUS_WRITE_ADDR, RING_ADDR_ETH);
    CSR_WRITE(RINGBUS_WRITE_DATA, MAPMOV_RESET_CMD);
    CSR_WRITE(RINGBUS_WRITE_EN, 0);


    CSR_WRITE_ZERO(DMA_1_FLUSH_SCHEDULE);
    CSR_WRITE_ZERO(DMA_2_FLUSH_SCHEDULE);
    flush_input_dma(VMEM_DMA_ADDRESS(junk), 16, 8192);
#endif

    _printf("%s", "boot\n");


    CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);

    // default_schedule(schedule);
    schedule_all_on(schedule);
    schedule->epoc_time = 0;

    // schedule->offset = 0x0; // random
    // // SET_REG(x4, 0);
    // // SET_REG(x4, schedule->id_mask);

    setup_duplex();
    setup_pointers();

    // _setup_perf();
    // _perf_set_acumulation_period(8); // must come after setup, this can be changed via ringbus

    setup_userdata();

    fb_parse_setup();
    fb_register_vector_callback(&fb_vector_type_callback);
    fb_register_stream_callback(&fb_stream_type_callback);
    fb_register_mapmov_callback(&user_data_callback);

    ring_register_callback(&callback_eq_setting, EQUALIZER_CMD);

    ring_register_callback(&sfo_adjustment_callback, SFO_PERIODIC_ADJ_CMD);
    ring_register_callback(&sfo_sign_callback, SFO_PERIODIC_SIGN_CMD);
    ring_register_callback(&sfo_timer_callback, SFO_COORDINATION_CMD);
    ring_register_callback(&handle_generic_callback_original, GENERIC_OPERATOR_CMD);
    ring_register_callback(&epoc_was_requested_callback, REQUEST_EPOC_CMD);
    ring_register_callback(&eth_cs20_reset_callback, MAPMOV_RESET_CMD);
    ring_register_callback(&cs20_control_progress_report, TX_PROGRESS_CMD);
    ring_register_callback(&cs20_dump_status, TX_MAIN_REPORT_STATUS_CMD);
    // ring_register_callback(&cs20_reset_lifetime_32, RESET_LIFETIME_CMD);
    // ring_register_callback(&cs20_add_lifetime_32, ADD_LIFETIME_CMD);
    ring_register_callback(&check_bootload_status, CHECK_BOOTLOAD_CMD);
    ring_register_callback(&set_ook_sc_callback, SET_TDMA_SC_CMD);
    ring_register_callback(&set_mask_sc_callback, SET_MASK_SC_CMD);
    // ring_register_callback(&calculate_epoc_callback, LIFETIME_TO_EPOC_CMD);
    ring_register_callback(&check_last_ud_callback, CHECK_LAST_USERDATA_CMD);
    ring_register_callback(&cooked_data_type_callback, COOKED_DATA_TYPE_CMD);
    ring_register_callback(&eq_rotation_callback, EQ_ROTATION_CMD);
    ring_register_callback(&readback_timer_callback, GET_TIMER_CMD);
    ring_register_callback(&app_barrel_shift_callback, APP_BARREL_SHIFT_CMD);
    ring_register_callback(&default_barrel_shift_callback, DEFAULT_APP_BARREL_SHIFT_CMD);
    ring_register_callback(&tone_2_value_callback, TX_TONE_2_VALUE);
    ring_register_callback(&ook_value_callback, TX_OOK_VALUE);
    // ring_register_callback(&stall_check_callback, STALL_CHECK_CMD);

    handle_generic_register_post_callback(&generic_op_finished);
    handle_generic_register_get_pointer(&pointer_for_generic_op);
    handle_generic_register_ring(&fb_queue_ring_eth);

    default_barrel_shift_callback(0);

#ifdef READBACK_RINGBUS_HASH
    ringbus_enable_readback(1);
#endif
    

#ifdef MEASURE_STACK_USAGE
    uint32_t counter = 0;
    uint32_t untouched_stack_bytes = 0;
#endif
#ifdef READBACK_EQ_HASH
    uint32_t counter2 = 0;
#endif

    while(1) {
        // _perf_top_idle_loop();
        pet_fb_parse();
        pet_eq_and_dma_out();

        check_ring(&ringbus);
        pet_epoc_readback();
        // SET_REG(x4, 0);
        // SET_REG(x4, schedule->id_mask);

#ifdef READBACK_EQ_HASH
        if( counter2 == 300 ) {
            pet_eq_hash();
            counter2 = 0;
        }
        counter2++;
#endif

#ifdef MEASURE_STACK_USAGE
        if(counter == 24000) {
            untouched_stack_bytes = stack_untouched();
        }
        if( counter >= 24001 ) {
            ring_block_send_eth(TEST_STACK_RESULTS_PCCMD | untouched_stack_bytes | (OUR_RING_ENUM<<20) );
            counter = 0;
        }
        counter++;
#endif

    }
    return 0;
}
