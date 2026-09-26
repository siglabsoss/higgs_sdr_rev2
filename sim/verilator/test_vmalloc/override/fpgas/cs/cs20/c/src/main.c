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


// tests that we get memory and then 0's after that
unsigned int test0(void) {
  unsigned int fail = 0;
  init_VMalloc(&mgr);

  vmem_t* burn;
  for(unsigned int i = 0; i < VMALLOC_CHUNK_COUNT; i++) {
    burn = vmalloc_single(&mgr);
    if((void*)burn == 0) {
      return 0xf000|i; // got a 0 during valid memory chunks
    }
  }

  burn = vmalloc_single(&mgr);
  if((void*)burn != 0) {
      return 2; // got a non 0 after filling up memory
  }
  return 0;
}

// tests that we can "re-use" constructor with no ill effect
unsigned int test1(void) {
  unsigned int fail = 0;
  init_VMalloc(&mgr);

  vmem_t* burn;
  for(unsigned int i = 0; i < VMALLOC_CHUNK_COUNT; i++) {
    burn = vmalloc_single(&mgr);
    if((void*)burn == 0) {
      return 0xf000|i;
    }
  }

  burn = vmalloc_single(&mgr);
  if((void*)burn != 0) {
      return 2;
  }
  return 0;
}

// test if can free and allocate 1 thing and if vmalloc_available works
unsigned int test2(void) {
  unsigned int fail = 0;
  init_VMalloc(&mgr);

  int cap;// = vmalloc_available(&mgr);
  cap = vmalloc_available(&mgr);
  // report_test_results(cap);
  if( cap != VMALLOC_CHUNK_COUNT) {
    return 1;
  }

  // unsigned int *ptr[VMALLOC_CHUNK_COUNT];
  int dump = 0;

  vmem_t* hold1;

  if(dump){dump_table();}
  hold1 = vmalloc_single(&mgr);
  if(dump){dump_table();}
  vfree(&mgr, hold1);
  if(dump){dump_table();}

  cap = vmalloc_available(&mgr);
  if( cap != VMALLOC_CHUNK_COUNT) {
    return 2;
  }

  return 0;
}

// allocates 2 nodes,
// and then spam allocate and deallocate the rest
// after this release 1 and then spam allocate until we are given the released node
// release the last and do the same
unsigned int test3(void) {
  unsigned int fail = 0;
  init_VMalloc(&mgr);

  int dump = 0;

  unsigned int *ptr[2];

  ptr[0] = vmalloc_single(&mgr);
  ptr[1] = vmalloc_single(&mgr);

  if(dump){dump_table();}

  // report_test_results(ptr[0]);
  // report_test_results(ptr[1]);

  vmem_t* hold1;
  vmem_t* hold2;
  // run a loop for 2 times number of slots
  // the loop allocates two blocks and then "randomly" frees them
  for(unsigned int i = 0; i < VMALLOC_CHUNK_COUNT*2; i++) {

    if( i % 2 == 0 ) {
      hold1 = vmalloc_single(&mgr);
      hold2 = vmalloc_single(&mgr);
    } else {
      hold2 = vmalloc_single(&mgr);
      hold1 = vmalloc_single(&mgr);
    }

    // fail if we get "given" a value that we allocated up above
    if(hold1 == ptr[0] || hold1 == ptr[1]) {
      return 1;
    }
    if(hold2 == ptr[0] || hold2 == ptr[1]) {
      return 2;
    }


    if( i % 3 == 0 ) {
      vfree(&mgr, hold1);
      vfree(&mgr, hold2);
    } else {
      vfree(&mgr, hold2);
      vfree(&mgr, hold1);
    }

    if(dump){dump_table();}
  }

  // free one of the pointers
  vfree(&mgr, ptr[1]);

  int pass = 0;
  // loop over remaining allocations until we hit the recently released memory
  for(unsigned int i = 0; i < VMALLOC_CHUNK_COUNT*2; i++) {
    hold1 = vmalloc_single(&mgr);
    hold2 = vmalloc_single(&mgr);

    if( hold1 == ptr[1] || hold2 == ptr[1] )
    {
      // break early, but we hafta copy paste these
      vfree(&mgr, hold2);
      vfree(&mgr, hold1);
      pass = 1;
      break;
    }

    vfree(&mgr, hold2);
    vfree(&mgr, hold1);
  }

  if(pass != 1) {
    return 3;
  }

  // release final pointer
  vfree(&mgr, ptr[0]);

  // reset pass
  pass = 0;
  // do same but more simple
  for(unsigned int i = 0; i < VMALLOC_CHUNK_COUNT*2; i++) {
    hold1 = vmalloc_single(&mgr);
    if( hold1 == ptr[0])
    {
      vfree(&mgr, hold1);
      pass = 1;
      break;
    }
    vfree(&mgr, hold1);
  }

  if(pass != 1) {
    return 4;
  }

  // burn = vmalloc_single(&mgr);
  // if((void*)burn != 0) {
  //     return 2;
  // }
  return 0;
}

// requires a large stack
// test if we can allocate all the nodes, and then release all of them (covers a bug when list is empty)
// release a single node and check that vmalloc_available is correct
// re-allocate them all again and check that we get the same addresses as the first time
unsigned int test4(void) {
  unsigned int fail = 0;
  init_VMalloc(&mgr);

  int dump = 0;
  int ava;

  unsigned int *ptr[VMALLOC_CHUNK_COUNT];

  if(dump){dump_table();}

  // allocate and save all
  for(unsigned int i = 0; i < VMALLOC_CHUNK_COUNT; i++) {
    ptr[i] = vmalloc_single(&mgr);
    if(i == (VMALLOC_CHUNK_COUNT-2)) {
      ava = vmalloc_available(&mgr);
      if(ava != 1) {
        return 0xf1;
      }
    }
  }

  ava = vmalloc_available(&mgr);
  if(ava != 0) {
    return 0xf2;
  }

  // free 0th thing
  vfree(&mgr, ptr[0]);

  ava = vmalloc_available(&mgr);
  if(ava != 1) {
    return 0xf3;
  }

  // free the rest
  for(unsigned int i = 1; i < VMALLOC_CHUNK_COUNT; i++) {
    vfree(&mgr, ptr[i]);
  }

  ava = vmalloc_available(&mgr);
  if(ava != VMALLOC_CHUNK_COUNT) {
    return 0xf4;
  }

  vmem_t* burn;

  // reallocate the whole thing to see if every single pointer here
  // is the same as the previous (order not enforced)
  for(unsigned int i = 0; i < VMALLOC_CHUNK_COUNT; i++) {

    // pick one
    burn = vmalloc_single(&mgr);
    int pass1 = 0;
    // loop through all previous ones to check if it's there
    for(unsigned int j = 0; j < VMALLOC_CHUNK_COUNT; j++) {
      if(ptr[j] == burn) {
        // found it
        pass1 = 1;
        break;
      }
    }

    // if we didn't find it with our pass, something is broken
    if( pass1 == 0) {
      return 0xf5;
    }

  }

  return 0;
}

// test immediate double free
unsigned int test5(void) {
  unsigned int fail = 0;
  init_VMalloc(&mgr);

  if(mgr.double_free_count != 0) {
    return 1; // starts out in error condition
  }

  int dump = 0;

  unsigned int *ptr[2];

  if(dump){dump_table();}

  ptr[0] = vmalloc_single(&mgr);

  if(dump){dump_table();}

  // free 0th thing
  vfree(&mgr, ptr[0]);

  if(dump){dump_table();}

  // DOUBLE free
  vfree(&mgr, ptr[0]);

  if(dump){dump_table();}

  if(mgr.tail == 0) {
    return 2;
  }

  if(mgr.double_free_count != 1) {
    return 3;
  }
  return 0;
}

// test delayed double free
unsigned int test6(void) {
  unsigned int fail = 0;
  init_VMalloc(&mgr);

  if(mgr.double_free_count != 0) {
    return 1; // starts out in error condition
  }

  int dump = 0;

  unsigned int *ptr[3];

  if(dump){dump_table();}

  ptr[0] = vmalloc_single(&mgr); // take one

  if(dump){dump_table();}

  ptr[1] = vmalloc_single(&mgr); // take another

  if(dump){dump_table();}

  vfree(&mgr, ptr[0]); // free 0th thing

  if(dump){dump_table();}

  vfree(&mgr, ptr[1]); // free 1st thing

  if(dump){dump_table();}

  if(mgr.double_free_count != 0) {
    return 2; // normal operations above caused a double_free
  }

  vfree(&mgr, ptr[0]); // DOUBLE FREE free 0th thing

  if(dump){dump_table();}

  if(mgr.tail == 0) {
    return 3;  // the double free set tail to zero (Was a bug) (is there a better way to detect corruption from double free?)
  }

  if(mgr.double_free_count != 1) {
    return 4; // class didn't detect and report the double free
  }

  return 0;
}

// test delayed double free
unsigned int test7(void) {
  unsigned int fail = 0;
  init_VMalloc(&mgr);

  if(mgr.double_free_count != 0) {
    return 1; // starts out in error condition
  }

  int dump = 0;

  unsigned int *ptr[4];

  ptr[0] = vmalloc_single(&mgr); // take one 
  ptr[1] = vmalloc_single(&mgr); // take another (will double free)
  ptr[2] = vmalloc_single(&mgr); // take 3rd
  ptr[3] = vmalloc_single(&mgr); // take

  vfree(&mgr, ptr[3]); // free
  ptr[3] = vmalloc_single(&mgr); // take in same slot

  vfree(&mgr, ptr[1]); // free

  vfree(&mgr, ptr[2]); // free
  vfree(&mgr, ptr[0]); // free
  ptr[2] = vmalloc_single(&mgr); // take in same slot
  ptr[0] = vmalloc_single(&mgr); // take in same slot
  
  // for(unsigned i = 0; i < 2; i++) {
  // }

  if(mgr.double_free_count != 0) {
    return 2; // normal operations above caused a double_free
  }

  vfree(&mgr, ptr[1]); // DOUBLE free

  if(mgr.double_free_count != 1) {
    return 3; // class didn't detect and report the double free
  }

  return 0;
}

int main(void) {
  unsigned int all_results = 0;
  unsigned int test;
 
  test = test0();
  all_results |= (test==0)<<0;

  test = test1();
  all_results |= (test==0)<<1;

  test = test2();
  all_results |= (test==0)<<2;

  test = test3();
  all_results |= (test==0)<<3;

  test = test4();
  all_results |= (test==0)<<4;

  test = test5();
  all_results |= (test==0)<<5;

  test = test6();
  all_results |= (test==0)<<6;

  test = test7();
  all_results |= (test==0)<<7;

  // report_test_results(test);
  report_test_results(all_results);
}
