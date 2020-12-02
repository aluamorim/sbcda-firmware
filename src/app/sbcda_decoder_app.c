/**
 * @file sbcda_decoder_app.c
 *
 *  @brief SBCDA Firmware Application
 *
 * <pre>
 *  Creates the firmware`s tasks and starts the
 *  FreeRTOS scheduler
 *  Task priorities:
 *  	ADC_Sampler_Task 	= 	2
 *  	PTT_Decoding_Task 	= 	1
 *  	CMD_Parser_Task 	= 	1
 *  	Housekeeping_Task 	= 	1
 *
 *  </pre>
 *  @author: Anderson Amorim
 *
 */

/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"
#include "app.h"
#include "../drivers/mss_gpio/mss_gpio.h"

void sbcda_decoder_app() {

	/*****************************************************
	 * Idle Task: reloads the WDT counter periodically
	 */
	if (xTaskCreate(Idle_Task,
	"Idle",
	configMINIMAL_STACK_SIZE * 2,
	NULL,
	tskIDLE_PRIORITY,
	NULL) != pdPASS) {
		print("\n\r> Could not create Idle_Task");
	}
	/****************************************************/

	/*****************************************************
	 * Command Task: handles communication with the
	 * on-board computer
	 */
	if (xTaskCreate(Command_Parser_Task, "Cmd",
	configMINIMAL_STACK_SIZE * 4,
	NULL, tskIDLE_PRIORITY,
	NULL) != pdPASS) {
		print("\n\r> Could not create Command Parser Task");
	}
	/****************************************************/

	/*****************************************************
	 * Housekeeping Task: collects and stores HK
	 * information.
	 */
	if (xTaskCreate(HouseKeeping_Task, "Hk",
	configMINIMAL_STACK_SIZE * 4,
	NULL, tskIDLE_PRIORITY,
	NULL) != pdPASS) {
		print("\n\r> Could not create Housekeeping Task ");
	}
	/****************************************************/

	/*****************************************************
	 * ADC Sampler Task: reads a 2048 ADC samples window
	 * and stores it on an ADC Package.
	 */
	if (xTaskCreate(ADCSampler_Task, "Adc",
	configMINIMAL_STACK_SIZE * 2,
	NULL, tskIDLE_PRIORITY+1,
	NULL) != pdPASS) {
		print("\n\r> Could not create ADC Sampler Task");
	}
	/****************************************************/

	/*****************************************************
	 * PTT Decoding: Handles the Detection/Decoding
	 * processes for PTT Signals.
	 */
	if (xTaskCreate(PTTDecoder_Task, "Ptt",
	configMINIMAL_STACK_SIZE * 6,
	NULL, tskIDLE_PRIORITY,
	PTT_GetTaskHandler ()) != pdPASS) {
		print("\n\r> Could not create PTT Reader Task");
	}
	/****************************************************/

	/* Start the tasks and timer running. */
	print("\n\r> Applicaton Started!\n\r\n\r");
	vTaskStartScheduler();

	// code shoud never get to this point
	print("\n\r> Error!. The program should not reach this point."
			"Big no-no for you!!\n\r\n\r");
	for (;;)
		;
}
