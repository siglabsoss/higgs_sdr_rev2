#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "ringbus.h"
#include "bootloader.h"
// #include "gain_control.h"
#include "sig_utils.h"
#include "blinklib.h"
#include "mapmov.h"
#include "uart_driver.h"
#include "subtract_timers.h"
#include "handle_generic_op.h"

#include <stdint.h>
#include <stdbool.h>

#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "random.h"

void setup_mapmov(void);

// this is a magic number, and should be compiled in by the tool, per riscv
#define HACKED_VMEM_DEPTH_WORDS (4096*16)

// must be power of 2
#define DMA_IN_COUNT (32)
// how big is each packet data payload, aka dma trigger size
#define DMA_IN_LEN (2)
#define DMA_IN_OFFSET (VMEM_DMA_ADDRESS(dma_in))

// mask here must be (DMA_IN_COUNT-1)
#define DMA_IN_NEXT_FOR(given) (((given)+1) & (DMA_IN_COUNT-1))

VMEM_SECTION unsigned int dma_in[DMA_IN_COUNT*DMA_IN_LEN];

#define DMA_IN_START(x) (DMA_IN_OFFSET+(DMA_IN_LEN*(x)))

unsigned int dma_in_last;
unsigned int dma_in_last_consumed;
unsigned int dma_in_expected_occupancy;

// 4 is probably too big for output but we do not care
#define DMA_OUT_COUNT (32)
#define DMA_OUT_LEN (1)
VMEM_SECTION unsigned int dma_out[DMA_OUT_COUNT*DMA_OUT_LEN];
#define DMA_OUT_OFFSET (VMEM_DMA_ADDRESS(dma_out))
#define DMA_OUT_START(x) (DMA_OUT_OFFSET+(DMA_OUT_LEN*(x)))
#define DMA_OUT_NEXT_INDEX() ((dma_out_last+1) & 0x3)
unsigned int dma_out_last;
// unsigned int dma_out_last_consumed = DMA_OUT_COUNT;
// just a guess, this is padding, we need to redo bootloader in eth
// currenlty no guarentee eth's bootloader won't run off into another section
// #define ETH_SPECIAL_BOOTLOAD_ARRAY_PADDING (0x400)
// VMEM_SECTION unsigned int literal_actual_bootload[0x1f00 + ETH_SPECIAL_BOOTLOAD_ARRAY_PADDING];
// #define BOOTLOADER_START (VMEM_DMA_ADDRESS(literal_actual_bootload))
unsigned int ring_send_pending;

uint32_t total_ring_sent = 0;
uint32_t disable_uart_readback = 0;


// void set_tx_channel(unsigned int channel){
//     CSR_WRITE(GPIO_WRITE_EN, ALL_GPIO_PIN);
//     CSR_CLEAR_BITS(GPIO_WRITE, channel);
//     for(unsigned int i = 0; i < 1000000*3; i++) {}
// }

uint32_t ringbus_overflow_counts = 0;

void detect_ringbus_buffer_overflow(
    unsigned int _dma_in_next,
    unsigned int _dma_in_last_consumed) {

    // _dma_in_next growing fast, wraps around to 8 when _dma_in_last_consumed is at 9

    if(_dma_in_next == _dma_in_last_consumed) {
        ringbus_overflow_counts++;
        // SET_REG(x3,0xffffffff);
        // SET_REG(x4,0xffffffff);
    }

}

void trig_in(void)
{
    // one to trig is next is last+1
  unsigned int dma_in_next = DMA_IN_NEXT_FOR(dma_in_last);//  (dma_in_last+1) & 0x3;

  // at this point we are triggering a future DMA which dooms the memoryu
  // we need to check if we overflowd


  // SET_REG(x3, 0x20000000 | dma_in_next);
  // SET_REG(x3, 0x40000000 | dma_in_last_consumed); 

  detect_ringbus_buffer_overflow(dma_in_next, dma_in_last_consumed);
  // x4 = dma_in_next;

  // the DMA_IN_START() macro here converts the index to a DMApointer
  CSR_WRITE(DMA_0_START_ADDR, DMA_IN_START(dma_in_next));
  CSR_WRITE(DMA_0_LENGTH, DMA_IN_LEN);
  CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff);  // start right away
  CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE); // any value

  dma_in_last = dma_in_next;
}

// do not call unless output DMA has occupancy
void trig_out(void)
{
  unsigned int dma_out_next = DMA_OUT_NEXT_INDEX();

  CSR_WRITE(DMA_1_START_ADDR, DMA_OUT_START(dma_out_next));
  CSR_WRITE(DMA_1_LENGTH, DMA_OUT_LEN);
  CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);  // start right away
  CSR_WRITE_ZERO(DMA_1_PUSH_SCHEDULE); // any value

  dma_out_last = dma_out_next;
}

// writes to memory and then fires off DMA
// do not call unless output DMA has occupancy
void queue_dma_out(unsigned int data) {
    // SET_REG(x3, 0x10000000);
    vector_memory[DMA_OUT_START(DMA_OUT_NEXT_INDEX())] = data;
    trig_out();
}

void queue_dma_out_blocking(unsigned int data) {
    // SET_REG(x3, 0x10000000);
    unsigned int occupancy;
    
    // block before we write to vector memory
    // this should prevent any corruption
    while(1) {
        CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
        if( occupancy < DMA_1_SCHEDULE_DEPTH) {
            break;
        }
    }

    queue_dma_out(data);
}

void eth_self_test(unsigned int message) {

    // reply with 10 packets
    if( message == 1 ) {
        for(unsigned int i = 0; i < 10; i++) {
            STALL(50);
            // FIXME: this does not check occupancy
            queue_dma_out(ETH_TEST_CMD | i);
        }
    }

    // send a ringbus that every other fpga will passthrough
    if( message == 2 ) {
        // should wrap around and come back to us
        CSR_WRITE(RINGBUS_WRITE_ADDR, (RING_BUS_LENGTH-1) );
        CSR_WRITE(RINGBUS_WRITE_DATA, 0xdeadbeef);
        CSR_WRITE_ZERO(RINGBUS_WRITE_EN);
    }
}

void readback_timer_eth(const unsigned int word) {
    unsigned timer;
    CSR_READ(TIMER_VALUE, timer);

    if(word == 0) {
        const unsigned hi = (timer>>16) & 0xffff;
        const unsigned low = timer & 0xffff;

        const unsigned us = (OUR_RING_ENUM&0xf)<<20;

        queue_dma_out(TIMER_RESULT_PCCMD | us | 0x00000 | low);
        queue_dma_out(TIMER_RESULT_PCCMD | us | 0x10000 | hi);
    }
}


void handle_internal_cmd(unsigned int data_packet){
    unsigned int type = data_packet&TYPE_MASK;
    unsigned int data = data_packet&DATA_MASK;

    switch(type) {
        case BOOTLOADER_CMD:
            // SET_REG(x3, 0xdead100d);
            no_exit_bootload(data, 1);
            break;
        case ETH_TEST_CMD:
            eth_self_test(data);
            break;
        // case CONFIG_DAC_CMD:
        //     config_dac(data);
        //     break;
        // case DISABLE_DAC_CMD:
        //     disable_dac(data);
        //     break;
        // case TX_CHANNEL_CMD:
        //     set_tx_channel(data);
        //     break;
        // case VGA_GAIN_CMD:
        //     set_vga_gain(data);
        //     break;
        // case DSA_GAIN_CMD:
        //     set_dsa_gain(data);
        //     break;
        case MAPMOV_RESET_CMD:
            setup_mapmov();
            break;
        case MAPMOV_MODE_CMD:
            mapmov_choose_mode(data, 0);
            break;
        case REQUEST_MAPMOV_REPORT:
            mapmov_report_settings();
            break;
        case UART_PUT_CHAR_CMD:
            uart_put_char(data);
            break;
        case GENERIC_OPERATOR_CMD:
            handle_generic_callback_original(data);
            break;
        case GET_TIMER_CMD:
            readback_timer_eth(data);
            break;
        default: // fixme add a counter for illegal commands arriving here?
            break;
    }
}

// only call this if guarenteed to be occupancy
// left in outbound ringbus
void to_ring(int index) {
    unsigned int addr;
    unsigned int data_packet;
    unsigned int mem_start = DMA_IN_START(index);

    addr        = vector_memory[mem_start + 0] & 0xff;
    data_packet = vector_memory[mem_start + 1];

    // SET_REG(x3, 0x30000000 | addr);
    // SET_REG(x4, data_packet);

    CSR_WRITE(RINGBUS_WRITE_ADDR, addr);
    CSR_WRITE(RINGBUS_WRITE_DATA, data_packet);
    CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

    total_ring_sent++;
}


void setup_dma_in(void) {
  trig_in();
  trig_in();
  trig_in();
  trig_in();
  // the core consumes a schedule when it starts running
  // so this number is 4-1
  dma_in_expected_occupancy = 4; 
}

// FIXME:
// re-write to skip our own first ringbus, and send directly out of the 2nd
void handle_single_dma_in(void)
{
  unsigned int occupancy;
  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
  unsigned int i;
  // check_dma = occupancy;
  for(i=occupancy;i<dma_in_expected_occupancy;i++) {
    trig_in(); // this can be earler for better performance
    ring_send_pending++;
    // check_dma = 0xd0000000 | ring_send_pending;
  }
}

void handle_pending_ring(void){
  unsigned int occupancy;
  while(ring_send_pending) {

    CSR_READ(RINGBUS_SCHEDULE_OCCUPANCY, occupancy);
    if( occupancy < RINGBUS_SCHEDULE_DEPTH ) {
      dma_in_last_consumed = DMA_IN_NEXT_FOR(dma_in_last_consumed);// (dma_in_last_consumed+1) & 0x3;
      // x4 = dma_in_last_consumed;
      to_ring(dma_in_last_consumed);
      ring_send_pending--;
    } else {
        break;
    }
  }
}

// a message for the first ringbus is sent to the computer
void handle_multi_ring_in(unsigned occ) {
  unsigned a;
  for(unsigned i = 0; i < occ; i++) {
    CSR_READ(RINGBUS_READ_DATA, a);
    queue_dma_out(a);
  }
}

// a message for the 2nd ringbus is handled internal
void handle_multi_ring_2_in(unsigned occ) {
  unsigned a;
  for(unsigned i = 0; i < occ; i++) {
    CSR_READ(RINGBUS_2_READ_DATA, a);
    handle_internal_cmd(a);
  }
}

unsigned int fill_level_p = 0;

// void pet_fill_level2(void) {
//     // this is the value in eth_top.sv which we need to write to check fill level
//     unsigned int control_data = 0x00001000;
//     unsigned int fill_level;

//     CSR_WRITE(CS_CONTROL, control_data);
//     CSR_READ(CS_STATUS, fill_level);

//     if( fill_level != fill_level_p ) {

//         if( fill_level ) {
//             // send full command
//             queue_dma_out(FILL_REPLY_PCCMD | 0x1);
//         } else {
//             // send empty command
//             queue_dma_out(FILL_REPLY_PCCMD | 0x0);
//         }

//         fill_level_p = fill_level;
//     }
// }


unsigned int last_fill_report = 0;

unsigned int fill_report_minimum_delay = 20000;

void pet_fill_level(void) {
    // this is the value in eth_top.sv which we need to write to check fill level
    const unsigned int control_data = 0x00001001;
    unsigned int fill_level;
    unsigned int now;

    CSR_WRITE(CS_CONTROL, control_data);
    CSR_READ(CS_STATUS, fill_level);

    if( fill_level != fill_level_p ) {
        CSR_READ(TIMER_VALUE, now);

        if( (now - last_fill_report) > fill_report_minimum_delay ) {

            queue_dma_out(FILL_REPLY_PCCMD | fill_level);

            fill_level_p = fill_level;

            last_fill_report = now;

        } else {
            return;
        }

    }
}

uint32_t force_error = 0;
uint32_t debug_error_hits = 0;

void software_errors(unsigned int err_state, unsigned int* load) {
    switch(err_state) {

/*
warning do not use as this overwrites hardware value 2
only used for testing of "random" blink pattersn
        case 2:
            // 0x10000
            // if random value is low AND we've only sent this a few times
            if( (simple_random() < 0xb000) && (debug_error_hits < 0x999) ) {
                *load = debug_error_hits;
                debug_error_hits++;
            } else {
                *load = 0;
            }
            break;

*/

// hardware has no 9
        case 9:
            *load = force_error;
            break;
        default:
            break;
    }
}

// FIXME USE ringbus_overflow_counts
// #define SOFTWARE_RB_OVERFLOW (0x10000000)

uint32_t blinks_at_enter = 0;
uint32_t error_saved;


unsigned int error_readout = 0;

// this will watch and update blinklib status
void pet_eth_error_flags(void) {
    static unsigned int err_state = 1;
    unsigned int error_count;
    // unsigned int err_state_p = 1;
    
    bool found_error = false;

    // queue_dma_out(err_state);
    switch(err_state) {
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x05:
        case 0x06:
        case 0x08:
        case 0x09: // over-ride in sw, missing in hw
        case 0x0b:
        case 0x0d:
        case 0x0e:
        case 0x0f:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17: // non zero is not fatal error
        case 0x18: // non zero is not fatal error
        
        case 0x1b: // non zero is not fatal error
        case 0x1c: // non zero is not fatal error
            // write this value to riscv_control
            // a verilog always block in eth_top.sv will wire the requested
            // data to riscv_status
            CSR_WRITE(CS_CONTROL, err_state);
            CSR_READ(CS_STATUS, error_count);

            software_errors(err_state, &error_count);


            if( error_count != 0 ) {
                found_error = true;
            }


            switch(error_readout) {

                // first time we see it
                case 0:
                    if( found_error ) {
                        blinks_at_enter = blinklib_times_shown();
                        error_readout = 1;
                        error_saved = error_count; // save error while current blink cycle completes
                    } else {
                        err_state++; // bump to check next error
                    }
                    break;
                // first completed blink since we saw it
                case 1:
                    if( blinklib_times_shown() > blinks_at_enter ) {
                        blinklib_status(err_state + 1); // +1 here because 1 is a valid error
                        queue_dma_out(err_state);
                        queue_dma_out(error_saved);
                        // SET_REG(x3, err_state);
                        // SET_REG(x3, error_count);
                        error_readout = 2;
                    }
                    break;
                //first completed blink of our most recent error code
                case 2:
                    if( blinklib_times_shown() != 0 ) {
                        error_readout = 0;
                        err_state++;
                        blinklib_status(1);
                    }
                    break;
                default:
                    break;
            }
            break;
        case 0x4:
        case 0x7:
        case 0xa: // missing
        case 0xc: // named cs30_data_buf_overflow_cntr but carries data from cs20

        case 0x19: // non zero is not fatal error
        case 0x1a: // non zero is not fatal error
            // skip these
            err_state++;
            break;
        default:
            err_state = 1;
    }
}

void reset_eth_mac(void) {
    const uint32_t reset_eth = 0x80000000;
    CSR_WRITE(CS_CONTROL, reset_eth);
    STALL(10);
    CSR_WRITE(CS_CONTROL, 0);
}



// timer
unsigned int last_forwarded_uart = 0;

// minimum timer values between send
const unsigned int uart_forward_min_delay = 2000;

void pet_forward_uart_to_pc(void) {
    int error;
    unsigned now;

    CSR_READ(TIMER_VALUE, now);
    const unsigned delta = subtract_timers(now, last_forwarded_uart);

    if( delta > uart_forward_min_delay ) {

        const char c = uart_get_char(&error);
        if( error != 0 ) {
            return;
        }

        if( !disable_uart_readback ) {
            queue_dma_out(UART_READOUT_PCCMD | c);
        }

        last_forwarded_uart = now;

    } else {
        return;
    }
}

uint32_t count = 0;
uint32_t manual_status_index = 0;
uint32_t manual_status_value = 0;
uint32_t manual_blink_status = 0;

uint32_t blink_track_local_read = 0;

uint32_t* pointer_for_generic_op(const uint32_t sel) {
    uint32_t *p = 0;
    switch(sel) {
        case 0:
            p = &ringbus_overflow_counts;
            break;
        case 1:
            p = &total_ring_sent;
            break;
        case 2:
            p = &disable_uart_readback;
            break;
        case 3:
            p = &manual_status_index;
            break;
        case 4:
            CSR_WRITE(CS_CONTROL, manual_status_index);
            CSR_READ(CS_STATUS, manual_status_value);
            p = &manual_status_value;
            break;
        case 5:
            p = &manual_blink_status;
            blinklib_status(manual_blink_status);
            break;
        case 6:
            p = &count;
            break;
        case 7:
            p = &count; // dummy pointer
            reset_eth_mac();
            break;
        case 8:
            blink_track_local_read = blinklib_times_shown();
            p = &blink_track_local_read;
            break;
        case 9:
            p = &force_error;
            break;
        default:
            break;
    }
    return p;
}





/*
 * CS20 has a reset that goes longer than other FPGA. Because ETH is upstream
 * from CS20, we need to burn cycles before sending our first message or else
 * the ringbus goes unstable
 */

void pre_delay(void) {
  int d1 = 40; // 20 works, 10 fails
  for(int i = 0; i < d1; i++) {
    asm("nop");
  }
}



void setup_mapmov(void) {
    // pass 1 to reset module
    // if we change to a different mode later on this should be 0
    mapmov_choose_mode(MAPMOV_SUBCARRIER_128, 1); // used to be MAPMOV_MODE_128_CENTERED
}

int main(void)
{
    dma_in_last = DMA_IN_COUNT-1;
    dma_in_last_consumed = DMA_IN_COUNT-1;
    dma_out_last = DMA_OUT_COUNT-1;

    ring_send_pending = 0;
    ringbus_overflow_counts = 0;
    fill_level_p = 0;

    pre_delay();

    CSR_WRITE_ZERO(DMA_0_FLUSH_SCHEDULE);
    CSR_WRITE_ZERO(DMA_1_FLUSH_SCHEDULE);

    setup_dma_in();

    setup_mapmov();

    CSR_WRITE(GPIO_WRITE_EN, ALL_GPIO_PIN);
    CSR_SET_BITS(GPIO_WRITE, DAC_SDENN_BIT |
                             TX_CHANNEL_A_BIT |
                             TX_CHANNEL_B_BIT |
                             VGA_CTRL_B_CS_N |
                             VGA_CTRL_A_CS_N);

    int count2 = 0;
    // int reset_once = 0;


    handle_generic_register_get_pointer(&pointer_for_generic_op);
    handle_generic_register_ring(&queue_dma_out);

    setup_blinklib();

    // default status is 1, which means a slow blink
    // to emulate previous behaviour
    blinklib_status(1);
    
    int busy = 0;
    int slow_blink = 0;

    char uart_c = 'A';

    unsigned int occupancy;
    unsigned int occupancy2;


    // int h1;
    while(1) {
      // SET_REG(x3, 0x00000000);
      // CSR_READ(mip, h1);

      CSR_READ(RINGBUS_READ_OCCUPANCY, occupancy);
      CSR_READ(RINGBUS_2_READ_OCCUPANCY, occupancy2);

      // MAKE IT HERE

      // FIXME this is a big change, what's the difference?
      // remove if, and keep body to test this out
      // if(h1 == 0) {
      handle_pending_ring();
      // }

      if(occupancy) {
        handle_multi_ring_in(occupancy);
        busy += 8;
      }

      if(occupancy2) {
        handle_multi_ring_2_in(occupancy2);
        busy += 8;
      }

      handle_single_dma_in();

      pet_fill_level();


      // busy gets added to whenever there is a ringbus
      // it slowly ticks down when there is no ringbus
      // we only get past this line when busy is zero
      // this allows fast ringbus at the beginning of the testbench
      // to not be dropped by the partial mapmov below
      // unsure if the values here (8 etc) are doing anything or are
      // even in the ballpark
      if( busy > 0 ) {
        busy--;
        continue;
      }

      pet_eth_error_flags();


      // clear this bit right away when we see it
      // because we do not block on output dma because they are so short
      // if(h1 & DMA_1_ENABLE_BIT) {
      //   CSR_WRITE(DMA_1_INTERRUPT_CLEAR, 0);
      // }

      // // clear because we are reading occupancy now
      // if(h1 & DMA_0_ENABLE_BIT) {
      //   CSR_WRITE(DMA_0_INTERRUPT_CLEAR, 0);
      // }

      if( busy == 0 ) {
        mapmov_pet_partial();
      }

      pet_forward_uart_to_pc();


      if( slow_blink >= 4 ) {
          pet_blinklib();
          slow_blink = 0;
      }

      // bring out of reset here
      // if(count == 1000000){

      //   // we need this flag because count will rollover every 1446 seconds
      //   if(!reset_once) {
      //       CSR_SET_BITS(GPIO_WRITE, DAC_RESETN_BIT);
      //     reset_once = 1;
      //   }


      // }
      count++;
      slow_blink++;

      if(0) {
          if(count2 == 100000) {
            uart_put_char_interrupt(uart_c);
            uart_c++;
            if( uart_c == 'Z' ) {
                uart_c = 'A';
            }
            count2 = 0;

            int error;
            char gotc;
            
            gotc = uart_get_char(&error);
            if( error == 0) {
                queue_dma_out(DEBUG_15_PCCMD | gotc);
            }

          }
          count2++;
      }


      // if(BITWISE_MOD(count,0x10000) == 0) {
      //   CSR_CLEAR_BITS(GPIO_WRITE, LED_GPIO_BIT);
      // }

      // if(BITWISE_MOD(count,0x80000) == 0) {
      //   CSR_SET_BITS(GPIO_WRITE, LED_GPIO_BIT);
      // }
    }
}
