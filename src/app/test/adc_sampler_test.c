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

void adc_sampler_test0(void) {

	// auxiliary variables
	uint32_t tmp0 = 0, tmp1 = 0, tmp2 = 0, i = 0;
	char bf0[100], bf1[20];
	MSS_GPIO_init();
	/*
	 * Configure MSS GPIOs.
	 */
	MSS_GPIO_config(DS3_LED_GPIO, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(DS4_LED_GPIO, MSS_GPIO_OUTPUT_MODE);

	MSS_GPIO_set_outputs(0);

	print("\n\r> ADC SAMPLER TEST ");
	uint8_t val = '0';
	while (1) {

		MSS_GPIO_set_outputs(DS3_LED_MASK); // led1 to signalize write phase
		print("\n\r> Trying to read rms values before enabling the ADC... ");
		print("\n\r> Values: ");

		val = '0';
		for (i = 0; i < 16; i++) {
			val = SBCDA_DECODER->adcRms; // offset 0x00
			printc(&val);
		}
		print("\n\r");
		print("\n\r> Enabling the ADC fifo ");
		SBCDA_DECODER->adcSamplerCtrl = DDS_ADC_SAMPCTRL_SAMP_ENABLE; /// enable adc sampler
		wait();
		print("\n\r> Values: ");
		for (i = 0; i < 16; i++) {
			val = SBCDA_DECODER->adcRms; // offset 0x00
			printc(&val);
		}
		print("\n\r");

		print("\n\r> Reading ADC_SAMPLER_FIFO... ");

		tmp0 = SBCDA_DECODER->adcSamplerCtrl;
		print("\n\r> Wait until FIFO isn't empty ");
		while (tmp0 & DDS_ADC_SAMPCTRL_SAMP_EMPTY) {
			print(".");
		}

		for (i = 0; i < 32; i++) {
			tmp2 = SBCDA_DECODER->adcSamplerCtrl;
			tmp1 = SBCDA_DECODER->adcSamplerData; // fifo read
			tmp0 = SBCDA_DECODER->adcSamplerCtrl;

			sprintf(bf0, "\n\n\r> Before:  Full: %02x HFull: %02x Empty: %02x ",
					(tmp2 & DDS_ADC_SAMPCTRL_SAMP_FULL) > 0,
					(tmp2 & DDS_ADC_SAMPCTRL_SAMP_HFULL) > 0,
					(tmp2 & DDS_ADC_SAMPCTRL_SAMP_EMPTY) > 0);
			print(bf0);

			sprintf(bf0, "\n\r> Value Read: 0x%08x ", tmp1);
			print(bf0);

			sprintf(bf0, "\n\r> After:   Full: %02x HFull: %02x Empty: %02x ",
					(tmp0 & DDS_ADC_SAMPCTRL_SAMP_FULL) > 0,
					(tmp0 & DDS_ADC_SAMPCTRL_SAMP_HFULL) > 0,
					(tmp0 & DDS_ADC_SAMPCTRL_SAMP_EMPTY) > 0);
			print(bf0);
		}

		print("\n\r> Wait until the fifo is full ");
		tmp0 = SBCDA_DECODER->adcSamplerCtrl;
		while (!(tmp0 & DDS_ADC_SAMPCTRL_SAMP_FULL)) {
			print(".");
		}

		print("\n\r> Clear and disable ADC_FIFO");
		SBCDA_DECODER->adcSamplerCtrl = DDS_ADC_SAMPCTRL_SAMP_SAMPCLR;
		wait();

		tmp0 = SBCDA_DECODER->adcSamplerCtrl;
		print("\n\r> FIFO Flags: ");
		sprintf(bf0, "\n\r> Full: %02x HFull: %02x Empty: %02x ",
				(tmp0 & DDS_ADC_SAMPCTRL_SAMP_FULL) > 0,
				(tmp0 & DDS_ADC_SAMPCTRL_SAMP_HFULL) > 0,
				(tmp0 & DDS_ADC_SAMPCTRL_SAMP_EMPTY) > 0);
		print(bf0);

		print("\n\r> Trying to read the ADC_RMS again... ");
		print("\n\r> Values: ");
		val = '0';
		for (i = 0; i < 16; i++) {
			val = SBCDA_DECODER->adcRms; // offset 0x00
			printc(&val);
		}

		print(
				"\n\n\r> ADC_SAMPLER REGs Test Finished.\n\r> Reset The board to run again.\n\r");
		MSS_GPIO_set_outputs(DS4_LED_MASK); // led2 to signalize read phase
		while (1)
			;
	}
}

void adc_sampler_test1(void) {

	uint64_t start_t, end_t, elapsed_t;
	uint32_t * buffer_ptr;
	uint8_t bf[250];
	uint16_t st, i0 = 0, i1 = 0;
	ADC_Service_Setup();
	st = ADC_GetState();
	buffer_ptr = ADC_GetSamplerBuffer();

	print("\n\r> ADC SAMPLER TEST: ");
	MSS_RTC_init(MSS_RTC_BINARY_MODE, 0); // rtc with 0 prescaler
	MSS_RTC_start();
	while (1) {

		start_t = MSS_RTC_get_binary_count();
		buffer_ptr[0] = ADC_PCKG_ID; // pckg id
		SBCDA_DECODER->adcSamplerCtrl = DDS_ADC_SAMPCTRL_SAMP_ENABLE;
		while (i0 < ADC_BUFFER_SIZE) {
			if (SBCDA_DECODER->adcSamplerCtrl & DDS_ADC_SAMPCTRL_SAMP_FULL) {
				print(" # ");
			}
			if (!(SBCDA_DECODER->adcSamplerCtrl & DDS_ADC_SAMPCTRL_SAMP_EMPTY)) {
				buffer_ptr[i0++] = SBCDA_DECODER->adcSamplerData;
			}

		}
		SBCDA_DECODER->adcSamplerCtrl = DDS_ADC_SAMPCTRL_SAMP_SAMPCLR;
		ADC_SetState(ADC_READY);
		end_t = MSS_RTC_get_binary_count();
		elapsed_t = end_t - start_t;

		i1++;
		sprintf(bf, "\n\r> Test Report Number %d  ", i1);
		print(bf);
		sprintf(bf, "\n\r> Number of samples read: %d ", i0);
		print(bf);
		sprintf(bf, "\n\r> Duration: %d us ", elapsed_t);
		print(bf);
		print("\n\r Samples: \n\r ");
		for (i0 = 0; i0 < 2048; i0++) {
			sprintf(bf, " %04x_%04x ", buffer_ptr[i0] >> 16,
					buffer_ptr[i0] & 0x0000FFFF);
			print(bf);

			if ((i0 + 1) % 10 == 0) {
				print("\n\r ");
			}
		}

		print("\n\r>  Test Finished! \n\r> ");
		while (1)
			;
	}
}

void adc_test_app(void) {
//	adc_sampler_test0();
	adc_sampler_test1();
}
