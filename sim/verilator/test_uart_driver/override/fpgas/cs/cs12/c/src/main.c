#include "csr_control.h"
#include "uart_driver.h"

int main(void)
{
    SET_REG(x3, 0x12000000);
    volatile unsigned int *clock_divider = (unsigned int *) 0xF0000008;
    volatile unsigned int *occupancy = (unsigned *) 0xF0000004;

    STALL(10);
    *clock_divider = 0;
    uart_put_char_interrupt(0xAA);
    uart_put_char_interrupt(0x55);
    uart_put_char_interrupt(0x12);
    SET_REG(x3, *occupancy);

    return 0;
}