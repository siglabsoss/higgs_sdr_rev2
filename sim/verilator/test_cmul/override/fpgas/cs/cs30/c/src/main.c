
#include "xbaseband.h"
#include "vmem.h"
#include "csr_control.h"
#include "bootloader.h"
#include "pass_fail.h"
#include "ringbus.h"

// #define LOAD_FFT_CFG
// #include "vmem_cfg.h"
#include "config_word_cmul_rx4_0f.h"
#include "config_word_cmul_rx4_00.h"
#include "config_word_cmul_eq_0f.h"
#include "config_word_conj_eq_0f.h"
#include "config_word_add_eq_00.h"
#include "config_word_sub_eq_00.h"
#include "config_word_magsquare_eq_00.h"


#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS30
#include "ringbus2_post.h"

#define VECTOR_REPORT_ADDRESS 0x3800
#define CP_LENGTH 256

VMEM_SECTION unsigned int input_data_0[16]={0xc07c0381, 0xe3e1fc61, 0xf58a0a65, 0x3ca2ec2c, 0xf3d11ebe, 0xed4fe9bd, 0x0ab70583, 0x2048d86e, 0x0fbffdf4, 0x1165dbeb, 0x46ec1991, 0xb816fafa, 0xef68fe3b, 0x276a07d2, 0xf69028e0, 0x0d74fc6d};
VMEM_SECTION unsigned int input_data_1[16]={0x2666050c, 0xe1a3288a, 0x0598d4c6, 0x0df8c407, 0xfd56f479, 0xdead1956, 0x1365c2e0, 0xd8a416e7, 0xdc471ff4, 0x4c59e4a0, 0xe9041eea, 0xf4f6f603, 0xff720cc6, 0x2f32d3ed, 0xe019f98e, 0xeba1eefa};
VMEM_SECTION unsigned int output_benchmark[16]={0xfe8c1331, 0xf7f4f82f, 0x03fdfcf2, 0xe16d02ac, 0x0075fcfa, 0x0219f6ba, 0xfbb8fbbf, 0x11f202d9, 0x048103e2, 0xe6c2fd57, 0x0c8a12e9, 0x060cfa31, 0xfe5affc0, 0xf550eec6, 0xf64afb97, 0xfec8029e};
VMEM_SECTION unsigned int output_data[16]={0};






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

void main()
{
  Ringbus ringbus;
   
  unsigned int input_data_location_0=VMEM_ADDRESS(input_data_0);
  unsigned int input_data_location_1=VMEM_ADDRESS(input_data_1);
  unsigned int benchmark_location=VMEM_ADDRESS(output_benchmark);
  unsigned int output_data_location = VMEM_ADDRESS(output_data);

  unsigned int cfg_cmulti_location=VMEM_ADDRESS(config_word_cmul_eq_0f); 

  MVXV_KNOP(V0, cfg_cmulti_location);
  VNOP_LK14(V0);

  STALL(10);
  STALL(10);
  STALL(10);

  MVXV_KNOP(V1, input_data_location_0);
  MVXV_KNOP(V2, input_data_location_1);
  MVXV_KNOP(V3, output_data_location);
  VNOP_LK8(V1);
  VNOP_LK9(V2);
  VNOP_SK1(V3);


  STALL(10);
  STALL(10);
  STALL(10);
    
  int flag    = cmul_result_check(benchmark_location, output_data_location, 16, 10, 1, 1);
  report_test_results(0xdeadbeef);
  if(flag==1)
    report_test_results(0xf);
  else
    report_test_results(0x00000000);

  /*while(1) {
    check_ring(&ringbus);
    }*/
    


}


   







void data_show(int start_address, int num_data)
{
  register volatile unsigned int x3 asm("x3");
  x3=0xdeaddead;
  for(int index=0; index<num_data;index++)
    {
      x3=index;
      x3=vector_memory[start_address*16+index];
    }
  x3=0xbeefbeef;
  
}


int cmul_result_check(int benchmark_start_row, int result_start_row, int total_sample, int diff_th, int benchmark_gap, int result_gap)
{
  register volatile unsigned int x3 asm("x3");
  int result0, result1;
  int diff_imag, diff_real;
  // int diff_th = 10;
  int flag=1;

  x3=0x77777777;

  for(int index=0; index<total_sample;index++)
    {
    
      result0=vector_memory[benchmark_start_row*16+index*benchmark_gap];
      STALL(10);

  
    
      result1=vector_memory[result_start_row*16+index*result_gap];
      STALL(10);

    
      //report_test_results(0xcafebeef);
      //report_test_results(result0);
      //report_test_results(result1);
      diff_real=(result0&0x0000ffff) - (result1&0x0000ffff);

      if((diff_real&0x00008000) == 0x8000)
	diff_real=diff_real | 0xffff0000;

      diff_imag=((result0>>16)&0x0000ffff) - ((result1>>16)&0x0000ffff);
     
      if((diff_imag&0x00008000) == 0x8000)
	diff_imag=diff_imag | 0xffff0000;

      if((diff_real<diff_th)&&(diff_real>diff_th*(-1)))
	diff_real=0;

      if((diff_imag<diff_th)&&(diff_imag>diff_th*(-1)))
	diff_imag=0;
    
      x3=diff_real;
      x3=diff_imag;




      if((diff_real!=0)||(diff_imag!=0))
	{
	  x3=index;
	  x3=result0;
	  x3=result1;
	  flag=0;
	  break;
	}
    }
  x3=0x88888888;
  x3=flag;
  return flag;
}


