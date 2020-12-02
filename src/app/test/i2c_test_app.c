/*
 * i2c_app.c
 *
 *  Created on: 06/02/2017
 *      Author: Anderson
 */

/*
 * sample_gpio.c
 *
 *  Created on: 06/02/2017
 *      Author: Anderson
 */

#include "../drivers/mss_gpio/mss_gpio.h"
#include "../drivers/mss_uart/mss_uart.h"
#include "../drivers/mss_i2c/mss_i2c.h"
#include "../drivers_config/sys_config/sys_config.h"
#include "app.h"
#include <stdio.h>
#include <stdlib.h>

#if (MSS_SYS_MDDR_CONFIG_BY_CORTEX == 1)
#error "Please turn off DDR initialization! See the comment in this file above."
#endif


static int new_rx_data;
// local function prototype
void slave_write_handler(mss_i2c_instance_t * this_i2c, uint8_t * p_rx_data,
		uint16_t rx_size) {
	new_rx_data = 1;
	printf ("\n\r [NEW DATA] Size= %d ", rx_size);

}

void i2c_sample_app() {
	int32_t delay_count = 0;
	char *memptr = (char *) RAM_BASE_ADDR;
	uint8_t slave_tx_buffer[60], slave_rx_buffer[60];
	new_rx_data = 0;

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

	printf("\n\r Helloooooo!   ");
	/*
	 * I2C Configuration
	 */
	// Initialize the MSS I2C driver with its I2C serial address and
	// serial clock divider.
	MSS_I2C_init(&g_mss_i2c0, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256);

	// Specify the buffer used to store the data written by the I2C master.
	MSS_I2C_set_slave_rx_buffer(&g_mss_i2c0, slave_rx_buffer,
			sizeof(slave_rx_buffer));
	// Specify the transmit buffer containing the data that will be
	// returned to the master during read and write-read transactions.
	MSS_I2C_set_slave_tx_buffer(&g_mss_i2c0, slave_tx_buffer,
			sizeof(slave_tx_buffer));
	MSS_I2C_register_write_handler(&g_mss_i2c0, (mss_i2c_slave_wr_handler_t) slave_write_handler);

	sprintf(slave_tx_buffer, " %s ", "  0123456789  ");
	printf("\n\r TX BUFFER: %s ", slave_tx_buffer); // test uart

	MSS_I2C_enable_slave(&g_mss_i2c0);
	uint32_t val = 0, cnt = 4, it = 0;
	for (;;) {

		if (--delay_count <= 0) {
			delay_count = DELAY_LOAD_VALUE;
			/*
			 * Toggle GPIO output pattern by doing an exclusive OR of
			 * all pattern bits with ones.
			 */
			val = MSS_GPIO_get_outputs() & (DS3_LED_MASK | DS4_LED_MASK);
			val = ((cnt = ~cnt) & (DS3_LED_MASK | DS4_LED_MASK));
			MSS_GPIO_set_outputs(val);

			if (new_rx_data) {
				new_rx_data = 0;

				printf("\n\r ** Rx: %s ** ", slave_rx_buffer);
				printf("\n\r I2C Slave [%d]: Status=%d Idx= %d  [%d] ", g_mss_i2c0.is_slave_enabled,
						g_mss_i2c0.slave_status, g_mss_i2c0.slave_tx_idx, it++);
				MSS_I2C_enable_slave(&g_mss_i2c0);
			}
			fflush(stdout);
		}

	}
}
