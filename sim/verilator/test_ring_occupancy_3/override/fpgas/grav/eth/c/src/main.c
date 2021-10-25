#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "ringbus.h"
#include "bootloader.h"
#include "gain_control.h"
#include "sig_utils.h"
#include "blinklib.h"
#include "mapmov.h"
#include "uart_driver.h"
#include "subtract_timers.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"

void setup_mapmov(void);

// this is a magic number, and should be compiled in by the tool, per riscv
#define HACKED_VMEM_DEPTH_WORDS (4096*16)

// must be power of 2
#define DMA_IN_COUNT (32)
// how big is each packet data payload, aka dma trigger size
#define DMA_IN_LEN (1)
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




void set_tx_channel(unsigned int channel){
    CSR_WRITE(GPIO_WRITE_EN, ALL_GPIO_PIN);
    CSR_CLEAR_BITS(GPIO_WRITE, channel);
    for(unsigned int i = 0; i < 1000000*3; i++) {}
}

unsigned int ringbus_overflow_counts;

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
        case CONFIG_DAC_CMD:
            config_dac(data);
            break;
        case DISABLE_DAC_CMD:
            disable_dac(data);
            break;
        case TX_CHANNEL_CMD:
            set_tx_channel(data);
            break;
        case VGA_GAIN_CMD:
            set_vga_gain(data);
            break;
        case DSA_GAIN_CMD:
            set_dsa_gain(data);
            break;
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
  SET_REG(x3, 0xe0000000 | occupancy);
  // check_dma = occupancy;
  for(i=occupancy;i<dma_in_expected_occupancy;i++) {
    SET_REG(x3, 0xf0000000 | i);
    STALL(20);
    while(1) {STALL(1);}
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

// FIXME USE ringbus_overflow_counts
// #define SOFTWARE_RB_OVERFLOW (0x10000000)

// this will watch and update blinklib status
void pet_eth_error_flags(void) {
    static unsigned int err_state = 1;
    unsigned int error_count;
    // unsigned int err_state_p = 1;
    static unsigned int do_update = 1;

    // queue_dma_out(err_state);
    switch(err_state) {
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x5:
        case 0x6:
        case 0x8:
        case 0xb:
        case 0xc:
        case 0xd:
            // write this value to riscv_control
            // a verilog always block in eth_top.sv will wire the requested
            // data to riscv_status
            CSR_WRITE(CS_CONTROL, err_state);
            CSR_READ(CS_STATUS, error_count);
            if(error_count != 0) {
                if(do_update) {
                    // we only want to set the status once, not over and over
                    // if we did, the pattern would not show.
                    // we use do_update to control this
                    blinklib_status(err_state);
                    do_update = 0;
                    queue_dma_out(err_state);
                    queue_dma_out(error_count);
                    // SET_REG(x3, err_state);
                    // SET_REG(x3, error_count);
                }

                // once the entire blink code has been shown, we can move on
                if( blinklib_times_shown() != 0 ) {
                    // queue_dma_out(0xfeed);
                    err_state++;
                    do_update = 1;
                }
            } else {
                err_state++;
                do_update = 1;
            }
            break;
        case 0x4:
        case 0x7:
        case 0x9:
        case 0xa:
            // skip these
            err_state++;
            break;
        default:
            err_state = 1;
    }
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


        queue_dma_out(UART_READOUT_PCCMD | c);

        last_forwarded_uart = now;

    } else {
        return;
    }
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

    int count = 0;
    int count2 = 0;
    int reset_once = 0;

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


      if( slow_blink >= 256 ) {
          pet_blinklib();
          slow_blink = 0;
      }

      // bring out of reset here
      if(count == 1000000){

        // we need this flag because count will rollover every 1446 seconds
        if(!reset_once) {
            CSR_SET_BITS(GPIO_WRITE, DAC_RESETN_BIT);
          reset_once = 1;
        }


      }
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
