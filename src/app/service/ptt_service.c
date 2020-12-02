/*
 * ptt_service.c
 *
 *	This file implements functions related to the ptt package read reading process
 *	Specific driver function of the decoder module are implemented on decoder/decoder_driver.c
 *  Created on: 16/02/2017
 *      Author: Anderson
 */

#include <stdint.h>
#include "service.h"
#include "app.h"

TaskHandle_t pttTaskHandler;

PTTPackage_Typedef pttPackageBuffer[PTT_PACKAGE_BUFFER_SIZE];

PTTPackage_Typedef * freePackagesStack[PTT_PACKAGE_BUFFER_SIZE];
PTTPackage_Typedef * readyPackagesFifo[PTT_PACKAGE_BUFFER_SIZE];

/**
 * Buffer control variables
 */
uint16_t freeStackTop;
uint8_t stackMutex;

uint16_t readyFifoHead;
uint16_t readyFifoTail;
uint16_t readyFifoCnt = 0;
uint8_t fifoMutex;

uint8_t * readyFifoOutput;
uint8_t packageReadMutex;

uint32_t no_pckg_ptr;

//static uint16_t pttGlobalCnt;

/**
 * PTT Task control
 */
uint8_t pttPauseDecoding;

uint8_t __getStackMutex() {
	return stackMutex;
}
uint8_t __getFifoMutex() {
	return fifoMutex;
}

void __setStackMutex() {
	stackMutex = 1;
}
void __setFifoMutex() {
	fifoMutex = 1;
}

void __clearStackMutex() {
	stackMutex = 0;
}
void __clearFifoMutex() {
	fifoMutex = 0;
}

void inc_ready_fifo_tail() {
	readyFifoTail++;
	if (readyFifoTail == PTT_PACKAGE_BUFFER_SIZE) {
		readyFifoTail = 0;
	}
}

void inc_ready_fifo_head() {

	// check if push has overwritten a previous ready package
	if (readyFifoHead == readyFifoTail && readyFifoCnt==PTT_PACKAGE_BUFFER_SIZE) {
		inc_ready_fifo_tail();
	}
	readyFifoHead++;
	if (readyFifoHead == PTT_PACKAGE_BUFFER_SIZE) {
		readyFifoHead = 0;
	}
}

/**
 * PTT_GetTaskHandler
 */
TaskHandle_t * PTT_GetTaskHandler(void) {
	return &pttTaskHandler;
}

/**
 * PTT_Service_Setup
 *
 * This function initializes the internal buffer pointers
 *
 */
void PTT_Service_Setup(void) {
	uint16_t i0 = 0;
	freeStackTop = 0;
	readyFifoHead = 0;
	readyFifoTail = 0;
	readyFifoCnt = 0;
	pttPauseDecoding = 0;

	stackMutex = 0;
	fifoMutex = 0;

	readyFifoOutput = NULL;
	packageReadMutex = 0;
	for (i0 = 0; i0 < PTT_PACKAGE_BUFFER_SIZE; i0++) {
		pttPackageBuffer[i0].pckgType = PTT_PCKG_ID;
		pttPackageBuffer[i0].status = PTT_FREE;
		PTT_ResetPackage(&pttPackageBuffer[i0]);
		PTT_PushFreePackage(&(pttPackageBuffer[i0])); // all free
		readyPackagesFifo[i0] = 0; // no ready
	}
	no_pckg_ptr = NO_PCKG_ID & 0x00FF;
	PTT_SetReadyFifoOutput(0);
	DDS_Setup();
}

void PTT_SetPackageReadMutex() {
	packageReadMutex = 1;
}

void PTT_ClearPackageReadMutex() {
	packageReadMutex = 0;
}

uint8_t PTT_GetPackageReadMutex(void) {
	return packageReadMutex;
}

uint8_t * PTT_GetReadyFifoOutput(void) {
	if (readyFifoOutput == 0) {
		readyFifoOutput = (uint8_t *) &no_pckg_ptr;
		return (uint8_t *) readyFifoOutput;
	} else {
		return readyFifoOutput;
	}

}

void PTT_SetReadyFifoOutput(uint8_t * ptr) {
	readyFifoOutput = ptr;
}
/**
 * PTT_Service_Pause
 */
void PTT_Service_Pause(void) {
	pttPauseDecoding = 1;
}

/**
 * PTT_Service_Resume
 */
void PTT_Service_Resume(void) {
	pttPauseDecoding = 0;
}

/**
 * PTT_isPaused
 */
uint8_t PTT_Service_isPaused(void) {
	return pttPauseDecoding;
}
void PTT_ResetPackage(PTTPackage_Typedef * ptr) {
	if (ptr != 0) {
		ptr->decoderNumber = -1;
		ptr->pttIdNumber = 0;
		ptr->msgLength = 0;
		ptr->freqMeasure = 0;
		ptr->ampRms = 0;
		ptr->crc = 0;
		ptr->total_symbol_cnt = 0;
		ptr->synch_patternA = 0;
		ptr->synch_patternB = 0;
		ptr->synch_state = 0;
		ptr->symb_cnt = 0;
		ptr->status = PTT_FREE;
		ptr->bit_cnt = 0;
		ptr->symb_array[0] = 0;
		ptr->symb_array[1] = 0;
		ptr->symb_array[2] = 0;
		ptr->symb_array[3] = 0;
		ptr->symb_array[4] = 0;
		ptr->symb_array[5] = 0;
		ptr->symb_array[6] = 0;
		ptr->symb_array[7] = 0;
	}
}

/**
 * PTT_PopFreePackage
 * Returns the next free package from the free packages stack
 * If the stack is empty, returns a 0 pointer
 */
PTTPackage_Typedef * PTT_PopFreePackage(void) {
	PTTPackage_Typedef * ptr = 0;
	while (__getStackMutex() == 1)
		;
	__setStackMutex();
	if (freeStackTop > 0) {
//		freeStackTop--;
		ptr = freePackagesStack[--freeStackTop];
		freePackagesStack[freeStackTop] = 0;
	}
	__clearStackMutex();

	return ptr;
}

/**
 * PTT_PushFreePackage
 * Pushes a free PTT package into the Free Packages Stack
 */
uint8_t PTT_PushFreePackage(PTTPackage_Typedef * ptr) {

	while (__getStackMutex() == 1)
		;
	__setStackMutex();
	if (freeStackTop >= PTT_PACKAGE_BUFFER_SIZE) {
		__clearStackMutex();
		return 0; // free stack is already full
	} else if (ptr != 0) {
		freePackagesStack[freeStackTop++] = ptr;
	}
	__clearStackMutex();

	return 1;
}

/**
 * PTT_PopReadyPackage
 * Returns the next free package from the free packages stack
 * If the stack is empty, returns a 0 pointer
 */
PTTPackage_Typedef * PTT_PopReadyPackage(void) {

	while (__getFifoMutex() == 1)
		;

	__setFifoMutex();
	if (readyFifoCnt > 0) {
		PTTPackage_Typedef * ptr = readyPackagesFifo[readyFifoTail];

			inc_ready_fifo_tail();
			readyFifoCnt--;
			__clearFifoMutex();
		if (ptr->status == PTT_READY) {
			return ptr;
		}else{
			// error: the program should never get here
			return (PTTPackage_Typedef *) &no_pckg_ptr;
		}
	}
	__clearFifoMutex();
	return (PTTPackage_Typedef *) &no_pckg_ptr;
}


/**
 * PTT_GetReadyFifoTail
 * Returns the next free package from the free packages stack
 * If the stack is empty, returns a 0 pointer
 */
PTTPackage_Typedef * PTT_GetReadyFifoTail(void) {

	if (readyFifoCnt > 0) {
		PTTPackage_Typedef * ptr = readyPackagesFifo[readyFifoTail];
		if (ptr->status == PTT_READY) {
			return ptr;
		}else{
			// error: program should never get here!
		}
	}
	return (PTTPackage_Typedef *) &no_pckg_ptr;
}

void PTT_PushReadyPackage(PTTPackage_Typedef * ptr) {

	while (__getFifoMutex() == 1)
		;
	__setFifoMutex();
	readyPackagesFifo[readyFifoHead] = ptr;
	inc_ready_fifo_head();
	readyFifoCnt++;
	__clearFifoMutex();
}

uint16_t PTT_FreePackages_Available(void) {
	return freeStackTop;
}

uint16_t PTT_ReadyPackages_Available(void) {
	return readyFifoCnt;
}

