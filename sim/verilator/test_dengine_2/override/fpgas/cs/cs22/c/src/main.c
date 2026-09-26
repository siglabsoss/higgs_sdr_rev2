#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "circular_buffer.h"
#include "fill.h"
#include "ringbus.h"

#include "coarse_sync.h"
#include "atan.h"
#include "xvcordic.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "check_bootload.h"
#include "fast_inv_sqrt.h"
#include "nco_data.h"
#include "vmem_copy.h"
#include "fixed_iir.h"
#include "trunk_types.h"
#include "self_sync.h"


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


  Ringbus ringbus;

  while(1) {
      check_ring(&ringbus);
  }
  return 0;
}
