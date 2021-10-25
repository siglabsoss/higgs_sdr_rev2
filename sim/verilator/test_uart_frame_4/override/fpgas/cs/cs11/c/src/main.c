#include "csr_control.h"
#include "uart_driver.h"
#include "vmem.h"
#include "random.h"
#include "stall2.h"
#include <stdint.h>
#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "tb_debug.h"
#include <stdarg.h>
#include <string.h>

VMEM_SECTION unsigned int vmem1 [1];


void between1() {
    STALL(450);
}

void between2() {
}

// void simple_printf(const char* fmt, ...)
// {
//     va_list args;
//     va_start(args, fmt);

//     int runs = 0;
//     // SET_REG(x3, 0xffffffff);
//     // SET_REG(x3, runs);
 
//     while (*fmt != '\0') {
//         SET_REG(x3, 0xffffffff);
//         SET_REG(x3, runs);
//         if (*fmt == 'd') {
//             int i = va_arg(args, int);
//             // printf("%d\n", i);
//         } else if (*fmt == 'c') {
//             // A 'char' variable will be promoted to 'int'
//             // A character literal in C is already 'int' by itself
//             int c = va_arg(args, int);
//             // printf("%c\n", c);
//         } else if (*fmt == 'f') {
//             double d = va_arg(args, double);
//             // printf("%f\n", d);
//         }
//         ++fmt;
//         runs++;
//     }
 
//     va_end(args);
// }


// typedef struct __attribute__((__packed__)) hoist_t {
//     uint8_t* format; 
//     unsigned length;
//     void *saved[32];
// } hoist_t;


// void _printf(const char* fmt, ...)
// {
//     va_list args;
//     va_start(args, fmt);

//     int runs = 0;

//     hoist_t hoist;
//     hoist.format = (uint8_t*)fmt;
//     hoist.length = 0;

//     unsigned intgr = 0;
 
//     while (*fmt != '\0') {
//         SET_REG(x3, 0xffffffff);
//         SET_REG(x3, runs);
//         switch(*fmt) {
//             case '%':
//                 intgr = va_arg(args, unsigned);
//                 SET_REG(x4, intgr);
//                 hoist.saved[hoist.length] = intgr;
//                 hoist.length++;
//                 break;
//         }
//         ++fmt;
//         ++runs;

//         if( hoist.length == 32 ) {
//             break;
//         }
//     }

//     dump_imem_bytes_printf(&hoist, sizeof(hoist));
//     block_until_dump_done();
 
//     va_end(args);
// }




int main(void)
{
    setup_debug();

    STALL(40);

    vmem1[0] = 0xdead;

    // dump_vmem_cpu(3, vmem1, 1);

    const char msg[] = "hello world\n";

    // dump_imem_bytes(3, msg, strlen(msg));

    // block_until_dump_done();

    // _printf("%s", msg);


    char *s;
    char *s2;
    char *s3;
    char *s4;
    char c;
    int i;
    int b;

    c = 'p';

    i = -123455;

    s =  "Lorem1 ipsum.\n";
    s2 = "Lorem2 ipsum.\n";
    s3 = "Lorem3 ipsum.\n";
    s4 = "Lorem4 ipsum.\n";

    // _printf("fasdf\n %s\n %c\n %s\n %s\n %s\n %i\n", s, c, s2, s3, s4, i);
    // _printf("fasdf%s%c%s%s%s%iX\n%c", s, c, s2, s3, s4, i, 'a');


    // int number = 19912507;
    // _printf("%i, %d\n", number, number);

    int max;
    int min;

    min = -2147483648;
    max = 2147483647;
    _printf("%d, %d\n", INT32_MAX, INT32_MIN);
    _printf("%i, %i\n", max, min);

    _printf("%x\n", 45);
    _printf("%x\n", 4512);
    _printf("%X\n", 45);
    _printf("%X\n", 4512);
    _printf("hi\n");
    _printf("hi%d\n", 42);
    _printf("hi%dx\n", 43);


    return 0;
}
