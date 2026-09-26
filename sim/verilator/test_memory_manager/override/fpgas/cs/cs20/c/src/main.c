#include "xbaseband.h"
#include "csr_control.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#define MEMORY_MANAGER_DEBUG_JUNK_VMEM


#define MEMORY_MANAGER_SIZE 1024
#define MEMORY_MANAGER_CHUNKS 8
#include "memory_manager.h"

// declare as global
// VMalloc mgr;

// #define VECTOR_REPORT_ADDRESS 5000

void dump_table(void) {
    for(unsigned int i = 0; i < MEMORY_MANAGER_CHUNKS; i++) {
        ring_block_send_eth(_mgr.full[i]);
    }
    ring_block_send_eth(memory_manager_available());
    ring_block_send_eth(_mgr.double_free_count);
    ring_block_send_eth(_mgr.illegal_free_count);
    // ring_block_send_eth(_mgr.full[])
//   report_test_results(1);
//   report_test_results(1);
//   report_test_results((unsigned int)mgr.head);
//   report_test_results((unsigned int)mgr.tail);
//   report_test_results(1);
//   for(unsigned int i = 0; i < MEMORY_MANAGER_CHUNKS; i++) {
//     report_test_results(VM_IDX_TO_ADDRESS(mgr.nodes[i].idx));
//     report_test_results((unsigned int)mgr.nodes[i].next);
//   }
}



// tests that we get memory and then 0's after that
unsigned int test0(void) {
    unsigned int burn;
    unsigned int fail = 0;

    SET_REG(x3,0xfeedbeef);
    init_memory_manager();
    SET_REG(x3,0xfeed0000);

    unsigned int avail;

    avail = memory_manager_available();

    SET_REG(x3,0xfeed0001);
    SET_REG(x3,avail);

    if( avail != MEMORY_MANAGER_CHUNKS ) {
        return 1;
    }

    // burn = memory_manger_get();

    // SET_REG(x3,0xfeed0002);
    // SET_REG(x3, burn);
    // SET_REG(x3, REVERSE_VMEM_DMA_ADDRESS(burn));



    for(unsigned int i = 0; i < MEMORY_MANAGER_CHUNKS; i++) {
        burn = memory_manger_get();
        SET_REG(x3,0xfeed0002);
        SET_REG(x3, burn);
        if(burn == -1) {
            return 0xf000|i; // got a 0 during valid memory chunks
        }
    }

    burn = memory_manger_get();
    if(burn != -1) {
          return 2; // got a non 0 after filling up memory
    }

    return 0;
}

// tests that we can "re-use" constructor with no ill effect
unsigned int test1(void) {
    unsigned int fail = 0;
    init_memory_manager();

    unsigned int burn;
    for(unsigned int i = 0; i < MEMORY_MANAGER_CHUNKS; i++) {
      burn = memory_manger_get();
      if(burn == -1) {
        return 0xf000|i;
      }
    }

    burn = memory_manger_get();
    if(burn != -1) {
      return 2;
    }
    return 0;
}

// test if can free and allocate 1 thing and if vmalloc_available works
unsigned int test2(void) {
  unsigned int fail = 0;
  init_memory_manager();

  int cap;// = memory_manager_available();
  cap = memory_manager_available();
  // report_test_results(cap);
  if( cap != MEMORY_MANAGER_CHUNKS) {
    return 1;
  }

  // unsigned int *ptr[MEMORY_MANAGER_CHUNKS];
  int dump = 0;

  unsigned int hold1;

  // if(dump){dump_table();}
  hold1 = memory_manger_get();
  // if(dump){dump_table();}
  memory_manger_free(hold1);
  // if(dump){dump_table();}

  cap = memory_manager_available();
  if( cap != MEMORY_MANAGER_CHUNKS) {
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
  init_memory_manager();

  // for(unsigned int i = 0; i < MEMORY_MANAGER_CHUNKS; i++) {
  //   ring_block_send_eth(_mgr.chunk_dma_address[i]);
  // }

  int dump = 0;

  unsigned int ptr[2];

  ptr[0] = memory_manger_get();
  ptr[1] = memory_manger_get();

  // ring_block_send_eth(ptr[0]);
  // ring_block_send_eth(ptr[1]);

  if(dump){dump_table();}

  // report_test_results(ptr[0]);
  // report_test_results(ptr[1]);

  unsigned int hold1;
  unsigned int hold2;
  // run a loop for 2 times number of slots
  // the loop allocates two blocks and then "randomly" frees them
  for(unsigned int i = 0; i < MEMORY_MANAGER_CHUNKS*2; i++) {

    if( i % 2 == 0 ) {
      hold1 = memory_manger_get();
      hold2 = memory_manger_get();
    } else {
      hold2 = memory_manger_get();
      hold1 = memory_manger_get();
    }

    // fail if we get "given" a value that we allocated up above
    if(hold1 == ptr[0] || hold1 == ptr[1]) {
      return 1;
    }
    if(hold2 == ptr[0] || hold2 == ptr[1]) {
      return 2;
    }


    if( i % 3 == 0 ) {
      memory_manger_free( hold1);
      memory_manger_free( hold2);
    } else {
      memory_manger_free( hold2);
      memory_manger_free( hold1);
    }


    if(dump) { 
        ring_block_send_eth(0xdeadbeef);
        dump_table();
    }
  }

  // free one of the pointers
  memory_manger_free(ptr[1]);

  // if( memory_manager_available() != 1 ) {
  //   return 6;
  // }

  int pass = 0;
  // loop over remaining allocations until we hit the recently released memory
  for(unsigned int i = 0; i < MEMORY_MANAGER_CHUNKS*2; i++) {
    hold1 = memory_manger_get();
    hold2 = memory_manger_get();

    if( hold1 == ptr[1] || hold2 == ptr[1] )
    {
      // break early, but we hafta copy paste these
      memory_manger_free( hold2);
      memory_manger_free( hold1);
      pass = 1;
      break;
    }

    memory_manger_free( hold2);
    memory_manger_free( hold1);
  }

  if(pass != 1) {
    return 3;
  }

  // release final pointer
  memory_manger_free( ptr[0]);

  if( memory_manager_available() != MEMORY_MANAGER_CHUNKS ) {
    return 5;
  }

  // reset pass
  pass = 0;
  // do same but more simple
  for(unsigned int i = 0; i < MEMORY_MANAGER_CHUNKS*2; i++) {
    hold1 = memory_manger_get();
    if( hold1 == ptr[0])
    {
      memory_manger_free( hold1);
      pass = 1;
      break;
    }
    memory_manger_free( hold1);
  }

  if(pass != 1) {
    return 4;
  }


  return 0;
}

// requires a large stack
// test if we can allocate all the nodes, and then release all of them (covers a bug when list is empty)
// release a single node and check that vmalloc_available is correct
// re-allocate them all again and check that we get the same addresses as the first time
unsigned int test4(void) {
  unsigned int fail = 0;
  init_memory_manager();

  int dump = 0;
  int ava;

  unsigned int ptr[MEMORY_MANAGER_CHUNKS];

  if(dump){dump_table();}

  // allocate and save all
  for(unsigned int i = 0; i < MEMORY_MANAGER_CHUNKS; i++) {
    ptr[i] = memory_manger_get();
    if(i == (MEMORY_MANAGER_CHUNKS-2)) {
      ava = memory_manager_available();
      if(ava != 1) {
        return 0xf1;
      }
    }
  }

  ava = memory_manager_available();
  if(ava != 0) {
    return 0xf2;
  }

  // free 0th thing
  memory_manger_free( ptr[0]);

  ava = memory_manager_available();
  if(ava != 1) {
    return 0xf3;
  }

  // free the rest
  for(unsigned int i = 1; i < MEMORY_MANAGER_CHUNKS; i++) {
    memory_manger_free( ptr[i]);
  }

  ava = memory_manager_available();
  if(ava != MEMORY_MANAGER_CHUNKS) {
    return 0xf4;
  }

  unsigned int burn;

  // reallocate the whole thing to see if every single pointer here
  // is the same as the previous (order not enforced)
  for(unsigned int i = 0; i < MEMORY_MANAGER_CHUNKS; i++) {

    // pick one
    burn = memory_manger_get();
    int pass1 = 0;
    // loop through all previous ones to check if it's there
    for(unsigned int j = 0; j < MEMORY_MANAGER_CHUNKS; j++) {
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
  init_memory_manager();

  if(_mgr.double_free_count != 0) {
    return 1; // starts out in error condition
  }

  int dump = 0;

  unsigned int ptr[2];

  if(dump){dump_table();}

  ptr[0] = memory_manger_get();

  if(dump){dump_table();}

  // free 0th thing
  memory_manger_free( ptr[0]);

  if(dump){dump_table();}

  // DOUBLE free
  memory_manger_free( ptr[0]);

  if(dump){dump_table();}

  // if(mgr.tail == 0) {
  //   return 2;
  // }

  if(_mgr.double_free_count != 1) {
    return 3;
  }
  return 0;
}

// test delayed double free
unsigned int test6(void) {
  unsigned int fail = 0;
  init_memory_manager();

  if(_mgr.double_free_count != 0) {
    return 1; // starts out in error condition
  }

  int dump = 0;

  unsigned int ptr[3];

  if(dump){dump_table();}

  ptr[0] = memory_manger_get(); // take one

  if(dump){dump_table();}

  ptr[1] = memory_manger_get(); // take another

  if(dump){dump_table();}

  memory_manger_free( ptr[0]); // free 0th thing

  if(dump){dump_table();}

  memory_manger_free( ptr[1]); // free 1st thing

  if(dump){dump_table();}

  if(_mgr.double_free_count != 0) {
    return 2; // normal operations above caused a double_free
  }

  memory_manger_free( ptr[0]); // DOUBLE FREE free 0th thing

  if(dump){dump_table();}

  // if(_mgr.tail == 0) {
  //   return 3;  // the double free set tail to zero (Was a bug) (is there a better way to detect corruption from double free?)
  // }

  if(_mgr.double_free_count != 1) {
    return 4; // class didn't detect and report the double free
  }

  return 0;
}

// test delayed double free
unsigned int test7(void) {
  unsigned int fail = 0;
  init_memory_manager();


  if(_mgr.double_free_count != 0) {
    return 1; // starts out in error condition
  }

  int dump = 0;

  // if(dump){dump_table();}

  unsigned int ptr[4];

  ptr[0] = memory_manger_get(); // take one 
  ptr[1] = memory_manger_get(); // take another (will double free)
  ptr[2] = memory_manger_get(); // take 3rd
  ptr[3] = memory_manger_get(); // take

  // ring_block_send_eth(ptr[1]);
  // ring_block_send_eth(0xdead);


  // if(dump){dump_table();}

  memory_manger_free( ptr[3]); // free
  ptr[3] = memory_manger_get(); // take in same slot

    if(dump){
        ring_block_send_eth(0xdead);
        dump_table();
    }

  memory_manger_free( ptr[1]); // free

if(dump){
        ring_block_send_eth(0xdead);
        dump_table();
    }

  memory_manger_free( ptr[2]); // free

  if(dump){
        ring_block_send_eth(0xdead);
        dump_table();
    }

  memory_manger_free( ptr[0]); // free

  if(dump){
        ring_block_send_eth(0xdead);
        dump_table();
    }

    // if(dump){dump_table();}


  ptr[2] = memory_manger_get(); // take in same slot

  if(dump){
        ring_block_send_eth(0xdead);
        dump_table();
    }

  ptr[0] = memory_manger_get(); // take in same slot

  if(dump){
        ring_block_send_eth(0xdead);
        dump_table();
    }
  
  // for(unsigned i = 0; i < 2; i++) {
  // }

  if(_mgr.double_free_count != 0) {
    return 2; // normal operations above caused a double_free
  }


  //     if(dump){
  //       ring_block_send_eth(0xdead);
  //       dump_table();
  //   }

  // memory_manger_free( ptr[1]); // DOUBLE free

  // if(_mgr.double_free_count != 1) {
  //   return 3; // class didn't detect and report the double free
  // }

  return 0;
}

int main(void) {

    for(unsigned int i = 0; i < 7000; i++) {
        STALL(1);
    }
    unsigned int all_results = 0;
    unsigned int test;

    SET_REG(x3,0xdeadbeef);


    test = test0();
    all_results |= (test==0)<<0;

    SET_REG(x4,0x00ffffff);
    SET_REG(x4,test);
    SET_REG(x4,all_results);

    test = test1();
    all_results |= (test==0)<<1;

    SET_REG(x3,0xf0000001);
    SET_REG(x3,test);
    SET_REG(x4,all_results);

    test = test2();
    all_results |= (test==0)<<2;

    SET_REG(x3,0xf0000002);
    SET_REG(x3,test);
    SET_REG(x4,all_results);

    test = test3();
    all_results |= (test==0)<<3;

    SET_REG(x3,0xf0000003);
    SET_REG(x3,test);
    SET_REG(x3,0xf0000003);
    SET_REG(x4,all_results);

    test = test4();
    all_results |= (test==0)<<4;

    SET_REG(x3,4);
    SET_REG(x3,test);
    SET_REG(x4,all_results);

    test = test5();
    all_results |= (test==0)<<5;

    SET_REG(x3,5);
    SET_REG(x3,test);
    SET_REG(x4,all_results);

    test = test6();
    all_results |= (test==0)<<6;

    SET_REG(x3,6);
    SET_REG(x3,test);
    SET_REG(x4,all_results);

    test = test7();
    all_results |= (test==0)<<7;

    SET_REG(x3,7);
    SET_REG(x3,test);


    SET_REG(x4,0xffffffff);
    SET_REG(x4,all_results);
    // report_test_results(test);
    ring_block_send_eth(all_results);
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
