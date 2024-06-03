#include "display.h"

/******************************************************************************
If low mux transistor switching speed either increase base resistance
or use this: (do not increase over 20 to avoid flickering)
******************************************************************************/
#if 1 
 #define SHADOW_HACK
 #define SHADOW_FACTOR 5
#endif
/*****************************************************************************/

// extended state ------------------------------------------------------
  // "basic" state
typedef enum {DPL_OFF, DPL_ON} dpl_state_t;
static dpl_state_t g_dpl_state;
  //  ------------------------------------------------------------------

  // externally reachable objects
bool        gb_display_on_msg;
bool        gb_display_off_msg; 

bool        gb_display_update_msg;
bool        gb_display_brightness_msg;  

bool 	    gb_display_select_msg;

uint8_t		g_display_brightness;
uint16_t	g_display_segs;

  // internal outbound objects
bool volatile        gb_display_can_sleep;

  // hardware resources
  // complete this code to achieve the FSM functionality  ++++++++++++++
static PwmOut  *gp_dsl;
static PwmOut  *gp_dsr;
static BusOut  *gp_seven_seg;

  // internal objects
  // complete this code to achieve the FSM functionality  ++++++++++++++
static bool     gb_dpl_initd;            // true after call to display_init()
static uint16_t dpl_code;
static uint8_t  pulse_width;

/**********************PULSE WIDTH SPECIFIED VALUE*******************/
static const uint8_t PWM_PERIOD_US = 40;

// mux stuff
static Ticker         g_mux_tick;
static bool volatile  gb_mux_evnt;

static void mux_isr (void) {
  gb_mux_evnt = true;
}


// FSM  ----------------------------------------------------------------
void display_fsm (void) {
  if (gb_dpl_initd) 
  {
	switch (g_dpl_state) 
	{
	  case DPL_OFF:
			gb_display_off_msg = false;
		  gb_display_update_msg = false;
		  gb_display_brightness_msg = false;
		  gb_mux_evnt = false;
		  gb_display_select_msg = false;

  		if (gb_display_on_msg)
		  {
		    gb_display_on_msg = false;
		    //CRITERIO 5: Asignacion de valor y brillo en gb_display_on_msg
		    //			y no en estado DPL_ON. Cambio orden FSM en DPL_ON.
		    gb_display_update_msg = true;
		    gb_display_brightness_msg = true;
		    //-------------------------------------------------------------------
			  g_mux_tick.attach(mux_isr, 4ms);          // 250 Hz, 125 Hz per Display
		    g_dpl_state = DPL_ON;
		  } //gb_display_on_msg
		break;

	  case DPL_ON:
		  gb_display_on_msg = false;
		
		  if (gb_display_update_msg)
		  {
		    gb_display_update_msg = false;
		  
		    //adjudicar g_display_segs
		    dpl_code = g_display_segs;
		  
		  } //gb_display_update_msg
		
		  if (gb_display_brightness_msg)
		  {
		    gb_display_brightness_msg = false;
		  
		    //pulse_width new value compute
		    //cuidado 8 bit : 99*40 == 3960 >> 255
		    pulse_width = g_display_brightness*PWM_PERIOD_US/100;	
		  } //gb_display_brightness_msg
			
		  // display multiplex
      if (gb_mux_evnt)
	    {
        gb_mux_evnt = false;
	      gb_display_select_msg = !gb_display_select_msg; 		//toggle
				if (gb_display_select_msg) 
		    {
		      *gp_dsr = 0;
					/*********THIS SIGNIFICANTLY IMPROVES SHADOWS********/
					#ifdef SHADOW_HACK
					  for (int i = 0; i < SHADOW_FACTOR; i++){*gp_dsr = 0;}
					#endif
		      *gp_seven_seg = (dpl_code >> 8);
		      gp_dsl->pulsewidth_us(pulse_width);
		    } //gb_display_select_msg
		    else 
		    {
		      *gp_dsl = 0;
					/*********THIS SIGNIFICANTLY IMPROVES SHADOWS********/
					#ifdef SHADOW_HACK
					  for (int i = 0; i < SHADOW_FACTOR; i++){*gp_dsl = 0;}
					#endif
		      *gp_seven_seg = dpl_code;
		      gp_dsr->pulsewidth_us(pulse_width);
		    } //!gb_display_select_msg
      } //gb_mux_evnt
		
	    if (gb_display_off_msg) 
		  {
		    gb_display_off_msg = false;
		    *gp_dsl = 0;
		    *gp_dsr = 0;
		    g_dpl_state = DPL_OFF;
		  } //gb_display_off_msg
		break;
	}
	
  // complete this code to achieve the FSM functionality (can sleep?) ++
    __disable_irq();
    if(!gb_display_on_msg && !gb_display_off_msg && !gb_display_update_msg 
		 && !gb_display_brightness_msg && !gb_mux_evnt) {
      gb_display_can_sleep = true;
    }
    __enable_irq();
  }
}
// end of FSM ----------------------------------------------------------

// initialize FSM machinery  -------------------------------------------
void display_init(PwmOut *dsl, PwmOut *dsr, BusOut *seven_seg){
  if (!gb_dpl_initd) {
	gb_dpl_initd = true;  
	
	//init state
	g_dpl_state = DPL_OFF;
	
	//main parameters pointer address assign 
	gp_dsl = dsl;
	gp_dsr = dsr;
	gp_seven_seg = seven_seg;
	
	//init value
	*gp_dsl = 0;
	*gp_dsr = 0;
	*gp_seven_seg = 0;
	
	gp_dsl->period_us(PWM_PERIOD_US);
	gp_dsr->period_us(PWM_PERIOD_US);
  }

}
