/*
 * adc_service.c
 *
 *  Created on: May 16, 2017
 *      Author: alu
 */
#include <stdint.h>
#include "service.h"
#include "app.h"
#include "adc.h"


uint8_t adcSamplerBuffer[ADC_BUFFER_SIZE_IN_BYTES];


uint8_t adc_state;


/*
 * ADC_Service_Setup
 *
 * This function initializes the internal buffer pointers
 *
 */
void ADC_Service_Setup(void) {
	uint16_t i0 = 0;

	adc_state = ADC_IDLE;
	for (i0=0; i0 < ADC_BUFFER_SIZE; i0++) {
		adcSamplerBuffer[i0] = 0; // no ready
	}
	adcSamplerBuffer[0] = ADC_PCKG_ID;
}

uint8_t * ADC_GetSamplerBuffer( void ){
	return (uint8_t * ) adcSamplerBuffer;
}

uint8_t ADC_GetState( void ){
	return adc_state;
}

void  ADC_SetState( uint8_t st ){
	if (st <=2)
		adc_state = st;
}


