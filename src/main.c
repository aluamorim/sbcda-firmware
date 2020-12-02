
/**
 *
 * @mainpage SBCDA Decoder Firmware for Smartfusion2 MSS CM3
 *
 *
 * <pre>
 * The SBCDA Decoder Firmware is responsible for the software portions
 * of the PTT Decoding Algorithm. It is meant to run at a Cortex-M3
 * processor within a SmartFusion2 FPGA Microcontroller System.
 *
 * Se the 'Modules' section to navigate throughout the documentation.
 * </pre>
 */

/**
 * @file main.c
 * @brief Firmware' starting point.
 *
 * The starting point of the SBCDA Firmware.
 * This function simply calls the sbcda_decoder_app
 * @author Anderson Amorim
 */


#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "../drivers_config/sys_config/sys_config.h"
#include "../drivers/mss_gpio/mss_gpio.h"
#include "../drivers/mss_uart/mss_uart.h"
#include "../drivers/mss_watchdog/mss_watchdog.h"
#include "app.h"
#include "service.h"
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
/**
 * @endcond
 */

#if (MSS_SYS_MDDR_CONFIG_BY_CORTEX == 1)
#error "Please turn off DDR initialization! See the comment in this file above. "
#endif



void NMI_Handler(void) { // clear WDT interrupt
	print("\n\r WDT Timeout IRQ n\r");
	MSS_WD_clear_timeout_irq();
	Reset_Handler(); // soft-reset
}

void RTC_Wakeup_IRQHandler(void) {
	print("\n\r RTC wake up IRQ \n\r");
	MSS_RTC_clear_irq();
}

void WdogWakeup_IRQHandler(void) {
	print("\n\r WDT wake up IRQ \n\r");
	MSS_WD_clear_wakeup_irq();
}

/**
 * main()
 */
int main() {

	//configure HW and memory for all applications
	HW_Setup();

	print("\n\r\n\r**************************************"
			"\n\r* SBCDA Decoder Firmware Evaluation  *   "
			"\n\r* Version 0.1  Date: 08/2017         * "
			"\n\r* Author: CRN-INPE                   * "
			"\n\r**************************************"
			"\n\r> Starting Decoder Applicaton... ");

	sbcda_decoder_app();
	
	return 0;
}
