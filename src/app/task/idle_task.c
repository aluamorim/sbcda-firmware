/*
 * idle_task.c
 *
 *  Created on: 10/02/2017
 *      Author: Anderson
 */

/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "../drivers/mss_gpio/mss_gpio.h"
#include "../drivers/mss_rtc/mss_rtc.h"
#include "../drivers/mss_watchdog/mss_watchdog.h"
#include "app.h"
#include "service.h"

/**
 * --------------------------------------------------------------------------
 *  Idle_Task
 * --------------------------------------------------------------------------
 *
 * 	Dummy task created for checking if RTOS is alive
 * 	The task simple blink SF2 board LEDs.
 * 	This task is responsible for reloading the WDT
 * 	counter.
 */
void Idle_Task(void *pvParameters) {

	uint8_t led_blinker_value;
	mss_rtc_calendar_t calendar;
	uint8_t debug_buffer[350];

	MSS_GPIO_init();
	/*
	 * Configure MSS GPIOs.
	 */
	MSS_GPIO_config(DS3_LED_GPIO, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(DS4_LED_GPIO, MSS_GPIO_OUTPUT_MODE);
	/*
	 * Set LEDs to initial state
	 */
	MSS_GPIO_set_outputs(MSS_GPIO_get_outputs() | DS3_LED_MASK | DS4_LED_MASK);

	led_blinker_value = 4;
	for (;;) {
		// check to reload WDT
		MSS_WD_reload();

		/**************************************************************************************
		 * WARNING: this code is for debug purposes only. It
		 * can be safely removed in the final version of the
		 * board.
		 **************************************************************************************/
		led_blinker_value = ~led_blinker_value;
		MSS_GPIO_set_outputs((led_blinker_value & (DS3_LED_MASK | DS4_LED_MASK)));


//		sprintf(debug_buffer,
//				"\r> [Ready= %02d; Fail= %02d; Free= %02d] [Active Decoders: %d] [Disc = %d; Total = %d]                ",
//				PTT_ReadyPackages_Available(),
//				DDS_GetDecodingFails_Count(),
//				PTT_FreePackages_Available(),
//				DDS_NUMBER_OF_DECODERS - DDS_GetAvailableDecoders(),
//				DDS_GetDiscardDetections_Count(),
//				DDS_GetTotalDetections_Count()
//				);
//		print(debug_buffer);
		/**************************************************************************************/
	}
}

