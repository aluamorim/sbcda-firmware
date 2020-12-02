/**
 * 	@file housekeeping.h
 *  @author: Anderson
 *
 *	\defgroup HK_Service HK_Service
 *	\ingroup SBCDA_APP
 * 	@{
 */

#ifndef INCLUDE_HOUSEKEEPING_H_
#define INCLUDE_HOUSEKEEPING_H_

/*
 *  HK CONFIG WORD
 *  Length- 8 bytes
 *
 *	bytes
 *  [0-1] 	- Config set bits. A set bit indicates which hk measures are active (BIG Endian)
 *  [2] 	- Refresh time in seconds
 *  [3-7]	- reserved for future inclusions
 *
 *  HK PACKAGE FORMAT
 *  Length: 32 bytes
 *	[0] 		- Pckg ID (HK_REQ)
 *	[1] 		- Pckg ST (0-request, 1-response)
 *	[2-9] 		- RTC Values [9-week, 8-wday, 7-sec, 6-min, ... 3-month, 2-year]
 *	[10-33]		- freq list for the 12 decoders
 *	[34-64]		- reserved for future implementation
 *
 */
#define HK_CFG_WORD_SIZE							0x02 // in bytes
#define HK_CFG_RTC									0x0001
#define HK_CFG_CURRENT_SENSORS						0x0002
#define HK_CFG_ADC_RMS_SAMPLE						0x0004
#define HK_CFG_PLL_SYNC_BIT							0x0008
#define HK_CFG_OVER_CURRENT							0x0010

#define HK_TIME_TAG_OCTETS_LENGTH					8

#define HOUSEKEEPING_PACKAGE_SIZE					17



/**
 * HKPackage_typedef
 *
 * \brief Defines the default format for HouseKeeping packages
 */
typedef struct {
	uint8_t pckgType; /**< Identification byte of the HK package. */
	uint16_t adc_rms; /**< ADC RMS latest value. */
	uint16_t current_sensor1 /**< Current sensor A value.  */;
	uint16_t current_sensor2;/**< Current sensor B value  */
	uint8_t pll_sync; /**< PLL Synch state flag  */
	uint8_t over_current_fail; /**< Over current indicator flag  */
	uint8_t timeTag [HK_TIME_TAG_OCTETS_LENGTH]; /**< RTC time tag, indicatig when the package was collected  */
}HKPackage_Typedef;

/**
 *
 *  HK_Service_Setup
 *
 *	\brief Initializes Housekeeping service.
 *
 */
void HK_Service_Setup();
/**
 *
 *  HK_Get_Buffer
 *
 *	\brief Returns a pointer to the HK Package
 *
 *	@return (uint8_t *) A pointer to the latest HK Package
 *
 */
uint8_t * HK_Get_Buffer();


/**
 *
 * HK_Set_Config
 *
 * \brief Sets the current HK config word.
 *
 * The configuration word masks defines which
 * HK measure is enabled or not. It can be updated
 * by the on-board computer using the HK_CONFIG
 * command with the firmware in Configuration Mode.
 * The configuration word is a 2-byte integer whose
 * bits are mapped as follows:
 * <pre>
 * 0 - RTC time tag
 * 1 - Current sensors
 * 2 - ADC RMS
 * 3 - PLL Synch flag
 * 4 - Over - current
 * 5-15 - Reserved for future use
 * </pre>
 *
 * @param word HK configuration word.
 *
 */
void HK_Set_Config(uint16_t word);

/**
 *
 * HK_Get_Config
 *
 * \brief Returns the current HK configuration word.
 *
 * @return Current HK config word
 */
uint16_t HK_Get_Config();

/**
 *
 * HK_Set_Refresh_Time
 *
 * \brief Sets the refresh time for collecting HK information.
 *
 * @param t Refresh time in ms.
 */
void HK_Set_Refresh_Time(uint8_t t);

/**
 *
 * HK_Set_Refresh_Time
 * \brief Returns the current refresh time of the HK Task in ms.
 *
 * @return refresh time in ms.
 */
uint8_t HK_Get_Refresh_Time();

/**
 * @}
 */
#endif /* INCLUDE_HOUSEKEEPING_H_ */
