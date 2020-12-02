/*
 * housekeeping_service.c
 *
 *  Created on: 13/02/2017
 *      Author: Anderson
 */

#include "service.h"

/*
 * Refresh time parameter
 */
uint8_t HK_REFRESH_TIME;

/*
 * Data buffer to store the Housekeeping Package
 */
uint8_t hk_buffer[HOUSEKEEPING_PACKAGE_SIZE];

/*
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
uint16_t HK_CFG_WORD;

/*
 * --------------------------------------------------------------------------
 *  HK_Service_Setup
 * --------------------------------------------------------------------------
 *	Initializes Housekeeping services
 */
void HK_Service_Setup(){

	int i0;
	hk_buffer[0] = HK_PCKG_ID;
	for (i0=1; i0<HOUSEKEEPING_PACKAGE_SIZE; i0++){
		hk_buffer[i0] = 0;
	}
	HK_CFG_WORD = 0x0F; // all measures active by default
	HK_REFRESH_TIME = 5;
}

/*
 * --------------------------------------------------------------------------
 * HK_Set_Config
 * --------------------------------------------------------------------------
 */
void HK_Set_Config(uint16_t c){
	HK_CFG_WORD = c;
}

/*
 * --------------------------------------------------------------------------
 * HK_Get_Config
 * --------------------------------------------------------------------------
 */
uint16_t HK_Get_Config(){
	return HK_CFG_WORD;
}

/*
 * --------------------------------------------------------------------------
 * HK_Set_Refresh_Time
 * --------------------------------------------------------------------------
 */
void HK_Set_Refresh_Time(uint8_t t){
	HK_REFRESH_TIME = t;
}

/*
 * --------------------------------------------------------------------------
 * HK_Set_Refresh_Time
 * --------------------------------------------------------------------------
 */
uint8_t HK_Get_Refresh_Time(){
	return HK_REFRESH_TIME;
}
/*
 * --------------------------------------------------------------------------
 *  HK_Get_Buffer
 * --------------------------------------------------------------------------
 *	Returns a pointer to the HK data package
 *
 */
uint8_t * HK_Get_Buffer(){
	return hk_buffer;
}
