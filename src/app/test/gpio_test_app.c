/*******************************************************************************
 * (c) Copyright 2012 Actel Corporation. All rights reserved.
 * (c) Copyright 2012 Emcraft Systems.  All rights reserved.
 *
 *  Simple SmartFusion2 microcontroller subsystem (MSS)
 *  GPIO and UART example program.This sample program is targeted at
 *  the Emcraft Systems SmartFusion2 SOM Starter Kit.
 *  It blinks the board's LEDs and prints messages onto UART using a delay loop.
 *
 *  IMPORTANT NOTE:
 *          If you re-generate this SoftConsole project in context of the Libero
 *          project the SoftConsole project is included with, Libero will
 *          set the following line:
 *
 *  #define MSS_SYS_MDDR_CONFIG_BY_CORTEX 1
 *          in
 *  g4m_system_MSS_CM3_hw_platform/drivers_config/sys_config/sys_config.h.
 *
 *          This is because the DDR interface is enabled in the Libero project.
 *          However, the DDR initialization code included with the current rev
 *          of Libero is not compatible with Emcraft Systems's SOM design.
 *          When called, the DDR initialization code will
 *          loop in config_ddr_subsys().
 *          Disable MSS_SYS_MDDR_CONFIG_BY_CORTEX to let this
 *          SoftConsole project run.
 *
 *  NOTE:   This sample project expects that the MSS GPIOs are connected to
 *          external pins through the FPGA fabric as follows:
 *              - MSS_GPIO_1: input from pin B20
 *              - MSS_GPIO_2: input from pin C19
 *          MSS UART0 is used as the serial console.
 *
 */

#include "../drivers/mss_gpio/mss_gpio.h"
#include "../drivers/mss_uart/mss_uart.h"
#include "../drivers_config/sys_config/sys_config.h"
#include "app.h"
#include <stdio.h>

void gpio_sample_app(){
	int32_t delay_count = 0;
		char *memptr = (char *) RAM_BASE_ADDR;

		/*
		 * Initialize MSS GPIOs.
		 */
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

		/*
		 * Set initial delay used to blink the LED.
		 */
		delay_count = DELAY_LOAD_VALUE;

		/*
		 * Initialize the UART0 controller (115200, 8N1)
		 */
		 MSS_UART_init(&g_mss_uart0, MSS_UART_115200_BAUD, MSS_UART_DATA_8_BITS |
		 MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);


		MDDR_Lpddr_Setup();

		sprintf(memptr, "%s\n\r", "Hello, SmartFusion2!");

		/*
		 * Infinite loop.
		 */
		uint32_t val = 0, cnt=4;
		for (;;) {

			/*
			 * Decrement delay counter.
			 */
			--delay_count;

			/*
			 * Check if delay expired.
			 */
			if (delay_count <= 0) {
				/*
				 * Reload delay counter.
				 */
				delay_count = DELAY_LOAD_VALUE;

				/*
				 * Toggle GPIO output pattern by doing an exclusive OR of
				 * all pattern bits with ones.
				 */
				/*
				 val = MSS_GPIO_get_outputs() & (DS3_LED_MASK | DS4_LED_MASK);
				 val ^= (DS3_LED_MASK | DS4_LED_MASK);
				 MSS_GPIO_set_outputs( val );
				 */
				val = MSS_GPIO_get_outputs() & (DS3_LED_MASK | DS4_LED_MASK);
				val = ((cnt = ~cnt) & (DS3_LED_MASK | DS4_LED_MASK));
				MSS_GPIO_set_outputs(val);
				/*
				 * Print the message to the UART console
				 */
				MSS_UART_polled_tx_string(&g_mss_uart0, "\n\rHello, SmartFusion2!");//(const uint8_t *) memptr);

			}
		}
}
