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


// #define ENABLE_TB_DEBUG

#ifndef ENABLE_TB_DEBUG
#define DISABLE_TB_DEBUG
#endif
#include "tb_debug.h"



#define NORMAL_OPERATION

#ifdef NORMAL_OPERATION

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

#include "vmalloc.h"
// declare as global
// VMalloc mgr;

// #define OUTPUT_FRAME_COUNT_WORST_CASE (40)
#define FFT_SIZE (1024)


unsigned int custom_subcarrier_index = 41;


unsigned int count_in = 0;
unsigned int count_out = 0;


unsigned int frame_num_all_output_period = 1024; //packet_num_SFO_adjustment_period
unsigned int frame_num_all_output_counter = 0;

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

#define ALL_ZERO_LENGTH (367*2)
VMEM_SECTION unsigned int all_zeros[ALL_ZERO_LENGTH];
#include "cs21_use_counter_data.h"

void setup_feedback_bus(void) {
    init_feedback_stream(&stream_default, FEEDBACK_PEER_SELF, false, true, FEEDBACK_STREAM_DEFAULT);
    init_feedback_stream(&stream_all_sc, FEEDBACK_PEER_SELF, false, true, FEEDBACK_STREAM_ALL_SC);

    init_feedback_vector(&vec_fine_sync, FEEDBACK_PEER_SELF, false, true, FEEDBACK_VEC_FINE_SYNC);
    init_feedback_vector(&vec_demod_data, FEEDBACK_PEER_SELF, false, true, FEEDBACK_VEC_DEMOD_DATA);

    // NEVER directly set the length, the length field is not simply the length of data
    set_feedback_vector_length(&vec_fine_sync, FINE_SYNC_LENGTH+FINE_SYNC_EXTRA);
}

void set_stream_length(unsigned int len) {
    SET_REG(x3, 0xface);
    SET_REG(x3, len);
    set_feedback_stream_length(&stream_default, len);
    set_feedback_stream_length(&stream_all_sc, 1024); // length is all subcarriers
}


#define DST_ROW_REV (VMEM_ROW_ADDRESS(dst_mem))
#define DST_ROW_REV2 (VMEM_ROW_ADDRESS(dst_mem_data))

unsigned int mapper_output_row;

unsigned int demod_mode = 0;


// int mover_working_on;

// FIXME lame way of doing this
#define MAX_SCHEDULE_COUNT 44

#define GARBAGE_ROW       (garbage_row)

// in the reverse mover, DST_ROW is actually the source

#define SRC_ROW_REV           (VMEM_ROW_ADDRESS(input_dma))

unsigned int garbage_row;
unsigned int dma_in_full = 0;
unsigned int dma_out_done = 0;


#ifdef USE_OLD_MOVER_SCHEDULE_FORMAT
Schedule schedules[MAX_SCHEDULE_COUNT][NSLICES];
#else
// Directly create this in vmem (we could also load this at compile time with a compile time change to the value of DST_ROW)
VMEM_SECTION VmemSchedule vmem_schedules[MAX_SCHEDULE_COUNT];
#endif


// how many fft's to move at a time
// must be the same as DMA_IN_CHUNKS
#define FRAME_MOVE_CHUNK (1)

// the 2 is for ping/pong
VMEM_SECTION unsigned int input_dma[(FFT_SIZE+DMA_IN_EXTRA)*FRAME_MOVE_CHUNK*2] = {};

// worst case 1024 enabled subcarriers
VMEM_SECTION unsigned int dst_mem[FFT_SIZE*FRAME_MOVE_CHUNK*2] = {};
VMEM_SECTION unsigned int dst_mem_data[FFT_SIZE*FRAME_MOVE_CHUNK*2] = {};
VMEM_SECTION unsigned int garbage_mem[16] = {};

// the 2 is for ping/pong
VMEM_SECTION unsigned int empty_row[16] = {};
VMEM_SECTION unsigned int empty[1] = {0xcafebabe};

#include "cs21_example_data.h"

unsigned int enabled_subcarriers; // delcared here but SET BY OUTPUT FROM schedule_maker.py
unsigned int number_active_schedules; // same as previous
unsigned int enabled_subcarriers_data; // delcared here but SET BY OUTPUT FROM schedule_maker.py
unsigned int number_active_schedules_data; // same as previous

unsigned int dma_in_dma_ptr;

VMEM_SECTION unsigned int rolling_sc_vmem[FINE_SYNC_EXTRA];

void setup_mover(void) {
  garbage_row = VMEM_ROW_ADDRESS(garbage_mem);

  dma_in_dma_ptr = VMEM_DMA_ADDRESS(input_dma);





#include "stream_mover_64.h"




#ifdef MODE_QAM_16_128
#include "slicer_mover_128.h"
#endif

#ifdef MODE_QAM_16_320
#include "slicer_mover_320.h"
#endif

#ifdef MODE_QPSK_320
#include "slicer_mover_320.h"
#endif

}

unsigned int input_frame_count;

#ifdef USE_DOUBLE_BUFFER
unsigned int mover_output_increment_words;
unsigned int mover_output_increment_row;
#endif
// call after setup_mover
void setup_mover_post(void) {
  input_frame_count = FRAME_MOVE_CHUNK;

#ifdef USE_DOUBLE_BUFFER
  // bumps for our "a" / "b" buffers
  mover_output_increment_words = input_frame_count << 10; // times 1024
  mover_output_increment_row = mover_output_increment_words >> 4; // over 16
#endif
}




// must be the same as FRAME_MOVE_CHUNK
#define DMA_IN_CHUNKS (1)

// in words
#define DMA_IN_SIZE ( (FFT_SIZE+DMA_IN_EXTRA)*DMA_IN_CHUNKS)

#define DMA_IN_CIRBUF_SIZE (DMA_IN_CHUNKS+1)
circular_buf_t dma_in_buffer;
unsigned int dma_in_buffer_storage[DMA_IN_CIRBUF_SIZE];

// setting this to 5 means buffer can hold 4
#define DMA_SCHEDULE_IN_SIZE (4+1)
circular_buf_t dma_schedule_in;
unsigned int dma_schedule_in_storage[DMA_SCHEDULE_IN_SIZE];

// #define DMA_SCHEDULE_OUT_SIZE (4+1)
// circular_buf_t dma_schedule_out;
// unsigned int dma_schedule_out_storage[DMA_SCHEDULE_OUT_SIZE];

unsigned int dma_trig_next = 0;
unsigned int dma_trig_previous = 0;
// converts a dma index (used in the cirbufs) to a dma_ptr
// the dma index counts each block of memory
unsigned int dma_idx_to_ptr(const unsigned int idx) {
  return dma_in_dma_ptr + (idx * DMA_IN_SIZE);
}

// convert a dma_ptr to an index
unsigned int dma_ptr_to_idx(const unsigned int ptr) {
  return (ptr - dma_in_dma_ptr) / DMA_IN_SIZE;
}

void trig_dma_in(const unsigned int dma_ptr) {
  // ring_block_send_eth(dma_ptr);
  CSR_WRITE(DMA_0_START_ADDR, dma_ptr);
  CSR_WRITE(DMA_0_LENGTH, DMA_IN_SIZE);
  CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff); // start right away
  CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);
  count_in++;
}

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


void trig_dma_in_next(void) {
    SET_REG(x3, 0xc0000000);

    trig_dma_in(dma_idx_to_ptr(dma_trig_next));

    circular_buf_put(&dma_schedule_in, dma_trig_next);

    dma_trig_previous = dma_trig_next;
    dma_trig_next = (dma_trig_next+1) % DMA_IN_CHUNKS;
}
//////////////////////////////////////////
//
// We run 2 circular buffers2
// the first buffer keeps track of outstanding input dma so they are always overlapping
// as these they dump into the 2nd circular buffer which is the "pending data" and also our fill level

void setup_dma_in(void) {
  // dma_in_dma_ptr = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));

  circular_buf_initialize(&dma_schedule_in, dma_schedule_in_storage, DMA_SCHEDULE_IN_SIZE);
  // circular_buf_initialize(&dma_in_buffer, dma_in_buffer_storage, DMA_IN_CIRBUF_SIZE);

  dma_in_full = 0;
  trig_dma_in_next();
}

// void setup_dma_out(void) {
//   circular_buf_initialize(&dma_schedule_out, dma_schedule_out_storage, DMA_SCHEDULE_OUT_SIZE);
// }

// unsigned int fake_work_todo = 0;

void recover_last(void) {
    ring_block_send_eth(DMA_LAST_ERROR_PCCMD | OUR_RING_ENUM);
    dma_run_till_last();
}

void pet_dma_inqueue(void) {
    int error;
    unsigned int just_finished_idx;
    unsigned int dma_occupancy;
    unsigned int status;
    // unsigned int dma_out_occupancy;

    CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, dma_occupancy);
    // CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, dma_out_occupancy);
    unsigned int filled = circular_buf_occupancy(&dma_schedule_in);

    // unsigned int outgoing_buf_occupancy;

    SET_REG(x3, 0xe0000000 | count_in );
    SET_REG(x3, 0xf0000000 | count_out );

    if( dma_in_full == 0 ) {
        if(dma_occupancy != filled) {
            CSR_READ(DMA_0_STATUS, status);
            if( status ) {
                recover_last();
            }

            dma_in_full = 1;
            SET_REG(x3, 0xbb000000);

            return;
        }
    }

  if( dma_occupancy != filled && dma_out_done == 1) {
    // just_finished_idx is the index of the dma that just finished
    error = circular_buf_get(&dma_schedule_in, &just_finished_idx); MY_ASSERT(error == 0);

    // now that dma is done with this chunk, we add it to the next
    // circular buffer which signals the program that there is fresh data to be processed
    // circular_buf_put(&dma_in_buffer, just_finished_idx);
    // ring_block_send_eth(dma_occupancy);
    // ring_block_send_eth(filled);

    // outgoing_buf_occupancy = circular_buf_occupancy(&dma_in_buffer);

    SET_REG(x3, 0xb0000000 | 1);

    // fake_work_todo += 10;


    // ring_block_send_eth(data);
    dma_in_full = 0;
    dma_out_done = 0;
    trig_dma_in_next();
  }

}


void pet_dma_in(void) {
  pet_dma_inqueue();
}

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


//unsigned int frame_track_counter;
unsigned int trunk_frame_count;
void setup_frame_tracking(void) {
    // frame_track_counter = 0;
    trunk_frame_count = 0;
}

// mover takes care of scheduling outputs
// because it runs so much faster we don't seem to have an output dma task
void pet_mover(void) {
  // SET_REG(x3, 0x2);

  unsigned int dma_idx_just_finished;
  // unsigned int incomming_occupancy;
  unsigned int outgoing_dma_occupancy;
  // int error;
  int occupancy;

  // incomming_occupancy = circular_buf_occupancy(&dma_in_buffer);
  CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, outgoing_dma_occupancy);

  if(dma_in_full && outgoing_dma_occupancy == 0) {
    // error = circular_buf_get(&dma_in_buffer, &dma_idx_just_finished); MY_ASSERT(error == 0);

    dma_idx_just_finished = 0;

    SET_REG(x3, 0xa0000000 | dma_idx_just_finished);
    // SET_REG(x4, 0xcafe);
    // SET_REG(x4, dma_idx_just_finished);

    // index of first word
    const unsigned int doff = (dma_idx_just_finished * (FFT_SIZE+DMA_IN_EXTRA) * FRAME_MOVE_CHUNK);

    const unsigned int input_row = VMEM_DMA_ADDRESS_TO_ROW(VMEM_DMA_ADDRESS(input_dma)+doff);
    (void)input_row;

    trunk_frame_count = vector_memory[dma_idx_to_ptr(dma_trig_previous) +
                                            FFT_SIZE +
                                            TRUNK_FRAME_COUNTER];

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
    if( frame_num_all_output_counter == frame_num_all_output_period ) {
        // send header
        stream_all_sc.seq = trunk_frame_count;
        send_feedback_bus_header(&stream_all_sc);

        const bool use_real_data = true;
        if( use_real_data ) {
            dma_block_send(  VMEM_DMA_ADDRESS(input_dma)+doff, 1024); 
        } else {
            dma_block_send(  VMEM_DMA_ADDRESS(counter_data), 1024); 

        }

        // dma_block_send(  VMEM_DMA_ADDRESS(&(input_dma[0+doff]))  , 1024); 
        // is index in doff words from the beginning of dma_in_dma_ptr
        frame_num_all_output_counter = 0;
    } else {
        // do nothing
    }
    frame_num_all_output_counter++;

    ///////
    //
    //  Run Mover
    //

    // input is 1 fft's at once
    const unsigned int mover_input_increment_row = (dma_idx_just_finished * (FFT_SIZE+DMA_IN_EXTRA) * FRAME_MOVE_CHUNK) / NSLICES;

    SET_REG(x4, mover_input_increment_row);

    for(unsigned int i = 0; i < number_active_schedules; i++) {
      // mover_load_offset_input( &(vmem_schedules[i]), mover_input_increment_row);
      // mover_roll(input_frame_count);
        mover_load_vmem_offset_input_single( &(vmem_schedules[i]), mover_input_increment_row);
      // SET_REG(x3, VMEM_ROW_ADDRESS(&(vmem_schedules[i])));
    }

#ifndef DISABLE_SLICE_MOVER
    for(unsigned int i = 0; i < number_active_schedules_data; i++) {
      // mover_load_offset_input( &(vmem_schedules[i+4]), mover_input_increment_row);
      // mover_roll(input_frame_count);
        mover_load_vmem_offset_input_single( &(vmem_schedules[i+number_active_schedules]), mover_input_increment_row);
      // SET_REG(x3, VMEM_ROW_ADDRESS(&(vmem_schedules[i+4])));
      // SET_REG(x3, DST_ROW_REV);
    }
#endif

    #ifdef MODE_QPSK_640_LIN
    mover_640_lin(input_row, DST_ROW_REV2);
    #endif
    #ifdef MODE_QAM_16_640_LIN
    mover_640_lin(input_row, DST_ROW_REV2);
    #endif
    #ifdef MODE_QPSK_512_LIN
    mover_512_lin(input_row, DST_ROW_REV2);
    #endif

    //
    ///////////

    ///////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    // unsigned int count1 = frame_track_counter % SCHEDULE_FRAMES;
    // unsigned int progress = count1 % SCHEDULE_LENGTH;

    // if(progress == 0) {
        
    // }
    ///////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

#ifdef OVERWRITE_CUSTOM_SC_INTO
    // unsigned int stream_dma_addr = DST_ROW_REV*NSLICES;
    unsigned int* stream_cpu_addr = (unsigned int*) REVERSE_VMEM_ROW_ADDRESS( DST_ROW_REV );
    unsigned int* original_cpu_addr = (unsigned int*) REVERSE_VMEM_ROW_ADDRESS( SRC_ROW_REV );

    stream_cpu_addr[OVERWRITE_CUSTOM_SC_INTO] = original_cpu_addr[custom_subcarrier_index];
#endif



    // output the header
    // since the length does not change, we can just send the same header every time
    // send the header first and then the data
    stream_default.seq = trunk_frame_count;
    send_feedback_bus_header(&stream_default);
    dma_block_send_finalized(DST_ROW_REV*NSLICES, enabled_subcarriers*FRAME_MOVE_CHUNK, 1);

    // now we need to sent the header for fine sync
    // this comes from the pre-moved buffer (could this also come before mover for efficieny?)
    vec_fine_sync.seq = trunk_frame_count;
    send_feedback_bus_header(&vec_fine_sync);
    // dma_out_set(  VMEM_DMA_ADDRESS(&(input_dma[doff]))  , FINE_SYNC_LENGTH);
    dma_block_send_finalized(  VMEM_DMA_ADDRESS(input_dma) + doff+FFT_SIZE, FINE_SYNC_LENGTH, 1);

    const uint32_t rolling_sc = 3;
    const uint32_t pick_rolling_sc = vector_memory[VMEM_DMA_ADDRESS(input_dma) + doff + rolling_sc];

    rolling_sc_vmem[0] = rolling_sc;
    rolling_sc_vmem[1] = pick_rolling_sc;

    dma_block_send_finalized( VMEM_DMA_ADDRESS(rolling_sc_vmem), FINE_SYNC_EXTRA, 1);

    vec_demod_data.seq = trunk_frame_count;
    
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

    // wait until all dma out transations are finished
    while(1) {
      CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
      if( occupancy == 0) {
        break;
      }
    }

    if( demod_mode != 0 ) {
        SET_REG(x3, 0x0ffff);
    }



    // pointer to sliced data
    // note if we are "unsliced" this is still the same
    unsigned int sliced_data_dma = DST_ROW_REV2*NSLICES;

#ifndef DONT_SLICE_DATA
    // sliced
    dma_block_send_sliced(sliced_data_dma, slice_input_length, 1, demod_mode);
#else
    // unsliced
    dma_block_send_finalized(sliced_data_dma, slice_input_length, 1);
#endif

    if( demod_mode != 0 ) {
        SET_REG(x3, 0x1ffff);
    }

    // wait till it is also finished
    while(1) {
      CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
      if( occupancy == 0) {
        break;
      }
    }

    if( demod_mode != 0 ) {
        SET_REG(x3, 0x2ffff);
    }

    // back to normal for operations after this
    CSR_WRITE(DEMAPPER_CONSTELLATION, FEEDBACK_MAPMOV_QPSK);

    if( demod_mode != 0 ) {
        SET_REG(x3, 0x3ffff);
    }


// #endif
    // for (int i = 0; i < TEST_DATA_LENGTH/2; ++i)
    // {
    //     dma_block_send_sliced(1+DST_ROW_REV*NSLICES+2*i, 1, 1);
    // }
    
    // Testing
    // for (int i = 0; i < TEST_DATA_LENGTH/2; ++i)
    // {
    //     dma_block_send_sliced(1+VMEM_DMA_ADDRESS(example_data)+2*i, 1, 1);
    // }

    // dma_block_send_sliced(DST_ROW_REV*NSLICES, TEST_DATA_LENGTH, 1);
    //Testing
    // dma_block_send_sliced(VMEM_DMA_ADDRESS(example_data), TEST_DATA_LENGTH, 1);

    unsigned int occupancy2;
    while(1) {
        CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy2);
        if( occupancy2 == 0) {
            break;
        }
    }

    count_out++;

    // dma_in_full = 1;
    dma_out_done = 1;



    // BUMP frame track counter
    // frame_track_counter++;
  }
  CSR_WRITE(GPIO_WRITE, (0x200000) | 2);
    
}

void feedback_bus_callback(unsigned int data) {
    if( data == 0 ) {
        dma_block_send(VMEM_DMA_ADDRESS(&all_zeros), ALL_ZERO_LENGTH);
    }
}

void update_demod_mode_callback(unsigned int data) {
    demod_mode = data;
}

void update_custom_subcarrier_index_callback(unsigned int data) {
    if( data >= 1024 ) {
        ring_block_send_eth(APP_ASSERT_PCCMD | (OUR_RING_ENUM)<<24 | 1 );
        return;
    }

    custom_subcarrier_index = data;
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

  // setup vmalloc
  // init_VMalloc(&mgr);

  // setup callbacks
  ring_register_callback(&feedback_bus_callback, FEEDBACK_BUS_CMD);
  ring_register_callback(&update_demod_mode_callback, RX_DEMOD_MODE);
  ring_register_callback(&check_bootload_status, CHECK_BOOTLOAD_CMD);
  ring_register_callback(&update_custom_subcarrier_index_callback, RX_CHOOSE_CUSTOM_SC_CMD);

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

  // for(unsigned int i = 0; i < 1024; i++) {
  //   vector_memory[i+input_dma_ptr] = 0xf000d000 + i;
  // }

  // setup mapper
  // setup_mapper();

  setup_mover();
  setup_mover_post(); // must be called afer previous

  setup_dma_in();

  setup_feedback_bus();

  // default output stream length is set in setup_mover()
  set_stream_length(enabled_subcarriers*FRAME_MOVE_CHUNK);
  // setup_dma_out();
  // setup_fill_level();

  setup_frame_tracking();


  CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);

  while(1) {
    pet_dma_in();

    pet_mover();

    check_ring(&ringbus);

    SET_REG(x3, 0x0);
  }
  return 0;
}

#else 

#define STREAM_CHUNK (64)

#include "do_forward_stream.h"

int main(void)
{
  no_exit_stream();
}




#endif