#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "ringbus.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS30
#include "ringbus2_post.h"





int main(void)
{
  Ringbus ringbus;

  ringbus.addr = 0;
  ringbus.data = 0xdead;

  send_cmd(&ringbus);

  for(unsigned int i = 0; i < 1000; i++) {
  	STALL(5);
  }

  ringbus.addr = 1;

  send_cmd(&ringbus);

}
