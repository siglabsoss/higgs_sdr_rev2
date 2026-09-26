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

#define PWR_BOOT 0
#define PWR_IDLE 1
#define PWR_CAPTURE 2
#define PWR_ANALYZE 3
#define PWR_JUDGE 4
#define PWR_DISABLE 5


unsigned int power_est_state = 0;


unsigned int needs_holdover = 0;

unsigned int coarse_sync_zero_output_counter = 0;

VMEM_SECTION unsigned int zero_buffer[1280] = {0};

// This is a junk area, use it for whatever
VMEM_SECTION unsigned int holdover_data[1280*2] = {0};

VMEM_SECTION unsigned int input_data[1280*4]={0};

VMEM_SECTION unsigned int output_conj_multi[1024]  = {0};
VMEM_SECTION unsigned int output_onetone_calculation[1024] = {0};
VMEM_SECTION unsigned int output_sum_complex[16] = {0};
VMEM_SECTION unsigned int output_coarse_sync[16*16] = {0};

VMEM_SECTION unsigned int bank_address_onetone_calculation[16] = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3};
VMEM_SECTION unsigned int permutation_address_onetone_calculation[16] = {0x0, 0x3000, 0x6000, 0x9000, 0xd000, 0xd000, 0xd000, 0xe000, 0xe000, 0xe000, 0xf000, 0xf000, 0xf000, 0x0, 0x0, 0x0};


VMEM_SECTION unsigned int exp_data_1280_ext_14[1280*2] = {0x00004000, 0xffb04000, 0xff5f3fff, 0xff0f3ffe, 0xfebe3ffd, 0xfe6e3ffb, 0xfe1e3ff9, 0xfdcd3ff6, 0xfd7d3ff3, 0xfd2c3ff0, 0xfcdc3fec, 0xfc8c3fe8, 0xfc3b3fe4, 0xfbeb3fdf, 0xfb9b3fd9, 0xfb4b3fd4, 0xfafb3fcd, 0xfaaa3fc7, 0xfa5a3fc0, 0xfa0a3fb9, 0xf9ba3fb1, 0xf96a3fa9, 0xf91a3fa1, 0xf8ca3f98, 0xf87a3f8e, 0xf82a3f85, 0xf7db3f7b, 0xf78b3f70, 0xf73b3f65, 0xf6ec3f5a, 0xf69c3f4f, 0xf64c3f43, 0xf5fd3f36, 0xf5ae3f2a, 0xf55e3f1c, 0xf50f3f0f, 0xf4c03f01, 0xf4713ef3, 0xf4223ee4, 0xf3d33ed5, 0xf3843ec5, 0xf3353eb5, 0xf2e63ea5, 0xf2973e94, 0xf2493e83, 0xf1fa3e72, 0xf1ac3e60, 0xf15d3e4e, 0xf10f3e3b, 0xf0c13e28, 0xf0733e15, 0xf0253e01, 0xefd73ded, 0xef893dd9, 0xef3c3dc4, 0xeeee3daf, 0xeea13d99, 0xee533d83, 0xee063d6c, 0xedb93d56, 0xed6c3d3f, 0xed1f3d27, 0xecd23d0f, 0xec863cf7, 0xec393cde, 0xebed3cc5, 0xeba03cac, 0xeb543c92, 0xeb083c78, 0xeabc3c5d, 0xea703c42, 0xea253c27, 0xe9d93c0b, 0xe98e3bef, 0xe9433bd3, 0xe8f73bb6, 0xe8ad3b99, 0xe8623b7c, 0xe8173b5e, 0xe7cc3b3f, 0xe7823b21, 0xe7383b02, 0xe6ee3ae3, 0xe6a43ac3, 0xe65a3aa3, 0xe6113a82, 0xe5c73a62, 0xe57e3a41, 0xe5353a1f, 0xe4ec39fd, 0xe4a339db, 0xe45a39b8, 0xe4123995, 0xe3ca3972, 0xe381394f, 0xe33a392b, 0xe2f23906, 0xe2aa38e2, 0xe26338bd, 0xe21c3897, 0xe1d53871, 0xe18e384b, 0xe1473825, 0xe10137fe, 0xe0ba37d7, 0xe07437b0, 0xe02e3788, 0xdfe93760, 0xdfa33737, 0xdf5e370e, 0xdf1936e5, 0xded436bc, 0xde8f3692, 0xde4b3667, 0xde07363d, 0xddc33612, 0xdd7f35e7, 0xdd3b35bb, 0xdcf83590, 0xdcb53563, 0xdc723537, 0xdc2f350a, 0xdbec34dd, 0xdbaa34af, 0xdb683481, 0xdb263453, 0xdae43425, 0xdaa333f6, 0xda6233c7, 0xda213398, 0xd9e03368, 0xd9a03338, 0xd95f3307, 0xd91f32d7, 0xd8e032a6, 0xd8a03274, 0xd8613243, 0xd8223211, 0xd7e331de, 0xd7a431ac, 0xd7663179, 0xd7283146, 0xd6ea3112, 0xd6ad30df, 0xd66f30aa, 0xd6323076, 0xd5f63041, 0xd5b9300c, 0xd57d2fd7, 0xd5412fa2, 0xd5052f6c, 0xd4ca2f36, 0xd48f2eff, 0xd4542ec8, 0xd4192e91, 0xd3df2e5a, 0xd3a42e22, 0xd36b2deb, 0xd3312db2, 0xd2f82d7a, 0xd2bf2d41, 0xd2862d08, 0xd24e2ccf, 0xd2152c95, 0xd1de2c5c, 0xd1a62c21, 0xd16f2be7, 0xd1382bac, 0xd1012b71, 0xd0ca2b36, 0xd0942afb, 0xd05e2abf, 0xd0292a83, 0xcff42a47, 0xcfbf2a0a, 0xcf8a29ce, 0xcf562991, 0xcf212953, 0xceee2916, 0xceba28d8, 0xce87289a, 0xce54285c, 0xce22281d, 0xcdef27de, 0xcdbd279f, 0xcd8c2760, 0xcd5a2720, 0xcd2926e1, 0xccf926a1, 0xccc82660, 0xcc982620, 0xcc6825df, 0xcc39259e, 0xcc0a255d, 0xcbdb251c, 0xcbad24da, 0xcb7f2498, 0xcb512456, 0xcb232414, 0xcaf623d1, 0xcac9238e, 0xca9d234b, 0xca702308, 0xca4522c5, 0xca192281, 0xc9ee223d, 0xc9c321f9, 0xc99921b5, 0xc96e2171, 0xc944212c, 0xc91b20e7, 0xc8f220a2, 0xc8c9205d, 0xc8a02017, 0xc8781fd2, 0xc8501f8c, 0xc8291f46, 0xc8021eff, 0xc7db1eb9, 0xc7b51e72, 0xc78f1e2b, 0xc7691de4, 0xc7431d9d, 0xc71e1d56, 0xc6fa1d0e, 0xc6d51cc6, 0xc6b11c7f, 0xc68e1c36, 0xc66b1bee, 0xc6481ba6, 0xc6251b5d, 0xc6031b14, 0xc5e11acb, 0xc5bf1a82, 0xc59e1a39, 0xc57e19ef, 0xc55d19a6, 0xc53d195c, 0xc51d1912, 0xc4fe18c8, 0xc4df187e, 0xc4c11834, 0xc4a217e9, 0xc484179e, 0xc4671753, 0xc44a1709, 0xc42d16bd, 0xc4111672, 0xc3f51627, 0xc3d915db, 0xc3be1590, 0xc3a31544, 0xc38814f8, 0xc36e14ac, 0xc3541460, 0xc33b1413, 0xc32213c7, 0xc309137a, 0xc2f1132e, 0xc2d912e1, 0xc2c11294, 0xc2aa1247, 0xc29411fa, 0xc27d11ad, 0xc267115f, 0xc2511112, 0xc23c10c4, 0xc2271077, 0xc2131029, 0xc1ff0fdb, 0xc1eb0f8d, 0xc1d80f3f, 0xc1c50ef1, 0xc1b20ea3, 0xc1a00e54, 0xc18e0e06, 0xc17d0db7, 0xc16c0d69, 0xc15b0d1a, 0xc14b0ccb, 0xc13b0c7c, 0xc12b0c2d, 0xc11c0bde, 0xc10d0b8f, 0xc0ff0b40, 0xc0f10af1, 0xc0e40aa2, 0xc0d60a52, 0xc0ca0a03, 0xc0bd09b4, 0xc0b10964, 0xc0a60914, 0xc09b08c5, 0xc0900875, 0xc0850825, 0xc07b07d6, 0xc0720786, 0xc0680736, 0xc05f06e6, 0xc0570696, 0xc04f0646, 0xc04705f6, 0xc04005a6, 0xc0390556, 0xc0330505, 0xc02c04b5, 0xc0270465, 0xc0210415, 0xc01c03c5, 0xc0180374, 0xc0140324, 0xc01002d4, 0xc00d0283, 0xc00a0233, 0xc00701e2, 0xc0050192, 0xc0030142, 0xc00200f1, 0xc00100a1, 0xc0000050, 0xc0000000, 0xc000ffb0, 0xc001ff5f, 0xc002ff0f, 0xc003febe, 0xc005fe6e, 0xc007fe1e, 0xc00afdcd, 0xc00dfd7d, 0xc010fd2c, 0xc014fcdc, 0xc018fc8c, 0xc01cfc3b, 0xc021fbeb, 0xc027fb9b, 0xc02cfb4b, 0xc033fafb, 0xc039faaa, 0xc040fa5a, 0xc047fa0a, 0xc04ff9ba, 0xc057f96a, 0xc05ff91a, 0xc068f8ca, 0xc072f87a, 0xc07bf82a, 0xc085f7db, 0xc090f78b, 0xc09bf73b, 0xc0a6f6ec, 0xc0b1f69c, 0xc0bdf64c, 0xc0caf5fd, 0xc0d6f5ae, 0xc0e4f55e, 0xc0f1f50f, 0xc0fff4c0, 0xc10df471, 0xc11cf422, 0xc12bf3d3, 0xc13bf384, 0xc14bf335, 0xc15bf2e6, 0xc16cf297, 0xc17df249, 0xc18ef1fa, 0xc1a0f1ac, 0xc1b2f15d, 0xc1c5f10f, 0xc1d8f0c1, 0xc1ebf073, 0xc1fff025, 0xc213efd7, 0xc227ef89, 0xc23cef3c, 0xc251eeee, 0xc267eea1, 0xc27dee53, 0xc294ee06, 0xc2aaedb9, 0xc2c1ed6c, 0xc2d9ed1f, 0xc2f1ecd2, 0xc309ec86, 0xc322ec39, 0xc33bebed, 0xc354eba0, 0xc36eeb54, 0xc388eb08, 0xc3a3eabc, 0xc3beea70, 0xc3d9ea25, 0xc3f5e9d9, 0xc411e98e, 0xc42de943, 0xc44ae8f7, 0xc467e8ad, 0xc484e862, 0xc4a2e817, 0xc4c1e7cc, 0xc4dfe782, 0xc4fee738, 0xc51de6ee, 0xc53de6a4, 0xc55de65a, 0xc57ee611, 0xc59ee5c7, 0xc5bfe57e, 0xc5e1e535, 0xc603e4ec, 0xc625e4a3, 0xc648e45a, 0xc66be412, 0xc68ee3ca, 0xc6b1e381, 0xc6d5e33a, 0xc6fae2f2, 0xc71ee2aa, 0xc743e263, 0xc769e21c, 0xc78fe1d5, 0xc7b5e18e, 0xc7dbe147, 0xc802e101, 0xc829e0ba, 0xc850e074, 0xc878e02e, 0xc8a0dfe9, 0xc8c9dfa3, 0xc8f2df5e, 0xc91bdf19, 0xc944ded4, 0xc96ede8f, 0xc999de4b, 0xc9c3de07, 0xc9eeddc3, 0xca19dd7f, 0xca45dd3b, 0xca70dcf8, 0xca9ddcb5, 0xcac9dc72, 0xcaf6dc2f, 0xcb23dbec, 0xcb51dbaa, 0xcb7fdb68, 0xcbaddb26, 0xcbdbdae4, 0xcc0adaa3, 0xcc39da62, 0xcc68da21, 0xcc98d9e0, 0xccc8d9a0, 0xccf9d95f, 0xcd29d91f, 0xcd5ad8e0, 0xcd8cd8a0, 0xcdbdd861, 0xcdefd822, 0xce22d7e3, 0xce54d7a4, 0xce87d766, 0xcebad728, 0xceeed6ea, 0xcf21d6ad, 0xcf56d66f, 0xcf8ad632, 0xcfbfd5f6, 0xcff4d5b9, 0xd029d57d, 0xd05ed541, 0xd094d505, 0xd0cad4ca, 0xd101d48f, 0xd138d454, 0xd16fd419, 0xd1a6d3df, 0xd1ded3a4, 0xd215d36b, 0xd24ed331, 0xd286d2f8, 0xd2bfd2bf, 0xd2f8d286, 0xd331d24e, 0xd36bd215, 0xd3a4d1de, 0xd3dfd1a6, 0xd419d16f, 0xd454d138, 0xd48fd101, 0xd4cad0ca, 0xd505d094, 0xd541d05e, 0xd57dd029, 0xd5b9cff4, 0xd5f6cfbf, 0xd632cf8a, 0xd66fcf56, 0xd6adcf21, 0xd6eaceee, 0xd728ceba, 0xd766ce87, 0xd7a4ce54, 0xd7e3ce22, 0xd822cdef, 0xd861cdbd, 0xd8a0cd8c, 0xd8e0cd5a, 0xd91fcd29, 0xd95fccf9, 0xd9a0ccc8, 0xd9e0cc98, 0xda21cc68, 0xda62cc39, 0xdaa3cc0a, 0xdae4cbdb, 0xdb26cbad, 0xdb68cb7f, 0xdbaacb51, 0xdbeccb23, 0xdc2fcaf6, 0xdc72cac9, 0xdcb5ca9d, 0xdcf8ca70, 0xdd3bca45, 0xdd7fca19, 0xddc3c9ee, 0xde07c9c3, 0xde4bc999, 0xde8fc96e, 0xded4c944, 0xdf19c91b, 0xdf5ec8f2, 0xdfa3c8c9, 0xdfe9c8a0, 0xe02ec878, 0xe074c850, 0xe0bac829, 0xe101c802, 0xe147c7db, 0xe18ec7b5, 0xe1d5c78f, 0xe21cc769, 0xe263c743, 0xe2aac71e, 0xe2f2c6fa, 0xe33ac6d5, 0xe381c6b1, 0xe3cac68e, 0xe412c66b, 0xe45ac648, 0xe4a3c625, 0xe4ecc603, 0xe535c5e1, 0xe57ec5bf, 0xe5c7c59e, 0xe611c57e, 0xe65ac55d, 0xe6a4c53d, 0xe6eec51d, 0xe738c4fe, 0xe782c4df, 0xe7ccc4c1, 0xe817c4a2, 0xe862c484, 0xe8adc467, 0xe8f7c44a, 0xe943c42d, 0xe98ec411, 0xe9d9c3f5, 0xea25c3d9, 0xea70c3be, 0xeabcc3a3, 0xeb08c388, 0xeb54c36e, 0xeba0c354, 0xebedc33b, 0xec39c322, 0xec86c309, 0xecd2c2f1, 0xed1fc2d9, 0xed6cc2c1, 0xedb9c2aa, 0xee06c294, 0xee53c27d, 0xeea1c267, 0xeeeec251, 0xef3cc23c, 0xef89c227, 0xefd7c213, 0xf025c1ff, 0xf073c1eb, 0xf0c1c1d8, 0xf10fc1c5, 0xf15dc1b2, 0xf1acc1a0, 0xf1fac18e, 0xf249c17d, 0xf297c16c, 0xf2e6c15b, 0xf335c14b, 0xf384c13b, 0xf3d3c12b, 0xf422c11c, 0xf471c10d, 0xf4c0c0ff, 0xf50fc0f1, 0xf55ec0e4, 0xf5aec0d6, 0xf5fdc0ca, 0xf64cc0bd, 0xf69cc0b1, 0xf6ecc0a6, 0xf73bc09b, 0xf78bc090, 0xf7dbc085, 0xf82ac07b, 0xf87ac072, 0xf8cac068, 0xf91ac05f, 0xf96ac057, 0xf9bac04f, 0xfa0ac047, 0xfa5ac040, 0xfaaac039, 0xfafbc033, 0xfb4bc02c, 0xfb9bc027, 0xfbebc021, 0xfc3bc01c, 0xfc8cc018, 0xfcdcc014, 0xfd2cc010, 0xfd7dc00d, 0xfdcdc00a, 0xfe1ec007, 0xfe6ec005, 0xfebec003, 0xff0fc002, 0xff5fc001, 0xffb0c000, 0x0000c000, 0x0050c000, 0x00a1c001, 0x00f1c002, 0x0142c003, 0x0192c005, 0x01e2c007, 0x0233c00a, 0x0283c00d, 0x02d4c010, 0x0324c014, 0x0374c018, 0x03c5c01c, 0x0415c021, 0x0465c027, 0x04b5c02c, 0x0505c033, 0x0556c039, 0x05a6c040, 0x05f6c047, 0x0646c04f, 0x0696c057, 0x06e6c05f, 0x0736c068, 0x0786c072, 0x07d6c07b, 0x0825c085, 0x0875c090, 0x08c5c09b, 0x0914c0a6, 0x0964c0b1, 0x09b4c0bd, 0x0a03c0ca, 0x0a52c0d6, 0x0aa2c0e4, 0x0af1c0f1, 0x0b40c0ff, 0x0b8fc10d, 0x0bdec11c, 0x0c2dc12b, 0x0c7cc13b, 0x0ccbc14b, 0x0d1ac15b, 0x0d69c16c, 0x0db7c17d, 0x0e06c18e, 0x0e54c1a0, 0x0ea3c1b2, 0x0ef1c1c5, 0x0f3fc1d8, 0x0f8dc1eb, 0x0fdbc1ff, 0x1029c213, 0x1077c227, 0x10c4c23c, 0x1112c251, 0x115fc267, 0x11adc27d, 0x11fac294, 0x1247c2aa, 0x1294c2c1, 0x12e1c2d9, 0x132ec2f1, 0x137ac309, 0x13c7c322, 0x1413c33b, 0x1460c354, 0x14acc36e, 0x14f8c388, 0x1544c3a3, 0x1590c3be, 0x15dbc3d9, 0x1627c3f5, 0x1672c411, 0x16bdc42d, 0x1709c44a, 0x1753c467, 0x179ec484, 0x17e9c4a2, 0x1834c4c1, 0x187ec4df, 0x18c8c4fe, 0x1912c51d, 0x195cc53d, 0x19a6c55d, 0x19efc57e, 0x1a39c59e, 0x1a82c5bf, 0x1acbc5e1, 0x1b14c603, 0x1b5dc625, 0x1ba6c648, 0x1beec66b, 0x1c36c68e, 0x1c7fc6b1, 0x1cc6c6d5, 0x1d0ec6fa, 0x1d56c71e, 0x1d9dc743, 0x1de4c769, 0x1e2bc78f, 0x1e72c7b5, 0x1eb9c7db, 0x1effc802, 0x1f46c829, 0x1f8cc850, 0x1fd2c878, 0x2017c8a0, 0x205dc8c9, 0x20a2c8f2, 0x20e7c91b, 0x212cc944, 0x2171c96e, 0x21b5c999, 0x21f9c9c3, 0x223dc9ee, 0x2281ca19, 0x22c5ca45, 0x2308ca70, 0x234bca9d, 0x238ecac9, 0x23d1caf6, 0x2414cb23, 0x2456cb51, 0x2498cb7f, 0x24dacbad, 0x251ccbdb, 0x255dcc0a, 0x259ecc39, 0x25dfcc68, 0x2620cc98, 0x2660ccc8, 0x26a1ccf9, 0x26e1cd29, 0x2720cd5a, 0x2760cd8c, 0x279fcdbd, 0x27decdef, 0x281dce22, 0x285cce54, 0x289ace87, 0x28d8ceba, 0x2916ceee, 0x2953cf21, 0x2991cf56, 0x29cecf8a, 0x2a0acfbf, 0x2a47cff4, 0x2a83d029, 0x2abfd05e, 0x2afbd094, 0x2b36d0ca, 0x2b71d101, 0x2bacd138, 0x2be7d16f, 0x2c21d1a6, 0x2c5cd1de, 0x2c95d215, 0x2ccfd24e, 0x2d08d286, 0x2d41d2bf, 0x2d7ad2f8, 0x2db2d331, 0x2debd36b, 0x2e22d3a4, 0x2e5ad3df, 0x2e91d419, 0x2ec8d454, 0x2effd48f, 0x2f36d4ca, 0x2f6cd505, 0x2fa2d541, 0x2fd7d57d, 0x300cd5b9, 0x3041d5f6, 0x3076d632, 0x30aad66f, 0x30dfd6ad, 0x3112d6ea, 0x3146d728, 0x3179d766, 0x31acd7a4, 0x31ded7e3, 0x3211d822, 0x3243d861, 0x3274d8a0, 0x32a6d8e0, 0x32d7d91f, 0x3307d95f, 0x3338d9a0, 0x3368d9e0, 0x3398da21, 0x33c7da62, 0x33f6daa3, 0x3425dae4, 0x3453db26, 0x3481db68, 0x34afdbaa, 0x34dddbec, 0x350adc2f, 0x3537dc72, 0x3563dcb5, 0x3590dcf8, 0x35bbdd3b, 0x35e7dd7f, 0x3612ddc3, 0x363dde07, 0x3667de4b, 0x3692de8f, 0x36bcded4, 0x36e5df19, 0x370edf5e, 0x3737dfa3, 0x3760dfe9, 0x3788e02e, 0x37b0e074, 0x37d7e0ba, 0x37fee101, 0x3825e147, 0x384be18e, 0x3871e1d5, 0x3897e21c, 0x38bde263, 0x38e2e2aa, 0x3906e2f2, 0x392be33a, 0x394fe381, 0x3972e3ca, 0x3995e412, 0x39b8e45a, 0x39dbe4a3, 0x39fde4ec, 0x3a1fe535, 0x3a41e57e, 0x3a62e5c7, 0x3a82e611, 0x3aa3e65a, 0x3ac3e6a4, 0x3ae3e6ee, 0x3b02e738, 0x3b21e782, 0x3b3fe7cc, 0x3b5ee817, 0x3b7ce862, 0x3b99e8ad, 0x3bb6e8f7, 0x3bd3e943, 0x3befe98e, 0x3c0be9d9, 0x3c27ea25, 0x3c42ea70, 0x3c5deabc, 0x3c78eb08, 0x3c92eb54, 0x3caceba0, 0x3cc5ebed, 0x3cdeec39, 0x3cf7ec86, 0x3d0fecd2, 0x3d27ed1f, 0x3d3fed6c, 0x3d56edb9, 0x3d6cee06, 0x3d83ee53, 0x3d99eea1, 0x3dafeeee, 0x3dc4ef3c, 0x3dd9ef89, 0x3dedefd7, 0x3e01f025, 0x3e15f073, 0x3e28f0c1, 0x3e3bf10f, 0x3e4ef15d, 0x3e60f1ac, 0x3e72f1fa, 0x3e83f249, 0x3e94f297, 0x3ea5f2e6, 0x3eb5f335, 0x3ec5f384, 0x3ed5f3d3, 0x3ee4f422, 0x3ef3f471, 0x3f01f4c0, 0x3f0ff50f, 0x3f1cf55e, 0x3f2af5ae, 0x3f36f5fd, 0x3f43f64c, 0x3f4ff69c, 0x3f5af6ec, 0x3f65f73b, 0x3f70f78b, 0x3f7bf7db, 0x3f85f82a, 0x3f8ef87a, 0x3f98f8ca, 0x3fa1f91a, 0x3fa9f96a, 0x3fb1f9ba, 0x3fb9fa0a, 0x3fc0fa5a, 0x3fc7faaa, 0x3fcdfafb, 0x3fd4fb4b, 0x3fd9fb9b, 0x3fdffbeb, 0x3fe4fc3b, 0x3fe8fc8c, 0x3fecfcdc, 0x3ff0fd2c, 0x3ff3fd7d, 0x3ff6fdcd, 0x3ff9fe1e, 0x3ffbfe6e, 0x3ffdfebe, 0x3ffeff0f, 0x3fffff5f, 0x4000ffb0, 0x40000000, 0x40000050, 0x3fff00a1, 0x3ffe00f1, 0x3ffd0142, 0x3ffb0192, 0x3ff901e2, 0x3ff60233, 0x3ff30283, 0x3ff002d4, 0x3fec0324, 0x3fe80374, 0x3fe403c5, 0x3fdf0415, 0x3fd90465, 0x3fd404b5, 0x3fcd0505, 0x3fc70556, 0x3fc005a6, 0x3fb905f6, 0x3fb10646, 0x3fa90696, 0x3fa106e6, 0x3f980736, 0x3f8e0786, 0x3f8507d6, 0x3f7b0825, 0x3f700875, 0x3f6508c5, 0x3f5a0914, 0x3f4f0964, 0x3f4309b4, 0x3f360a03, 0x3f2a0a52, 0x3f1c0aa2, 0x3f0f0af1, 0x3f010b40, 0x3ef30b8f, 0x3ee40bde, 0x3ed50c2d, 0x3ec50c7c, 0x3eb50ccb, 0x3ea50d1a, 0x3e940d69, 0x3e830db7, 0x3e720e06, 0x3e600e54, 0x3e4e0ea3, 0x3e3b0ef1, 0x3e280f3f, 0x3e150f8d, 0x3e010fdb, 0x3ded1029, 0x3dd91077, 0x3dc410c4, 0x3daf1112, 0x3d99115f, 0x3d8311ad, 0x3d6c11fa, 0x3d561247, 0x3d3f1294, 0x3d2712e1, 0x3d0f132e, 0x3cf7137a, 0x3cde13c7, 0x3cc51413, 0x3cac1460, 0x3c9214ac, 0x3c7814f8, 0x3c5d1544, 0x3c421590, 0x3c2715db, 0x3c0b1627, 0x3bef1672, 0x3bd316bd, 0x3bb61709, 0x3b991753, 0x3b7c179e, 0x3b5e17e9, 0x3b3f1834, 0x3b21187e, 0x3b0218c8, 0x3ae31912, 0x3ac3195c, 0x3aa319a6, 0x3a8219ef, 0x3a621a39, 0x3a411a82, 0x3a1f1acb, 0x39fd1b14, 0x39db1b5d, 0x39b81ba6, 0x39951bee, 0x39721c36, 0x394f1c7f, 0x392b1cc6, 0x39061d0e, 0x38e21d56, 0x38bd1d9d, 0x38971de4, 0x38711e2b, 0x384b1e72, 0x38251eb9, 0x37fe1eff, 0x37d71f46, 0x37b01f8c, 0x37881fd2, 0x37602017, 0x3737205d, 0x370e20a2, 0x36e520e7, 0x36bc212c, 0x36922171, 0x366721b5, 0x363d21f9, 0x3612223d, 0x35e72281, 0x35bb22c5, 0x35902308, 0x3563234b, 0x3537238e, 0x350a23d1, 0x34dd2414, 0x34af2456, 0x34812498, 0x345324da, 0x3425251c, 0x33f6255d, 0x33c7259e, 0x339825df, 0x33682620, 0x33382660, 0x330726a1, 0x32d726e1, 0x32a62720, 0x32742760, 0x3243279f, 0x321127de, 0x31de281d, 0x31ac285c, 0x3179289a, 0x314628d8, 0x31122916, 0x30df2953, 0x30aa2991, 0x307629ce, 0x30412a0a, 0x300c2a47, 0x2fd72a83, 0x2fa22abf, 0x2f6c2afb, 0x2f362b36, 0x2eff2b71, 0x2ec82bac, 0x2e912be7, 0x2e5a2c21, 0x2e222c5c, 0x2deb2c95, 0x2db22ccf, 0x2d7a2d08, 0x2d412d41, 0x2d082d7a, 0x2ccf2db2, 0x2c952deb, 0x2c5c2e22, 0x2c212e5a, 0x2be72e91, 0x2bac2ec8, 0x2b712eff, 0x2b362f36, 0x2afb2f6c, 0x2abf2fa2, 0x2a832fd7, 0x2a47300c, 0x2a0a3041, 0x29ce3076, 0x299130aa, 0x295330df, 0x29163112, 0x28d83146, 0x289a3179, 0x285c31ac, 0x281d31de, 0x27de3211, 0x279f3243, 0x27603274, 0x272032a6, 0x26e132d7, 0x26a13307, 0x26603338, 0x26203368, 0x25df3398, 0x259e33c7, 0x255d33f6, 0x251c3425, 0x24da3453, 0x24983481, 0x245634af, 0x241434dd, 0x23d1350a, 0x238e3537, 0x234b3563, 0x23083590, 0x22c535bb, 0x228135e7, 0x223d3612, 0x21f9363d, 0x21b53667, 0x21713692, 0x212c36bc, 0x20e736e5, 0x20a2370e, 0x205d3737, 0x20173760, 0x1fd23788, 0x1f8c37b0, 0x1f4637d7, 0x1eff37fe, 0x1eb93825, 0x1e72384b, 0x1e2b3871, 0x1de43897, 0x1d9d38bd, 0x1d5638e2, 0x1d0e3906, 0x1cc6392b, 0x1c7f394f, 0x1c363972, 0x1bee3995, 0x1ba639b8, 0x1b5d39db, 0x1b1439fd, 0x1acb3a1f, 0x1a823a41, 0x1a393a62, 0x19ef3a82, 0x19a63aa3, 0x195c3ac3, 0x19123ae3, 0x18c83b02, 0x187e3b21, 0x18343b3f, 0x17e93b5e, 0x179e3b7c, 0x17533b99, 0x17093bb6, 0x16bd3bd3, 0x16723bef, 0x16273c0b, 0x15db3c27, 0x15903c42, 0x15443c5d, 0x14f83c78, 0x14ac3c92, 0x14603cac, 0x14133cc5, 0x13c73cde, 0x137a3cf7, 0x132e3d0f, 0x12e13d27, 0x12943d3f, 0x12473d56, 0x11fa3d6c, 0x11ad3d83, 0x115f3d99, 0x11123daf, 0x10c43dc4, 0x10773dd9, 0x10293ded, 0x0fdb3e01, 0x0f8d3e15, 0x0f3f3e28, 0x0ef13e3b, 0x0ea33e4e, 0x0e543e60, 0x0e063e72, 0x0db73e83, 0x0d693e94, 0x0d1a3ea5, 0x0ccb3eb5, 0x0c7c3ec5, 0x0c2d3ed5, 0x0bde3ee4, 0x0b8f3ef3, 0x0b403f01, 0x0af13f0f, 0x0aa23f1c, 0x0a523f2a, 0x0a033f36, 0x09b43f43, 0x09643f4f, 0x09143f5a, 0x08c53f65, 0x08753f70, 0x08253f7b, 0x07d63f85, 0x07863f8e, 0x07363f98, 0x06e63fa1, 0x06963fa9, 0x06463fb1, 0x05f63fb9, 0x05a63fc0, 0x05563fc7, 0x05053fcd, 0x04b53fd4, 0x04653fd9, 0x04153fdf, 0x03c53fe4, 0x03743fe8, 0x03243fec, 0x02d43ff0, 0x02833ff3, 0x02333ff6, 0x01e23ff9, 0x01923ffb, 0x01423ffd, 0x00f13ffe, 0x00a13fff, 0x00504000,
                                                       0x00004000, 0xffb04000, 0xff5f3fff, 0xff0f3ffe, 0xfebe3ffd, 0xfe6e3ffb, 0xfe1e3ff9, 0xfdcd3ff6, 0xfd7d3ff3, 0xfd2c3ff0, 0xfcdc3fec, 0xfc8c3fe8, 0xfc3b3fe4, 0xfbeb3fdf, 0xfb9b3fd9, 0xfb4b3fd4, 0xfafb3fcd, 0xfaaa3fc7, 0xfa5a3fc0, 0xfa0a3fb9, 0xf9ba3fb1, 0xf96a3fa9, 0xf91a3fa1, 0xf8ca3f98, 0xf87a3f8e, 0xf82a3f85, 0xf7db3f7b, 0xf78b3f70, 0xf73b3f65, 0xf6ec3f5a, 0xf69c3f4f, 0xf64c3f43, 0xf5fd3f36, 0xf5ae3f2a, 0xf55e3f1c, 0xf50f3f0f, 0xf4c03f01, 0xf4713ef3, 0xf4223ee4, 0xf3d33ed5, 0xf3843ec5, 0xf3353eb5, 0xf2e63ea5, 0xf2973e94, 0xf2493e83, 0xf1fa3e72, 0xf1ac3e60, 0xf15d3e4e, 0xf10f3e3b, 0xf0c13e28, 0xf0733e15, 0xf0253e01, 0xefd73ded, 0xef893dd9, 0xef3c3dc4, 0xeeee3daf, 0xeea13d99, 0xee533d83, 0xee063d6c, 0xedb93d56, 0xed6c3d3f, 0xed1f3d27, 0xecd23d0f, 0xec863cf7, 0xec393cde, 0xebed3cc5, 0xeba03cac, 0xeb543c92, 0xeb083c78, 0xeabc3c5d, 0xea703c42, 0xea253c27, 0xe9d93c0b, 0xe98e3bef, 0xe9433bd3, 0xe8f73bb6, 0xe8ad3b99, 0xe8623b7c, 0xe8173b5e, 0xe7cc3b3f, 0xe7823b21, 0xe7383b02, 0xe6ee3ae3, 0xe6a43ac3, 0xe65a3aa3, 0xe6113a82, 0xe5c73a62, 0xe57e3a41, 0xe5353a1f, 0xe4ec39fd, 0xe4a339db, 0xe45a39b8, 0xe4123995, 0xe3ca3972, 0xe381394f, 0xe33a392b, 0xe2f23906, 0xe2aa38e2, 0xe26338bd, 0xe21c3897, 0xe1d53871, 0xe18e384b, 0xe1473825, 0xe10137fe, 0xe0ba37d7, 0xe07437b0, 0xe02e3788, 0xdfe93760, 0xdfa33737, 0xdf5e370e, 0xdf1936e5, 0xded436bc, 0xde8f3692, 0xde4b3667, 0xde07363d, 0xddc33612, 0xdd7f35e7, 0xdd3b35bb, 0xdcf83590, 0xdcb53563, 0xdc723537, 0xdc2f350a, 0xdbec34dd, 0xdbaa34af, 0xdb683481, 0xdb263453, 0xdae43425, 0xdaa333f6, 0xda6233c7, 0xda213398, 0xd9e03368, 0xd9a03338, 0xd95f3307, 0xd91f32d7, 0xd8e032a6, 0xd8a03274, 0xd8613243, 0xd8223211, 0xd7e331de, 0xd7a431ac, 0xd7663179, 0xd7283146, 0xd6ea3112, 0xd6ad30df, 0xd66f30aa, 0xd6323076, 0xd5f63041, 0xd5b9300c, 0xd57d2fd7, 0xd5412fa2, 0xd5052f6c, 0xd4ca2f36, 0xd48f2eff, 0xd4542ec8, 0xd4192e91, 0xd3df2e5a, 0xd3a42e22, 0xd36b2deb, 0xd3312db2, 0xd2f82d7a, 0xd2bf2d41, 0xd2862d08, 0xd24e2ccf, 0xd2152c95, 0xd1de2c5c, 0xd1a62c21, 0xd16f2be7, 0xd1382bac, 0xd1012b71, 0xd0ca2b36, 0xd0942afb, 0xd05e2abf, 0xd0292a83, 0xcff42a47, 0xcfbf2a0a, 0xcf8a29ce, 0xcf562991, 0xcf212953, 0xceee2916, 0xceba28d8, 0xce87289a, 0xce54285c, 0xce22281d, 0xcdef27de, 0xcdbd279f, 0xcd8c2760, 0xcd5a2720, 0xcd2926e1, 0xccf926a1, 0xccc82660, 0xcc982620, 0xcc6825df, 0xcc39259e, 0xcc0a255d, 0xcbdb251c, 0xcbad24da, 0xcb7f2498, 0xcb512456, 0xcb232414, 0xcaf623d1, 0xcac9238e, 0xca9d234b, 0xca702308, 0xca4522c5, 0xca192281, 0xc9ee223d, 0xc9c321f9, 0xc99921b5, 0xc96e2171, 0xc944212c, 0xc91b20e7, 0xc8f220a2, 0xc8c9205d, 0xc8a02017, 0xc8781fd2, 0xc8501f8c, 0xc8291f46, 0xc8021eff, 0xc7db1eb9, 0xc7b51e72, 0xc78f1e2b, 0xc7691de4, 0xc7431d9d, 0xc71e1d56, 0xc6fa1d0e, 0xc6d51cc6, 0xc6b11c7f, 0xc68e1c36, 0xc66b1bee, 0xc6481ba6, 0xc6251b5d, 0xc6031b14, 0xc5e11acb, 0xc5bf1a82, 0xc59e1a39, 0xc57e19ef, 0xc55d19a6, 0xc53d195c, 0xc51d1912, 0xc4fe18c8, 0xc4df187e, 0xc4c11834, 0xc4a217e9, 0xc484179e, 0xc4671753, 0xc44a1709, 0xc42d16bd, 0xc4111672, 0xc3f51627, 0xc3d915db, 0xc3be1590, 0xc3a31544, 0xc38814f8, 0xc36e14ac, 0xc3541460, 0xc33b1413, 0xc32213c7, 0xc309137a, 0xc2f1132e, 0xc2d912e1, 0xc2c11294, 0xc2aa1247, 0xc29411fa, 0xc27d11ad, 0xc267115f, 0xc2511112, 0xc23c10c4, 0xc2271077, 0xc2131029, 0xc1ff0fdb, 0xc1eb0f8d, 0xc1d80f3f, 0xc1c50ef1, 0xc1b20ea3, 0xc1a00e54, 0xc18e0e06, 0xc17d0db7, 0xc16c0d69, 0xc15b0d1a, 0xc14b0ccb, 0xc13b0c7c, 0xc12b0c2d, 0xc11c0bde, 0xc10d0b8f, 0xc0ff0b40, 0xc0f10af1, 0xc0e40aa2, 0xc0d60a52, 0xc0ca0a03, 0xc0bd09b4, 0xc0b10964, 0xc0a60914, 0xc09b08c5, 0xc0900875, 0xc0850825, 0xc07b07d6, 0xc0720786, 0xc0680736, 0xc05f06e6, 0xc0570696, 0xc04f0646, 0xc04705f6, 0xc04005a6, 0xc0390556, 0xc0330505, 0xc02c04b5, 0xc0270465, 0xc0210415, 0xc01c03c5, 0xc0180374, 0xc0140324, 0xc01002d4, 0xc00d0283, 0xc00a0233, 0xc00701e2, 0xc0050192, 0xc0030142, 0xc00200f1, 0xc00100a1, 0xc0000050, 0xc0000000, 0xc000ffb0, 0xc001ff5f, 0xc002ff0f, 0xc003febe, 0xc005fe6e, 0xc007fe1e, 0xc00afdcd, 0xc00dfd7d, 0xc010fd2c, 0xc014fcdc, 0xc018fc8c, 0xc01cfc3b, 0xc021fbeb, 0xc027fb9b, 0xc02cfb4b, 0xc033fafb, 0xc039faaa, 0xc040fa5a, 0xc047fa0a, 0xc04ff9ba, 0xc057f96a, 0xc05ff91a, 0xc068f8ca, 0xc072f87a, 0xc07bf82a, 0xc085f7db, 0xc090f78b, 0xc09bf73b, 0xc0a6f6ec, 0xc0b1f69c, 0xc0bdf64c, 0xc0caf5fd, 0xc0d6f5ae, 0xc0e4f55e, 0xc0f1f50f, 0xc0fff4c0, 0xc10df471, 0xc11cf422, 0xc12bf3d3, 0xc13bf384, 0xc14bf335, 0xc15bf2e6, 0xc16cf297, 0xc17df249, 0xc18ef1fa, 0xc1a0f1ac, 0xc1b2f15d, 0xc1c5f10f, 0xc1d8f0c1, 0xc1ebf073, 0xc1fff025, 0xc213efd7, 0xc227ef89, 0xc23cef3c, 0xc251eeee, 0xc267eea1, 0xc27dee53, 0xc294ee06, 0xc2aaedb9, 0xc2c1ed6c, 0xc2d9ed1f, 0xc2f1ecd2, 0xc309ec86, 0xc322ec39, 0xc33bebed, 0xc354eba0, 0xc36eeb54, 0xc388eb08, 0xc3a3eabc, 0xc3beea70, 0xc3d9ea25, 0xc3f5e9d9, 0xc411e98e, 0xc42de943, 0xc44ae8f7, 0xc467e8ad, 0xc484e862, 0xc4a2e817, 0xc4c1e7cc, 0xc4dfe782, 0xc4fee738, 0xc51de6ee, 0xc53de6a4, 0xc55de65a, 0xc57ee611, 0xc59ee5c7, 0xc5bfe57e, 0xc5e1e535, 0xc603e4ec, 0xc625e4a3, 0xc648e45a, 0xc66be412, 0xc68ee3ca, 0xc6b1e381, 0xc6d5e33a, 0xc6fae2f2, 0xc71ee2aa, 0xc743e263, 0xc769e21c, 0xc78fe1d5, 0xc7b5e18e, 0xc7dbe147, 0xc802e101, 0xc829e0ba, 0xc850e074, 0xc878e02e, 0xc8a0dfe9, 0xc8c9dfa3, 0xc8f2df5e, 0xc91bdf19, 0xc944ded4, 0xc96ede8f, 0xc999de4b, 0xc9c3de07, 0xc9eeddc3, 0xca19dd7f, 0xca45dd3b, 0xca70dcf8, 0xca9ddcb5, 0xcac9dc72, 0xcaf6dc2f, 0xcb23dbec, 0xcb51dbaa, 0xcb7fdb68, 0xcbaddb26, 0xcbdbdae4, 0xcc0adaa3, 0xcc39da62, 0xcc68da21, 0xcc98d9e0, 0xccc8d9a0, 0xccf9d95f, 0xcd29d91f, 0xcd5ad8e0, 0xcd8cd8a0, 0xcdbdd861, 0xcdefd822, 0xce22d7e3, 0xce54d7a4, 0xce87d766, 0xcebad728, 0xceeed6ea, 0xcf21d6ad, 0xcf56d66f, 0xcf8ad632, 0xcfbfd5f6, 0xcff4d5b9, 0xd029d57d, 0xd05ed541, 0xd094d505, 0xd0cad4ca, 0xd101d48f, 0xd138d454, 0xd16fd419, 0xd1a6d3df, 0xd1ded3a4, 0xd215d36b, 0xd24ed331, 0xd286d2f8, 0xd2bfd2bf, 0xd2f8d286, 0xd331d24e, 0xd36bd215, 0xd3a4d1de, 0xd3dfd1a6, 0xd419d16f, 0xd454d138, 0xd48fd101, 0xd4cad0ca, 0xd505d094, 0xd541d05e, 0xd57dd029, 0xd5b9cff4, 0xd5f6cfbf, 0xd632cf8a, 0xd66fcf56, 0xd6adcf21, 0xd6eaceee, 0xd728ceba, 0xd766ce87, 0xd7a4ce54, 0xd7e3ce22, 0xd822cdef, 0xd861cdbd, 0xd8a0cd8c, 0xd8e0cd5a, 0xd91fcd29, 0xd95fccf9, 0xd9a0ccc8, 0xd9e0cc98, 0xda21cc68, 0xda62cc39, 0xdaa3cc0a, 0xdae4cbdb, 0xdb26cbad, 0xdb68cb7f, 0xdbaacb51, 0xdbeccb23, 0xdc2fcaf6, 0xdc72cac9, 0xdcb5ca9d, 0xdcf8ca70, 0xdd3bca45, 0xdd7fca19, 0xddc3c9ee, 0xde07c9c3, 0xde4bc999, 0xde8fc96e, 0xded4c944, 0xdf19c91b, 0xdf5ec8f2, 0xdfa3c8c9, 0xdfe9c8a0, 0xe02ec878, 0xe074c850, 0xe0bac829, 0xe101c802, 0xe147c7db, 0xe18ec7b5, 0xe1d5c78f, 0xe21cc769, 0xe263c743, 0xe2aac71e, 0xe2f2c6fa, 0xe33ac6d5, 0xe381c6b1, 0xe3cac68e, 0xe412c66b, 0xe45ac648, 0xe4a3c625, 0xe4ecc603, 0xe535c5e1, 0xe57ec5bf, 0xe5c7c59e, 0xe611c57e, 0xe65ac55d, 0xe6a4c53d, 0xe6eec51d, 0xe738c4fe, 0xe782c4df, 0xe7ccc4c1, 0xe817c4a2, 0xe862c484, 0xe8adc467, 0xe8f7c44a, 0xe943c42d, 0xe98ec411, 0xe9d9c3f5, 0xea25c3d9, 0xea70c3be, 0xeabcc3a3, 0xeb08c388, 0xeb54c36e, 0xeba0c354, 0xebedc33b, 0xec39c322, 0xec86c309, 0xecd2c2f1, 0xed1fc2d9, 0xed6cc2c1, 0xedb9c2aa, 0xee06c294, 0xee53c27d, 0xeea1c267, 0xeeeec251, 0xef3cc23c, 0xef89c227, 0xefd7c213, 0xf025c1ff, 0xf073c1eb, 0xf0c1c1d8, 0xf10fc1c5, 0xf15dc1b2, 0xf1acc1a0, 0xf1fac18e, 0xf249c17d, 0xf297c16c, 0xf2e6c15b, 0xf335c14b, 0xf384c13b, 0xf3d3c12b, 0xf422c11c, 0xf471c10d, 0xf4c0c0ff, 0xf50fc0f1, 0xf55ec0e4, 0xf5aec0d6, 0xf5fdc0ca, 0xf64cc0bd, 0xf69cc0b1, 0xf6ecc0a6, 0xf73bc09b, 0xf78bc090, 0xf7dbc085, 0xf82ac07b, 0xf87ac072, 0xf8cac068, 0xf91ac05f, 0xf96ac057, 0xf9bac04f, 0xfa0ac047, 0xfa5ac040, 0xfaaac039, 0xfafbc033, 0xfb4bc02c, 0xfb9bc027, 0xfbebc021, 0xfc3bc01c, 0xfc8cc018, 0xfcdcc014, 0xfd2cc010, 0xfd7dc00d, 0xfdcdc00a, 0xfe1ec007, 0xfe6ec005, 0xfebec003, 0xff0fc002, 0xff5fc001, 0xffb0c000, 0x0000c000, 0x0050c000, 0x00a1c001, 0x00f1c002, 0x0142c003, 0x0192c005, 0x01e2c007, 0x0233c00a, 0x0283c00d, 0x02d4c010, 0x0324c014, 0x0374c018, 0x03c5c01c, 0x0415c021, 0x0465c027, 0x04b5c02c, 0x0505c033, 0x0556c039, 0x05a6c040, 0x05f6c047, 0x0646c04f, 0x0696c057, 0x06e6c05f, 0x0736c068, 0x0786c072, 0x07d6c07b, 0x0825c085, 0x0875c090, 0x08c5c09b, 0x0914c0a6, 0x0964c0b1, 0x09b4c0bd, 0x0a03c0ca, 0x0a52c0d6, 0x0aa2c0e4, 0x0af1c0f1, 0x0b40c0ff, 0x0b8fc10d, 0x0bdec11c, 0x0c2dc12b, 0x0c7cc13b, 0x0ccbc14b, 0x0d1ac15b, 0x0d69c16c, 0x0db7c17d, 0x0e06c18e, 0x0e54c1a0, 0x0ea3c1b2, 0x0ef1c1c5, 0x0f3fc1d8, 0x0f8dc1eb, 0x0fdbc1ff, 0x1029c213, 0x1077c227, 0x10c4c23c, 0x1112c251, 0x115fc267, 0x11adc27d, 0x11fac294, 0x1247c2aa, 0x1294c2c1, 0x12e1c2d9, 0x132ec2f1, 0x137ac309, 0x13c7c322, 0x1413c33b, 0x1460c354, 0x14acc36e, 0x14f8c388, 0x1544c3a3, 0x1590c3be, 0x15dbc3d9, 0x1627c3f5, 0x1672c411, 0x16bdc42d, 0x1709c44a, 0x1753c467, 0x179ec484, 0x17e9c4a2, 0x1834c4c1, 0x187ec4df, 0x18c8c4fe, 0x1912c51d, 0x195cc53d, 0x19a6c55d, 0x19efc57e, 0x1a39c59e, 0x1a82c5bf, 0x1acbc5e1, 0x1b14c603, 0x1b5dc625, 0x1ba6c648, 0x1beec66b, 0x1c36c68e, 0x1c7fc6b1, 0x1cc6c6d5, 0x1d0ec6fa, 0x1d56c71e, 0x1d9dc743, 0x1de4c769, 0x1e2bc78f, 0x1e72c7b5, 0x1eb9c7db, 0x1effc802, 0x1f46c829, 0x1f8cc850, 0x1fd2c878, 0x2017c8a0, 0x205dc8c9, 0x20a2c8f2, 0x20e7c91b, 0x212cc944, 0x2171c96e, 0x21b5c999, 0x21f9c9c3, 0x223dc9ee, 0x2281ca19, 0x22c5ca45, 0x2308ca70, 0x234bca9d, 0x238ecac9, 0x23d1caf6, 0x2414cb23, 0x2456cb51, 0x2498cb7f, 0x24dacbad, 0x251ccbdb, 0x255dcc0a, 0x259ecc39, 0x25dfcc68, 0x2620cc98, 0x2660ccc8, 0x26a1ccf9, 0x26e1cd29, 0x2720cd5a, 0x2760cd8c, 0x279fcdbd, 0x27decdef, 0x281dce22, 0x285cce54, 0x289ace87, 0x28d8ceba, 0x2916ceee, 0x2953cf21, 0x2991cf56, 0x29cecf8a, 0x2a0acfbf, 0x2a47cff4, 0x2a83d029, 0x2abfd05e, 0x2afbd094, 0x2b36d0ca, 0x2b71d101, 0x2bacd138, 0x2be7d16f, 0x2c21d1a6, 0x2c5cd1de, 0x2c95d215, 0x2ccfd24e, 0x2d08d286, 0x2d41d2bf, 0x2d7ad2f8, 0x2db2d331, 0x2debd36b, 0x2e22d3a4, 0x2e5ad3df, 0x2e91d419, 0x2ec8d454, 0x2effd48f, 0x2f36d4ca, 0x2f6cd505, 0x2fa2d541, 0x2fd7d57d, 0x300cd5b9, 0x3041d5f6, 0x3076d632, 0x30aad66f, 0x30dfd6ad, 0x3112d6ea, 0x3146d728, 0x3179d766, 0x31acd7a4, 0x31ded7e3, 0x3211d822, 0x3243d861, 0x3274d8a0, 0x32a6d8e0, 0x32d7d91f, 0x3307d95f, 0x3338d9a0, 0x3368d9e0, 0x3398da21, 0x33c7da62, 0x33f6daa3, 0x3425dae4, 0x3453db26, 0x3481db68, 0x34afdbaa, 0x34dddbec, 0x350adc2f, 0x3537dc72, 0x3563dcb5, 0x3590dcf8, 0x35bbdd3b, 0x35e7dd7f, 0x3612ddc3, 0x363dde07, 0x3667de4b, 0x3692de8f, 0x36bcded4, 0x36e5df19, 0x370edf5e, 0x3737dfa3, 0x3760dfe9, 0x3788e02e, 0x37b0e074, 0x37d7e0ba, 0x37fee101, 0x3825e147, 0x384be18e, 0x3871e1d5, 0x3897e21c, 0x38bde263, 0x38e2e2aa, 0x3906e2f2, 0x392be33a, 0x394fe381, 0x3972e3ca, 0x3995e412, 0x39b8e45a, 0x39dbe4a3, 0x39fde4ec, 0x3a1fe535, 0x3a41e57e, 0x3a62e5c7, 0x3a82e611, 0x3aa3e65a, 0x3ac3e6a4, 0x3ae3e6ee, 0x3b02e738, 0x3b21e782, 0x3b3fe7cc, 0x3b5ee817, 0x3b7ce862, 0x3b99e8ad, 0x3bb6e8f7, 0x3bd3e943, 0x3befe98e, 0x3c0be9d9, 0x3c27ea25, 0x3c42ea70, 0x3c5deabc, 0x3c78eb08, 0x3c92eb54, 0x3caceba0, 0x3cc5ebed, 0x3cdeec39, 0x3cf7ec86, 0x3d0fecd2, 0x3d27ed1f, 0x3d3fed6c, 0x3d56edb9, 0x3d6cee06, 0x3d83ee53, 0x3d99eea1, 0x3dafeeee, 0x3dc4ef3c, 0x3dd9ef89, 0x3dedefd7, 0x3e01f025, 0x3e15f073, 0x3e28f0c1, 0x3e3bf10f, 0x3e4ef15d, 0x3e60f1ac, 0x3e72f1fa, 0x3e83f249, 0x3e94f297, 0x3ea5f2e6, 0x3eb5f335, 0x3ec5f384, 0x3ed5f3d3, 0x3ee4f422, 0x3ef3f471, 0x3f01f4c0, 0x3f0ff50f, 0x3f1cf55e, 0x3f2af5ae, 0x3f36f5fd, 0x3f43f64c, 0x3f4ff69c, 0x3f5af6ec, 0x3f65f73b, 0x3f70f78b, 0x3f7bf7db, 0x3f85f82a, 0x3f8ef87a, 0x3f98f8ca, 0x3fa1f91a, 0x3fa9f96a, 0x3fb1f9ba, 0x3fb9fa0a, 0x3fc0fa5a, 0x3fc7faaa, 0x3fcdfafb, 0x3fd4fb4b, 0x3fd9fb9b, 0x3fdffbeb, 0x3fe4fc3b, 0x3fe8fc8c, 0x3fecfcdc, 0x3ff0fd2c, 0x3ff3fd7d, 0x3ff6fdcd, 0x3ff9fe1e, 0x3ffbfe6e, 0x3ffdfebe, 0x3ffeff0f, 0x3fffff5f, 0x4000ffb0, 0x40000000, 0x40000050, 0x3fff00a1, 0x3ffe00f1, 0x3ffd0142, 0x3ffb0192, 0x3ff901e2, 0x3ff60233, 0x3ff30283, 0x3ff002d4, 0x3fec0324, 0x3fe80374, 0x3fe403c5, 0x3fdf0415, 0x3fd90465, 0x3fd404b5, 0x3fcd0505, 0x3fc70556, 0x3fc005a6, 0x3fb905f6, 0x3fb10646, 0x3fa90696, 0x3fa106e6, 0x3f980736, 0x3f8e0786, 0x3f8507d6, 0x3f7b0825, 0x3f700875, 0x3f6508c5, 0x3f5a0914, 0x3f4f0964, 0x3f4309b4, 0x3f360a03, 0x3f2a0a52, 0x3f1c0aa2, 0x3f0f0af1, 0x3f010b40, 0x3ef30b8f, 0x3ee40bde, 0x3ed50c2d, 0x3ec50c7c, 0x3eb50ccb, 0x3ea50d1a, 0x3e940d69, 0x3e830db7, 0x3e720e06, 0x3e600e54, 0x3e4e0ea3, 0x3e3b0ef1, 0x3e280f3f, 0x3e150f8d, 0x3e010fdb, 0x3ded1029, 0x3dd91077, 0x3dc410c4, 0x3daf1112, 0x3d99115f, 0x3d8311ad, 0x3d6c11fa, 0x3d561247, 0x3d3f1294, 0x3d2712e1, 0x3d0f132e, 0x3cf7137a, 0x3cde13c7, 0x3cc51413, 0x3cac1460, 0x3c9214ac, 0x3c7814f8, 0x3c5d1544, 0x3c421590, 0x3c2715db, 0x3c0b1627, 0x3bef1672, 0x3bd316bd, 0x3bb61709, 0x3b991753, 0x3b7c179e, 0x3b5e17e9, 0x3b3f1834, 0x3b21187e, 0x3b0218c8, 0x3ae31912, 0x3ac3195c, 0x3aa319a6, 0x3a8219ef, 0x3a621a39, 0x3a411a82, 0x3a1f1acb, 0x39fd1b14, 0x39db1b5d, 0x39b81ba6, 0x39951bee, 0x39721c36, 0x394f1c7f, 0x392b1cc6, 0x39061d0e, 0x38e21d56, 0x38bd1d9d, 0x38971de4, 0x38711e2b, 0x384b1e72, 0x38251eb9, 0x37fe1eff, 0x37d71f46, 0x37b01f8c, 0x37881fd2, 0x37602017, 0x3737205d, 0x370e20a2, 0x36e520e7, 0x36bc212c, 0x36922171, 0x366721b5, 0x363d21f9, 0x3612223d, 0x35e72281, 0x35bb22c5, 0x35902308, 0x3563234b, 0x3537238e, 0x350a23d1, 0x34dd2414, 0x34af2456, 0x34812498, 0x345324da, 0x3425251c, 0x33f6255d, 0x33c7259e, 0x339825df, 0x33682620, 0x33382660, 0x330726a1, 0x32d726e1, 0x32a62720, 0x32742760, 0x3243279f, 0x321127de, 0x31de281d, 0x31ac285c, 0x3179289a, 0x314628d8, 0x31122916, 0x30df2953, 0x30aa2991, 0x307629ce, 0x30412a0a, 0x300c2a47, 0x2fd72a83, 0x2fa22abf, 0x2f6c2afb, 0x2f362b36, 0x2eff2b71, 0x2ec82bac, 0x2e912be7, 0x2e5a2c21, 0x2e222c5c, 0x2deb2c95, 0x2db22ccf, 0x2d7a2d08, 0x2d412d41, 0x2d082d7a, 0x2ccf2db2, 0x2c952deb, 0x2c5c2e22, 0x2c212e5a, 0x2be72e91, 0x2bac2ec8, 0x2b712eff, 0x2b362f36, 0x2afb2f6c, 0x2abf2fa2, 0x2a832fd7, 0x2a47300c, 0x2a0a3041, 0x29ce3076, 0x299130aa, 0x295330df, 0x29163112, 0x28d83146, 0x289a3179, 0x285c31ac, 0x281d31de, 0x27de3211, 0x279f3243, 0x27603274, 0x272032a6, 0x26e132d7, 0x26a13307, 0x26603338, 0x26203368, 0x25df3398, 0x259e33c7, 0x255d33f6, 0x251c3425, 0x24da3453, 0x24983481, 0x245634af, 0x241434dd, 0x23d1350a, 0x238e3537, 0x234b3563, 0x23083590, 0x22c535bb, 0x228135e7, 0x223d3612, 0x21f9363d, 0x21b53667, 0x21713692, 0x212c36bc, 0x20e736e5, 0x20a2370e, 0x205d3737, 0x20173760, 0x1fd23788, 0x1f8c37b0, 0x1f4637d7, 0x1eff37fe, 0x1eb93825, 0x1e72384b, 0x1e2b3871, 0x1de43897, 0x1d9d38bd, 0x1d5638e2, 0x1d0e3906, 0x1cc6392b, 0x1c7f394f, 0x1c363972, 0x1bee3995, 0x1ba639b8, 0x1b5d39db, 0x1b1439fd, 0x1acb3a1f, 0x1a823a41, 0x1a393a62, 0x19ef3a82, 0x19a63aa3, 0x195c3ac3, 0x19123ae3, 0x18c83b02, 0x187e3b21, 0x18343b3f, 0x17e93b5e, 0x179e3b7c, 0x17533b99, 0x17093bb6, 0x16bd3bd3, 0x16723bef, 0x16273c0b, 0x15db3c27, 0x15903c42, 0x15443c5d, 0x14f83c78, 0x14ac3c92, 0x14603cac, 0x14133cc5, 0x13c73cde, 0x137a3cf7, 0x132e3d0f, 0x12e13d27, 0x12943d3f, 0x12473d56, 0x11fa3d6c, 0x11ad3d83, 0x115f3d99, 0x11123daf, 0x10c43dc4, 0x10773dd9, 0x10293ded, 0x0fdb3e01, 0x0f8d3e15, 0x0f3f3e28, 0x0ef13e3b, 0x0ea33e4e, 0x0e543e60, 0x0e063e72, 0x0db73e83, 0x0d693e94, 0x0d1a3ea5, 0x0ccb3eb5, 0x0c7c3ec5, 0x0c2d3ed5, 0x0bde3ee4, 0x0b8f3ef3, 0x0b403f01, 0x0af13f0f, 0x0aa23f1c, 0x0a523f2a, 0x0a033f36, 0x09b43f43, 0x09643f4f, 0x09143f5a, 0x08c53f65, 0x08753f70, 0x08253f7b, 0x07d63f85, 0x07863f8e, 0x07363f98, 0x06e63fa1, 0x06963fa9, 0x06463fb1, 0x05f63fb9, 0x05a63fc0, 0x05563fc7, 0x05053fcd, 0x04b53fd4, 0x04653fd9, 0x04153fdf, 0x03c53fe4, 0x03743fe8, 0x03243fec, 0x02d43ff0, 0x02833ff3, 0x02333ff6, 0x01e23ff9, 0x01923ffb, 0x01423ffd, 0x00f13ffe, 0x00a13fff, 0x00504000};


VMEM_SECTION unsigned int exp_data_1280_ext_15[1280*2] = {0x00007fff, 0xff5f7fff, 0xfebe7ffe, 0xfe1d7ffc, 0xfd7d7ffa, 0xfcdc7ff6, 0xfc3b7ff2, 0xfb9a7fed, 0xfafa7fe7, 0xfa597fe0, 0xf9b87fd9, 0xf9187fd0, 0xf8777fc7, 0xf7d67fbd, 0xf7367fb3, 0xf6957fa7, 0xf5f57f9b, 0xf5557f8e, 0xf4b47f80, 0xf4147f72, 0xf3747f62, 0xf2d47f52, 0xf2347f41, 0xf1947f2f, 0xf0f57f1d, 0xf0557f0a, 0xefb57ef5, 0xef167ee1, 0xee767ecb, 0xedd77eb5, 0xed387e9d, 0xec997e85, 0xebfa7e6d, 0xeb5b7e53, 0xeabc7e39, 0xea1e7e1e, 0xe9807e02, 0xe8e17de5, 0xe8437dc8, 0xe7a57da9, 0xe7077d8a, 0xe66a7d6b, 0xe5cc7d4a, 0xe52f7d29, 0xe4927d07, 0xe3f47ce4, 0xe3587cc0, 0xe2bb7c9c, 0xe21e7c77, 0xe1827c51, 0xe0e67c2a, 0xe04a7c03, 0xdfae7bda, 0xdf137bb1, 0xde777b88, 0xdddc7b5d, 0xdd417b32, 0xdca77b06, 0xdc0c7ad9, 0xdb727aab, 0xdad87a7d, 0xda3e7a4e, 0xd9a57a1e, 0xd90b79ee, 0xd87279bc, 0xd7d9798a, 0xd7417957, 0xd6a87924, 0xd61078ef, 0xd57878ba, 0xd4e17885, 0xd449784e, 0xd3b27817, 0xd31c77df, 0xd28577a6, 0xd1ef776c, 0xd1597732, 0xd0c376f7, 0xd02e76bb, 0xcf99767f, 0xcf047642, 0xce707604, 0xcddc75c5, 0xcd487586, 0xccb47546, 0xcc217505, 0xcb8e74c3, 0xcafc7481, 0xca69743e, 0xc9d773fa, 0xc94673b6, 0xc8b57371, 0xc824732b, 0xc79372e4, 0xc703729d, 0xc6737255, 0xc5e4720d, 0xc55571c3, 0xc4c67179, 0xc437712e, 0xc3a970e3, 0xc31c7097, 0xc28e704a, 0xc2016ffc, 0xc1756fae, 0xc0e96f5f, 0xc05d6f0f, 0xbfd26ebf, 0xbf476e6e, 0xbebc6e1c, 0xbe326dca, 0xbda86d77, 0xbd1f6d23, 0xbc966ccf, 0xbc0d6c7a, 0xbb856c24, 0xbafe6bce, 0xba766b77, 0xb9ef6b1f, 0xb9696ac7, 0xb8e36a6e, 0xb85e6a14, 0xb7d869ba, 0xb754695f, 0xb6d06903, 0xb64c68a7, 0xb5c9684a, 0xb54667ec, 0xb4c3678e, 0xb442672f, 0xb3c066d0, 0xb33f666f, 0xb2bf660f, 0xb23f65ad, 0xb1bf654b, 0xb14064e9, 0xb0c26485, 0xb0436421, 0xafc663bd, 0xaf496358, 0xaecc62f2, 0xae50628c, 0xadd56225, 0xad5961bd, 0xacdf6155, 0xac6560ec, 0xabeb6083, 0xab726019, 0xaafa5fae, 0xaa825f43, 0xaa0a5ed7, 0xa9935e6b, 0xa91d5dfe, 0xa8a75d91, 0xa8325d23, 0xa7bd5cb4, 0xa7495c45, 0xa6d55bd5, 0xa6625b65, 0xa5f05af4, 0xa57e5a82, 0xa50c5a10, 0xa49b599e, 0xa42b592b, 0xa3bb58b7, 0xa34c5843, 0xa2dd57ce, 0xa26f5759, 0xa20256e3, 0xa195566d, 0xa12955f6, 0xa0bd557e, 0xa0525506, 0x9fe7548e, 0x9f7d5415, 0x9f14539b, 0x9eab5321, 0x9e4352a7, 0x9ddb522b, 0x9d7451b0, 0x9d0e5134, 0x9ca850b7, 0x9c43503a, 0x9bdf4fbd, 0x9b7b4f3e, 0x9b174ec0, 0x9ab54e41, 0x9a534dc1, 0x99f14d41, 0x99914cc1, 0x99304c40, 0x98d14bbe, 0x98724b3d, 0x98144aba, 0x97b64a37, 0x975949b4, 0x96fd4930, 0x96a148ac, 0x96464828, 0x95ec47a2, 0x9592471d, 0x95394697, 0x94e14611, 0x9489458a, 0x94324502, 0x93dc447b, 0x938643f3, 0x9331436a, 0x92dd42e1, 0x92894258, 0x923641ce, 0x91e44144, 0x919240b9, 0x9141402e, 0x90f13fa3, 0x90a13f17, 0x90523e8b, 0x90043dff, 0x8fb63d72, 0x8f693ce4, 0x8f1d3c57, 0x8ed23bc9, 0x8e873b3a, 0x8e3d3aab, 0x8df33a1c, 0x8dab398d, 0x8d6338fd, 0x8d1c386d, 0x8cd537dc, 0x8c8f374b, 0x8c4a36ba, 0x8c063629, 0x8bc23597, 0x8b7f3504, 0x8b3d3472, 0x8afb33df, 0x8aba334c, 0x8a7a32b8, 0x8a3b3224, 0x89fc3190, 0x89be30fc, 0x89813067, 0x89452fd2, 0x89092f3d, 0x88ce2ea7, 0x88942e11, 0x885a2d7b, 0x88212ce4, 0x87e92c4e, 0x87b22bb7, 0x877b2b1f, 0x87462a88, 0x871129f0, 0x86dc2958, 0x86a928bf, 0x86762827, 0x8644278e, 0x861226f5, 0x85e2265b, 0x85b225c2, 0x85832528, 0x8555248e, 0x852723f4, 0x84fa2359, 0x84ce22bf, 0x84a32224, 0x84782189, 0x844f20ed, 0x84262052, 0x83fd1fb6, 0x83d61f1a, 0x83af1e7e, 0x83891de2, 0x83641d45, 0x83401ca8, 0x831c1c0c, 0x82f91b6e, 0x82d71ad1, 0x82b61a34, 0x82951996, 0x827618f9, 0x8257185b, 0x823817bd, 0x821b171f, 0x81fe1680, 0x81e215e2, 0x81c71544, 0x81ad14a5, 0x81931406, 0x817b1367, 0x816312c8, 0x814b1229, 0x8135118a, 0x811f10ea, 0x810b104b, 0x80f60fab, 0x80e30f0b, 0x80d10e6c, 0x80bf0dcc, 0x80ae0d2c, 0x809e0c8c, 0x808e0bec, 0x80800b4c, 0x80720aab, 0x80650a0b, 0x8059096b, 0x804d08ca, 0x8043082a, 0x80390789, 0x803006e8, 0x80270648, 0x802005a7, 0x80190506, 0x80130466, 0x800e03c5, 0x800a0324, 0x80060283, 0x800401e3, 0x80020142, 0x800000a1, 0x80000000, 0x8000ff5f, 0x8002febe, 0x8004fe1d, 0x8006fd7d, 0x800afcdc, 0x800efc3b, 0x8013fb9a, 0x8019fafa, 0x8020fa59, 0x8027f9b8, 0x8030f918, 0x8039f877, 0x8043f7d6, 0x804df736, 0x8059f695, 0x8065f5f5, 0x8072f555, 0x8080f4b4, 0x808ef414, 0x809ef374, 0x80aef2d4, 0x80bff234, 0x80d1f194, 0x80e3f0f5, 0x80f6f055, 0x810befb5, 0x811fef16, 0x8135ee76, 0x814bedd7, 0x8163ed38, 0x817bec99, 0x8193ebfa, 0x81adeb5b, 0x81c7eabc, 0x81e2ea1e, 0x81fee980, 0x821be8e1, 0x8238e843, 0x8257e7a5, 0x8276e707, 0x8295e66a, 0x82b6e5cc, 0x82d7e52f, 0x82f9e492, 0x831ce3f4, 0x8340e358, 0x8364e2bb, 0x8389e21e, 0x83afe182, 0x83d6e0e6, 0x83fde04a, 0x8426dfae, 0x844fdf13, 0x8478de77, 0x84a3dddc, 0x84cedd41, 0x84fadca7, 0x8527dc0c, 0x8555db72, 0x8583dad8, 0x85b2da3e, 0x85e2d9a5, 0x8612d90b, 0x8644d872, 0x8676d7d9, 0x86a9d741, 0x86dcd6a8, 0x8711d610, 0x8746d578, 0x877bd4e1, 0x87b2d449, 0x87e9d3b2, 0x8821d31c, 0x885ad285, 0x8894d1ef, 0x88ced159, 0x8909d0c3, 0x8945d02e, 0x8981cf99, 0x89becf04, 0x89fcce70, 0x8a3bcddc, 0x8a7acd48, 0x8abaccb4, 0x8afbcc21, 0x8b3dcb8e, 0x8b7fcafc, 0x8bc2ca69, 0x8c06c9d7, 0x8c4ac946, 0x8c8fc8b5, 0x8cd5c824, 0x8d1cc793, 0x8d63c703, 0x8dabc673, 0x8df3c5e4, 0x8e3dc555, 0x8e87c4c6, 0x8ed2c437, 0x8f1dc3a9, 0x8f69c31c, 0x8fb6c28e, 0x9004c201, 0x9052c175, 0x90a1c0e9, 0x90f1c05d, 0x9141bfd2, 0x9192bf47, 0x91e4bebc, 0x9236be32, 0x9289bda8, 0x92ddbd1f, 0x9331bc96, 0x9386bc0d, 0x93dcbb85, 0x9432bafe, 0x9489ba76, 0x94e1b9ef, 0x9539b969, 0x9592b8e3, 0x95ecb85e, 0x9646b7d8, 0x96a1b754, 0x96fdb6d0, 0x9759b64c, 0x97b6b5c9, 0x9814b546, 0x9872b4c3, 0x98d1b442, 0x9930b3c0, 0x9991b33f, 0x99f1b2bf, 0x9a53b23f, 0x9ab5b1bf, 0x9b17b140, 0x9b7bb0c2, 0x9bdfb043, 0x9c43afc6, 0x9ca8af49, 0x9d0eaecc, 0x9d74ae50, 0x9ddbadd5, 0x9e43ad59, 0x9eabacdf, 0x9f14ac65, 0x9f7dabeb, 0x9fe7ab72, 0xa052aafa, 0xa0bdaa82, 0xa129aa0a, 0xa195a993, 0xa202a91d, 0xa26fa8a7, 0xa2dda832, 0xa34ca7bd, 0xa3bba749, 0xa42ba6d5, 0xa49ba662, 0xa50ca5f0, 0xa57ea57e, 0xa5f0a50c, 0xa662a49b, 0xa6d5a42b, 0xa749a3bb, 0xa7bda34c, 0xa832a2dd, 0xa8a7a26f, 0xa91da202, 0xa993a195, 0xaa0aa129, 0xaa82a0bd, 0xaafaa052, 0xab729fe7, 0xabeb9f7d, 0xac659f14, 0xacdf9eab, 0xad599e43, 0xadd59ddb, 0xae509d74, 0xaecc9d0e, 0xaf499ca8, 0xafc69c43, 0xb0439bdf, 0xb0c29b7b, 0xb1409b17, 0xb1bf9ab5, 0xb23f9a53, 0xb2bf99f1, 0xb33f9991, 0xb3c09930, 0xb44298d1, 0xb4c39872, 0xb5469814, 0xb5c997b6, 0xb64c9759, 0xb6d096fd, 0xb75496a1, 0xb7d89646, 0xb85e95ec, 0xb8e39592, 0xb9699539, 0xb9ef94e1, 0xba769489, 0xbafe9432, 0xbb8593dc, 0xbc0d9386, 0xbc969331, 0xbd1f92dd, 0xbda89289, 0xbe329236, 0xbebc91e4, 0xbf479192, 0xbfd29141, 0xc05d90f1, 0xc0e990a1, 0xc1759052, 0xc2019004, 0xc28e8fb6, 0xc31c8f69, 0xc3a98f1d, 0xc4378ed2, 0xc4c68e87, 0xc5558e3d, 0xc5e48df3, 0xc6738dab, 0xc7038d63, 0xc7938d1c, 0xc8248cd5, 0xc8b58c8f, 0xc9468c4a, 0xc9d78c06, 0xca698bc2, 0xcafc8b7f, 0xcb8e8b3d, 0xcc218afb, 0xccb48aba, 0xcd488a7a, 0xcddc8a3b, 0xce7089fc, 0xcf0489be, 0xcf998981, 0xd02e8945, 0xd0c38909, 0xd15988ce, 0xd1ef8894, 0xd285885a, 0xd31c8821, 0xd3b287e9, 0xd44987b2, 0xd4e1877b, 0xd5788746, 0xd6108711, 0xd6a886dc, 0xd74186a9, 0xd7d98676, 0xd8728644, 0xd90b8612, 0xd9a585e2, 0xda3e85b2, 0xdad88583, 0xdb728555, 0xdc0c8527, 0xdca784fa, 0xdd4184ce, 0xdddc84a3, 0xde778478, 0xdf13844f, 0xdfae8426, 0xe04a83fd, 0xe0e683d6, 0xe18283af, 0xe21e8389, 0xe2bb8364, 0xe3588340, 0xe3f4831c, 0xe49282f9, 0xe52f82d7, 0xe5cc82b6, 0xe66a8295, 0xe7078276, 0xe7a58257, 0xe8438238, 0xe8e1821b, 0xe98081fe, 0xea1e81e2, 0xeabc81c7, 0xeb5b81ad, 0xebfa8193, 0xec99817b, 0xed388163, 0xedd7814b, 0xee768135, 0xef16811f, 0xefb5810b, 0xf05580f6, 0xf0f580e3, 0xf19480d1, 0xf23480bf, 0xf2d480ae, 0xf374809e, 0xf414808e, 0xf4b48080, 0xf5558072, 0xf5f58065, 0xf6958059, 0xf736804d, 0xf7d68043, 0xf8778039, 0xf9188030, 0xf9b88027, 0xfa598020, 0xfafa8019, 0xfb9a8013, 0xfc3b800e, 0xfcdc800a, 0xfd7d8006, 0xfe1d8004, 0xfebe8002, 0xff5f8000, 0x00008000, 0x00a18000, 0x01428002, 0x01e38004, 0x02838006, 0x0324800a, 0x03c5800e, 0x04668013, 0x05068019, 0x05a78020, 0x06488027, 0x06e88030, 0x07898039, 0x082a8043, 0x08ca804d, 0x096b8059, 0x0a0b8065, 0x0aab8072, 0x0b4c8080, 0x0bec808e, 0x0c8c809e, 0x0d2c80ae, 0x0dcc80bf, 0x0e6c80d1, 0x0f0b80e3, 0x0fab80f6, 0x104b810b, 0x10ea811f, 0x118a8135, 0x1229814b, 0x12c88163, 0x1367817b, 0x14068193, 0x14a581ad, 0x154481c7, 0x15e281e2, 0x168081fe, 0x171f821b, 0x17bd8238, 0x185b8257, 0x18f98276, 0x19968295, 0x1a3482b6, 0x1ad182d7, 0x1b6e82f9, 0x1c0c831c, 0x1ca88340, 0x1d458364, 0x1de28389, 0x1e7e83af, 0x1f1a83d6, 0x1fb683fd, 0x20528426, 0x20ed844f, 0x21898478, 0x222484a3, 0x22bf84ce, 0x235984fa, 0x23f48527, 0x248e8555, 0x25288583, 0x25c285b2, 0x265b85e2, 0x26f58612, 0x278e8644, 0x28278676, 0x28bf86a9, 0x295886dc, 0x29f08711, 0x2a888746, 0x2b1f877b, 0x2bb787b2, 0x2c4e87e9, 0x2ce48821, 0x2d7b885a, 0x2e118894, 0x2ea788ce, 0x2f3d8909, 0x2fd28945, 0x30678981, 0x30fc89be, 0x319089fc, 0x32248a3b, 0x32b88a7a, 0x334c8aba, 0x33df8afb, 0x34728b3d, 0x35048b7f, 0x35978bc2, 0x36298c06, 0x36ba8c4a, 0x374b8c8f, 0x37dc8cd5, 0x386d8d1c, 0x38fd8d63, 0x398d8dab, 0x3a1c8df3, 0x3aab8e3d, 0x3b3a8e87, 0x3bc98ed2, 0x3c578f1d, 0x3ce48f69, 0x3d728fb6, 0x3dff9004, 0x3e8b9052, 0x3f1790a1, 0x3fa390f1, 0x402e9141, 0x40b99192, 0x414491e4, 0x41ce9236, 0x42589289, 0x42e192dd, 0x436a9331, 0x43f39386, 0x447b93dc, 0x45029432, 0x458a9489, 0x461194e1, 0x46979539, 0x471d9592, 0x47a295ec, 0x48289646, 0x48ac96a1, 0x493096fd, 0x49b49759, 0x4a3797b6, 0x4aba9814, 0x4b3d9872, 0x4bbe98d1, 0x4c409930, 0x4cc19991, 0x4d4199f1, 0x4dc19a53, 0x4e419ab5, 0x4ec09b17, 0x4f3e9b7b, 0x4fbd9bdf, 0x503a9c43, 0x50b79ca8, 0x51349d0e, 0x51b09d74, 0x522b9ddb, 0x52a79e43, 0x53219eab, 0x539b9f14, 0x54159f7d, 0x548e9fe7, 0x5506a052, 0x557ea0bd, 0x55f6a129, 0x566da195, 0x56e3a202, 0x5759a26f, 0x57cea2dd, 0x5843a34c, 0x58b7a3bb, 0x592ba42b, 0x599ea49b, 0x5a10a50c, 0x5a82a57e, 0x5af4a5f0, 0x5b65a662, 0x5bd5a6d5, 0x5c45a749, 0x5cb4a7bd, 0x5d23a832, 0x5d91a8a7, 0x5dfea91d, 0x5e6ba993, 0x5ed7aa0a, 0x5f43aa82, 0x5faeaafa, 0x6019ab72, 0x6083abeb, 0x60ecac65, 0x6155acdf, 0x61bdad59, 0x6225add5, 0x628cae50, 0x62f2aecc, 0x6358af49, 0x63bdafc6, 0x6421b043, 0x6485b0c2, 0x64e9b140, 0x654bb1bf, 0x65adb23f, 0x660fb2bf, 0x666fb33f, 0x66d0b3c0, 0x672fb442, 0x678eb4c3, 0x67ecb546, 0x684ab5c9, 0x68a7b64c, 0x6903b6d0, 0x695fb754, 0x69bab7d8, 0x6a14b85e, 0x6a6eb8e3, 0x6ac7b969, 0x6b1fb9ef, 0x6b77ba76, 0x6bcebafe, 0x6c24bb85, 0x6c7abc0d, 0x6ccfbc96, 0x6d23bd1f, 0x6d77bda8, 0x6dcabe32, 0x6e1cbebc, 0x6e6ebf47, 0x6ebfbfd2, 0x6f0fc05d, 0x6f5fc0e9, 0x6faec175, 0x6ffcc201, 0x704ac28e, 0x7097c31c, 0x70e3c3a9, 0x712ec437, 0x7179c4c6, 0x71c3c555, 0x720dc5e4, 0x7255c673, 0x729dc703, 0x72e4c793, 0x732bc824, 0x7371c8b5, 0x73b6c946, 0x73fac9d7, 0x743eca69, 0x7481cafc, 0x74c3cb8e, 0x7505cc21, 0x7546ccb4, 0x7586cd48, 0x75c5cddc, 0x7604ce70, 0x7642cf04, 0x767fcf99, 0x76bbd02e, 0x76f7d0c3, 0x7732d159, 0x776cd1ef, 0x77a6d285, 0x77dfd31c, 0x7817d3b2, 0x784ed449, 0x7885d4e1, 0x78bad578, 0x78efd610, 0x7924d6a8, 0x7957d741, 0x798ad7d9, 0x79bcd872, 0x79eed90b, 0x7a1ed9a5, 0x7a4eda3e, 0x7a7ddad8, 0x7aabdb72, 0x7ad9dc0c, 0x7b06dca7, 0x7b32dd41, 0x7b5ddddc, 0x7b88de77, 0x7bb1df13, 0x7bdadfae, 0x7c03e04a, 0x7c2ae0e6, 0x7c51e182, 0x7c77e21e, 0x7c9ce2bb, 0x7cc0e358, 0x7ce4e3f4, 0x7d07e492, 0x7d29e52f, 0x7d4ae5cc, 0x7d6be66a, 0x7d8ae707, 0x7da9e7a5, 0x7dc8e843, 0x7de5e8e1, 0x7e02e980, 0x7e1eea1e, 0x7e39eabc, 0x7e53eb5b, 0x7e6debfa, 0x7e85ec99, 0x7e9ded38, 0x7eb5edd7, 0x7ecbee76, 0x7ee1ef16, 0x7ef5efb5, 0x7f0af055, 0x7f1df0f5, 0x7f2ff194, 0x7f41f234, 0x7f52f2d4, 0x7f62f374, 0x7f72f414, 0x7f80f4b4, 0x7f8ef555, 0x7f9bf5f5, 0x7fa7f695, 0x7fb3f736, 0x7fbdf7d6, 0x7fc7f877, 0x7fd0f918, 0x7fd9f9b8, 0x7fe0fa59, 0x7fe7fafa, 0x7fedfb9a, 0x7ff2fc3b, 0x7ff6fcdc, 0x7ffafd7d, 0x7ffcfe1d, 0x7ffefebe, 0x7fffff5f, 0x7fff0000, 0x7fff00a1, 0x7ffe0142, 0x7ffc01e3, 0x7ffa0283, 0x7ff60324, 0x7ff203c5, 0x7fed0466, 0x7fe70506, 0x7fe005a7, 0x7fd90648, 0x7fd006e8, 0x7fc70789, 0x7fbd082a, 0x7fb308ca, 0x7fa7096b, 0x7f9b0a0b, 0x7f8e0aab, 0x7f800b4c, 0x7f720bec, 0x7f620c8c, 0x7f520d2c, 0x7f410dcc, 0x7f2f0e6c, 0x7f1d0f0b, 0x7f0a0fab, 0x7ef5104b, 0x7ee110ea, 0x7ecb118a, 0x7eb51229, 0x7e9d12c8, 0x7e851367, 0x7e6d1406, 0x7e5314a5, 0x7e391544, 0x7e1e15e2, 0x7e021680, 0x7de5171f, 0x7dc817bd, 0x7da9185b, 0x7d8a18f9, 0x7d6b1996, 0x7d4a1a34, 0x7d291ad1, 0x7d071b6e, 0x7ce41c0c, 0x7cc01ca8, 0x7c9c1d45, 0x7c771de2, 0x7c511e7e, 0x7c2a1f1a, 0x7c031fb6, 0x7bda2052, 0x7bb120ed, 0x7b882189, 0x7b5d2224, 0x7b3222bf, 0x7b062359, 0x7ad923f4, 0x7aab248e, 0x7a7d2528, 0x7a4e25c2, 0x7a1e265b, 0x79ee26f5, 0x79bc278e, 0x798a2827, 0x795728bf, 0x79242958, 0x78ef29f0, 0x78ba2a88, 0x78852b1f, 0x784e2bb7, 0x78172c4e, 0x77df2ce4, 0x77a62d7b, 0x776c2e11, 0x77322ea7, 0x76f72f3d, 0x76bb2fd2, 0x767f3067, 0x764230fc, 0x76043190, 0x75c53224, 0x758632b8, 0x7546334c, 0x750533df, 0x74c33472, 0x74813504, 0x743e3597, 0x73fa3629, 0x73b636ba, 0x7371374b, 0x732b37dc, 0x72e4386d, 0x729d38fd, 0x7255398d, 0x720d3a1c, 0x71c33aab, 0x71793b3a, 0x712e3bc9, 0x70e33c57, 0x70973ce4, 0x704a3d72, 0x6ffc3dff, 0x6fae3e8b, 0x6f5f3f17, 0x6f0f3fa3, 0x6ebf402e, 0x6e6e40b9, 0x6e1c4144, 0x6dca41ce, 0x6d774258, 0x6d2342e1, 0x6ccf436a, 0x6c7a43f3, 0x6c24447b, 0x6bce4502, 0x6b77458a, 0x6b1f4611, 0x6ac74697, 0x6a6e471d, 0x6a1447a2, 0x69ba4828, 0x695f48ac, 0x69034930, 0x68a749b4, 0x684a4a37, 0x67ec4aba, 0x678e4b3d, 0x672f4bbe, 0x66d04c40, 0x666f4cc1, 0x660f4d41, 0x65ad4dc1, 0x654b4e41, 0x64e94ec0, 0x64854f3e, 0x64214fbd, 0x63bd503a, 0x635850b7, 0x62f25134, 0x628c51b0, 0x6225522b, 0x61bd52a7, 0x61555321, 0x60ec539b, 0x60835415, 0x6019548e, 0x5fae5506, 0x5f43557e, 0x5ed755f6, 0x5e6b566d, 0x5dfe56e3, 0x5d915759, 0x5d2357ce, 0x5cb45843, 0x5c4558b7, 0x5bd5592b, 0x5b65599e, 0x5af45a10, 0x5a825a82, 0x5a105af4, 0x599e5b65, 0x592b5bd5, 0x58b75c45, 0x58435cb4, 0x57ce5d23, 0x57595d91, 0x56e35dfe, 0x566d5e6b, 0x55f65ed7, 0x557e5f43, 0x55065fae, 0x548e6019, 0x54156083, 0x539b60ec, 0x53216155, 0x52a761bd, 0x522b6225, 0x51b0628c, 0x513462f2, 0x50b76358, 0x503a63bd, 0x4fbd6421, 0x4f3e6485, 0x4ec064e9, 0x4e41654b, 0x4dc165ad, 0x4d41660f, 0x4cc1666f, 0x4c4066d0, 0x4bbe672f, 0x4b3d678e, 0x4aba67ec, 0x4a37684a, 0x49b468a7, 0x49306903, 0x48ac695f, 0x482869ba, 0x47a26a14, 0x471d6a6e, 0x46976ac7, 0x46116b1f, 0x458a6b77, 0x45026bce, 0x447b6c24, 0x43f36c7a, 0x436a6ccf, 0x42e16d23, 0x42586d77, 0x41ce6dca, 0x41446e1c, 0x40b96e6e, 0x402e6ebf, 0x3fa36f0f, 0x3f176f5f, 0x3e8b6fae, 0x3dff6ffc, 0x3d72704a, 0x3ce47097, 0x3c5770e3, 0x3bc9712e, 0x3b3a7179, 0x3aab71c3, 0x3a1c720d, 0x398d7255, 0x38fd729d, 0x386d72e4, 0x37dc732b, 0x374b7371, 0x36ba73b6, 0x362973fa, 0x3597743e, 0x35047481, 0x347274c3, 0x33df7505, 0x334c7546, 0x32b87586, 0x322475c5, 0x31907604, 0x30fc7642, 0x3067767f, 0x2fd276bb, 0x2f3d76f7, 0x2ea77732, 0x2e11776c, 0x2d7b77a6, 0x2ce477df, 0x2c4e7817, 0x2bb7784e, 0x2b1f7885, 0x2a8878ba, 0x29f078ef, 0x29587924, 0x28bf7957, 0x2827798a, 0x278e79bc, 0x26f579ee, 0x265b7a1e, 0x25c27a4e, 0x25287a7d, 0x248e7aab, 0x23f47ad9, 0x23597b06, 0x22bf7b32, 0x22247b5d, 0x21897b88, 0x20ed7bb1, 0x20527bda, 0x1fb67c03, 0x1f1a7c2a, 0x1e7e7c51, 0x1de27c77, 0x1d457c9c, 0x1ca87cc0, 0x1c0c7ce4, 0x1b6e7d07, 0x1ad17d29, 0x1a347d4a, 0x19967d6b, 0x18f97d8a, 0x185b7da9, 0x17bd7dc8, 0x171f7de5, 0x16807e02, 0x15e27e1e, 0x15447e39, 0x14a57e53, 0x14067e6d, 0x13677e85, 0x12c87e9d, 0x12297eb5, 0x118a7ecb, 0x10ea7ee1, 0x104b7ef5, 0x0fab7f0a, 0x0f0b7f1d, 0x0e6c7f2f, 0x0dcc7f41, 0x0d2c7f52, 0x0c8c7f62, 0x0bec7f72, 0x0b4c7f80, 0x0aab7f8e, 0x0a0b7f9b, 0x096b7fa7, 0x08ca7fb3, 0x082a7fbd, 0x07897fc7, 0x06e87fd0, 0x06487fd9, 0x05a77fe0, 0x05067fe7, 0x04667fed, 0x03c57ff2, 0x03247ff6, 0x02837ffa, 0x01e37ffc, 0x01427ffe, 0x00a17fff,
                                                          0x00007fff, 0xff5f7fff, 0xfebe7ffe, 0xfe1d7ffc, 0xfd7d7ffa, 0xfcdc7ff6, 0xfc3b7ff2, 0xfb9a7fed, 0xfafa7fe7, 0xfa597fe0, 0xf9b87fd9, 0xf9187fd0, 0xf8777fc7, 0xf7d67fbd, 0xf7367fb3, 0xf6957fa7, 0xf5f57f9b, 0xf5557f8e, 0xf4b47f80, 0xf4147f72, 0xf3747f62, 0xf2d47f52, 0xf2347f41, 0xf1947f2f, 0xf0f57f1d, 0xf0557f0a, 0xefb57ef5, 0xef167ee1, 0xee767ecb, 0xedd77eb5, 0xed387e9d, 0xec997e85, 0xebfa7e6d, 0xeb5b7e53, 0xeabc7e39, 0xea1e7e1e, 0xe9807e02, 0xe8e17de5, 0xe8437dc8, 0xe7a57da9, 0xe7077d8a, 0xe66a7d6b, 0xe5cc7d4a, 0xe52f7d29, 0xe4927d07, 0xe3f47ce4, 0xe3587cc0, 0xe2bb7c9c, 0xe21e7c77, 0xe1827c51, 0xe0e67c2a, 0xe04a7c03, 0xdfae7bda, 0xdf137bb1, 0xde777b88, 0xdddc7b5d, 0xdd417b32, 0xdca77b06, 0xdc0c7ad9, 0xdb727aab, 0xdad87a7d, 0xda3e7a4e, 0xd9a57a1e, 0xd90b79ee, 0xd87279bc, 0xd7d9798a, 0xd7417957, 0xd6a87924, 0xd61078ef, 0xd57878ba, 0xd4e17885, 0xd449784e, 0xd3b27817, 0xd31c77df, 0xd28577a6, 0xd1ef776c, 0xd1597732, 0xd0c376f7, 0xd02e76bb, 0xcf99767f, 0xcf047642, 0xce707604, 0xcddc75c5, 0xcd487586, 0xccb47546, 0xcc217505, 0xcb8e74c3, 0xcafc7481, 0xca69743e, 0xc9d773fa, 0xc94673b6, 0xc8b57371, 0xc824732b, 0xc79372e4, 0xc703729d, 0xc6737255, 0xc5e4720d, 0xc55571c3, 0xc4c67179, 0xc437712e, 0xc3a970e3, 0xc31c7097, 0xc28e704a, 0xc2016ffc, 0xc1756fae, 0xc0e96f5f, 0xc05d6f0f, 0xbfd26ebf, 0xbf476e6e, 0xbebc6e1c, 0xbe326dca, 0xbda86d77, 0xbd1f6d23, 0xbc966ccf, 0xbc0d6c7a, 0xbb856c24, 0xbafe6bce, 0xba766b77, 0xb9ef6b1f, 0xb9696ac7, 0xb8e36a6e, 0xb85e6a14, 0xb7d869ba, 0xb754695f, 0xb6d06903, 0xb64c68a7, 0xb5c9684a, 0xb54667ec, 0xb4c3678e, 0xb442672f, 0xb3c066d0, 0xb33f666f, 0xb2bf660f, 0xb23f65ad, 0xb1bf654b, 0xb14064e9, 0xb0c26485, 0xb0436421, 0xafc663bd, 0xaf496358, 0xaecc62f2, 0xae50628c, 0xadd56225, 0xad5961bd, 0xacdf6155, 0xac6560ec, 0xabeb6083, 0xab726019, 0xaafa5fae, 0xaa825f43, 0xaa0a5ed7, 0xa9935e6b, 0xa91d5dfe, 0xa8a75d91, 0xa8325d23, 0xa7bd5cb4, 0xa7495c45, 0xa6d55bd5, 0xa6625b65, 0xa5f05af4, 0xa57e5a82, 0xa50c5a10, 0xa49b599e, 0xa42b592b, 0xa3bb58b7, 0xa34c5843, 0xa2dd57ce, 0xa26f5759, 0xa20256e3, 0xa195566d, 0xa12955f6, 0xa0bd557e, 0xa0525506, 0x9fe7548e, 0x9f7d5415, 0x9f14539b, 0x9eab5321, 0x9e4352a7, 0x9ddb522b, 0x9d7451b0, 0x9d0e5134, 0x9ca850b7, 0x9c43503a, 0x9bdf4fbd, 0x9b7b4f3e, 0x9b174ec0, 0x9ab54e41, 0x9a534dc1, 0x99f14d41, 0x99914cc1, 0x99304c40, 0x98d14bbe, 0x98724b3d, 0x98144aba, 0x97b64a37, 0x975949b4, 0x96fd4930, 0x96a148ac, 0x96464828, 0x95ec47a2, 0x9592471d, 0x95394697, 0x94e14611, 0x9489458a, 0x94324502, 0x93dc447b, 0x938643f3, 0x9331436a, 0x92dd42e1, 0x92894258, 0x923641ce, 0x91e44144, 0x919240b9, 0x9141402e, 0x90f13fa3, 0x90a13f17, 0x90523e8b, 0x90043dff, 0x8fb63d72, 0x8f693ce4, 0x8f1d3c57, 0x8ed23bc9, 0x8e873b3a, 0x8e3d3aab, 0x8df33a1c, 0x8dab398d, 0x8d6338fd, 0x8d1c386d, 0x8cd537dc, 0x8c8f374b, 0x8c4a36ba, 0x8c063629, 0x8bc23597, 0x8b7f3504, 0x8b3d3472, 0x8afb33df, 0x8aba334c, 0x8a7a32b8, 0x8a3b3224, 0x89fc3190, 0x89be30fc, 0x89813067, 0x89452fd2, 0x89092f3d, 0x88ce2ea7, 0x88942e11, 0x885a2d7b, 0x88212ce4, 0x87e92c4e, 0x87b22bb7, 0x877b2b1f, 0x87462a88, 0x871129f0, 0x86dc2958, 0x86a928bf, 0x86762827, 0x8644278e, 0x861226f5, 0x85e2265b, 0x85b225c2, 0x85832528, 0x8555248e, 0x852723f4, 0x84fa2359, 0x84ce22bf, 0x84a32224, 0x84782189, 0x844f20ed, 0x84262052, 0x83fd1fb6, 0x83d61f1a, 0x83af1e7e, 0x83891de2, 0x83641d45, 0x83401ca8, 0x831c1c0c, 0x82f91b6e, 0x82d71ad1, 0x82b61a34, 0x82951996, 0x827618f9, 0x8257185b, 0x823817bd, 0x821b171f, 0x81fe1680, 0x81e215e2, 0x81c71544, 0x81ad14a5, 0x81931406, 0x817b1367, 0x816312c8, 0x814b1229, 0x8135118a, 0x811f10ea, 0x810b104b, 0x80f60fab, 0x80e30f0b, 0x80d10e6c, 0x80bf0dcc, 0x80ae0d2c, 0x809e0c8c, 0x808e0bec, 0x80800b4c, 0x80720aab, 0x80650a0b, 0x8059096b, 0x804d08ca, 0x8043082a, 0x80390789, 0x803006e8, 0x80270648, 0x802005a7, 0x80190506, 0x80130466, 0x800e03c5, 0x800a0324, 0x80060283, 0x800401e3, 0x80020142, 0x800000a1, 0x80000000, 0x8000ff5f, 0x8002febe, 0x8004fe1d, 0x8006fd7d, 0x800afcdc, 0x800efc3b, 0x8013fb9a, 0x8019fafa, 0x8020fa59, 0x8027f9b8, 0x8030f918, 0x8039f877, 0x8043f7d6, 0x804df736, 0x8059f695, 0x8065f5f5, 0x8072f555, 0x8080f4b4, 0x808ef414, 0x809ef374, 0x80aef2d4, 0x80bff234, 0x80d1f194, 0x80e3f0f5, 0x80f6f055, 0x810befb5, 0x811fef16, 0x8135ee76, 0x814bedd7, 0x8163ed38, 0x817bec99, 0x8193ebfa, 0x81adeb5b, 0x81c7eabc, 0x81e2ea1e, 0x81fee980, 0x821be8e1, 0x8238e843, 0x8257e7a5, 0x8276e707, 0x8295e66a, 0x82b6e5cc, 0x82d7e52f, 0x82f9e492, 0x831ce3f4, 0x8340e358, 0x8364e2bb, 0x8389e21e, 0x83afe182, 0x83d6e0e6, 0x83fde04a, 0x8426dfae, 0x844fdf13, 0x8478de77, 0x84a3dddc, 0x84cedd41, 0x84fadca7, 0x8527dc0c, 0x8555db72, 0x8583dad8, 0x85b2da3e, 0x85e2d9a5, 0x8612d90b, 0x8644d872, 0x8676d7d9, 0x86a9d741, 0x86dcd6a8, 0x8711d610, 0x8746d578, 0x877bd4e1, 0x87b2d449, 0x87e9d3b2, 0x8821d31c, 0x885ad285, 0x8894d1ef, 0x88ced159, 0x8909d0c3, 0x8945d02e, 0x8981cf99, 0x89becf04, 0x89fcce70, 0x8a3bcddc, 0x8a7acd48, 0x8abaccb4, 0x8afbcc21, 0x8b3dcb8e, 0x8b7fcafc, 0x8bc2ca69, 0x8c06c9d7, 0x8c4ac946, 0x8c8fc8b5, 0x8cd5c824, 0x8d1cc793, 0x8d63c703, 0x8dabc673, 0x8df3c5e4, 0x8e3dc555, 0x8e87c4c6, 0x8ed2c437, 0x8f1dc3a9, 0x8f69c31c, 0x8fb6c28e, 0x9004c201, 0x9052c175, 0x90a1c0e9, 0x90f1c05d, 0x9141bfd2, 0x9192bf47, 0x91e4bebc, 0x9236be32, 0x9289bda8, 0x92ddbd1f, 0x9331bc96, 0x9386bc0d, 0x93dcbb85, 0x9432bafe, 0x9489ba76, 0x94e1b9ef, 0x9539b969, 0x9592b8e3, 0x95ecb85e, 0x9646b7d8, 0x96a1b754, 0x96fdb6d0, 0x9759b64c, 0x97b6b5c9, 0x9814b546, 0x9872b4c3, 0x98d1b442, 0x9930b3c0, 0x9991b33f, 0x99f1b2bf, 0x9a53b23f, 0x9ab5b1bf, 0x9b17b140, 0x9b7bb0c2, 0x9bdfb043, 0x9c43afc6, 0x9ca8af49, 0x9d0eaecc, 0x9d74ae50, 0x9ddbadd5, 0x9e43ad59, 0x9eabacdf, 0x9f14ac65, 0x9f7dabeb, 0x9fe7ab72, 0xa052aafa, 0xa0bdaa82, 0xa129aa0a, 0xa195a993, 0xa202a91d, 0xa26fa8a7, 0xa2dda832, 0xa34ca7bd, 0xa3bba749, 0xa42ba6d5, 0xa49ba662, 0xa50ca5f0, 0xa57ea57e, 0xa5f0a50c, 0xa662a49b, 0xa6d5a42b, 0xa749a3bb, 0xa7bda34c, 0xa832a2dd, 0xa8a7a26f, 0xa91da202, 0xa993a195, 0xaa0aa129, 0xaa82a0bd, 0xaafaa052, 0xab729fe7, 0xabeb9f7d, 0xac659f14, 0xacdf9eab, 0xad599e43, 0xadd59ddb, 0xae509d74, 0xaecc9d0e, 0xaf499ca8, 0xafc69c43, 0xb0439bdf, 0xb0c29b7b, 0xb1409b17, 0xb1bf9ab5, 0xb23f9a53, 0xb2bf99f1, 0xb33f9991, 0xb3c09930, 0xb44298d1, 0xb4c39872, 0xb5469814, 0xb5c997b6, 0xb64c9759, 0xb6d096fd, 0xb75496a1, 0xb7d89646, 0xb85e95ec, 0xb8e39592, 0xb9699539, 0xb9ef94e1, 0xba769489, 0xbafe9432, 0xbb8593dc, 0xbc0d9386, 0xbc969331, 0xbd1f92dd, 0xbda89289, 0xbe329236, 0xbebc91e4, 0xbf479192, 0xbfd29141, 0xc05d90f1, 0xc0e990a1, 0xc1759052, 0xc2019004, 0xc28e8fb6, 0xc31c8f69, 0xc3a98f1d, 0xc4378ed2, 0xc4c68e87, 0xc5558e3d, 0xc5e48df3, 0xc6738dab, 0xc7038d63, 0xc7938d1c, 0xc8248cd5, 0xc8b58c8f, 0xc9468c4a, 0xc9d78c06, 0xca698bc2, 0xcafc8b7f, 0xcb8e8b3d, 0xcc218afb, 0xccb48aba, 0xcd488a7a, 0xcddc8a3b, 0xce7089fc, 0xcf0489be, 0xcf998981, 0xd02e8945, 0xd0c38909, 0xd15988ce, 0xd1ef8894, 0xd285885a, 0xd31c8821, 0xd3b287e9, 0xd44987b2, 0xd4e1877b, 0xd5788746, 0xd6108711, 0xd6a886dc, 0xd74186a9, 0xd7d98676, 0xd8728644, 0xd90b8612, 0xd9a585e2, 0xda3e85b2, 0xdad88583, 0xdb728555, 0xdc0c8527, 0xdca784fa, 0xdd4184ce, 0xdddc84a3, 0xde778478, 0xdf13844f, 0xdfae8426, 0xe04a83fd, 0xe0e683d6, 0xe18283af, 0xe21e8389, 0xe2bb8364, 0xe3588340, 0xe3f4831c, 0xe49282f9, 0xe52f82d7, 0xe5cc82b6, 0xe66a8295, 0xe7078276, 0xe7a58257, 0xe8438238, 0xe8e1821b, 0xe98081fe, 0xea1e81e2, 0xeabc81c7, 0xeb5b81ad, 0xebfa8193, 0xec99817b, 0xed388163, 0xedd7814b, 0xee768135, 0xef16811f, 0xefb5810b, 0xf05580f6, 0xf0f580e3, 0xf19480d1, 0xf23480bf, 0xf2d480ae, 0xf374809e, 0xf414808e, 0xf4b48080, 0xf5558072, 0xf5f58065, 0xf6958059, 0xf736804d, 0xf7d68043, 0xf8778039, 0xf9188030, 0xf9b88027, 0xfa598020, 0xfafa8019, 0xfb9a8013, 0xfc3b800e, 0xfcdc800a, 0xfd7d8006, 0xfe1d8004, 0xfebe8002, 0xff5f8000, 0x00008000, 0x00a18000, 0x01428002, 0x01e38004, 0x02838006, 0x0324800a, 0x03c5800e, 0x04668013, 0x05068019, 0x05a78020, 0x06488027, 0x06e88030, 0x07898039, 0x082a8043, 0x08ca804d, 0x096b8059, 0x0a0b8065, 0x0aab8072, 0x0b4c8080, 0x0bec808e, 0x0c8c809e, 0x0d2c80ae, 0x0dcc80bf, 0x0e6c80d1, 0x0f0b80e3, 0x0fab80f6, 0x104b810b, 0x10ea811f, 0x118a8135, 0x1229814b, 0x12c88163, 0x1367817b, 0x14068193, 0x14a581ad, 0x154481c7, 0x15e281e2, 0x168081fe, 0x171f821b, 0x17bd8238, 0x185b8257, 0x18f98276, 0x19968295, 0x1a3482b6, 0x1ad182d7, 0x1b6e82f9, 0x1c0c831c, 0x1ca88340, 0x1d458364, 0x1de28389, 0x1e7e83af, 0x1f1a83d6, 0x1fb683fd, 0x20528426, 0x20ed844f, 0x21898478, 0x222484a3, 0x22bf84ce, 0x235984fa, 0x23f48527, 0x248e8555, 0x25288583, 0x25c285b2, 0x265b85e2, 0x26f58612, 0x278e8644, 0x28278676, 0x28bf86a9, 0x295886dc, 0x29f08711, 0x2a888746, 0x2b1f877b, 0x2bb787b2, 0x2c4e87e9, 0x2ce48821, 0x2d7b885a, 0x2e118894, 0x2ea788ce, 0x2f3d8909, 0x2fd28945, 0x30678981, 0x30fc89be, 0x319089fc, 0x32248a3b, 0x32b88a7a, 0x334c8aba, 0x33df8afb, 0x34728b3d, 0x35048b7f, 0x35978bc2, 0x36298c06, 0x36ba8c4a, 0x374b8c8f, 0x37dc8cd5, 0x386d8d1c, 0x38fd8d63, 0x398d8dab, 0x3a1c8df3, 0x3aab8e3d, 0x3b3a8e87, 0x3bc98ed2, 0x3c578f1d, 0x3ce48f69, 0x3d728fb6, 0x3dff9004, 0x3e8b9052, 0x3f1790a1, 0x3fa390f1, 0x402e9141, 0x40b99192, 0x414491e4, 0x41ce9236, 0x42589289, 0x42e192dd, 0x436a9331, 0x43f39386, 0x447b93dc, 0x45029432, 0x458a9489, 0x461194e1, 0x46979539, 0x471d9592, 0x47a295ec, 0x48289646, 0x48ac96a1, 0x493096fd, 0x49b49759, 0x4a3797b6, 0x4aba9814, 0x4b3d9872, 0x4bbe98d1, 0x4c409930, 0x4cc19991, 0x4d4199f1, 0x4dc19a53, 0x4e419ab5, 0x4ec09b17, 0x4f3e9b7b, 0x4fbd9bdf, 0x503a9c43, 0x50b79ca8, 0x51349d0e, 0x51b09d74, 0x522b9ddb, 0x52a79e43, 0x53219eab, 0x539b9f14, 0x54159f7d, 0x548e9fe7, 0x5506a052, 0x557ea0bd, 0x55f6a129, 0x566da195, 0x56e3a202, 0x5759a26f, 0x57cea2dd, 0x5843a34c, 0x58b7a3bb, 0x592ba42b, 0x599ea49b, 0x5a10a50c, 0x5a82a57e, 0x5af4a5f0, 0x5b65a662, 0x5bd5a6d5, 0x5c45a749, 0x5cb4a7bd, 0x5d23a832, 0x5d91a8a7, 0x5dfea91d, 0x5e6ba993, 0x5ed7aa0a, 0x5f43aa82, 0x5faeaafa, 0x6019ab72, 0x6083abeb, 0x60ecac65, 0x6155acdf, 0x61bdad59, 0x6225add5, 0x628cae50, 0x62f2aecc, 0x6358af49, 0x63bdafc6, 0x6421b043, 0x6485b0c2, 0x64e9b140, 0x654bb1bf, 0x65adb23f, 0x660fb2bf, 0x666fb33f, 0x66d0b3c0, 0x672fb442, 0x678eb4c3, 0x67ecb546, 0x684ab5c9, 0x68a7b64c, 0x6903b6d0, 0x695fb754, 0x69bab7d8, 0x6a14b85e, 0x6a6eb8e3, 0x6ac7b969, 0x6b1fb9ef, 0x6b77ba76, 0x6bcebafe, 0x6c24bb85, 0x6c7abc0d, 0x6ccfbc96, 0x6d23bd1f, 0x6d77bda8, 0x6dcabe32, 0x6e1cbebc, 0x6e6ebf47, 0x6ebfbfd2, 0x6f0fc05d, 0x6f5fc0e9, 0x6faec175, 0x6ffcc201, 0x704ac28e, 0x7097c31c, 0x70e3c3a9, 0x712ec437, 0x7179c4c6, 0x71c3c555, 0x720dc5e4, 0x7255c673, 0x729dc703, 0x72e4c793, 0x732bc824, 0x7371c8b5, 0x73b6c946, 0x73fac9d7, 0x743eca69, 0x7481cafc, 0x74c3cb8e, 0x7505cc21, 0x7546ccb4, 0x7586cd48, 0x75c5cddc, 0x7604ce70, 0x7642cf04, 0x767fcf99, 0x76bbd02e, 0x76f7d0c3, 0x7732d159, 0x776cd1ef, 0x77a6d285, 0x77dfd31c, 0x7817d3b2, 0x784ed449, 0x7885d4e1, 0x78bad578, 0x78efd610, 0x7924d6a8, 0x7957d741, 0x798ad7d9, 0x79bcd872, 0x79eed90b, 0x7a1ed9a5, 0x7a4eda3e, 0x7a7ddad8, 0x7aabdb72, 0x7ad9dc0c, 0x7b06dca7, 0x7b32dd41, 0x7b5ddddc, 0x7b88de77, 0x7bb1df13, 0x7bdadfae, 0x7c03e04a, 0x7c2ae0e6, 0x7c51e182, 0x7c77e21e, 0x7c9ce2bb, 0x7cc0e358, 0x7ce4e3f4, 0x7d07e492, 0x7d29e52f, 0x7d4ae5cc, 0x7d6be66a, 0x7d8ae707, 0x7da9e7a5, 0x7dc8e843, 0x7de5e8e1, 0x7e02e980, 0x7e1eea1e, 0x7e39eabc, 0x7e53eb5b, 0x7e6debfa, 0x7e85ec99, 0x7e9ded38, 0x7eb5edd7, 0x7ecbee76, 0x7ee1ef16, 0x7ef5efb5, 0x7f0af055, 0x7f1df0f5, 0x7f2ff194, 0x7f41f234, 0x7f52f2d4, 0x7f62f374, 0x7f72f414, 0x7f80f4b4, 0x7f8ef555, 0x7f9bf5f5, 0x7fa7f695, 0x7fb3f736, 0x7fbdf7d6, 0x7fc7f877, 0x7fd0f918, 0x7fd9f9b8, 0x7fe0fa59, 0x7fe7fafa, 0x7fedfb9a, 0x7ff2fc3b, 0x7ff6fcdc, 0x7ffafd7d, 0x7ffcfe1d, 0x7ffefebe, 0x7fffff5f, 0x7fff0000, 0x7fff00a1, 0x7ffe0142, 0x7ffc01e3, 0x7ffa0283, 0x7ff60324, 0x7ff203c5, 0x7fed0466, 0x7fe70506, 0x7fe005a7, 0x7fd90648, 0x7fd006e8, 0x7fc70789, 0x7fbd082a, 0x7fb308ca, 0x7fa7096b, 0x7f9b0a0b, 0x7f8e0aab, 0x7f800b4c, 0x7f720bec, 0x7f620c8c, 0x7f520d2c, 0x7f410dcc, 0x7f2f0e6c, 0x7f1d0f0b, 0x7f0a0fab, 0x7ef5104b, 0x7ee110ea, 0x7ecb118a, 0x7eb51229, 0x7e9d12c8, 0x7e851367, 0x7e6d1406, 0x7e5314a5, 0x7e391544, 0x7e1e15e2, 0x7e021680, 0x7de5171f, 0x7dc817bd, 0x7da9185b, 0x7d8a18f9, 0x7d6b1996, 0x7d4a1a34, 0x7d291ad1, 0x7d071b6e, 0x7ce41c0c, 0x7cc01ca8, 0x7c9c1d45, 0x7c771de2, 0x7c511e7e, 0x7c2a1f1a, 0x7c031fb6, 0x7bda2052, 0x7bb120ed, 0x7b882189, 0x7b5d2224, 0x7b3222bf, 0x7b062359, 0x7ad923f4, 0x7aab248e, 0x7a7d2528, 0x7a4e25c2, 0x7a1e265b, 0x79ee26f5, 0x79bc278e, 0x798a2827, 0x795728bf, 0x79242958, 0x78ef29f0, 0x78ba2a88, 0x78852b1f, 0x784e2bb7, 0x78172c4e, 0x77df2ce4, 0x77a62d7b, 0x776c2e11, 0x77322ea7, 0x76f72f3d, 0x76bb2fd2, 0x767f3067, 0x764230fc, 0x76043190, 0x75c53224, 0x758632b8, 0x7546334c, 0x750533df, 0x74c33472, 0x74813504, 0x743e3597, 0x73fa3629, 0x73b636ba, 0x7371374b, 0x732b37dc, 0x72e4386d, 0x729d38fd, 0x7255398d, 0x720d3a1c, 0x71c33aab, 0x71793b3a, 0x712e3bc9, 0x70e33c57, 0x70973ce4, 0x704a3d72, 0x6ffc3dff, 0x6fae3e8b, 0x6f5f3f17, 0x6f0f3fa3, 0x6ebf402e, 0x6e6e40b9, 0x6e1c4144, 0x6dca41ce, 0x6d774258, 0x6d2342e1, 0x6ccf436a, 0x6c7a43f3, 0x6c24447b, 0x6bce4502, 0x6b77458a, 0x6b1f4611, 0x6ac74697, 0x6a6e471d, 0x6a1447a2, 0x69ba4828, 0x695f48ac, 0x69034930, 0x68a749b4, 0x684a4a37, 0x67ec4aba, 0x678e4b3d, 0x672f4bbe, 0x66d04c40, 0x666f4cc1, 0x660f4d41, 0x65ad4dc1, 0x654b4e41, 0x64e94ec0, 0x64854f3e, 0x64214fbd, 0x63bd503a, 0x635850b7, 0x62f25134, 0x628c51b0, 0x6225522b, 0x61bd52a7, 0x61555321, 0x60ec539b, 0x60835415, 0x6019548e, 0x5fae5506, 0x5f43557e, 0x5ed755f6, 0x5e6b566d, 0x5dfe56e3, 0x5d915759, 0x5d2357ce, 0x5cb45843, 0x5c4558b7, 0x5bd5592b, 0x5b65599e, 0x5af45a10, 0x5a825a82, 0x5a105af4, 0x599e5b65, 0x592b5bd5, 0x58b75c45, 0x58435cb4, 0x57ce5d23, 0x57595d91, 0x56e35dfe, 0x566d5e6b, 0x55f65ed7, 0x557e5f43, 0x55065fae, 0x548e6019, 0x54156083, 0x539b60ec, 0x53216155, 0x52a761bd, 0x522b6225, 0x51b0628c, 0x513462f2, 0x50b76358, 0x503a63bd, 0x4fbd6421, 0x4f3e6485, 0x4ec064e9, 0x4e41654b, 0x4dc165ad, 0x4d41660f, 0x4cc1666f, 0x4c4066d0, 0x4bbe672f, 0x4b3d678e, 0x4aba67ec, 0x4a37684a, 0x49b468a7, 0x49306903, 0x48ac695f, 0x482869ba, 0x47a26a14, 0x471d6a6e, 0x46976ac7, 0x46116b1f, 0x458a6b77, 0x45026bce, 0x447b6c24, 0x43f36c7a, 0x436a6ccf, 0x42e16d23, 0x42586d77, 0x41ce6dca, 0x41446e1c, 0x40b96e6e, 0x402e6ebf, 0x3fa36f0f, 0x3f176f5f, 0x3e8b6fae, 0x3dff6ffc, 0x3d72704a, 0x3ce47097, 0x3c5770e3, 0x3bc9712e, 0x3b3a7179, 0x3aab71c3, 0x3a1c720d, 0x398d7255, 0x38fd729d, 0x386d72e4, 0x37dc732b, 0x374b7371, 0x36ba73b6, 0x362973fa, 0x3597743e, 0x35047481, 0x347274c3, 0x33df7505, 0x334c7546, 0x32b87586, 0x322475c5, 0x31907604, 0x30fc7642, 0x3067767f, 0x2fd276bb, 0x2f3d76f7, 0x2ea77732, 0x2e11776c, 0x2d7b77a6, 0x2ce477df, 0x2c4e7817, 0x2bb7784e, 0x2b1f7885, 0x2a8878ba, 0x29f078ef, 0x29587924, 0x28bf7957, 0x2827798a, 0x278e79bc, 0x26f579ee, 0x265b7a1e, 0x25c27a4e, 0x25287a7d, 0x248e7aab, 0x23f47ad9, 0x23597b06, 0x22bf7b32, 0x22247b5d, 0x21897b88, 0x20ed7bb1, 0x20527bda, 0x1fb67c03, 0x1f1a7c2a, 0x1e7e7c51, 0x1de27c77, 0x1d457c9c, 0x1ca87cc0, 0x1c0c7ce4, 0x1b6e7d07, 0x1ad17d29, 0x1a347d4a, 0x19967d6b, 0x18f97d8a, 0x185b7da9, 0x17bd7dc8, 0x171f7de5, 0x16807e02, 0x15e27e1e, 0x15447e39, 0x14a57e53, 0x14067e6d, 0x13677e85, 0x12c87e9d, 0x12297eb5, 0x118a7ecb, 0x10ea7ee1, 0x104b7ef5, 0x0fab7f0a, 0x0f0b7f1d, 0x0e6c7f2f, 0x0dcc7f41, 0x0d2c7f52, 0x0c8c7f62, 0x0bec7f72, 0x0b4c7f80, 0x0aab7f8e, 0x0a0b7f9b, 0x096b7fa7, 0x08ca7fb3, 0x082a7fbd, 0x07897fc7, 0x06e87fd0, 0x06487fd9, 0x05a77fe0, 0x05067fe7, 0x04667fed, 0x03c57ff2, 0x03247ff6, 0x02837ffa, 0x01e37ffc, 0x01427ffe, 0x00a17fff};


VMEM_SECTION unsigned int all_one[16] = {0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000, 0x00004000};

//for STO and CFO test (0 offset, 0 Hz CFO)
//VMEM_SECTION unsigned int benchmark_coarse_sync[16] = {0x5, 0x4f8, 0x4f4, 0x4f9, 0x4f8, 0x4fb, 0x4f8, 0x4f6, 0x4f9, 0x4f8, 0x4fa, 0x4f8, 0x4f7, 0x4f9, 0x4f8};

// for STO and CFO test (0 offset, -50 Hz CFO)
//VMEM_SECTION unsigned int benchmark_coarse_sync[16] = {0x10, 0xb, 0x6, 0x5, 0x6, 0x8, 0x8, 0x6, 0x6, 0x6, 0x7, 0x7, 0x6, 0x6, 0x6};

// for STO and CFO test (0 offset, 100 Hz CFO)
//VMEM_SECTION unsigned int benchmark_coarse_sync[16] = {0x4fc, 0x7, 0x7, 0x4, 0x5, 0x3, 0x5, 0x6, 0x4, 0x5, 0x4, 0x5, 0x5, 0x5, 0x5};

// for STO and CFO test (0 offset, 75 Hz CFO)
//VMEM_SECTION unsigned int benchmark_coarse_sync[16] = {0x7, 0x1, 0x4fc, 0x4fe, 0x0, 0x1, 0x0, 0x4fe, 0x4ff, 0x0, 0x1, 0x0, 0x4ff, 0x4ff, 0x0};

// clean data for STO and CFO test 
VMEM_SECTION unsigned int benchmark_coarse_sync[16] = {0x2, 0x4ff, 0x4fc, 0x4fe, 0x1, 0x1, 0x0, 0x4ff, 0x4ff, 0x1, 0x1, 0x1, 0x0, 0x0, 0x1};






VMEM_SECTION unsigned int power_data_copy[1024] = {0};


// //////////////////////////////////////////////////////////////

unsigned int fsm_state;

unsigned int last_error_report;


#define MY_ASSERT(x) if(!(x)) { ring_block_send_eth(0xe0000000|__LINE__);}


void fft_accept_new(unsigned int dma_ptr);
void dma_out_set_safe(unsigned int dma_ptr, unsigned int size);

// declare as global
VMalloc mgr;


// #define FFT_CP_SAMPLES (384) // works (3/8)
// #define FFT_CP_SAMPLES (320) // works (5/16)
// #define FFT_CP_SAMPLES (288) // works (9/32)
#define FFT_CP_SAMPLES (256) // works (1/4)
// #define FFT_CP_SAMPLES (128) // does not work

#define DMA_IN_CHUNK (1024) + FFT_CP_SAMPLES

#define DMA_OUT_CHUNK (1024)

// must be power of two, must change next as well
#define DMA_IN_COUNT (4)
#define DMA_IN_COUNT_MASK 0x3

// this size runs before sync
#define FIRST_HOLDOVER_MULTIPLIER (2)
#define FIRST_HOLDOVER_SIZE (1280*FIRST_HOLDOVER_MULTIPLIER)


// how many ofdm symbols go by before we adjust
unsigned int packet_num_SFO_adjustment_period = 0; //71111;

unsigned int packet_counter_SFO_adjustment = 0; // how many ofdm symbols we have, seen since last adjustment

// determines the direction we adjust, and disables when zero
// 0 is no adjustment
// 1 delete
// 2 add
unsigned int packet_SFO_adjustment_direction = 0;

// how many samples to + / - each period
// ALWAYS positive, as sign is encoded into  packet_SFO_adjustment_direction
unsigned int packet_SFO_adjustment_step = 0; 

unsigned int packet_STO_adjustment_step = 0;

unsigned int packet_STO_adjustment_neg_step = 0;


unsigned int ringbus_sfo_adjustment_temp = 0;

// Call this first
void sfo_adjustment_callback(unsigned int data) {
    ringbus_sfo_adjustment_temp = data;
    // ring_block_send_eth(data);
}

// Call this second, settings are not applied till this is called
// 0 is disable sfo adjustment
// 1 is delete (1 sample)
// 2 is add (1 sample)
void __attribute__((optimize("Os"))) sfo_sign_callback(unsigned int data) {
    
    

    if( data == 0 ) 
    {
        packet_num_SFO_adjustment_period = ringbus_sfo_adjustment_temp;
        packet_SFO_adjustment_direction = data;
        packet_SFO_adjustment_step = 0;
        packet_counter_SFO_adjustment = 0;
    } else if (data == 1 || data == 2) 
    {
        packet_num_SFO_adjustment_period = ringbus_sfo_adjustment_temp;
        packet_SFO_adjustment_direction = data;
        packet_SFO_adjustment_step = 1;
        packet_counter_SFO_adjustment = 0;
    } 
    else if (data == 3)
    {
        packet_STO_adjustment_step = ringbus_sfo_adjustment_temp;
    }
    else if (data == 4)
    {
        packet_STO_adjustment_neg_step = ringbus_sfo_adjustment_temp;
    }

    
    // ring_block_send_eth(ringbus_sfo_adjustment_temp);
    // ring_block_send_eth(data);

}

// 0 a
// 1 b
int dma_state = 0;
int dma_in_valid = -1;
unsigned int dma_in_ptr[2];
unsigned int nco_in_ptr[2];
int fft_a_empty;
int fft_b_empty;

int fft_ready = -1;
int fft_valid = -1;
unsigned int fft_ptr[2];


int dma_out_valid = -1;
int dma_out_ready = -1;

unsigned int nco_angle = 0;
unsigned int nco_delta = 0;
unsigned int cfo_compensation_direction = 1;
unsigned int nco_angle_delta=0;

unsigned int input_expected_occupany = 0;


void post_sync_prime_ping_pong() {
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
void __attribute__((optimize("Os"))) trig_dma_in(unsigned int idx, unsigned int timer_start, unsigned int advance) {
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
        packet_STO_adjustment_neg_step = 0;
        packet_STO_adjustment_step = 0;
        SET_REG(x3, 0xf0000000);
    } else {


        if(packet_counter_SFO_adjustment != packet_num_SFO_adjustment_period) {
            CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx])+packet_STO_adjustment_neg_step);
            CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance-packet_STO_adjustment_neg_step+packet_STO_adjustment_step);
            CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
            CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value
            packet_STO_adjustment_neg_step = 0;
            packet_STO_adjustment_step = 0;
            packet_counter_SFO_adjustment++;
        } else {
            // mode 1 delete 
            SET_REG(x3, 0xd0000000);
            SET_REG(x4, packet_SFO_adjustment_direction);
            if( packet_SFO_adjustment_direction == 1 ) {
                SET_REG(x3, 0xc0000000);
                CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx]));
                CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance+packet_SFO_adjustment_step);
                CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
                CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value
            } else if (packet_SFO_adjustment_direction == 2) {
                SET_REG(x3, 0xb0000000);
                // mode 2 add
                CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx]));
                CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance-packet_SFO_adjustment_step);
                CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
                CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);

                // in add, we just copy
                if(packet_SFO_adjustment_step!=0)
                {
                    SET_REG(x3, 0xa0000000);
                    // FIXME: only valid when packet_SFO_adjustment_step = 1.
                    vector_memory[VMEM_DMA_ADDRESS(dma_in_ptr[idx])+1279] = vector_memory[VMEM_DMA_ADDRESS(dma_in_ptr[idx])+1278];
                }
                
            }
            // if we hit the counter
            packet_counter_SFO_adjustment = 0;
            // ring_block_send_eth(0xff000000);
        
        }
    } 

    

    // if(packet_SFO_adjustment_direction == 0)
    // {
    //     CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx]));
    //     CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance);
    //     CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
    //     CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value
  
    //     packet_counter_SFO_adjustment++;
    //     if(packet_counter_SFO_adjustment == (packet_num_SFO_adjustment_period-1))
    //     {
    //         packet_SFO_adjustment_direction = 2;
    //     }
    //   }
    //   else if(packet_SFO_adjustment_direction == 1)
    //   {
    //     CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx]));
    //     CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance+packet_SFO_adjustment_step);
    //     CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
    //     CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value
        
    //     packet_counter_SFO_adjustment = 0;
    //     packet_SFO_adjustment_direction = 0;
    //   } 
    //   else if(packet_SFO_adjustment_direction == 2)
    //   {
    //   	CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx]));
    //     CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance-packet_SFO_adjustment_step);
    //     CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
    //     CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0); 
        
    //     if(packet_SFO_adjustment_step!=0)
    //     {
    //     	vector_memory[VMEM_DMA_ADDRESS(dma_in_ptr[idx])+1279] = vector_memory[VMEM_DMA_ADDRESS(dma_in_ptr[idx])+1278];
    //     }
        
  
    //     packet_counter_SFO_adjustment = 0;
    //     packet_SFO_adjustment_direction = 0;
    // } 
  // 
  // if (packet_counter_SFO_adjustment == packet_num_SFO_adjustment_period)
  // {
  // 	packet_counter_SFO_adjustment = 0;


  //   CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(delete_data));
  //   CSR_WRITE(DMA_0_LENGTH, 1);
  //   CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
  //   CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);

  // }

  // timer_start += 4096;
}

#define MAX_TURNSTILE_ADVANCE (1024+256)

VMEM_SECTION unsigned int dma_buffer_a[1024+256+MAX_TURNSTILE_ADVANCE];
VMEM_SECTION unsigned int dma_buffer_b[1024+256+MAX_TURNSTILE_ADVANCE];

VMEM_SECTION unsigned int nco_buffer_a[1024];
VMEM_SECTION unsigned int nco_buffer_b[1024];

// FIXME: this function is big for some reason
// test in hardware and add
// __attribute__((optimize("Os")))
void  setup_dma_in(void) {
  dma_in_ptr[0] = (unsigned int) dma_buffer_a;
  dma_in_ptr[1] = (unsigned int) dma_buffer_b;

  nco_in_ptr[0] = (unsigned int) nco_buffer_a;
  nco_in_ptr[1] = (unsigned int) nco_buffer_b;


  trig_dma_in(0, 0xffffffff, 0);
  trig_dma_in(1, 0xffffffff, 0);

  trigger_update_nco(0);
  trigger_update_nco(1);

  input_expected_occupany = 2;
}

void mark_both_fft_empty(void) {
    fft_a_empty = 1;
    fft_b_empty = 1;
}

fft1024_cfg_t active_plan;

void setup_fft(void) {
  fft_ptr[0] = (unsigned int) vmalloc_single(&mgr);
  fft_ptr[1] = (unsigned int) vmalloc_single(&mgr);
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

void __attribute__((optimize("Os"))) fft_barrel_shift_write_out() {
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






void setup_dma_out(void) {
    CIRBUF_POW2_RUNTIME_INITIALIZE(dma_out_started);
}



void setup_fsm(void) {
    fsm_state = FSM_PING_PONG;
}


unsigned pending_sync_symbol_timing = 0;

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
                if(occupancy == 2 && input_expected_occupany == 2) {
                    pending_sync_symbol_timing = 0; // reset flag
                    next_state = FSM_FLUSHING;
                    needs_holdover = 1; // set flag so input dma will holdover buffer before sync
                    coarse_sync_zero_output_counter = 0; // set to zero, 
                }
            }
            break;
        case FSM_DO_SYNC:
            if(sync_finished) {
                next_state = FSM_PING_PONG;
                sync_finished = 0;

                post_sync_prime_ping_pong();

                setup_dma_in(); // re prime the pump, other setups don't need to be called so far
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
        // unsigned int dma = _mgr.chunk_dma_address[0];
        // unsigned int testbench_row = VMEM_DMA_ADDRESS_TO_ROW(dma);
        
        int consume_idx = dma_state;
        unsigned int* cpu_ptr_from_dma = (unsigned int*) dma_in_ptr[consume_idx];
        unsigned int input_row = VMEM_ROW_ADDRESS(cpu_ptr_from_dma);
        // vmem_copy_rows(input_row, testbench_row, 1024/16);

        // copy a second time

        unsigned int power_row = VMEM_ROW_ADDRESS(power_data_copy);
        vmem_copy_rows(input_row, power_row, 1024/16);

        SET_REG(x3, 0x10000000);
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
  int error;
  unsigned int data;
  unsigned int helper;
  unsigned int set_pace = 0;
  static unsigned int pace;

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
            SET_REG(x4, 0xa0000000 | pending_input_advance);
            ring_block_send_eth(DEBUG_13_PCCMD | (pending_input_advance&0xffffff) );
            pending_input_advance = 0;
        }

        // fire off one for every input
        trigger_update_nco(fft_ready);
    } else if( fsm_state == FSM_FLUSHING ) {

        // one time trigger a flushing buffer between ping-pong and sync
        if( needs_holdover ) {
            needs_holdover = 0;
            dma_in_set(VMEM_DMA_ADDRESS(holdover_data), FIRST_HOLDOVER_SIZE);
        }

    }


    // SET_REG(x3, (2<<8) | fft_ready);
    fft_ready = -1;

  }
}

// pass the index of the NCO buffer you wish to trigger into
void trigger_update_nco(unsigned int idx) {
    make_nco(VMEM_DMA_ADDRESS(nco_in_ptr[idx]), 1024, nco_angle, nco_delta);
    nco_angle = (nco_angle + nco_angle_delta);
}

// at some point the fft needed to respond to signals without going into the full pet_fft
// this was created to call more frequently to update flags
void pet_fft_respond_signals() {
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

    SET_REG(x3, (0x4a << 8) | dma_out_ready);
    SET_REG(x3, (0x4b << 8) | (fft_a_empty << 1) | fft_b_empty);

    dma_out_ready = -1;

  }
}


void pet_fft() {
  // example only starts to work once we have 2 items in the queue

  int error;
  static unsigned updates = 1;

  // deals with dma telling us we are done
  pet_fft_respond_signals();

  unsigned int output_blocked = 0;


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

    SET_REG(x3, (3 << 8) | (fft_a_empty << 1) | fft_b_empty);

    unsigned int* cpu_ptr_from_dma = (unsigned int*) dma_in_ptr[consume_idx];
    unsigned int* cpu_ptr_fft      = (unsigned int*) fft_ptr[consume_idx];

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

    fft_1024_run(&active_plan);
    // for(unsigned int i = 0; i < 250; i++) {
    //     STALL(100);
    // }

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
               ring_block_send_eth(RX_CS00_ERROR_PCCMD);
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
  int helper;
  unsigned int occupancy;
  int error;
  unsigned int data;
  unsigned int output_blocked = 0;
  unsigned int dma_occupancy_combined;
  unsigned int dma_occupancy;
  unsigned int dma_occupancy_status;

  // we can tell by the size of our cirbuf if we have room 
  // for instance we get a 3 here after 0x500, 0x501, 0x500 meaning that ...
  // if fft is signaling to us that it just finished a buffer
  if(fft_valid != -1) {
    occupancy = circular_buf2_occupancy(&dma_out_started);
    SET_REG(x3, (7<<8) | occupancy );
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

    SET_REG(x3, (0x5<<8) | consume_idx);

    unsigned int* cpu_ptr_from_fft = (unsigned int*) fft_ptr[consume_idx];

    if( enable_output_stream == 1 ) {

        // error = circular_buf_put(&dma_out_started, consume_idx); MY_ASSERT(error == 0);
        error = circular_buf2_put(&dma_out_started, consume_idx | IS_SECOND_DMA); MY_ASSERT(error == 0);



        // schedule the CP
        // dma_out_set_safe(VMEM_DMA_ADDRESS(cpu_ptr_from_fft)+(DMA_IN_CHUNK-FFT_CP_SAMPLES), FFT_CP_SAMPLES);

        // schedule the full FFT
        dma_out_set_safe(VMEM_DMA_ADDRESS(cpu_ptr_from_fft), DMA_OUT_CHUNK);

    } else {
        // output stream disabled

        // instantly free buffer for upstream.
        // unsure how to handle situation when changing from enabled to disabled and reverse
        
        dma_out_ready = consume_idx;
        pet_fft_respond_signals();
    }

    _perf_work(); // triggers when we SEND and not complete dma but that's ok

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
    SET_REG(x3, (6<<8) | data );

    if(error == 0) {
      if( data & IS_SECOND_DMA ) { // always true, there is only one output dma
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
void turnstile_advance_callback(unsigned int data) {
    turnstile_advance(data);
}


// data is already masked and has upper 8 bits removed at this point
void sync_callback(unsigned int data) {

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

    // enable_output_stream = 0;
}


// returns 0 for success, non zero when more output
// zeros are required
unsigned int hand_tune_coarse_sync_zeros() {
    // discovered using verilator and an input counter
    // to see how many samples are dropped during xbb_coarse_sync()
    // const unsigned int hand_tuned_chunks = 20;

    // unsigned int occupancy;
    // if(coarse_sync_zero_output_counter < hand_tuned_chunks) {
    //     // DMA_1_SCHEDULE_DEPTH
    //     CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);

    //     if( occupancy < DMA_1_SCHEDULE_DEPTH ) {
    //         dma_out_set(VMEM_DMA_ADDRESS(zero_buffer), DMA_OUT_CHUNK);
    //         coarse_sync_zero_output_counter++;
    //     }

    //     // return number of chunks left
    //     return hand_tuned_chunks - coarse_sync_zero_output_counter;

    // } else {
    //     return 0;
    // }
    return 0;
}

// call once manually when hand_tune_coarse_sync_zeros() is done
void hand_tune_coarse_sync_cleanup() {

    ///
    /// this function sends a message to cs21 to adjust lifetime counter
    /// this adjustment should only be used to remove glitches and not to add them
    ///

    //! change this
    //! a value of 2 here means a shift of 4 because each frame is 2 bits
    //! set to a value of 0 and no rb is event sent!, this will supress any reporting from 21
    //! as it will not get anything
    // signed int hand_tune = 0;

    // we have a fun 5:4 ratio, but if we do all coarse sync in multiple of 5, then we can output
    // in multiple of 4


    // we always add in new method so positibe
    // we are making up for lost time, so we do this

    // see r35.txt, adding values shifts to the left

    // from 15 to 25 need to add (6 frames?

    signed int hand_tune =
         FIRST_HOLDOVER_MULTIPLIER 
         + 4  // zhen does a 4 and 1 of size 1024, but that gives us 4
         + (((global_coarse_sync_num)/5)*4) // must be a multiple of 5

         // + 11 // or minus?
         + 8 + 1 + 1 - 9// this is my fudge
         ;

    // start with 2

    // number of 1024
    // COARSE_SYNC_OFDM_NUM

    // number of 1280
    // FIRST_HOLDOVER_MULTIPLIER


    // 2 plus this number over 5

    // plus 4 at the beginning and 1 at the end (Which is 5)
    // 
    // hand_tune -= 4; // plus 5 above

    unsigned int hand_tune_adjust_magnitude;
    // 1 is add
    // 2 is subtract
    // this number is shifted left by 16
    unsigned int dmode;

    if(hand_tune < 0) {
        hand_tune_adjust_magnitude = - hand_tune; // the - is negation
        dmode = ( (2)<<16 );
    } else { 
        hand_tune_adjust_magnitude = hand_tune;
        dmode = ( (1)<<16 );
    }


    unsigned int final_data = CS21_ADVANCE_LIFETIME | dmode | hand_tune_adjust_magnitude;

    if( hand_tune_adjust_magnitude != 0 ) {
        CSR_WRITE(RINGBUS_WRITE_ADDR, RING_ADDR_RX_TAGGER);
        CSR_WRITE(RINGBUS_WRITE_DATA, final_data);
        CSR_WRITE_ZERO(RINGBUS_WRITE_EN);
    }
}

// pass a pointer to a result struct
// pass a number of iterations to run sync `coarse_sync_number`
void xbb_coarse_sync(CFO_Results *CFO_results, unsigned int coarse_sync_number) 
{
  
  Ringbus ringbus;
  
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


  unsigned int occupancy;


  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
  SET_REG(x4, 0xe0000000 | occupancy);

  unsigned int start_clk, end_clk;

   

  unsigned int intput_data_location=VMEM_ROW_ADDRESS(input_data);
  unsigned int dma_in_address=VMEM_DMA_ADDRESS(input_data);


  //unsigned int exp_data_1280_ext_location=VMEM_ROW_ADDRESS(exp_data_1280_ext_14);
  unsigned int exp_data_1280_ext_location=VMEM_ROW_ADDRESS(exp_data_1280_ext_15);
  exp_data_1280_ext_location += 80;
    
  unsigned int output_coarse_sync_location = VMEM_ROW_ADDRESS(output_coarse_sync);
  unsigned int output_sum_complex_location = VMEM_ROW_ADDRESS(output_sum_complex);
  unsigned int output_onetone_calculation_location = VMEM_ROW_ADDRESS(output_onetone_calculation);
  unsigned int output_conj_multi_location = VMEM_ROW_ADDRESS(output_conj_multi);
  unsigned int benchmark_coarse_sync_location=VMEM_ROW_ADDRESS(benchmark_coarse_sync);


  unsigned int all_one_location=VMEM_ROW_ADDRESS(all_one);
  unsigned int bank_address_onetone_calculation_location = VMEM_ROW_ADDRESS(bank_address_onetone_calculation);
  unsigned int permutation_address_onetone_calculation_location = VMEM_ROW_ADDRESS(permutation_address_onetone_calculation);
  

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
  unsigned int temp_complex_pre;
  unsigned int index_done;
  unsigned int index_done_pre = 0;
  unsigned int index_final;
  unsigned int recorded_data_flag = 0; // did we ever overflow and record data into the histogram?


  unsigned int temp_angle;
  unsigned int temp_offset;
  unsigned int temp_offset_benchmark;

  MVXV_KNOP(V11, output_sum_complex_location);
  MVXV_KNOP(V10, output_onetone_calculation_location);

  SET_REG(x3, 0xdead0002);

  // use dma_block_get here so we will automatically not overflow input dma
  for(int index=0; index<4; index++)
  {
    SET_REG(x4, 0xa0000000 | index);
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

    hand_tune_coarse_sync_zeros(); // set output zeros
  }

  SET_REG(x4, 0xb0000000);
   

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

  unsigned int coarse_sync_estimate_array[(1280>>6)] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  soft_queue_ring_eth(0x66666666);
  unsigned int index_counter = 0;

  unsigned int temp_offset_average = 0;
  unsigned int temp_offset_counter = 0;
  // for each iteration
  for(int index=0; index<coarse_sync_number; index++)
  {

    SET_REG(x4, 0xc0000000 | index);
    
    while(1)
    {
     CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
     SET_REG(x4, 0xd0000000 | occupancy);
     if(occupancy == expected_occupancy) 
     {
      break;
     }

    }

    // CSR_READ(TIMER_VALUE, start_clk);

    //////// do coarse sync
    input_conj_multi_location_0=intput_data_location+coarse_sync_indicator*64;
    coarse_sync_indicator = (coarse_sync_indicator+1);
    if (coarse_sync_indicator == 5) {
        coarse_sync_indicator = 0;
    }
    
    input_conj_multi_location_1=intput_data_location+(coarse_sync_indicator)*64;

    // see https://beta.observablehq.com/@drom/ofdm-symbol-synchronisation/2

    // CSR_READ(TIMER_VALUE, start_clk);
    xbb_conj_multi(
        VMEM_ROW_ADDRESS(config_word_conj_eq_0f), 
        input_conj_multi_location_0, // previous 1024 samples X_m-1
        input_conj_multi_location_1, // current 1024 samples  X_m
        output_conj_multi_location // output Y_m
        );
    // CSR_READ(TIMER_VALUE, end_clk);   
    exp_data_location=exp_data_1280_ext_location;

    xbb_onetone_calculation(
        VMEM_ROW_ADDRESS(config_word_cmul_rx4_0f),
        output_conj_multi_location,        // input to this function, which is the output of the previous Y_m
        exp_data_location,                 // start address of the exponential sin wave (1024 samples)
        output_onetone_calculation_location,      // this is a 1024 vector of output, but only [0] has valid data
        bank_address_onetone_calculation_location, // data for internals of xbb_onetone_calculation
        permutation_address_onetone_calculation_location, // data for internals
        all_one_location // data for internals
        );

    /// we can try config_word_cmul_rx4_10 later
   // change on june 14
    // xbb_onetone_calculation(VMEM_ROW_ADDRESS(config_word_cmul_rx4_12), output_conj_multi_location, exp_data_location, output_onetone_calculation_location, bank_address_onetone_calculation_location, permutation_address_onetone_calculation_location, all_one_location);

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

    MVXV_KNOP(V0, VMEM_ROW_ADDRESS(config_word_add_eq_00));

    VNOP_LK14(V0);

    FLUSH_CONFIG_WORD(V15);

    VNOP_LK8(V10);
    VNOP_LK9(V11);
    VNOP_SK1(V11);
    STALL(50);

    temp_complex = vector_memory[(output_sum_complex_location)*16];

    index_final = index;

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
        index_done = index;

        // if we didn't overflow, save into temp_complex_pre
        temp_complex_pre = temp_complex;
    }

    // is this the last loop?
    unsigned last_loop = (index == (coarse_sync_number-1));

    // we force enter data into the histogram if this is the last loop iteration
    // and so far we have not recorded anything.  this protects against small ofdm numbers
    // returning 0
    unsigned save_last_loop = last_loop && (recorded_data_flag==0);

    // once we overflow, we don't use overflown (current) data, instead we use 
    // temp_complex_pre, which was the last good value before overflow
    // this only checks every 5 dma
     if( ( (flag_temp_complex == 0) && (index_counter == 4) ) || save_last_loop )
     {
        recorded_data_flag = 1;

        ATAN(temp_angle,temp_complex_pre,15);
        temp_angle = temp_angle&0xffff;
        temp_offset = ((((temp_angle+6553)&0xffff)*1280)>>16);

        temp_offset_average = temp_offset_average + temp_offset;
        temp_offset_counter = temp_offset_counter + 1;

        if(temp_offset == 1280)
            temp_offset = 0;
        
        if(index_done - index_done_pre>=0)
        {
            coarse_sync_estimate_array[temp_offset>>6] = coarse_sync_estimate_array[temp_offset>>6]+1;
            //ring_block_send_eth(index_done-index_done_pre);
            // ring_block_send_eth(temp_offset);
        }

        vector_memory[(output_sum_complex_location)*16] = 0;
        STALL(50);

        flag_temp_complex = 1;
        index_done_pre = index+1;

       

     }

     index_counter = index_counter + 1;
     if(index_counter == 5)
     {
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
    SET_REG(x4, 0xd0000000 | occupancy);
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
    hand_tune_coarse_sync_zeros();

  } //// for index < coarse_sync_number




  // ring_block_send_eth(0x88888888);
  // ring_block_send_eth(temp_offset);
  temp_offset = (unsigned int)(temp_offset_average*1.0)/(temp_offset_counter *1.0);
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
  SET_REG(x4, 0xd0000000 | occupancy);
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

  // CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
  // ring_block_send_eth(occupancy);

  // report value to pc
  ring_block_send_eth(COARSE_SYNC_PCCMD | (temp_offset&DATA_MASK));


  dma_in_set(VMEM_DMA_ADDRESS(holdover_data), 1280*2);

  while(hand_tune_coarse_sync_zeros()!=0) {
    // call again
  }

  hand_tune_coarse_sync_cleanup();

}

CFO_Results cfo_run_results;

void pet_symbol_sync() {
    if( fsm_state == FSM_DO_SYNC ) {
        xbb_coarse_sync(&cfo_run_results, global_coarse_sync_num);
        // ring_block_send_eth(0x8600000|global_coarse_sync_num);
        packet_counter_SFO_adjustment = 0;
        // packet_SFO_adjustment_direction = 0;
        sync_finished = 1;
    }
    // pending_sync_symbol_timing

     // if(pending_sync_symbol_timing == 1) {
    //     pending_sync_symbol_timing = 0;
    //     xbb_coarse_sync(&CFO_results, 15);

    //     // CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
    //     // while(occupancy > 1) {
    //     //     CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
    //     // }

    //     // CSR_WRITE(DMA_0_INTERRUPT_CLEAR, 0);
    //     // while(1) {
    //     //     CSR_READ(mip, helper);
    //     //       if(helper & DMA_0_ENABLE_BIT) {
    //     //         CSR_WRITE(DMA_0_INTERRUPT_CLEAR, 0);
    //     //         break;
    //     //         }
    //     //    }
    // }
}

#include "calculate_power.h"



unsigned int _get_sat_ratio(){
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

// unsigned int _estimate_pwr(unsigned int block_size){
//     unsigned int dma_start_addr = VMEM_ADDRESS(power_data_copy)*16;
//     unsigned int output_pwr_addr = VMEM_ADDRESS(output_3)*16;
//     // unsigned int max_block_size = 1024;
//     unsigned int signal_power;
//     unsigned int real_pwr;
//     unsigned int imag_pwr;

//     SET_REG(x3, 0xf1000000);
//     // Enable input DMA
//     CSR_WRITE(DMA_0_START_ADDR, dma_start_addr);
//     CSR_WRITE(DMA_0_LENGTH, block_size);
//     CSR_WRITE(DMA_0_TIMER_VAL, START_IMMED_TIME);
//     CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);

//     INTERRUPT_WAIT_CLEAR_DMA_0();

//     SET_REG(x3, 0xf2000000);

//     // for(unsigned int i = 0; i < max_block_size - block_size; i++){
//     //     vector_memory[dma_start_addr + block_size + i] = 0;
//     // }
//     vector_memory[output_pwr_addr] = 0;
//     get_pwr();

//     SET_REG(x3, 0xf3000000);

//     // Calculate power from OFDM symbol
//     signal_power = vector_memory[output_pwr_addr];


   

//     real_pwr = signal_power&0xffff;
//     imag_pwr = signal_power>>16;
//     signal_power = real_pwr + imag_pwr;

//     SET_REG(x3, signal_power);

//     SET_REG(x3, 0xf4000000);

//     return signal_power;
// }


// void auto_gain_ctrl(Ringbus *ringbus){
//     unsigned int atten_steps[3] = {8, 4, 2};
//     unsigned int max_clipping_size = 102;
//     // unsigned int min_pwr = 40000;
//     unsigned int min_pwr = 0;
//     // unsigned int max_pwr = 60000;
//     unsigned int max_pwr =20000;
//     unsigned int pwr_est_block_size = 1024;
//     unsigned int start_atten = 16;
//     // unsigned int output_pwr_addr = VMEM_ADDRESS(output_3)*16;
//     unsigned int samples_saturated;
//     unsigned int signal_power = 0;

//     ring_block_send_eth(0x76543210);

//     for(unsigned int i = 0; i < 4; i++){
//         ringbus->addr = 6;
//         ringbus->data = DSA_GAIN_CMD|(start_atten*4<<3);

//         send_cmd(ringbus);
//         // Need to wait for CMD to reach ETH before executing next line
//         for(unsigned int i = 0; i < 1000; i++){
//             asm("nop");
//         }
//         ring_block_send_eth(start_atten*4<<3);
//         // Function calls included for easier readability. Remove if speed
//         // optimization needed
//         samples_saturated = _get_sat_ratio();
//         if(samples_saturated > max_clipping_size){
//             start_atten += atten_steps[i]; 
//             // ring_block_send_eth(start_atten*4<<3);
//         }
//         else{
//         // Function calls included for easier readability. Remove if speed
//         // optimization needed
//             signal_power = _estimate_pwr(pwr_est_block_size);

//             // ring_block_send_eth(signal_power);

//             SET_REG(x4, signal_power);
//             if((signal_power >= min_pwr) && (signal_power <= max_pwr)){
                
//                 break;
//             }
//             else if(signal_power < min_pwr){
//                 start_atten -= atten_steps[i];
//                 // ring_block_send_eth(start_atten*4<<3);
//             }
//             else if(signal_power > max_pwr){
//                 start_atten += atten_steps[i];
//                 // ring_block_send_eth(start_atten*4<<3);
//             }
//         }
//     }
// }



static unsigned int underflow_then;

void reset_underflow_counter() {
    CSR_WRITE(CS_CONTROL, 0x1);
    CSR_WRITE(CS_CONTROL, 0x0);
}

unsigned grab_underflow_counter() {
    unsigned int riscv_status;
    CSR_READ(CS_STATUS, riscv_status);
    return riscv_status;
}


void ringbus_send_counter(unsigned counter) {
    ring_block_send_eth(TX_UNDERFLOW|0x030000|(counter&0xffff));
    ring_block_send_eth(TX_UNDERFLOW|0x040000|((counter>>16)&0xffff));
}



static unsigned pending_underflow_report = 0;


void request_underflow_report_callback(unsigned int data) {
    pending_underflow_report = 1;
}





#define UNDERFLOW_BECAME_OK  (0x000000)
#define UNDERFLOW_BECAME_BAD (0x010000)


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


void run_loop() {
    check_error_counter();
}


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

void debug_power() {
    SET_REG(x3, 0x20000000);
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
        // stall2(300);
        soft_queue_ring_eth(POWER_RESULT_PCCMD | 0x00010000 | ((ringbus_back>>16) & 0xffff));
        // stall2(300);

        soft_queue_ring_eth(POWER_RESULT_PCCMD | 0x00020000 | ((ringbus_back>>32) & 0xffff));
        // stall2(300);
        soft_queue_ring_eth(POWER_RESULT_PCCMD | 0x00030000 | ((ringbus_back>>48) & 0xffff));
        // stall2(300);

        // soft_queue_ring_eth(DEBUG_0_PCCMD | calculate_power_runtime_0() );
        // stall2(300);
        // soft_queue_ring_eth(DEBUG_1_PCCMD | calculate_power_runtime_1() );
        // stall2(300);

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


    // if( power_est_state == 1 && now > 0x8000 ) {
    //     SET_REG(x3, 0x30000000);
    //     power_est_state = 2;

        

    //     // Calculate power from OFDM symbol
    //     unsigned int signal_power = _estimate_pwr(1024);

    //     SET_REG(x3, 0x40000000);
    //     SET_REG(x3, signal_power);

    //     // unsigned int real_pwr;
    //     // unsigned int imag_pwr;
    //     // real_pwr = signal_power&0xffff;
    //     // imag_pwr = signal_power>>16;
    //     // signal_power = real_pwr + imag_pwr;


    //     // SET_REG(x3, real_pwr);
    //     // SET_REG(x3, imag_pwr);


    //     ring_block_send_eth(power_est_state);
    //     // STALL(100);
    //     ring_block_send_eth(signal_power);

    // }
}

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
                ptr = ( _mgr.chunk_dma_address[0] );
                break;
            case 1:
                ptr = VMEM_DMA_ADDRESS( pwr_output_0 );
                break;
            case 2:
                ptr = VMEM_DMA_ADDRESS( pwr_output_1 );
                break;
            case 3:
                ptr = VMEM_DMA_ADDRESS( pwr_output_2 );
                break;
            case 4:
                ptr = VMEM_DMA_ADDRESS( pwr_output_3 );
                break;
            case 5:
                ptr = VMEM_DMA_ADDRESS( variable_cfg_stage_0 );
                break;
            case 6:
                ptr = VMEM_DMA_ADDRESS( power_data_copy );
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
    if( data == 0 ) {
        if( power_est_state == PWR_DISABLE ) {
            power_est_state = PWR_BOOT;
        }
    }
}

#endif

int main(void) {

#ifdef STREAM_DEFAULT_TO_ON
    enable_output_stream = 1;
#else
    enable_output_stream = 0;
#endif
    pending_input_advance = 0;

    int occupancy;

    CSR_READ(TIMER_VALUE, last_error_report);


    init_VMalloc(&mgr);
    init_memory_manager();
    // unsigned int burn = 16;
    // for(unsigned int i = 0) 

    setup_calculate_power();

    CIRBUF_POW2_RUNTIME_INITIALIZE(__soft_ring_queue);
    setup_soft_ring(2, 1);

#ifdef ENABLE_DBGPOWER
    setup_debug();
#endif

    CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);



    ring_register_callback(&sfo_adjustment_callback, SFO_PERIODIC_ADJ_CMD);
    ring_register_callback(&sfo_sign_callback, SFO_PERIODIC_SIGN_CMD);



    ring_register_callback(&stream_callback, STREAM_CMD);
    ring_register_callback(&turnstile_advance_callback, TURNSTILE_CMD);
    ring_register_callback(&sync_callback, SYNCHRONIZATION_CMD);
    ring_register_callback(&corrupt_dma_callback, CORRUPT_DMA_OUT_CMD);
    ring_register_callback(&check_bootload_status, CHECK_BOOTLOAD_CMD);
    ring_register_callback(&request_underflow_report_callback, REQUEST_UNDERFLOW_REPORT_CMD);
    ring_register_callback(&fft_barrel_shift_callback, FFT_BARREL_SHIFT_CMD);
    ring_register_callback(&trigger_power_callback, POWER_ESTIMATION_CMD);


    // this can allow manual setting of barrel stages at boot
    // fft_1024_set_bs(&active_plan, 4, 0x0f);

    //xbb_coarse_sync(15);

    setup_fsm();
    setup_dma_in();
    setup_fft();
    setup_dma_out();

    unsigned int counter = 0;
    SET_REG(x3, 0xdead0000);

    // ring_block_send_eth(0xdead0000);
    // ring_block_send_eth(VMEM_ROW_ADDRESS(dma_buffer_a));
    // ring_block_send_eth(VMEM_ROW_ADDRESS(dma_buffer_b));

    _setup_perf();
    // ring_block_send_eth(dma_in_ptr[0]);
    // ring_block_send_eth(dma_in_ptr[1]);
    // ring_block_send_eth(fft_ptr[0]);
    // ring_block_send_eth(fft_ptr[1]);

    // wait till zhen's finish triggering

    Ringbus ringbus;
    unsigned int helper;


    int power_counter = 0;


  while(1) {
    _perf_top_idle_loop();

    // STALL(10);

    // STALL(100);
    // STALL(100);
    // STALL(100);
    // STALL(100);
    // STALL(100);
    // STALL(100);
    // STALL(100);

    pet_fsm();
    pet_dma_in();
    pet_fft();
    pet_dma_out();
    pet_dma_out();
    pet_dma_out();
    pet_dma_out();
    pet_symbol_sync();
#ifdef CHECK_ERROR_COUNTERS
    check_error_counter();
#endif
    check_ring(&ringbus);

#ifdef ENABLE_DBGPOWER
    if( power_counter >= 4 ) {
        debug_power();
        power_counter = 0;
    } else {
        power_counter++;
    }
    slow_report();
#endif

    pet_soft_ring();
    run_loop();

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












#else

#define INCLUDE_COUNTER_AS vmem_counter
#include "vmem_counter_8k.h"



// int main(void)
// {
//   Ringbus ringbus;
//   const unsigned int total_size = 0x2000;
//   const unsigned int chunks = 8; // update next value when changed
//   const unsigned int chunks_mask = 0x7; // mask for doing modulo when divisor is a power of 2, calculate from previous value
//   const unsigned int chunk =  total_size / chunks;
//   const unsigned int rate = 4000; // 1/4 output rate

//   ring_register_callback(&stream_callback, STREAM_CMD);
//   enable_output_stream = 0;

//   unsigned int enable_output_stream_p;

//   unsigned int start_time;
//   unsigned int chunk_index = 0;
//   unsigned int chunk_offset; // how many words past the start dma_ptr are we sending
//   // unsigned int next_schedule;
//   unsigned int base_addr_dma_ptr = VMEM_DMA_ADDRESS(vmem_counter);
//   unsigned int occupancy;

//   CSR_READ(TIMER_VALUE, start_time);
//   start_time += 1024; // advance this so we don't miss the timer
//   while(1) {

//     CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
    


//     if(enable_output_stream_p == 0 && enable_output_stream == 1) {
//         CSR_READ(TIMER_VALUE, start_time);
//         start_time += 1024; // advance this so we don't miss the timer
//     }


//     if( enable_output_stream == 1 && occupancy < DMA_1_SCHEDULE_DEPTH) {
//       chunk_offset = chunk_index * chunk;
//       dma_block_send_timer(base_addr_dma_ptr + chunk_offset, chunk, start_time);

//       start_time += 1024*rate; // always bump
//       chunk_index = (chunk_index+1) & chunks_mask; // same as % 8
//     }

//     enable_output_stream_p = enable_output_stream;
//     check_ring(&ringbus);
//   }
// }


// int main(void)
// {
//     while(1) {
//         dma_in_set(0, 4096);
//         INTERRUPT_WAIT_CLEAR_DMA_0();
//         dma_out_set(0, 4096);
//         INTERRUPT_WAIT_CLEAR_DMA_1();
//     }
// }


int main(void)
{
    ring_block_send_eth(0xdeaddead);
    // read in full memory
    dma_in_set(0, 1024*64);
    INTERRUPT_WAIT_CLEAR_DMA_0();
    ring_block_send_eth(0xdeadcafe);
    unsigned int i = 0;
    unsigned int j = 0;
    for(i = 0; i < 1024*64; i++) {
        ring_block_send_eth(vector_memory[i]);
        for(j = 0; j < 500; j++) {
            STALL(2);
        }
    }


  Ringbus ringbus;

    while(1) {
        check_ring(&ringbus);
    }

}




#endif




void stream_callback(unsigned int data) {
    // switch(data) {
    //     case 0:
    //     case 1:
    //         enable_output_stream = data;
    //         break;
    //     default:
    //         break;
    // }
}