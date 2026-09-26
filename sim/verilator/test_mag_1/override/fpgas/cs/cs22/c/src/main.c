#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "vmalloc.h"
#include "circular_buffer.h"
#include "fill.h"
#include "ringbus.h"
#include "coarse_sync.h"
#include "atan.h"
#include "xvcordic.h"

#include "flush_config_word_data.h"
#include "fft_1024_3914.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "check_bootload.h"

#include "cs21test_buffer.h"
#include "corrupt_dma.h"
#include "fast_inv_sqrt.h"
#include "nco_data.h"
#include "vmem_copy.h"
#include "fixed_iir.h"

#define EXFIL_SIZE (1024)
#define EXFIL_CHUNKS (44)
#define EXFIL_RINGBUS_TOLERANCE (2)
#define EXFIL_COUNTER_DELAY (0x410000)
#include "data_exfil.h"



#define NORMAL_OPERATION

#ifdef NORMAL_OPERATION





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

float rx_gain = 8640.0*256.0; //18000.0;


unsigned int feedback_data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

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

//     MVXV_KNOP(V12, VMEM_ADDRESS(bank_address_pilot_common_phase));
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
//     MVXV_KNOP(V12, VMEM_ADDRESS(permutation_memcopy));
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
//     MVXV_KNOP(V12, VMEM_ADDRESS(permutation_memcopy));
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

//     MVXV_KNOP(V13, VMEM_ADDRESS(bank_address_pilot_conjmul_0));
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

//     MVXV_KNOP(V12, VMEM_ADDRESS(permutation_pilot_conjmul_add));
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
//     MVXV_KNOP(V0, VMEM_ADDRESS(config_word_add_eq_00));
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
    const unsigned int force_zero_angle )
{


   // xbb_pilot_add_common_phase(VMEM_ADDRESS(config_word_add_rx4_03), input_data_location, VMEM_ADDRESS(output_common_phase));
   // unsigned int pilot_common_phase = vector_memory[VMEM_ADDRESS(output_common_phase)*16];

   // STALL(50);

    unsigned int one_pilot_observe = vector_memory[input_data_location*16+2];
    unsigned int second_pilot_observe = vector_memory[input_data_location*16+4];

    STALL(50); 


    //////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////

   
    unsigned int temp_common_angle;

    ATAN(temp_common_angle,one_pilot_observe,15);
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
    if( force_zero_angle ) {
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

   xbb_conj_multi(VMEM_ADDRESS(config_word_cmul_eq_0f), input_data_location, VMEM_ADDRESS(nco_data_phase_mask), temp_data_location);


   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   feedback_data[0] = vector_memory[input_data_location*16+1024];
   feedback_data[1] = vector_memory[input_data_location*16+1025];

   STALL(50);


   // SET_REG(x3,0xeeeeeeee);
   // SET_REG(x3,0);
   // SET_REG(x3, nco_data_phase_mask[0]);
   // SET_REG(x3, nco_data_phase_mask[1]);
   // SET_REG(x3, nco_data_phase_mask[2]);
   // SET_REG(x3, nco_data_phase_mask[3]);

   // first 2 are for the first transmitter

   vector_memory[output_data_location*16+1024] = feedback_data[0];
   // vector_memory[output_data_location*16+1025] = pilot_common_phase;

   vector_memory[output_data_location*16+1025] = one_pilot_observe;

   // next 2 are for the second transmitter
   vector_memory[output_data_location*16+1026] = feedback_data[1];
   vector_memory[output_data_location*16+1027] = second_pilot_observe;
   STALL(50);

}

//////////////////////////////////////////////////////////



#define MY_ASSERT(x) if(!(x)) { ring_block_send_eth(0xe0000000|__LINE__);}


void fft_accept_new(unsigned int dma_ptr);
void dma_out_set_safe(unsigned int dma_ptr, unsigned int size);

// declare as global
VMalloc mgr;


#define DMA_OUT_CHUNK_ATTACHMENT 16

#define DMA_IN_CHUNK (1024+DMA_OUT_CHUNK_ATTACHMENT)

#define DMA_OUT_CHUNK (1024+DMA_OUT_CHUNK_ATTACHMENT)

// must be power of two, must change next as well
#define DMA_IN_COUNT (4)
#define DMA_IN_COUNT_MASK 0x3


// 0 a
// 1 b
int dma_state = 0;
int dma_in_valid = -1;
unsigned int dma_in_ptr[2];
int fft_a_empty;
int fft_b_empty;

int fft_ready = -1;
int fft_valid = -1;
unsigned int fft_ptr[2];


int dma_out_valid = -1;
int dma_out_ready = -1;

#define DMA_OUT_CIRBUF_SIZE (4)
circular_buf_t dma_out_started;
unsigned int dma_out_started_storage[DMA_OUT_CIRBUF_SIZE+1];

// 
void trig_dma_in(unsigned int idx, unsigned int timer_start) {
  // dma_in_set(VMEM_DMA_ADDRESS(dma_in_ptr[idx]), DMA_IN_CHUNK);

  // static unsigned int timer_start = 4096;

  CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx]));
  CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK);
  CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
  CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value

  // timer_start += 4096;
}

VMEM_SECTION unsigned int dma_buffer_a[1024+256];
VMEM_SECTION unsigned int dma_buffer_b[1024+256];

VMEM_SECTION unsigned int out_buffer_a[1024+16];
VMEM_SECTION unsigned int out_buffer_b[1024+16];



void setup_dma_in(void) {
  dma_in_ptr[0] = (unsigned int) dma_buffer_a;
  dma_in_ptr[1] = (unsigned int) dma_buffer_b;


  trig_dma_in(0, 0xffffffff);
  trig_dma_in(1, 0xffffffff);
}

fft1024_t active_plan;

void setup_fft(void) {
  // fft_ptr[0] = vmalloc_single(&mgr);
  // fft_ptr[1] = vmalloc_single(&mgr);
  fft_ptr[0] = (unsigned int) out_buffer_a;
  fft_ptr[1] = (unsigned int) out_buffer_b;


  fft_a_empty = 1;
  fft_b_empty = 1;

  active_plan = get_fft1024_plan(0, 0);
}

void setup_dma_out(void) {

  dma_out_started.size = DMA_OUT_CIRBUF_SIZE+1;
  dma_out_started.buffer = dma_out_started_storage;
  circular_buf_reset(&dma_out_started);
}


unsigned test_data_progress = 0;
unsigned test_data_requested = 0;

void cs11_test_slicer_callback(unsigned int data) {
    test_data_progress = 0;
    test_data_requested = data;
}

void recover_last() {
    ring_block_send_eth(DMA_LAST_ERROR_PCCMD | OUR_RING_ENUM);
    dma_run_till_last();
}

void pet_dma_in(void) {
  unsigned int occupancy;
  int error;
  unsigned int data;
  unsigned int helper;
  unsigned int set_pace = 0;
  unsigned int status;
  static unsigned int pace;

  CSR_READ(mip, helper);

  if(helper & DMA_0_ENABLE_BIT) {
    CSR_READ(DMA_0_STATUS, status);
    CSR_WRITE(DMA_0_INTERRUPT_CLEAR, 0);
    CSR_WRITE(GPIO_WRITE, (1<<8) | dma_state);

    if( status ) {
        recover_last();
    }

    dma_in_valid = dma_state; // signal a buffer index downstream

    dma_state = (dma_state+1)&0x1;
  }

  if(fft_ready != -1) {
    // if(set_pace == 0) {
    //   CSR_READ(TIMER_VALUE, pace);
    //   pace += 8192; // in the future with a slightly shorter buffer
    //   set_pace = 1;
    // } else {
    //   pace += 8192;
    // }

    trig_dma_in(fft_ready, 0xffffffff); // pace
    CSR_WRITE(GPIO_WRITE, (2<<8) | fft_ready);
    fft_ready = -1;

  }
}





void pre_pet_fft() {
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

    CSR_WRITE(GPIO_WRITE, (0x4a << 8) | dma_out_ready);
    CSR_WRITE(GPIO_WRITE, (0x4b << 8) | (fft_a_empty << 1) | fft_b_empty);

    dma_out_ready = -1;
  }
}

// 0 is disabled
// 1 is 1:1
// 2 is 2:1 but not written yet

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
// int32_t mag_filter_gain = 65536 * (0.07);
int32_t mag_filter_gain = 65536 * (0.01525);       // Default of soft filter  (about 1000 in decimal)
// int32_t mag_filter_gain = 0;

// const unsigned use_filter = 0;
// mag_filter_gain is [0 - 65535]

void pet_fft() {

  unsigned int read_clk_1, read_clk_2;

  CSR_READ(TIMER_VALUE, read_clk_1); 
  
  // example only starts to work once we have 2 items in the queue

  int error;
  static unsigned updates = 1;

  // deals with dma telling us we are done
  pre_pet_fft();

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
        return; // early
      }
      fft_a_empty = 0;
    }
    if(dma_in_valid == 1) {
      if( fft_b_empty == 0) {
        return; // early
      }
      fft_b_empty = 0;
    }

    CSR_WRITE(GPIO_WRITE, (3 << 8) | (fft_a_empty << 1) | fft_b_empty);

    unsigned int* cpu_ptr_from_dma = (unsigned int*) dma_in_ptr[consume_idx];
    unsigned int* cpu_ptr_fft      = (unsigned int*) fft_ptr[consume_idx];

    if( exfil_active() ) {
        unsigned int exfil_row = exfil_working_on_row();
        SET_REG(x3, 0xffff0000);
        SET_REG(x3, exfil_row);
        SET_REG(x3, 0x0);
        vmem_copy_rows(VMEM_ROW_ADDRESS(cpu_ptr_from_dma), exfil_row, 64 );
        exfil_wrote_row();
    }


    // active_plan.data_location   = VMEM_ROW_ADDRESS(cpu_ptr_from_dma);  //input
    // active_plan.data_location_0 = VMEM_ROW_ADDRESS(cpu_ptr_fft);       //output 

    // CSR_WRITE(GPIO_WRITE, 0x30);

    //fft_1024_run(&active_plan);

    // set to 1 to disable phase rotation
    // this flag is backwards, so we use this logic to set true when more is 0
    // and false in other modes
    unsigned int disable_phase_correction = (phase_correction_mode==0);

    xbb_residue_phase(
        VMEM_ROW_ADDRESS(cpu_ptr_from_dma),
        VMEM_ROW_ADDRESS(temp_space),
        VMEM_ROW_ADDRESS(cpu_ptr_fft),
        disable_phase_correction);
    

    // for verilator only do not set this in hardware or mag will be stuck and affect other stuff
    // mag_adjust_flag = 1;
    
    if(mag_adjust_flag == 0) {
        // fixme replace with vmem_copy_rows (assuming V0,V1,V2 are not needed later)
        ///////////////////////////////////////////////////
        /////just memory copy
        MVXV_KNOP(V0, VMEM_ROW_ADDRESS(temp_space));
        MVXV_KNOP(V1, VMEM_ROW_ADDRESS(cpu_ptr_fft));
        MVXV_KNOP(V2, 1);

		// previously this was 65 to include the "trunk"
        // now 64 because trunk data was already copied in xbb_residue_phase
        for(int index=0; index<64; index++) {
            ADD_LK13(V0, V0, V2, 0);
            ADD_SK13(V1, V1, V2, 0);
        }
    } else if(mag_adjust_flag <= 2) {
        /// adjust qam coefficient round robin each time for one tone
        /// mag_counter starts at 2, and is added by 4 until it wraps
        // int start_clk, end_clk;
        // CSR_READ(TIMER_VALUE, start_clk);
        unsigned int channel_data, channel_data2;
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
        channel_data = vector_memory[VMEM_ADDRESS(temp_space)*16+mag_counter];

        SET_REG(x4, frames_parsed);
        SET_REG(x3, channel_data);


        /// If we are in "beamforming" magnitude mode
        /// we add two pilots together, and use the result to calculate our 1/x
        /// we need to calculate the next counter
        if( mag_adjust_flag == 2 ) {
            // grab the next tone
            mag_counter2 = mag_counter + 4;
            if( mag_counter2 >= 1024 ) {
                // if the next tone would have gone beyond edge
                // just add the same tone twice
                // this hack shouldn't affect our data as this is only on the edge subcarrier
                mag_counter2 = mag_counter;
            }

            channel_data2 = vector_memory[VMEM_ADDRESS(temp_space)*16+mag_counter2];
        }

        float mag_squared;
        int64_t re,im;

        if( mag_adjust_flag == 1 ) {
            // put the real and imaginary parts in 64 bits, so we won't overflow when we calculate the mag^2
            // we cast them to int16_t so that the compiler will sign extend for us
            // this is usually the fastest way to sign extend
            re =  (int16_t)(channel_data & 0xffff);
            im =  (int16_t)((channel_data>>16) & 0xffff);
        } else {
            // Here we are taking 1/(mag(v1+v2)) however this may cause an error
            // due to the fact that the new eq method does not adjust the eq for the pilot tones so we can track STO
            // Because of this we are suming two non zero degree eq vectors.  The rotations for the data tones are indeed
            // zero degrees, so when the data tones sum, they will have more magnitude than we calculate here
            re =  (int16_t)(channel_data & 0xffff);
            im =  (int16_t)((channel_data>>16) & 0xffff);

            re += (int16_t)(channel_data2 & 0xffff);
            im += (int16_t)((channel_data2>>16) & 0xffff);
        }



        // non zero enables the filter to run
        if( mag_filter_gain != 0 ) {

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




        // calculate magnitude squared
        mag_squared = (re*re)+(im*im);

        // calculate the 1/sqrt() of our magnitude_squared.
        // we multiple the result by rx_gain
        // this method is not sensative to rotation of the pilot tones
        unsigned int new_gain = rx_gain * fast_inv_sqrt(mag_squared);

        // old method, sensative to rotation
        // unsigned int new_gain = (unsigned int)(rx_gain/(channel_data & 0xffff));

        if(new_gain > 0x7fff) {
            new_gain = 0x7fff;
        }

        vector_memory[VMEM_ADDRESS(qam_adj_coeff)*16+mag_counter-1] = new_gain;
        vector_memory[VMEM_ADDRESS(qam_adj_coeff)*16+mag_counter+1] = new_gain;

        STALL(30);

        mag_counter += 4;
        if(mag_counter == 1026) {
            mag_counter = 2;
        }

        // CSR_READ(TIMER_VALUE, end_clk);
        // ring_block_send_eth(end_clk - start_clk);
        // ring_block_send_eth(channel_data);
        // ring_block_send_eth(new_gain);

        // ////////////////////////////////////////////////////////////////
        // just qam adjustment
        xbb_conj_multi(
            VMEM_ADDRESS(config_word_cmul_eq_08),
            VMEM_ROW_ADDRESS(temp_space),
            VMEM_ADDRESS(qam_adj_coeff), 
            VMEM_ROW_ADDRESS(cpu_ptr_fft));

        CSR_READ(TIMER_VALUE, read_clk_2); // this will be 0x124  (4 clock cycles later)

        SET_REG(x3, 0x33333333);
        SET_REG(x3, (read_clk_2 - read_clk_1));

        // ring_block_send_eth(0x8e000000|(read_clk_2 - read_clk_1));

    }
    ///////////////////
    

    //////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////


    CSR_WRITE(GPIO_WRITE, (0x3a<<8) | consume_idx );

    // reset signal we consumed
    dma_in_valid = -1;
    // signal back, this releases our reliance on the input that dma gave us, meaning dma is free to erase it
    fft_ready = consume_idx; // Release A to be over written

    fft_valid = consume_idx; // send C on to be DMA'd out

    frames_parsed++;
  }

  


}



// similar to just calling dma_out_set
void dma_out_set_safe(unsigned int dma_ptr, unsigned int size) {

 
  unsigned int occupancy;
  // unsigned int occupancy_busy;
  // unsigned int occupancy_combined1, occupancy_combined2;
  while(1) {
    CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
    if(occupancy < DMA_1_SCHEDULE_DEPTH) {
      break; // should break on first go
    } else {
      // stuck, can report this with ringbus
      ring_block_send_eth(0xd0000000);
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
  if(fft_valid != -1) {
    occupancy = circular_buf_occupancy(&dma_out_started);
    CSR_WRITE(GPIO_WRITE, (7<<8) | occupancy );
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

    CSR_WRITE(GPIO_WRITE, (0x5<<8) | consume_idx);

    unsigned int* cpu_ptr_from_fft = (unsigned int*) fft_ptr[consume_idx];

    // For receive side, only run one output dma


    error = circular_buf_put(&dma_out_started, consume_idx | IS_SECOND_DMA); MY_ASSERT(error == 0);

    if(test_data_progress < test_data_requested) {
        unsigned offset = 0;

        offset = (test_data_progress & (8-1)) * DMA_OUT_CHUNK;

        unsigned int dma_pointer = VMEM_DMA_ADDRESS(random_vmem_with_tail)+offset;

        dma_out_set_safe(dma_pointer, DMA_OUT_CHUNK);

        test_data_progress++;

        if( test_data_progress == test_data_requested ) {
            test_data_progress = 0;
            test_data_requested = 0;
        }

    } else {
        // normal behavior
        // schedule the full FFT
        dma_out_set_safe(VMEM_DMA_ADDRESS(cpu_ptr_from_fft), DMA_OUT_CHUNK);
    }


    fft_valid = -1; // set this to -1 so we don't get caught
  }


  // handles when a dma is finished
  // we will get two interrupts for a given buffer because we scheduled twice (CP)
  // each time we pull a value from the cirbuf letting us know what just finished
  // if the value has the IS_SECOND_DMA set, then we know the buffer is not needed again
  // CSR_READ(mip, helper);
  CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, dma_occupancy);
  unsigned int filled = circular_buf_occupancy(&dma_out_started);

  if( dma_occupancy != filled ) {

    CSR_WRITE(DMA_1_INTERRUPT_CLEAR, 0);
    // using a cirbuf we remember which dma was put first
    error = circular_buf_get(&dma_out_started, &data); MY_ASSERT(error == 0);
    CSR_WRITE(GPIO_WRITE, (6<<8) | data );

    if(error == 0) {
      if( data & IS_SECOND_DMA ) { // always true, there is only one output dma
        // signal back to fft that output dma is done
        // only runs one for every 2 interrupts
        dma_out_ready = data & DMA_A_B_MASK; 

        pre_pet_fft();
      }
    } else {
      // oh boy
      // not exactly sure how we are here but we are checking extra times for dma out being done
      // we can avoid this with better magic math above
    }

  }

}

// void fine_sync_callback(unsigned int data) {
//     // data will be 1
//     // not 0x21000001
//     if( data == 1) 
//     {
//         sto_sfo_angle_flag = MA_SIZE;

//         for(int index = 0; index < MA_SIZE; index++)
//         {
//           sorted_array[index] = 0;    
//           data_array[index] = 0;

//         }

//         sto_sfo_angle_indi=0;
//         ring_block_send_eth(0xdeadbeef);

//     } 
// }

void magnitude_adjustment_callback(unsigned int data)
{
    if( data <= 2 ) {
        mag_adjust_flag = data;
    } else {
        mag_adjust_flag = 0;
    }
}

void set_gain_callback(unsigned int data) {
    rx_gain = (float)data*256.0;
}

void trigger_exfil_callback(unsigned int data) {
    if( data == 0 ) {
        SET_REG(x3, 0xf3034444);
        SET_REG(x3, 0x0);
        exfil_request();
    }
}




// Sets the gain for the rx pilot tone estimates
// set to 0 to disable any filtering of rx pilot tone estimates 
// the max value is 0x10000
// see pet_fft() about line 734
// 
// sets global mag_filter_gain  which has a range [0 - 65535]
void set_mag_filter_callback(unsigned int data) {
    uint32_t gain = data & 0x1ffff;
    uint32_t reset = (data & 0xf00000) >> 20;
    
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
void set_phase_correction_mode_callback(unsigned int data) {
    if(data == 1 || data == 2) {
        phase_correction_mode = data;
        return;
    }
    phase_correction_mode = 0;
}


int main(void) {

   // ring_register_callback(fine_sync_callback, SYNCHRONIZATION_CMD);



  
  
  init_VMalloc(&mgr);
  // unsigned int burn = 16;
  // for(unsigned int i = 0)

  CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);

  //xbb_coarse_sync(15);

  setup_dma_in();
  setup_fft();
  setup_dma_out();

  unsigned int counter = 0;
  CSR_WRITE(GPIO_WRITE, 0xdeadbeef);

  // ring_block_send_eth(0xdead);
  // ring_block_send_eth(VMEM_ROW_ADDRESS(dma_buffer_a));
  // ring_block_send_eth(VMEM_ROW_ADDRESS(dma_buffer_b));


  ring_register_callback(&cs11_test_slicer_callback, CS11_TEST_SLICER_CMD);
  ring_register_callback(&magnitude_adjustment_callback, MAGADJUST_CMD);
  ring_register_callback(&set_gain_callback, CS11_SET_GAIN_CMD);
  ring_register_callback(&corrupt_dma_callback, CORRUPT_DMA_OUT_CMD);
  ring_register_callback(&check_bootload_status, CHECK_BOOTLOAD_CMD);
  ring_register_callback(&trigger_exfil_callback, TRIGGER_EXFIL_CMD);
  ring_register_callback(&set_mag_filter_callback, MAG_FILTER_GAIN_CMD);
  ring_register_callback(&set_phase_correction_mode_callback, RX_PHASE_CORRECTION_CMD);

  // ring_block_send_eth(dma_in_ptr[0]);
  // ring_block_send_eth(dma_in_ptr[1]);
  // ring_block_send_eth(fft_ptr[0]);
  // ring_block_send_eth(fft_ptr[1]);

  Ringbus ringbus;

  while(1) {
    pet_dma_in();
    pet_fft();
    pet_dma_out();
    pet_dma_out();
    pet_dma_out();
    pet_dma_out();


    pet_exfil();

    

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

}












#else
#endif
