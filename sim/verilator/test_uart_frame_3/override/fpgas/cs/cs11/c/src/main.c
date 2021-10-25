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


VMEM_SECTION unsigned int vmem0 [1] = {
0xffe0
};

VMEM_SECTION unsigned int vmem1 [1] = {
0xdeadf00d
};

VMEM_SECTION unsigned int vmem2 [2] = {
0xffffffff,
0xeeeeeeee
};


unsigned int imem0 [4] = {
0xffaaee0d,
0xffaaee1d,
0xffaaee2d,
0xffaaee3d};

typedef struct __attribute__((__packed__)) mystruct {
    unsigned a;
    char b;
    unsigned c;
    unsigned d;
    char e;
    unsigned f;
    char g;
    unsigned h;
    char i;
    unsigned j;
    char k;
    unsigned l;
} mystruct;


void between1(void) {
    STALL(450);
}

void between2(void) {
    block_until_dump_done();
}


int main(void)
{
    setup_debug();

    STALL(40);

    dump_vmem_dma(1, VMEM_DMA_ADDRESS(vmem0), 1);
    block_until_dump_done();
    dump_vmem_dma(1, VMEM_DMA_ADDRESS(vmem1), 1);
    block_until_dump_done();
    dump_vmem_dma(1, VMEM_DMA_ADDRESS(vmem2), 2);
    block_until_dump_done();
    dump_vmem_dma(1, VMEM_DMA_ADDRESS(vmem2)+1, 2);
    block_until_dump_done();

    dump_vmem_cpu(1, vmem2, 2);
    block_until_dump_done();

    dump_vmem_row(1, VMEM_ROW_ADDRESS(vmem2), 2);
    block_until_dump_done();
    // between2();

    // uart_put_char(0);
    // uart_put_char(0);

    dump_imem_words(2, imem0, 4);
    block_until_dump_done();

    mystruct foo;
    foo.a = 0xfeed0000;
    foo.b = 'a';
    foo.c = 0x12345678;
    foo.d = 0xff00ee00;
    foo.e = 'e';
    foo.f = 0x22334455;
    foo.g = 'g';
    foo.h = 0x22334455;
    foo.i = 'i';
    foo.j = 0x22334455;
    foo.k = 'k';
    foo.l = 0x22334455;

    dump_imem_words(2, &foo.a, 1);
    block_until_dump_done();
    dump_imem_words(2, &foo.c, 1);
    block_until_dump_done();
    dump_imem_words(2, &foo.c, 2);
    block_until_dump_done();
    dump_imem_words(2, &foo.f, 1);
    block_until_dump_done();
    dump_imem_words(2, &foo.h, 1);
    block_until_dump_done();
    dump_imem_words(2, &foo.j, 1);
    block_until_dump_done();
    dump_imem_words(2, &foo.l, 1);
    block_until_dump_done();


    return 0;
}
