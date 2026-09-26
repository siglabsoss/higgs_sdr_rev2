#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"



int main(void)
{
    for(unsigned int i = 0; i < 7000; i++) {
        STALL(1);
    }
  

  while(1) {
    STALL(1);
  }

}
