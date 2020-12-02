/**
 * @file ptt.h
 *
 * @author: Anderson
 *
 * \defgroup PTT_Service PTT_Service
 * \ingroup SBCDA_APP
 *
 * @{
 */

#ifndef INCLUDE_PTT_H_
#define INCLUDE_PTT_H_

#include "FreeRTOS.h"
#include "task.h"
#include <m2sxxx.h>   /* Cortex-M3 processor and core peripherals           */

/**
 * SBCDA Decoder instance description
 */



/**
 * PTT Package parameters
 */
#define PTT_TOTAL_RECORD_SIZE 									54 //bytes
#define PTT_TIME_TAG_OCTETS_LENGTH								8
#define PTT_SENSOR_DATA_OCTETS_MAX_LENGTH						31  // max 248 bits (pttIdNumber is excluded)
#define PTT_PACKAGE_BUFFER_SIZE									64  //  Max number of PCD records


/*
 * PTT Control states
 */
#define PTT_BUFFER_ERROR 										0
#define PTT_BUFFER_OK	 										1

/*******************************************************************************************
 * PTTPackage_Typedef
 *
 * \brief PTT Package data structure
 *
 * The PTT Package data stores PTT Sensor data along
 * with a control header.
 *
 *******************************************************************************************/
typedef struct {
	uint8_t pckgType; /**< Identification byte of the PTT Package. */
	uint8_t decoderNumber; /**< Number of the decoder used for decoding this package*/
	uint16_t ampRms; /**< RMS Amplitude of the signal */
	uint32_t freqMeasure; /**< Frequency containing the PTT signal of this package*/
	uint32_t pttIdNumber; /**< PTT Identification number */
	uint8_t msgLength; /**< Message Length parameter */
	uint8_t timeTag[PTT_TIME_TAG_OCTETS_LENGTH]; /**< Time tag indicating when the package was decoded */
	uint8_t sensorData[PTT_SENSOR_DATA_OCTETS_MAX_LENGTH]; /**< Sensor data */
	uint16_t crc; /**< CRC calculated over the SensorData vector */

	// this fields are for internatl control only,
	// they are not sent to on-board computer as part
	// of the package
	uint8_t status; /**< Current decoding state of the PTT Package (Control Only)*/
	uint8_t synch_state;
	uint16_t symb_array[8]; // to store symbols received
	uint32_t synch_patternA;
	uint32_t synch_patternB;
	uint16_t total_symbol_cnt;
	uint16_t bit_cnt; /**< Number of bits of the PTT message (Control Only)*/
	uint8_t symb_cnt;

} PTTPackage_Typedef;

/**
 * PTTDecode_Status
 * \brief Indicates the current decoding status of a PTT Package
 *
 */
typedef enum{
	PTT_FREE=0, /**<  Package clean */
	PTT_FRAME_SYNCH, /**<  Package is in Frame Synch phase. */
	PTT_MESSAGE_LENGTH, /**<  Package is receiving the Message Length parameter bits. */
	PTT_ID_NUMBER,/**<  Package is receiving the PTT ID Number parameter bits. */
	PTT_DATA,/**<  Package is receiving the PCD Sensor data bits */
	PTT_ERROR, /**<  Package failed the Fram Synch phase. */
	PTT_READY /**<  Package is is ready to be transmited to the on-board computer. */
}PTTDecode_Status;

//typedef struct {
//	uint8_t msgType;
//	uint8_t msgSubType;
//	uint8_t satelliteCode;
//	uint8_t satelliteSubCode;
//	uint8_t sourceSiteCode;
//	uint8_t subsystemType_Number;
//	uint8_t receiveSiteCode;
//	uint8_t spare;
//	uint8_t channelId;
//	uint8_t timeTag [PTT_TIME_TAG_OCTETS_LENGTH];
//	uint8_t freqMeasure [PTT_FREQ_MEASURE_OCTETS_LENGTH];
//	uint8_t msgLength;
//	uint8_t channelStatus;
//	uint8_t receivedSyncWord;
//	uint8_t platformId [PTT_PLATFORM_ID_OCTETS_LENGTH];
//	uint8_t sensorData[PTT_SENSOR_DATA_OCTETS_MAX_LENGTH];
//}PTTData_typedef;


/**
 * ************************************************************************************************
 * PTT SERVICE SPECIFIC FUNCTIONS
 * ************************************************************************************************
 */

/*
 * PTT_GetTaskHandler
 */
TaskHandle_t * PTT_GetTaskHandler (void );

/**
 * PTT_Service_Setup
 *
 * \brief This function initializes the internal package buffer pointers and sets up the detection task.
 *
 */
void PTT_Service_Setup( void );

/**
 * PTT_Service_Pause
 * \brief Pauses the detection task
 */
void PTT_Service_Pause( void );

/**
 * PTT_Service_Resume
 * \brief Resumes the detection task
 */
void PTT_Service_Resume( void );

/**
 * PTT_Service_isPaused
 * \brief Checks whether the detection task is paused or not
 *
 * @return PTT Decoding task state
 *         <pre>
 *         0 - Task is running;
 *         1 - Task is Paused;
 *         </pre>
 */
uint8_t PTT_Service_isPaused( void );

/**
 * PTT_ResetPackage
 * \brief Resets a PTT Package to its PTT_FREE state.
 */
void PTT_ResetPackage(PTTPackage_Typedef * ptr);


/* ************************************************************************************************
 * PTT PACKAGE BUFFER FUNCTIONS
 * ************************************************************************************************
 */


/**
 * PTT_PopFreePackage
 * \brief Returns the next free package from the free packages stack. If the stack is empty, returns a 0 pointer
 *
 * @return Free PTT Package
 */
PTTPackage_Typedef * PTT_PopFreePackage(void);

/**
 * PTT_PushFreePackage
 * \brief Pushes a free PTT package into the Free Packages Stack
 */
uint8_t PTT_PushFreePackage(PTTPackage_Typedef * ptr);

/**
 * PTT_PopReadyPackage
 * \brief Returns the next free package from the free packages stack. If the stack is empty, returns a 0 pointer
 *
 * @return Ready PTT Package
 */
PTTPackage_Typedef * PTT_PopReadyPackage(void);

/**
 * PTT_GetReadyFifoTail
 * Returns the next free package from the free packages stack
 * If the stack is empty, returns a 0 pointer
 */
PTTPackage_Typedef * PTT_GetReadyFifoTail(void);

/**
 * PTT_PushReadyPackage
 * \brief Pushes a ready PTT Package into the PTT Ready FIFO
 *
 * @param ptr Pointer to the package to be pushed.
 */
void PTT_PushReadyPackage(PTTPackage_Typedef * ptr);


/**
 * PTT_FreePackages_Available
 * \brief Returns the number of FREE PTT Packages available
 *
 * @return Number of free PTT Packages
 */
uint16_t PTT_FreePackages_Available( void );


/**
 * PTT_ReadyPackages_Available
 *
 * \brief Returns the number of PTT Records currently in the ready package buffer
 *
 * @return Number of ready PTT Packages
 */
uint16_t PTT_ReadyPackages_Available( void );


/**
 * PTT_SetPackageReadMutex
 *
 * \brief Used for indicating that a PTT_POP command was received.
 *
 * When the PackageReadMutex is set the PTT Decoding task takes
 * one PTT Ready package of the PTT Ready FIFO and updates
 * its pointers.
 */
void PTT_SetPackageReadMutex();

/**
 * PTT_ClearPackageReadMutex
 *
 * \brief Used for indicating that a pending PTT_POP command handled.
 *
 * When the PackageReadMutex is set the PTT Decoding task takes
 * one PTT Ready package of the PTT Ready FIFO and updates
 * its pointers. After updating the FIFO the PTT Decoding task resets
 * the PackageReadMutex, allowing other PTT_POP commands to be executed.
 */
void PTT_ClearPackageReadMutex();

/**
 * PTT_GetPackageReadMutex
 *
 * \brief Returns the current state of the PackageReadMutex
 *
 * @return PackageReadMutex
 */
uint8_t PTT_GetPackageReadMutex( void );

/**
 * PTT_GetReadyFifoOutput
 *
 * \brief Returns a pointer to the head of the PTT Ready packages FIFO.
 *
 * @return Ready PTT Package
 */
uint8_t * PTT_GetReadyFifoOutput( void );

/**
 * PTT_SetReadyFifoOutput
 *
 * \brief Sets the current output FIFO pointer.
 *
 * @param ptr Pointer to the next PTT READY package to be read by the on-board computer
 */
void PTT_SetReadyFifoOutput( uint8_t * ptr );

/**
 * @}
 */
#endif /* INCLUDE_PTT_H_ */
