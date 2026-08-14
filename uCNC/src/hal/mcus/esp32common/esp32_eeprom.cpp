/*
	Name: esp32_arduino.cpp
	Description: Contains all Arduino ESP32 C++ to C functions used by µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 27-07-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#if defined(ESP32) /*&& !(defined(ESP32C3) || defined(ESP32S3))*/
#include <Arduino.h>
#include <EEPROM.h>
#include "esp_task_wdt.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

extern "C"
{
#include "../../../cnc.h"
#if !defined(RAM_ONLY_SETTINGS) && defined(USE_ARDUINO_EEPROM_LIBRARY)
	static bool eeprom_initialized;

	void mcu_eeprom_init(int size)
	{
		if (eeprom_initialized)
		{
			return;
		}
		EEPROM.begin(size);
		eeprom_initialized = true;
	}

	uint8_t mcu_eeprom_getc(uint16_t address)
	{
		if (!eeprom_initialized)
		{
			mcu_eeprom_init(NVM_STORAGE_SIZE);
		}

		if (NVM_STORAGE_SIZE <= address)
		{
			DBGMSG("EEPROM invalid address @ %u", address);
			return 0;
		}
		return EEPROM.read(address);
	}

	void mcu_eeprom_putc(uint16_t address, uint8_t value)
	{
		if (!eeprom_initialized)
		{
			mcu_eeprom_init(NVM_STORAGE_SIZE);
		}

		if (NVM_STORAGE_SIZE <= address)
		{
			DBGMSG("EEPROM invalid address @ %u", address);
		}
		EEPROM.write(address, value);
	}

	void mcu_eeprom_flush(void)
	{
		if (!eeprom_initialized)
		{
			mcu_eeprom_init(NVM_STORAGE_SIZE);
		}

		if (!EEPROM.commit())
		{
			DBGMSG("EEPROM write error");
		}
	}

#endif
}

#endif
