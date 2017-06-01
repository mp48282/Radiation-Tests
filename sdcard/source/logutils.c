/*
 * logutils.c
 *
 *  Created on: 13 de dez de 2016
 *      Author: Jamile
 */


#include "logutils.h"
#include "usdcard.h"
#include <stdio.h>

//TODO Precisa ser dinâmico (buffer)?

int write_usdcard(char loginfo[], int addr){
	// if retv = 1, write error

	uint16_t buffer[512], i;
	int retv = -1, length = 0;
	length = strlen(loginfo);
	if(length <= 512) {
		for(i = 0; i < length ; i++){
			buffer[i] = (uint16) loginfo[i];
		}
		retv = usd_write_block(buffer, addr);
	}
	return retv;
}

//TODO Write flash

//TODO Function to Printf, write sdcard and write flash
