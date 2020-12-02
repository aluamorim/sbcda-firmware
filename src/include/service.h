/*
 * commands.h
 *
 *  Created on: 13/02/2017
 *      Author: Anderson
 */

#ifndef INCLUDE_SERVICE_H_
#define INCLUDE_SERVICE_H_


#include "../drivers/mss_rtc/mss_rtc.h"

#include "comm.h"
#include "ptt.h"
#include "rtc.h"
#include "housekeeping.h"
#include "dds.h"
#include "adc.h"
#include "command.h"

#define ACK_PCKG_ID					0x10
#define PTT_PCKG_ID					0x1A
#define ADC_PCKG_ID					0x1B
#define HK_PCKG_ID					0x1C
#define NO_PCKG_ID					0xFE


#define DEFAULT_TICK_DELAY 			1000 // about 1 second

#endif /* INCLUDE_SERVICE_H_ */
