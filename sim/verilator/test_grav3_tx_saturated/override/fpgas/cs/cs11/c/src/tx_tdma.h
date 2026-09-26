#include "random.h"


unsigned int tdma_mode = 0;
unsigned int tdma_mode_pending = 0;

unsigned int tdma_argument = 0;
unsigned int tdma_argument_pending = 0;

/// this value of tdma_mode and below (inclusive) will activate
/// mask_subcarriers
/// insert_data2
#define TDMA_INSERT_MODE_MAXIMUM (11)

/// Takes first 2 bits and returns those 2 bits repeated across 32 bits
/// this will take 2 bits and turn it into 16 QPSK symbols of the same point
uint32_t word_pattern_for_bits(uint32_t w) {
    uint32_t pat = w & 0x3;

    switch(pat) {
        case 0:
            return 0;
        case 1:
            return 0x55555555;
        case 2:
            return 0xaaaaaaaa;
        default:
        case 3:
            return 0xffffffff;
    }
}


uint32_t mode6_packet[3];
uint32_t mode6_hash;

///
/// Driven by ringbus from rx side
/// reads global tdma_mode
///
/// pulls 32 bit word at a time
/// each call will yield the next word in the sequence
/// @param reset pass non zero value to reset

unsigned int data_next_value(unsigned int reset, unsigned int timeslot, unsigned int progress) {
    static unsigned int message = 0xdeadbeef;
    static unsigned int s0 = 0;

    if(reset) {
        s0 = 0;

        // so pending puts us at 4.
        // however nothing happens at 4
        // once a reset comes by, we go to mode 1, which starts the first of 5 or 6 words
        // which culminate in the lifetime counter
        // that's what this does
        if(tdma_mode == 4) {
            tdma_mode = 1;
        }
    }

    unsigned int ret = 0;
    if(tdma_mode == 0) {
        if(message == 0x0) {
            message = 1;
        } else if(message == 1) {
            message = 2;
        } else if(message == 2) {
            message = 3;
        } else if(message == 3) {
            message = 4;
        } else if(message == 4) {
            message = 5;
        } else if(message == 5) {
            message = 6;
        } else if(message == 6) {
            message = 7;
        } else if(message == 7) {
            message = 0xdeadbeef;
        } else if(message == 0xaaaa0000) {
            message = 0x0;
        } else if(message == 0xffff0000) {
            message = 0xaaaa0000;
        } else if(message == 0xdeadbeef) {
            message = 0xffff0000;
        } else {
            message = 0;
        }
        ret = message;
    } else if(tdma_mode == 1) {
        s0 = 0;
        tdma_mode = 2;
        ret = 0;
    } else if(tdma_mode == 2) {
        s0++;
        if(s0 == 5) {
            ret = 0xbeefbeef;
        } else if(s0 == 6) {
            ret = 0;
            tdma_mode = 3;
        }
    } else if(tdma_mode == 3) {
        ret = 0;
    } else if(tdma_mode == 4) {
        // DO NOTHING, this is a holding state until a reset comes by
    } else if(tdma_mode == 6) {

        // progress is set based on the current offset etc
        // progress is in 0 - 25600
        // The double case are annoying but i haven't gone back and removed the duplicates
        // as we send the first 3 words, we build them into an array
        // at the 4th word, we send a hash based on the previous 3
        // the 5th words is simply a tail
        switch(progress) {
            case 0:
                ret = 0xdeadbeef;
                mode6_packet[0] = ret;
                break;
            case 15:
            case 16:
                ret = lifetime_32;
                mode6_packet[1] = ret;
                break;
            case 31:
            case 32:
                ret = schedule->epoc_time;
                mode6_packet[2] = ret;
                break;
            case 47:
            case 48:
                mode6_hash = xorshift32(1, (unsigned int*)mode6_packet, 3);
                ret = mode6_hash;
                break;
            case 63:
            case 64:
                ret = 0xff000000;
                break;
            case 79:
            case 80:
                ret = progress;
                break;
            default:
                ret = 0;
            break;
        }
    } else if(tdma_mode == 8 || tdma_mode == 9) {
        if( timeslot == tdma_mode && progress <= 16 ) {
            fb_queue_ring_eth(TDMA_REPLY_PCCMD | (timeslot<<16) );
            fb_queue_ring_eth(TDMA_REPLY_PCCMD | (progress) );
            ret = 0xca5e0000 | timeslot;
        } else {
            ret = 0;
        }
    } else if(tdma_mode == 10) {
        ret = 0; // parking
    } else if(tdma_mode == 11) {
        uint32_t *p;
        uint32_t sel = tdma_argument / 32;
        uint32_t shift = tdma_argument % 32;

        switch(sel) {
            default:
            case 0:
                p = (uint32_t*) &lifetime_32;
                break;
            case 1:
                p = (uint32_t*) &progress;
                break;
            case 2:
                p = (uint32_t*) &timeslot;
                break;
            case 3:
                p = (uint32_t*) &schedule->epoc_time;
                break;
        }
        uint32_t capture = *p;

        ret = word_pattern_for_bits(capture>>shift);

        // SET_REG(x3, 0x00006000);
        // SET_REG(x3, tdma_mode);
        // SET_REG(x3, tdma_argument);
        // SET_REG(x3, 0x00006001);
        // SET_REG(x3, sel);
        // SET_REG(x3, 0x00006002);
        // SET_REG(x3, shift);
        // SET_REG(x3, 0x00006003);
        // SET_REG(x3, capture);
        // SET_REG(x3, 0x00006004);
        // SET_REG(x3, ret);

    }


    return ret;
}


void insert_data2(unsigned int dma_ptr, unsigned int timeslot, unsigned int progress) {
    static unsigned int phase = 0;
    static int message = 0xdeadbeef;  // static assign does not grab value from data_next_value()

    // if(0) {
    //     message = data_next_value(1, timeslot, progress);
    //     phase = 0;
    // }

    if(timeslot == 0 && progress == 0) {
        phase = 0;
        message = data_next_value(1, timeslot, progress);
    }


    unsigned int shift = phase*2;

    unsigned int bits = (message>>shift)&0x3;

#define QPSK_POS (0x2a81) //+0.7
#define QPSK_NEG (0xd57e) //-0.7

    unsigned int qpsk_point;
    switch(bits) {
        case 0:
            qpsk_point = (QPSK_NEG << 16 | QPSK_NEG);
            break;
        case 1:
            qpsk_point = (QPSK_POS << 16 | QPSK_NEG);
            break;
        case 2:
            qpsk_point = (QPSK_NEG << 16 | QPSK_POS);
            break;
        case 3:
            qpsk_point = (QPSK_POS << 16 | QPSK_POS);
            break;
    }


    vector_memory[dma_ptr + tdma_subcarrier] = qpsk_point;

    phase++;

    if( tdma_mode == 8 ) {
        if(timeslot == 9) {
            vector_memory[dma_ptr + tdma_subcarrier] = 0;
        }
    }

    if( tdma_mode == 9 ) {
        if(timeslot == 8) {
            vector_memory[dma_ptr + tdma_subcarrier] = 0;
        }
    }

    // next mod of 5
    if(phase == 16) {
        message = data_next_value(0, timeslot, progress);
        phase = 0;
    }
}




///
///  ringbus call back for tdma sync
///  is set to 0
///  modes:
///    0 previously this was putting out a steady 0xdeadbeef, etc on the tone,
///      this gets set into dmode immediately as it is a kind of reset
///    4 this is set into pending
///
///  flow:
///      mode_pending -> 4
///      mode <- mode_pending  at start of ts0    line 576
///    data_value_next:
///      if mode == 4, mode = 1
///        data is sent,
///      mode goes 1,2,3 and parks at 3
///   because pending is never reset, we continue to set pending to 4
///
/// Using mode 0 is technically illegal because 0 is a reserved value 
/// for tdma_mode_pending

void tdma_callback(unsigned int data) {
    unsigned int arg = (data&0xffff);
    unsigned int dmode = (data >> 16) & 0xff;

    switch(dmode) {
        case 0:
        case 1:
            tdma_mode = dmode;
            tdma_mode_pending = 0;
            tdma_argument = 0;
            tdma_argument_pending = 0;
            break;
                // any mode listed here just sets and returns
                //
        case 6: // mode 6 is a new test thing
        case 4: // 4 is what was working before with alpha/demo
        case 7: // this gets us stuck in the first data row
        case 8: // how to tell
        case 9: // how to tell
        case 10: // parking
        case 11: // pattern where arg modifies arg
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
            tdma_mode_pending = dmode;
            tdma_argument_pending = arg;
            break;
        case 20:
            // any mode listed here for sets them to the same
            // sort of a disable state (disable happens elsewhere)
            tdma_mode_pending = dmode;
            tdma_argument_pending = arg;
            tdma_mode = dmode;
            break;
        case 21:
            // do nothing now that offset is removed
            // schedule->offset = 0;
            break;
        default:
            break;
    }
}

void setup_tdma(void) {
    tdma_mode_pending = tdma_mode = 20;
}
