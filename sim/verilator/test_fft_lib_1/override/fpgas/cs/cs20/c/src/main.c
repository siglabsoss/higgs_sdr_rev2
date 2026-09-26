
#include "xbaseband.h"
#include "vmem.h"
#include "csr_control.h"
#include "bootloader.h"
#include "pass_fail.h"
#include "ringbus.h"

#include "flush_config_word_data.h"

// includes a lot of constants
#define FFT_1024_INCLUDE_VERIFICATION
// #include "fft_1024_opt.h"
#include "fft_1024_3914.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS20
#include "ringbus2_post.h"

#include "vmalloc.h"
// declare as global
VMalloc mgr;


void report_test_results(unsigned int data)
{
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



// unsigned int xorshift32(unsigned int state*, unsigned int len)
unsigned int simple_hash(unsigned int *state, unsigned int len)
{
  /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
  unsigned int hash = 0;
  for(unsigned int i = 0; i < len; i++) {
    hash += state[i];
    hash ^= hash << 13;
    hash ^= hash >> 17;
    hash ^= hash << 5;
  }
  return hash;
}


// test if the first plan can run back to back
unsigned int test0(void) {
  init_VMalloc(&mgr);

  if( VMALLOC_CHUNK_SIZE != 4096) {
    while(1){}; // if stuck here vmalloc has changed
  }


  unsigned int output_row = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));

  unsigned int* input_ptr = vmalloc_single(&mgr);

  // this modifies the input, so we need to copy it before running
  for(unsigned int i = 0; i < 1024; i++) {
    input_ptr[i] = first_input_fft_data[i];
  }


  // fft1024_t plan = get_default_fft1024();
  fft1024_t first_plan = get_fft1024_plan(
    VMEM_ROW_ADDRESS(input_ptr), // input
    // tmp_row0,  // tmp
    // tmp_row1,
    // tmp_row2,
    // tmp_row3,
    output_row         // output
    );
  //report_test_results(__LINE__);
  fft_1024_run(&first_plan);
  //report_test_results(__LINE__);

  int pass = 0;

  int result_location = VMEM_ROW_ADDRESS(first_expected_fft_results_4);
  int res_flag = fft_1024_result_check(result_location, output_row, 1024);

  if(res_flag != 1) {
    return 1;
  }

  // copy again to the same location
  for(unsigned int i = 0; i < 1024; i++) {
    input_ptr[i] = first_input_fft_data[i];
  }


  fft_1024_run(&first_plan);

  res_flag = fft_1024_result_check(result_location, output_row, 1024);
  
  if(res_flag != 1) {
    return 2;
  }

  return 0;

}

// test if the lib passes all 3 tests
unsigned int test1(void) {
  init_VMalloc(&mgr);

  if( VMALLOC_CHUNK_SIZE != 4096) {
    while(1){}; // if stuck here vmalloc has changed
  }


  // has temp 0,1,2,3
  unsigned int tmp_row0 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
  unsigned int tmp_row1 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
  unsigned int tmp_row2 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
  unsigned int tmp_row3 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
  // unsigned int tmp_row2 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));

  unsigned int output_row = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));

  unsigned int* input_ptr = vmalloc_single(&mgr);

  // this modifies the input, so we need to copy it before running
  for(unsigned int i = 0; i < 1024; i++) {
    input_ptr[i] = first_input_fft_data[i];
  }


  fft1024_t first_plan = get_fft1024_plan(
    VMEM_ROW_ADDRESS(input_ptr), // input
    // tmp_row0,  // tmp
    // tmp_row1,
    // tmp_row2,
    // tmp_row3,
    output_row         // output
    );

  fft_1024_run(&first_plan);

  // hash in/out after 1st run
  // report_test_results(simple_hash(first_input_fft_data, 1024));
  // report_test_results(simple_hash(first_expected_fft_results_4, 1024));

  int pass = 0;

  int res_flag = fft_1024_result_check(
    VMEM_ROW_ADDRESS(first_expected_fft_results_4),
    output_row,
    1024);

  if(res_flag != 1) {
    return 1;
  }

  fft1024_t second_plan = get_fft1024_plan(
    VMEM_ROW_ADDRESS(input_ptr), // input
    // tmp_row0,  // tmp
    // tmp_row1,
    // tmp_row2,
    // tmp_row3,
    output_row         // output
    );

  // copy again to the same location
  for(unsigned int i = 0; i < 1024; i++) {
    input_ptr[i] = second_input_fft_data[i];
  }

  fft_1024_run(&second_plan);

  res_flag = fft_1024_result_check(
    VMEM_ROW_ADDRESS(second_expected_fft_results_4),
    output_row,
    1024);

  if(res_flag != 1) {
    return 2;
  }


  fft1024_t third_plan = get_fft1024_plan(
    VMEM_ROW_ADDRESS(input_ptr), // input
    // tmp_row0,  // tmp
    // tmp_row1,
    // tmp_row2,
    // tmp_row3,
    output_row         // output
    );

  // copy again to the same location
  for(unsigned int i = 0; i < 1024; i++) {
    input_ptr[i] = third_input_fft_data[i];
  }

  fft_1024_run(&third_plan);

  res_flag = fft_1024_result_check(
    VMEM_ROW_ADDRESS(third_expected_fft_results_4),
    output_row,
    1024);

  if(res_flag != 1) {
    return 3;
  }

  return 0;
}

// test if the we can malloc and free in a churn, but pass fft every time
unsigned int test2(void) {
  init_VMalloc(&mgr);

  if( VMALLOC_CHUNK_SIZE != 4096) {
    while(1){}; // if stuck here vmalloc has changed
  }

  // 8 malloc's per loop
  // currently there are 64 vmem chunks
  // so 8 iterations will use every chunk
  for(unsigned int j = 0; j < 13; j++ ) {
    unsigned int tmp_row0 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
    unsigned int tmp_row1 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
    unsigned int tmp_row2 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
    unsigned int tmp_row3 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
    unsigned int output_row = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
    unsigned int* first_input_ptr = vmalloc_single(&mgr);

    // this modifies the input, so we need to copy it before running
    for(unsigned int i = 0; i < 1024; i++) {
      first_input_ptr[i] = first_input_fft_data[i];
    }


    fft1024_t first_plan = get_fft1024_plan(
      VMEM_ROW_ADDRESS(first_input_ptr), // input
      // tmp_row0,  // tmp
      // tmp_row1,
      // tmp_row2,
      // tmp_row3,
      output_row         // output
      );

    fft_1024_run(&first_plan);

    // hash in/out after 1st run
    // report_test_results(simple_hash(first_input_fft_data, 1024));
    // report_test_results(simple_hash(first_expected_fft_results_4, 1024));

    int pass = 0;

    int res_flag = fft_1024_result_check(
      VMEM_ROW_ADDRESS(first_expected_fft_results_4),
      output_row,
      1024);

    if(res_flag != 1) {
      return 1;
    }

    unsigned int* second_input_ptr = vmalloc_single(&mgr);

    fft1024_t second_plan = get_fft1024_plan(
      VMEM_ROW_ADDRESS(second_input_ptr), // input
      // tmp_row0,  // tmp
      // tmp_row1,
      // tmp_row2,
      // tmp_row3,
      output_row         // output
      );

    // copy again to the same location
    for(unsigned int i = 0; i < 1024; i++) {
      second_input_ptr[i] = second_input_fft_data[i];
    }

    fft_1024_run(&second_plan);

    res_flag = fft_1024_result_check(
      VMEM_ROW_ADDRESS(second_expected_fft_results_4),
      output_row,
      1024);

    if(res_flag != 1) {
      return 2;
    }

    unsigned int* third_input_ptr = vmalloc_single(&mgr);


    fft1024_t third_plan = get_fft1024_plan(
      VMEM_ROW_ADDRESS(third_input_ptr), // input
      // tmp_row0,  // tmp
      // tmp_row1,
      // tmp_row2,
      // tmp_row3,
      output_row         // output
      );

    // copy again to the same location
    for(unsigned int i = 0; i < 1024; i++) {
      third_input_ptr[i] = third_input_fft_data[i];
    }

    fft_1024_run(&third_plan);

    res_flag = fft_1024_result_check(
      VMEM_ROW_ADDRESS(third_expected_fft_results_4),
      output_row,
      1024);

    if(res_flag != 1) {
      return 3;
    }

    if( output_row == 0 ) {
      return 4;
    }

    if( first_input_ptr == 0) {
      return 5;
    }

    if( second_input_ptr == 0) {
      return 6;
    }

    if( third_input_ptr == 0) {
      return 7;
    }

    // I shuffled the order here for effect
    vfree(&mgr, REVERSE_VMEM_ROW_ADDRESS(output_row));
    vfree(&mgr, first_input_ptr);
    vfree(&mgr, REVERSE_VMEM_ROW_ADDRESS(tmp_row0));
    vfree(&mgr, REVERSE_VMEM_ROW_ADDRESS(tmp_row1));
    vfree(&mgr, third_input_ptr);
    vfree(&mgr, REVERSE_VMEM_ROW_ADDRESS(tmp_row3));
    vfree(&mgr, second_input_ptr);
    vfree(&mgr, REVERSE_VMEM_ROW_ADDRESS(tmp_row2));
  }

  return 0;

}


// test if fft can run when we allocate all the memory
// weird test but this caused me to realize I was over subscribing when giving tmp (it's fixed)
unsigned int test3(void) {
  init_VMalloc(&mgr);

  if( VMALLOC_CHUNK_SIZE != 4096) {
    while(1){}; // if stuck here vmalloc has changed
  }

  // expensive, how many chunks left?
  unsigned int avail = vmalloc_available(&mgr);

  report_test_results(avail);

  unsigned int *ptr;

  // we know we will allocate 7 chunks here
  for(unsigned int i = 0; i < avail-6; i++) {
    ptr = vmalloc_single(&mgr);
  }

  report_test_results(vmalloc_available(&mgr));


  unsigned int tmp_row0 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
  unsigned int tmp_row1 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
  unsigned int tmp_row2 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
  unsigned int tmp_row3 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));

  report_test_results(vmalloc_available(&mgr));

  unsigned int output_row = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
  unsigned int* output_ptr = REVERSE_VMEM_ROW_ADDRESS(output_row);

  report_test_results(vmalloc_available(&mgr));

  unsigned int* input_ptr = vmalloc_single(&mgr);

  if( input_ptr == 0 )
  {
    return 3; // test hit 0 when doing setup, probably an error in loop above that takes all ther reset, or a bug in avail
  }
  report_test_results(vmalloc_available(&mgr));

  // this modifies the input, so we need to copy it before running
  for(unsigned int i = 0; i < 1024; i++) {
    input_ptr[i] = first_input_fft_data[i];
  }


  fft1024_t first_plan = get_fft1024_plan(
    VMEM_ROW_ADDRESS(input_ptr), // input
    // tmp_row0,  // tmp
    // tmp_row1,
    // tmp_row2,
    // tmp_row3,
    output_row         // output
    );

  fft_1024_run(&first_plan);


  int pass = 0;

  int result_location = VMEM_ROW_ADDRESS(first_expected_fft_results_4);
  int res_flag = fft_1024_result_check(result_location, output_row, 1024);

  if(res_flag != 1) {
    return 1;
  }

  // copy again to the same location
  for(unsigned int i = 0; i < 1024; i++) {
    input_ptr[i] = first_input_fft_data[i];
  }


  fft_1024_run(&first_plan);

  res_flag = fft_1024_result_check(result_location, output_row, 1024);

  if(res_flag != 1) {
    return 2;
  }

  return 0;

}


// // test if it can run when we allocate all the memory and then choose spaced out
// // pointers to give it (we also fill the memory)
// unsigned int test4(void) {
//   init_VMalloc(&mgr);

//   if( VMALLOC_CHUNK_SIZE != 4096) {
//     while(1){}; // if stuck here vmalloc has changed
//   }

//   // expensive
//   unsigned int avail = vmalloc_available(&mgr);

//   if( avail != 0x33) {
//     while(1){}; // if stuck here if ANYTHIng has chagned
//   }

//   unsigned int ptrs[avail]; 

//   for(unsigned int i = 0; i < avail; i++) {
//     ptrs[i] = (unsigned int) vmalloc_single(&mgr);

//     // report_test_results(ptrs[i]);

//     if(ptrs[i] == 0) {
//       return 0x100+i;
//     }
//   }

//   // consider each block of allocated memory
//   for(unsigned int i = 0; i < avail; i++) {
//     // convert to a riscv pointer
//     unsigned int *chunk = (unsigned int *) ptrs[i];

//     // pattern it
//     for(unsigned int j = 0; j < (VMALLOC_CHUNK_SIZE/4); j+=3) {
//       chunk[j] = 0xf0000000 | i;
//     }
//     chunk[(VMALLOC_CHUNK_SIZE/4)-1] = 0xdeadbeef; // make sure to get the last one
//   }
  


//   // has temp 0,1,2,3
//   unsigned int tmp_row0 = VMEM_ROW_ADDRESS(ptrs[15]);
//   unsigned int tmp_row1 = VMEM_ROW_ADDRESS(ptrs[17]);
//   unsigned int tmp_row2 = VMEM_ROW_ADDRESS(ptrs[19]);
//   unsigned int tmp_row3 = VMEM_ROW_ADDRESS(ptrs[21]);

//   unsigned int output_row = VMEM_ROW_ADDRESS(ptrs[9]);
//   unsigned int* output_ptr = REVERSE_VMEM_ROW_ADDRESS(output_row);

//   unsigned int* input_ptr = (unsigned int*) ptrs[13];

//   // this modifies the input, so we need to copy it before running
//   for(unsigned int i = 0; i < 1024; i++) {
//     input_ptr[i] = first_input_fft_data[i];
//   }

//   fft1024_t first_plan = get_fft1024_plan(
//     VMEM_ROW_ADDRESS(input_ptr), // input
//     // tmp_row0,  // tmp
//     // tmp_row1,
//     // tmp_row2,
//     // tmp_row3,
//     output_row         // output
//     );

//   fft_1024_run(&first_plan);


//   // FIXME Check pattern here in test

//   int pass = 0;

//   int result_location = VMEM_ROW_ADDRESS(first_expected_fft_results_4);
//   int res_flag = fft_1024_result_check(result_location, output_row, 1024);

//   if(res_flag != 1) {
//     return 1;
//   }

//   return 0;
// }



int main(void)
{
  unsigned int all_results = 0;
  unsigned int test = 0;

  Ringbus ringbus;

  report_test_results(0xdeadbeef);
 
  test = test0();
  all_results |= (test==0)<<0;

  test = test1();
  all_results |= (test==0)<<1;

  test = test2();
  all_results |= (test==0)<<2;

  test = test3();
  all_results |= (test==0)<<3;

  // test = test4();
  // all_results |= (test==0)<<4;


  report_test_results(test);
  report_test_results(all_results);

  //while(1) {
  //  check_ring(&ringbus);
  //}

  return 0;
}
