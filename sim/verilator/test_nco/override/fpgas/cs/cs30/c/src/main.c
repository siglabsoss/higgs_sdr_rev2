#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "dma.h"
#include "fill.h"
#include "mover.h"
#include "mapper.h"
#include "ringbus.h"
#include "vmem.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "bootloader.h"

#define nco_length 32768

/**
 *
 * Creates the following signal f(wt) = exp(j*(wt + theta)). This function
 * has a default theta and w value.
 *
 * Args:
 *     data_length (unsigned int): Amount of samples to represent signal
 *
 */
void create_nco(unsigned int data_length) {
    unsigned int occupancy;

    CSR_WRITE(NCO_START_ANGLE, 0xbfffffff);  // pi = 0x7fffffff
    CSR_WRITE(NCO_LENGTH, data_length);
    CSR_WRITE(NCO_DELTA, 1<<20); // delta=freq*131.072
    CSR_WRITE(NCO_PUSH_SCHEDULE, 0);

    CSR_WRITE(DMA_2_START_ADDR, 0);
    CSR_WRITE(DMA_2_LENGTH, data_length);
    CSR_WRITE(DMA_2_TIMER_VAL, START_IMMED_TIME);
    CSR_WRITE(DMA_2_PUSH_SCHEDULE, 0);

    // Block until done
    while(1) {
        CSR_READ(DMA_2_SCHEDULE_OCCUPANCY, occupancy);
        if(occupancy == 0) {
            break;
        }
    }

}

/**
 *
 * Pass data in CS30 vector memory to ETH. Data is passed in chunks of 367
 * 32 bit values with a data rate of 25 MBytes/sec. This particular speed was
 * selected because the maximum data rate of the Etherent cable is
 * 50 MBytes/sec. To increase or decrease the data rate, edit `cycle_delay` 
 *
 * Args:
 *     data_length (unsigned int): Size of data to pass to ETH.
 *
 */
void pass_data_to_eth(unsigned int data_length) {
    unsigned int odma_occupancy;
    unsigned int read_interrupt;
    unsigned int cycle_delay = 25000;
    unsigned int idma_start_addr = 0;
    unsigned int dma_packet_size = 367;
    global_dma_data_len = data_length;
    unsigned int pass_data_counter = global_dma_data_len;
    unsigned int timer_a, timer_b;

    // Pass data in chunks defined by dma_packet_size
    while((pass_data_counter > 0) && (pass_data_counter < 0x10000)){
        CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, odma_occupancy);
        if(odma_occupancy < 2){

            CSR_WRITE(DMA_1_START_ADDR, idma_start_addr);
            CSR_WRITE(DMA_1_LENGTH, dma_packet_size);
            CSR_WRITE(DMA_1_TIMER_VAL, START_IMMED_TIME);
            CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0);
            CSR_READ(TIMER_VALUE, timer_a);

            pass_data_counter -= dma_packet_size;
            idma_start_addr += dma_packet_size;

            while(1) {
                CSR_READ(TIMER_VALUE, timer_b);
                if(timer_b - timer_a > cycle_delay) {
                    break;
                }
            } 
        }

        CSR_READ(mip, read_interrupt);
        if(read_interrupt & DMA_1_ENABLE_BIT) {
            CSR_WRITE(DMA_1_INTERRUPT_CLEAR, 0);
        }
    }
}

/**
 *
 * A function to test the NCO. It creates an oscillator and passes the data to
 * ETH for verification in Python
 *
 * Args:
 *     data (unsigned int): The size of NCO
 *
 */
void test_nco(unsigned int data) {
    create_nco(data);
    pass_data_to_eth(data);
}

/**
 * Checks and execute incoming ringbus commands.
 *
 * Args:
 *     *ringbus (Ringbus): A pointer to a ringbus struct
 *
 */
void listen_cmd(Ringbus *ringbus){
    CSR_WRITE(GPIO_WRITE_EN, LED_GPIO_BIT);
    while(1) {
        CSR_SET_BITS(GPIO_WRITE, LED_GPIO_BIT);
            check_ring(ringbus);
        for(int j = 0; j < 100000; j++) {
            check_ring(ringbus);
        }
        CSR_CLEAR_BITS(GPIO_WRITE, LED_GPIO_BIT);
        for(int j = 0; j < 1000000; j++) {
            check_ring(ringbus);
        }
    }
}

int main(void) {
    Ringbus ringbus;

    ring_register_callback(&test_nco, NCO_TEST_CMD);
    listen_cmd(&ringbus);
    
    return 0;
}
