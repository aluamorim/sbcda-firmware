/**
 * @file adc.h
 *
 * @author Anderson Amorim
 *
 * \defgroup ADCSampler ADCSampler
 * \ingroup SBCDA_APP
 * @{
 */

#ifndef INCLUDE_ADC_H_
#define INCLUDE_ADC_H_


#define ADC_BUFFER_SIZE	2049//(2048*4 + 4) //in samples
#define ADC_BUFFER_SIZE_IN_BYTES	(ADC_BUFFER_SIZE*4)//(2048*4 + 4) //in bytes

/**
 * ADCState_Typedef
 * \brief Determines the current operation state of the ADC Sampler task.
 */
typedef enum{
	ADC_IDLE, /**< ADC Sampler task is in idle state (suspended)*/
	ADC_BUSY, /**< ADC Sampler task is in busy state (running) and a ADC Package is being read.*/
	ADC_READY /**< ADC Sampler task is finished reading a ADC Package window. */
}ADCState_Typedef;

/**
 * ADC_Service_Setup
 *
 * \brief This function initializes the internal ADC window buffer.
 *
 */
void ADC_Service_Setup(void);

uint8_t * ADC_GetSamplerBuffer( void );

/**
 * ADC_GetState
 *
 * Returns the current state of operation for the
 * ADC Sampler Task.
 * @return		0 = ADC_IDLE
 * @return		1 = ADC_BUSY
 * @return		2 - ADC_READY
 *
 */
uint8_t ADC_GetState( void );

/**
 * ADC_SetState
 *
 * Changes the current state of operation for the
 * ADC Sampler Task.
 * @param state <pre>
 * 				0 = ADC_IDLE
 * 				1 = ADC_BUSY
 * 				2 - ADC_READY
 *				</pre>
 */
void  ADC_SetState( uint8_t state );

/**
 * @}
 */

#endif /* INCLUDE_ADC_H_ */
