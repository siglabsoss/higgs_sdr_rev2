#include "xbaseband.h"
#include "vmem.h"
#include "csr_control.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS20
#include "ringbus2_post.h"

VMEM_SECTION unsigned int vmem_load0[32] = {
0xE00 | OUR_RING_ENUM ,
0xE10 | OUR_RING_ENUM ,
0xE20 | OUR_RING_ENUM ,
0xE30 | OUR_RING_ENUM ,
0xE40 | OUR_RING_ENUM ,
0xE50 | OUR_RING_ENUM ,
0xE60 | OUR_RING_ENUM ,
0xE70 | OUR_RING_ENUM ,
0xE80 | OUR_RING_ENUM ,
0xE90 | OUR_RING_ENUM ,
0xEA0 | OUR_RING_ENUM ,
0xEB0 | OUR_RING_ENUM ,
0xEC0 | OUR_RING_ENUM ,
0xED0 | OUR_RING_ENUM ,
0xEE0 | OUR_RING_ENUM ,
0xEF0 | OUR_RING_ENUM ,
0xF00 | OUR_RING_ENUM ,
0xF10 | OUR_RING_ENUM ,
0xF20 | OUR_RING_ENUM ,
0xF30 | OUR_RING_ENUM ,
0xF40 | OUR_RING_ENUM ,
0xF50 | OUR_RING_ENUM ,
0xF60 | OUR_RING_ENUM ,
0xF70 | OUR_RING_ENUM ,
0xF80 | OUR_RING_ENUM ,
0xF90 | OUR_RING_ENUM ,
0xFA0 | OUR_RING_ENUM ,
0xFB0 | OUR_RING_ENUM ,
0xFC0 | OUR_RING_ENUM ,
0xFD0 | OUR_RING_ENUM ,
0xFE0 | OUR_RING_ENUM ,
0xFF0 | OUR_RING_ENUM 
};

void block_send_ring(unsigned int ttl, unsigned int data){
  register volatile unsigned int x3 asm("x3");
  register volatile unsigned int x4 asm("x4");
  unsigned int occupancy;
  
  CSR_READ(RINGBUS_SCHEDULE_OCCUPANCY, occupancy);
  while( occupancy >= RINGBUS_SCHEDULE_DEPTH ) {
    CSR_READ(RINGBUS_SCHEDULE_OCCUPANCY, occupancy);
  }

  CSR_WRITE(RINGBUS_WRITE_ADDR, ttl);
  CSR_WRITE(RINGBUS_WRITE_DATA, data);
  CSR_WRITE(RINGBUS_WRITE_EN, 0);
}

int main(void)
{
  // let eth boot
  for(unsigned int j = 0; j < 8000; j++) {
    asm("nop");
  }
  for(unsigned int i = 0; i < 32; i++) {
    block_send_ring(RING_ADDR_PC, vmem_load0[i]);
  }
}
