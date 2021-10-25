#include "csr_control.h"
#include "uart_driver.h"
#include "vmem.h"
#include "tb_debug.h"

// old structure
// void gen_vmem_uart_frame(unsigned int tag, unsigned int addr, unsigned int length) {
//     uart_put_char(3);
//     uart_put_char(11);
//     uart_put_char(tag);
//     uart_put_char(addr & 0xFF);
//     uart_put_char((addr >> 8) & 0xFF);
//     uart_put_char((addr >> 16) & 0xFF);
//     uart_put_char((addr >> 24) & 0xFF);
//     uart_put_char(length & 0xFF);
//     uart_put_char((length >> 8) & 0xFF);
//     uart_put_char((length >> 16) & 0xFF);
//     uart_put_char((length >> 24) & 0xFF);
// }

int main(void)
{
    SET_REG(x3, 0x01000000);
    // volatile unsigned int *clock_divider = (unsigned int *) 0xF0000008;
    volatile unsigned int *occupancy = (unsigned *) 0xF0000004;
    for (unsigned int i = 0; i < 1024; i++) {
        vector_memory[i] = i;
    }
    
    STALL(10);
    *uart_clock_divider = 0;
    //uart_put_char_interrupt(0xA);
    //uart_put_char_interrupt(0xB);
    //uart_put_char_interrupt(0xC);
    // gen_vmem_uart_frame(2,4,10);
    dump_vmem_dma(2,400,10);
    
    SET_REG(x3, *occupancy);
    
    return 0;
}
