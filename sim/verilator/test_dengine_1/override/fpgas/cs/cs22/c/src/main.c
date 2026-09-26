#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "circular_buffer.h"
#include "fill.h"
#include "ringbus.h"
#include "coarse_sync.h"
#include "atan.h"
#include "xvcordic.h"

#include "flush_config_word_data.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "check_bootload.h"

#include "cs21test_buffer.h"
#include "corrupt_dma.h"
#include "fast_inv_sqrt.h"
#include "nco_data.h"
#include "vmem_copy.h"
#include "fixed_iir.h"
#include "trunk_types.h"
#include "self_sync.h"

#include <stdint.h>
#include <stdbool.h>

// #define EXFIL_SIZE (1024)
// #define EXFIL_CHUNKS (4)
// #define EXFIL_RINGBUS_TOLERANCE (2)
// #define EXFIL_COUNTER_DELAY (0x410000)
// #include "data_exfil.h"

// #define ENABLE_TB_DEBUG

#ifndef ENABLE_TB_DEBUG
#define DISABLE_TB_DEBUG
#endif
#include "tb_debug.h"


#define PING_PONG_BUFFER_SIZE (1024+TRUNK_LENGTH)

#include "ping_pong_driver.h"



///////////////////////////////////////
//// for fine sync
#include "config_word_cmul_rx4_0f.h"
#include "config_word_cmul_rx4_00.h"
#include "config_word_cmul_eq_0f.h"
#include "config_word_cmul_eq_08.h"
#include "config_word_cmul_eq_01.h"
#include "config_word_cmul_eq_00.h"
#include "config_word_conj_eq_11.h"
#include "config_word_conj_eq_0f.h"
#include "config_word_conj_eq_0b.h"
#include "config_word_conj_rx4_0f.h"
#include "config_word_add_eq_00.h"
#include "config_word_add_rx4_00.h"
#include "config_word_add_rx4_01.h"
#include "config_word_add_rx4_02.h"
#include "config_word_add_rx4_03.h"
#include "config_word_sub_eq_00.h"
#include "config_word_magsquare_eq_00.h"

VMEM_SECTION unsigned int nco_data[1024] = {0};
VMEM_SECTION unsigned int nco_data_phase_mask[1024] = {0};
VMEM_SECTION unsigned int nco_data_common_phase[16]={0};
// VMEM_SECTION unsigned int phase_mask_0[1024]={0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff};
// VMEM_SECTION unsigned int nco_data_common_phase_mask_0[1024]={0};
// VMEM_SECTION unsigned int phase_mask_1[1024]={0};
// VMEM_SECTION unsigned int nco_data_common_phase_mask_1[1024]={0};

// VMEM_SECTION unsigned int output_common_phase[256] = {0};

// VMEM_SECTION unsigned int permutation_memcopy[16] = {0xf000, 0xf000, 0xf000, 0xf000, 0xf000, 0xf000, 0xf000, 0xf000, 0xf000, 0xf000, 0xf000, 0xf000, 0xf000, 0xf000, 0xf000, 0xf000};
// VMEM_SECTION unsigned int permutation_pilot_conjmul_0[16] = {0x0000, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000, 0x9000, 0xa000, 0xb000, 0xc000, 0xd000, 0xe000, 0xf000, 0x0000};
// VMEM_SECTION unsigned int permutation_pilot_conjmul_1[16] = {0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000, 0x8000, 0xa000, 0xb000, 0xc000, 0xd000, 0xe000, 0xf000, 0x0000, 0x1000, 0x1000};
// VMEM_SECTION unsigned int bank_address_pilot_conjmul_0[16] = {0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1};
// VMEM_SECTION unsigned int bank_address_pilot_conjmul_1[16] = {0x2, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1};
// VMEM_SECTION unsigned int permutation_pilot_conjmul_add[16] = {0x0000, 0x3000, 0x6000, 0x9000, 0xd000, 0xd000, 0xd000, 0xe000, 0xe000, 0xe000, 0xf000, 0xf000, 0xf000, 0x0, 0x0, 0x0};
// VMEM_SECTION unsigned int bank_address_pilot_common_phase[16] = {0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};


VMEM_SECTION unsigned int qam_adj_coeff[1024] = {0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100};

unsigned int mag_counter = 2;

// Holds post rotated data
VMEM_SECTION unsigned int temp_space[1024+16];


#define MA_SIZE 16
#define MA_SIZE_LOG 4

int ring_flag=0;

int mag_adjust_flag = 0;

float rx_gain = 8640.0f*256.0f; //18000.0;

// imem copy of the input trunk
unsigned int input_trunk[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

VMEM_SECTION unsigned int do_not_rotate[16] = {
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



// void xbb_pilot_add_common_phase(unsigned int cfg_pilot_add_common_phase_location, unsigned int input_location, unsigned int output_location)
// {
    

//     // memory copy and shift

//     //positive tone

//     MVXV_KNOP(V12, VMEM_ROW_ADDRESS(bank_address_pilot_common_phase));
//     VNOP_LK15(V12);
//     MVK15V_KNOP(V12,0);


//     MVXV_KNOP(V0, input_location);
//     ADD_KNOP(V0, V0, V12, 0);

//     MVXV_KNOP(V1, output_location);
//     MVXV_KNOP(V2, 2);
    
//     for(int index=0; index<4; index++)
//     {
//         ADD_LK13(V0, V0, V2, 0);
//         ADD_SK13(V1, V1, V2, 0);
//     }
 

//     MVXV_KNOP(V0, input_location+1);
//     MVXV_KNOP(V1, output_location+1);
//     MVXV_KNOP(V12, VMEM_ROW_ADDRESS(permutation_memcopy));
//     VNOP_LK15(V12);
//     MVK15V_KNOP(V12,0);
//     ADD_KNOP(V1, V1, V12, 0);

//     MVXV_KNOP(V2, 2);
//     for(int index=0; index<4; index++)
//     {
//         ADD_LK13(V0, V0, V2, 0);
//         ADD_SK13(V1, V1, V2, 0);
//     }

//     // negative tone 

//     MVXV_KNOP(V0, input_location+56);
 
//     MVXV_KNOP(V1, output_location+8);
//     MVXV_KNOP(V2, 2);
    
//     for(int index=0; index<4; index++)
//     {
//         ADD_LK13(V0, V0, V2, 0);
//         ADD_SK13(V1, V1, V2, 0);
//     }
 

//     MVXV_KNOP(V0, input_location+56+1);
//     MVXV_KNOP(V1, output_location+8+1);
//     MVXV_KNOP(V12, VMEM_ROW_ADDRESS(permutation_memcopy));
//     VNOP_LK15(V12);
//     MVK15V_KNOP(V12,0);
//     ADD_KNOP(V1, V1, V12, 0);

//     MVXV_KNOP(V2, 2);
//     for(int index=0; index<4; index++)
//     {
//         ADD_LK13(V0, V0, V2, 0);
//         ADD_SK13(V1, V1, V2, 0);
//     }

//     // add pilot tones 

//     MVXV_KNOP(V0, cfg_pilot_add_common_phase_location);
//     VNOP_LK14(V0);

//     MVXV_KNOP(V13, VMEM_ROW_ADDRESS(bank_address_pilot_conjmul_0));
//     VNOP_LK15(V13);
//     MVK15V_KNOP(V13,0);

//     MVXV_KNOP(V1, output_location);
//     MVXV_KNOP(V2, output_location+2);
//     MVXV_KNOP(V3, output_location);
//     MVXV_KNOP(V4, 4);

//     ADD_KNOP(V1, V1, V13, 0);
//     ADD_KNOP(V2, V2, V13, 0);

//     for(int index = 0; index <4; index++)
//     {
//         ADD_LK8(V1, V1, V4, 0);
//         ADD_LK9(V2, V2, V4, 0);
//     }

//     for(int index =0; index <4; index++)
//     {
//         ADD_SK1(V3, V3, V4, 0);
//     }


//     MVXV_KNOP(V1, output_location);
//     MVXV_KNOP(V2, output_location+4);

//     MVXV_KNOP(V12, VMEM_ROW_ADDRESS(permutation_pilot_conjmul_add));
//     VNOP_LK15(V12);
//     MVK15V_KNOP(V12,0);

//     ADD_KNOP(V1, V1, V12, 0);
//     ADD_KNOP(V2, V2, V12, 0);

//     MVXV_KNOP(V5, 8);

//     MVXV_KNOP(V3, output_location);
    
//     for(int index = 0; index<2; index++)
//     {
//         ADD_LK8(V1, V1, V5, 0);
//         ADD_LK9(V2, V2, V5, 0);
//     }

//     for(int index = 0; index <2; index++)
//     {
//         ADD_SK1(V3, V3, V5, 0);
//     }

//     //add positive and negative tones
//     MVXV_KNOP(V0, VMEM_ROW_ADDRESS(config_word_add_eq_00));
//     VNOP_LK14(V0);

//     MVXV_KNOP(V1, output_location);
//     MVXV_KNOP(V2, output_location+8);
//     MVXV_KNOP(V3, output_location);

//     VNOP_LK8(V1);
//     VNOP_LK9(V2);
//     VNOP_SK1(V3);

//     STALL(50);



// }

/// extrats cfo subcarriers and loads trunk
/// performs a rotation of all DATA subcarriers by the same angle
/// pilot subcarriers are not rotated
/// this angle is read from one of the pilot tones
/// this is what removes the phase jitter
///
void xbb_residue_phase(
    const unsigned int input_data_location,
    const unsigned int temp_data_location,
    const unsigned int output_data_location,
    const unsigned int phase_mode ) // from global phase_correction_mode
{


   // xbb_pilot_add_common_phase(VMEM_ADDRESS(config_word_add_rx4_03), input_data_location, VMEM_ADDRESS(output_common_phase));
   // unsigned int pilot_common_phase = vector_memory[VMEM_ADDRESS(output_common_phase)*16];

   // STALL(50);

    const unsigned int one_pilot_observe
        = vector_memory[VMEM_ROW_ADDRESS_TO_DMA(input_data_location)+2];
    const unsigned int second_pilot_observe
        = vector_memory[VMEM_ROW_ADDRESS_TO_DMA(input_data_location)+4];

    STALL(50); 

    ////////////////////////////////////////////////////////////////////////////////
    ////// for beamforming rx residue phase

    unsigned int one_pilot_residue = 0x0;
    unsigned int second_pilot_residue = 0x0;

    if(phase_mode == 1)
    {
        one_pilot_residue = one_pilot_observe;
    }
    else if(phase_mode == 2)
    {
        second_pilot_residue = second_pilot_observe;
    }
    else if(phase_mode == 3)
    {
        one_pilot_residue = one_pilot_observe;
        second_pilot_residue = second_pilot_observe;
    }

    const int16_t a_r = (one_pilot_residue&0xffff);
    const int16_t a_i = (((one_pilot_residue)>>16)&0xffff);

    const int16_t b_r = (second_pilot_residue&0xffff);
    const int16_t b_i = (((second_pilot_residue)>>16)&0xffff);


    int32_t result_r = a_r + b_r; 
    int32_t result_i = a_i + b_i;

    if((result_r>0x7fff)||(result_r<-0x8000)||(result_i>0x7fff)||(result_i<-0x8000))
    {
        result_r = (result_r>>1);
        result_r = (result_i>>1);
    }

    const unsigned int pilot_residue = (((result_i)<<16)&0xffff0000) | ((result_r)&0xffff);

    unsigned int temp_common_angle;

    ATAN(temp_common_angle,pilot_residue,15);
    temp_common_angle = temp_common_angle&0xffff;
    

   
   // unsigned int temp_common_angle = fxpt_atan2(((pilot_common_phase>>16)&0xffff), (pilot_common_phase & 0xffff));

   const unsigned int nco_angle_common_phase = ((0x10000-temp_common_angle)<<16); // with common phase correction at rx

   //nco_angle_common_phase = 0;  // without common phase correction at rx
   make_nco(VMEM_DMA_ADDRESS(nco_data_common_phase), 16, nco_angle_common_phase, 0);
   
   unsigned int occupancy;

   while(1) {
    CSR_READ(DMA_2_SCHEDULE_OCCUPANCY, occupancy);
    if(occupancy == 0) {
      break;
    }
   }


    uint32_t buffer_predicate = 0xaaaa;

    // when this is set
    // we do not rotate any subcarriers
    // we still perform the multiplication, but it is by a unity vector
    if( phase_mode == 0 ) {
        // according to documentation from vmem_interleave_copy_repeat()
        // "bit position set to 1 will copy from a, set to 0 will copy from b"
        // this means we set all 0's and we will copy all values from do_not_rotate
        buffer_predicate = 0x0000;
    }

    // arg 0 is rotation             (16)
    // arg 1 is do not rotate        (16)
    // arg 2 is output               (1024)
    // arg 3 is number of rows to write to output
    // arg 4 is mask (predicate) 16 bits
    vmem_interleave_copy_repeat(
        VMEM_ROW_ADDRESS(nco_data_common_phase),
        VMEM_ROW_ADDRESS(do_not_rotate),
        VMEM_ROW_ADDRESS(nco_data_phase_mask),
        64,
        buffer_predicate
    );

   // unsigned int check_data = vector_memory[VMEM_ROW_ADDRESS(nco_data_common_phase)*16+1023];
   // ring_block_send_eth(check_data);


   // xbb_conj_multi(VMEM_ADDRESS(config_word_cmul_eq_0f), VMEM_ROW_ADDRESS(nco_data_common_phase), VMEM_ROW_ADDRESS(phase_mask_0), VMEM_ROW_ADDRESS(nco_data_common_phase_mask_0));
   // unsigned int check_data_0 = vector_memory[VMEM_ROW_ADDRESS(nco_data_common_phase_mask_0)*16+1023];
   // ring_block_send_eth(check_data_0);

   xbb_conj_multi(
    VMEM_ROW_ADDRESS(config_word_cmul_eq_0f),
    input_data_location,
    VMEM_ROW_ADDRESS(nco_data_phase_mask),
    temp_data_location);


   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   // copy these locations of the trunk to imem
   input_trunk[TRUNK_R0_SFO] = vector_memory[VMEM_ROW_ADDRESS_TO_DMA(input_data_location)+1024+TRUNK_R0_SFO];
   input_trunk[TRUNK_R1_SFO] = vector_memory[VMEM_ROW_ADDRESS_TO_DMA(input_data_location)+1024+TRUNK_R1_SFO];

   STALL(50);


   // SET_REG(x3,0xeeeeeeee);
   // SET_REG(x3,0);
   // SET_REG(x3, nco_data_phase_mask[0]);
   // SET_REG(x3, nco_data_phase_mask[1]);
   // SET_REG(x3, nco_data_phase_mask[2]);
   // SET_REG(x3, nco_data_phase_mask[3]);

   /// Write the output trunk.  Some comes from input_trunk which is imem
   /// some others come from one_pilot_observe,second_pilot_observe which were set above

   // first 2 are for the first transmitter
   vector_memory[VMEM_ROW_ADDRESS_TO_DMA(output_data_location)+1024+TRUNK_R0_SFO] = input_trunk[TRUNK_R0_SFO];
   // vector_memory[output_data_location*16+1025] = pilot_common_phase;

   vector_memory[VMEM_ROW_ADDRESS_TO_DMA(output_data_location)+1024+TRUNK_R0_CFO] = one_pilot_observe;

   // next 2 are for the second transmitter
   vector_memory[VMEM_ROW_ADDRESS_TO_DMA(output_data_location)+1024+TRUNK_R1_SFO] = input_trunk[TRUNK_R1_SFO];
   vector_memory[VMEM_ROW_ADDRESS_TO_DMA(output_data_location)+1024+TRUNK_R1_CFO] = second_pilot_observe;
   STALL(50);

}

//////////////////////////////////////////////////////////



#define MY_ASSERT(x) if(!(x)) { ring_block_send_eth(0xe0000000|__LINE__);}


#define DMA_OUT_CHUNK_ATTACHMENT (TRUNK_LENGTH)
#define DMA_IN_CHUNK (1024+DMA_OUT_CHUNK_ATTACHMENT)

#define DMA_OUT_CHUNK (1024+DMA_OUT_CHUNK_ATTACHMENT)


void recover_last(void) {
    ring_block_send_eth(DMA_LAST_ERROR_PCCMD | OUR_RING_ENUM);
    dma_run_till_last();
}


// Phase correction default on
unsigned int phase_correction_mode = 1;

#define INITIAL_MAG_VALUE (0x00000400)

// this is the filter state
// 

// These macros and include will generate
// uint32_t mag_filter_pilot_estimate[1024] = {x,x,x,x,...};
// where x is the default value

#define INCLUDE_VECTOR_AS mag_filter_pilot_estimate
#define INCLUDE_TYPE_AS uint32_t
#define VECTOR_INITIAL_VALUE INITIAL_MAG_VALUE
#include "dmem_vector_1k.h"

unsigned int frames_parsed = 0;


// This floating point math will calculate the iir filter coefficient
// note that this floating point math is done at compile time and gain will get
// baked in as a constant
// this is signed for later math
// int32_t mag_filter_gain = 65536 * (0.07f);
int32_t mag_filter_gain = 65536 * (0.01f);       // Default of soft filter
// int32_t mag_filter_gain = 0;

// const unsigned use_filter = 0;
// mag_filter_gain is [0 - 65535]


///
/// Accesses the globals:
///   mag_adjust_flag
///   mag_filter_gain
///   mag_counter
///   mag_filter_pilot_estimate
///   qam_adj_coeff

/// mag_adjust_flag
///  * 0     disable
///  * 1     1:1 mag adjust, every other
///  * 2     2:1 mag adjust, every other
///  * 3     1:1 mag adjust, every, running
///  * 4     1:1 mag adjust, every, locked
///
/// 
/// @param[in]   cpu_in    cpu pointer to input memory
/// @param[out]  cpu_out   cpu pointer to output memory

// void handle_1x_adjustment(void) {}


unsigned do_work(
                const unsigned int index,
                const unsigned int* const cpu_in,
                      unsigned int* const cpu_out
               ) {
  (void)index;

  unsigned int read_clk_1, read_clk_2;

  CSR_READ(TIMER_VALUE, read_clk_1); 
  

    const unsigned int* cpu_ptr_from_dma = cpu_in;//(unsigned int*) dma_in_ptr[consume_idx];
    const unsigned int* cpu_ptr_fft      = cpu_out;//(unsigned int*) fft_ptr[consume_idx];

    // if( exfil_active() ) {
    //     unsigned int exfil_row = exfil_working_on_row();
    //     SET_REG(x3, 0xffff0000);
    //     SET_REG(x3, exfil_row);
    //     SET_REG(x3, 0x0);
    //     vmem_copy_rows(VMEM_ROW_ADDRESS(cpu_ptr_from_dma), exfil_row, 64 );
    //     exfil_wrote_row();
    // }

    // set to 1 to disable phase rotation
    // this flag is backwards, so we use this logic to set true when more is 0
    // and false in other modes
    // unsigned int disable_phase_correction = (phase_correction_mode==0);

    xbb_residue_phase(
        VMEM_ROW_ADDRESS(cpu_ptr_from_dma),
        VMEM_ROW_ADDRESS(temp_space),
        VMEM_ROW_ADDRESS(cpu_ptr_fft),
        phase_correction_mode);


    const bool adjust_enabled = mag_adjust_flag > 0 && mag_adjust_flag <= 4;
    const bool adjust_beamform_mode = mag_adjust_flag == 2; // beamforming mode, aka true 2:1 false 1:1
    const bool adjust_every_other_pilot = mag_adjust_flag <= 2; // modes 1,2  (and 0 is do not care) 

    const bool adjust_enable_filter = mag_filter_gain != 0;

    const bool mag_locked = mag_adjust_flag == 4; // modes 1,2,3 always update, mode 4 is locked

    // for verilator only do not set this in hardware or mag will be stuck and affect other stuff
    // mag_adjust_flag = 1;
    
    if(mag_adjust_flag == 0) {

        // previously this was 65 to include the "trunk"
        // now 64 because trunk data was already copied in xbb_residue_phase
        vmem_copy_rows(
            VMEM_ROW_ADDRESS(temp_space),
            VMEM_ROW_ADDRESS(cpu_ptr_fft),
            64
            );
    } else if(adjust_enabled) {
        /// adjust qam coefficient round robin each time for one tone
        /// mag_counter starts at 2, and is added by 4 until it wraps
        // int start_clk, end_clk;
        // CSR_READ(TIMER_VALUE, start_clk);
        unsigned int channel_data;
        unsigned int channel_data2 = 0;
        unsigned int mag_counter2;

        //
        // Debuggers notes:
        // mag_counter = 38; // this must 2+(4*n)
        // pilot tone 38 will be used to adjust
        //   data tone 37
        //   data tone 39


        // should read from pilot data
        // FIXME: this equation is weird, and may involve extra shifts, this could
        // be replaced with something else from vmem.h
        channel_data = vector_memory[VMEM_DMA_ADDRESS(temp_space)+mag_counter];

        SET_REG(x4, frames_parsed);
        SET_REG(x3, channel_data);


        /// If we are in "beamforming" magnitude mode
        /// we add two pilots together, and use the result to calculate our 1/x
        /// we need to calculate the next counter
        if( adjust_beamform_mode ) {
            // grab the next tone
            mag_counter2 = mag_counter + 2;
            if( mag_counter2 >= 1024 ) {
                // if the next tone would have gone beyond edge
                // just add the same tone twice
                // this hack shouldn't affect our data as this is only on the edge subcarrier
                mag_counter2 = mag_counter;
            }

            channel_data2 = vector_memory[VMEM_DMA_ADDRESS(temp_space)+mag_counter2];
        }

        int64_t re,im;

        if( adjust_beamform_mode ) {
            // Here we are taking 1/(mag(v1+v2)) however this may cause an error
            // due to the fact that the new eq method does not adjust the eq for the pilot tones so we can track STO
            // Because of this we are suming two non zero degree eq vectors.  The rotations for the data tones are indeed
            // zero degrees, so when the data tones sum, they will have more magnitude than we calculate here
            re =  (int16_t)(channel_data & 0xffff);
            im =  (int16_t)((channel_data>>16) & 0xffff);

            re += (int16_t)(channel_data2 & 0xffff);
            im += (int16_t)((channel_data2>>16) & 0xffff);
        } else {
            // put the real and imaginary parts in 64 bits, so we won't overflow when we calculate the mag^2
            // we cast them to int16_t so that the compiler will sign extend for us
            // this is usually the fastest way to sign extend
            re =  (int16_t)(channel_data & 0xffff);
            im =  (int16_t)((channel_data>>16) & 0xffff);
        }


        //// at this point re,im are the sample we want to measure
        //// if we skip the filter, these raw values will have their 1/x taken




        // non zero enables the filter to run
        if( adjust_enable_filter ) {

            // repack
            // note this is a waste in single mode
            // but needed in 2:1 mode
            const uint32_t filter_update = (((im)<<16)&0xffff0000) | ((re)&0xffff);

            uint32_t* filter_state;

            // pointer arithmetic, but only if will result in a valid pointer in to the [1024] buffer
            if( mag_counter < 1024 ) {
                filter_state = mag_filter_pilot_estimate + mag_counter;
            } else {
                // Error condition, just use 0'th element
                filter_state = mag_filter_pilot_estimate;
                // SET_REG(0xff00)
            }

            // fixed_iir with a fixed shift of 16.
            // mag_filter_gain is [0 - 65535]
            fixed_iir_16(filter_state, &filter_update, mag_filter_gain);

            // unpack, needed so we can take mag below
            re =  (int16_t)(  (*filter_state)       & 0xffff);
            im =  (int16_t)( ((*filter_state) >>16) & 0xffff);
        }


        //// at this point re,im are the final, filtered values for 1/x



        // calculate magnitude squared
        const float mag_squared = (re*re)+(im*im);

        // calculate the 1/sqrt() of our magnitude_squared.
        // we multiple the result by rx_gain
        // this method is not sensative to rotation of the pilot tones
        unsigned int new_gain = rx_gain * fast_inv_sqrt(mag_squared);

        // old method, sensative to rotation
        // unsigned int new_gain = (unsigned int)(rx_gain/(channel_data & 0xffff));

        if(new_gain > 0x7fff) {
            new_gain = 0x7fff;
        }

        ///// at this line we've calculated the new_gain, and saturated it to a max value

        // mag counter starts at 2 and goes by 4
        // 2
        // 6
        // 10
        // ...
        // 1018
        // 1022
        // 1026

        if( adjust_every_other_pilot ) {
            if( !mag_locked ) {
                // unlocked, continue to update values
                vector_memory[VMEM_DMA_ADDRESS(qam_adj_coeff)+mag_counter-1] = new_gain;
                vector_memory[VMEM_DMA_ADDRESS(qam_adj_coeff)+mag_counter+1] = new_gain;
            }

            STALL(30);

            mag_counter += 4;
            if(mag_counter >= 1026) {
                mag_counter = 2;
            }
        } else {
            if( !mag_locked ) {
                // unlocked, continue to update values
                vector_memory[VMEM_DMA_ADDRESS(qam_adj_coeff)+mag_counter] = new_gain;
            }
            STALL(30);

            mag_counter++;
            if(mag_counter >= 1024) {
                mag_counter = 1;
            }
        }

        // CSR_READ(TIMER_VALUE, end_clk);
        // ring_block_send_eth(end_clk - start_clk);
        // ring_block_send_eth(channel_data);
        // ring_block_send_eth(new_gain);

        // ////////////////////////////////////////////////////////////////
        // just qam adjustment
        xbb_conj_multi(
            VMEM_ROW_ADDRESS(config_word_cmul_eq_08),
            VMEM_ROW_ADDRESS(temp_space),
            VMEM_ROW_ADDRESS(qam_adj_coeff), 
            VMEM_ROW_ADDRESS(cpu_ptr_fft));

        CSR_READ(TIMER_VALUE, read_clk_2); // this will be 0x124  (4 clock cycles later)

        SET_REG(x3, 0x33333333);
        SET_REG(x3, (read_clk_2 - read_clk_1));

        // ring_block_send_eth(0x8e000000|(read_clk_2 - read_clk_1));

    }
    ///////////////////
    

    //////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////

    // forward trunk 6, the other part of the trunk were handled in xbb_conj_multi
    const unsigned int trunk6 = vector_memory[VMEM_DMA_ADDRESS(cpu_ptr_from_dma) + 1024 + TRUNK_FRAME_COUNTER];
    vector_memory[VMEM_DMA_ADDRESS(cpu_ptr_fft) + 1024 + TRUNK_FRAME_COUNTER] = trunk6;


    frames_parsed++;

    return 1;
}


void magnitude_adjustment_callback(unsigned int data)
{
    if( data <= 4 ) {
        mag_adjust_flag = data;
    } else {
        mag_adjust_flag = 0;
    }
}

void set_gain_callback(unsigned int data) {
    rx_gain = (float)data*256.0f;
}

// void trigger_exfil_callback(unsigned int data) {
//     if( data == 0 ) {
//         SET_REG(x3, 0xf3034444);
//         SET_REG(x3, 0x0);
//         exfil_request();
//     }
// }




// Sets the gain for the rx pilot tone estimates
// set to 0 to disable any filtering of rx pilot tone estimates 
// the max value is 0x10000
// see pet_fft() about line 734
// 
// sets global mag_filter_gain  which has a range [0 - 65535]
void set_mag_filter_callback(const unsigned int data) {
    const uint32_t gain = data & 0x1ffff;
    // uint32_t reset = (data & 0xf00000) >> 20;
    
    if( data > 0x10000 ) {
        mag_filter_gain = 0;
    } else {
        mag_filter_gain = gain; // probably ends up being positive only
    }

    // fixme add reset
    // overwrite mag_filter_pilot_estimate or something
}

// only valid values are 0,1,2,
// Anything other than above values will result in the value
// being set to 0
// this function always updates the value
// controlls whether rx side will adjust phase of all sc based in pilot tones
// 0 do nothing
// 1 correct for radio 1
// 2 correct for radio 2
// 3 correct for radio 1,2 beamforming
void set_phase_correction_mode_callback(unsigned int data) {
    if(data == 1 || data == 2 || data == 3) {
        phase_correction_mode = data;
        return;
    }
    phase_correction_mode = 0;
}


int main2(void);
int main(void)
{
    self_sync_block_boot();
    main2();
    return 0;
}
int main2(void) {

   // ring_register_callback(fine_sync_callback, SYNCHRONIZATION_CMD);

  // unsigned int burn = 16;
  // for(unsigned int i = 0)

  CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);


  ping_pong_set_callback(&do_work);
  setup_ping_pong();

  unsigned int counter = 0;
  CSR_WRITE(GPIO_WRITE, 0xdeadbeef);


  // ring_register_callback(&cs11_test_slicer_callback, CS11_TEST_SLICER_CMD);
  ring_register_callback(&magnitude_adjustment_callback, MAGADJUST_CMD);
  ring_register_callback(&set_gain_callback, RX_SET_GAIN_CMD);
  ring_register_callback(&corrupt_dma_callback, CORRUPT_DMA_OUT_CMD);
  ring_register_callback(&check_bootload_status, CHECK_BOOTLOAD_CMD);
  // ring_register_callback(&trigger_exfil_callback, TRIGGER_EXFIL_CMD);
  ring_register_callback(&set_mag_filter_callback, MAG_FILTER_GAIN_CMD);
  ring_register_callback(&set_phase_correction_mode_callback, RX_PHASE_CORRECTION_CMD);

  Ringbus ringbus;

  while(1) {

    // pet_exfil();

    execute_ping_pong();

    if(counter == 100) {  // please use 100 as the ringbus interval
      check_ring(&ringbus);
      // unsigned int mem_free = vmalloc_available(&mgr);
      // ring_block_send_eth(0xc0000000 | mem_free);
      ring_flag = 1;
      counter = 0;
    }
    else
    {
        ring_flag = 0;
    }

    counter++;
    // ring_block_send_eth(counter);
  }
  return 0;
}
