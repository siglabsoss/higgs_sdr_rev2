#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "unit_test_ring.h"
 
volatile unsigned int d;
volatile unsigned int e; 



int main(void)
{
    for(unsigned int i = 0; i < 7000; i++) {
        STALL(1);
    }
  ring_block_send_eth(0xdead);
  unsigned int a, b, result;
  e = 120;
  d = 141324;

  
  CSR_READ(TIMER_VALUE, a);
  result =  d / e;
  CSR_READ(TIMER_VALUE, b);


  ring_block_send_eth(b-a);
  ring_block_send_eth(result);

  CSR_READ(TIMER_VALUE, a);
  result =  d * e;
  CSR_READ(TIMER_VALUE, b);

  ring_block_send_eth(b-a);
  ring_block_send_eth(result);

}
