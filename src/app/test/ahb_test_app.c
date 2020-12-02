/*
 * ahb_test_app.c
 *
 *  Created on: 06/04/2017
 *      Author: Anderson
 */

/* Standard includes. */
#include <stdio.h>
#include <stdint.h>

#include "app.h"
#include "../drivers/mss_gpio/mss_gpio.h"

#define NUMBER_OF_DETECT_WINDOWS 100
#define MAX_NUMBER_OF_SAMPLES	 12



void detect_test_app(void) {

	TimeReport_Typedef timeReports[NUMBER_OF_DETECT_WINDOWS];
	uint32_t tmp0 = 0, tmp1 = 0, new_freqs = 0, i = 0, j = 0;
	uint32_t samples[NUMBER_OF_DETECT_WINDOWS][MAX_NUMBER_OF_SAMPLES];
	uint32_t window_cnt = 0, sample_cnt = 0, test_fail = 0,
			freqs_per_window[NUMBER_OF_DETECT_WINDOWS],
			repl_freqs_per_window[NUMBER_OF_DETECT_WINDOWS],
			disc_freqs_per_window[NUMBER_OF_DETECT_WINDOWS];
	uint8_t bf0[200];

	MSS_GPIO_init();
	MSS_GPIO_config(DS3_LED_GPIO, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(DS4_LED_GPIO, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_set_outputs(0);

	MSS_RTC_init(MSS_RTC_BINARY_MODE, 0); // rtc with 0 prescaler
	MSS_RTC_start();

	print("\n\n\r> DETECT FIFO TEST ");
	uint8_t val = '0';
	PTT_Service_Setup();
	while (1) {
		print("\n\r Running detection loops... ");

		DDS_Setup();
		uint64_t e_window = 0, e_detection = 0, e_decoding = 0, e_wait = 0;
		i = 0;
		while (PTT_ReadyPackages_Available() <= 10) {

			timeReports[i].start_time = MSS_RTC_get_binary_count();
			new_freqs = DDS_Detection_Loop();
			timeReports[i].detect_time = MSS_RTC_get_binary_count();
			DDS_DecoderStartOp();
			DDS_Decoding_Loop();
			timeReports[i].decode_time = MSS_RTC_get_binary_count();

			while (DDS_DetectEop()) {
				freqs_per_window[i] = new_freqs;
				disc_freqs_per_window[i] = DDS_GetDiscardDetections_Count();
				repl_freqs_per_window[i] = DDS_GetReplacedFrequencies_Count();
				for (j = 0; j < MAX_NUMBER_OF_SAMPLES; j++) {
					samples[i][j] = SBCDA_DECODER->initFreq[j];
				}
			}
			timeReports[i].window_time = MSS_RTC_get_binary_count();

			e_window += timeReports[i].window_time - timeReports[i].start_time;
			e_detection += timeReports[i].detect_time
					- timeReports[i].start_time;
			e_decoding += timeReports[i].decode_time
					- timeReports[i].detect_time;
			e_wait +=  timeReports[i].window_time - timeReports[i].decode_time;
		}

		print("\n\r> Report: ");

		int freqv = 0, ampv = 0;
//		for (i = 0; i < NUMBER_OF_DETECT_WINDOWS; i++) {

		print(
				"\n\n\r******************************************************** ");
		sprintf(bf0, "\n\r> W%d:  ", i);
		print(bf0);

		sprintf(bf0, "\n\r>    Window Time:         %d us  ", e_window);
		print(bf0);
		sprintf(bf0, "\n\r>    Detection:           %d us ", e_detection);
		print(bf0);
		sprintf(bf0, "\n\r>    Decoding:            %d us ", e_decoding);
		print(bf0);
		sprintf(bf0, "\n\r>    Idle Time:           %d us ", e_wait);
		print(bf0);

		sprintf(bf0, "\n\r>    New Freqs:           %d ", freqs_per_window[i]);
		print(bf0);
		sprintf(bf0, "\n\r>    Repl. Freqs:         %d ",
				repl_freqs_per_window[i]);
		print(bf0);
		sprintf(bf0, "\n\r>    Disc. Freqs:         %d ",
				disc_freqs_per_window[i]);
		print(bf0);

		print("\n\r ");
		for (j = 0; j < MAX_NUMBER_OF_SAMPLES; j++) {
			tmp0 = samples[i][j];
			freqv = tmp0 & 0x0000FFFF;
			if (TEST_SIGN_BIT(freqv, DDS_FREQ_NUMBER_OF_BITS)) {
				//negative
				freqv = CONVERT_TO_32BIT_SIGNED(freqv, DDS_FREQ_NUMBER_OF_BITS);
			}
			freqv *= 62; // fft resolution (128000/2048)
			//			freqv /= 1000;
			sprintf(bf0, "[%04x FREQ: %08d Hz] ", (tmp0 & 0x0000FFFF), freqv);
			print(bf0);
			if ((j + 1) % 3 == 0) {
				print("\n\r ");
			}
		}

//		}
		print("\n\r******************************************************** ");
		print("\n\n\r> End ");

		MSS_GPIO_set_outputs(DS3_LED_MASK | DS4_LED_MASK); // led1 to signalize write phase
		while (1)
			;
	}
}

void ahb_test_app(void) {

	detect_test_app();
}
