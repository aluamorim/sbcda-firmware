/**
 * @file rtc.h
 *
 * @author Anderson
 *
 * \defgroup RTC_Service RTC_Service
 * \ingroup SBCDA_APP
 *
 * @{
 */

#ifndef INCLUDE_RTC_H_
#define INCLUDE_RTC_H_

/**
 *
 * RTC Command Interface
 */

/**
 *
 *  RTC_Init
 *
 *	\brief Initializes RTC system to calendar mode, setting prescale do 1Hz
 *
 */
void RTC_Setup();

/**
 *
 *  RTC_Set
 *
 *	\brief Sets Load Value to RTC
 *
 *	@param calendar mss_rtc_calendar_t structure containing Y M D H M S W Wd bytes.
 */
void RTC_Set(mss_rtc_calendar_t * calendar);

/**
 *
 *  RTC_Reset
 *
 *  \brief Resets the RTC to its initial value.
 *
 */
void RTC_Reset ();

/**
 *
 *  RTC_Stop
 *
 *  \brief Stops RTC
 */
void RTC_Stop ();
/**
 *
 *  RTC_Start
 *
 * \brief Starts RTC
 */
void RTC_Start ();


/**
 *
 *  RTC_Is_Initialized
 *
 *  \brief Returns a boolean value indicating if the RTC was initialized or not.
 *
 *  @return 0 RTC Not initialized
 *  @return 1 RTC Initialized
 *
 */
uint8_t RTC_Is_Initialized ();

/**
 *
 *  RTC_Get
 *
 *  \brief Gets the current value of the RTC into the mss_rtc_calendar_t structure reference
 *
 *  @param calendar mss_rtc_calendar_t structure reference where the current RTC value will be stored.
 */
void RTC_Get(mss_rtc_calendar_t * calendar);

/**
 * @}
 */
#endif /* INCLUDE_RTC_H_ */
