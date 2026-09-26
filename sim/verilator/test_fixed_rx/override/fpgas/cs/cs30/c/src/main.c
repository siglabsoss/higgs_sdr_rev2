#include "dma.h"
#include "mover.h"
#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"

#define SRC_ADDR (0x0)
#define DST_ADDR (0x20)

#define GARBAGE_ADDR          (4094*NSLICES)
#define SCRATCH_ADDR          (4095*NSLICES)

#define VECTOR_REPORT_ADDRESS 4093

register volatile unsigned int x3 asm("x3");

Schedule sch1 [NSLICES] = {  //SOURCE, DESTINATION
					(Schedule) {SRC_ADDR, 0x1, (0x3 << 12)   | (DST_ADDR+9), 128},    //vector column 0
					(Schedule) {SRC_ADDR, 0x1, (0x0 << 12)   | (GARBAGE_ADDR), 0},  //vector column 1    
					(Schedule) {SRC_ADDR, 0x1, (0x2 << 12)   | (DST_ADDR+54), 128},   //vector column 2  
					(Schedule) {SRC_ADDR, 0x1, (0x4 << 12)   | (DST_ADDR+56), 128},   //vector column 3  
					(Schedule) {SRC_ADDR, 0x1, (0x0 << 12)   | (GARBAGE_ADDR), 0},  //vector column 4
					(Schedule) {SRC_ADDR, 0x1, (0xd << 12)   | (DST_ADDR+8), 128},    //vector column 5
					(Schedule) {SRC_ADDR, 0x1, (0x0 << 12)   | (GARBAGE_ADDR), 0},  //vector column 6 
					(Schedule) {SRC_ADDR, 0x1, (0x0 << 12)   | (GARBAGE_ADDR), 0},  //vector column 7 
					(Schedule) {GARBAGE_ADDR, 0, (0xe << 12) | (DST_ADDR+55), 128},   //vector column 8 
					(Schedule) {GARBAGE_ADDR, 0, (0x0 << 12) | (GARBAGE_ADDR), 0},  //vector column 9 
					(Schedule) {GARBAGE_ADDR, 0, (0x7 << 12) | (DST_ADDR+7), 128},    //vector column 10 
					(Schedule) {GARBAGE_ADDR, 0, (0x0 << 12) | (GARBAGE_ADDR), 0},  //vector column 11
					(Schedule) {GARBAGE_ADDR, 0, (0x0 << 12) | (GARBAGE_ADDR), 0},  //vector column 12 
					(Schedule) {GARBAGE_ADDR, 0, (0x8 << 12) | (DST_ADDR+54), 128},   //vector column 13
					(Schedule) {GARBAGE_ADDR, 0, (0x0 << 12) | (GARBAGE_ADDR), 0},  //vector column 14
					(Schedule) {GARBAGE_ADDR, 0, (0x1 << 12) | (DST_ADDR+6), 128}
					};   //vector column 15




Schedule sch2 [NSLICES] = {  //SOURCE, DESTINATION
					(Schedule) {GARBAGE_ADDR, 0, ((11) << 12)| (64+DST_ADDR+9), 128},    //vector column 0
					(Schedule) {GARBAGE_ADDR, 0, ((0) << 12) | (GARBAGE_ADDR), 0},  //vector column 1    
					(Schedule) {GARBAGE_ADDR, 0, ((10) << 12)| (64+DST_ADDR+54), 128},   //vector column 2  
					(Schedule) {GARBAGE_ADDR, 0, ((12) << 12)| (64+DST_ADDR+56), 128},   //vector column 3  
					(Schedule) {GARBAGE_ADDR, 0, ((0) << 12) | (GARBAGE_ADDR), 0},  //vector column 4
					(Schedule) {GARBAGE_ADDR, 0, ((5) << 12) | (64+DST_ADDR+8), 128},    //vector column 5
					(Schedule) {GARBAGE_ADDR, 0, ((0) << 12) | (GARBAGE_ADDR), 0},  //vector column 6 
					(Schedule) {GARBAGE_ADDR, 0, ((0) << 12) | (GARBAGE_ADDR), 0},  //vector column 7 
					(Schedule) {SRC_ADDR, 0x1,   ((6) << 12) | (64+DST_ADDR+55), 128},   //vector column 8 
					(Schedule) {SRC_ADDR, 0x1,   ((0) << 12) | (GARBAGE_ADDR), 0},  //vector column 9 
					(Schedule) {SRC_ADDR, 0x1,   ((15) << 12)| (64+DST_ADDR+7), 128},    //vector column 10 
					(Schedule) {SRC_ADDR, 0x1,   ((0) << 12) | (GARBAGE_ADDR), 0},  //vector column 11
					(Schedule) {SRC_ADDR, 0x1,   ((0) << 12) | (GARBAGE_ADDR), 0},  //vector column 12 
					(Schedule) {SRC_ADDR, 0x1,   ((0) << 12) | (64+DST_ADDR+54), 128},   //vector column 13
					(Schedule) {SRC_ADDR, 0x1,   ((0) << 12) | (GARBAGE_ADDR), 0},  //vector column 14
					(Schedule) {SRC_ADDR, 0x1,   ((9) << 12) | (64+DST_ADDR+6), 128}
					};   //vector column 15


Schedule sch3 [NSLICES] = {  //SOURCE, DESTINATION
					(Schedule) {(0xf << 12)   | SRC_ADDR, 0x1,   (DST_ADDR+9), 128},    //vector column 0
					(Schedule) {(0x9 << 12)   | SRC_ADDR, 0x1,   (GARBAGE_ADDR), 0},  //vector column 1    
					(Schedule) {(0x3 << 12)   | SRC_ADDR, 0x1,   (DST_ADDR+54), 128},   //vector column 2  
					(Schedule) {(0xd << 12)   | SRC_ADDR, 0x1,   (DST_ADDR+56), 128},   //vector column 3  
					(Schedule) {(0xe << 12)   | SRC_ADDR, 0x1,   (GARBAGE_ADDR), 0},  //vector column 4
					(Schedule) {(0x8 << 12)   | SRC_ADDR, 0x1,   (DST_ADDR+8), 128},    //vector column 5
					(Schedule) {(0x2 << 12)   | SRC_ADDR, 0x1,   (GARBAGE_ADDR), 0},  //vector column 6 
					(Schedule) {(0xc << 12)   | SRC_ADDR, 0x1,   (GARBAGE_ADDR), 0},  //vector column 7 
					(Schedule) { (0x0 << 12)  | GARBAGE_ADDR, 0, (DST_ADDR+55), 128},   //vector column 8 
					(Schedule) { (0x0 << 12)  | GARBAGE_ADDR, 0, (GARBAGE_ADDR), 0},  //vector column 9 
					(Schedule) { (0x0 << 12)  | GARBAGE_ADDR, 0, (DST_ADDR+7), 128},    //vector column 10 
					(Schedule) { (0x0 << 12)  | GARBAGE_ADDR, 0, (GARBAGE_ADDR), 0},  //vector column 11
					(Schedule) { (0x0 << 12)  | GARBAGE_ADDR, 0, (GARBAGE_ADDR), 0},  //vector column 12 
					(Schedule) { (0x0 << 12)  | GARBAGE_ADDR, 0, (DST_ADDR+54), 128},   //vector column 13
					(Schedule) { (0x0 << 12)  | GARBAGE_ADDR, 0, (GARBAGE_ADDR), 0},  //vector column 14
					(Schedule) { (0x0 << 12)  | GARBAGE_ADDR, 0, (DST_ADDR+6), 128}
					};   //vector column 15

Schedule sch4 [NSLICES] = {  //SOURCE, DESTINATION
					(Schedule) {(0x0 << 12)   | GARBAGE_ADDR, 0x1,   (64+ DST_ADDR+9), 128},    //vector column 0
					(Schedule) {(0x0 << 12)   | GARBAGE_ADDR, 0x1,   (GARBAGE_ADDR), 0},  //vector column 1    
					(Schedule) {(0x0 << 12)   | GARBAGE_ADDR, 0x1,   (64+ DST_ADDR+54), 128},   //vector column 2  
					(Schedule) {(0x0 << 12)   | GARBAGE_ADDR, 0x1,   (64+ DST_ADDR+56), 128},   //vector column 3  
					(Schedule) {(0x0 << 12)   | GARBAGE_ADDR, 0x1,   (GARBAGE_ADDR), 0},  //vector column 4
					(Schedule) {(0x0 << 12)   | GARBAGE_ADDR, 0x1,   (64+ DST_ADDR+8), 128},    //vector column 5
					(Schedule) {(0x0 << 12)   | GARBAGE_ADDR, 0x1,   (GARBAGE_ADDR), 0},  //vector column 6 
					(Schedule) {(0x0 << 12)   | GARBAGE_ADDR, 0x1,   (GARBAGE_ADDR), 0},  //vector column 7 
					(Schedule) { (0x7 << 12)  | SRC_ADDR, 0x1, (64+ DST_ADDR+55), 128},   //vector column 8 
					(Schedule) { (0x1 << 12)  | SRC_ADDR, 0x1, (GARBAGE_ADDR), 0},  //vector column 9 
					(Schedule) { (0xb << 12)  | SRC_ADDR, 0x1, (64+ DST_ADDR+7), 128},    //vector column 10 
					(Schedule) { (0x5 << 12)  | SRC_ADDR, 0x1, (GARBAGE_ADDR), 0},  //vector column 11
					(Schedule) { (0x6 << 12)  | SRC_ADDR, 0x1, (GARBAGE_ADDR), 0},  //vector column 12 
					(Schedule) { (0x1 << 12)  | SRC_ADDR, 0x1, (64+ DST_ADDR+54), 128},   //vector column 13
					(Schedule) { (0xa << 12)  | SRC_ADDR, 0x1, (GARBAGE_ADDR), 0},  //vector column 14
					(Schedule) { (0x4 << 12)  | SRC_ADDR, 0x1, (64+ DST_ADDR+6), 128}
					};   //vector column 15


void report_test_results(unsigned int data)
{
  // put the value of pass_fail_0 into the
  // vector memory at a high address
  unsigned int occupancy;

  while(1)
  {
   CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
   if(occupancy == 0)
   {
    break;
   }
  }


  vector_memory[VECTOR_REPORT_ADDRESS] = data;
  // vector_report[0] = data;

  CSR_WRITE(DMA_1_START_ADDR, VECTOR_REPORT_ADDRESS);
  CSR_WRITE(DMA_1_LENGTH, 1);
  CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);  // start right away
  CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0); // any values
}

int main(void)
{

	unsigned int test;

	dma_in_set(DST_ADDR, 1024*32);

	for (unsigned int i = 0; i < 8*NSLICES; i++){
		vector_memory[SRC_ADDR+i] =  i+1;
	}

	mover_schedule(sch1, SCRATCH_ADDR); 
	mover_roll(8);

	mover_schedule(sch2, SCRATCH_ADDR);
	mover_roll(8);

	fill(0x0, 8, 0xdead);

	mover_schedule(sch3, SCRATCH_ADDR);
	mover_unroll(8);

	mover_schedule(sch4, SCRATCH_ADDR);
	mover_unroll(8);

	for (unsigned int i = 0; i < 8*NSLICES; i++){
		if (vector_memory[SRC_ADDR+i]!=i+1){
			x3 = i;
		}
	}

	unsigned int all_results = 0;
	report_test_results(0xdeadbeef);
	
	// test = test0();
	// all_results |= (test==0)<<0;

	report_test_results(test);
	report_test_results(all_results);

}
