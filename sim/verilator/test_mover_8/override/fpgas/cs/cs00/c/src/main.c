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



// #define FSM_INIT (0)
#define FSM_FLUSHING (1)
#define FSM_DO_SYNC (4)
#define FSM_PING_PONG (3)

// #define COARSE_SYNC_OFDM_NUM (25600)
// #define COARSE_SYNC_OFDM_NUM (50)

unsigned global_coarse_sync_num = 50;



#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "check_bootload.h"



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
#include "fft_1024_3914.h"


#include "config_word_cmul_rx4_0f.h"
#include "config_word_cmul_rx4_00.h"
#include "config_word_cmul_eq_0f.h"
#include "config_word_conj_eq_0f.h"
#include "config_word_conj_eq_0b.h"
#include "config_word_conj_rx4_0f.h"
#include "config_word_add_eq_00.h"
#include "config_word_add_rx_01.h"
#include "config_word_sub_eq_00.h"
#include "config_word_magsquare_eq_00.h"
#include "config_word_magsquare_rx4_00.h"
#include "config_word_magsquare_rx4_15.h"

#ifdef DEBUG_DUMP_FIRST_FRAME
unsigned int dump_first = 1;
#endif


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


// for power control
VMEM_SECTION unsigned int bank_address_1_pwr[16] = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3};
VMEM_SECTION unsigned int permutation_address_4[16] = {0x0, 0x3000, 0x6000, 0x9000, 0xd000, 0xd000, 0xd000, 0xe000, 0xe000, 0xe000, 0xf000, 0xf000, 0xf000, 0x0, 0x0, 0x0};

VMEM_SECTION unsigned int all_one_pwr[16] = {0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};

VMEM_SECTION unsigned int ofdm_data[1024] = {0};
VMEM_SECTION unsigned int test_data[16] = {0x00010001, 0x00020002, 0x00030003,0x00040004,0x00050005,0x00060006,0x00070007,0x00080008,0x00090009,0x000a000a,0x000b000b,0x000c000c,0x000d000d,0x000e000e};
VMEM_SECTION unsigned int output_0[512] = {0};
VMEM_SECTION unsigned int output_1[128] = {0};
VMEM_SECTION unsigned int output_2[32] = {0};
VMEM_SECTION unsigned int output_3[16] = {0};

VMEM_SECTION unsigned int delete_data[16]={0};

VMEM_SECTION unsigned int trash_data[256]={0};

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
void sfo_sign_callback(unsigned int data) {
    
    

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

void setup_dma_in(void) {
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

fft1024_t active_plan;

void setup_fft(void) {
  fft_ptr[0] = (unsigned int) vmalloc_single(&mgr);
  fft_ptr[1] = (unsigned int) vmalloc_single(&mgr);
  mark_both_fft_empty();

  active_plan = get_fft1024_plan(0, 0);
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
            ring_block_send_eth(pending_input_advance);
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
    SET_REG(x4, 0xfeedfeed);
    SET_REG(x4, cpu_ptr_from_dma[0]);
    SET_REG(x4, cpu_ptr_from_dma[1]);
    SET_REG(x4, cpu_ptr_from_dma[2]);
    SET_REG(x4, cpu_ptr_from_dma[3]);


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
        CSR_WRITE(RINGBUS_WRITE_ADDR, RING_ADDR_CS21);
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

  ring_block_send_eth(0x66666666);
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

void get_pwr(){
    MVXV_KNOP(V13, VMEM_ADDRESS(test_data));
    MVXV_KNOP(V14, VMEM_ADDRESS(test_data));
    MVXV_KNOP(V15, VMEM_ADDRESS(test_data));
    int ofdm_data_loc = VMEM_ADDRESS(ofdm_data);
    // int benchmark_location = VMEM_ADDRESS(benchmark);
    int bank_addr_loc = VMEM_ADDRESS(bank_address_1_pwr);
    int permutation_addr_loc = VMEM_ADDRESS(permutation_address_4);
    int all_one_loc = VMEM_ADDRESS(all_one_pwr);
    int output_loc_0 = VMEM_ADDRESS(output_0);
    int output_loc_1 = VMEM_ADDRESS(output_1);
    int output_loc_2 = VMEM_ADDRESS(output_2);
    int output_loc_3 = VMEM_ADDRESS(output_3);
    int cfg_loc;
    unsigned int start_clk;
    unsigned int end_clk;

    CSR_READ(TIMER_VALUE, start_clk);
    cfg_loc = VMEM_ADDRESS(config_word_magsquare_rx4_15);
    // cfg_loc = VMEM_ADDRESS(config_word_magsquare_rx4_00);
    MVXV_KNOP(V0, cfg_loc);
    VNOP_LK14(V0);
    // FLush out these inputs
    VNOP_LK8(V15);
    VNOP_LK9(V14);
    VNOP_SK1(V13);

    /// Stage 0
    MVXV_KNOP(V1, ofdm_data_loc);
    MVXV_KNOP(V2, ofdm_data_loc + 1);
    MVXV_KNOP(V3, output_loc_0);
    MVXV_KNOP(V4, 1);
    MVXV_KNOP(V5, 1-(1<<12));
    MVXV_KNOP(V6, 2);

    int latency_0 = 4;

    for(unsigned int index = 0; index < latency_0; index++){
        ADD_LK8(V1, V1, V6, 0);
        ADD_LK9(V2, V2, V6, 0);
    }

    for(unsigned int index = 0; index < 32 - latency_0; index++){
        ADD_LK8(V1, V1, V6, 0);
        ADD_LK9(V2, V2, V6, 0);
        ADD_SK1(V3, V3, V5, 0);    
    }

    for(unsigned int index = 0; index < latency_0; index++){
        ADD_SK1(V3, V3, V5, 0);
    }

    cfg_loc = VMEM_ADDRESS(config_word_add_rx_01);
    // cfg_loc = VMEM_ADDRESS(config_word_add_rx_00);
    MVXV_KNOP(V0, cfg_loc);
    VNOP_LK14(V0);
    // Flush out these inputs
    VNOP_LK8(V15);
    VNOP_LK9(V14);
    VNOP_SK1(V13);

    /// Stage 1
    MVXV_KNOP(V1, output_loc_0);
    MVXV_KNOP(V2, output_loc_1);
    MVXV_KNOP(V3, output_loc_0 + 4);
    MVXV_KNOP(V8, 8);
    MVXV_KNOP(V5, 1-(1<<12));
    
    MVXV_KNOP(V7, bank_addr_loc);
    VNOP_LK15(V7);
    MVK15V_KNOP(V7, 0);
    ADD_KNOP(V1, V1, V7, 0);
    ADD_KNOP(V3, V3, V7, 0);

    int latency_1 = 2;

    for(unsigned int index = 0; index < latency_1; index++){
        ADD_LK8(V1, V1, V8, 0);
        ADD_LK9(V3, V3, V8, 0);
    }

    for(unsigned int index = 0; index < 4 - latency_1; index++){
        ADD_LK8(V1, V1, V8, 0);
        ADD_LK9(V3, V3, V8, 0);
        ADD_SK1(V2, V2, V5, 0);
    }

    for(unsigned int index = 0; index < latency_1; index++){
        ADD_SK1(V2, V2, V5, 0);
    }

    cfg_loc = VMEM_ADDRESS(config_word_cmul_rx4_00);
    MVXV_KNOP(V0, cfg_loc);
    VNOP_LK14(V0);
    // Flush out these inputs
    VNOP_LK8(V15);
    VNOP_LK9(V14);
    VNOP_SK1(V13);

    /// Stage 2
    MVXV_KNOP(V1, all_one_loc);
    MVXV_KNOP(V2, output_loc_1);
    MVXV_KNOP(V3, output_loc_2);

    ADD_KNOP(V2, V2, V7, 0);

    VNOP_LK8(V1);
    VNOP_LK9(V2);
    VNOP_SK1(V3);

    /// Stage 3
    MVXV_KNOP(V2, output_loc_2);
    MVXV_KNOP(V3, output_loc_3);

    MVXV_KNOP(V12, permutation_addr_loc);
    VNOP_LK15(V12);
    MVK15V_KNOP(V12,0);
    ADD_KNOP(V2, V12, V2, 0);

    VNOP_LK8(V1);
    VNOP_LK9(V2);
    VNOP_SK1(V3);

    CSR_READ(TIMER_VALUE, end_clk);

    STALL(10);
    STALL(10);
    STALL(10);
    STALL(10);
    STALL(10);
}

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

unsigned int _estimate_pwr(unsigned int block_size){
    unsigned int dma_start_addr = VMEM_ADDRESS(ofdm_data)*16;
    unsigned int output_pwr_addr = VMEM_ADDRESS(output_3)*16;
    unsigned int max_block_size = 1024;
    unsigned int signal_power;
    unsigned int real_pwr;
    unsigned int imag_pwr;

    // Enable input DMA
    CSR_WRITE(DMA_0_START_ADDR, dma_start_addr);
    CSR_WRITE(DMA_0_LENGTH, block_size);
    CSR_WRITE(DMA_0_TIMER_VAL, START_IMMED_TIME);
    CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);

    INTERRUPT_WAIT_CLEAR_DMA_0();

    // for(unsigned int i = 0; i < max_block_size - block_size; i++){
    //     vector_memory[dma_start_addr + block_size + i] = 0;
    // }
    vector_memory[output_pwr_addr] = 0;
    get_pwr();

    // Calculate power from OFDM symbol
    signal_power = vector_memory[output_pwr_addr];

   

    real_pwr = signal_power&0xffff;
    imag_pwr = signal_power>>16;
    signal_power = real_pwr + imag_pwr;

    return signal_power;
}


void auto_gain_ctrl(Ringbus *ringbus){
    unsigned int atten_steps[3] = {8, 4, 2};
    unsigned int max_clipping_size = 102;
    // unsigned int min_pwr = 40000;
    unsigned int min_pwr = 0;
    // unsigned int max_pwr = 60000;
    unsigned int max_pwr =20000;
    unsigned int pwr_est_block_size = 1024;
    unsigned int start_atten = 16;
    unsigned int output_pwr_addr = VMEM_ADDRESS(output_3)*16;
    unsigned int samples_saturated;
    unsigned int signal_power = 0;

    ring_block_send_eth(0x76543210);

    for(unsigned int i = 0; i < 4; i++){
        ringbus->addr = 6;
        ringbus->data = DSA_GAIN_CMD|(start_atten*4<<3);

        send_cmd(ringbus);
        // Need to wait for CMD to reach ETH before executing next line
        for(unsigned int i = 0; i < 1000; i++){
            asm("nop");
        }
        ring_block_send_eth(start_atten*4<<3);
        // Function calls included for easier readability. Remove if speed
        // optimization needed
        samples_saturated = _get_sat_ratio();
        if(samples_saturated > max_clipping_size){
            start_atten += atten_steps[i]; 
            // ring_block_send_eth(start_atten*4<<3);
        }
        else{
        // Function calls included for easier readability. Remove if speed
        // optimization needed
            signal_power = _estimate_pwr(pwr_est_block_size);

            // ring_block_send_eth(signal_power);

            SET_HALF_REG_VAR(x4, signal_power);
            if((signal_power >= min_pwr) && (signal_power <= max_pwr)){
                
                break;
            }
            else if(signal_power < min_pwr){
                start_atten -= atten_steps[i];
                // ring_block_send_eth(start_atten*4<<3);
            }
            else if(signal_power > max_pwr){
                start_atten += atten_steps[i];
                // ring_block_send_eth(start_atten*4<<3);
            }
        }
    }
}


void check_error_counter(void) {
    // CSR_WRITE(CS_CONTROL, err_state);
    unsigned int riscv_status;
    CSR_READ(CS_STATUS, riscv_status);
    unsigned int now;
    CSR_READ(TIMER_VALUE, now);

    // if( riscv_status > 0 ) {
    //     // set LED
    //     // CSR_SET_BITS(GPIO_WRITE, LED_GPIO_BIT);
    //     CSR_WRITE(CS_CONTROL, 0x1); // Send counter reset
    //     CSR_WRITE(CS_CONTROL, 0x0);
    //     // 
    //     ring_block_send_eth_debug(riscv_status);
    // } else if(now > then) {
    //     ring_block_send_eth_debug(riscv_status);
    //     then = now + 125000000;
    // }  

    if(now < then){ // now wrapped
        then = 0;
    }

    if(now-then > (125000000)) {
        // ring_block_send_eth_debug(riscv_status);
        // ring_block_send_eth(riscv_status);
        ring_block_send_eth(RX_OVERFLOW|(riscv_status & 0xffffff));
        CSR_WRITE(CS_CONTROL, 0x1);
        CSR_WRITE(CS_CONTROL, 0x0);
        
        then = now;
    }
}


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
  // unsigned int burn = 16;
  // for(unsigned int i = 0)

  CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);


    ring_register_callback(&sfo_adjustment_callback, SFO_PERIODIC_ADJ_CMD);
    ring_register_callback(&sfo_sign_callback, SFO_PERIODIC_SIGN_CMD);



  ring_register_callback(&stream_callback, STREAM_CMD);
  ring_register_callback(&turnstile_advance_callback, TURNSTILE_CMD);
  ring_register_callback(&sync_callback, SYNCHRONIZATION_CMD);
  ring_register_callback(&corrupt_dma_callback, CORRUPT_DMA_OUT_CMD);
  ring_register_callback(&check_bootload_status, CHECK_BOOTLOAD_CMD);

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

// VMEM_SECTION unsigned int vmem_counter[] = {
// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527, 528, 529, 530, 531, 532, 533, 534, 535, 536, 537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 595, 596, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611, 612, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 625, 626, 627, 628, 629, 630, 631, 632, 633, 634, 635, 636, 637, 638, 639, 640, 641, 642, 643, 644, 645, 646, 647, 648, 649, 650, 651, 652, 653, 654, 655, 656, 657, 658, 659, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691, 692, 693, 694, 695, 696, 697, 698, 699, 700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720, 721, 722, 723, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735, 736, 737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 751, 752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 762, 763, 764, 765, 766, 767, 768, 769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 798, 799, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814, 815, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 831, 832, 833, 834, 835, 836, 837, 838, 839, 840, 841, 842, 843, 844, 845, 846, 847, 848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860, 861, 862, 863, 864, 865, 866, 867, 868, 869, 870, 871, 872, 873, 874, 875, 876, 877, 878, 879, 880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895, 896, 897, 898, 899, 900, 901, 902, 903, 904, 905, 906, 907, 908, 909, 910, 911, 912, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927, 928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 938, 939, 940, 941, 942, 943, 944, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 958, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 987, 988, 989, 990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, 1039, 1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055, 1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1067, 1068, 1069, 1070, 1071, 1072, 1073, 1074, 1075, 1076, 1077, 1078, 1079, 1080, 1081, 1082, 1083, 1084, 1085, 1086, 1087, 1088, 1089, 1090, 1091, 1092, 1093, 1094, 1095, 1096, 1097, 1098, 1099, 1100, 1101, 1102, 1103, 1104, 1105, 1106, 1107, 1108, 1109, 1110, 1111, 1112, 1113, 1114, 1115, 1116, 1117, 1118, 1119, 1120, 1121, 1122, 1123, 1124, 1125, 1126, 1127, 1128, 1129, 1130, 1131, 1132, 1133, 1134, 1135, 1136, 1137, 1138, 1139, 1140, 1141, 1142, 1143, 1144, 1145, 1146, 1147, 1148, 1149, 1150, 1151, 1152, 1153, 1154, 1155, 1156, 1157, 1158, 1159, 1160, 1161, 1162, 1163, 1164, 1165, 1166, 1167, 1168, 1169, 1170, 1171, 1172, 1173, 1174, 1175, 1176, 1177, 1178, 1179, 1180, 1181, 1182, 1183, 1184, 1185, 1186, 1187, 1188, 1189, 1190, 1191, 1192, 1193, 1194, 1195, 1196, 1197, 1198, 1199, 1200, 1201, 1202, 1203, 1204, 1205, 1206, 1207, 1208, 1209, 1210, 1211, 1212, 1213, 1214, 1215, 1216, 1217, 1218, 1219, 1220, 1221, 1222, 1223, 1224, 1225, 1226, 1227, 1228, 1229, 1230, 1231, 1232, 1233, 1234, 1235, 1236, 1237, 1238, 1239, 1240, 1241, 1242, 1243, 1244, 1245, 1246, 1247, 1248, 1249, 1250, 1251, 1252, 1253, 1254, 1255, 1256, 1257, 1258, 1259, 1260, 1261, 1262, 1263, 1264, 1265, 1266, 1267, 1268, 1269, 1270, 1271, 1272, 1273, 1274, 1275, 1276, 1277, 1278, 1279, 1280, 1281, 1282, 1283, 1284, 1285, 1286, 1287, 1288, 1289, 1290, 1291, 1292, 1293, 1294, 1295, 1296, 1297, 1298, 1299, 1300, 1301, 1302, 1303, 1304, 1305, 1306, 1307, 1308, 1309, 1310, 1311, 1312, 1313, 1314, 1315, 1316, 1317, 1318, 1319, 1320, 1321, 1322, 1323, 1324, 1325, 1326, 1327, 1328, 1329, 1330, 1331, 1332, 1333, 1334, 1335, 1336, 1337, 1338, 1339, 1340, 1341, 1342, 1343, 1344, 1345, 1346, 1347, 1348, 1349, 1350, 1351, 1352, 1353, 1354, 1355, 1356, 1357, 1358, 1359, 1360, 1361, 1362, 1363, 1364, 1365, 1366, 1367, 1368, 1369, 1370, 1371, 1372, 1373, 1374, 1375, 1376, 1377, 1378, 1379, 1380, 1381, 1382, 1383, 1384, 1385, 1386, 1387, 1388, 1389, 1390, 1391, 1392, 1393, 1394, 1395, 1396, 1397, 1398, 1399, 1400, 1401, 1402, 1403, 1404, 1405, 1406, 1407, 1408, 1409, 1410, 1411, 1412, 1413, 1414, 1415, 1416, 1417, 1418, 1419, 1420, 1421, 1422, 1423, 1424, 1425, 1426, 1427, 1428, 1429, 1430, 1431, 1432, 1433, 1434, 1435, 1436, 1437, 1438, 1439, 1440, 1441, 1442, 1443, 1444, 1445, 1446, 1447, 1448, 1449, 1450, 1451, 1452, 1453, 1454, 1455, 1456, 1457, 1458, 1459, 1460, 1461, 1462, 1463, 1464, 1465, 1466, 1467, 1468, 1469, 1470, 1471, 1472, 1473, 1474, 1475, 1476, 1477, 1478, 1479, 1480, 1481, 1482, 1483, 1484, 1485, 1486, 1487, 1488, 1489, 1490, 1491, 1492, 1493, 1494, 1495, 1496, 1497, 1498, 1499, 1500, 1501, 1502, 1503, 1504, 1505, 1506, 1507, 1508, 1509, 1510, 1511, 1512, 1513, 1514, 1515, 1516, 1517, 1518, 1519, 1520, 1521, 1522, 1523, 1524, 1525, 1526, 1527, 1528, 1529, 1530, 1531, 1532, 1533, 1534, 1535, 1536, 1537, 1538, 1539, 1540, 1541, 1542, 1543, 1544, 1545, 1546, 1547, 1548, 1549, 1550, 1551, 1552, 1553, 1554, 1555, 1556, 1557, 1558, 1559, 1560, 1561, 1562, 1563, 1564, 1565, 1566, 1567, 1568, 1569, 1570, 1571, 1572, 1573, 1574, 1575, 1576, 1577, 1578, 1579, 1580, 1581, 1582, 1583, 1584, 1585, 1586, 1587, 1588, 1589, 1590, 1591, 1592, 1593, 1594, 1595, 1596, 1597, 1598, 1599, 1600, 1601, 1602, 1603, 1604, 1605, 1606, 1607, 1608, 1609, 1610, 1611, 1612, 1613, 1614, 1615, 1616, 1617, 1618, 1619, 1620, 1621, 1622, 1623, 1624, 1625, 1626, 1627, 1628, 1629, 1630, 1631, 1632, 1633, 1634, 1635, 1636, 1637, 1638, 1639, 1640, 1641, 1642, 1643, 1644, 1645, 1646, 1647, 1648, 1649, 1650, 1651, 1652, 1653, 1654, 1655, 1656, 1657, 1658, 1659, 1660, 1661, 1662, 1663, 1664, 1665, 1666, 1667, 1668, 1669, 1670, 1671, 1672, 1673, 1674, 1675, 1676, 1677, 1678, 1679, 1680, 1681, 1682, 1683, 1684, 1685, 1686, 1687, 1688, 1689, 1690, 1691, 1692, 1693, 1694, 1695, 1696, 1697, 1698, 1699, 1700, 1701, 1702, 1703, 1704, 1705, 1706, 1707, 1708, 1709, 1710, 1711, 1712, 1713, 1714, 1715, 1716, 1717, 1718, 1719, 1720, 1721, 1722, 1723, 1724, 1725, 1726, 1727, 1728, 1729, 1730, 1731, 1732, 1733, 1734, 1735, 1736, 1737, 1738, 1739, 1740, 1741, 1742, 1743, 1744, 1745, 1746, 1747, 1748, 1749, 1750, 1751, 1752, 1753, 1754, 1755, 1756, 1757, 1758, 1759, 1760, 1761, 1762, 1763, 1764, 1765, 1766, 1767, 1768, 1769, 1770, 1771, 1772, 1773, 1774, 1775, 1776, 1777, 1778, 1779, 1780, 1781, 1782, 1783, 1784, 1785, 1786, 1787, 1788, 1789, 1790, 1791, 1792, 1793, 1794, 1795, 1796, 1797, 1798, 1799, 1800, 1801, 1802, 1803, 1804, 1805, 1806, 1807, 1808, 1809, 1810, 1811, 1812, 1813, 1814, 1815, 1816, 1817, 1818, 1819, 1820, 1821, 1822, 1823, 1824, 1825, 1826, 1827, 1828, 1829, 1830, 1831, 1832, 1833, 1834, 1835, 1836, 1837, 1838, 1839, 1840, 1841, 1842, 1843, 1844, 1845, 1846, 1847, 1848, 1849, 1850, 1851, 1852, 1853, 1854, 1855, 1856, 1857, 1858, 1859, 1860, 1861, 1862, 1863, 1864, 1865, 1866, 1867, 1868, 1869, 1870, 1871, 1872, 1873, 1874, 1875, 1876, 1877, 1878, 1879, 1880, 1881, 1882, 1883, 1884, 1885, 1886, 1887, 1888, 1889, 1890, 1891, 1892, 1893, 1894, 1895, 1896, 1897, 1898, 1899, 1900, 1901, 1902, 1903, 1904, 1905, 1906, 1907, 1908, 1909, 1910, 1911, 1912, 1913, 1914, 1915, 1916, 1917, 1918, 1919, 1920, 1921, 1922, 1923, 1924, 1925, 1926, 1927, 1928, 1929, 1930, 1931, 1932, 1933, 1934, 1935, 1936, 1937, 1938, 1939, 1940, 1941, 1942, 1943, 1944, 1945, 1946, 1947, 1948, 1949, 1950, 1951, 1952, 1953, 1954, 1955, 1956, 1957, 1958, 1959, 1960, 1961, 1962, 1963, 1964, 1965, 1966, 1967, 1968, 1969, 1970, 1971, 1972, 1973, 1974, 1975, 1976, 1977, 1978, 1979, 1980, 1981, 1982, 1983, 1984, 1985, 1986, 1987, 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026, 2027, 2028, 2029, 2030, 2031, 2032, 2033, 2034, 2035, 2036, 2037, 2038, 2039, 2040, 2041, 2042, 2043, 2044, 2045, 2046, 2047, 2048, 2049, 2050, 2051, 2052, 2053, 2054, 2055, 2056, 2057, 2058, 2059, 2060, 2061, 2062, 2063, 2064, 2065, 2066, 2067, 2068, 2069, 2070, 2071, 2072, 2073, 2074, 2075, 2076, 2077, 2078, 2079, 2080, 2081, 2082, 2083, 2084, 2085, 2086, 2087, 2088, 2089, 2090, 2091, 2092, 2093, 2094, 2095, 2096, 2097, 2098, 2099, 2100, 2101, 2102, 2103, 2104, 2105, 2106, 2107, 2108, 2109, 2110, 2111, 2112, 2113, 2114, 2115, 2116, 2117, 2118, 2119, 2120, 2121, 2122, 2123, 2124, 2125, 2126, 2127, 2128, 2129, 2130, 2131, 2132, 2133, 2134, 2135, 2136, 2137, 2138, 2139, 2140, 2141, 2142, 2143, 2144, 2145, 2146, 2147, 2148, 2149, 2150, 2151, 2152, 2153, 2154, 2155, 2156, 2157, 2158, 2159, 2160, 2161, 2162, 2163, 2164, 2165, 2166, 2167, 2168, 2169, 2170, 2171, 2172, 2173, 2174, 2175, 2176, 2177, 2178, 2179, 2180, 2181, 2182, 2183, 2184, 2185, 2186, 2187, 2188, 2189, 2190, 2191, 2192, 2193, 2194, 2195, 2196, 2197, 2198, 2199, 2200, 2201, 2202, 2203, 2204, 2205, 2206, 2207, 2208, 2209, 2210, 2211, 2212, 2213, 2214, 2215, 2216, 2217, 2218, 2219, 2220, 2221, 2222, 2223, 2224, 2225, 2226, 2227, 2228, 2229, 2230, 2231, 2232, 2233, 2234, 2235, 2236, 2237, 2238, 2239, 2240, 2241, 2242, 2243, 2244, 2245, 2246, 2247, 2248, 2249, 2250, 2251, 2252, 2253, 2254, 2255, 2256, 2257, 2258, 2259, 2260, 2261, 2262, 2263, 2264, 2265, 2266, 2267, 2268, 2269, 2270, 2271, 2272, 2273, 2274, 2275, 2276, 2277, 2278, 2279, 2280, 2281, 2282, 2283, 2284, 2285, 2286, 2287, 2288, 2289, 2290, 2291, 2292, 2293, 2294, 2295, 2296, 2297, 2298, 2299, 2300, 2301, 2302, 2303, 2304, 2305, 2306, 2307, 2308, 2309, 2310, 2311, 2312, 2313, 2314, 2315, 2316, 2317, 2318, 2319, 2320, 2321, 2322, 2323, 2324, 2325, 2326, 2327, 2328, 2329, 2330, 2331, 2332, 2333, 2334, 2335, 2336, 2337, 2338, 2339, 2340, 2341, 2342, 2343, 2344, 2345, 2346, 2347, 2348, 2349, 2350, 2351, 2352, 2353, 2354, 2355, 2356, 2357, 2358, 2359, 2360, 2361, 2362, 2363, 2364, 2365, 2366, 2367, 2368, 2369, 2370, 2371, 2372, 2373, 2374, 2375, 2376, 2377, 2378, 2379, 2380, 2381, 2382, 2383, 2384, 2385, 2386, 2387, 2388, 2389, 2390, 2391, 2392, 2393, 2394, 2395, 2396, 2397, 2398, 2399, 2400, 2401, 2402, 2403, 2404, 2405, 2406, 2407, 2408, 2409, 2410, 2411, 2412, 2413, 2414, 2415, 2416, 2417, 2418, 2419, 2420, 2421, 2422, 2423, 2424, 2425, 2426, 2427, 2428, 2429, 2430, 2431, 2432, 2433, 2434, 2435, 2436, 2437, 2438, 2439, 2440, 2441, 2442, 2443, 2444, 2445, 2446, 2447, 2448, 2449, 2450, 2451, 2452, 2453, 2454, 2455, 2456, 2457, 2458, 2459, 2460, 2461, 2462, 2463, 2464, 2465, 2466, 2467, 2468, 2469, 2470, 2471, 2472, 2473, 2474, 2475, 2476, 2477, 2478, 2479, 2480, 2481, 2482, 2483, 2484, 2485, 2486, 2487, 2488, 2489, 2490, 2491, 2492, 2493, 2494, 2495, 2496, 2497, 2498, 2499, 2500, 2501, 2502, 2503, 2504, 2505, 2506, 2507, 2508, 2509, 2510, 2511, 2512, 2513, 2514, 2515, 2516, 2517, 2518, 2519, 2520, 2521, 2522, 2523, 2524, 2525, 2526, 2527, 2528, 2529, 2530, 2531, 2532, 2533, 2534, 2535, 2536, 2537, 2538, 2539, 2540, 2541, 2542, 2543, 2544, 2545, 2546, 2547, 2548, 2549, 2550, 2551, 2552, 2553, 2554, 2555, 2556, 2557, 2558, 2559, 2560, 2561, 2562, 2563, 2564, 2565, 2566, 2567, 2568, 2569, 2570, 2571, 2572, 2573, 2574, 2575, 2576, 2577, 2578, 2579, 2580, 2581, 2582, 2583, 2584, 2585, 2586, 2587, 2588, 2589, 2590, 2591, 2592, 2593, 2594, 2595, 2596, 2597, 2598, 2599, 2600, 2601, 2602, 2603, 2604, 2605, 2606, 2607, 2608, 2609, 2610, 2611, 2612, 2613, 2614, 2615, 2616, 2617, 2618, 2619, 2620, 2621, 2622, 2623, 2624, 2625, 2626, 2627, 2628, 2629, 2630, 2631, 2632, 2633, 2634, 2635, 2636, 2637, 2638, 2639, 2640, 2641, 2642, 2643, 2644, 2645, 2646, 2647, 2648, 2649, 2650, 2651, 2652, 2653, 2654, 2655, 2656, 2657, 2658, 2659, 2660, 2661, 2662, 2663, 2664, 2665, 2666, 2667, 2668, 2669, 2670, 2671, 2672, 2673, 2674, 2675, 2676, 2677, 2678, 2679, 2680, 2681, 2682, 2683, 2684, 2685, 2686, 2687, 2688, 2689, 2690, 2691, 2692, 2693, 2694, 2695, 2696, 2697, 2698, 2699, 2700, 2701, 2702, 2703, 2704, 2705, 2706, 2707, 2708, 2709, 2710, 2711, 2712, 2713, 2714, 2715, 2716, 2717, 2718, 2719, 2720, 2721, 2722, 2723, 2724, 2725, 2726, 2727, 2728, 2729, 2730, 2731, 2732, 2733, 2734, 2735, 2736, 2737, 2738, 2739, 2740, 2741, 2742, 2743, 2744, 2745, 2746, 2747, 2748, 2749, 2750, 2751, 2752, 2753, 2754, 2755, 2756, 2757, 2758, 2759, 2760, 2761, 2762, 2763, 2764, 2765, 2766, 2767, 2768, 2769, 2770, 2771, 2772, 2773, 2774, 2775, 2776, 2777, 2778, 2779, 2780, 2781, 2782, 2783, 2784, 2785, 2786, 2787, 2788, 2789, 2790, 2791, 2792, 2793, 2794, 2795, 2796, 2797, 2798, 2799, 2800, 2801, 2802, 2803, 2804, 2805, 2806, 2807, 2808, 2809, 2810, 2811, 2812, 2813, 2814, 2815, 2816, 2817, 2818, 2819, 2820, 2821, 2822, 2823, 2824, 2825, 2826, 2827, 2828, 2829, 2830, 2831, 2832, 2833, 2834, 2835, 2836, 2837, 2838, 2839, 2840, 2841, 2842, 2843, 2844, 2845, 2846, 2847, 2848, 2849, 2850, 2851, 2852, 2853, 2854, 2855, 2856, 2857, 2858, 2859, 2860, 2861, 2862, 2863, 2864, 2865, 2866, 2867, 2868, 2869, 2870, 2871, 2872, 2873, 2874, 2875, 2876, 2877, 2878, 2879, 2880, 2881, 2882, 2883, 2884, 2885, 2886, 2887, 2888, 2889, 2890, 2891, 2892, 2893, 2894, 2895, 2896, 2897, 2898, 2899, 2900, 2901, 2902, 2903, 2904, 2905, 2906, 2907, 2908, 2909, 2910, 2911, 2912, 2913, 2914, 2915, 2916, 2917, 2918, 2919, 2920, 2921, 2922, 2923, 2924, 2925, 2926, 2927, 2928, 2929, 2930, 2931, 2932, 2933, 2934, 2935, 2936, 2937, 2938, 2939, 2940, 2941, 2942, 2943, 2944, 2945, 2946, 2947, 2948, 2949, 2950, 2951, 2952, 2953, 2954, 2955, 2956, 2957, 2958, 2959, 2960, 2961, 2962, 2963, 2964, 2965, 2966, 2967, 2968, 2969, 2970, 2971, 2972, 2973, 2974, 2975, 2976, 2977, 2978, 2979, 2980, 2981, 2982, 2983, 2984, 2985, 2986, 2987, 2988, 2989, 2990, 2991, 2992, 2993, 2994, 2995, 2996, 2997, 2998, 2999, 3000, 3001, 3002, 3003, 3004, 3005, 3006, 3007, 3008, 3009, 3010, 3011, 3012, 3013, 3014, 3015, 3016, 3017, 3018, 3019, 3020, 3021, 3022, 3023, 3024, 3025, 3026, 3027, 3028, 3029, 3030, 3031, 3032, 3033, 3034, 3035, 3036, 3037, 3038, 3039, 3040, 3041, 3042, 3043, 3044, 3045, 3046, 3047, 3048, 3049, 3050, 3051, 3052, 3053, 3054, 3055, 3056, 3057, 3058, 3059, 3060, 3061, 3062, 3063, 3064, 3065, 3066, 3067, 3068, 3069, 3070, 3071, 3072, 3073, 3074, 3075, 3076, 3077, 3078, 3079, 3080, 3081, 3082, 3083, 3084, 3085, 3086, 3087, 3088, 3089, 3090, 3091, 3092, 3093, 3094, 3095, 3096, 3097, 3098, 3099, 3100, 3101, 3102, 3103, 3104, 3105, 3106, 3107, 3108, 3109, 3110, 3111, 3112, 3113, 3114, 3115, 3116, 3117, 3118, 3119, 3120, 3121, 3122, 3123, 3124, 3125, 3126, 3127, 3128, 3129, 3130, 3131, 3132, 3133, 3134, 3135, 3136, 3137, 3138, 3139, 3140, 3141, 3142, 3143, 3144, 3145, 3146, 3147, 3148, 3149, 3150, 3151, 3152, 3153, 3154, 3155, 3156, 3157, 3158, 3159, 3160, 3161, 3162, 3163, 3164, 3165, 3166, 3167, 3168, 3169, 3170, 3171, 3172, 3173, 3174, 3175, 3176, 3177, 3178, 3179, 3180, 3181, 3182, 3183, 3184, 3185, 3186, 3187, 3188, 3189, 3190, 3191, 3192, 3193, 3194, 3195, 3196, 3197, 3198, 3199, 3200, 3201, 3202, 3203, 3204, 3205, 3206, 3207, 3208, 3209, 3210, 3211, 3212, 3213, 3214, 3215, 3216, 3217, 3218, 3219, 3220, 3221, 3222, 3223, 3224, 3225, 3226, 3227, 3228, 3229, 3230, 3231, 3232, 3233, 3234, 3235, 3236, 3237, 3238, 3239, 3240, 3241, 3242, 3243, 3244, 3245, 3246, 3247, 3248, 3249, 3250, 3251, 3252, 3253, 3254, 3255, 3256, 3257, 3258, 3259, 3260, 3261, 3262, 3263, 3264, 3265, 3266, 3267, 3268, 3269, 3270, 3271, 3272, 3273, 3274, 3275, 3276, 3277, 3278, 3279, 3280, 3281, 3282, 3283, 3284, 3285, 3286, 3287, 3288, 3289, 3290, 3291, 3292, 3293, 3294, 3295, 3296, 3297, 3298, 3299, 3300, 3301, 3302, 3303, 3304, 3305, 3306, 3307, 3308, 3309, 3310, 3311, 3312, 3313, 3314, 3315, 3316, 3317, 3318, 3319, 3320, 3321, 3322, 3323, 3324, 3325, 3326, 3327, 3328, 3329, 3330, 3331, 3332, 3333, 3334, 3335, 3336, 3337, 3338, 3339, 3340, 3341, 3342, 3343, 3344, 3345, 3346, 3347, 3348, 3349, 3350, 3351, 3352, 3353, 3354, 3355, 3356, 3357, 3358, 3359, 3360, 3361, 3362, 3363, 3364, 3365, 3366, 3367, 3368, 3369, 3370, 3371, 3372, 3373, 3374, 3375, 3376, 3377, 3378, 3379, 3380, 3381, 3382, 3383, 3384, 3385, 3386, 3387, 3388, 3389, 3390, 3391, 3392, 3393, 3394, 3395, 3396, 3397, 3398, 3399, 3400, 3401, 3402, 3403, 3404, 3405, 3406, 3407, 3408, 3409, 3410, 3411, 3412, 3413, 3414, 3415, 3416, 3417, 3418, 3419, 3420, 3421, 3422, 3423, 3424, 3425, 3426, 3427, 3428, 3429, 3430, 3431, 3432, 3433, 3434, 3435, 3436, 3437, 3438, 3439, 3440, 3441, 3442, 3443, 3444, 3445, 3446, 3447, 3448, 3449, 3450, 3451, 3452, 3453, 3454, 3455, 3456, 3457, 3458, 3459, 3460, 3461, 3462, 3463, 3464, 3465, 3466, 3467, 3468, 3469, 3470, 3471, 3472, 3473, 3474, 3475, 3476, 3477, 3478, 3479, 3480, 3481, 3482, 3483, 3484, 3485, 3486, 3487, 3488, 3489, 3490, 3491, 3492, 3493, 3494, 3495, 3496, 3497, 3498, 3499, 3500, 3501, 3502, 3503, 3504, 3505, 3506, 3507, 3508, 3509, 3510, 3511, 3512, 3513, 3514, 3515, 3516, 3517, 3518, 3519, 3520, 3521, 3522, 3523, 3524, 3525, 3526, 3527, 3528, 3529, 3530, 3531, 3532, 3533, 3534, 3535, 3536, 3537, 3538, 3539, 3540, 3541, 3542, 3543, 3544, 3545, 3546, 3547, 3548, 3549, 3550, 3551, 3552, 3553, 3554, 3555, 3556, 3557, 3558, 3559, 3560, 3561, 3562, 3563, 3564, 3565, 3566, 3567, 3568, 3569, 3570, 3571, 3572, 3573, 3574, 3575, 3576, 3577, 3578, 3579, 3580, 3581, 3582, 3583, 3584, 3585, 3586, 3587, 3588, 3589, 3590, 3591, 3592, 3593, 3594, 3595, 3596, 3597, 3598, 3599, 3600, 3601, 3602, 3603, 3604, 3605, 3606, 3607, 3608, 3609, 3610, 3611, 3612, 3613, 3614, 3615, 3616, 3617, 3618, 3619, 3620, 3621, 3622, 3623, 3624, 3625, 3626, 3627, 3628, 3629, 3630, 3631, 3632, 3633, 3634, 3635, 3636, 3637, 3638, 3639, 3640, 3641, 3642, 3643, 3644, 3645, 3646, 3647, 3648, 3649, 3650, 3651, 3652, 3653, 3654, 3655, 3656, 3657, 3658, 3659, 3660, 3661, 3662, 3663, 3664, 3665, 3666, 3667, 3668, 3669, 3670, 3671, 3672, 3673, 3674, 3675, 3676, 3677, 3678, 3679, 3680, 3681, 3682, 3683, 3684, 3685, 3686, 3687, 3688, 3689, 3690, 3691, 3692, 3693, 3694, 3695, 3696, 3697, 3698, 3699, 3700, 3701, 3702, 3703, 3704, 3705, 3706, 3707, 3708, 3709, 3710, 3711, 3712, 3713, 3714, 3715, 3716, 3717, 3718, 3719, 3720, 3721, 3722, 3723, 3724, 3725, 3726, 3727, 3728, 3729, 3730, 3731, 3732, 3733, 3734, 3735, 3736, 3737, 3738, 3739, 3740, 3741, 3742, 3743, 3744, 3745, 3746, 3747, 3748, 3749, 3750, 3751, 3752, 3753, 3754, 3755, 3756, 3757, 3758, 3759, 3760, 3761, 3762, 3763, 3764, 3765, 3766, 3767, 3768, 3769, 3770, 3771, 3772, 3773, 3774, 3775, 3776, 3777, 3778, 3779, 3780, 3781, 3782, 3783, 3784, 3785, 3786, 3787, 3788, 3789, 3790, 3791, 3792, 3793, 3794, 3795, 3796, 3797, 3798, 3799, 3800, 3801, 3802, 3803, 3804, 3805, 3806, 3807, 3808, 3809, 3810, 3811, 3812, 3813, 3814, 3815, 3816, 3817, 3818, 3819, 3820, 3821, 3822, 3823, 3824, 3825, 3826, 3827, 3828, 3829, 3830, 3831, 3832, 3833, 3834, 3835, 3836, 3837, 3838, 3839, 3840, 3841, 3842, 3843, 3844, 3845, 3846, 3847, 3848, 3849, 3850, 3851, 3852, 3853, 3854, 3855, 3856, 3857, 3858, 3859, 3860, 3861, 3862, 3863, 3864, 3865, 3866, 3867, 3868, 3869, 3870, 3871, 3872, 3873, 3874, 3875, 3876, 3877, 3878, 3879, 3880, 3881, 3882, 3883, 3884, 3885, 3886, 3887, 3888, 3889, 3890, 3891, 3892, 3893, 3894, 3895, 3896, 3897, 3898, 3899, 3900, 3901, 3902, 3903, 3904, 3905, 3906, 3907, 3908, 3909, 3910, 3911, 3912, 3913, 3914, 3915, 3916, 3917, 3918, 3919, 3920, 3921, 3922, 3923, 3924, 3925, 3926, 3927, 3928, 3929, 3930, 3931, 3932, 3933, 3934, 3935, 3936, 3937, 3938, 3939, 3940, 3941, 3942, 3943, 3944, 3945, 3946, 3947, 3948, 3949, 3950, 3951, 3952, 3953, 3954, 3955, 3956, 3957, 3958, 3959, 3960, 3961, 3962, 3963, 3964, 3965, 3966, 3967, 3968, 3969, 3970, 3971, 3972, 3973, 3974, 3975, 3976, 3977, 3978, 3979, 3980, 3981, 3982, 3983, 3984, 3985, 3986, 3987, 3988, 3989, 3990, 3991, 3992, 3993, 3994, 3995, 3996, 3997, 3998, 3999, 4000, 4001, 4002, 4003, 4004, 4005, 4006, 4007, 4008, 4009, 4010, 4011, 4012, 4013, 4014, 4015, 4016, 4017, 4018, 4019, 4020, 4021, 4022, 4023, 4024, 4025, 4026, 4027, 4028, 4029, 4030, 4031, 4032, 4033, 4034, 4035, 4036, 4037, 4038, 4039, 4040, 4041, 4042, 4043, 4044, 4045, 4046, 4047, 4048, 4049, 4050, 4051, 4052, 4053, 4054, 4055, 4056, 4057, 4058, 4059, 4060, 4061, 4062, 4063, 4064, 4065, 4066, 4067, 4068, 4069, 4070, 4071, 4072, 4073, 4074, 4075, 4076, 4077, 4078, 4079, 4080, 4081, 4082, 4083, 4084, 4085, 4086, 4087, 4088, 4089, 4090, 4091, 4092, 4093, 4094, 4095, 4096, 4097, 4098, 4099, 4100, 4101, 4102, 4103, 4104, 4105, 4106, 4107, 4108, 4109, 4110, 4111, 4112, 4113, 4114, 4115, 4116, 4117, 4118, 4119, 4120, 4121, 4122, 4123, 4124, 4125, 4126, 4127, 4128, 4129, 4130, 4131, 4132, 4133, 4134, 4135, 4136, 4137, 4138, 4139, 4140, 4141, 4142, 4143, 4144, 4145, 4146, 4147, 4148, 4149, 4150, 4151, 4152, 4153, 4154, 4155, 4156, 4157, 4158, 4159, 4160, 4161, 4162, 4163, 4164, 4165, 4166, 4167, 4168, 4169, 4170, 4171, 4172, 4173, 4174, 4175, 4176, 4177, 4178, 4179, 4180, 4181, 4182, 4183, 4184, 4185, 4186, 4187, 4188, 4189, 4190, 4191, 4192, 4193, 4194, 4195, 4196, 4197, 4198, 4199, 4200, 4201, 4202, 4203, 4204, 4205, 4206, 4207, 4208, 4209, 4210, 4211, 4212, 4213, 4214, 4215, 4216, 4217, 4218, 4219, 4220, 4221, 4222, 4223, 4224, 4225, 4226, 4227, 4228, 4229, 4230, 4231, 4232, 4233, 4234, 4235, 4236, 4237, 4238, 4239, 4240, 4241, 4242, 4243, 4244, 4245, 4246, 4247, 4248, 4249, 4250, 4251, 4252, 4253, 4254, 4255, 4256, 4257, 4258, 4259, 4260, 4261, 4262, 4263, 4264, 4265, 4266, 4267, 4268, 4269, 4270, 4271, 4272, 4273, 4274, 4275, 4276, 4277, 4278, 4279, 4280, 4281, 4282, 4283, 4284, 4285, 4286, 4287, 4288, 4289, 4290, 4291, 4292, 4293, 4294, 4295, 4296, 4297, 4298, 4299, 4300, 4301, 4302, 4303, 4304, 4305, 4306, 4307, 4308, 4309, 4310, 4311, 4312, 4313, 4314, 4315, 4316, 4317, 4318, 4319, 4320, 4321, 4322, 4323, 4324, 4325, 4326, 4327, 4328, 4329, 4330, 4331, 4332, 4333, 4334, 4335, 4336, 4337, 4338, 4339, 4340, 4341, 4342, 4343, 4344, 4345, 4346, 4347, 4348, 4349, 4350, 4351, 4352, 4353, 4354, 4355, 4356, 4357, 4358, 4359, 4360, 4361, 4362, 4363, 4364, 4365, 4366, 4367, 4368, 4369, 4370, 4371, 4372, 4373, 4374, 4375, 4376, 4377, 4378, 4379, 4380, 4381, 4382, 4383, 4384, 4385, 4386, 4387, 4388, 4389, 4390, 4391, 4392, 4393, 4394, 4395, 4396, 4397, 4398, 4399, 4400, 4401, 4402, 4403, 4404, 4405, 4406, 4407, 4408, 4409, 4410, 4411, 4412, 4413, 4414, 4415, 4416, 4417, 4418, 4419, 4420, 4421, 4422, 4423, 4424, 4425, 4426, 4427, 4428, 4429, 4430, 4431, 4432, 4433, 4434, 4435, 4436, 4437, 4438, 4439, 4440, 4441, 4442, 4443, 4444, 4445, 4446, 4447, 4448, 4449, 4450, 4451, 4452, 4453, 4454, 4455, 4456, 4457, 4458, 4459, 4460, 4461, 4462, 4463, 4464, 4465, 4466, 4467, 4468, 4469, 4470, 4471, 4472, 4473, 4474, 4475, 4476, 4477, 4478, 4479, 4480, 4481, 4482, 4483, 4484, 4485, 4486, 4487, 4488, 4489, 4490, 4491, 4492, 4493, 4494, 4495, 4496, 4497, 4498, 4499, 4500, 4501, 4502, 4503, 4504, 4505, 4506, 4507, 4508, 4509, 4510, 4511, 4512, 4513, 4514, 4515, 4516, 4517, 4518, 4519, 4520, 4521, 4522, 4523, 4524, 4525, 4526, 4527, 4528, 4529, 4530, 4531, 4532, 4533, 4534, 4535, 4536, 4537, 4538, 4539, 4540, 4541, 4542, 4543, 4544, 4545, 4546, 4547, 4548, 4549, 4550, 4551, 4552, 4553, 4554, 4555, 4556, 4557, 4558, 4559, 4560, 4561, 4562, 4563, 4564, 4565, 4566, 4567, 4568, 4569, 4570, 4571, 4572, 4573, 4574, 4575, 4576, 4577, 4578, 4579, 4580, 4581, 4582, 4583, 4584, 4585, 4586, 4587, 4588, 4589, 4590, 4591, 4592, 4593, 4594, 4595, 4596, 4597, 4598, 4599, 4600, 4601, 4602, 4603, 4604, 4605, 4606, 4607, 4608, 4609, 4610, 4611, 4612, 4613, 4614, 4615, 4616, 4617, 4618, 4619, 4620, 4621, 4622, 4623, 4624, 4625, 4626, 4627, 4628, 4629, 4630, 4631, 4632, 4633, 4634, 4635, 4636, 4637, 4638, 4639, 4640, 4641, 4642, 4643, 4644, 4645, 4646, 4647, 4648, 4649, 4650, 4651, 4652, 4653, 4654, 4655, 4656, 4657, 4658, 4659, 4660, 4661, 4662, 4663, 4664, 4665, 4666, 4667, 4668, 4669, 4670, 4671, 4672, 4673, 4674, 4675, 4676, 4677, 4678, 4679, 4680, 4681, 4682, 4683, 4684, 4685, 4686, 4687, 4688, 4689, 4690, 4691, 4692, 4693, 4694, 4695, 4696, 4697, 4698, 4699, 4700, 4701, 4702, 4703, 4704, 4705, 4706, 4707, 4708, 4709, 4710, 4711, 4712, 4713, 4714, 4715, 4716, 4717, 4718, 4719, 4720, 4721, 4722, 4723, 4724, 4725, 4726, 4727, 4728, 4729, 4730, 4731, 4732, 4733, 4734, 4735, 4736, 4737, 4738, 4739, 4740, 4741, 4742, 4743, 4744, 4745, 4746, 4747, 4748, 4749, 4750, 4751, 4752, 4753, 4754, 4755, 4756, 4757, 4758, 4759, 4760, 4761, 4762, 4763, 4764, 4765, 4766, 4767, 4768, 4769, 4770, 4771, 4772, 4773, 4774, 4775, 4776, 4777, 4778, 4779, 4780, 4781, 4782, 4783, 4784, 4785, 4786, 4787, 4788, 4789, 4790, 4791, 4792, 4793, 4794, 4795, 4796, 4797, 4798, 4799, 4800, 4801, 4802, 4803, 4804, 4805, 4806, 4807, 4808, 4809, 4810, 4811, 4812, 4813, 4814, 4815, 4816, 4817, 4818, 4819, 4820, 4821, 4822, 4823, 4824, 4825, 4826, 4827, 4828, 4829, 4830, 4831, 4832, 4833, 4834, 4835, 4836, 4837, 4838, 4839, 4840, 4841, 4842, 4843, 4844, 4845, 4846, 4847, 4848, 4849, 4850, 4851, 4852, 4853, 4854, 4855, 4856, 4857, 4858, 4859, 4860, 4861, 4862, 4863, 4864, 4865, 4866, 4867, 4868, 4869, 4870, 4871, 4872, 4873, 4874, 4875, 4876, 4877, 4878, 4879, 4880, 4881, 4882, 4883, 4884, 4885, 4886, 4887, 4888, 4889, 4890, 4891, 4892, 4893, 4894, 4895, 4896, 4897, 4898, 4899, 4900, 4901, 4902, 4903, 4904, 4905, 4906, 4907, 4908, 4909, 4910, 4911, 4912, 4913, 4914, 4915, 4916, 4917, 4918, 4919, 4920, 4921, 4922, 4923, 4924, 4925, 4926, 4927, 4928, 4929, 4930, 4931, 4932, 4933, 4934, 4935, 4936, 4937, 4938, 4939, 4940, 4941, 4942, 4943, 4944, 4945, 4946, 4947, 4948, 4949, 4950, 4951, 4952, 4953, 4954, 4955, 4956, 4957, 4958, 4959, 4960, 4961, 4962, 4963, 4964, 4965, 4966, 4967, 4968, 4969, 4970, 4971, 4972, 4973, 4974, 4975, 4976, 4977, 4978, 4979, 4980, 4981, 4982, 4983, 4984, 4985, 4986, 4987, 4988, 4989, 4990, 4991, 4992, 4993, 4994, 4995, 4996, 4997, 4998, 4999, 5000, 5001, 5002, 5003, 5004, 5005, 5006, 5007, 5008, 5009, 5010, 5011, 5012, 5013, 5014, 5015, 5016, 5017, 5018, 5019, 5020, 5021, 5022, 5023, 5024, 5025, 5026, 5027, 5028, 5029, 5030, 5031, 5032, 5033, 5034, 5035, 5036, 5037, 5038, 5039, 5040, 5041, 5042, 5043, 5044, 5045, 5046, 5047, 5048, 5049, 5050, 5051, 5052, 5053, 5054, 5055, 5056, 5057, 5058, 5059, 5060, 5061, 5062, 5063, 5064, 5065, 5066, 5067, 5068, 5069, 5070, 5071, 5072, 5073, 5074, 5075, 5076, 5077, 5078, 5079, 5080, 5081, 5082, 5083, 5084, 5085, 5086, 5087, 5088, 5089, 5090, 5091, 5092, 5093, 5094, 5095, 5096, 5097, 5098, 5099, 5100, 5101, 5102, 5103, 5104, 5105, 5106, 5107, 5108, 5109, 5110, 5111, 5112, 5113, 5114, 5115, 5116, 5117, 5118, 5119, 5120, 5121, 5122, 5123, 5124, 5125, 5126, 5127, 5128, 5129, 5130, 5131, 5132, 5133, 5134, 5135, 5136, 5137, 5138, 5139, 5140, 5141, 5142, 5143, 5144, 5145, 5146, 5147, 5148, 5149, 5150, 5151, 5152, 5153, 5154, 5155, 5156, 5157, 5158, 5159, 5160, 5161, 5162, 5163, 5164, 5165, 5166, 5167, 5168, 5169, 5170, 5171, 5172, 5173, 5174, 5175, 5176, 5177, 5178, 5179, 5180, 5181, 5182, 5183, 5184, 5185, 5186, 5187, 5188, 5189, 5190, 5191, 5192, 5193, 5194, 5195, 5196, 5197, 5198, 5199, 5200, 5201, 5202, 5203, 5204, 5205, 5206, 5207, 5208, 5209, 5210, 5211, 5212, 5213, 5214, 5215, 5216, 5217, 5218, 5219, 5220, 5221, 5222, 5223, 5224, 5225, 5226, 5227, 5228, 5229, 5230, 5231, 5232, 5233, 5234, 5235, 5236, 5237, 5238, 5239, 5240, 5241, 5242, 5243, 5244, 5245, 5246, 5247, 5248, 5249, 5250, 5251, 5252, 5253, 5254, 5255, 5256, 5257, 5258, 5259, 5260, 5261, 5262, 5263, 5264, 5265, 5266, 5267, 5268, 5269, 5270, 5271, 5272, 5273, 5274, 5275, 5276, 5277, 5278, 5279, 5280, 5281, 5282, 5283, 5284, 5285, 5286, 5287, 5288, 5289, 5290, 5291, 5292, 5293, 5294, 5295, 5296, 5297, 5298, 5299, 5300, 5301, 5302, 5303, 5304, 5305, 5306, 5307, 5308, 5309, 5310, 5311, 5312, 5313, 5314, 5315, 5316, 5317, 5318, 5319, 5320, 5321, 5322, 5323, 5324, 5325, 5326, 5327, 5328, 5329, 5330, 5331, 5332, 5333, 5334, 5335, 5336, 5337, 5338, 5339, 5340, 5341, 5342, 5343, 5344, 5345, 5346, 5347, 5348, 5349, 5350, 5351, 5352, 5353, 5354, 5355, 5356, 5357, 5358, 5359, 5360, 5361, 5362, 5363, 5364, 5365, 5366, 5367, 5368, 5369, 5370, 5371, 5372, 5373, 5374, 5375, 5376, 5377, 5378, 5379, 5380, 5381, 5382, 5383, 5384, 5385, 5386, 5387, 5388, 5389, 5390, 5391, 5392, 5393, 5394, 5395, 5396, 5397, 5398, 5399, 5400, 5401, 5402, 5403, 5404, 5405, 5406, 5407, 5408, 5409, 5410, 5411, 5412, 5413, 5414, 5415, 5416, 5417, 5418, 5419, 5420, 5421, 5422, 5423, 5424, 5425, 5426, 5427, 5428, 5429, 5430, 5431, 5432, 5433, 5434, 5435, 5436, 5437, 5438, 5439, 5440, 5441, 5442, 5443, 5444, 5445, 5446, 5447, 5448, 5449, 5450, 5451, 5452, 5453, 5454, 5455, 5456, 5457, 5458, 5459, 5460, 5461, 5462, 5463, 5464, 5465, 5466, 5467, 5468, 5469, 5470, 5471, 5472, 5473, 5474, 5475, 5476, 5477, 5478, 5479, 5480, 5481, 5482, 5483, 5484, 5485, 5486, 5487, 5488, 5489, 5490, 5491, 5492, 5493, 5494, 5495, 5496, 5497, 5498, 5499, 5500, 5501, 5502, 5503, 5504, 5505, 5506, 5507, 5508, 5509, 5510, 5511, 5512, 5513, 5514, 5515, 5516, 5517, 5518, 5519, 5520, 5521, 5522, 5523, 5524, 5525, 5526, 5527, 5528, 5529, 5530, 5531, 5532, 5533, 5534, 5535, 5536, 5537, 5538, 5539, 5540, 5541, 5542, 5543, 5544, 5545, 5546, 5547, 5548, 5549, 5550, 5551, 5552, 5553, 5554, 5555, 5556, 5557, 5558, 5559, 5560, 5561, 5562, 5563, 5564, 5565, 5566, 5567, 5568, 5569, 5570, 5571, 5572, 5573, 5574, 5575, 5576, 5577, 5578, 5579, 5580, 5581, 5582, 5583, 5584, 5585, 5586, 5587, 5588, 5589, 5590, 5591, 5592, 5593, 5594, 5595, 5596, 5597, 5598, 5599, 5600, 5601, 5602, 5603, 5604, 5605, 5606, 5607, 5608, 5609, 5610, 5611, 5612, 5613, 5614, 5615, 5616, 5617, 5618, 5619, 5620, 5621, 5622, 5623, 5624, 5625, 5626, 5627, 5628, 5629, 5630, 5631, 5632, 5633, 5634, 5635, 5636, 5637, 5638, 5639, 5640, 5641, 5642, 5643, 5644, 5645, 5646, 5647, 5648, 5649, 5650, 5651, 5652, 5653, 5654, 5655, 5656, 5657, 5658, 5659, 5660, 5661, 5662, 5663, 5664, 5665, 5666, 5667, 5668, 5669, 5670, 5671, 5672, 5673, 5674, 5675, 5676, 5677, 5678, 5679, 5680, 5681, 5682, 5683, 5684, 5685, 5686, 5687, 5688, 5689, 5690, 5691, 5692, 5693, 5694, 5695, 5696, 5697, 5698, 5699, 5700, 5701, 5702, 5703, 5704, 5705, 5706, 5707, 5708, 5709, 5710, 5711, 5712, 5713, 5714, 5715, 5716, 5717, 5718, 5719, 5720, 5721, 5722, 5723, 5724, 5725, 5726, 5727, 5728, 5729, 5730, 5731, 5732, 5733, 5734, 5735, 5736, 5737, 5738, 5739, 5740, 5741, 5742, 5743, 5744, 5745, 5746, 5747, 5748, 5749, 5750, 5751, 5752, 5753, 5754, 5755, 5756, 5757, 5758, 5759, 5760, 5761, 5762, 5763, 5764, 5765, 5766, 5767, 5768, 5769, 5770, 5771, 5772, 5773, 5774, 5775, 5776, 5777, 5778, 5779, 5780, 5781, 5782, 5783, 5784, 5785, 5786, 5787, 5788, 5789, 5790, 5791, 5792, 5793, 5794, 5795, 5796, 5797, 5798, 5799, 5800, 5801, 5802, 5803, 5804, 5805, 5806, 5807, 5808, 5809, 5810, 5811, 5812, 5813, 5814, 5815, 5816, 5817, 5818, 5819, 5820, 5821, 5822, 5823, 5824, 5825, 5826, 5827, 5828, 5829, 5830, 5831, 5832, 5833, 5834, 5835, 5836, 5837, 5838, 5839, 5840, 5841, 5842, 5843, 5844, 5845, 5846, 5847, 5848, 5849, 5850, 5851, 5852, 5853, 5854, 5855, 5856, 5857, 5858, 5859, 5860, 5861, 5862, 5863, 5864, 5865, 5866, 5867, 5868, 5869, 5870, 5871, 5872, 5873, 5874, 5875, 5876, 5877, 5878, 5879, 5880, 5881, 5882, 5883, 5884, 5885, 5886, 5887, 5888, 5889, 5890, 5891, 5892, 5893, 5894, 5895, 5896, 5897, 5898, 5899, 5900, 5901, 5902, 5903, 5904, 5905, 5906, 5907, 5908, 5909, 5910, 5911, 5912, 5913, 5914, 5915, 5916, 5917, 5918, 5919, 5920, 5921, 5922, 5923, 5924, 5925, 5926, 5927, 5928, 5929, 5930, 5931, 5932, 5933, 5934, 5935, 5936, 5937, 5938, 5939, 5940, 5941, 5942, 5943, 5944, 5945, 5946, 5947, 5948, 5949, 5950, 5951, 5952, 5953, 5954, 5955, 5956, 5957, 5958, 5959, 5960, 5961, 5962, 5963, 5964, 5965, 5966, 5967, 5968, 5969, 5970, 5971, 5972, 5973, 5974, 5975, 5976, 5977, 5978, 5979, 5980, 5981, 5982, 5983, 5984, 5985, 5986, 5987, 5988, 5989, 5990, 5991, 5992, 5993, 5994, 5995, 5996, 5997, 5998, 5999, 6000, 6001, 6002, 6003, 6004, 6005, 6006, 6007, 6008, 6009, 6010, 6011, 6012, 6013, 6014, 6015, 6016, 6017, 6018, 6019, 6020, 6021, 6022, 6023, 6024, 6025, 6026, 6027, 6028, 6029, 6030, 6031, 6032, 6033, 6034, 6035, 6036, 6037, 6038, 6039, 6040, 6041, 6042, 6043, 6044, 6045, 6046, 6047, 6048, 6049, 6050, 6051, 6052, 6053, 6054, 6055, 6056, 6057, 6058, 6059, 6060, 6061, 6062, 6063, 6064, 6065, 6066, 6067, 6068, 6069, 6070, 6071, 6072, 6073, 6074, 6075, 6076, 6077, 6078, 6079, 6080, 6081, 6082, 6083, 6084, 6085, 6086, 6087, 6088, 6089, 6090, 6091, 6092, 6093, 6094, 6095, 6096, 6097, 6098, 6099, 6100, 6101, 6102, 6103, 6104, 6105, 6106, 6107, 6108, 6109, 6110, 6111, 6112, 6113, 6114, 6115, 6116, 6117, 6118, 6119, 6120, 6121, 6122, 6123, 6124, 6125, 6126, 6127, 6128, 6129, 6130, 6131, 6132, 6133, 6134, 6135, 6136, 6137, 6138, 6139, 6140, 6141, 6142, 6143, 6144, 6145, 6146, 6147, 6148, 6149, 6150, 6151, 6152, 6153, 6154, 6155, 6156, 6157, 6158, 6159, 6160, 6161, 6162, 6163, 6164, 6165, 6166, 6167, 6168, 6169, 6170, 6171, 6172, 6173, 6174, 6175, 6176, 6177, 6178, 6179, 6180, 6181, 6182, 6183, 6184, 6185, 6186, 6187, 6188, 6189, 6190, 6191, 6192, 6193, 6194, 6195, 6196, 6197, 6198, 6199, 6200, 6201, 6202, 6203, 6204, 6205, 6206, 6207, 6208, 6209, 6210, 6211, 6212, 6213, 6214, 6215, 6216, 6217, 6218, 6219, 6220, 6221, 6222, 6223, 6224, 6225, 6226, 6227, 6228, 6229, 6230, 6231, 6232, 6233, 6234, 6235, 6236, 6237, 6238, 6239, 6240, 6241, 6242, 6243, 6244, 6245, 6246, 6247, 6248, 6249, 6250, 6251, 6252, 6253, 6254, 6255, 6256, 6257, 6258, 6259, 6260, 6261, 6262, 6263, 6264, 6265, 6266, 6267, 6268, 6269, 6270, 6271, 6272, 6273, 6274, 6275, 6276, 6277, 6278, 6279, 6280, 6281, 6282, 6283, 6284, 6285, 6286, 6287, 6288, 6289, 6290, 6291, 6292, 6293, 6294, 6295, 6296, 6297, 6298, 6299, 6300, 6301, 6302, 6303, 6304, 6305, 6306, 6307, 6308, 6309, 6310, 6311, 6312, 6313, 6314, 6315, 6316, 6317, 6318, 6319, 6320, 6321, 6322, 6323, 6324, 6325, 6326, 6327, 6328, 6329, 6330, 6331, 6332, 6333, 6334, 6335, 6336, 6337, 6338, 6339, 6340, 6341, 6342, 6343, 6344, 6345, 6346, 6347, 6348, 6349, 6350, 6351, 6352, 6353, 6354, 6355, 6356, 6357, 6358, 6359, 6360, 6361, 6362, 6363, 6364, 6365, 6366, 6367, 6368, 6369, 6370, 6371, 6372, 6373, 6374, 6375, 6376, 6377, 6378, 6379, 6380, 6381, 6382, 6383, 6384, 6385, 6386, 6387, 6388, 6389, 6390, 6391, 6392, 6393, 6394, 6395, 6396, 6397, 6398, 6399, 6400, 6401, 6402, 6403, 6404, 6405, 6406, 6407, 6408, 6409, 6410, 6411, 6412, 6413, 6414, 6415, 6416, 6417, 6418, 6419, 6420, 6421, 6422, 6423, 6424, 6425, 6426, 6427, 6428, 6429, 6430, 6431, 6432, 6433, 6434, 6435, 6436, 6437, 6438, 6439, 6440, 6441, 6442, 6443, 6444, 6445, 6446, 6447, 6448, 6449, 6450, 6451, 6452, 6453, 6454, 6455, 6456, 6457, 6458, 6459, 6460, 6461, 6462, 6463, 6464, 6465, 6466, 6467, 6468, 6469, 6470, 6471, 6472, 6473, 6474, 6475, 6476, 6477, 6478, 6479, 6480, 6481, 6482, 6483, 6484, 6485, 6486, 6487, 6488, 6489, 6490, 6491, 6492, 6493, 6494, 6495, 6496, 6497, 6498, 6499, 6500, 6501, 6502, 6503, 6504, 6505, 6506, 6507, 6508, 6509, 6510, 6511, 6512, 6513, 6514, 6515, 6516, 6517, 6518, 6519, 6520, 6521, 6522, 6523, 6524, 6525, 6526, 6527, 6528, 6529, 6530, 6531, 6532, 6533, 6534, 6535, 6536, 6537, 6538, 6539, 6540, 6541, 6542, 6543, 6544, 6545, 6546, 6547, 6548, 6549, 6550, 6551, 6552, 6553, 6554, 6555, 6556, 6557, 6558, 6559, 6560, 6561, 6562, 6563, 6564, 6565, 6566, 6567, 6568, 6569, 6570, 6571, 6572, 6573, 6574, 6575, 6576, 6577, 6578, 6579, 6580, 6581, 6582, 6583, 6584, 6585, 6586, 6587, 6588, 6589, 6590, 6591, 6592, 6593, 6594, 6595, 6596, 6597, 6598, 6599, 6600, 6601, 6602, 6603, 6604, 6605, 6606, 6607, 6608, 6609, 6610, 6611, 6612, 6613, 6614, 6615, 6616, 6617, 6618, 6619, 6620, 6621, 6622, 6623, 6624, 6625, 6626, 6627, 6628, 6629, 6630, 6631, 6632, 6633, 6634, 6635, 6636, 6637, 6638, 6639, 6640, 6641, 6642, 6643, 6644, 6645, 6646, 6647, 6648, 6649, 6650, 6651, 6652, 6653, 6654, 6655, 6656, 6657, 6658, 6659, 6660, 6661, 6662, 6663, 6664, 6665, 6666, 6667, 6668, 6669, 6670, 6671, 6672, 6673, 6674, 6675, 6676, 6677, 6678, 6679, 6680, 6681, 6682, 6683, 6684, 6685, 6686, 6687, 6688, 6689, 6690, 6691, 6692, 6693, 6694, 6695, 6696, 6697, 6698, 6699, 6700, 6701, 6702, 6703, 6704, 6705, 6706, 6707, 6708, 6709, 6710, 6711, 6712, 6713, 6714, 6715, 6716, 6717, 6718, 6719, 6720, 6721, 6722, 6723, 6724, 6725, 6726, 6727, 6728, 6729, 6730, 6731, 6732, 6733, 6734, 6735, 6736, 6737, 6738, 6739, 6740, 6741, 6742, 6743, 6744, 6745, 6746, 6747, 6748, 6749, 6750, 6751, 6752, 6753, 6754, 6755, 6756, 6757, 6758, 6759, 6760, 6761, 6762, 6763, 6764, 6765, 6766, 6767, 6768, 6769, 6770, 6771, 6772, 6773, 6774, 6775, 6776, 6777, 6778, 6779, 6780, 6781, 6782, 6783, 6784, 6785, 6786, 6787, 6788, 6789, 6790, 6791, 6792, 6793, 6794, 6795, 6796, 6797, 6798, 6799, 6800, 6801, 6802, 6803, 6804, 6805, 6806, 6807, 6808, 6809, 6810, 6811, 6812, 6813, 6814, 6815, 6816, 6817, 6818, 6819, 6820, 6821, 6822, 6823, 6824, 6825, 6826, 6827, 6828, 6829, 6830, 6831, 6832, 6833, 6834, 6835, 6836, 6837, 6838, 6839, 6840, 6841, 6842, 6843, 6844, 6845, 6846, 6847, 6848, 6849, 6850, 6851, 6852, 6853, 6854, 6855, 6856, 6857, 6858, 6859, 6860, 6861, 6862, 6863, 6864, 6865, 6866, 6867, 6868, 6869, 6870, 6871, 6872, 6873, 6874, 6875, 6876, 6877, 6878, 6879, 6880, 6881, 6882, 6883, 6884, 6885, 6886, 6887, 6888, 6889, 6890, 6891, 6892, 6893, 6894, 6895, 6896, 6897, 6898, 6899, 6900, 6901, 6902, 6903, 6904, 6905, 6906, 6907, 6908, 6909, 6910, 6911, 6912, 6913, 6914, 6915, 6916, 6917, 6918, 6919, 6920, 6921, 6922, 6923, 6924, 6925, 6926, 6927, 6928, 6929, 6930, 6931, 6932, 6933, 6934, 6935, 6936, 6937, 6938, 6939, 6940, 6941, 6942, 6943, 6944, 6945, 6946, 6947, 6948, 6949, 6950, 6951, 6952, 6953, 6954, 6955, 6956, 6957, 6958, 6959, 6960, 6961, 6962, 6963, 6964, 6965, 6966, 6967, 6968, 6969, 6970, 6971, 6972, 6973, 6974, 6975, 6976, 6977, 6978, 6979, 6980, 6981, 6982, 6983, 6984, 6985, 6986, 6987, 6988, 6989, 6990, 6991, 6992, 6993, 6994, 6995, 6996, 6997, 6998, 6999, 7000, 7001, 7002, 7003, 7004, 7005, 7006, 7007, 7008, 7009, 7010, 7011, 7012, 7013, 7014, 7015, 7016, 7017, 7018, 7019, 7020, 7021, 7022, 7023, 7024, 7025, 7026, 7027, 7028, 7029, 7030, 7031, 7032, 7033, 7034, 7035, 7036, 7037, 7038, 7039, 7040, 7041, 7042, 7043, 7044, 7045, 7046, 7047, 7048, 7049, 7050, 7051, 7052, 7053, 7054, 7055, 7056, 7057, 7058, 7059, 7060, 7061, 7062, 7063, 7064, 7065, 7066, 7067, 7068, 7069, 7070, 7071, 7072, 7073, 7074, 7075, 7076, 7077, 7078, 7079, 7080, 7081, 7082, 7083, 7084, 7085, 7086, 7087, 7088, 7089, 7090, 7091, 7092, 7093, 7094, 7095, 7096, 7097, 7098, 7099, 7100, 7101, 7102, 7103, 7104, 7105, 7106, 7107, 7108, 7109, 7110, 7111, 7112, 7113, 7114, 7115, 7116, 7117, 7118, 7119, 7120, 7121, 7122, 7123, 7124, 7125, 7126, 7127, 7128, 7129, 7130, 7131, 7132, 7133, 7134, 7135, 7136, 7137, 7138, 7139, 7140, 7141, 7142, 7143, 7144, 7145, 7146, 7147, 7148, 7149, 7150, 7151, 7152, 7153, 7154, 7155, 7156, 7157, 7158, 7159, 7160, 7161, 7162, 7163, 7164, 7165, 7166, 7167, 7168, 7169, 7170, 7171, 7172, 7173, 7174, 7175, 7176, 7177, 7178, 7179, 7180, 7181, 7182, 7183, 7184, 7185, 7186, 7187, 7188, 7189, 7190, 7191, 7192, 7193, 7194, 7195, 7196, 7197, 7198, 7199, 7200, 7201, 7202, 7203, 7204, 7205, 7206, 7207, 7208, 7209, 7210, 7211, 7212, 7213, 7214, 7215, 7216, 7217, 7218, 7219, 7220, 7221, 7222, 7223, 7224, 7225, 7226, 7227, 7228, 7229, 7230, 7231, 7232, 7233, 7234, 7235, 7236, 7237, 7238, 7239, 7240, 7241, 7242, 7243, 7244, 7245, 7246, 7247, 7248, 7249, 7250, 7251, 7252, 7253, 7254, 7255, 7256, 7257, 7258, 7259, 7260, 7261, 7262, 7263, 7264, 7265, 7266, 7267, 7268, 7269, 7270, 7271, 7272, 7273, 7274, 7275, 7276, 7277, 7278, 7279, 7280, 7281, 7282, 7283, 7284, 7285, 7286, 7287, 7288, 7289, 7290, 7291, 7292, 7293, 7294, 7295, 7296, 7297, 7298, 7299, 7300, 7301, 7302, 7303, 7304, 7305, 7306, 7307, 7308, 7309, 7310, 7311, 7312, 7313, 7314, 7315, 7316, 7317, 7318, 7319, 7320, 7321, 7322, 7323, 7324, 7325, 7326, 7327, 7328, 7329, 7330, 7331, 7332, 7333, 7334, 7335, 7336, 7337, 7338, 7339, 7340, 7341, 7342, 7343, 7344, 7345, 7346, 7347, 7348, 7349, 7350, 7351, 7352, 7353, 7354, 7355, 7356, 7357, 7358, 7359, 7360, 7361, 7362, 7363, 7364, 7365, 7366, 7367, 7368, 7369, 7370, 7371, 7372, 7373, 7374, 7375, 7376, 7377, 7378, 7379, 7380, 7381, 7382, 7383, 7384, 7385, 7386, 7387, 7388, 7389, 7390, 7391, 7392, 7393, 7394, 7395, 7396, 7397, 7398, 7399, 7400, 7401, 7402, 7403, 7404, 7405, 7406, 7407, 7408, 7409, 7410, 7411, 7412, 7413, 7414, 7415, 7416, 7417, 7418, 7419, 7420, 7421, 7422, 7423, 7424, 7425, 7426, 7427, 7428, 7429, 7430, 7431, 7432, 7433, 7434, 7435, 7436, 7437, 7438, 7439, 7440, 7441, 7442, 7443, 7444, 7445, 7446, 7447, 7448, 7449, 7450, 7451, 7452, 7453, 7454, 7455, 7456, 7457, 7458, 7459, 7460, 7461, 7462, 7463, 7464, 7465, 7466, 7467, 7468, 7469, 7470, 7471, 7472, 7473, 7474, 7475, 7476, 7477, 7478, 7479, 7480, 7481, 7482, 7483, 7484, 7485, 7486, 7487, 7488, 7489, 7490, 7491, 7492, 7493, 7494, 7495, 7496, 7497, 7498, 7499, 7500, 7501, 7502, 7503, 7504, 7505, 7506, 7507, 7508, 7509, 7510, 7511, 7512, 7513, 7514, 7515, 7516, 7517, 7518, 7519, 7520, 7521, 7522, 7523, 7524, 7525, 7526, 7527, 7528, 7529, 7530, 7531, 7532, 7533, 7534, 7535, 7536, 7537, 7538, 7539, 7540, 7541, 7542, 7543, 7544, 7545, 7546, 7547, 7548, 7549, 7550, 7551, 7552, 7553, 7554, 7555, 7556, 7557, 7558, 7559, 7560, 7561, 7562, 7563, 7564, 7565, 7566, 7567, 7568, 7569, 7570, 7571, 7572, 7573, 7574, 7575, 7576, 7577, 7578, 7579, 7580, 7581, 7582, 7583, 7584, 7585, 7586, 7587, 7588, 7589, 7590, 7591, 7592, 7593, 7594, 7595, 7596, 7597, 7598, 7599, 7600, 7601, 7602, 7603, 7604, 7605, 7606, 7607, 7608, 7609, 7610, 7611, 7612, 7613, 7614, 7615, 7616, 7617, 7618, 7619, 7620, 7621, 7622, 7623, 7624, 7625, 7626, 7627, 7628, 7629, 7630, 7631, 7632, 7633, 7634, 7635, 7636, 7637, 7638, 7639, 7640, 7641, 7642, 7643, 7644, 7645, 7646, 7647, 7648, 7649, 7650, 7651, 7652, 7653, 7654, 7655, 7656, 7657, 7658, 7659, 7660, 7661, 7662, 7663, 7664, 7665, 7666, 7667, 7668, 7669, 7670, 7671, 7672, 7673, 7674, 7675, 7676, 7677, 7678, 7679, 7680, 7681, 7682, 7683, 7684, 7685, 7686, 7687, 7688, 7689, 7690, 7691, 7692, 7693, 7694, 7695, 7696, 7697, 7698, 7699, 7700, 7701, 7702, 7703, 7704, 7705, 7706, 7707, 7708, 7709, 7710, 7711, 7712, 7713, 7714, 7715, 7716, 7717, 7718, 7719, 7720, 7721, 7722, 7723, 7724, 7725, 7726, 7727, 7728, 7729, 7730, 7731, 7732, 7733, 7734, 7735, 7736, 7737, 7738, 7739, 7740, 7741, 7742, 7743, 7744, 7745, 7746, 7747, 7748, 7749, 7750, 7751, 7752, 7753, 7754, 7755, 7756, 7757, 7758, 7759, 7760, 7761, 7762, 7763, 7764, 7765, 7766, 7767, 7768, 7769, 7770, 7771, 7772, 7773, 7774, 7775, 7776, 7777, 7778, 7779, 7780, 7781, 7782, 7783, 7784, 7785, 7786, 7787, 7788, 7789, 7790, 7791, 7792, 7793, 7794, 7795, 7796, 7797, 7798, 7799, 7800, 7801, 7802, 7803, 7804, 7805, 7806, 7807, 7808, 7809, 7810, 7811, 7812, 7813, 7814, 7815, 7816, 7817, 7818, 7819, 7820, 7821, 7822, 7823, 7824, 7825, 7826, 7827, 7828, 7829, 7830, 7831, 7832, 7833, 7834, 7835, 7836, 7837, 7838, 7839, 7840, 7841, 7842, 7843, 7844, 7845, 7846, 7847, 7848, 7849, 7850, 7851, 7852, 7853, 7854, 7855, 7856, 7857, 7858, 7859, 7860, 7861, 7862, 7863, 7864, 7865, 7866, 7867, 7868, 7869, 7870, 7871, 7872, 7873, 7874, 7875, 7876, 7877, 7878, 7879, 7880, 7881, 7882, 7883, 7884, 7885, 7886, 7887, 7888, 7889, 7890, 7891, 7892, 7893, 7894, 7895, 7896, 7897, 7898, 7899, 7900, 7901, 7902, 7903, 7904, 7905, 7906, 7907, 7908, 7909, 7910, 7911, 7912, 7913, 7914, 7915, 7916, 7917, 7918, 7919, 7920, 7921, 7922, 7923, 7924, 7925, 7926, 7927, 7928, 7929, 7930, 7931, 7932, 7933, 7934, 7935, 7936, 7937, 7938, 7939, 7940, 7941, 7942, 7943, 7944, 7945, 7946, 7947, 7948, 7949, 7950, 7951, 7952, 7953, 7954, 7955, 7956, 7957, 7958, 7959, 7960, 7961, 7962, 7963, 7964, 7965, 7966, 7967, 7968, 7969, 7970, 7971, 7972, 7973, 7974, 7975, 7976, 7977, 7978, 7979, 7980, 7981, 7982, 7983, 7984, 7985, 7986, 7987, 7988, 7989, 7990, 7991, 7992, 7993, 7994, 7995, 7996, 7997, 7998, 7999, 8000, 8001, 8002, 8003, 8004, 8005, 8006, 8007, 8008, 8009, 8010, 8011, 8012, 8013, 8014, 8015, 8016, 8017, 8018, 8019, 8020, 8021, 8022, 8023, 8024, 8025, 8026, 8027, 8028, 8029, 8030, 8031, 8032, 8033, 8034, 8035, 8036, 8037, 8038, 8039, 8040, 8041, 8042, 8043, 8044, 8045, 8046, 8047, 8048, 8049, 8050, 8051, 8052, 8053, 8054, 8055, 8056, 8057, 8058, 8059, 8060, 8061, 8062, 8063, 8064, 8065, 8066, 8067, 8068, 8069, 8070, 8071, 8072, 8073, 8074, 8075, 8076, 8077, 8078, 8079, 8080, 8081, 8082, 8083, 8084, 8085, 8086, 8087, 8088, 8089, 8090, 8091, 8092, 8093, 8094, 8095, 8096, 8097, 8098, 8099, 8100, 8101, 8102, 8103, 8104, 8105, 8106, 8107, 8108, 8109, 8110, 8111, 8112, 8113, 8114, 8115, 8116, 8117, 8118, 8119, 8120, 8121, 8122, 8123, 8124, 8125, 8126, 8127, 8128, 8129, 8130, 8131, 8132, 8133, 8134, 8135, 8136, 8137, 8138, 8139, 8140, 8141, 8142, 8143, 8144, 8145, 8146, 8147, 8148, 8149, 8150, 8151, 8152, 8153, 8154, 8155, 8156, 8157, 8158, 8159, 8160, 8161, 8162, 8163, 8164, 8165, 8166, 8167, 8168, 8169, 8170, 8171, 8172, 8173, 8174, 8175, 8176, 8177, 8178, 8179, 8180, 8181, 8182, 8183, 8184, 8185, 8186, 8187, 8188, 8189, 8190, 8191
// };



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