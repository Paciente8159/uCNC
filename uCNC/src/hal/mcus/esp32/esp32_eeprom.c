/**
 * EEPROM in Flash C implementation
 */

#include "../../../cnc.h"
#if (CONFIG_IDF_TARGET_ESP32)
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "esp_ipc.h"
#include "driver/uart.h"
#include "driver/timer.h"
#include "soc/i2s_struct.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#if !defined(RAM_ONLY_SETTINGS) && !defined(USE_ARDUINO_EEPROM_LIBRARY)
#include <nvs.h>
#include <esp_partition.h>
// Non volatile memory
typedef struct
{
	nvs_handle_t nvs_handle;
	size_t size;
	bool dirty;
	uint8_t data[NVM_STORAGE_SIZE];
} flash_eeprom_t;

static flash_eeprom_t mcu_eeprom;

void mcu_eeprom_init(int size)
{
	// starts nvs
	mcu_eeprom.size = 0;
	memset(mcu_eeprom.data, 0, NVM_STORAGE_SIZE);
	if (nvs_open("eeprom", NVS_READWRITE, &mcu_eeprom.nvs_handle) == ESP_OK)
	{
		// determines the maximum sector size of NVS that can be read/write
		nvs_get_blob(mcu_eeprom.nvs_handle, "eeprom", NULL, &mcu_eeprom.size);
		if (NVM_STORAGE_SIZE > mcu_eeprom.size)
		{
			log_e("eeprom does not have enough space");
			mcu_eeprom.size = 0;
		}

		nvs_get_blob(mcu_eeprom.nvs_handle, "eeprom", mcu_eeprom.data, &mcu_eeprom.size);
	}
	else
	{
		log_e("eeprom failed to open");
	}
}

uint8_t mcu_eeprom_getc(uint16_t address)
{
	if (NVM_STORAGE_SIZE <= address)
	{
		DBGMSG("EEPROM invalid address @ %u", address);
		return 0;
	}
#ifndef RAM_ONLY_SETTINGS
	// return esp32_eeprom_read(address);
	size_t size = mcu_eeprom.size;
	if (size)
	{
		return mcu_eeprom.data[address];
	}
#endif
	return 0;
}

/**
 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
	if (NVM_STORAGE_SIZE <= address)
	{
		DBGMSG("EEPROM invalid address @ %u", address);
	}
#ifndef RAM_ONLY_SETTINGS
	// esp32_eeprom_write(address, value);
	size_t size = mcu_eeprom.size;
	if (size)
	{
		mcu_eeprom.dirty |= (mcu_eeprom.data[address] != value);
		mcu_eeprom.data[address] = value;
	}
#endif
}

/**
 * flushes all recorded registers into the eeprom.
 * */
void mcu_eeprom_flush(void)
{
#ifndef RAM_ONLY_SETTINGS
	// esp32_eeprom_flush();
	// esp32_eeprom_write(address, value);
	if (mcu_eeprom.size && mcu_eeprom.dirty)
	{
		nvs_set_blob(mcu_eeprom.nvs_handle, "eeprom", mcu_eeprom.data, mcu_eeprom.size);
		nvs_commit(mcu_eeprom.nvs_handle);
	}
#endif
}

#endif

#endif