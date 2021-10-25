#include "csr_control.h"
#include "uart_driver.h"

int main(void)
{
    SET_REG(x3, 0x11000000);
    int error;
    volatile unsigned int *clock_divider = (unsigned int *) 0xF0000008;
    volatile unsigned int *frame = (unsigned int *) 0xF000000C;
    volatile unsigned int *write_cmd = (unsigned int *) 0xF0000000;
    volatile unsigned int *read = (unsigned int *) 0xF0000000;
    // [20:16] Write occupancy [28:24] Read occupancy
    volatile unsigned int *occupancy = (unsigned *) 0xF0000004;

    STALL(10);
    *clock_divider = 0;
    uart_put_char_interrupt(0xAA);
    uart_put_char_interrupt(0x55);
    uart_put_char_interrupt(0x12);
    SET_REG(x3, *occupancy);
    return 0;
}