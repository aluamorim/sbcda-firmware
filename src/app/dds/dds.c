/*
 * decoder_driver.c
 *
 *  Created on: 31/03/2017
 *      Author: Anderson
 */

#include <stdint.h>
#include "dds.h"
#include "ptt.h"
#include "app.h"

DDS_FreqsRecord_Typedef DDS_DETECTED_FREQS[DDS_NUMBER_OF_DECODERS];

uint8_t currently_working_packages, currently_available_decoders;
uint32_t ACTIVE_DECODERS_MASK = 0;

// report related variables
uint32_t decode_full_cnt = 0, early_detect_cnt = 0, discarded_detection_cnt = 0,
		decoding_fails_cnt = 0, total_detections_cnt = 0, replace_freqs_cnt = 0,
		ready_packages_cnt = 0; // simple counter

PTTPackage_Typedef * working_packages[DDS_NUMBER_OF_DECODERS];

uint8_t DETECT_WINDOW_START;

// debug variables
uint32_t pckg_counter = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////
// GENERAL DRIVER INTERFACE
/**
 * DDS_Setup
 */
void DDS_Setup(void) {
	uint8_t i0;
	DDS_DisableDecoder(); // to clean the internal fifos
	replace_freqs_cnt = 0;
	total_detections_cnt = 0;
	discarded_detection_cnt = 0;
	currently_working_packages = 0;
	currently_available_decoders = DDS_NUMBER_OF_DECODERS;
	// initialize
	for (i0 = 0; i0 < DDS_NUMBER_OF_DECODERS; i0++) {
		DDS_DETECTED_FREQS[i0].freq_idx = 0;
		DDS_DETECTED_FREQS[i0].freq_amp = 0;
		DDS_DETECTED_FREQS[i0].detect_state = FREQ_NONE;
		DDS_DETECTED_FREQS[i0].timeout = 0;
		SBCDA_DECODER->initFreq[i0] = 0;
	}
	for (i0 = 0; i0 < 16; i0++) {
		working_packages[i0] = 0;
	}
	SBCDA_DECODER->activeList = 0;
	DETECT_WINDOW_START = 0;
	DDS_SetAmpThreshold(DDS_DEFAULT_AMP_THRESHOLD);
	// enables detect_eop as interrupt input
	MSS_GPIO_config( DDS_DETECT_EOP_GPIO,
	MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_NEGATIVE);
	MSS_GPIO_enable_irq( DDS_DETECT_EOP_GPIO);
	NVIC_SetPriority(GPIO0_IRQn, 0x0); // sets highest priority
	DDS_EnableDecoder(); // enable the decoder
}

/**
 * DDS_isNewWindowReady
 */
uint8_t DDS_isNewWindowReady(void) {
	return DETECT_WINDOW_START;
}

/**
 * DDS_clearNewWindowFlag
 */
void DDS_clearNewWindowFlag(void) {
	DETECT_WINDOW_START = 0;
}

/**
 * DDS_setrNewWindowFlag
 */
void DDS_setNewWindowFlag(void) {
	DETECT_WINDOW_START = 1;
}

/**
 * DDS_GetDecoderInstance
 */
DDS_Typedef * DDS_GetDecoderInstance(void) {
//	if (!SBCDA_DECODER)
//		SBCDA_DECODER = DDS_DECODER;
	return SBCDA_DECODER;

}

DDS_FreqsRecord_Typedef * DDS_GetDetectedFreqs() {
	return DDS_DETECTED_FREQS;
}

void DDS_SetDecoder(uint8_t decoder_number) {
	if (decoder_number < DDS_NUMBER_OF_DECODERS) {
		ACTIVE_DECODERS_MASK |= (0x00000001 << decoder_number);
		SBCDA_DECODER->activeList = ACTIVE_DECODERS_MASK;
	}
}

void DDS_ClearDecoder(uint8_t decoder_number) {
	if (decoder_number < DDS_NUMBER_OF_DECODERS) {
		ACTIVE_DECODERS_MASK &= (~(0x00000001 << decoder_number));
		SBCDA_DECODER->activeList = ACTIVE_DECODERS_MASK & 0x00000FFF; //12 bits // disable that decoder
	}
}

uint32_t DDS_GetActiveMask(void) {
	return ACTIVE_DECODERS_MASK;
}

void DDS_SetDecoderMask(uint32_t decoder_mask) {
	ACTIVE_DECODERS_MASK = decoder_mask;
	SBCDA_DECODER->activeList = decoder_mask;

}

uint8_t DDS_GetAvailableDecoders() {
	return currently_available_decoders;
}

PTTPackage_Typedef ** DDS_GetWorkingPackages(void) {
	return working_packages;
}

uint32_t DDS_GetTotalDetections_Count(void) {
	return total_detections_cnt;
}

uint32_t DDS_GetDecodingFails_Count(void) {
	return decoding_fails_cnt;
}

uint32_t DDS_GetDiscardDetections_Count(void) {
	return discarded_detection_cnt;
}

uint32_t DDS_GetReplacedFrequencies_Count(void) {
	return replace_freqs_cnt;
}

uint32_t DDS_GetReadyPackages_Count(void) {
	return ready_packages_cnt;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * DDS_Enable_Decoder_Module()
 *
 * This function enables the detect/decoder module on FPGA
 * Detection/decoding processes run only if ENABLE is set
 *
 */
void DDS_EnableDecoder(void) {

	SBCDA_DECODER->decodState = DDS_DECODSTATE_DECOD_ENABLE;
}

/**
 * DDS_Disable_Decoder()
 *
 * This function disables the detect/decoder module on FPGA
 * Detection/decoding processes run only if ENABLE is set
 *
 */
void DDS_DisableDecoder(void) {
	SBCDA_DECODER->decodState = 0;
}

/**
 * DDS_DecoderStartOp()
 */
void DDS_DecoderStartOp(void) {

	SBCDA_DECODER->decodState = (DDS_DECODSTATE_DECOD_ENABLE
			| DDS_DECODSTATE_DDS_STARTOP);
}

/**
 * DDS_DetectEop
 */
uint8_t DDS_DetectEop(void) {
//	uint32_t tmp = SBCDA_DECODER->detectBufferCtrl;
//	return tmp & DDS_DETECTCTRL_DETECT_EOP;
	return (SBCDA_DECODER->detectBufferCtrl & DDS_DETECTCTRL_DETECT_EOP); //&& (tmp & DDS_DETECTCTRL_BUFFER_EMPTY);
}

/**
 * DDS_DetectEmpty
 *
 * A new detection is signalized when the DDS_DETECTOUT_BUFFER_EMPTY is clear
 */
uint8_t DDS_DetectEmpty(void) {
	return (SBCDA_DECODER->detectBufferCtrl & DDS_DETECTCTRL_BUFFER_EMPTY); //&& !(st & DDS_DETECTCTRL_DETECT_EOP));
}

/**
 * DDS_DecoderEop
 */
uint8_t DDS_DecoderEop(void) {
	return (SBCDA_DECODER->pttdBufferCtrl & DDS_PTTDCTRL_EOP);
}

/**
 * DDS_DecoderEmpty
 *
 * A new detection is signalized when the DDS_DETECTOUT_BUFFER_EMPTY is clear
 */
uint8_t DDS_DecoderEmpty(void) {
	return (SBCDA_DECODER->pttdBufferCtrl & DDS_PTTDCTRL_BUFFER_EMPTY); //&& !(st & DDS_DETECTCTRL_DETECT_EOP));
}

/**
 * DDS_POP_DetectBuffer
 *
 * This function performs a READ operation on the
 * detection data buffer. Each READ pops one element
 * of the data fifo, updating the control register
 */
uint32_t DDS_ReadDetectBuffer(void) {
	return SBCDA_DECODER->detectBufferData;
}

/**
 * DDS_SetAmpThreshold
 */
void DDS_SetAmpThreshold(uint32_t val) {
	SBCDA_DECODER->detectThreshold = val;
}

/**
 * Returns the correspondent data portion size in bits
 * for a given message length code.
 * OBS: Assuming a 28bit ID_NUMBER format
 */
uint16_t DDS_CalcMessageLength(uint8_t msgLength) {

	switch (msgLength) {
	case 0:
		return 24 /*24 bits*/;
		break;
	case 3:
		return 56 /*56 bits*/;
		break;
	case 5:
		return 88 /*88 bits*/;
		break;
	case 6:
		return 120 /*120 bits*/;
		break;
	case 9:
		return 152 /*152 bits*/;
		break;
	case 10:
		return 184 /*184 bits*/;
		break;
	case 12:
		return 216 /*216 bits*/;
		break;
	case 15:
		return 248 /*248 bits*/;
		break;
	default:
		return 248 /*248 bits*/;
		break;
	}
}
/**
 * Every symbol is a 10bit value in 2's complement format. However,
 * they are placed into a 16bit unsigned int with no sign extension.
 * This function sign extends the symbol values and then compare
 * them in order to determine if they correspond to a 1 or a 0 bit.
 * The following convention is assumed:
 *
 * if symb0 < symb1 => (-1) to (1) transition => bit 1
 * if symb0 >= symb1 =>  (1) to (-1) transition => bit 0
 *
 */
uint32_t patternA, patternB;

uint8_t DDS_DecodeBit(uint16_t symb0, uint16_t symb1) {
	int16_t stmp0 = 0, stmp1 = 0;

	// test the 10th bit and sign extends to 32 bit signed int (sign extend)
	stmp0 = symb0 & 0x0200 ? (0xFFFFFC00 | symb0) : (0x000003FF & symb0);
	stmp1 = symb1 & 0x0200 ? (0xFFFFFC00 | symb1) : (0x000003FF & symb1);

	if (stmp0 > stmp1) { // bit 1
		return 1;
	} else {
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Functions related to the DECODING process
void DDS_Frame_Synch(PTTPackage_Typedef * wpckg, uint16_t pttd_symbol) {


	//PTT_Synch_FSM(wpckg, pttd_symbol);
	if (wpckg->total_symbol_cnt > DDS_MAX_SYNCH_SYMBOLS) { // synch failed
		wpckg->synch_state = PTT_SYNCH_FAILED;
		wpckg->status = PTT_ERROR;
//		uint8_t bf[100];
//		sprintf(bf, "\n\r> FRAME_SYNCH ERROR: PatternA: 0x%08x ; PatternB: 0x%08x\n\r",
//				wpckg->synch_patternA, wpckg->synch_patternB);
//		print(bf);
	} else {

		wpckg->symb_array[0] = wpckg->symb_array[1]; // previous
		wpckg->symb_array[1] = pttd_symbol; // current
		patternA = wpckg->synch_patternA;
		patternB = wpckg->synch_patternB;
		uint8_t decoded_bit = DDS_DecodeBit(wpckg->symb_array[0], wpckg->symb_array[1]);
		if (wpckg->total_symbol_cnt >= 2){
			if ((wpckg->total_symbol_cnt % 2) == 0) {
				wpckg->synch_patternA = (wpckg->synch_patternA) << 1;
				if (decoded_bit) { // bit 1 detected
					wpckg->synch_patternA = (wpckg->synch_patternA) | 0x0001;
				}
			} else {
				wpckg->synch_patternB = (wpckg->synch_patternB) << 1;
				if (decoded_bit) { // bit 1 detected
					wpckg->synch_patternB = (wpckg->synch_patternB) | 0x0001;
				}
			}
			if ((wpckg->synch_patternA & DDS_SYNCH_MIN_MASK)
					== (DDS_SYNCH_PATTERN & DDS_SYNCH_MIN_MASK)
					|| (wpckg->synch_patternB & DDS_SYNCH_MIN_MASK)
							== (DDS_SYNCH_PATTERN & DDS_SYNCH_MIN_MASK)) { // match!!!
				wpckg->status = PTT_MESSAGE_LENGTH;
				wpckg->synch_state = PTT_SYNCH_OK;
				wpckg->symb_cnt = 0;
			}
		}
	}
}
void __DDS_Frame_Synch(PTTPackage_Typedef * wpckg, uint16_t pttd_symbol) {

	//PTT_Synch_FSM(wpckg, pttd_symbol);
	if (wpckg->total_symbol_cnt > DDS_MAX_SYNCH_SYMBOLS) { // synch failed
		wpckg->synch_state = PTT_SYNCH_FAILED;
		wpckg->status = PTT_ERROR;
	} else {
		if (wpckg->synch_state == PTT_SYNCH_START) { // wait for correct frame synch start
			wpckg->symb_array[0] = wpckg->symb_array[1]; // previous
			wpckg->symb_array[1] = pttd_symbol; // current
			wpckg->symb_cnt++;

			if (DDS_DecodeBit(wpckg->symb_array[0], wpckg->symb_array[1])) { // bit 1 detected
				wpckg->synch_patternA = (wpckg->synch_patternA) << 1;
				wpckg->synch_patternA = (wpckg->synch_patternA) | 0x0001;
				wpckg->synch_state = PTT_SYNCH_BUSY;
				wpckg->symb_array[0] = 0;
				wpckg->symb_array[1] = 0;
				wpckg->symb_cnt = 0;
			}

		} else if (wpckg->synch_state == PTT_SYNCH_BUSY) { // transform symbols into bits
			if (wpckg->symb_cnt < 2) {
				wpckg->symb_array[wpckg->symb_cnt++] = pttd_symbol; // just accumulate 8 symbols for now
			}
			if (wpckg->symb_cnt == 2) {
				wpckg->synch_patternA = (wpckg->synch_patternA) << 1;
				if (DDS_DecodeBit(wpckg->symb_array[0], wpckg->symb_array[1])) { // bit 1 detected

					wpckg->synch_patternA = (wpckg->synch_patternA) | 0x0001;

				}
				wpckg->symb_cnt = 0;
				wpckg->symb_array[0] = 0;
				wpckg->symb_array[1] = 0;
			}
			if ((wpckg->synch_patternA & DDS_SYNCH_MIN_MASK)
					== (DDS_SYNCH_PATTERN & DDS_SYNCH_MIN_MASK)) { // match!!!
				wpckg->status = PTT_MESSAGE_LENGTH;
				wpckg->synch_state = PTT_SYNCH_OK;
				wpckg->symb_cnt = 0;
			}
		}
	}

}

void DDS_Read_MessageLength(PTTPackage_Typedef * wpckg, uint16_t pttd_symbol) {

	uint8_t i0;
	if (wpckg->symb_cnt < DDS_MAX_MSG_LENGTH_SYMBOLS) {
		wpckg->symb_array[wpckg->symb_cnt++] = pttd_symbol; // just accumulate 8 symbols for now
	}

	if (wpckg->symb_cnt == DDS_MAX_MSG_LENGTH_SYMBOLS) { // all message length symbols were received
		wpckg->msgLength = 0;
		for (i0 = 0; i0 < DDS_MAX_MSG_LENGTH_SYMBOLS; i0 += 2) {
			wpckg->msgLength = (wpckg->msgLength) << 1; // will shifted 4x to the left
			if (DDS_DecodeBit(wpckg->symb_array[i0],
					wpckg->symb_array[i0 + 1])) { // bit 1
				wpckg->msgLength = (wpckg->msgLength) | 0x0001;
			}

		}
		wpckg->symb_cnt = 0;
		wpckg->bit_cnt = 0;
		wpckg->status = PTT_ID_NUMBER;
		wpckg->pttIdNumber = 0;
	}
}

void DDS_Read_IDNumber(PTTPackage_Typedef * wpckg, uint16_t pttd_symbol) {

	uint8_t i0;
	if (wpckg->symb_cnt < DDS_MAX_SYMBOLS_PER_WINDOW) {
		wpckg->symb_array[wpckg->symb_cnt++] = pttd_symbol; // just accumulate 8 symbols for now
	}

	if (wpckg->symb_cnt == DDS_MAX_SYMBOLS_PER_WINDOW) { // all message length symbols were received
		for (i0 = 0; i0 < DDS_MAX_SYMBOLS_PER_WINDOW; i0 += 2) {
			wpckg->pttIdNumber = (wpckg->pttIdNumber) << 1; // will shifted 4x to the left
			if (DDS_DecodeBit(wpckg->symb_array[i0],
					wpckg->symb_array[i0 + 1])) { // bit 1
				wpckg->pttIdNumber = (wpckg->pttIdNumber) | 0x01;
			}
		}
		wpckg->bit_cnt += 4;
		wpckg->symb_cnt = 0;
		if (wpckg->bit_cnt >= DDS_MAX_IDNUMBER_BITS) {
			wpckg->bit_cnt = 0;
			wpckg->status = PTT_DATA;
		}
	}

}

void DDS_Read_Data(PTTPackage_Typedef * wpckg, uint16_t pttd_symbol) {

	uint8_t i0;
	uint32_t tmp0;

	if (wpckg->symb_cnt < DDS_MAX_SYMBOLS_PER_WINDOW) {
		wpckg->symb_array[wpckg->symb_cnt++] = pttd_symbol; // just accumulate 8 symbols for now
	}
	if (wpckg->symb_cnt == DDS_MAX_SYMBOLS_PER_WINDOW) { // all message length symbols were received

		for (i0 = 0; i0 < DDS_MAX_SYMBOLS_PER_WINDOW; i0 += 2) {

			// bit_cnt>>3 == bit_cnt / 8 => gives the byte position on the stream
			tmp0 = wpckg->sensorData[(wpckg->bit_cnt) >> 3];
			tmp0 <<= 1;
			if (DDS_DecodeBit(wpckg->symb_array[i0],
					wpckg->symb_array[i0 + 1])) { // bit 1
				tmp0 = tmp0 | 0x01;
			}
			wpckg->sensorData[(wpckg->bit_cnt) >> 3] = tmp0;

			wpckg->bit_cnt++;
		}
		wpckg->symb_cnt = 0;
	}
	if (wpckg->bit_cnt >= DDS_CalcMessageLength(wpckg->msgLength)) {
		wpckg->status = PTT_READY;
		DDS_ClearDecoder(wpckg->decoderNumber); // stop decoder immediately
	}

}
/////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * DDS_Insert_Detected_Freq
 * This procedure tries to insert a new detected freq/amp pair into
 * the current frequencies vector. New frequencies must be at least
 * 3.2KHz apart from previously allocated ones.
 *  @param new_idx = Newly detected frequency index.
 *  @param new_amp = Newly detected frequency amp.
 *  @param assigned_position    = Number of the decoder assigned for the detected frequency
 *
 *
 *  @return DDS_INSERT_FREQ_NEW: new frequency assigned to a free decoder position
 *  @return DDS_INSERT_FREQ_REPLACE: new freq replace a previously assigned decoder
 *  @return DDS_INSERT_FREQ_NONE: new freq could not be insterted because all decoders where busy.
 */
uint8_t DDS_Insert_Detected_Freq(uint32_t newIdx, uint32_t newAmp,
		uint8_t * assigned_decoder) {

	/**
	 * Frequency idx have are 11 bits long with a precision
	 * of 128/2^11 kHz. The idx are signed and organized around
	 * zero (FFT result) and go from -1024 to +1023 (2^11) in
	 * 2's-complement format. However, after read the idx values
	 * are stored on 32bit unsigned integer variables, whithout
	 * sign extension.
	 *
	 * Step resolution: 	0,0625 kHz
	 * Overlap bandwidth: 	3.2 kHz = 51.2 * 0,0625 kHz
	 *
	 * Each new detected peak must be at least 51 freqIdxes apart
	 * (left and right)
	 */
	int32_t i, snew_idx = 0, idx_min_distance = DDS_FREQ_GUARD_BAND,
			old_idx = 0, tmp1 = 0, insert_idx = 0, lower_limit = 0,
			upper_limit = 0;
	uint8_t ret_value = DDS_INSERT_FREQ_NONE;

	*assigned_decoder = DDS_INSERT_FREQ_INVALID;
	for (i = 0; i < DDS_NUMBER_OF_DECODERS; i++) {

		old_idx = DDS_DETECTED_FREQS[i].freq_idx; // freq is unsigned but occupies only 11 bits of the 32 integer, no sign extent is necessary
		if (old_idx == 0) {
			*assigned_decoder = i; // free spot
		} else {
			// test replace condition
			snew_idx = (int) newIdx;
			if (TEST_SIGN_BIT(newIdx, DDS_FREQ_NUMBER_OF_BITS)) { // test sign bit (11th)
				snew_idx = CONVERT_TO_32BIT_SIGNED(newIdx,
						DDS_FREQ_NUMBER_OF_BITS); // sign extent
				// now its in 2's complement 32bit
			}
			if (TEST_SIGN_BIT(old_idx, DDS_FREQ_NUMBER_OF_BITS)) { // test sign bit (11th)
				old_idx = CONVERT_TO_32BIT_SIGNED(old_idx,
						DDS_FREQ_NUMBER_OF_BITS); // sign extent
				// now its in 2's complement 32bit
			}
			lower_limit = (old_idx - idx_min_distance);
			upper_limit = (old_idx + idx_min_distance);

			if (lower_limit < -1024) {
				lower_limit = -1024; // overload
			}
			if (upper_limit > 1023) {
				upper_limit = 1023;
			}
			if ((snew_idx >= lower_limit && snew_idx <= upper_limit)) { // replace conflicting idx
				if (newAmp >= DDS_DETECTED_FREQS[i].freq_amp) {
					if (snew_idx == old_idx) { // double detection
						*assigned_decoder = i;
						if (DDS_DETECTED_FREQS[*assigned_decoder].detect_state
								!= FREQ_DECODING) {
							DDS_DETECTED_FREQS[*assigned_decoder].detect_state =
									FREQ_DETECTED_TWICE;
							DDS_DETECTED_FREQS[*assigned_decoder].timeout =
							DDS_FREQUENCY_TIMEOUT;
						}else{
							*assigned_decoder = DDS_INSERT_FREQ_INVALID;
						}
					} else {
						/*
						 * Freq idx is replaced if the new index is less then
						 * 3,2kHz apart AND the new_amp is higher then
						 * the previous one.
						 * The replacement can only happen if the
						 * replaced freq is not decoding yet.
						 */
						if (DDS_DETECTED_FREQS[i].detect_state
								!= FREQ_DECODING) {
							*assigned_decoder = i;
							DDS_DETECTED_FREQS[i].detect_state = FREQ_REPLACED;
							DDS_DETECTED_FREQS[i].timeout =
							DDS_FREQUENCY_TIMEOUT;
						}else{
							*assigned_decoder = DDS_INSERT_FREQ_INVALID;
						}
					}
					/**
					 * Found a frequency that is:
					 * 1 - within the safe guard band of
					 * another allocated freq;
					 */
					break; // exit the loop;
				} else {
					*assigned_decoder = DDS_INSERT_FREQ_INVALID;
					return DDS_INSERT_FREQ_DISCARD;
				}
			}
		}
	}
	if (*assigned_decoder != DDS_INSERT_FREQ_INVALID) {
		if (DDS_DETECTED_FREQS[*assigned_decoder].detect_state
				!= FREQ_DECODING) {
			DDS_DETECTED_FREQS[*assigned_decoder].freq_amp = newAmp;
			DDS_DETECTED_FREQS[*assigned_decoder].freq_idx = newIdx;
			if (DDS_DETECTED_FREQS[*assigned_decoder].detect_state
					== FREQ_DETECTED_TWICE) { // double detection
				return DDS_INSERT_FREQ_NEW;
			} else if (DDS_DETECTED_FREQS[*assigned_decoder].detect_state
					== FREQ_REPLACED) {
				return DDS_INSERT_FREQ_REPLACE;
			} else if(DDS_DETECTED_FREQS[*assigned_decoder].detect_state
					== FREQ_DETECTED_ONCE){
				// do nothing
			}else{
				DDS_DETECTED_FREQS[*assigned_decoder].detect_state =
						FREQ_DETECTED_ONCE;
				DDS_DETECTED_FREQS[*assigned_decoder].timeout =
				DDS_FREQUENCY_TIMEOUT;
			}
		}else{
			*assigned_decoder = DDS_INSERT_FREQ_INVALID;
		}
	}
	return DDS_INSERT_FREQ_NONE;
}

/**
 * DDS_Update_Freq_Timeout
 */
void DDS_Update_Freq_Timeout(void) {
	int i = 0;
	PTTPackage_Typedef * wpckg;
	for (i = 0; i < DDS_NUMBER_OF_DECODERS; i++) {

		if (DDS_DETECTED_FREQS[i].timeout == 0) { // frequency timeout
			DDS_DETECTED_FREQS[i].freq_amp = 0;
			DDS_DETECTED_FREQS[i].freq_idx = 0;
			DDS_DETECTED_FREQS[i].detect_state = FREQ_NONE;

			wpckg = working_packages[i];
			if (wpckg != 0) {
				/**
				 * When previously assigned is replaced, it must leave space
				 * for the the new detected one. The previous decoding process
				 * must be stopped and its corresponding memory must be
				 * cleaned. The cleaning process will be performed
				 * during the Decoding_Loop when the package is
				 * marked as PTT_ERROR.
				 */
				DDS_ClearDecoder(i);
				working_packages[i] = 0;
				currently_available_decoders++;
				currently_working_packages--;
				decoding_fails_cnt++;
				wpckg->status = PTT_FREE;
				PTT_ResetPackage(wpckg);
				PTT_PushFreePackage(wpckg); // sent back to free stack
				SBCDA_DECODER->initFreq[i] = 0; // assigns it here to save time
			}
		} else {
			DDS_DETECTED_FREQS[i].timeout--;
		}
	}
}

/*
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
uint32_t DDS_Detection_Loop() {

	uint32_t tmp0, tmp_freq, tmp_amp;
	uint8_t new_detected_freqs = 0;
	int8_t assigned_decoder = 0;
	PTTPackage_Typedef * wpckg;

	DDS_Update_Freq_Timeout();

	// 1 - Detect new frequencies
	// In every iteration it must check if new frequencies have
	// been detected  and include them in the decoding queue if
	// there are still available decoder units
	while (!DDS_DetectEop() || !DDS_DetectEmpty()) { // the detection phase is indicated by the detectEOP signal
		while (!DDS_DetectEmpty()) { // while not empty read new frequencies
			tmp0 = SBCDA_DECODER->detectBufferData; // read fifo
			tmp_freq = DDS_DETECTDATA_DETECT_FREQ_VALUE(tmp0);
			tmp_amp = DDS_DETECTDATA_DETECT_AMP_VALUE(tmp0);

			tmp0 = DDS_Insert_Detected_Freq(tmp_freq, tmp_amp,
					&assigned_decoder);

			if (tmp0 == DDS_INSERT_FREQ_NEW) { // update active list
				wpckg = PTT_PopFreePackage();
				if (wpckg != 0) {
					PTT_ResetPackage(wpckg);
					wpckg->status = PTT_FRAME_SYNCH;
					wpckg->synch_state = PTT_SYNCH_START;
					wpckg->decoderNumber = assigned_decoder;
					working_packages[assigned_decoder] = wpckg;
					SBCDA_DECODER->initFreq[assigned_decoder] = tmp_freq; // assigns it here to save time
					DDS_SetDecoder(assigned_decoder);

					new_detected_freqs++;
					currently_working_packages++;
					currently_available_decoders--;
					DDS_DETECTED_FREQS[assigned_decoder].detect_state =
							FREQ_DECODING;
				} else { // frequency detected but there are no free ptt_packages
					DDS_DETECTED_FREQS[assigned_decoder].detect_state =
							FREQ_NONE;
					DDS_DETECTED_FREQS[assigned_decoder].freq_amp = 0;
					DDS_DETECTED_FREQS[assigned_decoder].freq_idx = 0;
					DDS_DETECTED_FREQS[assigned_decoder].timeout = 0;
					discarded_detection_cnt++;
				}
			} else if (tmp0 == DDS_INSERT_FREQ_REPLACE) {
				wpckg = working_packages[assigned_decoder];
				if (wpckg != 0) {
					/**
					 * When previously assigned is replaced, it must leave space
					 * for the the new detected one. The previous decoding process
					 * must be stopped and its corresponding memory must be
					 * cleaned. The cleaning process will be performed
					 * during the Decoding_Loop when the package is
					 * marked as PTT_ERROR.
					 */
					wpckg->status = PTT_ERROR;
					replace_freqs_cnt++;
				} else {
					/**
					 * In this case the frequency was detected once and
					 * replaced another frequency that was also detected
					 * once. In the next iteration this frequency shall
					 * be 'detected_twice' and will be treated as a new
					 * detection.
					 */
					// NOTHING
				}

			} else { // frequency detected but all decoders were occupied;
				discarded_detection_cnt++;
			}
			total_detections_cnt++;
		}
	}
	return new_detected_freqs;
}

/*
 * DDS_Decoding_Loop
 *
 * The decoding loop is the procedure responsible for reading
 * symbols out of the PTTD Buffer FIFO, converting them into
 * bit representation and attaching each bit to its corresponding
 * building package.
 *
 * @return The number of finished packages in the current loop
 */

uint32_t DDS_Decoding_Loop() {

	mss_rtc_calendar_t calendar;
	uint32_t pttd_ctrl = 0, pttd_data0 = 0, pttd_data1 = 0;
	uint16_t pttd_symbol = 0;
	uint8_t pttd_ch = 0, pttd_vga_mant, pttd_vga_exp = 0;
	uint32_t new_ready_packages = 0;

	PTTPackage_Typedef * wpckg;

	while (!DDS_DecoderEop() || !DDS_DecoderEmpty()) {
		// - Now all the active decoder will spit
		// symbols through the PTT_BUFFER_FIFO
		// - All the symbols before the symb_lock are discarded for
		// that channel.
		// - After the symb_lock signal rises, the symbols
		// must be converted to bits and compared to the frame_synch word.
		// - Finally,after the frame synchronization, the package building starts
		while (!DDS_DecoderEmpty()) {
			pttd_data0 = SBCDA_DECODER->pttdBufferData0;
			pttd_data1 = SBCDA_DECODER->pttdBufferData1; /** fifo pop on this load **/
			pttd_ch = DDS_PTTD1_CH(pttd_data1);
			pttd_symbol = DDS_PTTD1_SYMB_VALUE(pttd_data1);
			wpckg = working_packages[pttd_ch];
			if (wpckg != 0) {
				if (wpckg->status == PTT_ERROR) {
					if ((wpckg->total_symbol_cnt % DDS_MAX_SYMBOLS_PER_WINDOW)
							== 0) {
						// a decoder can only bee freed when all the symbols for
						// that window have been consumed
						SBCDA_DECODER->initFreq[pttd_ch] = 0; // assigns it here to save time
						DDS_ClearDecoder(pttd_ch);
						working_packages[pttd_ch] = 0;
						currently_available_decoders++;
						currently_working_packages--;

						DDS_DETECTED_FREQS[pttd_ch].freq_idx = 0;
						DDS_DETECTED_FREQS[pttd_ch].freq_amp = 0;
						DDS_DETECTED_FREQS[pttd_ch].detect_state = FREQ_NONE;
						DDS_DETECTED_FREQS[pttd_ch].timeout = 0;

						decoding_fails_cnt++;
						wpckg->status = PTT_FREE;
						PTT_ResetPackage(wpckg);
						PTT_PushFreePackage(wpckg); // sent back to free stack
					}
				} else if ((pttd_data1 & DDS_PTTD1_SYMB_LOCK)) {
					//pttd_symbol = DDS_PTTD1_SYMB_VALUE(pttd_data1);
					wpckg->total_symbol_cnt++;
					// Step 1: frame synch
					if (wpckg->status == PTT_FRAME_SYNCH) { // frame synch length
						DDS_Frame_Synch(wpckg, pttd_symbol);
					} // Step 2: recover message length
					else if (wpckg->status == PTT_MESSAGE_LENGTH) {
						// 1 - recover message length
						DDS_Read_MessageLength(wpckg, pttd_symbol);
					} // Step 3: recover id number
					else if (wpckg->status == PTT_ID_NUMBER) {
						DDS_Read_IDNumber(wpckg, pttd_symbol);
					} // Step 4: recover ptt id number
					else if (wpckg->status == PTT_DATA) {
						DDS_Read_Data(wpckg, pttd_symbol);
						if (wpckg->status == PTT_READY) {
							// record frequency
							if (RTC_Is_Initialized()) {
								RTC_Get(&calendar);
								wpckg->timeTag[7] = calendar.week;
								wpckg->timeTag[6] = calendar.weekday;
								wpckg->timeTag[5] = calendar.second;
								wpckg->timeTag[4] = calendar.minute;
								wpckg->timeTag[3] = calendar.hour;
								wpckg->timeTag[2] = calendar.day;
								wpckg->timeTag[1] = calendar.month;
								wpckg->timeTag[0] = calendar.year;
							}

							if (wpckg->freqMeasure == 0) {
								wpckg->freqMeasure =
										DDS_DETECTED_FREQS[pttd_ch].freq_idx;
								wpckg->ampRms =
										DDS_DETECTED_FREQS[pttd_ch].freq_amp;
							}
							wpckg->decoderNumber = pttd_ch;
							// calc CRC over the data bytes
							wpckg->crc = calcCRC(wpckg->sensorData,
							PTT_SENSOR_DATA_OCTETS_MAX_LENGTH);

							SBCDA_DECODER->initFreq[pttd_ch] = 0; // assigns it here to save time
							DDS_ClearDecoder(pttd_ch);

							DDS_DETECTED_FREQS[pttd_ch].freq_idx = 0;
							DDS_DETECTED_FREQS[pttd_ch].freq_amp = 0;
							DDS_DETECTED_FREQS[pttd_ch].detect_state =
									FREQ_NONE;
							DDS_DETECTED_FREQS[pttd_ch].timeout = 0;

							working_packages[pttd_ch] = 0;
							currently_available_decoders++;
							currently_working_packages--;
							ready_packages_cnt++;
							new_ready_packages++;
							PTT_PushReadyPackage(wpckg);
						}
					}
				}
			}
		}
	}
	return new_ready_packages;
}
