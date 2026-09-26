#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "dma.h"
#include "fill.h"
#include "mover.h"
#include "mapper.h"
#include "ringbus.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS00
#include "ringbus2_post.h"

// #define ETH_RING_ENUM      (6)
#define DST_ADDR           (0x12)
#define SRC_ADDR           (0x1)

#define DMA_OUT_SIZE 1280

#include "config_word_cmul_eq_0f.h"
#include "coarse_sync.h"

void main()
{
	STALL(10);
}