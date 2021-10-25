#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS20
#include "ringbus2_post.h"




void hr() {
  register volatile unsigned int x3 asm("x3");
  register volatile unsigned int x4 asm("x4");

  CSR_WRITE(RINGBUS_INTERRUPT_CLEAR, 0);
  int a;
  CSR_READ(RINGBUS_READ_DATA, x3);

}


void d1(unsigned int ttl, unsigned int data) {
  int occupancy;
  // unsigned int ttl,data;
  CSR_READ(RINGBUS_SCHEDULE_OCCUPANCY, occupancy);
  if( occupancy < RINGBUS_SCHEDULE_DEPTH ) {

  CSR_WRITE(RINGBUS_WRITE_ADDR, ttl);
  CSR_WRITE(RINGBUS_WRITE_DATA, data);
  CSR_WRITE(RINGBUS_WRITE_EN, 0);
    
  }
}

// int main(void)
// {
//   d1(0, 0xdeadbeef);
//   d1(0, 0x12345678);
// }

int main(void)
{
  register volatile unsigned int x3 asm("x3");
  register volatile unsigned int x4 asm("x4");

  x4 = 0;
  int h1;
  while(1) {
    CSR_READ(mip, h1);

    if(h1 & RINGBUS_ENABLE_BIT) {
      hr();
      x4++;
    }

  }
}
