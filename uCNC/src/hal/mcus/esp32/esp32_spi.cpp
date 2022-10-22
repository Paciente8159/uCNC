#include "../../../../cnc_config.h"
#ifdef ESP32
#include <Arduino.h>
#include <SPI.h>
extern "C"
{
	void esp32_spi_init(uint32_t freq, uint8_t mode, int8_t clk, int8_t sdi, int8_t sdo)
	{
		SPI.begin(clk, sdi, sdo, -1);
		SPI.setFrequency(freq);
		SPI.setDataMode(mode);
	}

	void esp32_spi_config(uint8_t mode, uint32_t freq)
	{
		SPI.setFrequency(freq);
		SPI.setDataMode(mode);
	}

	uint8_t esp32_spi_xmit(uint8_t data)
	{
		return SPI.transfer(data);
	}
}
#endif