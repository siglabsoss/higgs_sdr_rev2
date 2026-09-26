#include "test_dma_torture.h"



int main(void)
{
  CSR_WRITE(DMA_0_FLUSH_SCHEDULE, 0);
  CSR_WRITE(DMA_1_FLUSH_SCHEDULE, 0);

  simple_random_seed(0xb33a3780); // 0xa4fe12de

  run_receiver();

  run_sender();

  Ringbus ringbus;
  while(1) {
    check_ring(&ringbus);
}
}
