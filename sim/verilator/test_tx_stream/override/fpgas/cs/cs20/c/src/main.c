#include "fill.h"
#include "xbaseband.h"
#include "csr_control.h"
#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS20
#include "ringbus2_post.h"
#include "vmem.h"
#include "mapper.h"
#include "mover.h"
#include "vmalloc.h"
#include "ringbus.h"
VMalloc mgr; // required for vmalloc to work

int should_exit = 0;
#include "fft_1024.h"

fft1024_t plan1024;
unsigned int fft_output_ptr;
unsigned int fft_input_ptr;

void do_fft_wrap(unsigned int *cpu_ptr);
void do_allocate_subcarriers(unsigned int dma_ptr_in, unsigned int size_in);
void do_fft(unsigned int input_ptr);
void map_single_word(unsigned int* word_p, unsigned int* output_p);
void do_output_dma(unsigned int input_ptr);
void map_every_n_word(unsigned int* word_p, unsigned int stride, unsigned int* output_p);
void map_debug_tone(unsigned int* output_p);

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

Table bpsk_table;

int setup_mapper(void) {
  // vmalloc this address, convert to DMA style
  // this address is held by global (Table bpsk_table) above
  unsigned int tmp = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));

  bpsk_table = mapper_bpsk_table(tmp);

  // if we wanted to free we would do something like
  // vfree(&mgr, REVERSE_VMEM_DMA_ADDRESS(bpsk_table.addr));
}



//void trig_out()
//{
//  unsigned int dma_out_next = DMA_OUT_NEXT_INDEX();
//
//  CSR_WRITE(DMA_1_START_ADDR, DMA_OUT_START(dma_out_next));
//  CSR_WRITE(DMA_1_LENGTH, DMA_OUT_LEN);
//  CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);  // start right away
//  CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0); // any value
//
//  dma_out_last = dma_out_next;
//}

#define OUTPUT_BUF_START (0)
#define DMA_LEN (1500)

// unsigned int subcarrier_count;

// void set_subcarrier_count(unsigned int z) {
//   subcarrier_count = z;
// }

// #define DMA_IN_OUTSTANDING (4)
// unsigned int dma_

#define DMA_IN_COUNT (4)
#define DMA_IN_LEN (2)
#define DMA_IN_OFFSET (0)

#define DMA_IN_START(x) dma_ptr[(x)]
unsigned int dma_ptr[DMA_IN_COUNT];

unsigned int dma_in_last = DMA_IN_COUNT-1;
unsigned int dma_in_last_consumed = DMA_IN_COUNT-1; // fixme port this to eth
unsigned int dma_in_expected_occupancy = 0;


void trigger_dma_in(void) {
  unsigned int dma_in_next = (dma_in_last+1) & 0x3;

  CSR_WRITE(DMA_0_START_ADDR, DMA_IN_START(dma_in_next));
  CSR_WRITE(DMA_0_LENGTH, DMA_IN_LEN);
  CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff);  // start right away
  CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0); // any value
  dma_in_last = dma_in_next;
}

void setup_dma_in(void) {
  dma_ptr[0] = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));
  dma_ptr[1] = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));
  dma_ptr[2] = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));
  dma_ptr[3] = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));

  trigger_dma_in();
  trigger_dma_in();
  trigger_dma_in();
  trigger_dma_in();
  // the core consumes a schedule when it starts running
  // so this number is 4-1
  dma_in_expected_occupancy = 3; 
}

// void do_map_data(unsigned int dma_ptr_in) {
//   unsigned int *p = (unsigned int *) REVERSE_VMEM_DMA_ADDRESS(dma_ptr_in);
//   CSR_WRITE(RINGBUS_WRITE_ADDR, RING_ADDR_ETH);
//   CSR_WRITE(RINGBUS_WRITE_DATA, p[0]);
//   CSR_WRITE(RINGBUS_WRITE_EN, 0);

//   CSR_WRITE(RINGBUS_WRITE_DATA, p[1]);
//   CSR_WRITE(RINGBUS_WRITE_EN, 0);
// }

unsigned int dma_post_map;

void do_map_data(unsigned int dma_ptr) {
  dma_post_map = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));

  // vector_memory[0] = 0xfeafcafe;
  // vector_memory[1] = 0x0fffffff;

  // mapper_rload_bpsk(&bpsk_table, dma_post_map + NSLICES*0,  0);
  // mapper_lload_bpsk(&bpsk_table, dma_post_map + NSLICES*1,  0);

  // // second 32bit word translates to two rows
  // mapper_rload_bpsk(&bpsk_table, dma_post_map + NSLICES*2,  1);
  // mapper_lload_bpsk(&bpsk_table, dma_post_map + NSLICES*3,  1);

  // first 32bit words translates to two rows

  // input size in words, each word produces 32 outputs in bpsk

  unsigned int outputs;
  unsigned int size_words = 2;

  const unsigned int type_is_bpsk = 1;

  if(type_is_bpsk) {
    outputs = size_words*32;

    // assumes that the number of output rows will all fit in one vmalloc

    for(unsigned int i = 0; i < size_words; i++) {
      // accepts addresses in DMA format
      unsigned int rowa = 2*i;
      unsigned int rowb = (2*i) + 1;

      mapper_rload_bpsk(&bpsk_table, dma_post_map + NSLICES*rowa,  dma_ptr);
      mapper_lload_bpsk(&bpsk_table, dma_post_map + NSLICES*rowb,  dma_ptr);
    }

    // nark and crash
    // report_test_results(0xf02);
    // report_test_results(dma_post_map<<2);
    // while(1) {
    //   asm("j 0xE4");
    // }
    // dma pointer in, size in words
    do_allocate_subcarriers(dma_post_map, outputs);
  }

  vfree(&mgr, REVERSE_VMEM_DMA_ADDRESS(dma_post_map));
}

// copies the same inputs to a specific list of channels
void map_specific_channels_a(unsigned int* word_p, unsigned int* output_p) {
  unsigned int channels[8] = {111, 122, 133, 144, 866, 877, 888, 899};
  unsigned int flag;

  for(unsigned int j = 0; j < 1024; j++) {
    flag = 0;
    for(unsigned int k = 0; k < 8; k++) {
      if(j == channels[k]) {
        flag = 1;
        break;
      }
    }
    if(flag) {
      output_p[j] = *word_p;
    } else {
      output_p[j] = 0;
    }
  }
}

// has "pilot tones"
void map_specific_channels_b(unsigned int* word_p, unsigned int* output_p) {
  // fixme magic numbers
  unsigned int channels[8] = {111, 122, 133, 144, 866, 877, 888, 899};
  unsigned int pilot[2] = {333, 691};
  unsigned int flagdata, flagpilot;
  unsigned int pilot_value = 0x7FFF0000;

  for(unsigned int j = 0; j < 1024; j++) {
    flagdata = 0;
    flagpilot = 0;
    for(unsigned int k = 0; k < 8; k++) {
      if(j == channels[k]) {
        flagdata = 1;
        break;
      }
    }
    for(unsigned int k = 0; k < 2; k++) {
      if(j == pilot[k]) {
        flagpilot = 1;
        break;
      }
    }

    if(flagdata) {
      output_p[j] = *word_p;
    } else if(flagpilot) {
      output_p[j] = pilot_value;
    } else {
      output_p[j] = 0;
    }
  }
}

// dma_pointer, size in words
// takes a single subcarrier, optionally copies it, and produces 1024 outputs for ever input word
void do_allocate_subcarriers(unsigned int dma_ptr_in, unsigned int size_words) {

  // back out from dma_pointer to cpu_pointer
  unsigned int* cpu_ptr_in = (unsigned int *) REVERSE_VMEM_DMA_ADDRESS(dma_ptr_in);

  // vmalloc a chunk where we write full fft frames (over and over)
  unsigned int* cpu_allocated_channel = vmalloc_single(&mgr);

  int stride = 128;

  for(unsigned int i = 0; i < size_words; i++) {
    // map_every_n_word(cpu_ptr_in+i, stride, cpu_allocated_channel);
    // cpu pointer in, cpu pointer out

    // no pilots
    // map_specific_channels_a(cpu_ptr_in+i, cpu_allocated_channel);

    // with pilots
    map_specific_channels_b(cpu_ptr_in+i, cpu_allocated_channel);
    
    // send to pc
    report_test_results(*(cpu_ptr_in+i));

    // destroyes data in input, but we will re-write at next loop
    // accepts cpu pointer for input
    do_fft_wrap(cpu_allocated_channel);
  }

  vfree(&mgr, cpu_allocated_channel);
}











// void do_allocate_subcarriers(unsigned int dma_ptr_in, unsigned int size_in) {

//   unsigned int *p = (unsigned int *) REVERSE_VMEM_DMA_ADDRESS(dma_ptr_in);

//   unsigned int* allocated_channel_ptr = vmalloc_single(&mgr);


//   // for every input size, we make 1024 outputs
//   // this needs to be refactored

//   // CSR_WRITE(RINGBUS_WRITE_ADDR, RING_ADDR_ETH);
//   // for(unsigned int i = 0; i < size_in; i++) {
//     int w0 = 0x00007000;
//     int w1 = 0x00006000;
//     int w2 = 0x00003000;
//     int w3 = 0x00004000;

//     // map_single_word(&w0, allocated_channel_ptr);
//     // do_fft_wrap(allocated_channel_ptr);
//     // map_single_word(&w1, allocated_channel_ptr+(1024*1));
//     // do_fft_wrap(allocated_channel_ptr);
//     // map_single_word(&w2, allocated_channel_ptr+(1024*2));
//     // do_fft_wrap(allocated_channel_ptr);
//     // map_single_word(&w3, allocated_channel_ptr+(1024*3));
//     // do_fft_wrap(allocated_channel_ptr);

//     // map_every_n_word(&w0, 16, allocated_channel_ptr);

//     // place a debug tone in this pointer
//     map_debug_tone(allocated_channel_ptr);

//     // do_output_dma(allocated_channel_ptr);
//     // "used placed data" aka fft and send
//     do_fft_wrap(allocated_channel_ptr);
//     while(1) {
//       asm("j 0xE4");
//     }
//     do_fft_wrap(allocated_channel_ptr);

//     map_every_n_word(&w1, 16, allocated_channel_ptr);
//     do_fft_wrap(allocated_channel_ptr);



//     // for(unsigned int i = 0; i < 7; i++) {
//     //   report_test_results(allocated_channel_ptr[i+(0)]);
//     //   // report_test_results(allocated_channel_ptr[i+(1024*1)]);
//     // }
//     // report_test_results(allocated_channel_ptr);
//     // report_test_results(VMEM_DMA_ADDRESS(allocated_channel_ptr));
//     // do_output_dma(allocated_channel_ptr);


//     // for(unsigned int j = 0; j < 1024; j++) {
//     //   if(j == single_active_subcarrier) {

//     //   }
//     // }
//     // report_test_results(p[63-i]);
//     // // CSR_WRITE(RINGBUS_WRITE_DATA, p[i]);
//     // // CSR_WRITE(RINGBUS_WRITE_EN, 0);
//   // }
// }


void do_fft_wrap(unsigned int *cpu_ptr) {
  do_fft((unsigned int) cpu_ptr);
}


unsigned int single_active_subcarrier = 4;
// writes 1024 to output_row

// accepts cpu_pointer for word, and cpu_pointer for output
void map_single_word(unsigned int* word_p, unsigned int* output_p) {
  for(unsigned int j = 0; j < 1024; j++) {
    if(j == single_active_subcarrier) {
      output_p[j] = *word_p;
    } else {
      output_p[j] = 0;
    }
  }
}

void map_every_n_word(unsigned int* word_p, unsigned int stride, unsigned int* output_p) {
  for(unsigned int j = 0; j < 1024; j++) {
    if((j % stride) == 0) {
      output_p[j] = *word_p;
    } else {
      output_p[j] = 0;
    }
  }
}


int dmem_test_sin[] = {0, 11837, 22075, 29332, 32627, 31516, 26149, 17250, 6021, -6021, -17250, -26149, -31516, -32627, -29332 -22075};
void map_debug_tone(unsigned int* output_p) {
  for(unsigned int j = 0; j < 1024; j++) {
    if(j < ARRAY_SIZE(dmem_test_sin)) {
      output_p[j] = 0xffff & dmem_test_sin[j];
    } else {
      output_p[j] = 0;
    }
  }
}

// void do_allocate_subcarriers(unsigned int dma_ptr_in) {
//   unsigned int *p = (unsigned int *) REVERSE_VMEM_DMA_ADDRESS(dma_ptr_in);
//   // CSR_WRITE(RINGBUS_WRITE_ADDR, RING_ADDR_ETH);
//   for(unsigned int i = 0; i < 64; i++) {
//     report_test_results(p[63-i]);
//     // CSR_WRITE(RINGBUS_WRITE_DATA, p[i]);
//     // CSR_WRITE(RINGBUS_WRITE_EN, 0);
//   }
// }

// hacked version for internal usage only
// external DMA may interfear if firing
void check_dma_in_force(unsigned int w0, unsigned int w1) {
  unsigned int *p1 = REVERSE_VMEM_DMA_ADDRESS(dma_ptr[3]);
  p1[0] = w0;
  p1[1] = w1;
  do_map_data(dma_ptr[3]);
}

void check_dma_in(void) {
  unsigned int occupancy;
  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
  unsigned int i;

  if(occupancy != dma_in_expected_occupancy) {
    // move this early (and consume)
    // report_test_results(dma_in_last_consumed);
    dma_in_last_consumed = (dma_in_last_consumed+1) & 0x3;
    unsigned int pass_along = dma_ptr[dma_in_last_consumed]; // address in dma

    // report_test_results(0xf01);
    // report_test_results(pass_along<<2);
    // while(1) {
    //   asm("j 0xE4");
    // }

    // dma address
    do_map_data(pass_along);

    // report_test_results(pass_along);

    trigger_dma_in(); // will bump occupancy
    // dma_in_expected_occupancy--;

    // should_exit = 1;

    // unsigned int* pass_along_p = (unsigned int *) REVERSE_VMEM_DMA_ADDRESS(pass_along);
    // report_test_results(pass_along_p[0]);
    // report_test_results(pass_along_p[1]);

    // do_map_data(dma_in_last_consumed);
  }
  // check_dma = occupancy;
  // for(i=occupancy;i<dma_in_expected_occupancy;i++) {
  //   trig_in(); // this can be earler for better performance
  //   ring_send_pending++;
  // }
}


void setup_fft() {
    unsigned int tmp_row0 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
    unsigned int tmp_row1 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
    unsigned int tmp_row2 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
    unsigned int tmp_row3 = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
    
    // also kept as globals
    // fft_output_ptr = vmalloc_single(&mgr);
    // fft_input_ptr = vmalloc_single(&mgr);


  plan1024 = get_fft1024_plan(
      0, // junk, will set later
      tmp_row0,  // tmp
      tmp_row1,
      tmp_row2,
      tmp_row3,
      0 // junk, will set later
      );
}

void do_output_dma_block(unsigned int* cpu_ptr);

void do_fft(unsigned int input_ptr) {
  unsigned int* cpu_fft_output;

  // allocate output for the fft
  cpu_fft_output = (unsigned int) vmalloc_single(&mgr);

  // update the plan
  plan1024.data_location = VMEM_ROW_ADDRESS(input_ptr);
  plan1024.output_data_location_4 = VMEM_ROW_ADDRESS(cpu_fft_output);

  // run it
  fft_1024_run(&plan1024);

  // output dma and block until done;
  do_output_dma_block(cpu_fft_output);

  vfree(&mgr, cpu_fft_output);

}

// accepts a cpu pointer, assumes 1024 size words
void do_output_dma_block(unsigned int* cpu_ptr) {
  // pre-emptive force clear
  CSR_WRITE(DMA_1_INTERRUPT_CLEAR, 0);
  do_output_dma(cpu_ptr);

  // block until set, then clear and break
  int helper;
  while(1) {
    CSR_READ(mip, helper);
    if(helper & DMA_1_ENABLE_BIT) {
      CSR_WRITE(DMA_1_INTERRUPT_CLEAR, 0);
      break;
    }
  }

}

// accepts a cpu pointer, assumes 1024 size words
void do_output_dma(unsigned int input_ptr) {
  CSR_WRITE(DMA_1_START_ADDR, VMEM_DMA_ADDRESS(input_ptr));
  CSR_WRITE(DMA_1_LENGTH, 1024);
  CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);  // start right away
  CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0); // any value
}





int main(void)
{
  CSR_WRITE(DMA_0_FLUSH_SCHEDULE,  0);
  CSR_WRITE(DMA_1_FLUSH_SCHEDULE,  0);

  CSR_WRITE(GPIO_WRITE_EN, LED_GPIO_BIT);
  CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);
  // required for vmalloc to work

  report_test_results(OUR_RING_ENUM);

  Ringbus ringbus;



  // 10 blinks
  // for(unsigned int i = 0; i < 10; i++) {
  //   CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);
  //     check_ring(&ringbus);
  //   for(int j = 0; j < 100000; j++) {  // 10000
  //     check_ring(&ringbus);
  //   }
  //   CSR_WRITE(GPIO_WRITE, 0);
  //   for(int j = 0; j < 1000000; j++) {  // 1000000
  //     check_ring(&ringbus);
  //   }
  // }



  init_VMalloc(&mgr);

  // pour one out for my homies
  // vmalloc_single(&mgr);

  // delme
  // vmalloc_single(&mgr);
  // unsigned int orig = (unsigned int) vmalloc_single(&mgr);
  // report_test_results(OUR_RING_ENUM); // boot
  // report_test_results(VMEM_DMA_ADDRESS(orig));
  // report_test_results(REVERSE_VMEM_DMA_ADDRESS(VMEM_DMA_ADDRESS(orig)));
  // while(1){}





  setup_dma_in();
  setup_mapper();
  setup_fft();

  should_exit = 0;

  // set to 1, accept inbound dma and
  // set to 0, generate single fake "inbound dma" internally
  int normal = 0;

  if(normal) {
    while(!should_exit) {
      check_dma_in();
    }
  } else {
    // debug, this doesn't really check the inboudn dma
    // instead uses values provided as starts the rest of the chain
    check_dma_in_force(0x8765432a, 0xffffffff);
  }



  check_ring(&ringbus);


  while(1) {
    CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);
      check_ring(&ringbus);
    for(int j = 0; j < 100000; j++) {  // 10000
      check_ring(&ringbus);
    }
    CSR_WRITE(GPIO_WRITE, 0);
    for(int j = 0; j < 1000000; j++) {  // 1000000
      check_ring(&ringbus);
    }
  }


  // set_subcarrier_count(1);



  // CSR_WRITE(RINGBUS_WRITE_ADDR, RING_ADDR_CS30);

  // for(unsigned int i = 0; i < 1000; i++ )
  // {

  // }
  // CSR_WRITE(RINGBUS_WRITE_ADDR, 0);
 //    CSR_WRITE(RINGBUS_WRITE_DATA, 0x04cafebb);
 //    CSR_WRITE(RINGBUS_WRITE_EN, 0);
  // for(int i = OUTPUT_BUF_START; i < OUTPUT_BUF_START+DMA_LEN; i++) {
  //   vector_memory[i] = i;
  // }

  // vector_memory[0] = 0x2;

  // CSR_WRITE(DMA_1_START_ADDR, OUTPUT_BUF_START);
  // CSR_WRITE(DMA_1_LENGTH, DMA_LEN);
  // CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);  // start right away
  // CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0); // any value



}


int main2(void)
{
  CSR_WRITE(DMA_0_FLUSH_SCHEDULE,  0);
  CSR_WRITE(DMA_1_FLUSH_SCHEDULE,  0);

  CSR_WRITE(GPIO_WRITE_EN, LED_GPIO_BIT);
  CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);
  Ringbus ringbus;
  
  while(1) {
    CSR_WRITE(GPIO_WRITE, LED_GPIO_BIT);
      check_ring(&ringbus);
    for(int j = 0; j < 100000; j++) {  // 10000
      check_ring(&ringbus);
    }
    CSR_WRITE(GPIO_WRITE, 0);
    for(int j = 0; j < 1000000; j++) {  // 1000000
      check_ring(&ringbus);
    }
  }
  
}

