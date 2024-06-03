#include "range_finder.h"

// extended state ------------------------------------------------------
  // "basic" state
typedef enum {RF_IDLE, RF_TRG, RF_WAIT, RF_MEAS} rf_state_t;
static rf_state_t g_rf_state;

  // externally reachable objects
bool              gb_rf_start_msg;    // set (externally) to start a measurement
bool              gb_rf_done_msg;     // set when a measurement is completed
    // parameter to rf_done_msg
int32_t           g_rf_range_cm;      // measured range in cm
bool volatile     gb_rf_can_sleep;    // this FSM can sleep

  // hardware resources
static DigitalOut     *gp_rf_trigger;
static InterruptIn    *gp_rf_echo;
static Timeout        g_rf_to;        // timeout
static Timer          g_rf_tmr;       // measurement timer

  // internal objects
static bool           gb_rf_initd;            // true after call to rf_init()
static bool volatile  gb_rf_echo_rise_evnt;   // rise on echo event
static bool volatile  gb_rf_echo_fall_evnt;   // fall on echo event
static bool volatile  gb_rf_to_evnt;          // timeout elapsed event
// end of extended state  ----------------------------------------------

// ISRs ----------------------------------------------------------------
  // echo rise ISR
static void rf_echo_rise_isr(void) {
  gb_rf_echo_rise_evnt = true;
  gb_rf_can_sleep = false;
}

  // echo fall isr
static void rf_echo_fall_isr(void) {
  gb_rf_echo_fall_evnt = true;
  gb_rf_can_sleep = false;
}

  // timeout ISR
static void rf_to_isr(void) {
  gb_rf_to_evnt = true;
  gb_rf_can_sleep = false;
}
// end of ISRs  --------------------------------------------------------

// FSM  ----------------------------------------------------------------
void rf_fsm (void) {
  if (gb_rf_initd) {  // protect against calling rf_fsm() w/o a previous call to rf_init()
    switch (g_rf_state) {
      // complete this code to achieve the FSM functionality  ++++++++++
      case RF_IDLE:
				
        gb_rf_echo_rise_evnt = false;
			  gb_rf_echo_fall_evnt = false;
			  gb_rf_to_evnt = false;
			  gb_rf_done_msg = false;
			
        if(gb_rf_start_msg)
        {
          gb_rf_start_msg = false;
          *gp_rf_trigger = 1;
          g_rf_to.attach(rf_to_isr, 1ms);
          g_rf_state = RF_TRG;
        }
      break;

      case RF_TRG:
				gb_rf_echo_rise_evnt = false;
        gb_rf_echo_fall_evnt = false;
        gb_rf_done_msg = false;
        gb_rf_start_msg = false;
        
			  if(gb_rf_to_evnt)
        {
          gb_rf_to_evnt = false;
          *gp_rf_trigger = 0;
          g_rf_tmr.stop();
          g_rf_tmr.reset();
          g_rf_to.attach(rf_to_isr, 50ms);
          g_rf_state = RF_WAIT;
        }
      break;

      case RF_WAIT:
				
			  gb_rf_echo_fall_evnt = false;
        gb_rf_done_msg = false;
        gb_rf_start_msg = false;
			
        if(gb_rf_echo_rise_evnt)
        {
          gb_rf_echo_rise_evnt = false;
					g_rf_tmr.reset();
          g_rf_tmr.start();
		      g_rf_to.detach();
					g_rf_to.attach(rf_to_isr, 50ms);
          g_rf_state = RF_MEAS;
        }
        else if (gb_rf_to_evnt)
        {
          gb_rf_to_evnt = false;
          g_rf_range_cm = -1;
		      gb_rf_done_msg = true;
          g_rf_state = RF_IDLE;
        }
      break;

      case RF_MEAS:
				gb_rf_echo_rise_evnt = false;
        gb_rf_done_msg = false;
        gb_rf_start_msg = false;
        
			  if(gb_rf_echo_fall_evnt)
        {
		      gb_rf_echo_fall_evnt = false;
          g_rf_tmr.stop();
					g_rf_to.detach();
          g_rf_range_cm = g_rf_tmr.elapsed_time().count()/58;
		      gb_rf_done_msg = true;
          g_rf_state = RF_IDLE;
        }
        else if (gb_rf_to_evnt)
        {
          gb_rf_to_evnt = false;
          g_rf_range_cm = -1; 
		      gb_rf_done_msg = true;
          g_rf_state = RF_IDLE;
        }
        break;
    //  ----------------------------------------------------------------
    } // switch (rf_state)


    __disable_irq();
    if(!gb_rf_echo_rise_evnt && !gb_rf_echo_fall_evnt && !gb_rf_to_evnt) {
      gb_rf_can_sleep = true;
    }
    __enable_irq();
  } // if (gb_rf_initd)
}
// end of FSM ----------------------------------------------------------

// initialize FSM machinery  -------------------------------------------
void rf_init (DigitalOut *trigger, InterruptIn *echo) {
  if (!gb_rf_initd) {
    gb_rf_initd = true;   // protect against multiple calls to rf_init

    // initialize state
    g_rf_state = RF_IDLE;
    gb_rf_echo_rise_evnt = false;
    gb_rf_echo_fall_evnt = false;
    g_rf_to.detach();
    gb_rf_to_evnt = false;

    // initial actions
    gp_rf_trigger = trigger;              // save pointers to the pins
    gp_rf_echo = echo;

    *gp_rf_trigger = 0;                   // drive trigger
    gp_rf_echo->rise(rf_echo_rise_isr);   // register echo ISRs
    gp_rf_echo->fall(rf_echo_fall_isr);
  }
}
// end of FSM initialization  ------------------------------------------
