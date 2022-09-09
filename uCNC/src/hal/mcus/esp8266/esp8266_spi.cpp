#include "../../../../cnc_config.h"
#ifdef ESP8266
#ifdef MCU_HAS_SPI
#include <Arduino.h>
#include <SPI.h>
#include "esp_peri.h"
extern "C"
{
	void esp8266_spi_init(uint32_t freq, uint8_t mode)
	{
		SPI.begin();
		SPI.setFrequency(freq);
		SPI.setDataMode(mode);
	}

	void esp8266_spi_config(uint8_t mode, uint32_t freq)
	{
		SPI.setFrequency(freq);
		SPI.setDataMode(mode);
	}
}

#endif
#endif