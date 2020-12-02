/*
 * util.c
 *
 *  Created on: 13/02/2017
 *      Author: Anderson
 */

#include <stdio.h>
#include "../drivers/mss_uart/mss_uart.h"
#include "app.h"

const unsigned char CRC7_POLY = 0x91;

unsigned char calcCRC(unsigned char message[], unsigned char length) {
	unsigned char i, j, crc = 0;

	for (i = 0; i < length; i++) {
		crc ^= message[i];
		for (j = 0; j < 8; j++) {
			if (crc & 1)
				crc ^= CRC7_POLY;
			crc >>= 1;
		}
	}
	return crc;
}

void print_hex(uint8_t * buffer, uint8_t length) {
	char i = 0;
	char tmp[10];
	for (i = 0; i < length; i++) {
// seems bizarre, buf sprintf wasn't working.
// at this point. Don't know why :-)
		sprintf(tmp, "%02x ", buffer[i]);
//		tmp[0] = ' ';
//		tmp[1] = ((buffer[i] & 0xf0) >> 4) + '0';
//		tmp[2] = ((buffer[i] & 0x0f)) + '0';
//		tmp[3] = ' ';
//		tmp[4] = 0;
		print(tmp);
	}
}
void mymemcpy(uint8_t * dest, const uint8_t * org, const int length){
	int i0;
	for (i0=0; i0<length; i0++){
		dest[i0] = org[i0];
	}
}

void wait() {
	unsigned long delay = DELAY_LOAD_VALUE * 10;
	while (delay-- > 0)
		;
	delay = DELAY_LOAD_VALUE;
}


