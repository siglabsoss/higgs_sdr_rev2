#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS30
#include "ringbus2_post.h"

#include "unit_test_ring.h"




int main(void)
{
  ring_unit_test_as(OUR_RING_ENUM);

  // unsigned char ttl = 0;
  // unsigned int data = 0x12345678; 


  // check_dma = 0xf0f0f0f0;

  // CSR_WRITE(RINGBUS_WRITE_ADDR, ttl);
  // CSR_WRITE(RINGBUS_WRITE_DATA, data);
  // CSR_WRITE(RINGBUS_WRITE_EN, 0);

  // check_dma = 0x0a0a0a0a;


}
