/*
 * comm.c
 *
 *  Created on: 09/02/2017
 *      Author: Anderson
 *
 *
 *      I2C Comm
 *
 *      This file contains the I2C implementation of the communication API
 *      described  by the comm.h header.
 */

#include "../drivers_config/sys_config/sys_config.h"
#include "../../CMSIS/mss_assert.h"
#include <stdint.h>

#include "app.h"
#include "comm.h"



#ifdef COMM_I2C

uint16_t rx_bytes_available;

#include "../drivers/mss_i2c/mss_i2c.h"

void Comm_Write_Handler(mss_i2c_instance_t * this_i2c, uint8_t * p_rx_data,
		uint16_t rx_size) {

	if (rx_size > 0) {

		/*
		 * Every new write to the I2C Rx buffer overwrites
		 * the previous one. Data must be consumed
		 * before a new write happen
		 */
		rx_bytes_available = rx_size;
		/**
		 * PTT_POP doesn't require an ACK response.
		 * The POP command must be processed immediately. It is
		 * the only command that can not expect the Command_Parser_Task
		 * time share to be executed. That's why this Read signal is
		 * here, telling the PTT_Task to update its ReadyFifo
		 * pointers.
		 */
		if (p_rx_data[0] == PTT_POP && PTT_GetPackageReadMutex()==0) {
			PTT_SetPackageReadMutex();
		}
	}
}

void Comm_Setup(uint8_t * tx_buffer, uint8_t * rx_buffer) {

	/*
	 * I2C Configuration
	 */
	// Initialize the MSS I2C driver with its I2C serial address and
	// serial clock divider.
	MSS_I2C_init(&COMM_I2C_IFACE, COMM_SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256);

	// Specify the buffer used to store the data written by the I2C master.
	MSS_I2C_set_slave_rx_buffer(&COMM_I2C_IFACE, rx_buffer,
	COMM_RX_BUFFER_SIZE);
	// Specify the transmit buffer containing the data that will be
	// returned to the master during read and write-read transactions.
	MSS_I2C_set_slave_tx_buffer(&COMM_I2C_IFACE, tx_buffer,
	COMM_TX_BUFFER_SIZE);
	MSS_I2C_register_write_handler(&COMM_I2C_IFACE,
			(mss_i2c_slave_wr_handler_t) Comm_Write_Handler);

	MSS_I2C_enable_slave(&COMM_I2C_IFACE);

	rx_bytes_available = 0;
}

/**
 * Comm_Resume
 */
void Comm_Resume() {

	/*
	 * I2C Configuration
	 */
	// Initialize the MSS I2C driver with its I2C serial address and
	// serial clock divider.
	MSS_I2C_init(&COMM_I2C_IFACE, COMM_SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256);
	MSS_I2C_enable_slave(&COMM_I2C_IFACE);
}

/**
 * -------------------------------------------------------------------------
 * Comm_Read
 * -------------------------------------------------------------------------
 * 	Reads n bytes from the RX buffer
 * 	Data is place inside the output buffer parameter starting from
 * 	position 0. This function also frees the I2C bus for receiving more
 * 	writes. It's user's responsibility to consume all bytes available
 *	@param n: 		number of bytes to be read
 *	@param output:	output buffer where data is placed
 *	@return
 */
void Comm_Read(uint8_t * output, uint32_t n) {

	ASSERT((n > 0 && n < COMM_RX_BUFFER_SIZE && output != NULL));
	mymemcpy(output, Comm_Get_Rx_Buffer(), n);
	rx_bytes_available = 0;
//	MSS_I2C_enable_slave(&COMM_I2C_IFACE);
}

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
uint8_t Comm_Read_Bytes_Available(uint8_t * output) {

	ASSERT(output != NULL);
	if (rx_bytes_available > 0) {
		mymemcpy(output, Comm_Get_Rx_Buffer(), rx_bytes_available);
		uint8_t tmp = rx_bytes_available;
		rx_bytes_available = 0;
//		MSS_I2C_enable_slave(&COMM_I2C_IFACE);
		return tmp;
	} else {
		return -1;
	}
}

/**
 * -------------------------------------------------------------------------
 * Comm_Bytes_Available
 * -------------------------------------------------------------------------
 * Returns the number of bytes available for reading in
 * the RX buffer
 */
uint16_t Comm_Get_Bytes_Available() {
//	uint8_t bf[100];
//		sprintf(bf, "\r SEnb: %d Pending: %d Status: %x Trans: %x ", COMM_I2C_IFACE.is_slave_enabled,
//				COMM_I2C_IFACE.is_transaction_pending, COMM_I2C_IFACE.slave_status, COMM_I2C_IFACE.transaction);
//		print(bf);
	return rx_bytes_available;
}

/**
 * -------------------------------------------------------------------------
 * Comm_Bytes_Available
 * -------------------------------------------------------------------------
 * Returns the number of bytes available for reading in
 * the RX buffer
 */
void Comm_Set_Bytes_Available(uint16_t bytes) {
	rx_bytes_available = bytes;
}

/**
 * Comm_ClearPendingTransactions
 */
void Comm_ClearPendingTransactions() {
	COMM_I2C_IFACE.slave_status = MSS_I2C_SUCCESS;
	COMM_I2C_IFACE.transaction =  0u; //NO_TRANSACTION
}

/**
 * -------------------------------------------------------------------------
 * Comm_Set_TxBuffer
 * -------------------------------------------------------------------------
 * Sets a new buffer as the default output buffer for I2C
 */
void Comm_Set_Tx_Buffer(uint8_t * buffer, uint16_t size) {
	MSS_I2C_set_slave_tx_buffer(&COMM_I2C_IFACE, buffer, size);
	MSS_I2C_enable_slave(&COMM_I2C_IFACE);
}

/**
 * -------------------------------------------------------------------------
 * Comm_Get_TxBuffer
 * -------------------------------------------------------------------------
 * Returns a pointer to the internal TX buffer
 */
uint8_t * Comm_Get_Tx_Buffer() {
	return (uint8_t *) COMM_I2C_IFACE.slave_tx_buffer;
}

/**
 * -------------------------------------------------------------------------
 * Comm_EnableSlave
 * -------------------------------------------------------------------------
 */
void Comm_EnableSlave() {
	MSS_I2C_enable_slave(&COMM_I2C_IFACE);
}

/**
 * -------------------------------------------------------------------------
 * Comm_DisableSlave
 * -------------------------------------------------------------------------
 */
void Comm_DisableSlave() {
	MSS_I2C_disable_slave(&COMM_I2C_IFACE);
}

/**
 * -------------------------------------------------------------------------
 * Comm_EnableInterrupt
 * -------------------------------------------------------------------------
 */
void Comm_EnableInterrupt() {
	NVIC_EnableIRQ( COMM_I2C_IFACE.irqn );
}

/**
 * -------------------------------------------------------------------------
 * Comm_DisableInterrupt
 * -------------------------------------------------------------------------
 */
void Comm_DisableInterrupt() {
	NVIC_DisableIRQ( COMM_I2C_IFACE.irqn );
}

/**
 * -------------------------------------------------------------------------
 * Comm_GetTransactionStatus
 * -------------------------------------------------------------------------
 */
uint8_t Comm_GetTransactionStatus(){
	return COMM_I2C_IFACE.transaction;
}

/**
 * -------------------------------------------------------------------------
 * Comm_GetSlaveStatus
 * -------------------------------------------------------------------------
 */
uint8_t Comm_GetSlaveStatus(){
	return COMM_I2C_IFACE.slave_status;
}

/**
 * -------------------------------------------------------------------------
 * Comm_isSlaveEnabled
 * -------------------------------------------------------------------------
 */
uint8_t Comm_isSlaveEnabled(){
	return COMM_I2C_IFACE.is_slave_enabled;
}
/**
 * -------------------------------------------------------------------------
 * Comm_WaitComplete
 * -------------------------------------------------------------------------
 */
void Comm_WaitComplete(){
	MSS_I2C_wait_complete(&COMM_I2C_IFACE, 10);
}

/**
 * -------------------------------------------------------------------------
 * Comm_Get_RxBuffer
 * -------------------------------------------------------------------------
 * Returns a pointer to the internal TX buffer
 */
uint8_t * Comm_Get_Rx_Buffer() {
	return (uint8_t *) COMM_I2C_IFACE.slave_rx_buffer;
}

#endif

