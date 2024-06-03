#include "range_finder.h"
#include "to_7seg.h"
#include "control.h"
#if 1
# define VERBOSE
#endif

// extended state -------------------------------------------------------------
  // "basic" state
typedef enum {CTL_OPTN, CTL_Li, CTL_di, CTL_LE, CTL_OF} ctl_state_t;
static ctl_state_t g_ctl_state;

  // externally reachable objects
bool volatile     gb_ctl_can_sleep;    // this FSM can sleep

  // hardware resources
static PwmOut         *gp_led_c;
static DigitalOut     *gp_led_r;
static DigitalOut     *gp_led_l;

static AnalogIn       *gp_ldr;
static Ticker         g_100ms_tick;
static Timeout        g_timeout_50ms;

  // internal objects
static bool           gb_ctl_initd;            // true after call to rf_init()
static bool volatile  gb_ldr_meas_evnt;
static bool volatile  gb_di_meas_evnt;

// end of extended state  -----------------------------------------------------

// ISRs -----------------------------------------------------------------------

  //LED_R/LED_L ISR
static void led_to_isr(void)
{
  //in order to turn off at 50ms and not wait for all code to be run
  g_timeout_50ms.detach();
  *gp_led_l = 0;
  *gp_led_r = 0;
}

static void ldr_meas_isr(void){
  gb_ldr_meas_evnt = true;
}

static void di_meas_isr(void){
  gb_di_meas_evnt = true;
}

// end of ISRs  ---------------------------------------------------------------

//Custom Functions ------------------------------------------------------------
static void        short_press_det(void);
static void        long_press_det(void);
static uint16_t    display_menu(uint8_t counter);
static uint16_t    display_number(int16_t value);

//Required local variables ----------------------------------------------------
static uint8_t     cnt;
static uint8_t     bright;


// FSM  ----------------------------------------------------------------
void ctl_fsm (void) {
  if (gb_ctl_initd) {  // protect against calling ctl_fsm() w/o a previous call to ctl_init()
    switch (g_ctl_state) {
      case CTL_OPTN:
        //Options Menu in operation
        gb_di_meas_evnt  = false;
        gb_ldr_meas_evnt = false;
        gb_rf_done_msg   = false;
			
        g_display_segs = display_menu(cnt);
        gb_display_update_msg = true;

        if(gb_swm_msg)
        {
          gb_swm_msg = false;
          short_press_det();
          cnt += (cnt == 3) ? -cnt : 1;
        } //gb_swm_msg

        if(gb_swm_long_msg)
        {
          gb_swm_long_msg = false;
          long_press_det();
          switch(cnt){
            case 0:
              g_ctl_state = CTL_Li;
              g_100ms_tick.attach(ldr_meas_isr, 100ms);      // 10 Hz
              /***********************VERBOSE SERIAL OUTPUT***********************/
              #ifdef VERBOSE
                printf("=> Li\r\n");
              #endif //VERBOSE
            break;
            case 1:
              g_ctl_state = CTL_di;
              g_100ms_tick.attach(di_meas_isr, 100ms);      // 10 Hz
              /***********************VERBOSE SERIAL OUTPUT***********************/
              #ifdef VERBOSE
                printf("=> di\r\n");
              #endif //VERBOSE
            break;
            case 2:
              g_ctl_state = CTL_LE;
              g_display_segs = 0x4040;
              gp_led_c->pulsewidth_ms(50);
              /***********************VERBOSE SERIAL OUTPUT***********************/
              #ifdef VERBOSE
                printf("=> LE\r\n");
              #endif //VERBOSE
            break;
            case 3:
              g_ctl_state = CTL_OF;
              gb_display_off_msg = true;
              /***********************VERBOSE SERIAL OUTPUT***********************/
              #ifdef VERBOSE
                printf("Off\r\n");
              #endif //VERBOSE
            break;
          } //switch(cnt)
        } //gb_swm_long_msg
      break; //case CTL_OPTN:

      case CTL_Li:
        //CTL_Li in operation
        gb_di_meas_evnt = false;
        gb_rf_done_msg  = false;
        
        if(gb_ldr_meas_evnt)
        {
          gb_ldr_meas_evnt = false;
          bright = gp_ldr->read_u16()*100/65535;
          g_display_segs = display_number(bright);
        }
        if (gb_swm_msg)
        {
          gb_swm_msg = false;
          short_press_det();
        }

        //Back to Options Menu
        if (gb_swm_long_msg)
        {
          gb_swm_long_msg = false;
          long_press_det();
          g_100ms_tick.detach();
					
          cnt = 0;
          g_ctl_state = CTL_OPTN;
					
		  /***********************VERBOSE SERIAL OUTPUT***********************/
          #ifdef VERBOSE
            printf("<= Li\r\n");
          #endif //VERBOSE
        }
      break; //case CTL_Li:

      case CTL_di:
        //CTL_di in operation
        gb_ldr_meas_evnt = false;
        
        // start a new range measurement every 100 ms
        if(gb_di_meas_evnt)
        {
          gb_di_meas_evnt = false;
          gb_rf_start_msg = true;
        }
        // when the measurement is complete, update variable g_display_segs
        if (gb_rf_done_msg)
        {
          gb_rf_done_msg = false;
          g_display_segs = display_number(g_rf_range_cm);
        }
        if (gb_swm_msg)
        {
          gb_swm_msg = false;
          short_press_det();
        }
        //Back to Options Menu
        if (gb_swm_long_msg)
        {
          gb_swm_long_msg = false;
          long_press_det();
          g_100ms_tick.detach();

          cnt = 1;
          g_ctl_state = CTL_OPTN;
					
		  /***********************VERBOSE SERIAL OUTPUT***********************/
          #ifdef VERBOSE
            printf("<= di\r\n");
          #endif //VERBOSE
        }
      break; //case CTL_di:

      case CTL_LE:
        //CTL_LE in operation
        gb_di_meas_evnt  = false;
        gb_ldr_meas_evnt = false;
        gb_rf_done_msg   = false;
    /*******************LED-CENTER PWM ALREADY RUNNING*********************/
		/********************nothing to see here******************************/
			
        if (gb_swm_msg)
        {
          gb_swm_msg = false;
          short_press_det();
        }
				
        //Back to Options Menu
        if (gb_swm_long_msg)
        {
          gb_swm_long_msg = false;
          long_press_det();
          *gp_led_c = 0;
          cnt = 2;
          g_ctl_state = CTL_OPTN;
		  
          #ifdef VERBOSE
            printf("<= LE\r\n");
          #endif //VERBOSE
        }
      break; //case CTL_LE:

      case CTL_OF:
        //CTL_OF in operation
        gb_di_meas_evnt  = false;
        gb_ldr_meas_evnt = false;
        gb_rf_done_msg   = false;
        gb_swm_msg       = false;
        /********************nothing to see here******************************/
			
        //Back to Options Menu
        if (gb_swm_long_msg)
        {
          gb_swm_long_msg = false;
          cnt = 0;
          g_ctl_state = CTL_OPTN;
		  
		  /***********************VERBOSE SERIAL OUTPUT***********************/
          #ifdef VERBOSE
            printf("On\r\n");
          #endif //VERBOSE

          //This time turn on the screen
          gb_display_on_msg = true;
        }
			break; //case CTL_OF:
    //  ----------------------------------------------------------------
    } // switch (ctl_state)

    __disable_irq();
    if(!gb_ldr_meas_evnt && !gb_di_meas_evnt 
			  && !gb_swm_msg && !gb_swm_long_msg) {
      gb_ctl_can_sleep = true;
    }
    __enable_irq();
  } // if (gb_ctl_initd)
}
// end of FSM ----------------------------------------------------------

// initialize FSM machinery  -------------------------------------------
void ctl_init(DigitalOut *led_l, PwmOut *pwm_led_c, DigitalOut *led_r, AnalogIn *ldr)
{
  if (!gb_ctl_initd)
  {
    gb_ctl_initd = true;   // protect against multiple calls to ctl_init

    // initialize state
    g_ctl_state = CTL_OF;

    // init isr flags
    gb_ldr_meas_evnt = false;
    gb_di_meas_evnt  = false;
		
    //main parameters pointer address assign
    gp_led_l = led_l;
    gp_led_r = led_r;
    gp_led_c = pwm_led_c;
    gp_ldr = ldr;
	  // initial actions
    *gp_led_l = 0;
    *gp_led_r = 0;
    *gp_led_c = 0;
    gp_led_c -> period_ms(100);
	
    

  }
}
// end of FSM initialization  ------------------------------------------

// Functions ------------------------------------------------------------
/******************************************************************************
* Generates a readable menu output to be sent to hardware given a option
* value. Also, updates the brightness value to maximum in order to be readable.
*
* @param  counter  the current menu value selected.
* @return        the 16bit hex word to be sent to hw.
*******************************************************************************/
static uint16_t display_menu(uint8_t counter)
{
  const uint16_t code[]={0x3810,0x5E10,0x3879,0x3F71};

  g_display_brightness = 100;
  gb_display_brightness_msg = true;

  return code[counter];
}

/******************************************************************************
* Parses the intended value between 0 to 99 included to a 16bit word,
* concatenating each parsed digit. Contemplates error message (value == -1)
* and overthreshold values (values over 99). As negative values are
* treated as error codes, it is required to use 16bit values even when using 
* up to <99> numbers. 
*
* @param  value  the actual value to be shown from 0 upto 99.
* @return        the 16bit hex word to be sent to hw.
******************************************************************************/
static uint16_t display_number(int16_t value)
{
  uint16_t num = 0;
  uint8_t digit_right = value % 10;
  uint8_t digit_left  = value / 10;

  if (value == -1)
  {
    num = 0x7950;   //Er
    g_display_brightness = 100;
  }
  else if (value > 99)
  {
    num = 0x4040;   //--
    g_display_brightness = 100;
  }
  else
  {
    num = (to_7seg(digit_left) << 8) | (to_7seg(digit_right));
    g_display_brightness = value;
  }

  gb_display_update_msg = true;
  gb_display_brightness_msg = true;
  
  /***********************VERBOSE SERIAL OUTPUT***********************/
  #ifdef VERBOSE
    printf("    %d\r\n", value);
  #endif //VERBOSE

  return num;
}
/**
 LED blink Handling function (50ms blink) 
*/
static void short_press_det(void)
{
  *gp_led_r = 1;
  g_timeout_50ms.attach(led_to_isr,50ms);
}
/**
 LED blink Handling function (50ms blink) 
*/
static void long_press_det(void)
{
  *gp_led_l = 1;
  g_timeout_50ms.attach(led_to_isr,50ms);
}
