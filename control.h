#ifndef CONTROL_H
#define CONTROL_H

#include "mbed.h"

// // messages
//   // input messages
extern bool           gb_swm_msg;
extern bool           gb_swm_long_msg;
extern bool           gb_rf_done_msg;
extern int32_t				g_rf_range_cm;


  // output messages
extern bool volatile  gb_ctl_can_sleep;       //this FSM can sleep
extern bool           gb_rf_start_msg;
extern bool           gb_display_brightness_msg;  // update brightness
extern bool           gb_display_update_msg;
extern bool           gb_display_off_msg;
extern bool           gb_display_on_msg;
extern uint8_t        g_display_brightness;       // percentage (0-99) of brightness
extern uint16_t       g_display_segs;             // bits 6:0 are the segments of the right display
//                                                   // bits 14:8 are the segments of the left display
//                                                   // order is GFEDCBA

// the FSM
void ctl_fsm(void);

// initialize the FSM. The input parameters are pointers to the mbed
//  objects associated to display pins dsl, dsr and seven_seg bus
void ctl_init(DigitalOut *led_l, PwmOut *pwm_led_c, DigitalOut *led_r, AnalogIn *ldr);

#endif  // CONTROL_H
