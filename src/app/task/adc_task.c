/*
 * adc_task.c
 *
 *  Created on: May 16, 2017
 *      Author: anderson
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "app.h"
#include "service.h"
#include "adc.h"

/**
 *
 * ADCSampler_Task
 *
 * It is responsible for collect ADC Sampler windows (2048 adc samples)
 * and  storing them into the ADC Sampler Buffer. The ADC Sampler
 * is high priority task, it  can not be preempted while collecting
 * a ADC window to avoid losing samples.
 * It runs on a infinite loop alternating between two states:
 *
 * - ADC_IDLE: the tasks suspends for 0,5s, allowing other tasks to run;
 *
 * - ADC_BUSY: the tasks collects 2048 samples from the ADC Sampler FIFO
 *             in the Decoder Unit and stores them into a ADC Sampler Package.
 *             After the reading is done the tasks automatically switches to
 *             ADC_IDLE state.
 *
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
void ADCSampler_Task(void *pvParameters) {

	uint32_t * buffer_ptr;
	uint32_t buffer_size = 0;
	uint16_t i0;
	uint8_t st;

	buffer_size = (ADC_BUFFER_SIZE);
	buffer_ptr = (uint32_t *) ADC_GetSamplerBuffer();
	while (1) {
		st = ADC_GetState();
		if (st == ADC_IDLE || st == ADC_READY) {
			// better make sure it is disabled
			SBCDA_DECODER->adcSamplerCtrl = DDS_ADC_SAMPCTRL_SAMP_SAMPCLR;
			vTaskDelay(1000);
		} else if (st == ADC_BUSY) {
			portENTER_CRITICAL();
			i0 = 1;
			buffer_ptr[0] = ADC_PCKG_ID; // pckg id
			SBCDA_DECODER->adcSamplerCtrl = DDS_ADC_SAMPCTRL_SAMP_ENABLE
					| DDS_ADC_SAMPCTRL_SAMP_SAMPCLR;
			while (i0 < buffer_size) {
				if (!(SBCDA_DECODER->adcSamplerCtrl
						& DDS_ADC_SAMPCTRL_SAMP_EMPTY)) {
					buffer_ptr[i0++] = SBCDA_DECODER->adcSamplerData;
				}
			}

//			uint8_t bf[30];
//			print("\n\r adc_sampler_q = [ ");
//			for (i0 = 1; i0 < buffer_size; i0++) {
//				int32_t st = (buffer_ptr[i0] & 0x0000FFFF);
//				st = st & 0x08000 ? 0xFFFF0000 | st : st;
//				sprintf(bf, "%d, ", st);
//				print(bf);
//				if ((i0 + 1) % 15 == 0) {
//					print("... \n\r");
//				}
//			}
//			print("]; \n\r");
//
//			print("\n\r adc_sampler_i = [ ");
//			for (i0 = 1; i0 < buffer_size; i0++) {
//				int32_t st = ((buffer_ptr[i0]>>16) & 0x0000FFFF);
//				st = st & 0x08000 ? 0xFFFF0000 | st : st;
//				sprintf(bf, "%d, ", st);
//				print(bf);
//				if ((i0 + 1) % 15 == 0) {
//					print("... \n\r");
//				}
//			}
//			print("]; \n\r");

			SBCDA_DECODER->adcSamplerCtrl = DDS_ADC_SAMPCTRL_SAMP_SAMPCLR;
			ADC_SetState(ADC_READY);
			portEXIT_CRITICAL();
		}
	}
}

