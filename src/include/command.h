/*
 * command.h
 *
 *  Created on: 16/02/2017
 *      Author: Anderson
 */

#ifndef INCLUDE_COMMAND_H_
#define INCLUDE_COMMAND_H_


#define CMD_MAX_SIZE_IN_BYTES 									12
#define CMD_MAX_PARAMETER_SIZE_IN_BYTES 						9

typedef enum {
	NONE = 0,
	RTC_SET=1,
	RTC_PAUSE=2,
	RTC_RESUME=3,
	RTC_RESET=4,
	HK_CONFIG=5,
	PTT_POP=6,
	PTT_AVAILABLE=7,
	PTT_PAUSE=8,
	PTT_RESUME=9,
	PTT_ISPAUSED=10,
	PTT_RESET = 18,

	ADC_LOAD = 11,
	ADC_STATE = 12,
	ADC_RESET = 13,

	CMD_SET_CONFIG_MODE = 14,
	CMD_SET_PTT_MODE = 15,
	CMD_SET_HK_MODE = 16,
	CMD_SET_ADC_MODE = 17,


	ECHO=0xFA,
	EXIT = -1
} CommandId_Typedef;

typedef enum {
	CMD_OK = 0, CMD_FAILED, CMD_WAITING
} CommandResult_Typedef;

typedef enum{
	CMD_CONFIG_MODE,
	CMD_PTT_MODE,
	CMD_HK_MODE,
	CMD_ADC_MODE,
}CommandMode_Typedef;


/**
 * Cmd_Typedef
 *
 * Each command is a 10bytes sequence
 */
typedef struct {

	uint8_t id;
	uint8_t status;
	uint8_t result;
	uint8_t param[CMD_MAX_PARAMETER_SIZE_IN_BYTES];
} Command_Typedef;


CommandMode_Typedef Cmd_GetCurrentOperationMode(void);

#endif /* INCLUDE_COMMAND_H_ */
