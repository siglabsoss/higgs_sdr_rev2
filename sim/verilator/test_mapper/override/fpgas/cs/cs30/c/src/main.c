#include "xbaseband.h"
#include "apb_bus.h"
#include "mapper.h"
#include "table.h"
#include "sig_utils.h"

#define OUTPUT_BUF_START (1024)
#define INPUT_BUF_START (0)
#define DMA_LEN  (1024)
#define MEM_TOP   (1024*63)
#define MASK_CMD (0x04000000)
#define POS_ONE (0x7FFF) //largest pos number
#define NEG_ONE (0x8001) //2's compliment of POS_ONE

#define DEST_ADDR        (32)
#define SOURCE_ADDR      (0)
#define SCHEDULE_ADDR    (64)         
#define TABLE_START_ADDR (1024*(64-1))
#define SRC_POINTER_REG    (0x3)
#define DEST_POINTER_REG   (0x4)
#define PERM_REG           (0x9)
#define TRUE               (0x1)
#define FALSE              (0x0)
#define GARBAGE            ((1024-1))

int main(void) 
{


	vector_memory[0] = 0xAAAAAAAA;
	vector_memory[1] = 0x7;
	vector_memory[2] = 0xF;
	vector_memory[3] = 0xF0000000;

	Table qpsk_table = mapper_qpsk_table(2048*NSLICES);
	Table bpsk_table = mapper_bpsk_table(3000*NSLICES);

	mapper_load_qpsk(&qpsk_table, 1024*NSLICES, 0x1);
	mapper_rload_bpsk(&bpsk_table, 1025*NSLICES, 0x1);
	mapper_lload_bpsk(&bpsk_table, 1026*NSLICES, 0x1);
	mapper_load_qpsk(&qpsk_table, 1027*NSLICES, 0x2);
	mapper_lload_bpsk(&bpsk_table, 1028*NSLICES, 0x3);
	mapper_rload_bpsk(&bpsk_table, 1029*NSLICES, 0x3);

}

