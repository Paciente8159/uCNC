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

static uint8_t eeprom_data[SPI_FLASH_SEC_SIZE];
static uint16_t eeprom_current_page;
static uint16_t eeprom_current_sector;
static bool eeprom_modified;

#define EEPROM_FLASH_SECTORS ((uint16_t)((NVM_STORAGE_SIZE - 1) / SPI_FLASH_SEC_SIZE) + 1)
#define EEPROM_MAX_PAGES ((uint16_t)((SPI_FLASH_SEC_SIZE - 1) / NVM_STORAGE_SIZE) + 1)
#define EEPROM_FLASH_BASE_SECTOR ((EEPROM_start - 0x40200000) / SPI_FLASH_SEC_SIZE)
#define EEPROM_SECTOR(offset) (EEPROM_FLASH_BASE_SECTOR + (((offset) / SPI_FLASH_SEC_SIZE)))
#define EEPROM_FLASH_ADDR(sector, page, offset) (EEPROM_FLASH_BASE_SECTOR + (sector) * SPI_FLASH_SEC_SIZE + (page) * NVM_STORAGE_SIZE + (offset))
#define EEPROM_SECTOR_ADDR(page, offset) (((page) * NVM_STORAGE_SIZE + (offset)) & (SPI_FLASH_SEC_SIZE - 1))

static uint16_t mcu_access_flash_page(uint16_t address)
{
	uint16_t sector = EEPROM_SECTOR(address + (eeprom_current_page * NVM_STORAGE_SIZE));
	// if the NVM takes multiple sectors load the new sector
	if (eeprom_current_sector != sector)
	{
		mcu_eeprom_flush();
		eeprom_modified = false;
		eeprom_current_sector = sector;
		// loads a new sector to memory
		mcu_disable_global_isr();
		spi_flash_read(EEPROM_FLASH_ADDR(sector, 0, 0), eeprom_data, SPI_FLASH_SEC_SIZE);
		mcu_enable_global_isr();
	}

	// returns the offset within that sector
	return (uint16_t)EEPROM_SECTOR_ADDR(eeprom_current_page, address);
}

void mcu_eeprom_init(void)
{
	eeprom_current_page = 0;
	eeprom_current_sector = 0;
	eeprom_modified = 0;

	mcu_disable_global_isr();
	spi_flash_read(EEPROM_FLASH_ADDR(0, 0, 0), eeprom_data, SPI_FLASH_SEC_SIZE);
	mcu_enable_global_isr();

	// if multiple pages fit in a section find the last writen page
	if ((EEPROM_MAX_PAGES > 1))
	{
		for (uint16_t i = 0; i < EEPROM_MAX_PAGES; i++)
		{
			if (eeprom_data[EEPROM_SECTOR_ADDR(i, 0)] != 0)
			{
				eeprom_current_page = i;
			}
			else
			{
				break;
			}
		}
	}
}

// Non volatile memory
/**
 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
uint8_t mcu_eeprom_getc(uint16_t address)
{
	// gets the address within the sector
	uint16_t sector_address = mcu_access_flash_page(address);
	return eeprom_data[sector_address];
}

/**
 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
	// gets the address within the sector
	uint16_t sector_address = mcu_access_flash_page(address);
	// check if the value is changed or not
	if (eeprom_data[sector_address] != value)
	{
		eeprom_modified = true;
	}

	eeprom_data[sector_address] = value;
}

/**
 * flushes all recorded registers into the eeprom.
 * */
void mcu_eeprom_flush(void)
{
	// leave if untouched
	if (!eeprom_modified)
	{
		return;
	}

	eeprom_current_page++;
	uint16_t sector = EEPROM_SECTOR((eeprom_current_page * NVM_STORAGE_SIZE));
	bool erase_sector = (sector != eeprom_current_sector) ? true : false;

	if (eeprom_current_page >= EEPROM_MAX_PAGES)
	{
		eeprom_current_page = 0;
		erase_sector = true; // looped. Must erase
		eeprom_current_sector = 0;
	}

	// recompute sector (beacuse it might have overflowed and is now pointing to the first sector again)
	sector = EEPROM_SECTOR((eeprom_current_page * NVM_STORAGE_SIZE));

	for (uint16_t i = 0; i < EEPROM_FLASH_SECTORS; i++)
	{ // erases the sector if needed
		if (erase_sector)
		{
			mcu_disable_global_isr();
			spi_flash_erase_sector(sector + i);
			mcu_enable_global_isr();
		}

		mcu_disable_global_isr();
		spi_flash_write(EEPROM_FLASH_ADDR((sector + i), eeprom_current_page, 0), eeprom_data, MIN(NVM_STORAGE_SIZE, SPI_FLASH_SEC_SIZE));
		mcu_enable_global_isr();
	}
}
#endif