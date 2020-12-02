/*
 * cmd_parser_task.c
 *
 *  Created on: 10/02/2017
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

CommandMode_Typedef CURRENT_OPERATION_MODE;

CommandMode_Typedef Cmd_GetCurrentOperationMode(void) {
	return CURRENT_OPERATION_MODE;
}
/**
 * Executes the set RTC operation
 */
void Cmd_RTC_Set(Command_Typedef * cmd) {

	mss_rtc_calendar_t cal;

	if (!RTC_Is_Initialized()) {
		RTC_Setup();
	}
	/*
	 * RTC 'month', 'day', 'week' and 'weekday' parameters
	 * require a minimum value of 1. If zero (or less) is
	 * provided the RTC drives halts the processor.
	 * That is stupid but is the way the driver
	 * was developed.
	 */
	cal.year = cmd->param[0]; // see command specification table
	cal.month = cmd->param[1] > 0 ? cmd->param[1] : 1;
	cal.day = cmd->param[2] > 0 ? cmd->param[2] : 1;
	cal.hour = cmd->param[3];
	cal.minute = cmd->param[4];
	cal.second = cmd->param[5];
	cal.week = cmd->param[6] > 0 ? cmd->param[6] : 1;
	cal.weekday = cmd->param[7] > 0 ? cmd->param[7] : 1;
	RTC_Set(&cal);
	RTC_Start();
}

void Cmd_HK_Set_Config(Command_Typedef * cmd) {

	uint8_t refresh_time = 0;
	uint16_t config_word = 0;

	// big endian
	config_word = cmd->param[0];
	config_word = (config_word << 8) | cmd->param[1];
	refresh_time = cmd->param[2];

	HK_Set_Config(config_word);
	HK_Set_Refresh_Time(refresh_time);
}

/**
 * Command_Parser_Task
 */
void Command_Parser_Task(void *pvParameters) {

	uint32_t pendingTransTimeout = COMM_TRANSACTION_TIMOUT;
	uint8_t rx_buffer[COMM_RX_BUFFER_SIZE];
	uint8_t tx_buffer[COMM_TX_BUFFER_SIZE];
	uint8_t *buffer_ptr;
	tx_buffer[0] = ACK_PCKG_ID;

	uint8_t cmd_buffer[CMD_MAX_SIZE_IN_BYTES];
	Command_Typedef * cmd_received;
	CommandResult_Typedef cmd_result = CMD_OK;
	uint32_t cmd_data;
	CURRENT_OPERATION_MODE = CMD_PTT_MODE;

	Comm_Setup(tx_buffer, rx_buffer); // initializes comm layer

	buffer_ptr = (uint8_t*) PTT_GetReadyFifoOutput();
	Comm_Set_Tx_Buffer(buffer_ptr, PTT_TOTAL_RECORD_SIZE);

	for (;;) {

		// waits until next command is received
		if (Comm_Get_Bytes_Available() != 0) {
			cmd_received = (Command_Typedef *) Comm_Get_Rx_Buffer();

			switch (cmd_received->id) {
			case CMD_SET_CONFIG_MODE:
				/*
				 * The configuration mode is used to setup configuration
				 * patterns to the Decoder_Unit hardware. This command
				 * must be sent before any other configuration command
				 * (RTC_SET, HK_CONFIG, etc.) in order
				 * to allow the OS to suspend the PTT package and respond
				 * all other requests in time.
				 */
				//PTT_Service_Pause();
				CURRENT_OPERATION_MODE = CMD_CONFIG_MODE;
				cmd_result = CMD_OK;
				cmd_data = 0;
				Comm_Set_Tx_Buffer(tx_buffer, COMM_TX_BUFFER_SIZE);
				break;
			case CMD_SET_HK_MODE:
				/*
				 * Setting the parser operation to HK_MODE will simply
				 * change the current output buffer to the HK_OUTPUT_BUFFER.
				 * There is no ack in this mode, the master simply reads the
				 * current HK package and verifies its type (first byte of
				 * every package) and data. These are the only ways for the
				 * master to check if this command was properly executed
				 */
				Comm_Set_Tx_Buffer(HK_Get_Buffer(), HOUSEKEEPING_PACKAGE_SIZE);
				CURRENT_OPERATION_MODE = CMD_HK_MODE;
				cmd_result = CMD_OK;
				cmd_data = 0;
				break;
			case CMD_SET_PTT_MODE:
				/*
				 * This command sets the current ouput buffer ptr to the
				 * default READY_FIFO_OUTPUT for ptt packages. There is no
				 * ACK repply to this command. In order to check if it was
				 * executed properly, the master has to check whether
				 * the received packages are of PTT_TYPE (first byte).
				 */
				//PTT_Service_Resume();
				Comm_Set_Tx_Buffer(PTT_GetReadyFifoOutput(),
				PTT_TOTAL_RECORD_SIZE);
				CURRENT_OPERATION_MODE = CMD_PTT_MODE;
				cmd_result = CMD_OK;
				cmd_data = 0;
				break;
				///// configuration commands
			case CMD_SET_ADC_MODE:
				/*
				 * This command sets the current ouput buffer ptr to the
				 * default ADC_BUFFER_OUTPUT for adc sample packages.
				 */
				//PTT_Service_Pause();
				Comm_Set_Tx_Buffer((uint8_t*) ADC_GetSamplerBuffer(),
				ADC_BUFFER_SIZE_IN_BYTES);
				CURRENT_OPERATION_MODE = CMD_ADC_MODE;
				cmd_result = CMD_OK;
				cmd_data = 0;
				break;
				///// configuration commands
			case ECHO:
				print("\n\r> Echo: ");
				print_hex((uint8_t*) cmd_received, 10);
				print("\n\r");
				cmd_result = CMD_OK;
				cmd_data = 0;
				break;
			case RTC_SET:
				Cmd_RTC_Set(cmd_received);
				cmd_result = CMD_OK;
				cmd_data = 0;
				break;
			case RTC_PAUSE:
				RTC_Stop();
				cmd_result = CMD_OK;
				cmd_data = 0;
				break;
			case RTC_RESUME:
				RTC_Start();
				cmd_result = CMD_OK;
				cmd_data = 0;
				break;
			case RTC_RESET:
				RTC_Reset();
				cmd_result = CMD_OK;
				cmd_data = 0;
				break;
			case HK_CONFIG:
				Cmd_HK_Set_Config(cmd_received);
				cmd_result = CMD_OK;
				cmd_data = 0;
				break;
			case PTT_AVAILABLE:
				/*
				 * PTT_AVAILABLE must be executed in CMD_MODE
				 * in order to generate a valid ACK response
				 */
				cmd_data = PTT_ReadyPackages_Available();
				cmd_result = CMD_OK;
				break;
				//		case PTT_POP: // executed in the Comm_Write_Handler ISR
				//			/*
				//			 * PTT_POP doesn't require an ACK response.
				//			 * The master simply sends the PTT_POP and then
				//			 * checks if the read PTT_PACKAGE was updated.
				//			 * PTT_POP can be executed in both PTT and CMD
				//			 * modes.
				//			 */
				//			PTT_SetPackageReadMutex();
				//			while (PTT_GetPackageReadMutex())
				//				;
				//			cmd_result = CMD_OK;
				//			cmd_data = 0;
				//			break;
			case PTT_PAUSE:
				PTT_Service_Pause();
				cmd_result = CMD_OK;
				cmd_data = 0;
				break;
			case PTT_RESUME:
				PTT_Service_Resume();
				cmd_result = CMD_OK;
				cmd_data = 0;
				break;
			case PTT_RESET:
				PTT_Service_Setup ();
				cmd_result = CMD_OK;
				cmd_data = 0;
				break;
			case PTT_ISPAUSED:
				/*
				 * PTT_ISPAUSED must be executed in CMD_MODE
				 * in order to generate a valid ACK response
				 */
				cmd_data = PTT_Service_isPaused() & 0x01;
				cmd_result = CMD_OK;
				cmd_data = 0;
				break;
			case ADC_LOAD:
				ADC_SetState(ADC_BUSY);
				cmd_result = CMD_OK;
				cmd_data = 0;
				break;
			case ADC_STATE:
				cmd_result = CMD_OK;
				cmd_data = ADC_GetState();
				break;
			case ADC_RESET:
				ADC_SetState(ADC_IDLE);
				cmd_result = CMD_OK;
				cmd_data = ADC_GetState();
				break;
			default:
				cmd_result = CMD_FAILED;
				cmd_data = 0;
				break;
			}
			if (CURRENT_OPERATION_MODE == CMD_CONFIG_MODE) { // in config mode all commands have ack
				tx_buffer[0] = ACK_PCKG_ID;
				tx_buffer[1] = cmd_received->id;
				tx_buffer[2] = cmd_result;
				// some commands may send back a 32 bit data value as result (MSB)
				tx_buffer[3] = cmd_data >> 24;
				tx_buffer[4] = cmd_data >> 16;
				tx_buffer[5] = cmd_data >> 8;
				tx_buffer[6] = cmd_data;
				tx_buffer[7] = 0;
				tx_buffer[8] = 0;
				Comm_Set_Tx_Buffer(tx_buffer, COMM_TX_BUFFER_SIZE);
			}
			Comm_Set_Bytes_Available(0);
		} else {
			/***********************************************************
			 * WARNING: Comm Layer must be reset from time
			 * to time to avoid locking on a broken transaction
			 * forever.
			 ***********************************************************/
			if (Comm_GetTransactionStatus() || Comm_GetSlaveStatus()
					|| !Comm_isSlaveEnabled()) {
				pendingTransTimeout--;
			} else {
				pendingTransTimeout = COMM_TRANSACTION_TIMOUT;
			}
			if (pendingTransTimeout == 0) {
				pendingTransTimeout = COMM_TRANSACTION_TIMOUT;
				//Comm_EnableSlave();
				Comm_Setup(Comm_Get_Tx_Buffer(), Comm_Get_Rx_Buffer());
			}
			/***********************************************************/
		}
	}
}
