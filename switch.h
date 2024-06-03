#ifndef SWITCH_H
#define SWITCH_H

#include "mbed.h"

// messages
  // output messages
extern bool           gb_swm_long_msg;      // long pulsation
extern bool           gb_swm_msg;           // short pulsation
extern bool volatile  gb_swm_can_sleep;     // this FSM can sleep

// the FSM
void swm_fsm(void);

// initialize the FSM. The input parameters is a pointer to the switch
//  to manage
void swm_init(InterruptIn *swm);

#endif  // SWITCH_H
