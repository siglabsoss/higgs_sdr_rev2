#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "unit_test_ring.h"
#include "ring_assert.h"

void report_test(unsigned int testfails) {
  CSR_WRITE(RINGBUS_WRITE_ADDR, RING_ADDR_PC);
  CSR_WRITE(RINGBUS_WRITE_DATA, testfails);
  CSR_WRITE_ZERO(RINGBUS_WRITE_EN);
}


#define G GPIO_WRITE

unsigned int testfails = 0;

int main(void)
{
  unsigned int v,v0,v1,v2;
  register volatile unsigned int x3 asm("x3");
  register volatile unsigned int x4 asm("x4");

  CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);

  CSR_WRITE(G, 0x0);

  CSR_READ(G, v);

  ASSERT(v == 0);

  CSR_WRITE(G, 0xa0);


  int previous;
  int mask = 0xf00;

  CSR_READ_SET_BITS(G, mask, previous);

  ASSERT(previous == 0xa0);

  CSR_READ(G, v);
  ASSERT(v == 0xfa0);

  mask = 0xf000;
  CSR_SET_BITS(G, mask);

  CSR_READ(G, v);
  ASSERT(v == 0xffa0);

  mask = 0x2200;
  CSR_READ_CLEAR_BITS(G, mask, previous);
  
  ASSERT(previous == 0xffa0);

  CSR_READ(G, v);
  ASSERT(v == 0xDDa0);

  CSR_READ_CLEAR_BITS(G, 0x0, previous);

  ASSERT(previous == 0xDDa0);

  CSR_CLEAR_BITS(G, 0xFFFF);
  CSR_READ(G, v);
  ASSERT(v == 0x0);


  x3 = previous;

  report_test(testfails);

}
