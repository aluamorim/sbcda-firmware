/*
 * adc_sampler_test.c
 *
 *  Created on: May 17, 2017
 *      Author: alu
 */

#include <stdint.h>
#include <stdio.h>
#include "service.h"
#include "app.h"
#include "adc.h"
#include "../drivers/mss_gpio/mss_gpio.h"
#include "../drivers/mss_uart/mss_uart.h"
#include "../drivers/mss_watchdog/mss_watchdog.h"
#include "app.h"

#include "service.h"
#include "dds.h"

#define MAX_SYMBOLS 608
#define MAX_AGC_SAMPLES 8*MAX_SYMBOLS

void forward_agc_app() {

	uint32_t symbol_vec[MAX_SYMBOLS], symbol_cnt = 0, tmp0 = 0, i0 = 0;
	uint32_t agc_samples[MAX_AGC_SAMPLES], agc_sample_cnt = 0;
	uint32_t pttd_ctrl = 0, pttd_data0 = 0, pttd_data1 = 0;
	uint16_t pttd_symbol = 0;
	uint8_t pttd_ch = 0, cnt0 = 4, bf0[200];

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

	PTTPackage_Typedef * current_pckg;

	PTT_Service_Setup();
	DDS_Setup();
	// The main loop is supposed to iterate once for each 10ms window.
	DDS_EnableDecoder();

	print("\n\r > AGC SAMPLER TEST: \n\r");
	while (1) {
		/////////////////////////////////////////////////////////

		// block until a new window is ready to be processed
		while (!DDS_isNewWindowReady())
			;
		DDS_clearNewWindowFlag();
		/////////////////////////////////////////////////////////

		DDS_Detection_Loop();

		if (DDS_GetActiveMask() != 0) { // there are still decoders running?
			DDS_DecoderStartOp();
		}

		while (!DDS_DecoderEop() || !DDS_DecoderEmpty()) {
			// - Now all the active decoder will spit
			// symbols through the PTT_BUFFER_FIFO
			// - All the symbols before the symb_lock are discarded for
			// that channel.
			// - After the symb_lock signal rises, the symbols
			// must be converted to bits and compared to the frame_synch word.
			// - Finally,after the frame synchronization, the package building starts
			while (!DDS_DecoderEmpty()) {
				pttd_data0 = SBCDA_DECODER->pttdBufferData0;
				pttd_data1 = SBCDA_DECODER->pttdBufferData1; /** fifo pop on this load **/
				pttd_ch = DDS_PTTD1_CH(pttd_data1);
				pttd_symbol = DDS_PTTD1_SYMB_VALUE(pttd_data1);

				if (symbol_cnt < MAX_SYMBOLS) {
					tmp0 = pttd_ch & 0x00FF;
					tmp0 = (tmp0 << 16) | pttd_symbol;
					symbol_vec[symbol_cnt++] = tmp0;
				} else {
					DDS_ClearDecoder(pttd_ch);
				}
			}
		}

		while (!(SBCDA_DECODER->agcSamplerCtrl & 0x01)) {
			if (agc_sample_cnt < MAX_AGC_SAMPLES) {
				agc_samples[agc_sample_cnt++] = SBCDA_DECODER->agcSamplerData;
			} else {
				break;
			}
		}

		if (agc_sample_cnt == MAX_AGC_SAMPLES || symbol_cnt == MAX_SYMBOLS) {
			int16_t stmp0 = 0;
			print("\n\r agc_re = [");
			for (i0 = 0; i0 < MAX_AGC_SAMPLES; i0++) {
				tmp0 = (agc_samples[i0] >> 10);
				stmp0 = tmp0 & 0x0200 ? 0xFFFFFC00 | tmp0 : (0x000003FF & tmp0);

				sprintf(bf0, "%d, ", stmp0);
				print(bf0);
				if ((i0 + 1) % 16 == 0) {
					print("... \n\r");
				}
			}
			print("\n\r ]; \n\r");
			print("\n\r agc_im = [");
			for (i0 = 0; i0 < MAX_AGC_SAMPLES; i0++) {
				tmp0 = (agc_samples[i0]) & 0x03FF;
				stmp0 = tmp0 & 0x0200 ? 0xFFFFFC00 | tmp0 : (0x000003FF & tmp0);
				sprintf(bf0, "%d, ", stmp0);
				print(bf0);
				if ((i0 + 1) % 16 == 0) {
					print("... \n\r");
				}
			}
			print("\n\r ]; \n\r");

			print("\n\r agc_ch = [");
			for (i0 = 0; i0 < MAX_AGC_SAMPLES; i0++) {
				sprintf(bf0, "%d, ", (agc_samples[i0] >> 20) & 0x0F);
				print(bf0);
				if ((i0 + 1) % 16 == 0) {
					print("... \n\r");
				}
			}
			print("\n\r ]; \n\r");

			print("\n\r symb_ch = [");
			for (i0 = 0; i0 < MAX_SYMBOLS; i0++) {
				sprintf(bf0, "%d, ", (symbol_vec[i0] >> 16) & 0x00FF);
				print(bf0);
				if ((i0 + 1) % 16 == 0) {
					print("... \n\r");
				}
			}
			print("\n\r ]; \n\r");

			print("\n\r symb_vec = [");
			for (i0 = 0; i0 < MAX_SYMBOLS; i0++) {
				tmp0 = (symbol_vec[i0]) & 0x00FFFF;
				stmp0 = tmp0 & 0x0200 ? 0xFFFFFC00 | tmp0 : (0x000003FF & tmp0);

				sprintf(bf0, "%d, ", stmp0 );
				print(bf0);
				if ((i0 + 1) % 16 == 0) {
					print("... \n\r");
				}
			}
			print("\n\r ]; \n\r");

			while (1) {
				MSS_WD_reload();
				cnt0 = ~cnt0;
				MSS_GPIO_set_outputs((cnt0 & (DS3_LED_MASK | DS4_LED_MASK)));
				tmp0 = 32000;
				while (tmp0--)
					;
			}
		}
		/////////////////////////////////////////////////////////
	}
}

