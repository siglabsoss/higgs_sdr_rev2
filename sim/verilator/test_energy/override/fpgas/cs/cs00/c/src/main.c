#include "ringbus.h"
#include "xbaseband.h"
#include "vmem.h"
#include "csr_control.h"
#include "bootloader.h"
#include "pass_fail.h"

// #define LOAD_FFT_CFG
// #include "vmem_cfg.h"
#include "config_word_cmul_rx4_0f.h"
#include "config_word_cmul_rx4_00.h"
#include "config_word_cmul_rx4_01.h"
#include "config_word_cmul_eq_0f.h"
#include "config_word_conj_eq_0f.h"
#include "config_word_add_eq_00.h"
#include "config_word_sub_eq_00.h"
#include "config_word_add_rx4_00.h"
#include "config_word_add_rx4_01.h"
#include "config_word_magsquare_eq_00.h"
#include "config_word_magsquare_rx4_00.h"
#include "config_word_magsquare_rx4_15.h"


#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_ETH
#include "ringbus2_post.h"

#define VECTOR_REPORT_ADDRESS 0x3800
#define CP_LENGTH 256

VMEM_SECTION unsigned int bank_address_1[16] = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3};
VMEM_SECTION unsigned int permutation_address_4[16] = {0x0, 0x3000, 0x6000, 0x9000, 0xd000, 0xd000, 0xd000, 0xe000, 0xe000, 0xe000, 0xf000, 0xf000, 0xf000, 0x0, 0x0, 0x0};

VMEM_SECTION unsigned int all_one[16] = {0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};


// the first example
VMEM_SECTION unsigned int ofdm_data[1024] = {0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001};
VMEM_SECTION unsigned int benchmark[16] = {0x02000200};

VMEM_SECTION unsigned int test_data[16] = {0x00010001, 0x00020002, 0x00030003,0x00040004,0x00050005,0x00060006,0x00070007,0x00080008,0x00090009,0x000a000a,0x000b000b,0x000c000c,0x000d000d,0x000e000e};
VMEM_SECTION unsigned int output_0[512] = {0};
VMEM_SECTION unsigned int output_1[128] = {0};
VMEM_SECTION unsigned int output_2[32] = {0};
VMEM_SECTION unsigned int output_3[16] = {0};

void get_pwr(){
    MVXV_KNOP(V13, VMEM_ADDRESS(test_data));
    MVXV_KNOP(V14, VMEM_ADDRESS(test_data));
    MVXV_KNOP(V15, VMEM_ADDRESS(test_data));
    int ofdm_data_loc = VMEM_ADDRESS(ofdm_data);
    int benchmark_location = VMEM_ADDRESS(benchmark);
    int bank_addr_loc = VMEM_ADDRESS(bank_address_1);
    int permutation_addr_loc = VMEM_ADDRESS(permutation_address_4);
    int all_one_loc = VMEM_ADDRESS(all_one);
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

    cfg_loc = VMEM_ADDRESS(config_word_add_rx4_01);
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

void power_estimation(unsigned int data){
    unsigned int dma_start_addr = VMEM_ADDRESS(ofdm_data)*16;
    unsigned int output_pwr_addr = VMEM_ADDRESS(output_3)*16;
    unsigned int max_block_size = 1024;
    unsigned int block_size = 1024;
    unsigned int signal_power;
    unsigned int real_pwr;
    unsigned int imag_pwr;
    Ringbus ringbus;

    // Setting DSA gain
    ringbus.addr = 6;
    ringbus.data = DSA_GAIN_CMD|data;
    send_cmd(&ringbus);

    // Waiting for DSA gain command to be executed
    for(unsigned int i = 0; i < 1200; i++){
        asm("nop");
    }

    // Enable input DMA
    CSR_WRITE(DMA_0_START_ADDR, dma_start_addr);
    CSR_WRITE(DMA_0_LENGTH, block_size);
    CSR_WRITE(DMA_0_TIMER_VAL, START_IMMED_TIME);
    CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);

    INTERRUPT_WAIT_CLEAR_DMA_0();

    for(unsigned int i = 0; i < max_block_size - block_size; i++){
        vector_memory[dma_start_addr + block_size + i] = 0;
    }
    // Calculate power of OFDM symbol
    get_pwr();

    // Sending results to PC
    signal_power = vector_memory[output_pwr_addr];
    real_pwr = signal_power&0xffff;
    imag_pwr = signal_power>>16;
    signal_power = real_pwr + imag_pwr;
    SET_HALF_REG_VAR(x4, signal_power);
    ringbus.addr = 5;
    ringbus.data = signal_power;
    send_cmd(&ringbus);
}

void _saturation_ratio(Ringbus *ringbus, unsigned int gain){
    unsigned int delay[7] = {3, 6, 13, 26, 51, 102, 204};
    unsigned int samples_size = gain&0xF;
    unsigned int delay_value = delay[samples_size];
    unsigned int gain_value = gain&0xFFFFF0;
    unsigned int samples_saturated;
    
    ringbus->addr = 6;
    ringbus->data = DSA_GAIN_CMD|gain_value;
    send_cmd(ringbus);
    for(unsigned int i = 0; i < 1000; i++){
        asm("nop");
    }
    // Clear saturation counter
    CSR_READ(SATDETECT, samples_saturated);
    for(unsigned int i = 0; i < delay_value; i++){
        asm("nop");
    }
    CSR_READ(SATDETECT, samples_saturated);
    SET_HALF_REG_VAR(x4, samples_saturated);
    ringbus->addr = 5;
    ringbus->data = samples_saturated;
    send_cmd(ringbus);

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

    for(unsigned int i = 0; i < max_block_size - block_size; i++){
        vector_memory[dma_start_addr + block_size + i] = 0;
    }
    vector_memory[output_pwr_addr] = 0;
    get_pwr();

    // Calculate power from OFDM symbol
    signal_power = vector_memory[output_pwr_addr];
    real_pwr = signal_power&0xffff;
    imag_pwr = signal_power>>16;
    signal_power = real_pwr + imag_pwr;

    return signal_power;
}

unsigned int auto_gain_ctrl(Ringbus *ringbus){
    unsigned int atten_steps[3] = {8, 4, 2};
    unsigned int max_clipping_size = 102;
    unsigned int min_pwr = 40000;
    unsigned int max_pwr = 60000;
    unsigned int pwr_est_block_size = 1024;
    unsigned int start_atten = 16;
    unsigned int output_pwr_addr = VMEM_ADDRESS(output_3)*16;
    unsigned int samples_saturated;
    unsigned int signal_power = 0;

    for(unsigned int i = 0; i < 4; i++){
        ringbus->addr = 6;
        ringbus->data = DSA_GAIN_CMD|(start_atten*4<<3);
        send_cmd(ringbus);
        // Need to wait for CMD to reach ETH before executing next line
        for(unsigned int i = 0; i < 1000; i++){
            asm("nop");
        }
        // Function calls included for easier readability. Remove if speed
        // optimization needed
        samples_saturated = _get_sat_ratio();
        if(samples_saturated > max_clipping_size){
            start_atten += atten_steps[i]; 
        }
        else{
        // Function calls included for easier readability. Remove if speed
        // optimization needed
            signal_power = _estimate_pwr(pwr_est_block_size);
            // ring_block_send_eth(signal_power);
            SET_HALF_REG_VAR(x4, signal_power);
            if((signal_power >= min_pwr) && (signal_power <= max_pwr)){
                return start_atten*4<<3;
                break;
            }
            else if(signal_power < min_pwr){
                start_atten -= atten_steps[i];
            }
            else if(signal_power > max_pwr){
                start_atten += atten_steps[i];
            }
        }
    }
    return start_atten*4<<3;
}

void test_agc(unsigned int data){
    Ringbus ringbus;
    unsigned int atten_value = 0xdeadbeef;
    atten_value = auto_gain_ctrl(&ringbus);

    ringbus.addr = 5;
    ringbus.data = atten_value;
    send_cmd(&ringbus);
}

void test_saturation_ratio(unsigned int data){
    Ringbus ringbus;
    _saturation_ratio(&ringbus, data);
}

void listen_cmd(Ringbus *ringbus){
    CSR_WRITE(GPIO_WRITE_EN, LED_GPIO_BIT);
    while(1) {
        CSR_SET_BITS(GPIO_WRITE, LED_GPIO_BIT);
            check_ring(ringbus);
        for(int j = 0; j < 100000; j++) {
            check_ring(ringbus);
        }
        CSR_CLEAR_BITS(GPIO_WRITE, LED_GPIO_BIT);
        for(int j = 0; j < 1000000; j++) {
            check_ring(ringbus);
        }
    }
}

int main()
{
    Ringbus ringbus;
    ring_register_callback(&power_estimation, POWER_ESTIMATION_CMD);
    ring_register_callback(&test_agc, AGC_TEST_CMD);
    ring_register_callback(&test_saturation_ratio, SATURATION_RATIO_CMD);
    listen_cmd(&ringbus);

    return 0;
}