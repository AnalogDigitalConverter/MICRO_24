#ifndef DISPLAY_H
#define DISPLAY_H

#include "mbed.h"

// messages
  // input messages
extern bool           gb_display_on_msg;          // start display
extern bool           gb_display_off_msg;         // stop display
extern bool           gb_display_update_msg;      // update display
extern bool           gb_display_select_msg;

    // parameter for gb_display_update_msg (and gb_display_on_msg)
extern uint16_t       g_display_segs;             // bits 6:0 are the segments of the right display
                                                  // bits 14:8 are the segments of the left display
                                                  // order is GFEDCBA
extern bool           gb_display_brightness_msg;  // update brightness
    // parameter for gb_display_brightness_msg (and gb_display_on_msg)
extern uint8_t        g_display_brightness;       // percentage (0-99) of brightness
  // output messages
extern bool volatile  gb_display_can_sleep;       //this FSM can sleep

// the FSM
void display_fsm(void);

// initialize the FSM. The input parameters are pointers to the mbed
//  objects associated to display pins dsl, dsr and seven_seg bus
void display_init(PwmOut *dsl, PwmOut *dsr, BusOut *seven_seg);

#endif  // DISPLAY_H
