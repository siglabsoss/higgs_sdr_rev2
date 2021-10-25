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



typedef struct __attribute__((__packed__)) mystruct {
    char a;
    uint8_t b[3];
    char c;
    uint8_t d[7];
} mystruct;

// uint8_t chars1[] = {1,2,3,4};

uint8_t chars2[] = {20};
uint8_t chars6[] = {0x30,0x31,0x32,0x33,0x34};

int main(void)
{
    setup_debug();

    STALL(40);

    uint8_t chars3[] = {21,22};
    uint8_t chars4[] = {23,24,25};
    uint8_t chars5[] = {26,27,28,29};


    mystruct foo;
    foo.a = 'a';
    foo.b[0] = 0x40;
    foo.b[1] = 0x41;
    foo.b[2] = 0x42;
    foo.c = 'c';

    for(int i = 0; i < 7; i++) {
        foo.d[i] = 0x50+i;
    }


    dump_imem_bytes(2, chars2, 1);
    block_until_dump_done();
    dump_imem_bytes(2, chars3, 2);
    block_until_dump_done();
    dump_imem_bytes(2, chars4, 3);
    block_until_dump_done();
    dump_imem_bytes(2, chars5, 4);
    block_until_dump_done();
    dump_imem_bytes(2, chars6, 5);
    block_until_dump_done();

    dump_imem_bytes(2, foo.b, 3);
    block_until_dump_done();

    dump_imem_bytes(2, foo.d, 7);
    block_until_dump_done();

    SET_REG(x3, chars2[0]);
    SET_REG(x3, chars3[1]);
    // dump_imem_bytes(2, chars1, 4);
    // block_until_dump_done();

    return 0;
}
