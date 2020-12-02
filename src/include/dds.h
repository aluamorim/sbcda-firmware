/*
 * @file decoder.h
 *
 *	This file contains the description for the detection/decoder system (DDS)
 *	control API.
 *
 * @author Anderson
 */

#ifndef INCLUDE_DDS_H_
#define INCLUDE_DDS_H_

#include <stdint.h>
#include <m2sxxx.h>   /* Cortex-M3 processor and core peripherals           */
#include "ptt.h"


//#define GENERATE_REPORT

/**
 * \defgroup DDS DDS
 * \ingroup SBCDA_APP
 * @{
 */

/**
 * Detect/Decoder bit masks
 */
#define DDS_BASE												0x30000000
#define DDS_DECODER												((DDS_Typedef *) DDS_BASE)
#define DDS_NUMBER_OF_DECODERS									10//12

// ADC SAMPLER REG
#define DDS_ADC_RMS_VALUE(reg)									(reg&0x0000FFFF)
#define DDS_ADC_SAMPCTRL_SAMP_ENABLE							0x00000010
#define DDS_ADC_SAMPCTRL_SAMP_SAMPCLR							0x00000008
#define DDS_ADC_SAMPCTRL_SAMP_FULL								0x00000004
#define DDS_ADC_SAMPCTRL_SAMP_HFULL								0x00000002
#define DDS_ADC_SAMPCTRL_SAMP_EMPTY								0x00000001

#define DDS_ADC_SAMPDATA_REAL									0xFFFF0000
#define DDS_ADC_SAMPDATA_IMG									0x0000FFFF

// DECODE STATE REG
#define DDS_DECODSTATE_DECOD_ENABLE								0x00000010
#define DDS_DECODSTATE_MEM_BAK_SEL								0x00000008
#define DDS_DECODSTATE_DETECT_BUSY								0x00000004
#define DDS_DECODSTATE_DDS_BUSY									0x00000002
#define DDS_DECODSTATE_DDS_STARTOP								0x00000001

// ACTIVE LIST REG
#define DDS_ACTIVE_LIST_VALUE(reg)								(reg&0x00000FFF)
#define DDS_ACTIVE_LIST_SET(reg)								(0x00000001<<reg)
#define DDS_ACTIVE_LIST_CLEAR(reg)								(~(0x00000001<<reg))

// INIT FREQ REGs
#define DDS_INIT_FREQ_VALUE(reg)								(reg&0x0000FFFF)

// DDSOUTx REG
#define DDS_PTTDCTRL_EOP										0x00000008
#define DDS_PTTDCTRL_BUFFER_FULL								0x00000004
#define DDS_PTTDCTRL_BUFFER_HFUL								0x00000002
#define DDS_PTTDCTRL_BUFFER_EMPTY								0x00000001

#define DDS_PTTD0_DDSD_FREQ_VALUE(reg)							((reg>>12)&0x000FFFFF) //[31:12]
#define DDS_PTTD0_DDSD_MANT_VALUE(reg)							((reg>>6)&0x0000003F) //[11:6]
#define DDS_PTTD0_DDSD_EXP_VALUE(reg)							((reg)&0x0000003F) //[5:0]


#define DDS_PTTD1_CH(reg)										((reg>>24)&0x000F) // DDSdCh <= DDSdOut1[27:24] - 4 bits long
#define DDS_PTTD1_FREQ_LOCK										0x00800000 // BIT 23
#define DDS_PTTD1_SYMB_LOCK										0x00400000 // BIT 22
#define DDS_PTTD1_SYMB_VALUE(reg)								(reg&0x000003FF) //[9:0]


// DETECT CTRL REG
#define DDS_DETECTCTRL_DETECT_EOP								0x00000008
#define DDS_DETECTCTRL_BUFFER_FULL								0x00000004
#define DDS_DETECTCTRL_BUFFER_HFUL								0x00000002
#define DDS_DETECTCTRL_BUFFER_EMPTY								0x00000001

// DETECT CTRL REG
#define DDS_DETECTDATA_DETECT_FREQ_VALUE(reg)					((reg>>16)&0x000007FF) // 11 bits [26:16]
#define DDS_DETECTDATA_DETECT_AMP_VALUE(reg) 					(reg&0x0000FFFF)

#define DDS_INSERT_FREQ_INVALID									-1
#define DDS_INSERT_FREQ_NONE									0
#define DDS_INSERT_FREQ_NEW 									1
#define DDS_INSERT_FREQ_REPLACE 								2
#define DDS_INSERT_FREQ_DISCARD									3


#define DDS_SYNCH_MIN_MASK										0x000FFFF//0x000003FF// the last 10 bits of the synch pattern
#define DDS_SYNCH_PATTERN										0xFFFE2F // 0b1111_1111_1111_1110_0010_1111 - PTT-A2
#define DDS_MAX_SYNCH_SYMBOLS								   	128//56//128
#define DDS_MAX_MSG_LENGTH_SYMBOLS								8
#define DDS_MAX_IDNUMBER_BITS									28
#define DDS_MAX_SYMBOLS_PER_WINDOW								8

#define DDS_FREQ_GUARD_BAND										51 //3200Hz = 62.5*51.2
#define DDS_FREQ_RESOLUTION_HZ									62
#define DDS_FREQ_NUMBER_OF_BITS									11

#define DDS_DEFAULT_AMP_THRESHOLD								100

#define DDS_FREQUENCY_TIMEOUT									200 // windows of 10ms

#define DDS_DETECT_EOP_GPIO										MSS_GPIO_0
#define DDS_DETECT_EOP_MASK										MSS_GPIO_0_MASK

/************************************************************************************************************
 *	DDSDecoder_typedef
 *	Decoder register bank representation. All registers are 32bit long, although not
 *	all bits are necessarily used.
 */
typedef struct {
	__IO uint32_t adcRms; /*RMS level of ADC output averaged over an interval of 10 ms and update every 5 ms */
	__IO uint32_t adcSamplerCtrl;
	__IO uint32_t adcSamplerData;
	__IO uint32_t decodState;
	__IO uint32_t activeList;
	__IO uint32_t initFreq[12/*DDS_NUMBER_OF_DECODERS*/];
	__IO uint32_t pttdBufferCtrl;
	__IO uint32_t pttdBufferData0; /* DATA0 must always be read before DATA1*/
	__IO uint32_t pttdBufferData1; /* A DATA1 Read causes an update on the Buffer Fifo*/
	__IO uint32_t detectBufferCtrl;
	__IO uint32_t detectBufferData;
	__IO uint32_t detectThreshold;
	__IO uint32_t agcSamplerCtrl;
	__IO uint32_t agcSamplerData;
}DDS_Typedef;


typedef enum{
	PTT_SYNCH_START=0,
	PTT_SYNCH_BUSY,
	PTT_SYNCH_OK,
	PTT_SYNCH_FAILED
}DDSFrameSynch_Status;

typedef enum {
	FREQ_NONE,
	FREQ_DETECTED_ONCE,
	FREQ_DETECTED_TWICE,
	FREQ_REPLACED,
	FREQ_DECODING
} DDS_FreqsState_Typedef;

typedef struct {
	uint32_t freq_idx;
	uint32_t freq_amp;
	uint8_t detect_state;
	uint8_t timeout;
} DDS_FreqsRecord_Typedef;

/* ************************************************************************************************
 * DDS DRIVER FUNCTIONS
 * ************************************************************************************************
 */

/**
 * DDSDecoder regbank instance
 */
//DDS_Typedef * SBCDA_DECODER;

#define SBCDA_DECODER DDS_DECODER
/**
 * DDS_GetDecoderInstance
 */
DDS_Typedef * DDS_GetDecoderInstance( void );

/**
 * DDS_Driver_Init
 */
/**
 * DDS_Setup
 * This function must be called before the detection/decoding
 * process starts, in order to initialize the detection
 * driver internal variables correctly.
 */
void DDS_Setup(void) ;

/**
 * DDS_isNewWindowReady
 */
uint8_t DDS_isNewWindowReady (void);

/**
 * DDS_clearNewWindowFlag
 */
void DDS_clearNewWindowFlag (void);

/**
 * DDS_setrNewWindowFlag
 */
void DDS_setNewWindowFlag (void);

uint32_t DDS_GetTotalDetections_Count( void );

uint32_t DDS_GetDecodingFails_Count( void );

uint32_t DDS_GetDiscardDetections_Count( void );

uint32_t DDS_GetReplacedFrequencies_Count(void);

uint32_t DDS_GetReadyPackages_Count(void);

DDS_FreqsRecord_Typedef * DDS_GetDetectedFreqs();

uint32_t DDS_GetActiveMask( void );

uint8_t DDS_GetAvailableDecoders( void );

void DDS_SetDecoder(uint8_t decoder_number);

void DDS_ClearDecoder(uint8_t decoder_number);

void DDS_SetDecoderMask(uint32_t decoder_mask) ;

PTTPackage_Typedef ** DDS_GetWorkingPackages( void );

/**
 * DDS_Enable_Decoder_Module()
 *
 * This function enables the detect/decoder module on FPGA
 * Detection/decoding processes run only if ENABLE is set
 *
 */
void DDS_EnableDecoder( void );

/**
 * DDS_Disable_Decoder()
 *
 * This function disables the detect/decoder module on FPGA
 * Detection/decoding processes run only if ENABLE is set
 *
 */
void DDS_DisableDecoder( void );


/**
 * DDS_DecoderStartOp()
 */
void DDS_DecoderStartOp( void );

/**
 * DDS_DetectEop
 */
uint8_t DDS_DetectEop( void );

/**
 * DDS_DetectEmpty
 *
 * A new detection is signalized when the DDS_DETECTOUT_BUFFER_EMPTY is clear
 */
uint8_t DDS_DetectEmpty( void );

/**
 * DDS_DecoderEop
 */
uint8_t DDS_DecoderEop( void );


/**
 * DDS_DecoderEmpty
 *
 * A new detection is signalized when the DDS_DETECTOUT_BUFFER_EMPTY is clear
 */
uint8_t DDS_DecoderEmpty( void );

/**
 * DDS_POP_DetectBuffer
 *
 * This function performs a READ operation on the
 * detection data buffer. Each READ pops one element
 * of the data fifo, updating the control register
 */
uint32_t DDS_ReadDetectBuffer( void );

/**
 * DDS_SetAmpThreshold
 */
void DDS_SetAmpThreshold(uint32_t val);

/**
 * Returns the correspondent data portion size in bits
 * for a given message length code.
 * OBS: Assuming a 28bit ID_NUMBER format
 */
uint16_t DDS_CalcMessageLength(uint8_t msgLength);
/**
 * Every symbol is a 10bit value in 2's complement format. However,
 * they are placed into a 16bit unsigned int with no sign extension.
 * This function sign extends the symbol values and then compare
 * them in order to determine if they correspond to a 1 or a 0 bit.
 * The following convention is assumed:
 *
 * if symb0 <= symb1 => (-1) to (1) transition => bit 1
 * if symb0 > symb1 =>  (1) to (-1) transition => bit 0
 *
 */
uint8_t DDS_DecodeBit(uint16_t symb0, uint16_t symb1);

/////////////////////////////////////////////////////////////////////////////////////////////////
// Functions related to the DECODING process
//void DDS_Frame_Synch(PTTPackage_Typedef * wpckg,
//		uint16_t pttd_symbol) ;
//
//void DDS_Read_MessageLength(PTTPackage_Typedef * wpckg,
//		uint16_t pttd_symbol);
//
//void DDS_Read_IDNumber(PTTPackage_Typedef * wpckg,
//		uint16_t pttd_symbol);
//
//void DDS_Read_Data(PTTPackage_Typedef * wpckg,
//		uint16_t pttd_symbol);

/////////////////////////////////////////////////////////////////////////////////////////////////

// Functions related to the DETECTION process

/**
 * DDS_Insert_Detected_Freq
 *
 * This procedure tries to insert a new detected freq/amp pair into
 * the current frequencies vector. New frequencies must be at least
 * 3.2KHz apart from previously allocated ones.
 *

 *  @param new_idx = Newly detected frequency index.
 *  @param new_amp = Newly detected frequency amp.
 *  @param assigned_position    = Number of the decoder assigned for the detected frequency
 *
 *  @return DDS_INSERT_FREQ_NEW: new frequency assigned to a free decoder position
 *  @return DDS_INSERT_FREQ_REPLACE: new freq replace a previously assigned decoder
 *  @return DDS_INSERT_FREQ_NONE: new freq could not be insterted because all decoders where busy.
 */
uint8_t DDS_Insert_Detected_Freq(uint32_t newIdx, uint32_t newAmp,
		uint8_t * assigned_decoder);


/**
 * DDS_Detection_Loop
 *
 * The Detection Loop is responsible for acquiring new frequencies,
 * assigning a free decoder to them and allocation a new PTT Package
 * for the decoding process. It is also responsible
 * for applying the software version of the Detection Mask for each
 * new frequency, comparing them to the previously allocated ones.
 *
 * Every new frequency is compared to the previously detected ones
 * to avoid conflicts. A conflict happens when a new frequency is
 * located less than 1.6 KHz (right or left) distant from any other
 * frequency already assigned to a decoder. When a conflict occurs the
 * system keeps the frequency associated to the highest amplitude,
 * discarding the other. At the end of the Detection Loop the system
 * is guaranteed to operate only with the highest components of the
 * spectrum within the current 10ms window.
 *
 * @return The number of newly detected frequencies during the current iteration
 *
 */
uint32_t DDS_Detection_Loop();

/**
 * DDS_Update_Freq_Timeout
 */
void DDS_Update_Freq_Timeout(void);

/**
 * DDS_Decoding_Loop
 *
 * The decoding loop is the procedure responsible for reading
 * symbols out of the PTTD Buffer FIFO, converting them into
 * bit representation and attaching each bit to its corresponding
 * building package.
 *
 * @return The number of finished packages in the current loop
 */
uint32_t DDS_Decoding_Loop();


/**
 * @}
 */

#endif /* INCLUDE_DDS_H_ */
