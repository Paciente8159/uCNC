/*
	Name: mmcsd_card.c
	Description: SD card module for µCNC.
	This adds SD card support via SPI

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 08-09-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include "../softspi.h"
#include "ffconf.h"
#include "mmcsd.h"
#include "ff.h"
#include "diskio.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef MMCSD_MAX_NCR
#define MMCSD_MAX_NCR 10
#endif
#ifndef MMCSD_MAX_TIMEOUT
#define MMCSD_MAX_TIMEOUT 255
#endif
#define MMCSD_TIMEOUT (mcu_millis() + MMCSD_MAX_TIMEOUT)
#ifndef MMCSD_MAX_BUFFER_SIZE
#define MMCSD_MAX_BUFFER_SIZE 512
#endif

#ifndef SD_CARD_USE_SW_SPI
#define SD_CARD_USE_HW_SPI
#endif

#if (!defined(SD_CARD_USE_HW_SPI) || !defined(MCU_HAS_SPI))
#ifndef SD_SPI_CLK
#define SD_SPI_CLK DOUT30
#endif
#ifndef SD_SPI_SDO
#define SD_SPI_SDO DOUT29
#endif
#ifndef SD_SPI_SDI
#define SD_SPI_SDI DIN29
#endif
#ifndef SD_SPI_CS
#define SD_SPI_CS SPI_CS
#endif
SOFTSPI(mmcsd_spi, 100000UL, 0, SD_SPI_SDO, SD_SPI_SDI, SD_SPI_CLK);
#define spi_xmit(c) softspi_xmit(&mmcsd_spi, c)
#else
#ifndef SD_SPI_CLK
#define SD_SPI_CLK SPI_CLK
#endif
#ifndef SD_SPI_SDO
#define SD_SPI_SDO SPI_SDO
#endif
#ifndef SD_SPI_SDI
#define SD_SPI_SDI SPI_SDI
#endif
#ifndef SD_SPI_CS
#define SD_SPI_CS SPI_CS
#endif
#define spi_xmit(c) mcu_spi_xmit(c)
#endif

static mmcsd_card_t mmcsd_card;

void nodelay(void)
{
}

FORCEINLINE static void mmcsd_spi_speed(bool highspeed)
{
	if (highspeed)
	{
#if (!defined(SD_CARD_USE_HW_SPI) || !defined(MCU_HAS_SPI))
		mmcsd_spi.wait = &nodelay;
#else
		mcu_spi_config(0, 20000000UL);
#endif
	}
	else
	{
#if (!defined(SD_CARD_USE_HW_SPI) || !defined(MCU_HAS_SPI))
		mmcsd_spi.wait = &mmcsd_spi_wait;
#else
		mcu_spi_config(0, 100000UL);
#endif
	}
}

uint8_t mmcsd_waittoken(void)
{
	uint32_t timeout = MMCSD_TIMEOUT;
	uint8_t token = spi_xmit(0xFF);
	while (token == 0xFF && (timeout > mcu_millis()))
	{
		token = spi_xmit(0xFF);
	}

	return token;
}

bool mmcsd_waitready(void)
{
	uint32_t timeout = MMCSD_TIMEOUT;
	while (spi_xmit(0xFF) != 0xFF)
	{
		if ((timeout < mcu_millis()))
		{
			return false;
		}
	}

	return true;
}

bool mmcsd_response(uint8_t *result, uint16_t len, bool wait_token)
{
	mcu_clear_output(SD_SPI_CS);

	if (wait_token)
	{
		if (mmcsd_waittoken() != 0xFE)
		{
			mcu_set_output(SD_SPI_CS);
			memset(result, 0, len);
			return false;
		}
	}

	while (len--)
	{
		*result++ = spi_xmit(0xFF);
	}

	// discard CRC
	spi_xmit(0xFF);
	spi_xmit(0xFF);
	mcu_set_output(SD_SPI_CS);
	return true;
}

bool mmcsd_message(const uint8_t *buff, uint16_t count, uint8_t token)
{
	mcu_clear_output(SD_SPI_CS);

	do
	{
		// sends token
		if (!mmcsd_waitready())
		{
			return false;
		}

		spi_xmit(token);

		// sends data to buffer
		for (uint32_t i = 0; i < 512; i++)
		{
			spi_xmit(*buff++);
		}

		// CRC dummy
		spi_xmit(0xFF);
		spi_xmit(0xFF);
		// If not accepted, return with error
		if ((spi_xmit(0xFF) & 0x1F) != 0x05)
		{
			return false;
		}

	} while (--count);

	if (token == 0xFC)
	{
		if (!mmcsd_waitready())
		{
			return false;
		}

		spi_xmit(0xFD);
	}

	mcu_set_output(SD_SPI_CS);
	return true;
}

FORCEINLINE static uint8_t mmcsd_command(uint8_t cmd, uint32_t arg, int8_t crc)
{
	uint8_t packet[6];
	uint8_t response;
	uint8_t *bytes = (uint8_t *)&arg;

	packet[0] = cmd | 0x40;
	packet[1] = bytes[3];
	packet[2] = bytes[2];
	packet[3] = bytes[1];
	packet[4] = bytes[0];

#ifdef MMCSD_CRC_CHECK
	packet[5] = crc7(packet);
#else
	packet[5] = crc;
#endif

	// flushes command flow
	mcu_set_output(SD_SPI_CS);
	spi_xmit(0xFF);
	mcu_clear_output(SD_SPI_CS);
	spi_xmit(0xFF);

	// sends command, arg and CRC
	spi_xmit(packet[0]);
	spi_xmit(packet[1]);
	spi_xmit(packet[2]);
	spi_xmit(packet[3]);
	spi_xmit(packet[4]);
	spi_xmit(packet[5]);

	// returns response (R1 format)
	uint8_t tries = MMCSD_MAX_NCR;
	do
	{
		response = spi_xmit(0xFF);
	} while (CHECKBIT(response, 7) && tries--);

	mcu_set_output(SD_SPI_CS);

	return response;
}

DSTATUS disk_status(BYTE pdrv)
{
	if (!mmcsd_card.initialized)
	{
		return (STA_NOINIT);
	}

	if (mmcsd_card.writeprotected)
	{
		return (STA_PROTECT);
	}

	return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
	if (disk_status(0) & (STA_NOINIT | STA_NODISK))
	{
		return RES_NOTRDY;
	}

	// address must be a multiple of the block size
	uint32_t address = sector;

	if (count == 1)
	{
		// send read command
		if (mmcsd_command(17, address, 0xFF))
		{
			return RES_ERROR;
		}

		if (!mmcsd_response(buff, 512, true))
		{
			return RES_ERROR;
		}
	}
	else
	{
		if (mmcsd_command(18, address, 0xFF))
		{
			return RES_ERROR;
		}
		do
		{
			if (!mmcsd_response(buff, 512, true))
			{
				return RES_ERROR;
			}

			buff += 512;
		} while (--count);
		mmcsd_command(12, 0, 0xFF);
		for (uint8_t i = 10; i != 0; i--)
		{
			spi_xmit(0xFF);
		}
	}

	return RES_OK;
}

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
	if (!mmcsd_card.initialized)
	{
		return RES_NOTRDY;
	}

	if (mmcsd_card.writeprotected)
	{
		return RES_WRPRT;
	}

	if (count == 1)
	{
		// write cmd
		if (mmcsd_command(24, sector, 0xFF) && !mmcsd_message(buff, count, 0xFE))
		{
			return RES_ERROR;
		}
	}
	else
	{
		if (mmcsd_card.card_type == MMCv3)
		{
			mmcsd_command(55, 0, 0xFF);
			mmcsd_command(23, count, 0xFF);
		}

		if (mmcsd_command(25, sector, 0xFF) && !mmcsd_message(buff, count, 0xFC))
		{
			return RES_ERROR;
		}
	}

	return RES_OK;
}

#endif

DSTATUS disk_initialize(BYTE pdrv)
{
	uint8_t i, resp[4], crc41;
	uint32_t high_arg;

	memset(&mmcsd_card, 0, sizeof(mmcsd_card_t));
	mmcsd_spi_speed(false);

	// initializes card
	mcu_set_output(SD_SPI_CS);
	// flushes transmision
	for (i = 15; i != 0; i--)
	{
		spi_xmit(0xFF);
	}

	if (mmcsd_command(0, 0x00, 0x95) != 0x01)
	{
		return STA_NOINIT;
	}

	// sends ACMD41 with bit HCS set to 1 if it's v2 (high density cards) else sends HCS set to 0
	mmcsd_card.detected = 1;
	crc41 = 0xE5;
	high_arg = 0;

	// tests if card is SD/MMC v2+ (if CMD8 is legal-interface conditions)
	if (mmcsd_command(8, 0x1AA, 0x87) == 1)
	{
		mmcsd_response(resp, 4, false);
		// card is not v2 or not usable
		if (!(resp[2] & 0x01) || resp[3] != 0xAA)
		{
			return STA_NOINIT;
		}
		// card is v2 and usable
		mmcsd_card.card_type = SDv2;
		crc41 = 0x77;
		high_arg = 0x40000000;
	}

	// CMD55 is a specific SD card command
	uint32_t timeout = MMCSD_TIMEOUT;
	while ((mmcsd_command(55, 0x00, 0x65) > 1) || mmcsd_command(41, high_arg, crc41))
	{
		if (timeout < mcu_millis())
		{
			mmcsd_card.card_type = MMCv3;
			timeout = MMCSD_TIMEOUT;
			while (mmcsd_command(1, 0x00, 0xF9))
			{
				if (timeout < mcu_millis())
				{
					return STA_NOINIT;
				}
			}
			break;
		}
	}

	if (mmcsd_card.card_type == NOCARD)
	{
		mmcsd_card.card_type = SDv1;
	}

	// checks if it's high density card (if returns error retries with arg = 0)
	disk_ioctl(pdrv, MMC_GET_OCR, resp);

	if (CHECKBIT(resp[0], 6))
	{
		mmcsd_card.is_highdensity = 1;
	}

	// turn off crc if cmd available
	mmcsd_command(59, 0x00, 0x91);
	// sets the block size to 512 if cmd available
	mmcsd_command(16, 512, 0x15);

	uint8_t csd[16];
	disk_ioctl(pdrv, MMC_GET_CSD, csd);

	uint32_t c_size = (0x3F & csd[7]);
	c_size <<= 8;
	c_size += csd[8];
	c_size <<= 8;
	c_size += csd[9];
	mmcsd_card.sectors = (c_size + 1) << 10;
	mmcsd_card.size = mmcsd_card.sectors << 9;

	mcu_set_output(SD_SPI_CS);
	mmcsd_card.initialized = 1;
	mmcsd_spi_speed(true);
	return 0;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
	switch (cmd)
	{
	case CTRL_SYNC:
		if (!mmcsd_waitready())
		{
			return RES_ERROR;
		}
		break;
	case GET_SECTOR_COUNT:
		*((UINT *)buff) = mmcsd_card.sectors;
		break;
	case GET_SECTOR_SIZE:
	case GET_BLOCK_SIZE:
		*((UINT *)buff) = 512;
		break;
	case CTRL_TRIM:
		/* code */
		break;
	case MMC_GET_TYPE:
		*((uint8_t *)buff) = mmcsd_card.card_type;
		break;
	case MMC_GET_CSD:
		if (mmcsd_command(9, 0x00, 0xFF))
		{
			return RES_ERROR;
		}

		mmcsd_response(buff, 16, true);
		break;
	case MMC_GET_CID:
		if (mmcsd_command(10, 0x00, 0xFF))
		{
			return RES_ERROR;
		}

		mmcsd_response(buff, 16, true);
		break;
	case MMC_GET_OCR:
		mmcsd_command(58, 0x40000000, 0x6F);
		mmcsd_response(buff, 4, false);
		if (CHECKBIT(((uint8_t *)buff)[2], 6))
		{
			mmcsd_command(58, 0x00, 0xFD);
			mmcsd_response(buff, 4, false);
		}
		break;
	case MMC_GET_SDSTAT:
		/* code */
		break;
	case ISDIO_READ:
		/* code */
		break;
	case ISDIO_WRITE:
		/* code */
		break;
	case ISDIO_MRITE:
		/* code */
		break;
	default:
		break;
	}

	return 0;
}
/*
int32 mmcsd_get_cardsize()
{
	uint8_t i, timeout = MMCSD_MAX_TIMEOUT;
	uint8_t block[16];
	int32 CARD_size;

	CARD_error.func = 0;
	CARD_error.error = 0;
	CARD_error.detail1 = 0;
	CARD_error.detail2 = 0;

	// envia o comando CID
	i = mmcsd_command(9, 0x00, 0xFF);
	if (i)
	{
		CARD_error.func = 4;
		CARD_error.error = 1;
		CARD_error.detail1 = i;
		CARD_error.detail2 = 0;
		return 0;
	}

	mcu_clear_output(SD_SPI_CS);
	// aguarda testemunho
	while (spi_xmit(0xFF) != 0xFE && timeout)
		timeout--;

	// verifica se ocorreu timeout
	if (!timeout)
	{
		CARD_error.func = 4;
		CARD_error.error = 2;
		CARD_error.detail1 = MMCSD_TIMEOUT;
		CARD_error.detail2 = 0;
		return 0;
	}

	for (i = 0; i < 16; i++)
		block[i] = spi_xmit(0xFF);

	spi_xmit(0xFF);
	spi_xmit(0xFF);

	// extrai informa��o
	CARD_size = (0x03 & block[6]) << 8;
	CARD_size |= block[7];
	CARD_size <<= 2;
	CARD_size |= (block[8] >> 6);
	i = (block[9] & 0x03) << 1;
	i |= CHECKBIT(block[10], 7);
	i += 2;
	i += (block[5] & 0x0F);
	CARD_size <<= i;

	mcu_set_output(SD_SPI_CS);
	return CARD_size;
}
*/
