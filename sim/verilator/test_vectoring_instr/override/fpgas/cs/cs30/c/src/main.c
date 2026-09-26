#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "vmalloc.h"
#include "linker_symbols.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS30
#include "ringbus2_post.h"

#include "xvcordic.h"

// declare as global
VMalloc mgr;

// #define VECTOR_REPORT_ADDRESS 5000

void report_test_results(unsigned int data)
{
  // put the value of pass_fail_0 into the
  // vector memory at a high address
  unsigned int occupancy;

  while(1)
  {
   CSR_READ(RINGBUS_SCHEDULE_OCCUPANCY, occupancy);
   if(occupancy < RINGBUS_SCHEDULE_DEPTH)
   {
    break;
   }
  }

  CSR_WRITE(RINGBUS_WRITE_ADDR, RING_ADDR_PC);
  CSR_WRITE(RINGBUS_WRITE_DATA, data);
  CSR_WRITE(RINGBUS_WRITE_EN, 0);
}


int main(void) {
  unsigned int all_results = 0;
  unsigned int test;

  int input = 0x80008000;
  //int input = 0x80018001;
  int iters = 4;
  int output;
  
  ATAN(output,input,iters);
  report_test_results(output);
  iters=8;
  ATAN(output,input,iters);
  report_test_results(output);
  iters=10;
  ATAN(output,input,iters);
  report_test_results(output);
  iters=12;
  ATAN(output,input,iters);
  report_test_results(output);
  iters=14;
  ATAN(output,input,iters);
  report_test_results(output);
  iters=15;
  ATAN(output,input,iters);
  report_test_results(output);
  // iters=0x1A;
  // ATAN(output,input,iters);

  iters=15;

  report_test_results(iters);
  ATAN(output,0x7fff,iters); //0000
  report_test_results(output);
  ATAN(output,0x7fff7fff,iters); //3fff
  report_test_results(output); 
  ATAN(output,0x7fff0000,iters); //7fff
  report_test_results(output);
  ATAN(output,0x7fff8000,iters); //3fff
  report_test_results(output);
  ATAN(output,0x8000,iters); //0000
  report_test_results(output);
  ATAN(output,0x80008000,iters); //c000
  report_test_results(output);
  ATAN(output,0x80000000,iters); //8000
  report_test_results(output);
  ATAN(output,0x80007fff,iters); //c000
  report_test_results(output);
  
  //  input = 0x
  // report_test_results(test);
  //  report_test_results(output);
}








// report_test_results((unsigned int)& START_LOC);
// report_test_results((unsigned int)& PASS_FAIL_LEN);
// report_test_results((unsigned int)& BOOT_LEN);
// report_test_results((unsigned int)& __imem_size);
// report_test_results((unsigned int)& __stack_size);
// report_test_results((unsigned int)& DMEM_LEN);
// report_test_results((unsigned int)& IMEM_LEN);
// report_test_results((unsigned int)& __bss_vma_start);
// report_test_results((unsigned int)& __dedicated_mem_end);
// report_test_results((unsigned int)& __bss_vma_end);
// report_test_results((unsigned int)& __sp_start);
// report_test_results((unsigned int)& __pass_fail_0);
// report_test_results((unsigned int)& __pass_fail_1);
// report_test_results((unsigned int)& V_MEM_LEN);
// report_test_results((unsigned int)& V_MEM_START);
// report_test_results((unsigned int)& __vmem_start);
// report_test_results((unsigned int)& __vmem_end);
