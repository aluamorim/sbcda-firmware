/*
 * rtc_cmd.c
 *
 *  Created on: 13/02/2017
 *      Author: Anderson
 */

#include "service.h"
#include "../drivers/mss_rtc/mss_rtc.h"


//#define RTC_PRESCALER	0x2FAF070 // CLK=50MHz //0x03FFFFFFu
#define RTC_PRESCALER	1000000 // CLK=1MHz
#define RTC_INIT_CODE	12345

static int rtc_initialized;

/**
 *
 *  RTC_Setup
 *
 *	\brief Initializes RTC system to calendar mode, setting prescale do 1Hz
 *
 */
void RTC_Setup(){

	/** Init RTC **/
	MSS_RTC_init(MSS_RTC_CALENDAR_MODE, RTC_PRESCALER);
	MSS_RTC_enable_irq(); // enables RTC wake-up IRQ
	rtc_initialized = RTC_INIT_CODE;
}

/*
 * --------------------------------------------------------------------------
 *  RTC_Set
 * --------------------------------------------------------------------------
 *	Sets Load Value to RTC
 */
void RTC_Set(mss_rtc_calendar_t * calendar){
	MSS_RTC_set_calendar_count(calendar);
}

/*
 * --------------------------------------------------------------------------
 *  RTC_Reset
 * --------------------------------------------------------------------------
 */
void RTC_Reset (){
	MSS_RTC_reset_counter();
}

/*
 * --------------------------------------------------------------------------
 *  RTC_Stop
 * --------------------------------------------------------------------------
 */
void RTC_Stop (){
	MSS_RTC_stop();
}

/*
 * --------------------------------------------------------------------------
 *  RTC_Start
 * --------------------------------------------------------------------------
 */
void RTC_Start (){
	MSS_RTC_start();
}


/*
 * --------------------------------------------------------------------------
 *  RTC_Is_Initialized
 * --------------------------------------------------------------------------
 */
uint8_t RTC_Is_Initialized (){
	return rtc_initialized==RTC_INIT_CODE;
}

/*
 * --------------------------------------------------------------------------
 *  RTC_Get
 * --------------------------------------------------------------------------
 * Gets the current value of the RTC
 */
void RTC_Get(mss_rtc_calendar_t * calendar){
	MSS_RTC_get_calendar_count(calendar);
}
