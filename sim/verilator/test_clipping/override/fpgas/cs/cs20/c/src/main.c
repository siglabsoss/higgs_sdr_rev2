#include "csr_control.h"

register volatile unsigned int check_dma asm("a7");
void handle_single_ring_in(void) {
    CSR_WRITE(RINGBUS_INTERRUPT_CLEAR, 0);
    int a;
    CSR_READ(RINGBUS_READ_DATA, a);

  if( a == 0xdeadbeef )
  {
    CSR_WRITE(GPIO_WRITE,1);
    check_dma = a;
    for(int i = 0; i < 1000000; i++) 
    {
        asm("nop");
    }
    CSR_WRITE(GPIO_WRITE,0);
    check_dma = a + 1;
    for(int i = 0; i < 1000000; i++) 
    {
        asm("nop");
    }
  }
}

int main(void)
{
    CSR_WRITE(GPIO_WRITE_EN,1);
    check_dma = 0xdeadbeef;
    int h1;
    while(1) {
        CSR_READ(mip, h1);
        check_dma = 0xdeadcafe;
        if(h1 & RINGBUS_ENABLE_BIT) {  
            handle_single_ring_in();
        }


    }
}