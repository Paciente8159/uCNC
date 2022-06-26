#include "../../../../cnc_config.h"

#ifndef RAM_ONLY_SETTINGS
#include <Arduino.h>
#include <EEPROM.h>
#include <stdint.h>
extern "C"
{
	void esp8266_eeprom_init(int size)
	{
		EEPROM.begin(size);
	}

	void esp8266_eeprom_read(uint16_t address)
	{
		EEPROM.read(address);
	}

	void esp8266_eeprom_write(uint16_t address, uint8_t value)
	{
		EEPROM.write(address, value);
	}

	void esp8266_eeprom_flush(void)
	{
		EEPROM.commit();
	}
}

#endif