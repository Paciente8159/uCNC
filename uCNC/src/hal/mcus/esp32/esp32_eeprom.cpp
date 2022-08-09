#include "../../../../cnc_config.h"
#ifdef ESP32
#ifndef RAM_ONLY_SETTINGS
#include <Arduino.h>
#include <EEPROM.h>
#include <stdint.h>
extern "C"
{
	void esp32_eeprom_init(int size)
	{
		EEPROM.begin(size);
	}

	uint8_t esp32_eeprom_read(uint16_t address)
	{
		return EEPROM.read(address);
	}

	void esp32_eeprom_write(uint16_t address, uint8_t value)
	{
		EEPROM.write(address, value);
	}

	void esp32_eeprom_flush(void)
	{
		if (!EEPROM.commit())
		{
			Serial.println("[MSG: EEPROM write error]");
		}
	}
}

#endif
#endif