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
#include "random.h"
#include "subtract_timers.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "vmalloc.h"
// declare as global
VMalloc mgr;

////////////////////////////////
//
// output of the mapper needs x * 1024 words of memory in a row
// this number is calculated by: 
//   input_word_chunk = 16
//   bits_per_word = 32
//   bits_per_symbol = 2
//   enabled_subcarriers = 8
// formula:
//   input_word_chunk * bits_per_word / bits_per_symbol / enabled_subcarriers
//   16 * 32 / 2 / 8
//
#define OUTPUT_FRAME_COUNT_WORST_CASE (40)
#define FFT_SIZE (1024)

#define OUR_RING_ENUM RING_ENUM_CS20
#define ETH_RING_ENUM      (6)


#define TRUE               (0x1)
#define FALSE              (0x0)

#define INIT_STATE         (0x0)
#define RX_STATE           (0x1)
#define MAP_STATE          (0x2)
#define MOVE_STATE         (0x3)
#define TX_STATE           (0x4)
#define WAITING_STATE      (0x5)
#define FINISH_STATE       (0x6)

// normal operation
// accept words, map to bpsk, move to subcarriers, output

// modes
// accepts words, map to "debug bpsk" (values of 0,1,2,3), move to subcarriers, output
// #define USE_FAKE_BPSK

// ignore input, map counter values to subcarriers, output
// when this is set, USE_FAKE_BPSK, has no effect
// #define USE_FAKE_MOVER_INPUT

// prn mode
#define USE_FAKE_MAPPER_INPUT

// controlls which style of schedule is consumed
// simply enabling this is not enough, setup_mover() should also be edited
// old means dmem schedule and mover_schedule()
// new means vmem schedule and mover_load_vmem()
// #define USE_OLD_MOVER_SCHEDULE_FORMAT


// might only be valid when USE_OLD_MOVER_SCHEDULE_FORMAT is not enabled
// disabling this means we will wait for every dma output to complete before scheduling
// the next
// #define USE_DOUBLE_BUFFER


// Gain
// Put a positive 16 bit signed number for qpsk gain
#define QPSK_GAIN (0x1000)

#define MY_ASSERT(x) if(!(x)) { ring_block_send_eth(0xe0000000|__LINE__);}



// register volatile unsigned int x3 asm("x3");
// register volatile unsigned int x4 asm("x4");


Table qpsk_table;
Table bpsk_table;

#define MOVER_SRC_ROW          (mapper_output_row)
#define MAPPER_DEST_ROW        (mapper_output_row)

unsigned int mapper_output_row;

unsigned int qpsk_table_dma;
unsigned int bpsk_table_dma;


void setup_mapper(void) {
  qpsk_table_dma = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));
#ifdef USE_FAKE_BPSK
  qpsk_table = mapper_debug_qpsk_table(qpsk_table_dma);
#else
  qpsk_table = mapper_qpsk_table_gain(qpsk_table_dma, QPSK_GAIN);
#endif
  
  bpsk_table_dma = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));
  bpsk_table = mapper_bpsk_table(bpsk_table_dma);

  // output of mapper, input to mover
  mapper_output_row = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
}


int mover_working_on;

// FIXME lame way of doing this
#define MAX_SCHEDULE_COUNT 16

#define GARBAGE_ROW       (garbage_row)
#define SCRATCH_DMA       (scratch_dma_ptr)
#define DST_ROW           (VMEM_ROW_ADDRESS(mover_output))

unsigned int garbage_row;


#ifdef USE_OLD_MOVER_SCHEDULE_FORMAT
Schedule schedules[MAX_SCHEDULE_COUNT][NSLICES];
#else
// Directly create this in vmem (we could also load this at compile time with a compile time change to the value of DST_ROW)
VMEM_SECTION VmemSchedule vmem_schedules[MAX_SCHEDULE_COUNT];
#endif

// mover_output worst case (when we have the fewest enabled subcarriers)
// we map 256 at a time, so more subacciers means we have less output
// (higher mapped number means we pack inputs more densly in output)

VMEM_SECTION unsigned int mover_output[FFT_SIZE*OUTPUT_FRAME_COUNT_WORST_CASE] = {};

unsigned int enabled_subcarriers; // delcared here but SET BY OUTPUT FROM schedule_maker.py
unsigned int number_active_schedules; // same as previous

void setup_mover(void) {
  garbage_row = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));

// Total Subcarrier #: 256








//   Bins:
// 00 (16): [16, 32, 48, 64, 80, 96, 112, 128, 880, 896, 912, 928, 944, 960, 976, 992]
// 01 (16): [17, 33, 49, 65, 81, 97, 113, 129, 881, 897, 913, 929, 945, 961, 977, 993]
// 02 (16): [18, 34, 50, 66, 82, 98, 114, 130, 882, 898, 914, 930, 946, 962, 978, 994]
// 03 (16): [19, 35, 51, 67, 83, 99, 115, 131, 883, 899, 915, 931, 947, 963, 979, 995]
// 04 (16): [20, 36, 52, 68, 84, 100, 116, 132, 884, 900, 916, 932, 948, 964, 980, 996]
// 05 (16): [21, 37, 53, 69, 85, 101, 117, 133, 885, 901, 917, 933, 949, 965, 981, 997]
// 06 (16): [22, 38, 54, 70, 86, 102, 118, 134, 886, 902, 918, 934, 950, 966, 982, 998]
// 07 (16): [23, 39, 55, 71, 87, 103, 119, 135, 887, 903, 919, 935, 951, 967, 983, 999]
// 08 (16): [24, 40, 56, 72, 88, 104, 120, 136, 888, 904, 920, 936, 952, 968, 984, 1000]
// 09 (16): [25, 41, 57, 73, 89, 105, 121, 137, 889, 905, 921, 937, 953, 969, 985, 1001]
// 10 (16): [26, 42, 58, 74, 90, 106, 122, 138, 890, 906, 922, 938, 954, 970, 986, 1002]
// 11 (16): [27, 43, 59, 75, 91, 107, 123, 139, 891, 907, 923, 939, 955, 971, 987, 1003]
// 12 (16): [28, 44, 60, 76, 92, 108, 124, 140, 892, 908, 924, 940, 956, 972, 988, 1004]
// 13 (16): [29, 45, 61, 77, 93, 109, 125, 141, 893, 909, 925, 941, 957, 973, 989, 1005]
// 14 (16): [30, 46, 62, 78, 94, 110, 126, 142, 894, 910, 926, 942, 958, 974, 990, 1006]
// 15 (16): [31, 47, 63, 79, 95, 111, 127, 143, 895, 911, 927, 943, 959, 975, 991, 1007]
// Longest bin: 16

// Schedule usage
// 00: 100.00 %
// 01: 100.00 %
// 02: 100.00 %
// 03: 100.00 %
// 04: 100.00 %
// 05: 100.00 %
// 06: 100.00 %
// 07: 100.00 %
// 08: 100.00 %
// 09: 100.00 %
// 10: 100.00 %
// 11: 100.00 %
// 12: 100.00 %
// 13: 100.00 %
// 14: 100.00 %
// 15: 100.00 %











//////////////////////////////////////////////////////////////////////////////////////////////////
//
//     Forward  Schedule
//
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Total Subcarriers: [16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895, 896, 897, 898, 899, 900, 901, 902, 903, 904, 905, 906, 907, 908, 909, 910, 911, 912, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927, 928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 938, 939, 940, 941, 942, 943, 944, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 958, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 987, 988, 989, 990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007]
// input_stride = 16
// output_stride = 64
//
// global constants:
enabled_subcarriers = 256;
number_active_schedules = 16;

// Forward for chunk(0): [16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31]
//                       [16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31]
vmem_schedules[0] = (VmemSchedule) {
{ MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1), (0x0 << 12) | (DST_ROW + 1) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(1): [32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47]
//                       [32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47]
vmem_schedules[1] = (VmemSchedule) {
{ MOVER_SRC_ROW+1, MOVER_SRC_ROW+1, MOVER_SRC_ROW+1, MOVER_SRC_ROW+1, MOVER_SRC_ROW+1, MOVER_SRC_ROW+1, MOVER_SRC_ROW+1, MOVER_SRC_ROW+1, MOVER_SRC_ROW+1, MOVER_SRC_ROW+1, MOVER_SRC_ROW+1, MOVER_SRC_ROW+1, MOVER_SRC_ROW+1, MOVER_SRC_ROW+1, MOVER_SRC_ROW+1, MOVER_SRC_ROW+1 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2), (0x0 << 12) | (DST_ROW + 2) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(2): [48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63]
//                       [48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63]
vmem_schedules[2] = (VmemSchedule) {
{ MOVER_SRC_ROW+2, MOVER_SRC_ROW+2, MOVER_SRC_ROW+2, MOVER_SRC_ROW+2, MOVER_SRC_ROW+2, MOVER_SRC_ROW+2, MOVER_SRC_ROW+2, MOVER_SRC_ROW+2, MOVER_SRC_ROW+2, MOVER_SRC_ROW+2, MOVER_SRC_ROW+2, MOVER_SRC_ROW+2, MOVER_SRC_ROW+2, MOVER_SRC_ROW+2, MOVER_SRC_ROW+2, MOVER_SRC_ROW+2 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3), (0x0 << 12) | (DST_ROW + 3) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(3): [64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79]
//                       [64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79]
vmem_schedules[3] = (VmemSchedule) {
{ MOVER_SRC_ROW+3, MOVER_SRC_ROW+3, MOVER_SRC_ROW+3, MOVER_SRC_ROW+3, MOVER_SRC_ROW+3, MOVER_SRC_ROW+3, MOVER_SRC_ROW+3, MOVER_SRC_ROW+3, MOVER_SRC_ROW+3, MOVER_SRC_ROW+3, MOVER_SRC_ROW+3, MOVER_SRC_ROW+3, MOVER_SRC_ROW+3, MOVER_SRC_ROW+3, MOVER_SRC_ROW+3, MOVER_SRC_ROW+3 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4), (0x0 << 12) | (DST_ROW + 4) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(4): [80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95]
//                       [80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95]
vmem_schedules[4] = (VmemSchedule) {
{ MOVER_SRC_ROW+4, MOVER_SRC_ROW+4, MOVER_SRC_ROW+4, MOVER_SRC_ROW+4, MOVER_SRC_ROW+4, MOVER_SRC_ROW+4, MOVER_SRC_ROW+4, MOVER_SRC_ROW+4, MOVER_SRC_ROW+4, MOVER_SRC_ROW+4, MOVER_SRC_ROW+4, MOVER_SRC_ROW+4, MOVER_SRC_ROW+4, MOVER_SRC_ROW+4, MOVER_SRC_ROW+4, MOVER_SRC_ROW+4 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5), (0x0 << 12) | (DST_ROW + 5) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(5): [96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111]
//                       [96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111]
vmem_schedules[5] = (VmemSchedule) {
{ MOVER_SRC_ROW+5, MOVER_SRC_ROW+5, MOVER_SRC_ROW+5, MOVER_SRC_ROW+5, MOVER_SRC_ROW+5, MOVER_SRC_ROW+5, MOVER_SRC_ROW+5, MOVER_SRC_ROW+5, MOVER_SRC_ROW+5, MOVER_SRC_ROW+5, MOVER_SRC_ROW+5, MOVER_SRC_ROW+5, MOVER_SRC_ROW+5, MOVER_SRC_ROW+5, MOVER_SRC_ROW+5, MOVER_SRC_ROW+5 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6), (0x0 << 12) | (DST_ROW + 6) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(6): [112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127]
//                       [112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127]
vmem_schedules[6] = (VmemSchedule) {
{ MOVER_SRC_ROW+6, MOVER_SRC_ROW+6, MOVER_SRC_ROW+6, MOVER_SRC_ROW+6, MOVER_SRC_ROW+6, MOVER_SRC_ROW+6, MOVER_SRC_ROW+6, MOVER_SRC_ROW+6, MOVER_SRC_ROW+6, MOVER_SRC_ROW+6, MOVER_SRC_ROW+6, MOVER_SRC_ROW+6, MOVER_SRC_ROW+6, MOVER_SRC_ROW+6, MOVER_SRC_ROW+6, MOVER_SRC_ROW+6 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7), (0x0 << 12) | (DST_ROW + 7) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(7): [128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143]
//                       [128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143]
vmem_schedules[7] = (VmemSchedule) {
{ MOVER_SRC_ROW+7, MOVER_SRC_ROW+7, MOVER_SRC_ROW+7, MOVER_SRC_ROW+7, MOVER_SRC_ROW+7, MOVER_SRC_ROW+7, MOVER_SRC_ROW+7, MOVER_SRC_ROW+7, MOVER_SRC_ROW+7, MOVER_SRC_ROW+7, MOVER_SRC_ROW+7, MOVER_SRC_ROW+7, MOVER_SRC_ROW+7, MOVER_SRC_ROW+7, MOVER_SRC_ROW+7, MOVER_SRC_ROW+7 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8), (0x0 << 12) | (DST_ROW + 8) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(8): [880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895]
//                       [880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895]
vmem_schedules[8] = (VmemSchedule) {
{ MOVER_SRC_ROW+8, MOVER_SRC_ROW+8, MOVER_SRC_ROW+8, MOVER_SRC_ROW+8, MOVER_SRC_ROW+8, MOVER_SRC_ROW+8, MOVER_SRC_ROW+8, MOVER_SRC_ROW+8, MOVER_SRC_ROW+8, MOVER_SRC_ROW+8, MOVER_SRC_ROW+8, MOVER_SRC_ROW+8, MOVER_SRC_ROW+8, MOVER_SRC_ROW+8, MOVER_SRC_ROW+8, MOVER_SRC_ROW+8 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55), (0x0 << 12) | (DST_ROW + 55) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(9): [896, 897, 898, 899, 900, 901, 902, 903, 904, 905, 906, 907, 908, 909, 910, 911]
//                       [896, 897, 898, 899, 900, 901, 902, 903, 904, 905, 906, 907, 908, 909, 910, 911]
vmem_schedules[9] = (VmemSchedule) {
{ MOVER_SRC_ROW+9, MOVER_SRC_ROW+9, MOVER_SRC_ROW+9, MOVER_SRC_ROW+9, MOVER_SRC_ROW+9, MOVER_SRC_ROW+9, MOVER_SRC_ROW+9, MOVER_SRC_ROW+9, MOVER_SRC_ROW+9, MOVER_SRC_ROW+9, MOVER_SRC_ROW+9, MOVER_SRC_ROW+9, MOVER_SRC_ROW+9, MOVER_SRC_ROW+9, MOVER_SRC_ROW+9, MOVER_SRC_ROW+9 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56), (0x0 << 12) | (DST_ROW + 56) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(10): [912, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927]
//                        [912, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927]
vmem_schedules[10] = (VmemSchedule) {
{ MOVER_SRC_ROW+10, MOVER_SRC_ROW+10, MOVER_SRC_ROW+10, MOVER_SRC_ROW+10, MOVER_SRC_ROW+10, MOVER_SRC_ROW+10, MOVER_SRC_ROW+10, MOVER_SRC_ROW+10, MOVER_SRC_ROW+10, MOVER_SRC_ROW+10, MOVER_SRC_ROW+10, MOVER_SRC_ROW+10, MOVER_SRC_ROW+10, MOVER_SRC_ROW+10, MOVER_SRC_ROW+10, MOVER_SRC_ROW+10 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57), (0x0 << 12) | (DST_ROW + 57) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(11): [928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 938, 939, 940, 941, 942, 943]
//                        [928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 938, 939, 940, 941, 942, 943]
vmem_schedules[11] = (VmemSchedule) {
{ MOVER_SRC_ROW+11, MOVER_SRC_ROW+11, MOVER_SRC_ROW+11, MOVER_SRC_ROW+11, MOVER_SRC_ROW+11, MOVER_SRC_ROW+11, MOVER_SRC_ROW+11, MOVER_SRC_ROW+11, MOVER_SRC_ROW+11, MOVER_SRC_ROW+11, MOVER_SRC_ROW+11, MOVER_SRC_ROW+11, MOVER_SRC_ROW+11, MOVER_SRC_ROW+11, MOVER_SRC_ROW+11, MOVER_SRC_ROW+11 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58), (0x0 << 12) | (DST_ROW + 58) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(12): [944, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 958, 959]
//                        [944, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 958, 959]
vmem_schedules[12] = (VmemSchedule) {
{ MOVER_SRC_ROW+12, MOVER_SRC_ROW+12, MOVER_SRC_ROW+12, MOVER_SRC_ROW+12, MOVER_SRC_ROW+12, MOVER_SRC_ROW+12, MOVER_SRC_ROW+12, MOVER_SRC_ROW+12, MOVER_SRC_ROW+12, MOVER_SRC_ROW+12, MOVER_SRC_ROW+12, MOVER_SRC_ROW+12, MOVER_SRC_ROW+12, MOVER_SRC_ROW+12, MOVER_SRC_ROW+12, MOVER_SRC_ROW+12 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59), (0x0 << 12) | (DST_ROW + 59) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(13): [960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975]
//                        [960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975]
vmem_schedules[13] = (VmemSchedule) {
{ MOVER_SRC_ROW+13, MOVER_SRC_ROW+13, MOVER_SRC_ROW+13, MOVER_SRC_ROW+13, MOVER_SRC_ROW+13, MOVER_SRC_ROW+13, MOVER_SRC_ROW+13, MOVER_SRC_ROW+13, MOVER_SRC_ROW+13, MOVER_SRC_ROW+13, MOVER_SRC_ROW+13, MOVER_SRC_ROW+13, MOVER_SRC_ROW+13, MOVER_SRC_ROW+13, MOVER_SRC_ROW+13, MOVER_SRC_ROW+13 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60), (0x0 << 12) | (DST_ROW + 60) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(14): [976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 987, 988, 989, 990, 991]
//                        [976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 987, 988, 989, 990, 991]
vmem_schedules[14] = (VmemSchedule) {
{ MOVER_SRC_ROW+14, MOVER_SRC_ROW+14, MOVER_SRC_ROW+14, MOVER_SRC_ROW+14, MOVER_SRC_ROW+14, MOVER_SRC_ROW+14, MOVER_SRC_ROW+14, MOVER_SRC_ROW+14, MOVER_SRC_ROW+14, MOVER_SRC_ROW+14, MOVER_SRC_ROW+14, MOVER_SRC_ROW+14, MOVER_SRC_ROW+14, MOVER_SRC_ROW+14, MOVER_SRC_ROW+14, MOVER_SRC_ROW+14 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61), (0x0 << 12) | (DST_ROW + 61) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(15): [992, 993, 994, 995, 996, 997, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007]
//                        [992, 993, 994, 995, 996, 997, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007]
vmem_schedules[15] = (VmemSchedule) {
{ MOVER_SRC_ROW+15, MOVER_SRC_ROW+15, MOVER_SRC_ROW+15, MOVER_SRC_ROW+15, MOVER_SRC_ROW+15, MOVER_SRC_ROW+15, MOVER_SRC_ROW+15, MOVER_SRC_ROW+15, MOVER_SRC_ROW+15, MOVER_SRC_ROW+15, MOVER_SRC_ROW+15, MOVER_SRC_ROW+15, MOVER_SRC_ROW+15, MOVER_SRC_ROW+15, MOVER_SRC_ROW+15, MOVER_SRC_ROW+15 },
{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
{ (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62), (0x0 << 12) | (DST_ROW + 62) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};







}






// assumes forward mover
// pass in a schedule and a column in that schedule and this will return the subcarrier #
// WARNING does not check if schedule # is within bounds
unsigned int channel_from_schedule(unsigned int schedule, unsigned int column) {
    unsigned int raw;
    raw = vmem_schedules[schedule].dst_addr[column] & 0xfff;  // lower 12 bits are destination address

    unsigned int as_row;

    as_row = raw - DST_ROW;

    unsigned int sc;

    sc = as_row * 16 + column;

    return sc;
}




unsigned int output_frame_count;

#ifdef USE_DOUBLE_BUFFER
unsigned int mover_output_increment_words;
unsigned int mover_output_increment_row;
#endif
// call after setup_mover
void setup_mover_post() {
  output_frame_count = (16 * 16) / enabled_subcarriers;

#ifdef USE_DOUBLE_BUFFER
  // bumps for our "a" / "b" buffers
  mover_output_increment_words = output_frame_count << 10; // times 1024
  mover_output_increment_row = mover_output_increment_words >> 4; // over 16
#endif
}
unsigned int dma_in_dma_ptr;

// in words
#define DMA_IN_SIZE (16)

// 64 is maximum here unless more memory is vmalloc'd
#define DMA_IN_CHUNKS (64)

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

// converts a dma index (used in the cirbufs) to a dma_ptr
// the dma index counts each block of memory
unsigned int dma_idx_to_ptr(unsigned int idx) {
  return dma_in_dma_ptr + (idx * DMA_IN_SIZE);
}

// convert a dma_ptr to an index
unsigned int dma_ptr_to_idx(unsigned int ptr) {
  return (ptr - dma_in_dma_ptr) / DMA_IN_SIZE;
}

void trig_dma_in(unsigned int dma_ptr) {
  CSR_WRITE(DMA_0_START_ADDR, dma_ptr);
  CSR_WRITE(DMA_0_LENGTH, DMA_IN_SIZE);
  CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff); // start right away
  CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);
}


// void trig_dma_out(unsigned int dma_ptr) {
//   CSR_WRITE(DMA_1_START_ADDR, dma_ptr);
//   CSR_WRITE(DMA_1_LENGTH, DMA_IN_SIZE);
//   CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff); // start right away
//   CSR_WRITE_ZERO(DMA_1_PUSH_SCHEDULE);
// }


void trig_dma_in_next() {
  trig_dma_in(dma_idx_to_ptr(dma_trig_next));

  circular_buf_put(&dma_schedule_in, dma_trig_next);

  dma_trig_next = (dma_trig_next+1) % DMA_IN_CHUNKS;
}
//////////////////////////////////////////
//
// We run 2 circular buffers
// the first buffer keeps track of outstanding input dma so they are always overlapping
// as these they dump into the 2nd circular buffer which is the "pending data" and also our fill level

void setup_dma_in(void) {
  dma_in_dma_ptr = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));

  circular_buf_initialize(&dma_schedule_in, dma_schedule_in_storage, DMA_SCHEDULE_IN_SIZE);
  circular_buf_initialize(&dma_in_buffer, dma_in_buffer_storage, DMA_IN_CIRBUF_SIZE);


  trig_dma_in_next();
  trig_dma_in_next();
  trig_dma_in_next();
  trig_dma_in_next();

  // how many chunks we get from a single vmalloc
  // unsigned int chunks = (VMALLOC_CHUNK_SIZE/4) / dma_in_size;

  // ring_block_send_eth(chunks);

  // trig_dma_in(0, 0xffffffff);
  // trig_dma_in(1, 0xffffffff);
}

// void setup_dma_out(void) {
//   circular_buf_initialize(&dma_schedule_out, dma_schedule_out_storage, DMA_SCHEDULE_OUT_SIZE);
// }

// unsigned int fake_work_todo = 0;

void pet_dma_inqueue() {
  int error;
  unsigned int just_finished_idx;
  unsigned int dma_occupancy;

  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, dma_occupancy);
  unsigned int filled = circular_buf_occupancy(&dma_schedule_in);


  if( dma_occupancy != filled ) {
    // just_finished_idx is the index of the dma that just finished
    error = circular_buf_get(&dma_schedule_in, &just_finished_idx); MY_ASSERT(error == 0);

    // now that dma is done with this chunk, we add it to the next
    // circular buffer which signals the program that there is fresh data to be processed
    circular_buf_put(&dma_in_buffer, just_finished_idx);
    // ring_block_send_eth(dma_occupancy);
    // ring_block_send_eth(filled);

    // fake_work_todo += 10;


    // ring_block_send_eth(data);
    trig_dma_in_next();
  }

}


void pet_dma_in() {
  pet_dma_inqueue();
}


// #define FILL_UNDERFLOW  (0x1 << 16)
// #define FILL_OVERFLOW   (0x1 << 17)

#define LESS_THAN_UNDERFLOW (4)
#define GREATER_THAN_OVERFLOW (62)

#define FILL_LEVEL_REPORT_TIME (20000000)

unsigned int report_timer, report_timer_check, fill_flags;

void setup_fill_level(void) {
  // fill level update timer, ringbus takes about 500 counter ticks to write
  // so this update should be much slower then that
  CSR_READ(TIMER_VALUE, report_timer);
  fill_flags = 0;
}


#define FILL_L_SHIFT (0)
#define FILL_L_MASK (0xFF)

#define FILL_H_SHIFT (8)
#define FILL_H_MASK (0xFF)

#define FILL_UNDERFLOW_SHIFT (16)
#define FILL_UNDERFLOW_MASK (0x1)

#define FILL_OVERFLOW_SHIFT (17)
#define FILL_OVERFLOW_MASK (0x1)

unsigned int prn_channels[16*5];



// there is no fill level in prn version of this.
void pet_fill_level(void) {
  Ringbus ringbus;

  // unsigned int fill;
  // static unsigned int fill_low = 0xffff;
  // static unsigned int fill_high = 0x0000;

  // CSR_READ(TIMER_VALUE, report_timer_check);

  // fill = circular_buf_occupancy(&dma_in_buffer);

  // // check for under/over every time
  // if(fill <= LESS_THAN_UNDERFLOW) {
  //   fill_flags |= (1<<FILL_UNDERFLOW_SHIFT);
  // }

  // if(fill >= GREATER_THAN_OVERFLOW) {
  //   fill_flags |= (1<<FILL_OVERFLOW_SHIFT);
  // }

  // // set high/lows
  // fill_low =  MIN(fill_low,  fill);
  // fill_high = MAX(fill_high, fill);

  // // report flags slowly
  // // if( report_timer_check >= (report_timer+FILL_LEVEL_REPORT_TIME) ) {
  // if( subtract_timers(report_timer_check, report_timer) >= FILL_LEVEL_REPORT_TIME) {
  //   ring_block_send_eth(0x0b000000 | 
  //     fill_flags | 
  //     ((fill_low & FILL_L_MASK) << FILL_L_SHIFT) |
  //     ((fill_high & FILL_H_MASK) << FILL_H_SHIFT)
  //     );
  //   // ring_block_send_eth(fill_low);
  //   // ring_block_send_eth(fill_high);

  //   report_timer = report_timer_check;
    

  //   fill_low = 0xffff;
  //   fill_high = 0x0000;
  //   fill_flags = 0;
  // }

  check_ring(&ringbus); // check bootloader more often than we report
}

void debug_readout(unsigned int count) {
  unsigned int dma_idx_just_finished;
  unsigned int* dma_cpu_pointer;
  int error;
  for(unsigned i = 0; i < count; i++) {
    error = circular_buf_get(&dma_in_buffer, &dma_idx_just_finished); MY_ASSERT(error == 0);

    dma_cpu_pointer = REVERSE_VMEM_DMA_ADDRESS(dma_idx_to_ptr(dma_idx_just_finished));

    for(unsigned int j = 0; j < 16; j++) {
      ring_block_send_eth(dma_cpu_pointer[j]);
    }


  }
}


// writes to the global variable prn_channels
// assumes 64 enabled subcarriers.
// there are 1024 subcarriers, so 10 bits.
unsigned int load_prn_channels() {
    unsigned int tmask,tshift;
    unsigned int load[16];// = {3,0,1,2,3,0,1,2,3,0,1,2,3,0,1,2};

    for(unsigned int time = 0; time < 5; time++) {

        tshift = time*2;
        tmask = 0x3 << (tshift); // mask 2 bits along as we go

        for(unsigned int schedule = 0; schedule < 16; schedule++) {
            unsigned int output_word_index = time*16+schedule;

            for(unsigned int i = 0; i < 16; i++) {

                // unsigned int schedule = i/16;
                unsigned int col = i;
                unsigned int sc = channel_from_schedule(schedule, col);
                // unsigned int val = 

                load[i] = (sc & tmask) >> tshift;

                // ring_block_send_eth(index);
                // ring_block_send_eth(schedule);
            }
            prn_channels[output_word_index] = mover_debug_load_qpsk(load);

        }
      // ring_block_send_eth(0xdead);
    }
}


// this only needs to be as big as number_active_schedules
VMEM_SECTION unsigned int debug_mapper_input[16*16] = {};

int main(void)
{
  unsigned int rtn;
  Ringbus ringbus;

  // setup vmalloc
  init_VMalloc(&mgr);


  unsigned int* input_cpu_ptr =  vmalloc_single(&mgr);
  unsigned int input_dma_ptr = VMEM_DMA_ADDRESS(input_cpu_ptr);

  unsigned int* scrach_cpu_ptr = vmalloc_single(&mgr);
  unsigned int scratch_dma_ptr = VMEM_DMA_ADDRESS(scrach_cpu_ptr);

  unsigned int* debug_cpu_ptr =  vmalloc_single(&mgr);
  unsigned int debug_dma_ptr = VMEM_DMA_ADDRESS(debug_cpu_ptr);

  mover_working_on = 0;

  unsigned int a, b, c, d;

  // for(unsigned int i = 0; i < 16; i++) {
  //   debug_cpu_ptr[i] = 0xa0000000 | i;
  // }

  for(unsigned int i = 0; i < 16*16; i++) {
    debug_mapper_input[i] = 0xa0000000 | i;
  }




  // // dump our input and reset
  // CSR_WRITE(DMA_0_START_ADDR, dma_idx_to_ptr(0));
  // CSR_WRITE(DMA_0_LENGTH, 1024);
  // CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff); // start right away
  // CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);
  // CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);

  // for(unsigned int i = 0; i < 10000; i++) {
  //   STALL(1);
  // }

  CSR_WRITE(DMA_0_FLUSH_SCHEDULE,  0);
  CSR_WRITE(DMA_1_FLUSH_SCHEDULE,  0);




  // for(unsigned int i = 0; i < 1024; i++) {
  //   vector_memory[i+input_dma_ptr] = 0xf000d000 + i;
  // }

  // setup mapper
  setup_mapper();

  setup_mover();
  setup_mover_post(); // must be called afer previous

  setup_dma_in();
  // setup_dma_out();
  setup_fill_level();

#define RING_OUT_ADDRESS

#ifdef RING_OUT_ADDRESS

  ring_block_send_eth(0xdead); // boot

  // debug ringbus out row addresses of all memory for easy lookup into cs20.out (row+1 = linenumber)
  ring_block_send_eth(input_dma_ptr);
  ring_block_send_eth(scratch_dma_ptr);
  ring_block_send_eth(debug_dma_ptr);
  ring_block_send_eth(MAPPER_DEST_ROW);
  ring_block_send_eth(VMEM_ROW_ADDRESS(mover_output));
  ring_block_send_eth(VMEM_ROW_ADDRESS(REVERSE_VMEM_DMA_ADDRESS(SCRATCH_DMA)));
  ring_block_send_eth(DST_ROW);
  // ring_block_send_eth(DST_ROW+mover_output_increment_row);
  // ring_block_send_eth(VMEM_ROW_ADDRESS(&vs0));

  // ring_block_send_eth(VMEM_ROW_ADDRESS(input_cpu_ptr));
  // ring_block_send_eth(VMEM_ROW_ADDRESS(scrach_cpu_ptr));
  // ring_block_send_eth(GARBAGE_ROW);
  // ring_block_send_eth(mapper_output_row);
  // ring_block_send_eth(qpsk_table_dma/16);

#endif
  unsigned int ben = 0;
  // ring_block_send_eth(0xdead); // boot

  CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);
  unsigned int fsm_state = INIT_STATE;
  unsigned int return_time;



  unsigned int incomming_occupancy;
  unsigned int outgoing_dma_occupancy;
  unsigned int dma_idx_just_finished;
  unsigned int needs_mapping_dma_ptr;
  int error;

  unsigned int ben_counter = 0;

  int flushing = -1; // determines where the nextstate goes in FINISH_STATE

  simple_random_seed(0xfe91148c);

  unsigned int debug_counter = 0xf0000000;
  unsigned int debug_counter2 = simple_random();
  unsigned int debug_counter3 = 0;
  unsigned int fake_mapper_track = 0;
  unsigned int fake_mapper_state = 0;

  unsigned int debug_mapper_input_dma_ptr = VMEM_DMA_ADDRESS(debug_mapper_input);

  unsigned int prn_pattern[11];// = {0x0, 0xffff0000, 0x0000ffff, 0xffffffff};

  prn_pattern[0] = mover_debug_load_qpsk((unsigned int[16]){0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3});
  prn_pattern[1] = mover_debug_load_qpsk((unsigned int[16]){1,2,3,0,1,2,3,0,1,2,3,0,1,2,3,0});
  prn_pattern[2] = mover_debug_load_qpsk((unsigned int[16]){2,3,0,1,2,3,0,1,2,3,0,1,2,3,0,1});
  prn_pattern[3] = mover_debug_load_qpsk((unsigned int[16]){3,0,1,2,3,0,1,2,3,0,1,2,3,0,1,2});
  prn_pattern[4] = prn_pattern[0];
  prn_pattern[5] = prn_pattern[1];
  prn_pattern[6] = prn_pattern[2];
  prn_pattern[7] = prn_pattern[3];
  prn_pattern[8] = prn_pattern[0];
  prn_pattern[9] = prn_pattern[1];
  prn_pattern[10] = prn_pattern[2];


  prn_pattern[0]  = prn_pattern[1]  = prn_pattern[2]  = prn_pattern[3]  = prn_pattern[4]  = prn_pattern[5]  = prn_pattern[6]  = prn_pattern[7]  = prn_pattern[8]  = prn_pattern[9]  = prn_pattern[10]  = 0;

  load_prn_channels();

  // prn_pattern[4] = mover_debug_load_qpsk((unsigned int[16]){0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3});
  // prn_pattern[5] = mover_debug_load_qpsk((unsigned int[16]){1,2,3,0,1,2,3,0,1,2,3,0,1,2,3,0});
  // prn_pattern[6] = mover_debug_load_qpsk((unsigned int[16]){2,3,0,1,2,3,0,1,2,3,0,1,2,3,0,1});
  // prn_pattern[7] = mover_debug_load_qpsk((unsigned int[16]){3,0,1,2,3,0,1,2,3,0,1,2,3,0,1,2});
  // prn_pattern[9] = prn_pattern[5];
  // prn_pattern[10] = prn_pattern[6];
  // prn_pattern[11] = prn_pattern[7];
  // prn_pattern[12] = prn_pattern[4];
  // prn_pattern[13] = prn_pattern[5];
  // prn_pattern[14] = prn_pattern[6];
  // prn_pattern[15] = prn_pattern[7];

    ring_block_send_eth(0xdead);


// ring_block_send_eth(prn_pattern[0]);
// ring_block_send_eth(prn_pattern[1]);
// ring_block_send_eth(prn_pattern[2]);
// ring_block_send_eth(prn_pattern[3]);

  while(1) {
    pet_dma_in();
    pet_dma_in();
    pet_dma_in();
    pet_dma_in();

    // pet_dma_out();

    pet_fill_level(); // this can be called in other places, maybe where a blocking loop is


    // 0x300000 is sets two upper most bits of GPIO
    // CSR_WRITE(GPIO_WRITE, (0x300000) | fsm_state);

    //Want this to timeout if no dma is rcved. Do something else and try again. 


    if (fsm_state==MAP_STATE) {


// prn section
#ifdef USE_FAKE_MAPPER_INPUT

        // unsigned int prn_value = prn_pattern[debug_counter3%4];
        unsigned int prn_jamming;

        if( fake_mapper_state >= 5 ) {

            prn_jamming = prn_pattern[ (fake_mapper_state-5) ];
        }


        // unsigned int constructed_value = 
        //     mover_debug_load_qpsk((unsigned int[16]){0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3});


        // for(unsigned int i = 0; i < number_active_schedules i++ ) {
        //     debug_mapper_input[0] = constructed_value;
        // }



        // 16 is number of active schedules
        for(unsigned int i = 0; i < number_active_schedules; i++) {

            if( fake_mapper_state < 5) {
                unsigned int load_index = fake_mapper_state*16+i;
                // hack this goes/in/out of vmem a few times, could be faster
                vector_memory[debug_mapper_input_dma_ptr+0] = prn_channels[load_index];
            } else {
                vector_memory[debug_mapper_input_dma_ptr+0] = prn_jamming;
            }

// prn_channels
            // for(unsigned int j = 0; j < 16; j++) {
            //     ring_block_send_eth(channel_from_schedule(i,j));
            // }

            // vector_memory[debug_mapper_input_dma_ptr+0] = constructed_value;


            mapper_load_qpsk(&qpsk_table, ( (i*NSLICES) + (MAPPER_DEST_ROW*NSLICES) ), debug_mapper_input_dma_ptr+0);
            // SET_REG(x4, debug_counter2 );
            // debug_counter2 = simple_random();
        }



        // 4 is length of pattern we are ripping through
        fake_mapper_state = (fake_mapper_state+1) % 16;
        // fake_mapper_track = (fake_mapper_track+1) % number_active_schedules;
        // if( fake_mapper_track == 0 ) {
            // add one for every set of subcarriers
            debug_counter3++;
        // }

#endif // FAKE_MAPPER




      // previous state (WAITING_STATE) guarantees that there is 1 or more item in this cirbuf
      // so we just grab it
      error = circular_buf_get(&dma_in_buffer, &dma_idx_just_finished); //MY_ASSERT(error == 0);

      if( error == 0 ) {
        needs_mapping_dma_ptr = dma_idx_to_ptr(dma_idx_just_finished);




#ifndef USE_FAKE_MOVER_INPUT

        //Got 16 words, lets map bpsk
        // this stage takes an input from a changing address (dependin on which chunk we are doing)
        // however the output always goes to the same address (a single buffer)

        // CSR_READ(TIMER_VALUE, a);
        // ring_block_send_eth(needs_mapping_dma_ptr);
#ifndef USE_FAKE_MAPPER_INPUT



        for (unsigned int i = 0; i < 16; i++) {  // 16 here is rows of mapped data.
            // 2nd argument goes by stride of 16
            // 3rd argument inches along 1 by 1

            // 3rd argument is a dma pointer to input memory
            mapper_load_qpsk(&qpsk_table, ( (i*NSLICES) + (MAPPER_DEST_ROW*NSLICES) ), i+needs_mapping_dma_ptr);

            SET_REG(x4, vector_memory[i+needs_mapping_dma_ptr] );
            // mapper_load_qpsk(&qpsk_table, ( (i*NSLICES) + (MAPPER_DEST_ROW*NSLICES) ), i+debug_dma_ptr);

            // mapper_load_qpsk(&qpsk_table, ( (i*NSLICES) + (MAPPER_DEST_ROW*NSLICES) ), debug_dma_ptr);
            // debug_counter++;
        }


#endif // FAKE_MAPPER

        // CSR_READ(TIMER_VALUE, b);

        // ring_block_send_eth(0x0c000000 | b-a);

#else // USE_FAKE_MOVER_INPUT

        unsigned int mapper_output_dma_ptr = (MAPPER_DEST_ROW*NSLICES);
        for(unsigned int i = 0; i < NSLICES*DMA_IN_SIZE; i++) {
          vector_memory[i+mapper_output_dma_ptr] = debug_counter;
          debug_counter++;
        }
#endif //USE_FAKE_MOVER_INPUT


        CSR_WRITE(GPIO_WRITE, (0x200000) | 1);
      } else {
        CSR_WRITE(GPIO_WRITE, (0x200000) | 2); // error condition
      }

    }

    if (fsm_state==MOVE_STATE) {



      // spin until previous output dma is done (one by one)
      while(1) {
        CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, outgoing_dma_occupancy);
#ifdef USE_DOUBLE_BUFFER
        if(outgoing_dma_occupancy < 2) {
#else
        if(outgoing_dma_occupancy == 0) {
#endif
          break;
        } else {
          pet_dma_in();
          pet_fill_level();
        }
      }

#ifdef USE_OLD_MOVER_SCHEDULE_FORMAT
      for(unsigned int i = 0; i < number_active_schedules; i++) {
        mover_schedule(schedules[i], SCRATCH_DMA);
        mover_roll(output_frame_count);
      }

#ifdef USE_DOUBLE_BUFFER
#pragma GCC error "double buffer not supported when USE_OLD_MOVER_SCHEDULE_FORMAT is set"
#endif

#else

#ifdef USE_DOUBLE_BUFFER
      if( mover_working_on == 0) {
        
        // move into beginning of buffer
        for(unsigned int i = 0; i < number_active_schedules; i++) {
          mover_load_vmem(&(vmem_schedules[i]));
          mover_roll(output_frame_count);
        }
      } else {

        // move into offset of buffer
        for(unsigned int i = 0; i < number_active_schedules; i++) {
          mover_load_offset_output( &(vmem_schedules[i]), mover_output_increment_row);
          mover_roll(output_frame_count);
        }
      }
#else
      for(unsigned int i = 0; i < number_active_schedules; i++) {
          mover_load_vmem(&(vmem_schedules[i]));
          mover_roll(output_frame_count);
        }
#endif

#endif
      // exit2(0);

      // CSR_READ(TIMER_VALUE, d);

      // ring_block_send_eth(0x0d000000 | d-c);
      

      CSR_WRITE(GPIO_WRITE, (0x200000) | 2);
    
    }

    if (fsm_state==TX_STATE) {
      // trigger output

      // mover_output_increment_words
      // mover_output_increment_row


#ifdef USE_DOUBLE_BUFFER
      // schedule buffer "A" for output dma
      if( mover_working_on == 0 ) {
        dma_out_set(DST_ROW*NSLICES, FFT_SIZE*output_frame_count);
        mover_working_on = 1;
      } else {
        // buffer "B"
        dma_out_set(DST_ROW*NSLICES+mover_output_increment_words, FFT_SIZE*output_frame_count);
        mover_working_on = 0;
      }
#else
      dma_out_set(DST_ROW*NSLICES, FFT_SIZE*output_frame_count);
#endif





      CSR_WRITE(GPIO_WRITE, (0x200000) | 3);
    }

    if(fsm_state==FINISH_STATE) {
      
    }

    switch(fsm_state) {

       case INIT_STATE:
           fsm_state = WAITING_STATE;
           break;
           
           // we wait in this state, and check the size of the dma_in_buffer
           // as soon as there is something there, we start working on it
           // this value should never reach zero during "normal operation"
           // if it does, we are in an underflow condition
       case WAITING_STATE:
#ifndef USE_FAKE_MAPPER_INPUT
            incomming_occupancy = circular_buf_occupancy(&dma_in_buffer);

            if(incomming_occupancy > 0) {
               fsm_state = MAP_STATE;
              // debug_readout(16);
              flushing = 1;
             }
#else
             fsm_state = MAP_STATE;
             flushing = 1;
#endif
           break;
       
       case MAP_STATE:
           fsm_state = MOVE_STATE;
           break;

       case MOVE_STATE:
           fsm_state = TX_STATE;
           break;

       case TX_STATE:
           // CSR_WRITE(GPIO_WRITE, (0x200000) | 5);
           fsm_state = FINISH_STATE;
           break;     

       case FINISH_STATE:
           // CSR_WRITE(GPIO_WRITE, (0x200000) | 4);
           // rtn = dma_out_check(128);

           // if (rtn) {
             // CSR_WRITE(GPIO_WRITE, (0x200000) | 6);
            if(flushing >= 0) {
              flushing--;
              fsm_state = MAP_STATE;
            } else {
              flushing = -1; // disable
              fsm_state = WAITING_STATE;
            }
           // } 
           break;

       default: /* Optional */
           fsm_state = INIT_STATE;
           break;
    }

    SET_REG(x4, fsm_state);

  }

}

