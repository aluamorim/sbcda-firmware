/**
 * @file app.h
 *
 * @brief SBCDA Decoder Application
 *
 * \defgroup SBCDA_APP SBCDA_APP
 * @{
 */

#ifndef INCLUDE_APP_H_
#define INCLUDE_APP_H_

#include <stdint.h>
#include "service.h"
#include "../drivers/mss_uart/mss_uart.h"
#include "../drivers/mss_gpio/mss_gpio.h"
#include "../drivers/mss_sys_services/mss_sys_services.h"
/*
 * Delay loop down counter load value.
 */
#define DELAY_LOAD_VALUE     0x00010000

/*
 * LEDs GPIOs
 */
#define DS3_LED_GPIO	MSS_GPIO_2
#define DS4_LED_GPIO	MSS_GPIO_3
#define DS3_LED_MASK	MSS_GPIO_2_MASK
#define DS4_LED_MASK	MSS_GPIO_3_MASK

#define RAM_BASE_ADDR	0xA0000000

#define SLAVE_SER_ADDR 	0x10u

typedef struct {

	uint64_t start_time;
	uint64_t window_time;
	uint64_t detect_time;
	uint64_t decode_time;
	uint64_t wait_time;

} TimeReport_Typedef;

/**
 * \defgroup App_Tasks App_Tasks
 * \ingroup SBCDA_APP
 *
 * Application Tasks
 * @{
 */

/**
 *
 * Command_Parser_Task
 *
 * Task responsible for setting the current operation mode and its
 * corresponding output package buffer. The Command Parser task is
 * also responsible for executing configuration commands sent by
 * the on-board computer. The result of each configuration command
 * or request in configuration mode is a Configuration Data Package
 * starting with the identifier byte 0x10.
 * This task has low execution priority and can only run when
 * the ADCSampler_Task and the PTTDecoder_Task are suspended.
 * This behavior gives a small window of time for the on-board
 * computer execute configuration commands or request HK information.
 * Therefore, the on board computer must always check the type of a
 * received data package (first byte) to be sure of which
 * operation mode is currently set.
 */
void Command_Parser_Task		(void *pvParameters);

/**
 *
 * HouseKeeping_Task
 *
 * This task constantly reads housekeeping information
 * and keeps an updated Housekeeping Package. The collected
 * information are defined by the Housekeeping "configuration
 * word."
 * The housekeeping information includes:
 *
 * 1 - RTC Current time tag;
 * 2 - ADC 7894 current RMS value;
 * 3 - Current sensors A and B updated values;
 * 4 - PLL Synchronization flag;
 * 5 - Over-current register flag;
 *
 * The HK info is stored into a single HK Package,
 * provided by the HK Service Layer.
 * All Housekeeping measures are enabled by default.
 * If the on-board computer deactivates one or more
 * HK measures, their corresponding space is simply
 * filled with zeros.
 * The configuration word masks defines which
 * HK measure is enabled or not. It can be updated
 * by the on-board computer using the HK_CONFIG
 * command with the firmware in Configuration Mode.
 * The configuration word is a 2-byte integer whose
 * bits are mapped as follows:
 *
 *<pre>
 * 	0 	- RTC time tag
 * 	1 	- Current sensors
 * 	2 	- ADC RMS
 * 	3 	- PLL Synch flag
 * 	4 	- Over - current
 * 	5-15 - Reserved for future use
 *</pre>
 */
void HouseKeeping_Task		( void *pvParameters );


/**
 * PTTDecoder_Task
 *
 * The PTT Decoding task is responsible for coordinating the decoding process of newly detected PTT
 * signals, as much as building PTT Packages. It rellies on services provided by the PTT Service
 * and the DDS Layers.
 * The PTT task runs in cycles of 10 ms, called windows.
 * During each window the task performs the following activities:
 * <pre>
 *   1.	Run a Detection Loop to identify new PTT signals;
 *   2.	Send a Decoding Start Operation signal if there are any active decoders;
 *   3.	Run a Decoding Loop, procedure that adds new decoded bits to the currently building PTT Packages;
 *   4.	Check if the firmware has received a PTT_POP command and, in this case, update the pointer of the PTT Packages FIFO  ;
 *   5.	Wait until the next window starts.
 * </pre>
 * Steps 1 to 3 are executed in high priority mode. The task cannot be preempted during these steps.
 * Together they last about 4ms (40% of the window time). During the remaining time,
 * the PTT task runs in low priority mode, allowing other tasks to control the processor.
 * An interrupt signal is generated on GPIO_0 pin to trigger a new window.
 * When this interruption is detected the system is forced to reenter the high priority state and restart the decoding process.
 * The on-board computer can pause or resume the decoding process by using the PTT_PAUSE and PTT_RESUME
 * commands in Configuration Mode. When the decoder is paused steps 1, 2 and 3 are not executed.
 */
void PTTDecoder_Task		( void *pvParameters );


/**
 *
 * ADCSampler_Task
 *
 * This task is responsible for collecting ADC Sampler windows (2048 adc samples)
 * and  storing them into the ADC Sampler Buffer. The ADC Sampler
 * is high priority task, it  can not be preempted while collecting
 * a ADC window to avoid losing samples.
 * It runs on a infinite loop alternating between two states:
 *
 *<pre>
 * - ADC_IDLE: the tasks suspends for 0,5s, allowing other tasks to run;
 *
 * - ADC_BUSY: the tasks collects 2048 samples from the ADC Sampler FIFO
 *             in the Decoder Unit and stores them into a ADC Sampler Package.
 *             After the reading is done the tasks automatically switches to
 *             ADC_IDLE state.
 * </pre>
 *   When the task is in ADC_BUSY state it is not preempted,
 *   therefore other critical process, such as the PTT Decoding Task,
 *   must be suspended before the ADC Sampler switches to ADC_BUSY. To
 *   initiate an ADC window read operation the on-board computer must put the
 *   firmware into Configuration Mode and then send the ADC_LOAD command.
 *   The task will be ADC_BUSY state for roughly 16ms and then switch
 *   back to ADC_IDLE. The on-board computer must issue the
 *   ADC_STATE command to know when the ADC reading is finished.
 *
 */
void ADCSampler_Task		( void *pvParameters );

/**
 *
 *  Idle_Task
 *
 * 	\brief Dummy task created for checking if RTOS is alive. The task simple blink SF2 board LEDs
 */
void Idle_Task				(void *pvParameters);

/**
 * @}
 */

/**
 * Hardware Configuration
 * \defgroup HW_Setup HW_Setup
 * \ingroup SBCDA_APP
 * @{
 */

/**
 * HW_Setup
 *
 * Initializes the SoC's peripherals.
 * Must be called before the SBCDA_APP starts.
 *
 */
void HW_Setup				( void );

/**
 * AD987_Setup
 * Sends configuration parameters to AD9874
 * through SPI1 iface.
 * Configuration sequence is:
 */
void AD987_Setup			( void );

/**
 * MDDR_Lpddr_Setup
 */
void MDDR_Lpddr_Setup		( void );

/**
 * @}
 */


/**
 * sbcda_decoder_app
 */
void sbcda_decoder_app	 	( void );


#ifndef DOXYGEN_SHOULD_SKIP_THIS
/*
 * \defgroup Test_Applications Test_Applications
 * \ingroup SBCDA_APP
 * @{
 */
void gpio_sample_app 		( void );
void i2c_sample_app  		( void );
void blinky_rtos_app 		( void );
void detect_test_app		( void );
void adc_test_app			( void );
void forward_agc_app		( void );
/**
 * @}
 */
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/*
 * Some useful functions and macros
 */
#define DEBUG_PRINT
#ifdef DEBUG_PRINT
#define print(val) MSS_UART_polled_tx_string(&g_mss_uart0, val)
#define printc(val) MSS_UART_polled_tx(&g_mss_uart0, val, 1);
#else
#define print(val)
#define printc(val)
#endif
void print_hex(uint8_t * buffer, uint8_t length);
unsigned char calcCRC(unsigned char message[], unsigned char length);
void mymemcpy(uint8_t * dest, const uint8_t * org, const int length);
void wait();

#define CONVERT_TO_32BIT_SIGNED(val, nbits)		(int) (val | (0xFFFFFFFF<<(nbits-1)))
#define TEST_SIGN_BIT(val, nbits)				((val & (0x00000001<<(nbits-1)))!=0)

#endif


/**
 * @}
 */

#endif /* INCLUDE_APP_H_ */
