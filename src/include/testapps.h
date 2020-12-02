/*
 * app.h
 *
 *  Created on: 06/02/2017
 *      Author: Anderson
 */

#ifndef APP_TESTAPPS_H_
#define APP_TESTAPPS_H_



/*
 * Delay loop down counter load value.
 */
#define DELAY_LOAD_VALUE     0x00100000

/*
 * LEDs GPIOs
 */
#define DS3_LED_GPIO	MSS_GPIO_1
#define DS4_LED_GPIO	MSS_GPIO_2
#define DS3_LED_MASK	MSS_GPIO_1_MASK
#define DS4_LED_MASK	MSS_GPIO_2_MASK

#define RAM_BASE_ADDR	0xA0000000

void gpio_sample_app();
void i2c_sample_app();

#endif /* APP_TESTAPPS_H_ */
