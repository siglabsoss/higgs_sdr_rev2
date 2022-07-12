#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "dma.h"
#include "fill.h"
#include "mover.h"
#include "mapper.h"
#include "ringbus.h"
#include "circular_buffer.h"
#include "feedback_bus.h"
#include "trunk_types.h"
#include "vmem_copy.h"
#include "self_sync.h"
#include "stamped_default_stream.h"
#include "zero_default_stream.h"
#include "duplex_schedule.h"
#include "handle_generic_op.h"
#include "vector_add.h"


// special for test_feedback_bus_25
// #define DEBUG_TEST25_SEND_ALL_RB







// #define ENABLE_TB_DEBUG

#ifndef ENABLE_TB_DEBUG
#define DISABLE_TB_DEBUG
#endif
#include "tb_debug.h"


#define PING_PONG_DISABLE_OUTPUT
#define PING_PONG_BUFFER_SIZE_IN (1024+1024+TRUNK_LENGTH)

#include "ping_pong_driver.h"


#define FFT_SIZE (1024)

// offset of words to the start of the trunk
#define TRUNK_OFFSET (1024+1024)


duplex_timeslot_t duplex;       // set to our own mode
static uint32_t lifetime_32 = 0;
static uint32_t duplex_mode;
static uint32_t duplex_progress;
static uint32_t duplex_mode_rx;
duplex_timeslot_t duplex_rx;    // always set to rx mode
static unsigned cooked_data_type = 0;
static bool forward_eq = false;
// static uint32_t enable_composite_ul_pilot = 1;

// static uint32_t debug_F_composite = 0;


// extra feedback memory for cs20 -> cs11 feedback bus frames
// this is when the feedback bus destination is internal to the higgs board
// static VMEM_SECTION unsigned internal_mem_a[FFT_SIZE];
// static VMEM_SECTION unsigned internal_mem_b[FFT_SIZE];
// static unsigned* internal_cpu[2]; // cpu pointer

// static VMEM_SECTION unsigned int garbage_vmem[16];
// unsigned int garbage_row;

static VMEM_SECTION unsigned int vmem_zeros[FFT_SIZE];




// #define MODE_QPSK_128
// #define MODE_QAM_16_128
// #define MODE_QAM_16_320
#define MODE_QPSK_320
// #define MODE_QPSK_640_LIN
// #define MODE_QAM_16_640_LIN
// #define MODE_QPSK_512_LIN

// #define DONT_SLICE_DATA


#ifdef MODE_QPSK_640_LIN
#define DISABLE_SLICE_MOVER
#endif
#ifdef MODE_QAM_16_640_LIN
#define DISABLE_SLICE_MOVER
#endif
#ifdef MODE_QPSK_512_LIN
#define DISABLE_SLICE_MOVER
#endif


// -----------------------------

#ifdef MODE_QAM_16_128
#define SLICER_MOVER_ROWS (16)
#endif

#if defined MODE_QAM_16_320 || defined MODE_QPSK_320
#define SLICER_MOVER_ROWS (40)
#endif





#define DMA_IN_EXTRA (TRUNK_LENGTH)

#define FINE_SYNC_EXTRA (2)
#define FINE_SYNC_LENGTH (4)

// overwrite the 63th sc with index 62
// (63 is what s-modem uses for soft demod)
// comment this and main loop will not compile code to overwrite sc
#define OVERWRITE_CUSTOM_SC_INTO (62)




// #define FLUSH_AT_START


#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "check_bootload.h"




unsigned int custom_subcarrier_index = 41;


unsigned int count_in = 0;
unsigned int count_out = 0;


// unsigned int frame_num_all_output_period = 1024; //packet_num_SFO_adjustment_period
// unsigned int frame_num_all_output_counter = 0;

unsigned stamp_output_mode = 0;

uint32_t zero_output_mode = 0;

#define ZERO_OUTPUT_MASK_0   0xff
#define ZERO_OUTPUT_MASK_1   0xff
#define ZERO_OUTPUT_SHIFT_0  0
#define ZERO_OUTPUT_SHIFT_1  8

#define ZERO_OUTPUT_OPT_0 ((zero_output_mode>>ZERO_OUTPUT_SHIFT_0)&ZERO_OUTPUT_MASK_0)
#define ZERO_OUTPUT_OPT_1 ((zero_output_mode>>ZERO_OUTPUT_SHIFT_1)&ZERO_OUTPUT_MASK_1)


static uint32_t stall_check = 0;

// normal operation
// accept words, map to bpsk, move to subcarriers, output

// modes
// accepts words, map to "debug bpsk" (values of 0,1,2,3), move to subcarriers, output
// #define USE_FAKE_BPSK

// ignore input, map counter values to subcarriers, output
// when this is set, USE_FAKE_BPSK, has no effect
// #define USE_FAKE_MOVER_INPUT

// controlls which style of schedule is consumed
// simply enabling this is not enough, setup_mover() should also be edited
// old means dmem schedule and mover_schedule()
// new means vmem schedule and mover_load_vmem()
// #define USE_OLD_MOVER_SCHEDULE_FORMAT


// might only be valid when USE_OLD_MOVER_SCHEDULE_FORMAT is not enabled
// disabling this means we will wait for every dma output to complete before scheduling
// the next
// #define USE_DOUBLE_BUFFER


#define MY_ASSERT(x) if(!(x)) { ring_block_send_eth(0xe0000000|__LINE__);}


#define TEST_DATA_LENGTH 64

VMEM_SECTION feedback_frame_vector_filled_t vec_fine_sync;
VMEM_SECTION feedback_frame_vector_filled_t vec_demod_data;
VMEM_SECTION feedback_frame_stream_filled_t stream_default;
VMEM_SECTION feedback_frame_stream_filled_t stream_all_sc;
VMEM_SECTION feedback_frame_vector_filled_t vec_eq_feedback_analog;
VMEM_SECTION feedback_frame_vector_filled_t vec_eq_feedback_correction;

#define ALL_ZERO_LENGTH (367*2)
VMEM_SECTION unsigned int all_zeros[ALL_ZERO_LENGTH];
#include "cs21_use_counter_data.h"

void setup_feedback_bus(void) {
    init_feedback_stream(&stream_default, FEEDBACK_PEER_SELF, false, true, FEEDBACK_STREAM_DEFAULT);
    init_feedback_stream(&stream_all_sc, FEEDBACK_PEER_SELF, false, true, FEEDBACK_STREAM_ALL_SC);

    init_feedback_vector(&vec_fine_sync, FEEDBACK_PEER_SELF, false, true, FEEDBACK_VEC_FINE_SYNC);
    init_feedback_vector(&vec_demod_data, FEEDBACK_PEER_SELF, false, true, FEEDBACK_VEC_DEMOD_DATA);

    init_feedback_vector(&vec_eq_feedback_analog, FEEDBACK_PEER_SELF, true, false, FEEDBACK_VEC_EQ_ANALOG);
    set_feedback_vector_length(&vec_eq_feedback_analog, FFT_SIZE);

    init_feedback_vector(&vec_eq_feedback_correction, FEEDBACK_PEER_SELF, true, false, FEEDBACK_VEC_EQ_CORRECTION);
    set_feedback_vector_length(&vec_eq_feedback_correction, FFT_SIZE);

    // NEVER directly set the length, the length field is not simply the length of data
    set_feedback_vector_length(&vec_fine_sync, FINE_SYNC_LENGTH+FINE_SYNC_EXTRA);
}

void set_stream_length(unsigned int len) {
    SET_REG(x3, 0xface);
    SET_REG(x3, len);
    set_feedback_stream_length(&stream_default, len);
    set_feedback_stream_length(&stream_all_sc, FFT_SIZE); // length is all subcarriers
}


// used for stream mover
#define DST_ROW_REV (VMEM_ROW_ADDRESS(dst_mem))



// FIXME replace with pointer from ping pong
#define SLICED_DATA_ROW2 0 //(VMEM_ROW_ADDRESS(dst_mem_data))

unsigned int mapper_output_row;

unsigned int demod_mode = 0;


// int mover_working_on;

// FIXME lame way of doing this
#define MAX_SCHEDULE_COUNT 44

#define GARBAGE_ROW       (garbage_row)

// in the reverse mover, DST_ROW is actually the source

// must be 0 for new ping pong setup
#define SRC_ROW_REV           (0)

// used for slicer mover (must be 0 for new ping pong setup)
#define DST_ROW_REV2 (0)


//FIXME remove and replace with pointer from ping pong
// #define INPUT_DMA_DMA_ADDRESS (0) // (VMEM_ROW_ADDRESS(input_dma))

unsigned int garbage_row;
// unsigned int dma_in_full = 0;
// unsigned int dma_out_done = 0;


// Directly create this in vmem (we could also load this at compile time with a compile time change to the value of DST_ROW)
VMEM_SECTION VmemSchedule vmem_schedules[MAX_SCHEDULE_COUNT];

VMEM_SECTION VmemSchedule mover_a[SLICER_MOVER_ROWS];
VMEM_SECTION VmemSchedule mover_b[SLICER_MOVER_ROWS];


// VMEM_SECTION_OFFSET
// the 2 is for ping/pong
// VMEM_SECTION unsigned int input_dma[(FFT_SIZE+DMA_IN_EXTRA)*2] = {};

// worst case 1024 enabled subcarriers
// VMEM_SECTION unsigned int dst_mem[FFT_SIZE*2] = {};
// VMEM_SECTION unsigned int dst_mem_data[FFT_SIZE*2] = {};
VMEM_SECTION unsigned int garbage_mem[16] = {};

VMEM_SECTION unsigned int dst_mem_data_a[FFT_SIZE];
VMEM_SECTION unsigned int dst_mem_data_b[FFT_SIZE];

// the 2 is for ping/pong
// VMEM_SECTION unsigned int empty_row[16] = {};
// VMEM_SECTION unsigned int empty[1] = {0xcafebabe};

#include "cs21_example_data.h"

unsigned int enabled_subcarriers; // delcared here but SET BY OUTPUT FROM schedule_maker.py
unsigned int number_active_schedules; // same as previous
unsigned int enabled_subcarriers_data; // delcared here but SET BY OUTPUT FROM schedule_maker.py
unsigned int number_active_schedules_data; // same as previous

// unsigned int dma_in_dma_ptr;

VMEM_SECTION unsigned int rolling_sc_vmem[FINE_SYNC_EXTRA];

#include "slicer_mover_320_predicate.h"


/// Copies a mover schedule, to a new location
/// the copy then has the input and output buffers overwritten to the supplied buffers
/// the output buffers are only partially overwritten. we used a predicate array to keep the
/// garbage rows intact
/// @param[in] src cpu pointer to src vmem schedule.  both input and output buffers need to be zero
/// @param[in] dst cpu pointer to final output vmem schedule
/// @param[in] len how many schedules are there (note each schedule is actually 4 rows of memory)
/// @param[in] cpu_buffer_in cpu pointer to input memory, will be written dst schedule
/// @param[in] cpu_buffer_out cpu pointer to output memory, will be written dst schedule
void generate_ping_pong_mover(
    const VmemSchedule* src,
    const VmemSchedule* dst,
    const uint32_t len,
    const unsigned* const cpu_buffer_in,
    const unsigned* const cpu_buffer_out
    ) {

    const unsigned src_row = VMEM_ROW_ADDRESS((const unsigned int*)src);
    const unsigned dst_row = VMEM_ROW_ADDRESS((const unsigned int*)dst);

    // copy the entire schedule, without modification
    vmem_copy_rows(
        src_row,
        dst_row,
        4*len
    );

    const unsigned in_buffer_row  = VMEM_ROW_ADDRESS(cpu_buffer_in);
    const unsigned out_buffer_row = VMEM_ROW_ADDRESS(cpu_buffer_out);

    // modify-in-place the every 4th row starting from index 0.
    // add the input buffer row address
    vector_add_scalar(dst_row, in_buffer_row, len, 4);

    // modify-in-place the every 4th row starting from index 2
    // add the output buffer row address
    // note that we need to keep the garbage row in-tact.  so we 
    // have a parallel calculated vector of predicates
    uint16_t* pred = (uint16_t*)vmem_schedule_predicate;

    vector_add_scalar_predicate(dst_row+2, out_buffer_row, len, 4, pred);

}

void setup_mover(void) {
    garbage_row = VMEM_ROW_ADDRESS(garbage_mem);

    // dma_in_dma_ptr = VMEM_DMA_ADDRESS(input_dma);




    // pulled from stream_mover_64.h
    enabled_subcarriers = 64;
    number_active_schedules = 4;
// #include "stream_mover_64.h"




#ifdef MODE_QAM_16_128
#include "slicer_mover_128.h"
#endif

#ifdef MODE_QAM_16_320
#include "slicer_mover_320.h"
#endif

#ifdef MODE_QPSK_320
#include "slicer_mover_320.h"
#endif


    dump_vmem_cpu(0, (unsigned int*)(vmem_schedules+4), 4*16*40);

    // +4 here is due to hand edits to slicer_mover_320.h

    generate_ping_pong_mover(
        vmem_schedules+4,
        mover_a,
        SLICER_MOVER_ROWS,
        input_dma_buf,
        dst_mem_data_a
    );
    generate_ping_pong_mover(
        vmem_schedules+4,
        mover_b,
        SLICER_MOVER_ROWS,
        input_dma_buf + PING_PONG_BUFFER_SIZE_IN, // cpu pointer pointer arithmetic
        dst_mem_data_b
    );

    dump_vmem_cpu(0, (unsigned int*)(mover_a), 4*16*40);
}



unsigned int input_frame_count;

#ifdef USE_DOUBLE_BUFFER
unsigned int mover_output_increment_words;
unsigned int mover_output_increment_row;
#endif
// call after setup_mover
void setup_mover_post(void) {
  input_frame_count = 1;

#ifdef USE_DOUBLE_BUFFER
  // bumps for our "a" / "b" buffers
  mover_output_increment_words = input_frame_count << 10; // times 1024
  mover_output_increment_row = mover_output_increment_words >> 4; // over 16
#endif
}




// must be the same as FRAME_MOVE_CHUNK
#define DMA_IN_CHUNKS (1)

// in words
// #define DMA_IN_SIZE ( (FFT_SIZE+DMA_IN_EXTRA)*DMA_IN_CHUNKS)

// #define DMA_IN_CIRBUF_SIZE (DMA_IN_CHUNKS+1)
// circular_buf_t dma_in_buffer;
// unsigned int dma_in_buffer_storage[DMA_IN_CIRBUF_SIZE];

// setting this to 5 means buffer can hold 4
// #define DMA_SCHEDULE_IN_SIZE (4+1)
// circular_buf_t dma_schedule_in;
// unsigned int dma_schedule_in_storage[DMA_SCHEDULE_IN_SIZE];

// #define DMA_SCHEDULE_OUT_SIZE (4+1)
// circular_buf_t dma_schedule_out;
// unsigned int dma_schedule_out_storage[DMA_SCHEDULE_OUT_SIZE];

// unsigned int dma_trig_next = 0;
// unsigned int dma_trig_previous = 0;
// converts a dma index (used in the cirbufs) to a dma_ptr
// the dma index counts each block of memory
// unsigned int dma_idx_to_ptr(const unsigned int idx) {
//   return dma_in_dma_ptr + (idx * DMA_IN_SIZE);
// }

// // convert a dma_ptr to an index
// unsigned int dma_ptr_to_idx(const unsigned int ptr) {
//   return (ptr - dma_in_dma_ptr) / DMA_IN_SIZE;
// }

// void trig_dma_in(const unsigned int dma_ptr) {
//   // ring_block_send_eth(dma_ptr);
//   CSR_WRITE(DMA_0_START_ADDR, dma_ptr);
//   CSR_WRITE(DMA_0_LENGTH, DMA_IN_SIZE);
//   CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff); // start right away
//   CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);
//   count_in++;
// }

void dma_block_send_sliced(
    const unsigned int dma_ptr,
    const unsigned int word_count,
    const unsigned int slice_type,
    const unsigned int qam_constellation) {
  unsigned int occupancy;
  while(1) {
    CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
    if( occupancy < DMA_1_SCHEDULE_DEPTH) {
      break;
    }
  }
  CSR_WRITE(DMA_1_START_ADDR, dma_ptr);
  CSR_WRITE(DMA_1_LENGTH, word_count);
  CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);
  CSR_WRITE(SLICER, slice_type);
  CSR_WRITE(DEMAPPER_CONSTELLATION, qam_constellation);
  CSR_WRITE(DMA_1_LAST_RTL, 1);
  CSR_WRITE_ZERO(DMA_1_PUSH_SCHEDULE);
}
// void trig_dma_out(unsigned int dma_ptr) {
//   CSR_WRITE(DMA_1_START_ADDR, dma_ptr);
//   CSR_WRITE(DMA_1_LENGTH, DMA_IN_SIZE);
//   CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff); // start right away
//   CSR_WRITE_ZERO(DMA_1_PUSH_SCHEDULE);
// }


// void trig_dma_in_next(void) {
//     SET_REG(x3, 0xc0000000);

//     trig_dma_in(dma_idx_to_ptr(dma_trig_next));

//     circular_buf_put(&dma_schedule_in, dma_trig_next);

//     dma_trig_previous = dma_trig_next;
//     dma_trig_next = (dma_trig_next+1) % DMA_IN_CHUNKS;
// }
//////////////////////////////////////////
//
// We run 2 circular buffers2
// the first buffer keeps track of outstanding input dma so they are always overlapping
// as these they dump into the 2nd circular buffer which is the "pending data" and also our fill level

// void setup_dma_in(void) {
//   // dma_in_dma_ptr = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));

//   circular_buf_initialize(&dma_schedule_in, dma_schedule_in_storage, DMA_SCHEDULE_IN_SIZE);
//   // circular_buf_initialize(&dma_in_buffer, dma_in_buffer_storage, DMA_IN_CIRBUF_SIZE);

//   dma_in_full = 0;
//   trig_dma_in_next();
// }

// void setup_dma_out(void) {
//   circular_buf_initialize(&dma_schedule_out, dma_schedule_out_storage, DMA_SCHEDULE_OUT_SIZE);
// }

// unsigned int fake_work_todo = 0;

void recover_last(void) {
    ring_block_send_eth(DMA_LAST_ERROR_PCCMD | OUR_RING_ENUM);
    dma_run_till_last();
}

// void pet_dma_inqueue(void) {
//     int error;
//     unsigned int just_finished_idx;
//     unsigned int dma_occupancy;
//     unsigned int status;
//     // unsigned int dma_out_occupancy;

//     CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, dma_occupancy);
//     // CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, dma_out_occupancy);
//     unsigned int filled = circular_buf_occupancy(&dma_schedule_in);

//     // unsigned int outgoing_buf_occupancy;

//     SET_REG(x3, 0xe0000000 | count_in );
//     SET_REG(x3, 0xf0000000 | count_out );

//     if( dma_in_full == 0 ) {
//         if(dma_occupancy != filled) {
//             CSR_READ(DMA_0_STATUS, status);
//             if( status ) {
//                 recover_last();
//             }

//             dma_in_full = 1;
//             SET_REG(x3, 0xbb000000);

//             return;
//         }
//     }

//   if( dma_occupancy != filled && dma_out_done == 1) {
//     // just_finished_idx is the index of the dma that just finished
//     error = circular_buf_get(&dma_schedule_in, &just_finished_idx); MY_ASSERT(error == 0);

//     // now that dma is done with this chunk, we add it to the next
//     // circular buffer which signals the program that there is fresh data to be processed
//     // circular_buf_put(&dma_in_buffer, just_finished_idx);
//     // ring_block_send_eth(dma_occupancy);
//     // ring_block_send_eth(filled);

//     // outgoing_buf_occupancy = circular_buf_occupancy(&dma_in_buffer);

//     SET_REG(x3, 0xb0000000 | 1);

//     // fake_work_todo += 10;


//     // ring_block_send_eth(data);
//     dma_in_full = 0;
//     dma_out_done = 0;
//     trig_dma_in_next();
//   }

// }


// void pet_dma_in(void) {
//   pet_dma_inqueue();
// }

void send_feedback_bus_header(const void* const cpu_ptr) {
    dma_block_send_finalized(VMEM_DMA_ADDRESS(cpu_ptr), FEEDBACK_HEADER_WORDS, 1);
}

// unsigned int debug_readout(unsigned int count) {
//   unsigned int dma_idx_just_finished;
//   unsigned int* dma_cpu_pointer;
//   int error;
//   for(unsigned i = 0; i < count; i++) {
//     error = circular_buf_get(&dma_in_buffer, &dma_idx_just_finished); MY_ASSERT(error == 0);

//     dma_cpu_pointer = REVERSE_VMEM_DMA_ADDRESS(dma_idx_to_ptr(dma_idx_just_finished));

//     for(unsigned int j = 0; j < 16; j++) {
//       ring_block_send_eth(dma_cpu_pointer[j]);
//     }


//   }
// }




// 640 lin doesn't use the reverse mover at all
// instead we use 2 carefully placed mem copy
static void mover_640_lin(const unsigned int input_row, const unsigned int output_row) {
    // 40 is 640 subcarriers
    vmem_copy_rows(input_row + 1, output_row, 20);

    vmem_copy_rows(input_row + 64 - 1 - 20, output_row+20, 20);
}

// 640 lin doesn't use the reverse mover at all
// instead we use 2 carefully placed mem copy
static void mover_512_lin(const unsigned int input_row, const unsigned int output_row) {
    // 40 is 640 subcarriers

    // dump_vmem_row(2, input_row, 1024);

    vmem_copy_rows(input_row + 8, output_row, 20-8);

    vmem_copy_rows(input_row + 64 - 20, output_row+20-8, 20);

    // STALL(50);

    // dump_vmem_row(3, output_row, 1024);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

/// Takes care of the GnuRadio stream.  These are 64 raw subcarriers in the negative band
/// These do not get sliced.
void __attribute__((always_inline)) handle_grc_stream(const unsigned* const cpu_in) {
    // (void)mover_input_increment_row;

    // note that size is also set in "enabled_subcarriers" which is included by
    // 'stream_mover_64.h'
    // it's actually enabled_subcarriers which sets the length in the header
    // so change it there (it's 64)
    const uint32_t grc_stream_start = 944;


    // original way to overwrite custom subcarrier (second slider in gnuradio)
    // if( 0 ) {
    //     for(unsigned int i = 0; i < number_active_schedules; i++) {
    //       // mover_load_offset_input( &(vmem_schedules[i]), mover_input_increment_row);
    //       // mover_roll(input_frame_count);
    //         mover_load_vmem_offset_input_single( &(vmem_schedules[i]), mover_input_increment_row);
    //       // SET_REG(x3, VMEM_ROW_ADDRESS(&(vmem_schedules[i])));
    //     }

    //     // unsigned int stream_dma_addr = DST_ROW_REV*NSLICES;
    //     unsigned int* stream_cpu_addr = (unsigned int*) REVERSE_VMEM_ROW_ADDRESS( DST_ROW_REV );
    //     unsigned int* original_cpu_addr = (unsigned int*) REVERSE_VMEM_ROW_ADDRESS( SRC_ROW_REV );

    //     stream_cpu_addr[OVERWRITE_CUSTOM_SC_INTO] = original_cpu_addr[custom_subcarrier_index];
    // }
    
    // dma address of input_dma.  This is the dma address of the variable called "input_dma"
    const uint32_t _input_dma = VMEM_DMA_ADDRESS(cpu_in);//VMEM_ROW_ADDRESS_TO_DMA(INPUT_DMA_DMA_ADDRESS);



    // output the header
    // since the length does not change, we can just send the same header every time
    // send the header first and then the data
    stream_default.seq = lifetime_32;
    send_feedback_bus_header(&stream_default);

    bool stamp_stream = false;

    switch( stamp_output_mode ) {
        case 1:
            if((duplex_progress < 4))  {
                stamp_stream = true;
            }
            break;
        case 2:
            if((duplex_progress >= 4) && (duplex_progress < 8))  {
                stamp_stream = true;
            }
            break;
        case 3:
            if(duplex_progress == 0)  {
                stamp_stream = true;
            }
            break;
        case 4:
            if(duplex_progress == 1)  {
                stamp_stream = true;
            }
            break;
        case 5:
            if(duplex_progress == 0 || duplex_progress == 2)  {
                stamp_stream = true;
            }
            break;
        default:
            break;
    }

    bool zero_stream = false;

    // if zero tag is not set, one tag is not checked
    if( ZERO_OUTPUT_OPT_0 ) {
        
        // does the current progress fall within the tagged region? (zero tag)
        bool tag_0 = duplex_tag_section(duplex_progress, ZERO_OUTPUT_OPT_0);
		
        // This holds the value of another tag? (one tag) (0-28)
		uint32_t additional_tag = ZERO_OUTPUT_OPT_1;

        // only lookup if "one tag" is non zero, otherwise shortcut to false
		// this bool holds whether the current progress falls within the tagged region
        bool tag_1 = additional_tag?duplex_tag_section(duplex_progress, additional_tag):false;

        bool tag_both = tag_0 || tag_1;

        zero_stream = !tag_both;  // if we are not in the region, zero
    }

    if( zero_stream ) {
        dma_block_send_finalized(VMEM_DMA_ADDRESS(zero_default_stream), enabled_subcarriers, 1);
    } else {
        if( stamp_stream ) {
            dma_block_send_finalized(VMEM_DMA_ADDRESS(stamped_default_stream), enabled_subcarriers, 1);
        } else {
            // dma_block_send_finalized(DST_ROW_REV*NSLICES, enabled_subcarriers, 1);


            /// Originally we used the reverse mover to move the 64 subcarriers for gnuradio
            /// This meant that we had a separate copy of those subcarriers, making it easy
            /// to overwrite with the custom subcarrier for the second slider in grc
            /// 
            /// Now we do not have a separate copy of the input memory
            /// This means we need to do 3 DMA's in order to get the same output

            uint32_t start0  = _input_dma + grc_stream_start;
            uint32_t length0 = OVERWRITE_CUSTOM_SC_INTO;

            uint32_t start1  = _input_dma + custom_subcarrier_index;
            uint32_t length1 = 1;

            uint32_t start2  = _input_dma + grc_stream_start + OVERWRITE_CUSTOM_SC_INTO + 1;
            uint32_t length2 = enabled_subcarriers - OVERWRITE_CUSTOM_SC_INTO - 1;

            dma_block_send(start0, length0);
            dma_block_send(start1, length1);
            dma_block_send(start2, length2);
        }
    }
}

/// Handles mover and sliced data
/// 
void __attribute__((always_inline)) handle_sliced_data(const unsigned index) {


    // pass true as this is RX Chain
    const bool is_ud = duplex_do_userdata(&duplex, duplex_progress, true);

    if( !is_ud ) {
        return;
    }

    // (void)mover_input_increment_row;
    unsigned occupancy;

    // SET_REG(x4, mover_input_increment_row);



#ifndef DISABLE_SLICE_MOVER

    
    VmemSchedule* active_mover = mover_a;

    if( index != 0 ) {
        active_mover = mover_b;
    }

    // mover_setup(vmem_schedules+4, 0);

    // mover_roll2(vmem_schedules+4, number_active_schedules_data);
    mover_roll2(active_mover, number_active_schedules_data);
    // mover_roll3(vmem_schedules+4, number_active_schedules_data);

    // STALL(50);

    dump_vmem_cpu(1, dst_mem_data, 320);


    if( 0 ) {
        // for(unsigned int i = 0; i < number_active_schedules_data; i++) {
        //     mover_roll_single();
        // }
    }

    if( 0 ) {
        for(unsigned int i = 0; i < number_active_schedules_data; i++) {
            mover_load_vmem_offset_input_single( &(vmem_schedules[i+number_active_schedules]), 0);
        }
    }


#endif

    #ifdef MODE_QPSK_640_LIN
    mover_640_lin(input_row, SLICED_DATA_ROW2);
    #endif
    #ifdef MODE_QAM_16_640_LIN
    mover_640_lin(input_row, SLICED_DATA_ROW2);
    #endif
    #ifdef MODE_QPSK_512_LIN
    mover_512_lin(input_row, SLICED_DATA_ROW2);
    #endif


    vec_demod_data.seq = lifetime_32;
    
    unsigned int slice_input_length;
    unsigned int demod_body_length;
    (void)demod_body_length;

    #ifdef MODE_QPSK_128
    demod_body_length = 8;
    #endif

    #ifdef MODE_QAM_16_128
    demod_body_length = 16;
    slice_input_length = enabled_subcarriers_data;
    #endif

    #ifdef MODE_QAM_16_320
    demod_body_length = 40;
    slice_input_length = enabled_subcarriers_data;
    #endif

    #ifdef MODE_QPSK_320
    demod_body_length = 20;
    slice_input_length = enabled_subcarriers_data;
    #endif

    #ifdef MODE_QPSK_640_LIN
    demod_body_length = 40;
    slice_input_length = 640;
    #endif

    #ifdef MODE_QAM_16_640_LIN
    demod_body_length = 80;
    slice_input_length = 640;
    #endif

    #ifdef MODE_QPSK_512_LIN
    demod_body_length = 32;
    slice_input_length = 512;
    #endif

    // _printf("%s%d\n", "len: ", demod_body_length);

#ifndef DONT_SLICE_DATA
    // sliced
    set_feedback_vector_length(&vec_demod_data, demod_body_length);
#else
    // unsliced
    set_feedback_vector_length(&vec_demod_data, slice_input_length);
#endif

    send_feedback_bus_header(&vec_demod_data);

    SET_REG(x3, 0xaffff);

    // pointer to sliced data
    // note if we are "unsliced" this is still the same
    const unsigned int sliced_data_dma = 
        ( index == 0 ) ? VMEM_DMA_ADDRESS(dst_mem_data_a) : VMEM_DMA_ADDRESS(dst_mem_data_b);


    // wait until all dma out transations are finished
    while(1) {
      CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
      if( occupancy == 0 ) {
        break;
      }
    }

    // if( demod_mode != 0 ) {
        SET_REG(x3, 0x0ffff);
    // }




#ifndef DONT_SLICE_DATA
    // sliced
    dma_block_send_sliced(sliced_data_dma, slice_input_length, 1, demod_mode);
#else
    // unsliced
    dma_block_send_finalized(sliced_data_dma, slice_input_length, 1);
#endif

    // if( demod_mode != 0 ) {
        SET_REG(x3, 0x1ffff);
    // }

    // wait till it is also finished
    while(1) {
      CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
      if( occupancy == 0 )  {
        break;
      }
    }

    // if( demod_mode != 0 ) {
        SET_REG(x3, 0x2ffff);
    // }

    // back to normal for operations after this
    CSR_WRITE(DEMAPPER_CONSTELLATION, FEEDBACK_MAPMOV_QPSK);
}



void __attribute__((always_inline)) handle_fine_sync(const unsigned* cpu_in) {
        // now we need to sent the header for fine sync
    // this comes from the pre-moved buffer (could this also come before mover for efficieny?)
    vec_fine_sync.seq = lifetime_32;
    send_feedback_bus_header(&vec_fine_sync);
    // dma_out_set(  VMEM_DMA_ADDRESS(&(input_dma[doff]))  , FINE_SYNC_LENGTH);
    dma_block_send_finalized(  VMEM_DMA_ADDRESS(cpu_in) + TRUNK_OFFSET, FINE_SYNC_LENGTH, 1);

    const uint32_t rolling_sc = 3;
    const uint32_t pick_rolling_sc = vector_memory[VMEM_DMA_ADDRESS(cpu_in) + rolling_sc];

    rolling_sc_vmem[0] = rolling_sc;
    rolling_sc_vmem[1] = pick_rolling_sc;

    dma_block_send_finalized( VMEM_DMA_ADDRESS(rolling_sc_vmem), FINE_SYNC_EXTRA, 1);
}


static uint32_t eq_frame_duplex_progress = 0;
static uint32_t eq_frame_tag_divisor = 7;
static uint32_t eq_frame_tag_divisor_offset = 0;

// handles a full OFDM frame, which we previously used for eq
void __attribute__((always_inline)) handle_full_eq_frame(const unsigned int input_dma) {

    if( unlikely(duplex_progress == eq_frame_duplex_progress) ) {

        if( ((lifetime_32 / DUPLEX_FRAMES)+eq_frame_tag_divisor_offset) % eq_frame_tag_divisor == 0 ) {
            // send header
            stream_all_sc.seq = lifetime_32;
            send_feedback_bus_header(&stream_all_sc);

            // const bool use_real_data = true;
            // if( use_real_data ) {
            dma_block_send(input_dma, 1024); 
            // } else {
            //     dma_block_send(VMEM_DMA_ADDRESS(counter_data), 1024);
            // }
        }
    } else {
        // do nothing
    }
}


#pragma GCC diagnostic pop


// should we feedback the d-engine corrected vector?
uint32_t feedback_dengine_corrected = 1;


unsigned do_work(
                const unsigned index,
                const unsigned* const cpu_in,
                      unsigned* const cpu_out
               ) {

    (void)cpu_out;

    if( 1 ) {
        for(unsigned i = 0; i < stall_check; i++) {
            STALL(5);
        }
    }

    const unsigned int input_dma = VMEM_DMA_ADDRESS(cpu_in);
    // (void)input_row;

    lifetime_32 = cpu_in[TRUNK_OFFSET + TRUNK_FRAME_COUNTER];

#ifdef DEBUG_TEST25_SEND_ALL_RB
    ring_block_send_eth(lifetime_32);
#endif

    duplex_progress = lifetime_32 % DUPLEX_FRAMES;

    duplex_mode = get_duplex_mode(&duplex, duplex_progress, lifetime_32);
    duplex_mode_rx = get_duplex_mode(&duplex_rx, duplex_progress, lifetime_32);

    forward_eq = duplex_should_forward_eq_data(&duplex, duplex_progress, cooked_data_type);  // use duplex as it may be tx or rx


    
    if( unlikely(forward_eq) ) {

        uint32_t feedback_dma = input_dma;

        if( likely(feedback_dengine_corrected) ) {
            feedback_dma += 1024; // use d-engine correction
        }

        if( duplex.role == DUPLEX_ROLE_RX ) {
            // we are the receiver. the transmitter is sending us pilot tones "p"
            vec_eq_feedback_analog.seq = lifetime_32;
            send_feedback_bus_header(&vec_eq_feedback_analog);
            dma_block_send_finalized( feedback_dma, FFT_SIZE, 1);
        } else {
            // we are the transmitter, the receiver has sent us corrections

            vec_eq_feedback_correction.seq = lifetime_32;
            send_feedback_bus_header(&vec_eq_feedback_correction);
            dma_block_send_finalized( feedback_dma, FFT_SIZE, 1);
        }
    }
    

    // SET_REG(x4, doff);

    // SET_REG(x3, input_dma[0+doff]);
    // SET_REG(x3, input_dma[1+doff]);
    // SET_REG(x3, input_dma[2+doff]);
    // SET_REG(x3, input_dma[3+doff]);
    // SET_REG(x3, input_dma[1024+doff]);
    // SET_REG(x3, input_dma[1025+doff]);
    // SET_REG(x3, input_dma[1026+doff]);
    // SET_REG(x3, input_dma[1027+doff]);
    // SET_REG(x3, input_dma[1028+doff]);


    // PET full ofdm frame output
    // WE run this before the mover because
    //   1) mover does not modify input
    //   2) the output DMA will run while we are moving in parallel
    handle_full_eq_frame(input_dma);
    
    handle_grc_stream(cpu_in);

    handle_fine_sync(cpu_in);

    handle_sliced_data(index);


    count_out++;

    // dma_in_full = 1;
    // dma_out_done = 1;



    // BUMP frame track counter
    // frame_track_counter++;
  CSR_WRITE(GPIO_WRITE, (0x200000) | 2);
    return 1;
}

void feedback_bus_callback(const unsigned int data) {
    if( data == 0 ) {
        dma_block_send(VMEM_DMA_ADDRESS(&all_zeros), ALL_ZERO_LENGTH);
    }
}

void update_demod_mode_callback(const unsigned int data) {
    demod_mode = data;
}

void update_custom_subcarrier_index_callback(const unsigned int data) {
    if( data >= 1024 ) {
        ring_block_send_eth(APP_ASSERT_PCCMD | (OUR_RING_ENUM)<<24 | 1 );
        return;
    }

    custom_subcarrier_index = data;
}

void stamp_callback(const unsigned int data) {
    stamp_output_mode = data;
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
            p = &zero_output_mode;
            break;
        case 2:
            p = &stall_check;
            break;
        case 3:
            p = &feedback_dengine_corrected;
            break;
        case 4:
            p = &eq_frame_duplex_progress;
            break;
        case 5:
            p = &eq_frame_tag_divisor;
            break;
        case 6:
            p = &eq_frame_tag_divisor_offset;
            break;
        case 10:
            p = (uint32_t*) &duplex.role;
            break;
        default:
            break;
    }
    return p;
}

void generic_op_finished(const uint32_t sel, const uint32_t op, const uint32_t value ) {
    (void)op;
    (void)value;
    switch(sel) {
        case 4:
            if( eq_frame_duplex_progress > DUPLEX_FRAMES ) {
                eq_frame_duplex_progress = 0;
            }
            break;
        case 5:
            if( eq_frame_tag_divisor == 0 ) {
                eq_frame_tag_divisor = 7; // default
            }
            break;
        case 10:
            update_duplex_role(&duplex);
            break;
    }
}



void setup_duplex(void) {
    init_duplex(&duplex, DUPLEX_ROLE_RX);
    init_duplex(&duplex_rx, DUPLEX_ROLE_RX);
}

void setup_pointers(void) {
}


int main2(void);
int main(void)
{
    self_sync_block_boot();
    main2();
    return 0;
}
int main2(void) {
  // unsigned int rtn;
  Ringbus ringbus;

#ifdef FLUSH_AT_START

  CSR_WRITE(DMA_0_FLUSH_SCHEDULE, 0);
  CSR_WRITE(DMA_1_FLUSH_SCHEDULE, 0);
  CSR_WRITE(DMA_2_FLUSH_SCHEDULE, 0);

#endif

    ping_pong_set_callback(&do_work);
    setup_ping_pong();

  // uint32_t* a;
  // a = (uint32_t*) (vmem_schedule_predicate + 0);

  // SET_REG(x3, *a);

  // a = (uint32_t*) (vmem_schedule_predicate + 1);

  // SET_REG(x3, *a);

  //   a = (uint32_t*) (vmem_schedule_predicate + 2);

  // SET_REG(x3, *a);

  // setup vmalloc
  // init_VMalloc(&mgr);

  // setup callbacks
  ring_register_callback(&feedback_bus_callback, FEEDBACK_BUS_CMD);
  ring_register_callback(&check_bootload_status, CHECK_BOOTLOAD_CMD);
  ring_register_callback(&update_custom_subcarrier_index_callback, RX_CHOOSE_CUSTOM_SC_CMD);
  ring_register_callback(&handle_generic_callback_original, GENERIC_OPERATOR_CMD);
  ring_register_callback(&stamp_callback, STAMP_STREAM_OUTPUT_CMD);
  ring_register_callback(&update_demod_mode_callback, RX_DEMOD_MODE);
  ring_register_callback(&cooked_data_type_callback, COOKED_DATA_TYPE_CMD);

    handle_generic_register_post_callback(&generic_op_finished);
    handle_generic_register_get_pointer(&pointer_for_generic_op);
    handle_generic_register_ring(&ring_block_send_eth);

  // unsigned int* input_cpu_ptr =  vmalloc_single(&mgr);
  // unsigned int input_dma_ptr = VMEM_DMA_ADDRESS(input_cpu_ptr);

  // mover_working_on = 0;

  // unsigned int a, b, c, d;

#ifdef FLUSH_AT_START
  CSR_WRITE(DMA_0_FLUSH_SCHEDULE,  0);
  CSR_WRITE(DMA_1_FLUSH_SCHEDULE,  0);
#endif

#ifdef MODE_QPSK_128
  demod_mode = FEEDBACK_MAPMOV_QPSK;
#endif
#ifdef MODE_QAM_16_128
  demod_mode = FEEDBACK_MAPMOV_QAM16;
#endif
#ifdef MODE_QAM_16_320
  demod_mode = FEEDBACK_MAPMOV_QAM16;
#endif
#ifdef MODE_QPSK_320
  demod_mode = FEEDBACK_MAPMOV_QPSK;
#endif
#ifdef MODE_QAM_16_640_LIN
  demod_mode = FEEDBACK_MAPMOV_QAM16;
#endif
  

  setup_debug();

  setup_duplex();
  setup_pointers();

  // for(unsigned int i = 0; i < 1024; i++) {
  //   vector_memory[i+input_dma_ptr] = 0xf000d000 + i;
  // }

  // setup mapper
  // setup_mapper();

  setup_mover();
  setup_mover_post(); // must be called afer previous

  // setup_dma_in();

  setup_feedback_bus();

  // default output stream length is set in setup_mover()
  set_stream_length(enabled_subcarriers);
  // setup_dma_out();
  // setup_fill_level();



  CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);

    while(1) {
        execute_ping_pong();
        check_ring(&ringbus);
    }
    return 0;
}
