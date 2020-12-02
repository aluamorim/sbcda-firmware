/*
 * comm.h
 *
 *  Created on: 10/02/2017
 *      Author: Anderson
 *
 *      Communcation Layer API
 *
 *      This file defines the function prototypes of the communication layer. The communication
 *      API is independent of internal implementation. Same functionalities must be offered either
 *      for a I2C or RS244 channel of communication.
 *
 *
 */
#ifndef INCLUDE_COMM_H_
#define INCLUDE_COMM_H_

#define COMM_I2C
//#define COMM_UART
#define COMM_RX_BUFFER_SIZE	16
#define COMM_TX_BUFFER_SIZE	16
#define COMM_I2C_IFACE	g_mss_i2c0
#define COMM_SLAVE_SER_ADDR 0x10u

#define COMM_TRANSACTION_TIMOUT 100000


/**
 * -------------------------------------------------------------------------
 * Comm_Init
 * -------------------------------------------------------------------------
 * 	Initializes the communication layer.
 *
 */

void Comm_Setup(uint8_t * tx_buffer, uint8_t * rx_buffer) ;

/**
 * Comm_Resume
 */
void Comm_Resume( void );

/**
 * -------------------------------------------------------------------------
 * Comm_Read
 * -------------------------------------------------------------------------
 * 	Reads n bytes from the RX buffer
 * 	Data is place inside the output buffer parameter starting from
 * 	position 0.
 *	@param n: 		number of bytes to be read
 *	@param output:	output buffer where data is placed
 *	@return
 */
void Comm_Read(uint8_t * output, uint32_t n) ;

/**
 * -------------------------------------------------------------------------
 * Comm_Read_Bytes_Available
 * -------------------------------------------------------------------------
 * 	Reads all bytes available in the rx_buffer
 * 	Data is place inside the output buffer parameter starting from
 * 	position 0.
 *	@param n: 		number of bytes to be read
 *	@param output:	output buffer where data is placed
 *	@return 		number of bytes read
 *					-1 if no bytes are available for reading
 *
 */
uint8_t Comm_Read_Bytes_Available(uint8_t * output);

/**
 * -------------------------------------------------------------------------
 * Comm_Bytes_Available
 * -------------------------------------------------------------------------
 * Returns the number of bytes available for reading in
 * the RX buffer
 */
uint16_t Comm_Get_Bytes_Available();

/**
 * -------------------------------------------------------------------------
 * Comm_Set_Bytes_Available
 * -------------------------------------------------------------------------
 * Returns the number of bytes available for reading in
 * the RX buffer
 */
void Comm_Set_Bytes_Available(uint16_t bytes);

/**
 * Comm_ClearPendingTransactions
 */
void Comm_ClearPendingTransactions( void );

/**
 * -------------------------------------------------------------------------
 * Comm_Set_TxBuffer
 * -------------------------------------------------------------------------
 * Sets a new buffer as the default output buffer for I2C
 */
void Comm_Set_Tx_Buffer(uint8_t * buffer, uint16_t size);

/**
 * -------------------------------------------------------------------------
 * Comm_Get_TxBuffer
 * -------------------------------------------------------------------------
 * Returns a pointer to the internal TX buffer
 */
uint8_t * Comm_Get_Tx_Buffer();

/**
  * -------------------------------------------------------------------------
 * Comm_Set_Slave_Mode
 * -------------------------------------------------------------------------
 */
void Comm_EnableSlave();

/**
 * -------------------------------------------------------------------------
 * Comm_DisableSlave
 * -------------------------------------------------------------------------
 */
void Comm_DisableSlave();

/**
 * -------------------------------------------------------------------------
 * Comm_EnableInterrupt
 * -------------------------------------------------------------------------
 */
void Comm_EnableInterrupt();

/**
 * -------------------------------------------------------------------------
 * Comm_DisableInterrupt
 * -------------------------------------------------------------------------
 */
void Comm_DisableInterrupt();

/**
 * -------------------------------------------------------------------------
 * Comm_GetTransactionStatus
 * -------------------------------------------------------------------------
 */
uint8_t Comm_GetTransactionStatus();

/**
 * -------------------------------------------------------------------------
 * Comm_GetSlaveStatus
 * -------------------------------------------------------------------------
 */
uint8_t Comm_GetSlaveStatus();

/**
 * -------------------------------------------------------------------------
 * Comm_isSlaveEnabled
 * -------------------------------------------------------------------------
 */
uint8_t Comm_isSlaveEnabled();

/**
 * -------------------------------------------------------------------------
 * Comm_WaitComplete
 * -------------------------------------------------------------------------
 */
void Comm_WaitComplete();

/**
 * -------------------------------------------------------------------------
 * Comm_Get_RxBuffer
 * -------------------------------------------------------------------------
 * Returns a pointer to the internal TX buffer
 */
uint8_t * Comm_Get_Rx_Buffer();


#endif
