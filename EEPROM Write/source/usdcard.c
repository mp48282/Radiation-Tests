/**
 *	\file motor.c
 *	\brief Implementation of the functions used to write, read and erase sectors of the card.
 *	\author Davyd Melo
 */

#include "usdcard.h"

//
// SPI1 configuration parameters (see HALCOGEN configuration on SPI1 peripheral).
//
spiDAT1_t usd_dtconf = { .CS_HOLD = TRUE, .WDEL = FALSE, .DFSEL = SPI_FMT_0, .CSNR = SPI_CS_0 };

//
// Stores the card type after the initialization process.
//
uint8 sdtype = 0;

uint8 usd_init()
{
	uint16 i;
	uint16 r1 = 0x00FF;
	uint16 buffer[] = { 0xFFFF };
	uint16 rspbuffer[] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };

	if (usd_check_card_presence())
	{
		return USD_ERROR_CARD_NOT_DETECTED;
	}

	// 128 pulses of clock with CS = 1
	gioSetBit(spiPORT1, 0, 1);
	for (i = 0; i < 16; i++)
	{
		spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
	}

	// CMD0 with CS = 0 (CMD0+)
	usd_spi_enable_card();
	r1 = usd_send_command(USD_CMD0_GO_IDLE_STATE, 0x00);

	// Try to read the correct response for CMD0 command hundred times
	for (i = 0; i < 100; i++)
	{
		if (r1 == 0x0001) {	break; }
		spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);
	}

	// If the response is not the expected disable the SPI and return a error state
	if (r1 != 0x0001)
	{
		usd_spi_disable_card();
		return USD_ERROR_IDLE_STATE;
	}

	// CMD8 - This command asks to the card if it can operate on the voltage range of 2.7-3.6V (0b0001)
	// If the response is not 'illegal command' the card is a Version 2.00+ card
	// Otherwise, the card is from type Ver1.X ou is not a card
	r1 = usd_send_command(USD_CMD8_SEND_IF_COND, 0x000001AA);

	// Try to read the correct response for CMD8 command hundred times
	for (i = 0; i < 100; i++)
	{
		if (r1 == 0x0001) { break; }
		spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);
	}

	// If CMD8 command is successful  => Card from type Ver2.00+
	if (r1 == 0x0001)
	{
		uint8 j;

		// Read the remaining 4 bytes from the R7 response (total 40 bits).
		for (j = 0; j < 4; ++j)
		{
			spiReceiveData(spiREG1, &usd_dtconf, 1U, (rspbuffer + j));
		}

		// Verifies if the card response contains the supply voltage.
		// Checks also the echo of the check pattern.
		// In positive case continue to send commands.
		if (rspbuffer[2] != 0x01 || rspbuffer[3] != 0xAA)
		{
			usd_spi_disable_card();
			return USD_ERROR_SEND_IF_COND;
		}

		i = 1000; // Init the timout counter
		r1 = 0x00FF; // Cleans the response variable

		// CMD55 e ACMD41 Commands
		// CMD55 - Next command will be an application specific command
		// ACMD41 - Operation Conditions
		do
		{
			r1 = usd_send_command(USD_CMD55_APP_CMD, 0);
			r1 = usd_send_command(USD_ACMD41_SD_SEND_OP_COND, 0);

			// If response is successful
			if (r1 == 0x0000)
			{
				break;
			}
		}
		while(r1 != 0x0000 && --i > 0);

		// ACMD41 Timeout, disable the card and return an error code
		if (i == 0)
		{
			usd_spi_disable_card();
			return USD_ERROR_SD_SEND_OP_COND;
		}

		// CMD58 - Reads the OCR register (Operation Conditions Register).
		// The response type is R3 [R1 (1 byte) + OCR (4 bytes)]
		r1 = usd_send_command(USD_CMD58_READ_OCR, 0);

		// The CMD58 response was successful
		if (r1 == 0)
		{
			// Reading the last 4 bytes from R3 response
			for (j = 0; j < 4; ++j)
			{
				spiReceiveData(spiREG1, &usd_dtconf, 1U, (rspbuffer + j));
			}

			// Verifying the state of CCS (Card Capacity Status) bit.
			// If it is on we have a SDHC or SDXC card.
			if (rspbuffer[0] & (1<<6))
			{
				sdtype = USD_TYPE_VER_2_SDHC_SDXC;
			}
			// If the CSS is off we have a SDSC card
			else
			{
				sdtype = USD_TYPE_VER_2_SDSC;
			}
		}
		// The response failed
		else
		{
			usd_spi_disable_card();
			return USD_ERROR_READ_OCR;
		}
	}
	// CMD8 failed  => Card type Ver1.XX or is not card
	else
	{
		// Return a error state
		return USD_ERROR_OLD_VERSION_NOT_CARD;
	}

	// Raises the clock frequency (1MHz)
	spiREG1->FMT0 = spiREG1->FMT0 | (uint32)((uint32)10U << 8U);

	// Finishes the transaction with CS = 1;
	gioSetBit(spiPORT1, 0, 1);

	return SUCCESS;
}

uint16 usd_send_command(uint8 cmd, uint32 arg)
{
	uint16 j;
	uint16 buffer[] = { 0x0000 };

	switch(cmd)
	{
		case USD_CMD0_GO_IDLE_STATE:
			buffer[0] = 0x0040;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0095;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer); // pre-calculated crc
		break;

		case USD_CMD8_SEND_IF_COND:
			buffer[0] = 0x0048;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0001;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x00AA;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0087;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer); // pre-calculated crc
		break;

		case USD_CMD55_APP_CMD:
			buffer[0] = 0x0077;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x00FF;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
		break;

		case USD_ACMD41_SD_SEND_OP_COND:
			buffer[0] = 0x0069;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0040;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x00FF;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
		break;

		case USD_CMD58_READ_OCR:
			buffer[0] = 0x007A;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x00FF;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
		break;

		case USD_CMD13_SEND_STATUS:
			buffer[0] = 0x004D;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x0000;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x00FF;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
		break;

		case USD_CMD24_WRITE_BLOCK:
			buffer[0] = 0x0058;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0xFF000000) >> 24; spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x00FF0000) >> 16; spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x0000FF00) >> 8;  spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x000000FF); 	  spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x00FF;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
		break;

		case USD_CMD17_READ_SINGLE_BLOCK:
			buffer[0] = 0x0051;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0xFF000000) >> 24; spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x00FF0000) >> 16; spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x0000FF00) >> 8;  spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x000000FF); 	  spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x00FF;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
		break;

		case USD_CMD32_ERASE_WR_BLK_START_ADDRESS:
			buffer[0] = 0x0060;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0xFF000000) >> 24; spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x00FF0000) >> 16; spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x0000FF00) >> 8;  spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x000000FF); 	  spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x00FF;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
		break;

		case USD_CMD33_ERASE_WR_BLK_END_ADDRESS:
			buffer[0] = 0x0061;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0xFF000000) >> 24; spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x00FF0000) >> 16; spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x0000FF00) >> 8;  spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x000000FF); 	  spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x00FF;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
		break;

		case USD_CMD38_ERASE:
			buffer[0] = 0x0066;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0xFF000000) >> 24; spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x00FF0000) >> 16; spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x0000FF00) >> 8;  spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = (arg & 0x000000FF); 	  spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
			buffer[0] = 0x00FF;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
		break;
	}

	for (j = 0; j < 8; ++j)
	{
		spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);

		if (buffer[0] != 0x00FF)
		{
			break;
		}
	}

	return buffer[0];
}

uint8 usd_write_block(uint16* data, uint32 blkaddr)
{
	uint16 i;
	uint16 buffer[] = { 0x0000 };
	uint16 r1 = 0x00FF;

	// Enables the card (CS = 0)
	usd_spi_enable_card();

	// CMD24 - Send a command to write a block
	r1 = usd_send_command(USD_CMD24_WRITE_BLOCK, blkaddr);

	// If the CMD24 fails, disable CS and returns an error code
	if (r1 != 0)
	{
		usd_spi_disable_card();
		return 1;
	}

	// Data Token transmission. Its used in the commands CMD17/CMD18/CMD24
	buffer[0] = 0x00FE;	spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);

	// Send the data to the card
	spiTransmitData(spiREG1, &usd_dtconf, 512U, data);

	// Sends two dummy bytes for the CRC
	buffer[0] = 0x00FF; spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);
	buffer[0] = 0x00FF; spiTransmitData(spiREG1, &usd_dtconf, 1U, buffer);

	// Reads the Response Token
	spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);

	// Checks the Response Token to verify if the data was accepted.
	// If not, disable the card and return an error code.
	if (((buffer[0] & 0x000E) >> 1) != 0x0002)
	{
		usd_spi_disable_card();
		return 1;
	}

	/////////////////////////////

	// Wait the card to leave the busy state
	i = 0xFFFF; // Timeout variable
	do
	{
		spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);
	}
	while(buffer[0] == 0x0000 && --i);

	// Writing timout. Disables the card and returns an error code
	if (i == 0)
	{
		usd_spi_disable_card();
		return 1;
	}

	// Disables the card (CS = 1)
	usd_spi_disable_card();

	return SUCCESS;
}

uint8 usd_read_block(uint16* data, uint32 blkaddr)
{
	uint16 i;
	uint16 buffer[] = { 0x0000 };
	uint16 r1 = 0x00FF, timeout;

	// Enables the card (CS = 0)
	usd_spi_enable_card();

	// CMD17 -
	r1 = usd_send_command(USD_CMD17_READ_SINGLE_BLOCK, blkaddr);

	if (r1 != 0)
	{
		usd_spi_disable_card();
		return 1;
	}

	// Wait to receive the Start Block Token
	for (timeout = 1000; timeout; timeout--)
	{
		spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);

		if (buffer[0] == 0x00FE) { break; }
	}

	// If Start Block Token don't come, disable card and return an error code
	if (timeout == 0)
	{
		usd_spi_disable_card();
		return 1;
	}

	// Write the data
	for (i = 0; i < 512; i++)
	{
		spiReceiveData(spiREG1, &usd_dtconf, 1U, (data + i));
	}

	// Read two CRC bytes
	spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);
	spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);

	// Disables the card (CS = 1)
	usd_spi_disable_card();

	return SUCCESS;
}

uint8 usd_erase_blocks(uint32 blkaddr_start, uint32 blkaddr_stop)
{
	uint16 timeout;
	uint16 buffer[] = { 0x0000 };
	uint16 r1 = 0x00FF;

	// Enables the card (CS = 0)
	usd_spi_enable_card();

	// CMD32 - Command to specify the start sector
	r1 = usd_send_command(USD_CMD32_ERASE_WR_BLK_START_ADDRESS, blkaddr_start);

	// If response value is not a expected value, disable card and return an error code
	if (r1 != 0)
	{
		usd_spi_disable_card();
		return 1;
	}

	// CMD33 - Command to specify the end sector
	r1 = usd_send_command(USD_CMD33_ERASE_WR_BLK_END_ADDRESS, blkaddr_stop);

	// If response value is not a expected value, disable card and return an error code
	if (r1 != 0)
	{
		usd_spi_disable_card();
		return 1;
	}

	// CMD38 - Erase the specified range of blocks
	r1 = usd_send_command(USD_CMD38_ERASE, 0x00);

	// Wait the card to leave the busy state
	timeout = 0xFFFF; // Timeout variable
	do
	{
		spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);
	}
	while(buffer[0] == 0x0000 && --timeout);

	// Erasing timout, disable the card and return a error code
	if (timeout == 0)
	{
		usd_spi_disable_card();
		return 1;
	}

	// Disables the card (CS = 1)
	usd_spi_disable_card();

	return SUCCESS;
}

void usd_spi_enable_card()
{
	// Put the Chip Select in a LOW logic state
	gioSetBit(spiPORT1, 0, 0);
}

void usd_spi_disable_card()
{
	uint16 buffer[] = { 0x0000 };

	// Put the Chip Select in a HIGH logic state
	gioSetBit(spiPORT1, 0, 1);

	spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);
	spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);
	spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);
	spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);

	spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);
	spiReceiveData(spiREG1, &usd_dtconf, 1U, buffer);
}

uint32 usd_check_card_presence()
{
	return gioGetBit(hetPORT1, 24);
}
