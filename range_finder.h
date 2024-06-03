#ifndef RANGE_FINDER_H
#define RANGE_FINDER_H

#include "mbed.h"

// messages
  // input messages
extern bool           gb_rf_start_msg;      // start measurement
  // output messages
extern bool           gb_rf_done_msg;       // measurement done
    // parameter for the previous message
extern int32_t        g_rf_range_cm;        // measurement result in cm (-1 if error)
extern bool volatile  gb_rf_can_sleep;      // this FSM can sleep

// the FSM
void rf_fsm(void);

// initialize the FSM. The input parameters are pointers to the mbed
//  objects associated to sensor pins Trigger & Echo
void rf_init(DigitalOut *trg, InterruptIn *ech);

#endif  // RANGE_FINDER_H
