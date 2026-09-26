#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "vmalloc.h"
#include "linker_symbols.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"

// declare as global
VMalloc mgr;

VMEM_SECTION unsigned int dma_in[1] = {};

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

void dump_table(void) {
  report_test_results(1);
  report_test_results(1);
  report_test_results((unsigned int)mgr.head);
  report_test_results((unsigned int)mgr.tail);
  report_test_results(1);
  for(unsigned int i = 0; i < VMALLOC_CHUNK_COUNT; i++) {
    report_test_results(VM_IDX_TO_ADDRESS(mgr.nodes[i].idx));
    report_test_results((unsigned int)mgr.nodes[i].next);
  }
}

// it's tricky to test but a simple one is to allocate a single vmem byte
// and see that we get back chunks -1
unsigned int test0(void) {
  unsigned int fail = 0;
  init_VMalloc(&mgr);

  int dump = 0;

  // if(dump){dump_table();}

  // unsigned int *ptr[VMALLOC_CHUNK_COUNT+2];
  // // allocate and save all

  unsigned int got = 0;

  vmem_t* burn;

  for(unsigned int i = 0; i < VMALLOC_CHUNK_COUNT*2; i++) {
    burn = vmalloc_single(&mgr);

    if(burn != 0) {
      got++;
    } else {
      break;
    }
    // report_test_results(ptr[i]);
  }

  if(got != VMALLOC_CHUNK_COUNT-1) {
    return 1;  // test if allocating a byte in the file gives us 1 less section
  }
  return 0;

  // if(dump){dump_table();}

  // for(unsigned int i = 0; i < VMALLOC_CHUNK_COUNT-1; i++) {
  //   vfree(&mgr, ptr[(VMALLOC_CHUNK_COUNT-1-1)-i]);
  // }

  // if(dump){dump_table();}
}

// meant to be displayed, always passes
unsigned int test1_display(void) {
  unsigned int fail = 0;
  init_VMalloc(&mgr);

  int dump = 1;

  if(dump){dump_table();}

  unsigned int *ptr[VMALLOC_CHUNK_COUNT+2];
  // // allocate and save all
  for(unsigned int i = 0; i < VMALLOC_CHUNK_COUNT-1; i++) {
    ptr[i] = vmalloc_single(&mgr);
    // report_test_results(ptr[i]);
  }

  if(dump){dump_table();}

  for(unsigned int i = 0; i < VMALLOC_CHUNK_COUNT-1; i++) {
    vfree(&mgr, ptr[(VMALLOC_CHUNK_COUNT-1-1)-i]);
  }

  if(dump){dump_table();}

  return 0;
}


int main(void) {
    for(unsigned int i = 0; i < 7000; i++) {
        STALL(1);
    }
  unsigned int all_results = 0;
  unsigned int test;

  // force gcc to not optimized these out
  SET_REG(x3,dma_in[0]);
 
  test = test0();
  all_results |= (test==0)<<0;

  // test = test1();
  // all_results |= (test==0)<<1;

  // test = test2();
  // all_results |= (test==0)<<2;

  // test = test3();
  // all_results |= (test==0)<<3;

  // test = test4();
  // all_results |= (test==0)<<4;

  // test = test5();
  // all_results |= (test==0)<<5;

  // test = test6();
  // all_results |= (test==0)<<6;

  // test = test7();
  // all_results |= (test==0)<<7;

  // report_test_results(test);
  report_test_results(all_results);
}


