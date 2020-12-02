/*
 * hw_setup.c
 *
 *  Created on: 24/05/2017
 *      Author: Anderson
 */
#include "../drivers_config/sys_config/sys_config.h"
#include "../drivers/mss_gpio/mss_gpio.h"
#include "../drivers/mss_uart/mss_uart.h"
#include "../drivers/mss_watchdog/mss_watchdog.h"
#include "../drivers/mss_spi/mss_spi.h"
#include "app.h"
#include "service.h"

void HW_Setup( void ) {
	/*
	 * Initialize the UART0 controller (115200, 8N1)
	 */

	MSS_UART_init(&g_mss_uart0, MSS_UART_115200_BAUD,
	MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

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
	 * Init Watchdog timer
	 * WDT enable/disable is part of the hardware flow.
	 * By default it is not enabled in SF2-MS050, requiring
	 * a hardware change to enable it. Nevertheless the initial
	 * confiuration is placed here and the reloads are made
	 * in the application's Idle Task, just in case the WDT
	 * is enabled.
	 */
	MSS_WD_init();
	MSS_WD_enable_wakeup_irq();
	MSS_WD_enable_timeout_irq();
	MSS_WD_reload();

	MDDR_Lpddr_Setup();

	// config the AD9874 through SPI1 iface
	AD987_Setup();
}
