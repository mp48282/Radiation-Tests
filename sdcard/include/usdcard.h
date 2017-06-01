/*
 * usdcard.h
 *
 *  Created on: 26/11/2015
 *      Author: ADMIN
 */

#include "spi.h"
#include "gio.h"
#include "error.h"
#include "het.h"
#include "hal_stdtypes.h"

#ifndef CONASAT_INCLUDE_USDCARD_H_
#define CONASAT_INCLUDE_USDCARD_H_

typedef struct
{
	uint8 command;
	uint8 rsp_size;
}
cmd_rsp_lkup;

#define USD_CDM_SIZE 6U

//
//
//
#define USD_TYPE_UNDEFINED  	 0x00
#define USD_TYPE_NOT_A_CARD 	 0x01
#define USD_TYPE_VER_1_SD        0x02
#define USD_TYPE_VER_2_SDHC_SDXC 0x03
#define USD_TYPE_VER_2_SDSC      0x04

//
//
//
#define USD_CMD0_GO_IDLE_STATE      		 0x00
#define USD_CMD1_SEND_OP_COND       		 0x01
#define USD_CMD8_SEND_IF_COND				 0x08
#define USD_CMD13_SEND_STATUS       		 0x0D
#define USD_CMD16_SET_BLOCKLEN      		 0x10
#define USD_CMD17_READ_SINGLE_BLOCK 		 0x11
#define USD_CMD24_WRITE_BLOCK       		 0x18
#define USD_CMD32_ERASE_WR_BLK_START_ADDRESS 0x20
#define USD_CMD33_ERASE_WR_BLK_END_ADDRESS   0x21
#define USD_CMD38_ERASE 					 0x26
#define USD_ACMD41_SD_SEND_OP_COND			 0x29
#define USD_CMD55_APP_CMD					 0x37
#define USD_CMD58_READ_OCR        			 0x3A

/**
 * 	@brief Initialize the uSDCARD sending the correct command sequence and indetify its type
 *
 *  At the end of the initialization process the SPI1 clock is raised to 25MHz for maximum speed
 *  in the transactions.
 *
 *  @return SUCCESS - Card initialized with success.
 *  		USD_ERROR_READ_OCR -
 *  		USD_ERROR_IDLE_STATE -
 *  		USD_ERROR_SEND_IF_COND -
 *  		USD_ERROR_SD_SEND_OP_COND -
 *  		USD_ERROR_READ_OCR -
 *  		USD_OLD_VERSION_NOT_CARD -
 */
uint8 usd_init();

/**
 * 	@brief Write a buffer with 512 bytes in a sector.
 *
 *	@param data: An array of data with 512 bytes.
 *	@param blkaddr - An integer identifying the sector to be written.
 *
 *  @return SUCCESS -
 *  		USD_ERROR_WRITE -
 */
uint8 usd_write_block(uint16* data, uint32 blkaddr);

/**
 * 	@brief Reads a buffer with 512 bytes from a sector.
 *
 *	@param data: A pointer to an array of bytes to store the reading.
 *	@param blkaddr - An integer identifying the sector to be read.
 *
 *  @return SUCCESS -
 *  		USD_ERROR_READ -
 */
uint8 usd_read_block(uint16* data, uint32 blkaddr);

/**
 * 	@brief Reads a buffer with 512 bytes from a sector.
 *
 *	@param data: A pointer to an array of bytes to store the reading.
 *	@param blkaddr - An integer identifying the sector to be read.
 *
 *  @return SUCCESS -
 *  		USD_ERROR_READ -
 */
uint8 usd_erase_blocks(uint32 blkaddr_start, uint32 blkaddr_stop);

/**
 * 	@brief Enable the Chip Select (CS0) in SPI1 module to activate the uSDCARD.
 *
 *  @return This function returns nothing.
 */
void usd_spi_enable_card(void);

/**
 * 	@brief Disabel the Chip Select (CS0) in SPI1 module to activate the uSDCARD.
 *
 *  @return This function returns nothing.
 */
void usd_spi_disable_card(void);

uint16 usd_send_command(uint8 command, uint32 argument);
uint32 usd_check_card_presence(void);
#endif /* CONASAT_INCLUDE_USDCARD_H_ */
