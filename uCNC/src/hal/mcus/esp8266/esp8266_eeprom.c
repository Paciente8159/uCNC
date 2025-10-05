/*
	Name: esp8266_eeprom.c
	Description: Implements EEPROM emulation in flash for ESP8266.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 04-02-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (MCU == MCU_ESP8266)
#include "flash_hal.h"
#include "c_types.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "spi_flash.h"

#ifdef ENABLE_DEBUG_STREAM
#undef DBGMSG(fmt, ...)
#define DBGMSG(fmt, ...)                                       \
	prt_fmt(&mcu_uart_putc, PRINT_CALLBACK, fmt, ##__VA_ARGS__); \
	mcu_uart_flush()
#endif

#ifndef RAM_ONLY_SETTINGS

#define NVM_STORAGE_SIZE_ALIGNED ((((NVM_STORAGE_SIZE - 1) >> 2) + 1) << 2)
#define FLASH_VALUE(X) (X ^ 0xFF)

#if (NVM_STORAGE_SIZE_ALIGNED > SPI_FLASH_SEC_SIZE)
#define EEPROM_DATA_SIZE SPI_FLASH_SEC_SIZE // loads a partial page into RAM
static uint8_t eeprom_current_sector;
#error "NVM cannot exceed Flash sector size"
#else
#define EEPROM_DATA_SIZE NVM_STORAGE_SIZE_ALIGNED // loads a full page into RAM
#endif

#define EEPROM_MODIFIED 0x01
#define EEPROM_NEWPAGE_REQUIRED 0x02

static uint32_t eeprom_data[EEPROM_DATA_SIZE >> 2]; // loads a full page into RAM
uint8_t *const eeprom_ptr = (uint8_t *)eeprom_data;
static uint16_t eeprom_current_page;
static uint8_t eeprom_status;

#define EEPROM_FLASH_SECTORS ((uint16_t)((NVM_STORAGE_SIZE_ALIGNED - 1) / SPI_FLASH_SEC_SIZE) + 1)
#define EEPROM_MAX_PAGES ((uint16_t)((SPI_FLASH_SEC_SIZE - 1) / NVM_STORAGE_SIZE_ALIGNED) + 1)
#define EEPROM_FLASH_BASE_SECTOR ((EEPROM_start - 0x40200000) / SPI_FLASH_SEC_SIZE)
#define EEPROM_SECTOR(offset) (EEPROM_FLASH_BASE_SECTOR + (((offset) / SPI_FLASH_SEC_SIZE)))
#define EEPROM_FLASH_ADDR(sector, page, offset) ((EEPROM_FLASH_BASE_SECTOR * SPI_FLASH_SEC_SIZE * (sector + 1)) + (page) * NVM_STORAGE_SIZE_ALIGNED + (offset))
#define EEPROM_SECTOR_ADDR(page, offset) (((page) * NVM_STORAGE_SIZE_ALIGNED + (offset)) & (SPI_FLASH_SEC_SIZE - 1))

static uint16_t mcu_access_flash_page(uint16_t address)
{
#if (NVM_STORAGE_SIZE_ALIGNED > SPI_FLASH_SEC_SIZE)
	uint8_t sector = (uint8_t)address / SPI_FLASH_SEC_SIZE;
	// if the NVM takes multiple sectors load the new sector
	if (eeprom_current_sector != sector)
	{
		mcu_eeprom_flush();
		eeprom_status = 0;
		eeprom_current_sector = sector;
		// loads a new sector to memory
		ATOMIC_BLOCK
		{
			spi_flash_read(EEPROM_FLASH_ADDR(sector, 0, 0), eeprom_data, EEPROM_DATA_SIZE);
		}
	}

	// returns the offset within that sector
	return (uint16_t)EEPROM_SECTOR_ADDR(eeprom_current_page, address);
#else
	return (uint16_t)address;
#endif
}

void mcu_eeprom_init(void)
{
	eeprom_current_page = 0;
	eeprom_status = 0;

#if (NVM_STORAGE_SIZE_ALIGNED > SPI_FLASH_SEC_SIZE)
	eeprom_current_sector = 0;
	ATOMIC_BLOCK
	{
		spi_flash_read(EEPROM_FLASH_ADDR(0, 0, 0), eeprom_ptr, EEPROM_DATA_SIZE);
	}
#else
	DBGMSG("search eeprom start\n");
	// if multiple pages fit in a section find the last writen page
	uint8_t tmp[4];
	for (uint16_t i = 0; i < EEPROM_MAX_PAGES; i++)
	{
		// just load the 4 first bytes
		memset(tmp, 0xFF, 4);
		DBGMSG("read address %lx\n", EEPROM_FLASH_ADDR(0, i, 0));
		ATOMIC_BLOCK
		{
			spi_flash_read(EEPROM_FLASH_ADDR(0, i, 0), (uint32_t *)tmp, 4);
		}
		if (tmp[0] == FLASH_VALUE('V'))
		{
			DBGMSG("found at %u (try next)\n", i);
			eeprom_current_page = i;
		}
		else
		{
			DBGMSG("not found at %u\n", i);
			if (i == 0)
			{
				DBGMSG("sector untouched\n", i);
				// jump to last that means that all memory is erased and the first index is the one to write to
				eeprom_current_page = EEPROM_MAX_PAGES;
				eeprom_status |= EEPROM_NEWPAGE_REQUIRED;
				memset(eeprom_data, 0xFF, EEPROM_DATA_SIZE);
				return;
			}
			break;
		}
	}

	DBGMSG("load address %lx\n", EEPROM_FLASH_ADDR(0, eeprom_current_page, 0));
	ATOMIC_BLOCK
	{
		spi_flash_read(EEPROM_FLASH_ADDR(0, eeprom_current_page, 0), eeprom_data, EEPROM_DATA_SIZE);
	}
#endif
}

// Non volatile memory
/**
 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
uint8_t mcu_eeprom_getc(uint16_t address)
{
	// invalid addresss
	if (address > NVM_STORAGE_SIZE_ALIGNED)
	{
		return 0;
	}
	// gets the address within the sector
	uint16_t data_address = mcu_access_flash_page(address);
	return FLASH_VALUE(eeprom_ptr[data_address]);
}

/**
 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
	// invalid address
	if (address > NVM_STORAGE_SIZE_ALIGNED)
	{
		return;
	}

	// convert to flash value
	value = FLASH_VALUE(value);

	// gets the address within the sector
	uint16_t data_address = mcu_access_flash_page(address);
	// check if the value is changed or not
	if (eeprom_ptr[data_address] != value)
	{
		if (((eeprom_ptr[data_address] ^ value) & value)) // bits have changed and need to return to 1
		{
			eeprom_status |= EEPROM_NEWPAGE_REQUIRED;
		}
		eeprom_status |= EEPROM_MODIFIED;
	}

	eeprom_ptr[data_address] = value;
}

/**
 * flushes all recorded registers into the eeprom.
 * */
void mcu_eeprom_flush(void)
{
	// leave if untouched
	if (!eeprom_status)
	{
		return;
	}

	DBGMSG("flush - current page %u\n", eeprom_current_page);

	if (eeprom_status & EEPROM_NEWPAGE_REQUIRED)
	{
		// requires a new page
		uint16_t newpage = eeprom_current_page + 1;
		eeprom_current_page = newpage;
		bool needs_erase = (newpage >= EEPROM_MAX_PAGES);

		if (needs_erase)
		{
			eeprom_current_page = 0;
			DBGMSG("erase all sectors\n");
			for (uint16_t i = 0; i < EEPROM_FLASH_SECTORS; i++)
			{ // erases the sector if needed

				DBGMSG("erasing sector %u\n", i);
				ATOMIC_BLOCK
				{
					if (spi_flash_erase_sector(EEPROM_FLASH_BASE_SECTOR + i) != SPI_FLASH_RESULT_OK)
					{
						DBGMSG("erase error");
						return;
					}
				}
			}
		}
	}

	if (eeprom_status & EEPROM_MODIFIED)
	{
		for (uint16_t i = 0; i < EEPROM_FLASH_SECTORS; i++)
		{ // erases the sector if needed
			DBGMSG("write address %lx\n", EEPROM_FLASH_ADDR(i, eeprom_current_page, 0));
			ATOMIC_BLOCK
			{
				if (spi_flash_write(EEPROM_FLASH_ADDR(i, eeprom_current_page, 0), eeprom_data, EEPROM_DATA_SIZE) != SPI_FLASH_RESULT_OK)
				{
					DBGMSG("write error");
				}
			}
		}
	}

	eeprom_status = 0;
}
#endif
#endif