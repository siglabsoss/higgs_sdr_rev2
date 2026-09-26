#include "test_dma_torture.h"



int main(void)
{
  CSR_WRITE(DMA_0_FLUSH_SCHEDULE, 0);
  CSR_WRITE(DMA_1_FLUSH_SCHEDULE, 0);

  simple_random_seed(0x1c4bece3);

  run_receiver();

  run_sender();

  Ringbus ringbus;
  while(1) {
    check_ring(&ringbus);
}
}
