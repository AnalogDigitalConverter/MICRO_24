#include "switch.h"

// extended state ------------------------------------------------------
  // "basic" state
  // complete this code to achieve the FSM functionality  ++++++++++++++
typedef enum {SW_IDLE, SW_IN, SW_OUT} sw_state_t;
static sw_state_t g_swm_state;
  //  ------------------------------------------------------------------

  // externally reachable objects
  // complete this code to achieve the FSM functionality  ++++++++++++++
bool              gb_swm_msg;          // set at swm rising edge and timer <1s
bool              gb_swm_long_msg;     // set at swm rising edge and timer 1s
bool volatile     gb_swm_can_sleep;    // this FSM can sleep

  //  ------------------------------------------------------------------

  // hardware resources
  // complete this code to achieve the FSM functionality  ++++++++++++++
static InterruptIn  *gp_swm;
static Timeout      g_sw_to;


  //  ------------------------------------------------------------------

  // internal objects
  // complete this code to achieve the FSM functionality  ++++++++++++++
static bool           gb_swm_initd;            // true after call to swm_init()
static bool volatile  gb_swm_rise_evnt;        // rise on swm event
static bool volatile  gb_swm_fall_evnt;        // fall on swm event
static bool volatile  gb_swm_to_evnt;          // timeout elapsed event


  //  ------------------------------------------------------------------
// end of extended state  ----------------------------------------------

// ISRs ----------------------------------------------------------------

  // complete this code to achieve the FSM functionality  +++++++++++++
  // swm rise ISR
static void swm_rise_isr(void) {
  gb_swm_rise_evnt = true;
  gb_swm_can_sleep = false;
}

  // swm fall isr
static void swm_fall_isr(void) {
  gb_swm_fall_evnt = true;
  gb_swm_can_sleep = false;
}

  // timeout ISR
static void swm_to_isr(void) {
  gb_swm_to_evnt = true;
  gb_swm_can_sleep = false;
}
   //  -----------------------------------------------------------------
// end of ISRs  --------------------------------------------------------

// FSM  ----------------------------------------------------------------
void swm_fsm (void) {
  if (gb_swm_initd) {  // protect against calling swm_fsm() w/o a previous call to swm_init()
    switch (g_swm_state) {

      // complete this code to achieve the FSM functionality  ++++++++++
      case SW_IDLE:
				gb_swm_rise_evnt = false;
			  gb_swm_to_evnt = false;
			
        if(gb_swm_fall_evnt)
        {
          gb_swm_fall_evnt = false;
          g_sw_to.attach(swm_to_isr, 40ms);
          g_swm_state = SW_IN;
        }
        break;
      case SW_IN:
				gb_swm_rise_evnt = false;
			  gb_swm_fall_evnt = false;
			
        if (gb_swm_to_evnt)
        {
          gb_swm_to_evnt = false;
          if (!*gp_swm)
          {
            g_sw_to.attach(swm_to_isr, 1000ms);
            g_swm_state = SW_OUT;
          }
          else
          {
            g_swm_state = SW_IDLE;
          }
        }
        break;
      case SW_OUT:
				gb_swm_fall_evnt = false;
			
        if(gb_swm_to_evnt)
        {
          gb_swm_to_evnt = false;
          gb_swm_msg = (*gp_swm)? true : false;
          gb_swm_long_msg = !gb_swm_msg;
          g_swm_state = SW_IDLE;
        }
		else if (gb_swm_rise_evnt)
        {
          gb_swm_rise_evnt = false;
          g_sw_to.detach();
          gb_swm_to_evnt = false;
          gb_swm_msg = true;
          gb_swm_long_msg = !gb_swm_msg;
          g_swm_state = SW_IDLE;
        }
		else{}
        break;
      //  ----------------------------------------------------------------
    } // switch (swm_state)

  // complete this code to achieve the FSM functionality (can sleep?) ++
    __disable_irq();
    if(!gb_swm_fall_evnt && !gb_swm_rise_evnt && !gb_swm_to_evnt) {
      gb_swm_can_sleep = true;
    }
    __enable_irq();

  //  ------------------------------------------------------------------
  } // if (gb_swm_initd)
}
// end of FSM ----------------------------------------------------------

// initialize FSM machinery  -------------------------------------------
void swm_init (InterruptIn *swm) {
  if (!gb_swm_initd) {
    gb_swm_initd = true;   // protect against multiple calls to swm_init

    // complete this code to achieve the FSM functionality  ++++++++++++
    // initialize state
    g_swm_state = SW_IDLE;
    gb_swm_rise_evnt = false;
    gb_swm_fall_evnt = false;
    g_sw_to.detach();
    gb_swm_to_evnt = false;

    // initial actions
    gp_swm = swm;              // save pointers to the pins

    // register ISRs
    gp_swm->rise(swm_rise_isr);   // register echo ISRs
    gp_swm->fall(swm_fall_isr);

    //  ----------------------------------------------------------------
  }
}
// end of FSM initialization  ------------------------------------------
