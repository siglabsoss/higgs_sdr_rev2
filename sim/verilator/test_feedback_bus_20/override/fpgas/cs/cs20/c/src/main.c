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
#include "config_word_add_eq_00.h"
// #include "performance.h"


// #define MEASURE_STACK_USAGE

#ifdef MEASURE_STACK_USAGE
#include "stack_test.h"
#endif

#include "schedule.h"
#include "check_bootload.h"
#include "random.h"

#define FORCE_INITIAL_TIMER {0x304d,(6144-8-4-4-7-6)-100}

// #define HARDWARE_MODE


///
/// If this is defined, 
/// the main loop periodically sends out ringbus with tdma_mode values
///
// #define DEBUG_TDMA_VALUES


// buffer of allocated dma pointers
// FEEDBACK_BUS_USER_POINTERS is 8, how many buffers we can allocate out at once
circular_buf_pow2_t __full_dma_pointers = CIRBUF_POW2_STATIC_CONSTRUCTOR(__full_dma_pointers, FEEDBACK_BUS_USER_POINTERS_NEXT_POW2);
circular_buf_pow2_t* full_dma_pointers = &__full_dma_pointers;

// how many frames to read into our buffer ahead of time
#define USER_DATA_PAUSE_AFTER ((FEEDBACK_BUS_USER_POINTERS)-1)

unsigned int pending_data = 0;
unsigned int pending_timeslot = 0;
unsigned int pending_length = 0;

unsigned int output_frame_count = 0;

unsigned int epoc_needs_latch = 0;

unsigned int epoc_was_latched = 0;
unsigned int epoc_latch_frame = 0;
unsigned int epoc_latch_second = 0;

unsigned int progress_report_remaining = 0;



/// 
/// States for userdata state machine
///
#define UD_NOT_ACTIVE (0)
#define UD_BUFFER (1)
#define UD_WAIT (2)
#define UD_SENDING (3)
#define UD_SENDING_TAIL (4)
#define UD_DUMPING (5)


unsigned int user_callbacks = 0;
unsigned int ud_state = UD_NOT_ACTIVE;
// requested timeslot, epoc
unsigned int ud_timeslot = 0;
unsigned int ud_epoc = 0;
unsigned int ud_total_chunks = 0;
unsigned int ud_chunk_index_consumed = 0; // chunk index of the next thing we pull out of the fifo
unsigned int ud_chunk_length;
unsigned int ud_asked_for_pause;


schedule_t _schedule;
schedule_t* schedule = &_schedule;
VMEM_SECTION unsigned int vmem_zeros[1024];

// #include "demod_output.h"
// #include "cooked_data.h" // vmem_counter
// #include "cooked_data_unrotated.h" // vmem_counter
#include "cooked_data_unrotated_320.h" // vmem_counter
#include "equalization_vectors.h" // eq_vector eq_vector_default


VMEM_SECTION unsigned int nco_data[1024] = {0};

unsigned int lifetime_frame_counter = 0;
unsigned int lifetime_32 = 0;//215; // 32 bit counter

// true if we are waiting to apply
unsigned int pending_lifetime_update = 0;
// if pending is true, when we apply
unsigned int incoming_future_lifetime_counter;


unsigned int pending_packet_SFO_adjustment_nco_freq;
unsigned int pending_packet_num_SFO_adjustment_period;
unsigned int pending_packet_SFO_adjustment_direction;

unsigned int packet_SFO_adjustment_nco_freq;
unsigned int packet_num_SFO_adjustment_period;
unsigned int packet_SFO_adjustment_direction;
unsigned int packet_counter_SFO_adjustment = 0;
unsigned int ringbus_sfo_adjustment_temp = 0;

unsigned int userdata_needs_fill_report = 0;

unsigned int eq_need_hash = 0;
unsigned int eq_hash_state = 0;
unsigned int eq_hash_progress = 0;

#define OVERWRITE_COUNTER
#define OV_COUNTER_SC (17)
// #define OV_COUNTER_SC (25)

bool run_sfo = false;

// called after the last dma from this userdata has been queued
// SETS the state to UD_NOT_ACTIVE among other things
void user_data_finished() {
    ud_chunk_index_consumed = 0;
    fb_unpause_mapmov();
    ud_timeslot = 0;
    ud_chunk_length = 0;
    ud_asked_for_pause = 0;

    ud_state = UD_NOT_ACTIVE;

    // unsigned int fill_level = circular_buf2_occupancy(full_dma_pointers);

    // if(fill_level != 0) {
    //     ring_block_send_eth(CS20_USERDATA_ERROR | 7);
    // }
}

void user_data_callback(
    unsigned int *cpu_header,
    unsigned int dma_body,
    unsigned int this_chunk_length,
    unsigned int chunk_index,
    unsigned int total_chunks) {

    unsigned int insta_free;

    feedback_frame_vector_t* header = (feedback_frame_vector_t*) cpu_header;
    
    // ring_block_send_eth(DEBUG_1_PCCMD | user_callbacks | (header->seq<<16) );

    // save the body, we always have to do this, if we don't we drop data
    // this means we need to pause the feedback bus at any time that we can't accept
    // additional pointers
    int insert_error = circular_buf2_put(full_dma_pointers, dma_body);

    unsigned int fill_level = circular_buf2_occupancy(full_dma_pointers);

    switch(ud_state) {
        case UD_NOT_ACTIVE:
            // only pay for vmem->imem access cost one time, at the start
            // however the *cpu_header should be valid for every frame
            ud_timeslot = header->seq;  // timeslot
            ud_epoc = header->seq2;     // epoc
            ud_chunk_length = this_chunk_length; // ASSUME ALL CHUNKS are same length...
            ud_total_chunks = total_chunks;
            userdata_needs_fill_report = 1;

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
        fb_queue_ring_eth(CS20_USERDATA_ERROR | 8);
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
    
    if(incoming_future_lifetime_counter>0)
    {
        if(run_sfo==false)
        {
            //FIXME: remove +2 or re-calculate this
            incoming_future_lifetime_counter = incoming_future_lifetime_counter+2;
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
        
        pending_packet_SFO_adjustment_nco_freq=(unsigned int)(4194304.0/(pending_packet_num_SFO_adjustment_period*1.0));

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

unsigned int frame_counter = 0;

VMEM_SECTION unsigned int work_area[1024*5];

VMEM_SECTION unsigned int pilot_tone_output[1024];


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


unsigned int tdma_mode = 0;
unsigned int tdma_mode_pending = 0;

#ifdef OVERWRITE_COUNTER


///
/// Driven by ringbus from rx side
/// reads global tdma_mode
///
/// pulls 32 bit word at a time
/// each call will yield the next word in the sequence
/// @param reset pass non zero value to reset

unsigned int data_next_value(unsigned int reset, unsigned int timeslot, unsigned int progress) {
    static unsigned int message = 0xdeadbeef;
    static unsigned int s0 = 0;

    if(reset) {
        s0 = 0;

        // so pending puts us at 4.
        // however nothing happens at 4
        // once a reset comes by, we go to mode 1, which starts the first of 5 or 6 words
        // which culminate in the lifetime counter
        // that's what this does
        if(tdma_mode == 4) {
            tdma_mode = 1;
        }
    }

    unsigned int ret;
    if(tdma_mode == 0) {
        if(message == 0x0) {
            message = 1;
        } else if(message == 1) {
            message = 2;
        } else if(message == 2) {
            message = 3;
        } else if(message == 3) {
            message = 4;
        } else if(message == 4) {
            message = 5;
        } else if(message == 5) {
            message = 6;
        } else if(message == 6) {
            message = 7;
        } else if(message == 7) {
            message = 0xdeadbeef;
        } else if(message == 0xaaaa0000) {
            message = 0x0;
        } else if(message == 0xffff0000) {
            message = 0xaaaa0000;
        } else if(message == 0xdeadbeef) {
            message = 0xffff0000;
        } else {
            message = 0;
        }
        ret = message;
    } else if(tdma_mode == 1) {
        s0 = 0;
        tdma_mode = 2;
        ret = 0;
    } else if(tdma_mode == 2) {
        s0++;
        if(s0 == 5) {
            ret = 0xbeefbeef;
        } else if(s0 == 6) {
            ret = lifetime_frame_counter;
            tdma_mode = 3;
        }
    } else if(tdma_mode == 3) {
        ret = 0;
    } else if(tdma_mode == 4) {
        // DO NOTHING, this is a holding state until a reset comes by
    } else if(tdma_mode == 6) {
        switch(progress) {
            case 0:
                // ret = 0xc01dbeef;
                ret = 0xdeadbeef;
                break;
            case 15:
            case 16:
                ret = SYNCHRONIZATION_CMD | timeslot;
                break;
            case 31:
            case 32:
                ret = 0xff000000;
                break;
            default:
                ret = 0;
            break;
        }
    } else if(tdma_mode == 8 || tdma_mode == 9) {
        if( timeslot == tdma_mode && progress <= 16 ) {
            fb_queue_ring_eth(TDMA_REPLY_PCCMD | (timeslot<<16) );
            fb_queue_ring_eth(TDMA_REPLY_PCCMD | (progress) );
            ret = 0xca5e0000 | timeslot;
        } else {
            ret = 0;
        }
    }


    return ret;
}


void insert_data2(unsigned int dma_ptr, unsigned int timeslot, unsigned int progress) {
    static unsigned int phase = 0;
    static int message = 0xdeadbeef;  // static assign does not grab value from data_next_value()

    // if(0) {
    //     message = data_next_value(1, timeslot, progress);
    //     phase = 0;
    // }

    if(timeslot == 0 && progress == 0) {
        phase = 0;
        message = data_next_value(1, timeslot, progress);
    }


    unsigned int shift = phase*2;

    unsigned int bits = (message>>shift)&0x3;

#define QPSK_POS (0x2a81) //+0.7
#define QPSK_NEG (0xd57e) //-0.7

    unsigned int qpsk_point;
    switch(bits) {
        case 0:
            qpsk_point = (QPSK_NEG << 16 | QPSK_NEG);
            break;
        case 1:
            qpsk_point = (QPSK_POS << 16 | QPSK_NEG);
            break;
        case 2:
            qpsk_point = (QPSK_NEG << 16 | QPSK_POS);
            break;
        case 3:
            qpsk_point = (QPSK_POS << 16 | QPSK_POS);
            break;
    }


    vector_memory[dma_ptr + OV_COUNTER_SC] = qpsk_point;

    phase++;

    if( tdma_mode == 8 ) {
        if(timeslot == 9) {
            vector_memory[dma_ptr + OV_COUNTER_SC] = 0;
        }
    }

    if( tdma_mode == 9 ) {
        if(timeslot == 8) {
            vector_memory[dma_ptr + OV_COUNTER_SC] = 0;
        }
    }

    // next mod of 5
    if(phase == 16) {
        message = data_next_value(0, timeslot, progress);
        phase = 0;
    }
}
#endif


// this is run right before sfo_eq
void handle_cs10_cs20_sfo_sync() {
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



// Instead of ping/pong
// this operates with 5 input buffers, 5 output buffers
// each input is processed into it's matching output as fast as possible
// output is done with a _block_send() so we will never have more than 4 buffers
// out at once. this guarentees we never write to a buffer while output DMA is reading
// uint32_t pending_frame = 0;
uint32_t vmem_raw_dma_ptr;
void pet_eq_and_dma_out(void) {

    uint32_t input_dma_offset;
    unsigned output_dma_offset;
    uint32_t input_offset_row;
    uint32_t output_offset_row;
    uint32_t input_dma_start;
    uint32_t work_area_dma;
    uint32_t input_start_row;
    uint32_t work_area_row;
    // uint32_t pilot_tone_output_row;
    uint32_t eq_row;
    uint32_t* input_base;
    uint32_t* output_base;
    // uint32_t* pilot_tone_output_base = pilot_tone_output;
    // choose which of our 5 frames to output,
    uint32_t progress, timeslot, can_tx, epoc, accumulated_progress;
    // schedule_t *o, uint32_t counter, uint32_t* start, uint32_t* end, uint32_t* progress, uint32_t* timeslot, uint32_t* us);
    // this is expensive to run, we can save output and all but progress would be able to be cached
    schedule_get_timeslot2(schedule,
                    lifetime_32,
                    &progress,
                    &accumulated_progress,
                    &timeslot,
                    &epoc,
                    &can_tx);

    // this might glitch when entering / exiting a timeslot
    // due to output buffers not being rotated correctly
    // this number for now is still critical to the functioning of this block
    // we can be lazy about certain things because of our 5 buffer style.
    // that being said we have an expentive mod we should get rid of here
    uint32_t frame_phase = (accumulated_progress % 5);

    if(userdata_needs_fill_report) {
        userdata_needs_fill_report = 0;
        int8_t e_delta = (int8_t)(ud_epoc - epoc);
        int16_t frame_delta = (int16_t)((ud_timeslot<<SCHEDULE_LENGTH_BITS) - accumulated_progress);
        uint32_t masked = ((e_delta<<16)&0xff0000) | (frame_delta&0xffff);
        fb_queue_ring_eth(CS20_FILL_LEVEL_PCCMD | masked );
    }


    switch(ud_state) {
            // These first 2 states
            // this means we came in JUST EARLY ENOUGH
            // this should be a warning at least
            // possibly error condition

            // ring_block_send_eth(CS20_USERDATA_ERROR | 1); // warning, technically might still be ok to run
            // NO BREAK
        case UD_BUFFER:
        case UD_WAIT:
            /// if requested epoc second is in the past
            if( ud_epoc != 0 && (ud_epoc < epoc) ) {
                ud_state = UD_DUMPING;
                fb_queue_ring_eth(CS20_USERDATA_ERROR | 2);
            } else if( (ud_epoc == epoc) ) {
                // it is currently the epoc second
                if( (ud_timeslot < timeslot) ) {
                    // if requested timeslot is in the past
                    ud_state = UD_DUMPING;
                    fb_queue_ring_eth(CS20_USERDATA_ERROR | 7);
                } else if (ud_timeslot == timeslot && progress == 0) {
                    if( can_tx ) {
                        // timeslot is ok, we are allowed to transmit, go time
                        ud_state = UD_SENDING;
                        fb_unpause_mapmov(); // we were either paused, or came in with super low buffer, so unpause
                        // ring_block_send_eth(0xdead000a);
                    } else {
                        // timeslot is ok for us to go, but schedule says it is illegal to transmit
                        ud_state = UD_DUMPING;
                        fb_queue_ring_eth(CS20_USERDATA_ERROR | 4);
                    }
                    // technically we could check for buffer fill level
                    // but it should be impossible to get into this state with a bad fill level?
                } else if (ud_timeslot > timeslot) {
                    // do nothing,
                    // we are in the right second, but not the right timeslot
                } else {
                    // fail, we got the packet in the right epoc second, but timeslot was too late
                    // even if timeslot was correct, progress was not zero, so still fail
                    ud_state = UD_DUMPING;
                    fb_queue_ring_eth(CS20_USERDATA_ERROR | 5);
                }
                SET_REG(x3, 0x00003000);
                SET_REG(x3, ud_epoc);
                SET_REG(x3, epoc);
                SET_REG(x3, ud_timeslot);
                SET_REG(x3, timeslot);
                SET_REG(x3, progress);


            } else {
                // do nothing, epoc is in the future
                SET_REG(x3, 0x00003001);
                SET_REG(x3, ud_epoc);
                SET_REG(x3, epoc);
                SET_REG(x3, ud_timeslot);
                SET_REG(x3, timeslot);
                SET_REG(x3, progress);

            }
            break;

        case UD_DUMPING:
        case UD_SENDING:
        case UD_SENDING_TAIL:
        case UD_NOT_ACTIVE:
            SET_REG(x3, 0x00003002);
            SET_REG(x3, ud_epoc);
            SET_REG(x3, epoc);
            SET_REG(x3, ud_timeslot);
            SET_REG(x3, timeslot);
            SET_REG(x3, progress);
            break;
        default:
            break;
    }

    // // SET_REG(x3, 0xfeed);
    // // SET_REG(x3, timeslot);
    // // SET_REG(x3, myy);

    unsigned int do_free = 0; // INITIALIZE THIS
    unsigned int dma_pointer;

    unsigned int did_underflow_this_time = 0;

    if(can_tx) {
        if(
               ud_state == UD_SENDING
            || ud_state == UD_SENDING_TAIL
           ) {

            unsigned int ud_will_consume = ud_chunk_index_consumed;
            int error = circular_buf2_get(full_dma_pointers, &dma_pointer);

            if( error != 0 ) {
                // we were in sending state but we UNDERFLOWED

                fb_queue_ring_eth(
                    CS20_USERDATA_ERROR | 6 |
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

                output_dma_offset = (0*1024);
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

            output_dma_offset = (frame_phase*1024);
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
            if( tdma_mode == 7 ) {
                // tdma_mode is controlled by rb
                // see setPartnerTDMA() in HiggsPartner.cpp
                frame_phase = 1;
            }
            // which phase of the canned output are we on?
            // this resets to 0 at the start of a timeslot
            // input_dma_offset = (input_frame_phase*1024);
            input_dma_offset = (frame_phase*1024);
            input_offset_row = input_dma_offset/16;
            output_dma_offset = (frame_phase*1024);
            output_offset_row = output_dma_offset/16;
        //   output_dma_offset = input_dma_offset = (frame_phase*1024);
        // output_offset_row = input_offset_row = input_dma_offset/16; 
            // these are cpu pointers to base of a 5 buffer long array
            input_base = (uint32_t*)vmem_counter;
            output_base = (uint32_t*)work_area;
        }
    } else {
        // output_dma_offset = (frame_phase*1024);
        output_dma_offset = (0*1024);
        output_offset_row = input_dma_offset/16;

        // zero buffer is only 1 long not 5, so always start at the beginning
        input_dma_offset = 0;
        input_offset_row = 0;

        input_base = (uint32_t*)vmem_zeros;
        output_base = (uint32_t*)work_area;
    }


    // calculate a bunch row counters etc
    // calculate start dma/row for input
    input_dma_start = VMEM_DMA_ADDRESS(input_base) + input_dma_offset;
    input_start_row = VMEM_ROW_ADDRESS(input_base) + input_offset_row;

    // SET_REG(x31, 0xABCDABCD);
    // SET_REG(x31, input_dma_start);
    // SET_REG(x31, input_start_row);
    // SET_REG(x31, mapper_count);
    // SET_REG(x31, pending_frame);

    // calculate start dma/row for output
    work_area_dma = VMEM_DMA_ADDRESS(output_base) + output_dma_offset;
    work_area_row = VMEM_ROW_ADDRESS(output_base) + output_offset_row;

    // SET_REG(x3, input_dma_start);
    // SET_REG(x3, input_start_row);
    // SET_REG(x3, work_area_dma);
    // SET_REG(x3, work_area_row);

    // never changes
    eq_row = VMEM_ROW_ADDRESS(eq_vector);

#ifdef OVERWRITE_COUNTER
    // optionaly overwrite a subcarrier

    /// 
    /// This "latches" mode_pending  into tdma_mode
    ///
    /// see tdma_callback()
    ///
    if(progress == 0 && timeslot == 0 && tdma_mode_pending != 0) {
        tdma_mode = tdma_mode_pending;
    }

if(can_tx) {

    // do not insert when tdma_mode is larger than 4
    if( tdma_mode <= 9 ) {
        insert_data2(input_dma_start, timeslot, progress);
    }
}
#endif
    // pilot_tone_output_row = VMEM_ROW_ADDRESS(pilot_tone_output_base);

    unsigned int* input_as_cpu = REVERSE_VMEM_DMA_ADDRESS(input_dma_start);
    SET_REG(x3, 0xfeedfeed);
    SET_REG(x3, input_as_cpu[2]);



    // multiply by eq
    vector_multiply_1024(0, input_start_row, eq_row, work_area_row );

    // handles:
    //  incoming_future_lifetime_counter
    //  lifetime_frame_counter
    // sets:
    //  run_sfo
    // these are just counters, no vector work
    handle_cs10_cs20_sfo_sync();

    // if we are not in sfo mode 0
    if(run_sfo)
    {
        // Pre-rotate each subcarrier a slightly different amount to cancel the effect of SFO
        sfo_phase_correction(work_area_row, packet_counter_SFO_adjustment, packet_SFO_adjustment_nco_freq, packet_SFO_adjustment_direction);
        packet_counter_SFO_adjustment= packet_counter_SFO_adjustment+1;
        if(packet_counter_SFO_adjustment == packet_num_SFO_adjustment_period)
        {
            packet_counter_SFO_adjustment = 0;
        }
    }

    dma_block_send(work_area_dma, 1024);

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
                ring_block_send_eth(CS20_EPOC_REPORT_PCCMD | (epoc&0xffffff) );
            } else {
                ring_block_send_eth(CS20_PROGRESS_REPORT_PCCMD | (accumulated_progress) );
            }
            progress_report_remaining--;
        }
    }


    frame_counter++;
    lifetime_frame_counter = (lifetime_frame_counter+1)&0xffffff;
    lifetime_32++; // runs at same rate as frame_counter but overflows less frequently

    if(frame_counter == 5) {
        frame_counter = 0;
    }
    // if(pending_frame==17){
    //     pending_frame=0;
    // }

}


// epoc_latch_second
// epoc_latch_frame

// returns if more data to be sent
unsigned int pet_epoc_readback2() {

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
void pet_epoc_readback() {
    while(pet_epoc_readback2()) {
    }
}


void fb_vector_type_callback(unsigned int *header, unsigned int *body, unsigned int vector_length, unsigned int vtype) {
    unsigned input_row;
    unsigned eq_row = VMEM_ROW_ADDRESS(eq_vector);
    fb_queue_ring_eth(0xdead0001);
    switch(vtype) {
        case FEEDBACK_VEC_TX_EQ:
            input_row = VMEM_ROW_ADDRESS(body);
            vmem_copy_rows(input_row, eq_row, 1024/16);
            // ring_block_send_eth(0xdeadfeed);
            eq_need_hash = 1;
            break;
        case FEEDBACK_VEC_SCHEDULE:
            // memcpy(schedule->s, body, SCHEDULE_SLOTS*sizeof(uint32_t));
            for(unsigned int i = 0; i < SCHEDULE_SLOTS; i++) {
                schedule->s[i] = body[i];
            }
            break;
        case FEEDBACK_VEC_STATUS_REPLY:
            break; // prevent default
        default:
            fb_vector_default();
            break;
    }
}


void fb_stream_type_callback(unsigned int *header, unsigned int *body, unsigned int stream_length, unsigned int stype) {
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
    }
}

///
///  ringbus call back for tdma sync
///  is set to 0
///  modes:
///    0 previously this was putting out a steady 0xdeadbeef, etc on the tone,
///      this gets set into dmode immediately as it is a kind of reset
///    4 this is set into pending
///
///  flow:
///      mode_pending -> 4
///      mode <- mode_pending  at start of ts0    line 576
///    data_value_next:
///      if mode == 4, mode = 1
///        data is sent,
///      mode goes 1,2,3 and parks at 3
///   because pending is never reset, we continue to set pending to 4

void tdma_callback(unsigned int data) {
    unsigned int dmode = (data >> 16) & 0xff;

    switch(dmode) {
        case 0:
        case 1:
            tdma_mode = dmode;
            tdma_mode_pending = 0;
            break;
                // any mode listed here just sets and returns
                //
        case 6: // mode 6 is a new test thing
        case 4: // 4 is what was working before with alpha/demo
        case 7: // this gets us stuck in the first data row
        case 8: // how to tell
        case 9: // how to tell
            tdma_mode_pending = dmode;
            break;
        case 20:
            // any mode listed here for sets them to the same
            // sort of a disable state (disable happens elsewhere)
            tdma_mode_pending = dmode;
            tdma_mode = dmode;
            break;
        case 21:
            pending_data = 0;
            pending_timeslot = 0;
            pending_length = 0;
            schedule->offset = 0;
        default:
            break;
    }
}

void setup_tdma() {
    tdma_mode_pending = tdma_mode = 20;
}

void schedule_callback(unsigned int data) {
    schedule->offset += data;
}

void schedule_epoc_callback(unsigned int data) {
    unsigned int dmode = (data >> 16) & 0xff;
    unsigned int data_16 = data & 0xffff;
    
    
}

void epoc_was_requested_callback(unsigned int data) {
    // unsigned int dmode = (data >> 16) & 0xff;
    // unsigned int data_16 = data & 0xffff;
    epoc_needs_latch = 1;
    
}


// tricky, because we want to leave the lifetime counter alone
// so we bend over backwards to pick a schedule->offset such that it will be back to 0
void schedule_reset_callback(unsigned int data) {

    uint32_t now = lifetime_32 + schedule->offset;

    uint32_t sample = now % SCHEDULE_FRAMES;

    schedule->offset += SCHEDULE_FRAMES - sample;
}

void setup_userdata() {
    CIRBUF_POW2_RUNTIME_INITIALIZE(__full_dma_pointers);

    ud_state = UD_NOT_ACTIVE;
    user_data_finished(); // a mini reset, call it here why not
}

// tries to do best to reset map/mov/feedback bus code
void eth_cs20_reset_callback(unsigned int data) {
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

void cs20_reset_lifetime_32(unsigned int data) {
    lifetime_32 = 0;
    schedule->epoc_time = 0;
}

void cs20_add_lifetime_32(unsigned int data) {
    int sig = ((int)data)<<8>>8; // sign extend
    lifetime_32 += sig;

    int mod = sig / SCHEDULE_FRAMES;

    schedule->epoc_time += mod;
}

void cs20_control_progress_report(unsigned int data) {
    progress_report_remaining = data;
}

void cs20_dump_status(unsigned int data) {
    int inc = 0x00000000;

    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (frame_counter & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (lifetime_frame_counter & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (lifetime_32 & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | ((lifetime_32>>16) & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (pending_data & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (pending_timeslot & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (pending_length & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (output_frame_count & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (epoc_needs_latch & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (epoc_was_latched & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (progress_report_remaining & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (user_callbacks & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (ud_state & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (ud_timeslot & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (ud_epoc & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (ud_total_chunks & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (ud_chunk_index_consumed & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (ud_chunk_length & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (ud_asked_for_pause & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (pending_lifetime_update & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (incoming_future_lifetime_counter & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (userdata_needs_fill_report & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (run_sfo & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (tdma_mode & 0x0000ffff)  ); inc += 0x00010000;
    fb_queue_ring_eth( CS20_REPORT_STATUS_PCCMD | inc | (tdma_mode_pending & 0x0000ffff)  ); inc += 0x00010000;
}

void pet_eq_hash() {
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

        eq_hash_state = xorshift32(eq_hash_state, eq_vector + eq_hash_progress, 16);

        eq_hash_progress += 16;

        if( eq_hash_progress >= 1024 ) {
            eq_hash_progress = 0;

            fb_queue_ring_eth( FEEDBACK_HASH_PCCMD | 0x00000 | (eq_hash_state & 0x0000ffff)  );
            fb_queue_ring_eth( FEEDBACK_HASH_PCCMD | 0x10000 | (eq_hash_state>>16 & 0x0000ffff)  );
        }
    }

}


VMEM_SECTION unsigned int junk[16];

int main(void)
{
    Ringbus ringbus;

    SET_REG(x3, 0x00000000 | OUR_RING_ENUM);

    CSR_WRITE_ZERO(DMA_1_FLUSH_SCHEDULE);
    CSR_WRITE_ZERO(DMA_2_FLUSH_SCHEDULE);
    flush_input_dma(VMEM_DMA_ADDRESS(junk), 16, 8192);

#ifdef MEASURE_STACK_USAGE
    fill_stack();
#endif


    for(unsigned int i = 0; i < 500; i++) {
        // vector_memory[i] = 0xF0000000 | i;
        STALL(1);
    }

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





    CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);

    unsigned int a,b;

    unsigned int state = 0;




    // default_schedule(schedule);
    schedule_all_on(schedule);

    schedule->offset = 0x0; // random
    // // SET_REG(x4, 0);
    // // SET_REG(x4, schedule->id_mask);

#ifdef FORCE_INITIAL_TIMER
    schedule->epoc_time = (uint32_t[2])FORCE_INITIAL_TIMER[0];
    lifetime_32         =  (uint32_t[2])FORCE_INITIAL_TIMER[1];
#endif

    setup_tdma();

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
    ring_register_callback(&tdma_callback, TDMA_CMD);
    ring_register_callback(&schedule_callback, SCHEDULE_CMD);
    ring_register_callback(&schedule_reset_callback, SCHEDULE_RESET_CMD);
    ring_register_callback(&schedule_epoc_callback, SCHEDULE_EPOC_CMD);
    ring_register_callback(&epoc_was_requested_callback, REQUEST_EPOC_CMD);
    ring_register_callback(&eth_cs20_reset_callback, MAPMOV_RESET_CMD);
    ring_register_callback(&cs20_control_progress_report, CS20_PROGRESS_CMD);
    ring_register_callback(&cs20_dump_status, CS20_MAIN_REPORT_STATUS_CMD);
    ring_register_callback(&cs20_reset_lifetime_32, RESET_LIFETIME_CMD);
    ring_register_callback(&cs20_add_lifetime_32, ADD_LIFETIME_CMD);
    ring_register_callback(&check_bootload_status, CHECK_BOOTLOAD_CMD);
    

#ifdef MEASURE_STACK_USAGE
    uint32_t counter = 0;
    uint32_t untouched_stack_bytes = 0;
#endif

    while(1) {
        // _perf_top_idle_loop();
        pet_fb_parse();
        pet_eq_and_dma_out();

        check_ring(&ringbus);
        pet_epoc_readback();
        // SET_REG(x4, 0);
        // SET_REG(x4, schedule->id_mask);
        pet_eq_hash();

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

}
