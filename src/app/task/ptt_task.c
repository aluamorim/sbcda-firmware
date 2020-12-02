/*
 * pcdreader_task.c
 *
 *  Created on: 13/02/2017
 *      Author: Anderson
 */

/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "../drivers/mss_uart/mss_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "app.h"

#include "service.h"
#include "dds.h"

//#define TIME_REPORTS 		1
#define MAX_TIME_REPORTS 	32

/**
 * Interrupt handler
 * DDS_EOP is internally connected to GPIO_0 pin of the MSS.
 * This ISR clears a pending interrupt and sets
 * the "new_window" variable to 1;
 */
void GPIO0_IRQHandler(void) {
	DDS_setNewWindowFlag();
	MSS_GPIO_clear_irq( DDS_DETECT_EOP_GPIO);

}

/**
 * PTTDecoder_Task
 */
void PTTDecoder_Task(void *pvParameters) {

	PTTPackage_Typedef * current_pckg;

#ifdef TIME_REPORTS
	uint32_t i0, tmp0, tmp1, new_packages = 0, window_cnt = 0, new_freqs = 0;
	uint8_t bf0[250];
	double e_window = 0.0, e_detection = 0.0, e_decoding = 0.0, e_wait = 0.0;
	double r_window = 0.0, r_detection = 0.0, r_decoding = 0.0, r_wait = 0.0;
	double e_total = 0.0;
	TimeReport_Typedef timeReport;
	timeReport.start_time = 0.0;
	timeReport.detect_time = 0.0;
	timeReport.decode_time = 0.0;
	timeReport.window_time = 0.0;
	timeReport.wait_time = 0.0;

	MSS_RTC_init(MSS_RTC_BINARY_MODE, 2); // rtc with 0 prescaler
	MSS_RTC_start();
#endif
	PTT_Service_Setup();
	DDS_Setup();
	// The main loop is supposed to iterate once for each 10ms window.
	DDS_EnableDecoder();


#ifdef TIME_REPORTS
	e_total = MSS_RTC_get_binary_count();
#endif
	while (1) {
		/////////////////////////////////////////////////////////
		if (!PTT_Service_isPaused()) {
#ifdef TIME_REPORTS
			timeReport.start_time = MSS_RTC_get_binary_count();
#endif
			// block until a new window is ready to be processed
			while (!DDS_isNewWindowReady());
			DDS_clearNewWindowFlag();
			/////////////////////////////////////////////////////////
			// the decoding process cannot be preempted
			portENTER_CRITICAL();

#ifdef TIME_REPORTS
			timeReport.detect_time = MSS_RTC_get_binary_count();
			new_freqs =DDS_Detection_Loop();
#else
			DDS_Detection_Loop();
#endif
#ifdef TIME_REPORTS
			timeReport.decode_time = MSS_RTC_get_binary_count();
#endif
			if (DDS_GetActiveMask() != 0) { // there are still decoders running?
				DDS_DecoderStartOp();
			}
			DDS_Decoding_Loop();
			portEXIT_CRITICAL();
			/////////////////////////////////////////////////////////
#ifdef TIME_REPORTS
			timeReport.window_time = MSS_RTC_get_binary_count();
			e_window += (timeReport.window_time - timeReport.start_time);
			e_detection += (timeReport.decode_time - timeReport.detect_time);
			e_decoding += (timeReport.window_time - timeReport.decode_time);
			e_wait += (timeReport.detect_time - timeReport.start_time);

			if (new_freqs != 0 || window_cnt > 0) {
				window_cnt++;
			}
#endif
		}
		/////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////
		// check if a package was read by the on-board computer,
		// update fifos
//		portENTER_CRITICAL();
		if (PTT_GetPackageReadMutex()) {
			current_pckg = (PTTPackage_Typedef *) PTT_GetReadyFifoOutput();
			if (current_pckg != NULL && current_pckg->pckgType != NO_PCKG_ID) {
				PTT_ResetPackage(current_pckg);
				PTT_PushFreePackage(current_pckg);
			}
			PTT_SetReadyFifoOutput((uint8_t *) PTT_PopReadyPackage());
			if (Cmd_GetCurrentOperationMode() == CMD_PTT_MODE) {
				Comm_Set_Tx_Buffer(PTT_GetReadyFifoOutput(),
						PTT_TOTAL_RECORD_SIZE);
			}
			PTT_ClearPackageReadMutex();
		} else {
			current_pckg = (PTTPackage_Typedef *) PTT_GetReadyFifoOutput();
			if (current_pckg == NULL || current_pckg->pckgType == NO_PCKG_ID) {
				PTT_SetReadyFifoOutput((uint8_t *) PTT_GetReadyFifoTail());
				if (Cmd_GetCurrentOperationMode() == CMD_PTT_MODE) {
					Comm_Set_Tx_Buffer(PTT_GetReadyFifoOutput(),
							PTT_TOTAL_RECORD_SIZE);
				}
			}
		}
//		portEXIT_CRITICAL();
		/////////////////////////////////////////////////////////
#ifdef TIME_REPORTS
		if ((PTT_ReadyPackages_Available()) == 1) {
		//if (window_cnt >= 100) {
			window_cnt = 0;
			portENTER_CRITICAL();

			e_total = (MSS_RTC_get_binary_count() - e_total) * 1.0;

			r_window = ((e_window * 1.0) / (e_total)) * 100.0;
			r_detection = ((e_detection * 1.0) / (e_total)) * 100.0;
			r_decoding = ((e_decoding * 1.0) / (e_total)) * 100.0;
			r_wait = ((e_wait * 1.0) / (e_total)) * 100.0;

			/* min prescaler is 2. Therefore the RTC counts
			 * once at every 2us. Devide by 500 to get the
			 * measure in ms
			 */
			e_window /= 500.0;
			e_detection /= 500.0;
			e_decoding /= 500.0;
			e_wait /= 500.0;
			e_total /= 500.0;

			print(
					"\n\n\r******************************************************************* ");

			sprintf(bf0,
					"\n\r>    Total Time:          %.03f \t ms \t (%.02f %%) ",
					e_total, 100.0);
			print(bf0);
			sprintf(bf0,
					"\n\r>    Window Time:         %.03f \t ms \t (%.02f %%) ",
					e_window, r_window);
			print(bf0);
			sprintf(bf0,
					"\n\r>    Detection:           %.03f \t ms \t (%.02f %%)",
					e_detection, r_detection);
			print(bf0);
			sprintf(bf0,
					"\n\r>    Decoding:            %.03f \t ms \t (%.02f %%)",
					e_decoding, r_decoding);
			print(bf0);
			sprintf(bf0,
					"\n\r>    Idle Time:           %.03f \t ms \t (%.02f %%)",
					e_wait, r_wait);
			print(bf0);

			print(
					"\n\r******************************************************************* ");
			print("\n\n\r");


			print ("\n\r>>>>>>>>>>>>>>>>>>>>>> SYMBOLS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ");
			uint16_t * vec = DDS_GetSymbVec();
			print ("\n\r symb_vec = [");
			int16_t stmp0=0;
			for (i0=0; i0<620; i0++){
				stmp0 = vec[i0] & 0x0200 ? (0xFFFFFC00 | vec[i0]) : (0x000003FF & vec[i0]);
				sprintf(bf0,"%d, ", stmp0);
				print(bf0);
				if (((i0+1)%21)==0){
					print (" ... \n\r");
				}
			}
			print ("];");
			print ("\n\r>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ");

			portEXIT_CRITICAL();
			while (1)
			;
		}
#endif
	}

}
