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
#include "mmcsd.h"
#include <stdint.h>
#include <stdbool.h>

#define PETIT_FAT_FS 1
#define FAT_FS 2

#ifndef SD_FAT_FS
#define SD_FAT_FS PETIT_FAT_FS
#endif

#if (SD_FAT_FS == PETIT_FAT_FS)
#include "petit_fat_fs/pff.h"
#else
#include "fat_fs/ff.h"
#endif

#include "diskio.h"

#ifndef MMCSD_MAX_NCR
#define MMCSD_MAX_NCR 255
#endif
#ifndef MMCSD_MAX_TIMEOUT
#define MMCSD_MAX_TIMEOUT 500
#endif
#define MMCSD_TIMEOUT (mcu_millis() + MMCSD_MAX_TIMEOUT)
#ifndef MMCSD_MAX_BUFFER_SIZE
#define MMCSD_MAX_BUFFER_SIZE 512
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
#ifndef SD_HWSPI_PORT
#define SD_HWSPI_PORT mcu_spi_port
#endif
HARDSPI(mmcsd_spi, 100000UL, 0, SD_HWSPI_PORT);
#endif

#define SD_SPI_PORT (&mmcsd_spi)

// #define SD_DEBUG
#ifdef SD_DEBUG
#define DEBUGSTR(x) serial_print_str(x "\r\n")
#define DEBUGINT(x)    \
	serial_print_int(x); \
	serial_print_str("\r\n")
#define DEBUGFFLT(x)   \
	serial_print_flt(x); \
	serial_print_str("\r\n")
#else
#define DEBUGSTR(x) ({})
#define DEBUGINT(x) ({})
#define DEBUGFFLT(x) ({})
#endif

static mmcsd_card_t mmcsd_card;

FORCEINLINE static void mmcsd_spi_speed(bool highspeed)
{
	if (highspeed)
	{
		softspi_set_frequency(SD_SPI_PORT, 10000000UL);
	}
	else
	{
		softspi_set_frequency(SD_SPI_PORT, 100000UL);
	}
}

void mmcsd_release(uint8_t *val)
{
	softspi_stop(SD_SPI_PORT);
	mcu_set_output(SD_SPI_CS);
	softspi_xmit(SD_SPI_PORT, 0xFF);
}

bool mmcsd_waittoken(uint8_t token)
{
	uint32_t timeout = MMCSD_TIMEOUT;
	while ((softspi_xmit(SD_SPI_PORT, 0xFF) != token) && (timeout > mcu_millis()))
		;

	if ((timeout <= mcu_millis()))
	{
		return false;
	}

	return true;
}

bool mmcsd_waitready(void)
{
	uint32_t timeout = MMCSD_TIMEOUT;
	while (softspi_xmit(SD_SPI_PORT, 0xFF) != 0xFF)
	{
		if ((timeout < mcu_millis()))
		{
			return false;
		}
	}

	return true;
}

bool mmcsd_response(uint8_t *result, uint16_t len, uint8_t token)
{
	if (token)
	{
		if (!mmcsd_waittoken(token))
		{
			memset(result, 0x00, len);
			return false;
		}
	}

	memset(result, 0xFF, len);
	uint32_t s = mcu_micros();
	softspi_bulk_xmit(SD_SPI_PORT, result, result, len);
	uint32_t e = mcu_micros();
	serial_print_int((e - s));
	serial_print_str("us\n");

	// discard CRC
	softspi_xmit(SD_SPI_PORT, 0xFF);
	softspi_xmit(SD_SPI_PORT, 0xFF);
	return true;
}

bool mmcsd_message(const uint8_t *buff, uint16_t count, uint8_t token)
{
	do
	{
		softspi_xmit(SD_SPI_PORT, 0xFF);
		softspi_xmit(SD_SPI_PORT, token);

		// sends data to buffer
		softspi_bulk_xmit(SD_SPI_PORT, buff, NULL, 512);

		// CRC dummy
		softspi_xmit(SD_SPI_PORT, 0xFF);
		softspi_xmit(SD_SPI_PORT, 0xFF);
		// If not accepted, return with error
		if ((softspi_xmit(SD_SPI_PORT, 0xFF) & 0x1F) != 0x05)
		{
			DEBUGSTR("data not accepted");
			return false;
		}

		count--;
	} while (count);

	// wait for write to complete
	if (!mmcsd_waitready())
	{
		DEBUGSTR("SD card ready msg timeout");
		return false;
	}

	// sends token
	if (token == 0xFC)
	{
		softspi_xmit(SD_SPI_PORT, 0xFD);
	}

	return true;
}

FORCEINLINE static uint8_t mmcsd_command(uint8_t cmd, uint32_t arg, int8_t crc)
{
	uint8_t packet[6];
	uint8_t response;
	uint8_t *bytes = (uint8_t *)&arg;

	DEBUGSTR("cmd");
	DEBUGINT(cmd);

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

	mcu_set_output(SD_SPI_CS);
	softspi_xmit(SD_SPI_PORT, 0xFF);
	mcu_clear_output(SD_SPI_CS);
	softspi_xmit(SD_SPI_PORT, 0xFF);

	// sends command, arg and CRC
	softspi_xmit(SD_SPI_PORT, packet[0]);
	softspi_xmit(SD_SPI_PORT, packet[1]);
	softspi_xmit(SD_SPI_PORT, packet[2]);
	softspi_xmit(SD_SPI_PORT, packet[3]);
	softspi_xmit(SD_SPI_PORT, packet[4]);
	softspi_xmit(SD_SPI_PORT, packet[5]);

	// returns response (R1 format)
	uint8_t tries = MMCSD_MAX_NCR;
	do
	{
		response = softspi_xmit(SD_SPI_PORT, 0xFF);
	} while ((response & 0x80) && tries--);

	return response;
}

DSTATUS disk_status(BYTE pdrv)
{
	if (!mmcsd_card.initialized)
	{
		DEBUGSTR("SD card status not init");
		return (STA_NOINIT);
	}

	if (mmcsd_card.writeprotected)
	{
		return (STA_PROTECT);
	}

	return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
	uint8_t cleanup __attribute__((__cleanup__(mmcsd_release))) = 1;
	if (mcu_get_output(SD_SPI_CS))
	{
		softspi_start(SD_SPI_PORT);
		mcu_clear_output(SD_SPI_CS);
	}

	if (disk_status(0) & (STA_NOINIT | STA_NODISK))
	{
		return RES_NOTRDY;
	}

	if (!mmcsd_card.is_highdensity)
	{
		sector <<= 9;
	}

	// address must be a multiple of the block size
	DEBUGSTR("SD card read address");
	DEBUGINT(sector);
	DEBUGINT(count);

	while (!mmcsd_waitready())
	{
		DEBUGSTR("SD card timeout ready read");
	}

	if (count == 1)
	{
		// send read command
		uint8_t error = mmcsd_command(17, sector, 0xFF);
		if (error)
		{
			DEBUGSTR("SD card read error CMD17");
			DEBUGINT(error);
			return RES_ERROR;
		}

		if (!mmcsd_response(buff, 512, 0xFE))
		{
			DEBUGSTR("SD card read error CMD17 on response");
			return RES_ERROR;
		}
	}
	else
	{
		if (mmcsd_command(18, sector, 0xFF))
		{
			DEBUGSTR("SD card read error CMD18");
			return RES_ERROR;
		}
		do
		{
			if (!mmcsd_response(buff, 512, 0xFE))
			{
				DEBUGSTR("SD card read error CMD18 on response");
				return RES_ERROR;
			}

			buff += 512;
		} while (--count);
		mmcsd_command(12, 0, 0xFF);
		for (uint8_t i = 10; i != 0; i--)
		{
			softspi_xmit(SD_SPI_PORT, 0xFF);
		}
	}

	return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
	uint8_t cleanup __attribute__((__cleanup__(mmcsd_release))) = 1;
	if (mcu_get_output(SD_SPI_CS))
	{
		softspi_start(SD_SPI_PORT);
		mcu_clear_output(SD_SPI_CS);
	}

	if (!mmcsd_card.initialized)
	{
		return RES_NOTRDY;
	}

	if (!mmcsd_card.is_highdensity)
	{
		sector <<= 9;
	}

	DEBUGSTR("SD card write address");
	DEBUGINT(sector);
	DEBUGINT(count);

	if (mmcsd_card.writeprotected)
	{
		return RES_WRPRT;
	}

	if (count == 1)
	{
		// write cmd
		uint8_t error = mmcsd_command(24, sector, 0xFF);
		if (error)
		{
			DEBUGSTR("SD card read error CMD24");
			DEBUGINT(error);
			return RES_ERROR;
		}

		if (!mmcsd_message(buff, count, 0xFE))
		{
			DEBUGSTR("SD card write error CMD24 on block write");
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

		if (mmcsd_command(25, sector, 0xFF))
		{
			DEBUGSTR("SD card read error CMD25");
			return RES_ERROR;
		}

		if (!mmcsd_message(buff, count, 0xFC))
		{
			DEBUGSTR("SD card write error CMD25 on block write");
			return RES_ERROR;
		}

		mmcsd_command(12, 0, 0xFF);
		for (uint8_t i = 10; i != 0; i--)
		{
			softspi_xmit(SD_SPI_PORT, 0xFF);
		}
	}

	if (!mmcsd_waitready())
	{
		DEBUGSTR("SD card write timeout");
		return RES_ERROR;
	}

	uint8_t res = 0;
	if (mmcsd_command(13, 0, 0xFF) && !mmcsd_response(&res, 1, 0))
	{
		DEBUGSTR("SD card read error CMD13");
		return RES_ERROR;
	}

	if (res)
	{
		DEBUGSTR("SD card CMD13 status error code");
		DEBUGINT(res);
		return RES_ERROR;
	}

	return RES_OK;
}

DSTATUS disk_initialize(BYTE pdrv)
{
	uint8_t cleanup __attribute__((__cleanup__(mmcsd_release))) = 1;
	spi_config_t conf = {0};
	softspi_config(SD_SPI_PORT, conf, 100000UL);
	mcu_clear_output(SD_SPI_CS);

	uint8_t resp[4], crc41;
	uint32_t high_arg;

	memset(&mmcsd_card, 0, sizeof(mmcsd_card_t));
	mmcsd_spi_speed(false);
	mmcsd_card.detected = 0;
	mmcsd_card.card_type = NOCARD;
	mmcsd_card.is_highdensity = 0;
	mmcsd_card.initialized = 0;
	mmcsd_card.sectors = 0;
	mmcsd_card.size = 0;
	mmcsd_card.writeprotected = 0;

	// initializes card
	// flushes transmision
	for (uint8_t j = 3; j != 0; j--)
	{
		mcu_set_output(SD_SPI_CS);
		for (uint8_t i = 10; i != 0; i--)
		{
			softspi_xmit(SD_SPI_PORT, 0xFF);
		}

		mcu_clear_output(SD_SPI_CS);
		if (mmcsd_command(0, 0x00, 0x95) == 0x01)
		{
			mmcsd_card.detected = 1;
			break;
		}

		DEBUGSTR("failed to init card");
	}

	if (!mmcsd_card.detected)
	{
		return STA_NOINIT;
	}

	// sends ACMD41 with bit HCS set to 1 if it's v2 (high density cards) else sends HCS set to 0
	mmcsd_card.detected = 1;
	crc41 = 0xE5;
	high_arg = 0;

	// tests if card is SD/MMC v2+ (if CMD8 is legal-interface conditions)
	if (mmcsd_command(8, 0x1AA, 0x87) == R1(R1_IDLE))
	{
		mmcsd_response(resp, 4, 0);
		// card is not v2 or not usable
		if (!(resp[2] & 0x01) || resp[3] != 0xAA)
		{
			DEBUGSTR("SD card not v2 or not usable");
			return STA_NOINIT;
		}
		// card is v2 and usable
		mmcsd_card.card_type = SDv2;
		crc41 = 0x77;
		high_arg = 0x40000000;
		DEBUGSTR("SD card is v2+");
	}

	// CMD55 is a specific SD card command
	uint32_t timeout = MMCSD_TIMEOUT;
	while ((mmcsd_command(55, 0x00, 0x65) > 1) || mmcsd_command(41, high_arg, crc41))
	{
		if (timeout < mcu_millis())
		{
			DEBUGSTR("SD card failed ACMD41");
			mmcsd_card.card_type = MMCv3;
			timeout = MMCSD_TIMEOUT;
			while (mmcsd_command(1, 0x00, 0xF9))
			{
				if (timeout < mcu_millis())
				{
					DEBUGSTR("SD card failed CMD1");
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

	DEBUGSTR("SD card type is");
	DEBUGINT(mmcsd_card.card_type);

	// checks if it's high density card (if returns error retries with arg = 0)
	disk_ioctl(pdrv, MMC_GET_OCR, resp);

	if (CHECKBIT(resp[0], 6))
	{
		DEBUGSTR("SD card is high density");
		mmcsd_card.is_highdensity = 1;
	}

	// turn off crc if cmd available
	mmcsd_command(59, 0x00, 0x91);
	// sets the block size to 512 if cmd available
	mmcsd_command(16, 512, 0x15);

	uint8_t csd[16];
	uint32_t csize = 0;
	disk_ioctl(pdrv, MMC_GET_CSD, csd);

	if ((csd[0] >> 6) == 1)
	{ /* SDC ver 2.00 */
		csize = csd[9] + ((WORD)csd[8] << 8) + 1;
		mmcsd_card.sectors = csize << 10;
	}
	else
	{ /* MMC or SDC ver 1.XX */
		uint8_t n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
		csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
		mmcsd_card.sectors = (DWORD)csize << (n - 9);
	}
	/*uint32_t c_size = (0x3F & csd[7]);
	c_size <<= 8;
	c_size += csd[8];
	c_size <<= 8;
	c_size += csd[9];
	mmcsd_card.sectors = (c_size + 1) << 10;*/
	mmcsd_card.size = mmcsd_card.sectors << 9;
	mmcsd_card.initialized = 1;

	DEBUGSTR("SD card size");
	DEBUGINT(mmcsd_card.size);
	mmcsd_spi_speed(true);
	return 0;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
	uint8_t cleanup __attribute__((__cleanup__(mmcsd_release))) = 1;
	if (mcu_get_output(SD_SPI_CS))
	{
		softspi_start(SD_SPI_PORT);
		mcu_clear_output(SD_SPI_CS);
	}

	switch (cmd)
	{
	case CTRL_SYNC:
		if (!mmcsd_waitready())
		{
			DEBUGSTR("SD card timeout");
			return RES_ERROR;
		}
		break;
	case GET_SECTOR_COUNT:
		*((UINT *)buff) = mmcsd_card.sectors;
		break;
	case GET_SECTOR_SIZE:
		*((UINT *)buff) = 512;
		break;
	case MMC_GET_TYPE:
		*((uint8_t *)buff) = mmcsd_card.card_type;
		break;
	case MMC_GET_CSD:
		if (mmcsd_command(9, 0x00, 0xFF))
		{
			DEBUGSTR("SD card CSD error");
			return RES_ERROR;
		}
		mmcsd_response(buff, 16, 0xFE);
		break;
	case MMC_GET_CID:
		if (mmcsd_command(10, 0x00, 0xFF))
		{
			DEBUGSTR("SD card CID error");
			return RES_ERROR;
		}
		mmcsd_response(buff, 16, 0xFE);
		break;
	case MMC_GET_OCR:
		if (mmcsd_command(58, 0, 0xFD))
		{
			DEBUGSTR("SD card OCR error");
			return RES_ERROR;
		}
		mmcsd_response(buff, 4, 0);
		break;
	default:
		return RES_ERROR;
	}

	return 0;
}

/*-----------------------------------------------------------------------*/
/* Read partial sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp(
		BYTE *buff,		/* Pointer to the read buffer (NULL:Forward to the stream) */
		DWORD sector, /* Sector number (LBA) */
		UINT offset,	/* Byte offset to read from (0..511) */
		UINT count		/* Number of bytes to read (ofs + cnt mus be <= 512) */
)
{
	uint8_t cleanup __attribute__((__cleanup__(mmcsd_release))) = 1;
	if (mcu_get_output(SD_SPI_CS))
	{
		softspi_start(SD_SPI_PORT);
		mcu_clear_output(SD_SPI_CS);
	}

	if (disk_status(0) & (STA_NOINIT | STA_NODISK))
	{
		return RES_NOTRDY;
	}

	if (!mmcsd_card.is_highdensity)
	{
		sector <<= 9;
	}

	// address must be a multiple of the block size
	DEBUGSTR("SD card read address");
	DEBUGINT(sector);
	DEBUGINT(count);

	while (!mmcsd_waitready())
	{
		DEBUGSTR("SD card timeout ready read");
	}

	uint8_t error = mmcsd_command(17, sector, 0xFF);
	if (error)
	{
		DEBUGSTR("SD card read error CMD17");
		DEBUGINT(error);
		return RES_ERROR;
	}

	if (!mmcsd_waittoken(0xFE))
	{
		memset(buff, 0, count);
		DEBUGSTR("SD card read error CMD17 on response");
		return RES_ERROR;
	}

	uint16_t reminder = MAX((512 - offset - count), 0);
	// discard offset
	while (offset)
	{
		softspi_xmit(SD_SPI_PORT, 0xFF);
		offset--;
	}

	while (count)
	{
		*buff++ = softspi_xmit(SD_SPI_PORT, 0xFF);
		count--;
	}

	// discard reminder
	while (reminder)
	{
		softspi_xmit(SD_SPI_PORT, 0xFF);
		reminder--;
	}

	// discard CRC
	softspi_xmit(SD_SPI_PORT, 0xFF);
	softspi_xmit(SD_SPI_PORT, 0xFF);

	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write partial sector                                                  */
/*-----------------------------------------------------------------------*/

DRESULT disk_writep(
		const BYTE *buff, /* Pointer to the bytes to be written (NULL:Initiate/Finalize sector write) */
		DWORD sector			/* Number of bytes to send, Sector number (LBA) or zero */
)
{
	uint8_t error = RES_OK;
	uint8_t cleanup __attribute__((__cleanup__(mmcsd_release))) = 1;
	if (mcu_get_output(SD_SPI_CS))
	{
		softspi_start(SD_SPI_PORT);
		mcu_clear_output(SD_SPI_CS);
	}

	if (disk_status(0) & (STA_NOINIT | STA_NODISK | STA_PROTECT))
	{
		return RES_NOTRDY;
	}

	if (!mmcsd_card.is_highdensity)
	{
		sector <<= 9;
	}

	DEBUGSTR("SD card write address");
	DEBUGINT(sector);

	if (disk_status(0) & STA_PROTECT)
	{
		return RES_WRPRT;
	}

	if (buff)
	{
		for (uint32_t i = 0; i < sector; i++)
		{
			softspi_xmit(SD_SPI_PORT, *buff++);
		}
	}
	else if (sector)
	{
		// write cmd
		error = mmcsd_command(24, sector, 0xFF);
		if (error)
		{
			DEBUGSTR("SD card read error CMD24");
			DEBUGINT(error);
			return RES_ERROR;
		}

		// start token
		softspi_xmit(SD_SPI_PORT, 0xFF);
		softspi_xmit(SD_SPI_PORT, 0xFE);
	}
	else
	{
		// CRC dummy
		softspi_xmit(SD_SPI_PORT, 0xFF);
		softspi_xmit(SD_SPI_PORT, 0xFF);
		// If not accepted, return with error
		if ((softspi_xmit(SD_SPI_PORT, 0xFF) & 0x1F) != 0x05)
		{
			DEBUGSTR("data not accepted");
			error = RES_ERROR;
		}
	}

	return error;
}
