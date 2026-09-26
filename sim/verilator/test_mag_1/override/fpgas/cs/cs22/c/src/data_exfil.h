#include "subtract_timers.h"

#ifdef __EXFIL_SIZE
#error "are you including this file twice?"
#endif

#ifndef EXFIL_RINGBUS_TOLERANCE
#define EXFIL_RINGBUS_TOLERANCE (2)
#endif

#define __EXFIL_SIZE ((EXFIL_SIZE)*(EXFIL_CHUNKS))

VMEM_SECTION unsigned int _exfil_chunks[__EXFIL_SIZE];

unsigned int exfil_state = 0;
unsigned int exfil_working = 0;
unsigned int exfil_sending = 0;
unsigned int exfil_load = 0;

void exfil_request() {
    if( exfil_state == 0) {
        exfil_state = 1;
        exfil_working = 0;
        exfil_sending = 0;
    }
}


unsigned int exfil_active() {
    return exfil_state == 1;
}


unsigned int exfil_working_on_row(void) {
    unsigned int *cpu = _exfil_chunks + (EXFIL_SIZE*exfil_working);
    return VMEM_ROW_ADDRESS(cpu);
}

void exfil_wrote_row(void) {

    if( (exfil_working+1) < EXFIL_CHUNKS ) {
        exfil_working++;
    } else {
        exfil_state = 2;
        // SET_REG(x3, 0xeeee0000);
    }
}

void exfil_single();

unsigned int last_exfil_rb = 0;

void pet_exfil(void) {
    if( exfil_state == 2 ) {
        // SET_REG(x3, 0xbbbb0000);
        unsigned now;
        CSR_READ(TIMER_VALUE, now);

        //                       calculates a-b
        unsigned counter_delta = subtract_timers(now,last_exfil_rb);

        if( counter_delta > EXFIL_COUNTER_DELAY ) {
            unsigned int occupancy;
            CSR_READ(RINGBUS_SCHEDULE_OCCUPANCY, occupancy);
            if(occupancy <= EXFIL_RINGBUS_TOLERANCE) {
                last_exfil_rb = now;
                exfil_single();
            }
        }
    }
}


void exfil_single() {
    unsigned int rb;
    if( (exfil_sending % 2) == 0 ) {
        exfil_load = _exfil_chunks[ exfil_sending / 2 ];
        // SET_REG(x3, 0xcccc0000);
        // SET_REG(x3, exfil_load);
        // SET_REG(x3, 0x99990000);
        // SET_REG(x3, exfil_sending);
        // SET_REG(x3, 0x0);
        


        rb = EXFIL_READOUT_PCCMD | ((exfil_sending&0xff)<<16) | (exfil_load&0xffff);
    } else {
        rb = EXFIL_READOUT_PCCMD | ((exfil_sending&0xff)<<16) | ((exfil_load>>16)&0xffff);
    }

    ring_block_send_eth(rb);


    if( (exfil_sending+1) < (__EXFIL_SIZE*2) ) {
        exfil_sending++;
    } else {
        exfil_state = 0;
    }


}

// TRIGGER_EXFIL_CMD
// EXFIL_READOUT_PCCMD