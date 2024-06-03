#include "mbed.h"
#include "pinout.h"
#include "hardware.h"
#include "range_finder.h"
#include "switch.h"
#include "display.h"
#include "control.h"


int main (void) {

  hw_init();
  
  // initialize the range finder FSM
  rf_init(&g_trg, &g_ech);

  // initialize the middle switch FSM
  swm_init(&g_swm);
  
  // initialize the seven seg display FSM
  display_init(&g_dsl, &g_dsr, &g_seven_seg);

  // initialize the control FSM
  ctl_init(&g_led_l, &g_led_c, &g_led_r,  &g_ldr);

  //  ------------------------------------------------------------------
  for (;;) {
    // the range finder FSM
    rf_fsm();
    // the middle switch FSM
    swm_fsm();
    // the seven seg display FSM
    display_fsm();
    // the control FSM
    ctl_fsm();
    //  ----------------------------------------------------------------



    // sleep
    __disable_irq();
    if (gb_rf_can_sleep && gb_swm_can_sleep
		&& gb_display_can_sleep && gb_ctl_can_sleep) {
      __WFI();
    }
    __enable_irq();
  } // forever
} // main()

