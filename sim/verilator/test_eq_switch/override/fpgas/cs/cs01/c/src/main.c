
#define STREAM_CHUNK (256)
#define RUN_LOOP_CALLBACK run_loop();

#include "dma.h"
#include "subtract_timers.h"
#include "csr_control.h"

#include "ringbus.h"
#include "ringbus2_pre.h"
#include "ringbus2_post.h"

static unsigned int underflow_then;

void reset_underflow_counter() {
    CSR_WRITE(CS_CONTROL, 0x1);
    CSR_WRITE(CS_CONTROL, 0x0);
}

unsigned grab_underflow_counter() {
    unsigned int riscv_status;
    CSR_READ(CS_STATUS, riscv_status);
    return riscv_status;
}

void ringbus_send_counter(unsigned counter) {
    ring_block_send_eth(TX_UNDERFLOW|0x030000|(counter&0xffff));
    ring_block_send_eth(TX_UNDERFLOW|0x040000|((counter>>16)&0xffff));
}


static unsigned pending_underflow_report = 0;

void request_underflow_report_callback(unsigned int data) {
    pending_underflow_report = 1;
}


#define UNDERFLOW_BECAME_OK  (0x000000)
#define UNDERFLOW_BECAME_BAD (0x010000)

// 0 is ok 
// 1 is underflowing

static unsigned underflow_state = 1; // start in underflowing state

void check_error_counter(void) {
    // CSR_WRITE(CS_CONTROL, err_state);

    unsigned int now;
    CSR_READ(TIMER_VALUE, now);

    unsigned counter_delta = subtract_timers(now,underflow_then);

    if(counter_delta > (125000000/10)) {
        underflow_then = now;

        unsigned int counter;
        counter = grab_underflow_counter();

        unsigned underflow_now = counter!=0;

        unsigned int transition = ((underflow_state & 0x1) << 1) | (underflow_now);

        if( pending_underflow_report ) {
            pending_underflow_report = 0;

            // if we are requesting a report
            // set the "then" bit to the opposite of the "now" bit
            // which will force a report

            transition = (transition & (~0x2)); // clear bit
            if(!underflow_now) {
                transition |= 0x2; // set bit if now is unset, otherwise, leave unset
            }
        }

        // bit 1           |    bit 0
        // was underflow   |    currently underflow

        switch(transition) {

            // was not underflowing, currently not underflowing
            case 0x0:
                // do nothing
                break;

            // was not underflowing, currently underflowing
            case 0x1:
                ring_block_send_eth(TX_UNDERFLOW|UNDERFLOW_BECAME_BAD);
                ringbus_send_counter(counter);
                underflow_state = 1;
                break;

            // was underflowing, currently not underflowing
            case 0x2:
                ring_block_send_eth(TX_UNDERFLOW|UNDERFLOW_BECAME_OK);
                underflow_state = 0;
                break;

            // was underflowing, currently underflowing
            case 0x3:
                // do nothing
                break;
            default:
                // illegal!
                break;
        }

        // ring_block_send_eth_debug(riscv_status);
        // ring_block_send_eth(riscv_status);
        // ring_block_send_eth(TX_UNDERFLOW|(riscv_status & 0xffffff));
        reset_underflow_counter();
        
    }
}


void run_loop() {
    check_error_counter();
}

#include "do_forward_stream.h"
#include "nco_data.h"

int main(void)
{
    ring_register_callback(&request_underflow_report_callback, REQUEST_UNDERFLOW_REPORT_CMD);
    no_exit_stream();
}


// #include "fill.h"
// #include "xbaseband.h"
// #include "apb_bus.h"
// #include "csr_control.h"
// #include "pass_fail.h"
// #include "turnstile.h"
// #include "symbol.h"
// //#include "symbol1.h"
// #include "ringbus.h"

// #include "ringbus2_pre.h"
// //#define OUR_RING_ENUM RING_ENUM_CS10
// #include "ringbus2_post.h"

// //#include "unit_test_ring.h"

// int main(void)
// {
//   Ringbus ringbus;
//   register volatile unsigned int x3 asm("x3");
//   register volatile unsigned int x4 asm("x4");

//   unsigned int i;
//   for(i = 0; i < 1024; i++)
//   {
//     vector_memory[i] = dmem_sin[i];
//     // vector_memory[i] = 0x7000;
//   }

//   x3 = 0;

//   unsigned int start_addr;
//   unsigned int data_len;
//   unsigned int start_time;

//   start_addr = 0;
//   data_len = 1024;
//   start_time = 0xffffffff;

//   CSR_WRITE(GPIO_WRITE_EN, LED_GPIO_BIT);
//   CSR_SET_BITS(GPIO_WRITE, LED_GPIO_BIT);
//   int led = 1;
//   check_ring(&ringbus);

//   unsigned int occupancy;
//   unsigned int h1;

//   int counter = 0;

//   CSR_WRITE(DMA_1_FLUSH_SCHEDULE, 0);
//   while(1) {
//     CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
//     check_ring(&ringbus);
//     if(occupancy < 2)
//     {
//       CSR_WRITE(DMA_1_START_ADDR, start_addr);
//       CSR_WRITE(DMA_1_LENGTH, data_len);
//       CSR_WRITE(DMA_1_TIMER_VAL, start_time);
//       CSR_WRITE(DMA_1_PUSH_SCHEDULE, 0);
//       x3++;
//       check_ring(&ringbus);
//     }

//     CSR_READ(mip, h1);
//     if(h1 & DMA_1_ENABLE_BIT) {
//       CSR_WRITE(DMA_1_INTERRUPT_CLEAR, 0);
//       check_ring(&ringbus);
//     }

//     counter++;
//     if(counter == 100000) {
//       CSR_SET_BITS(GPIO_WRITE, LED_GPIO_BIT);
//     }

//     if(counter == 1000000) {
//       counter = 0;
//       CSR_CLEAR_BITS(GPIO_WRITE, LED_GPIO_BIT);
//     }

//   }
// }