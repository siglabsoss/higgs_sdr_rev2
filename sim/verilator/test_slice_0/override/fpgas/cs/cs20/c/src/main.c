
#include "xbaseband.h"
#include "vmem.h"
#include "csr_control.h"
#include "bootloader.h"
#include "pass_fail.h"
#include "sig_utils.h"
#include "ringbus.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"


// call this ONLY ONE TIME per project to offset the beginning of vmem allocation
VMEM_SECTION_OFFSET_WORDS(0);

// we need a dma location to send stuff out to the testbench
// #define VECTOR_REPORT_ADDRESS (2048)

/////////////////
//
// there is a bug, (saved on branch higgs_sdr_rev2/bug_fast_dus)
//
//
// where repeaded dbus loops causes dBus_rsp_ready to come back 1 clk early
// This is a simple define which allows us test or avoid the bug
//

#define DMEM_RSP_READY_FIX()
// #define DMEM_RSP_READY_FIX()  asm("nop");asm("nop");



// do { \
// vector_memory[VECTOR_REPORT_ADDRESS] = x; \
// CSR_WRITE(DMA_1_START_ADDR, VECTOR_REPORT_ADDRESS); \
// CSR_WRITE(DMA_1_LENGTH, 1); \
// CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff); \
// CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0); \
// } while (0)

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


void xbb_multiple_unit_test(void)
{
  *pass_fail_0=0x00000000;


  // // // // // // --- 0
  // // individual opcode test   
  int rs1=0;
  unsigned int a, b, c;
  a = 0x10;
  MVXV_KNOP(V0, a);
  b = 0x11;
  MVXV_KNOP(V1, b);
  c = 0x13;
  MVXV_KNOP(V2, c);
  ADD_KNOP(V2, V1, V0, rs1);
  // for demo pass/fail purpose
  MVVK15_KNOP(V2);
  MVXV_KNOP(V0,0);
  VNOP_SK15(V0);
  STALL(10);
  int answer0=a+b;
  if(vector_memory[0]==answer0)
  {
    *pass_fail_0 = *pass_fail_0 | 0x1;
  }


  

  // // // // //--- 1
  // // add overflow test 
  rs1=0x0;
  a = 0xf7ff;
  MVXV_KNOP(V0, a);
  b = 0xfdff;
  MVXV_KNOP(V1, b);
  ADD_KNOP(V2, V1, V0, rs1); 
  // for demo pass/fail purpose
  MVVK15_KNOP(V2);
  MVXV_KNOP(V0,0);
  VNOP_SK15(V0);
  asm("nop");
  int c1 = a+b;
  int d1 = c1 & 0x00000fff;
  int e1 = ((a&0x0000f000)+(b&0x0000f000));
  int answer1 = (e1&0x0000f000)+d1;
  STALL(10);
  if(vector_memory[0]==answer1)
  {
    *pass_fail_0 = *pass_fail_0 | 0x2;
  }



  // // // // // --- 2
  // // sub test 
  rs1=0x0;
  a = 0x4;
  MVXV_KNOP(V1, a);
  b = 0x100;
  MVXV_KNOP(V0, b);
  SUB_KNOP(V2, V1, V0, rs1); 
  // for demo pass/fail purpose
  MVVK15_KNOP(V2);
  MVXV_KNOP(V0,0);
  VNOP_SK15(V0);
  int c2 = a-b;
  int d2 = c2 & 0x00000fff;
  int e2 = ((a&0x0000f000)-(b&0x0000f000));
  int answer2 = (e2&0x0000f000)+d2;
  STALL(10);
  if(vector_memory[0]==answer2)
  {
    *pass_fail_0 = *pass_fail_0 | 0x4;
  }


  // // // // // // --- 3
  // // // simple datapath test
  // int flag3=1;
  // vector_memory[0]=0xffffffff;
  // for(int i=1;i<0x000000ff;i++)
  // {
  //   vector_memory[i]=i-1;
  // }
  // MVXV_KNOP(V0, 1);
  // VNOP_LK8(V0);
  // MVXV_KNOP(V0, 2);
  // VNOP_LK9(V0);
  // MVXV_KNOP(V0, 3);
  // VNOP_LK10(V0);
  // MVXV_KNOP(V0, 4);
  // VNOP_LK11(V0);
  // MVXV_KNOP(V0, 0);
  // VNOP_LK14(V0);
  // VNOP_SK1(V0);
  // // for demo pass/fail purpose
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // if(vector_memory[0]!=0xffffffdf)
  // {
  //   flag3=0;
  // }
  // VNOP_SK2(V0);
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // asm("nop");
  // if(vector_memory[0]!=0xffffffdf)
  // {
  //   flag3=0;
  // }
  // if(flag3==1)
  // {
  //   *pass_fail_0 = *pass_fail_0 | 0x8;
  // }


  // // // // // --- 4
  // // compound opcode test
  int flag4 = 1;
  for(int i=0;i<0x000000ff;i++)
  {
    vector_memory[i]=i;
  }
  a=0x0;
  b=0x4;



  MVXV_KNOP(V0, a);
  MVXV_KNOP(V1, b);
  SUB_LK8(V4, V1, V0);
  c=0x1;
  MVXV_KNOP(V0, c);
  ADD_LK9(V3, V1, V0);
  // for demo pass/fail purpose
  MVVK15_KNOP(V4);
  MVXV_KNOP(V0);
  VNOP_SK15(V0);
  STALL(10);
  if(vector_memory[0]!=b-a)
  {
    flag4=0;
  }


  
  MVVK15_KNOP(V3);
  MVXV_KNOP(V0);
  VNOP_SK15(V0);
  STALL(10);
  if(vector_memory[0]!=b+c)
  {
    flag4=0;
  }
  if(flag4==1)
  {
    *pass_fail_0 = *pass_fail_0 | 0x10;
  }



  // // // // // 
  // // permutation network test --- load
  // rs1 = 0xfffffffe;
  // for(int i=0;i<32;i++)
  // {
  //   //test one
  //   // MVXV_KNOP(V2, (i<<11)+i);
  //   // MV_KNOP(V0, V2, rs1);
  //   // rs1=(rs1<<1)+1;
    
  //   //test two
  //   //MVXV_KNOP(V0, 0x1000);

  //   //test three
  //   // MVXV_KNOP(V2, ((31-i)<<11));
  //   // MV_KNOP(V0, V2, rs1);
  //   // rs1=(rs1<<1)+1;

  //   //test four
  //   // MVXV_KNOP(V2, ((i%4)<<11));
  //   // MV_KNOP(V0, V2, rs1);
  //   // rs1=(rs1<<1)+1;

  //   //test five
  //   // MVXV_KNOP(V2, ((i>>2)<<11));
  //   // MV_KNOP(V0, V2, rs1);
  //   // rs1=(rs1<<1)+1;

  //   //test six
  //   MVXV_KNOP(V2, ((i%4)<<11)+i);
  //   MV_KNOP(V0, V2, rs1);
  //   rs1=(rs1<<1)+1;
  // }
  // for(unsigned int i=0;i<0x00000400;i++)
  // {
  //   vector_memory[i]=i;
  // }
  // VNOP_LK15(V0);

  // // // // // --- 5
  // // permutation network test ---  load & store
  int flag5 = 1;
  rs1 = 0xfffffffe;
  for(int i=0;i<16;i++)
  {
    MVXV_KNOP(V2, (0<<12));
    MV_KNOP(V0, V2, rs1);
    rs1=(rs1<<1)+1;
  }
  STALL(10);
  for(unsigned int i=0;i<0x00000010;i++)
  {
    vector_memory[i]=i;
  }
  STALL(10);
  VNOP_LK13(V0);
  
  STALL(10);
  rs1 = 0xfffffffe;
  for(int i=0;i<16;i++)
  {
    MVXV_KNOP(V2, ((0)<<12)+1);
    MV_KNOP(V1, V2, rs1);
    rs1=(rs1<<1)+1;
  }
  VNOP_SK13(V1);
  // for demo pass/fail purpose
  STALL(10);

  // this loop fails with fast_dbus only with -O3
  for(int i=0;i<16;i++)
  {
    if(vector_memory[0x10+i]!=i)
    {
      flag5=0;
      DMEM_RSP_READY_FIX();
    }
  }
  STALL(10);
  if(flag5==1)
  {
    *pass_fail_0 = *pass_fail_0 | 0x20;  // this bit fails with fast_dbus
  }



  // // // // // --- 6
  // // permutation network test ---  load & store
  int flag6 = 1;
  rs1 = 0xfffffffe;
  for(int i=0;i<16;i++)
  {
    MVXV_KNOP(V2, (0<<12));
    MV_KNOP(V0, V2, rs1);
    rs1=(rs1<<1)+1;
  }
  for(unsigned int i=0;i<0x000000ff;i++)
  {
    vector_memory[i]=i;
  }
  STALL(10);
  VNOP_LK13(V0);

  rs1 = 0xfffffffe;
  for(int i=0;i<16;i++)
  {
    MVXV_KNOP(V2, (0<<12));
    MV_KNOP(V1, V2, rs1);
    rs1=(rs1<<1)+1;
  }
  VNOP_SK13(V1);
  // for demo pass/fail purpose
  asm("nop");
  for(int i=0;i<16;i++)
  {
    if(vector_memory[i]!=(i))
    {
      flag6=0;
      DMEM_RSP_READY_FIX();
    }
  }
  if(flag6==1)
  {
    *pass_fail_0 = *pass_fail_0 | 0x40; // this bit fails with fast_dbus
  }
  
  // // // // // --- 7
  // // permutation network test ---  load & store
  int flag7 = 1;
  rs1 = 0xfffffffe;
  for(int i=0;i<16;i++)
  {
    MVXV_KNOP(V2, (0<<12)+i);
    MV_KNOP(V0, V2, rs1);
    rs1=(rs1<<1)+1;
  }
  STALL(10);
  for(unsigned int i=0;i<0x00000400;i++)
  {
    vector_memory[i]=i;
  }
  STALL(10);
  VNOP_LK13(V0);

  rs1 = 0xfffffffe;
  for(int i=0;i<16;i++)
  {
    MVXV_KNOP(V2, (0<<12));
    MV_KNOP(V1, V2, rs1);
    rs1=(rs1<<1)+1;
  }
  VNOP_SK13(V1);
  // for demo pass/fail purpose
  STALL(10);
  // this loop fails with fast_dbus only with -O3
  for(int i=0;i<16;i++)
  {
    if(vector_memory[i]!=(0x10*i+i))
    {
      flag7=0;
      DMEM_RSP_READY_FIX();
    }
  }
  if(flag7==1)
  {
    *pass_fail_0 = *pass_fail_0 | 0x80; // this bit fails with fast_dbus
  }

  // // // // // --- 8
  // // ROR test
  int flag8 = 1;
  rs1=0xfffffffe;
  for(int i=0;i<16;i++)
  {
    MVXV_KNOP(V2, i);
    MV_KNOP(V0, V2, rs1);
    rs1=(rs1<<1)+1;
  }
  ROR_KNOP(V7,V0, 0x0);
  // for demo pass/fail purpose
  MVVK15_KNOP(V7);
  rs1=0xfffffffe;
  // int aaaa[32];
  // aaaa[0]=4;
  for(int i=0;i<16;i++)
  {
    MVXV_KNOP(V2, 0<<12);
    MV_KNOP(V0, V2, rs1);
    rs1=(rs1<<1)+1;
  }
  VNOP_SK15(V0);
  STALL(10);
  for(int i=0;i<16;i++)
  {
    if(vector_memory[i]!=(i+1)%16)
    {
      flag8=0;
      DMEM_RSP_READY_FIX();
    }
  }
  if(flag8==1)
  {
    *pass_fail_0 = *pass_fail_0 | 0x100;
  }

  // // // // // --- 9
  // // ROL test
  int flag9 = 1;
  rs1=0xfffffffe;
  for(int i=0;i<16;i++)
  {
    MVXV_KNOP(V2, i);
    MV_KNOP(V0, V2, rs1);
    rs1=(rs1<<1)+1;
  }
  ROL_KNOP(V7,V0, 0x0);
  // for demo pass/fail purpose
  MVVK15_KNOP(V7);
  rs1=0xfffffffe;
  for(int i=0;i<16;i++)
  {
    MVXV_KNOP(V2, 0<<12);
    MV_KNOP(V0, V2, rs1);
    rs1=(rs1<<1)+1;
  }
  VNOP_SK15(V0);
  STALL(10);
  for(int i=0;i<16;i++)
  {
    if(vector_memory[i]!=(i-1+16)%16)
    {
      flag9=0;
      DMEM_RSP_READY_FIX();
    }
  }
  if(flag9==1)
  {
    *pass_fail_0 = *pass_fail_0 | 0x200;
  }

  // // // // // // --- 10
  // // // double load and store
  int flag10 = 1; 

  for(unsigned int i=0;i<0x00000400;i++)
  {
    vector_memory[i]=i;
  }

  rs1 = 0xfffffffe;
  for(int i=0;i<16;i++)
  {
    MVXV_KNOP(V2, (0<<12)+0);
    MV_KNOP(V0, V2, rs1);
    rs1=(rs1<<1)+1;
  }
  VNOP_LK13(V0);

  // rs1 = 0xfffffffe;
  // for(int i=0;i<16;i++)
  // {
  //   MVXV_KNOP(V2, (i<<12)+5);
  //   MV_KNOP(V0, V2, rs1);
  //   rs1=(rs1<<1)+1;
  // }
  // VNOP_SK15(V0);

  rs1 = 0xfffffffe;
  for(int i=0;i<16;i++)
  {
    MVXV_KNOP(V2, (0<<12)+1);
    MV_KNOP(V0, V2, rs1);
    rs1=(rs1<<1)+1;
  }
  VNOP_LK13(V0);


  rs1 = 0xfffffffe;
  for(int i=0;i<16;i++)
  {
    MVXV_KNOP(V2, (0<<12)+5);
    MV_KNOP(V0, V2, rs1);
    rs1=(rs1<<1)+1;
  }
  VNOP_SK13(V0);

 
  rs1 = 0xfffffffe;
  for(int i=0;i<16;i++)
  {
    MVXV_KNOP(V2, (0<<12)+6);
    MV_KNOP(V0, V2, rs1);
    rs1=(rs1<<1)+1;
  }
  VNOP_SK13(V0);
  STALL(3);

  for(int i=0;i<16*2;i++)
  {
    if(vector_memory[i+16*5]!=i)
    {
      flag10=0;
      DMEM_RSP_READY_FIX();
    }
  }


  if(flag10==1)
  {
    *pass_fail_0 = *pass_fail_0 | 0x400;
  }




  // expected value to pass is 0x7f7
  // if this changes you must edit tb.cpp

  report_test_results(*pass_fail_0);
}



int main(void)
{
    for(unsigned int i = 0; i < 7000; i++) {
        STALL(1);
    }
  Ringbus ringbus;
  // tb is expecting this
  report_test_results(0xdeadbeef);

  xbb_multiple_unit_test();

  /*while(1) {
    check_ring(&ringbus);
    }*/
  
  return 0;
}
