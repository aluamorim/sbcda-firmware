/*
 * housekeeping_task.c
 *
 *  Created on: 13/02/2017
 *      Author: Anderson
 */
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "app.h"
#include "service.h"

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
 * 0 - RTC time tag
 * 1 - Current sensors
 * 2 - ADC RMS
 * 3 - PLL Synch flag
 * 4 - Over - current
 * 5-15 - Reserved for future use
 *
 */
void HouseKeeping_Task(void *pvParameters) {

	uint8_t refresh_time = 1;
	uint16_t config_word = 0, cnt = 0;
	mss_rtc_calendar_t calendar;
	HK_Service_Setup();
	HKPackage_Typedef * hk_pckt = (HKPackage_Typedef *) HK_Get_Buffer();;
	refresh_time = HK_Get_Refresh_Time() * 1000;

	while(1) {
		config_word = HK_Get_Config();
		if (config_word & HK_CFG_RTC) {
			if (RTC_Is_Initialized()) {
				RTC_Get(&calendar);
				hk_pckt->timeTag[7] = calendar.week;
				hk_pckt->timeTag[6] = calendar.weekday;
				hk_pckt->timeTag[5] = calendar.second;
				hk_pckt->timeTag[4] = calendar.minute;
				hk_pckt->timeTag[3] = calendar.hour;
				hk_pckt->timeTag[2] = calendar.day;
				hk_pckt->timeTag[1] = calendar.month;
				hk_pckt->timeTag[0] = calendar.year;
			}
		}

		if (config_word & HK_CFG_ADC_RMS_SAMPLE) {
			hk_pckt->adc_rms = SBCDA_DECODER->adcRms; //cnt++;
		}

		if (config_word & HK_CFG_CURRENT_SENSORS) {
			// TODO read current sensors values
			hk_pckt->current_sensor1 = 0x0304;
			hk_pckt->current_sensor2 = 0x0506;
		}

		if (config_word & HK_CFG_PLL_SYNC_BIT) {
			// TODO read pll sync bit
			hk_pckt->pll_sync = 0x07;
		}
		if (config_word & HK_CFG_OVER_CURRENT) {
			// TODO read over current cnt
			hk_pckt->over_current_fail = 0x08;
		}
		hk_pckt->pckgType = HK_PCKG_ID; // command status

	}
}

