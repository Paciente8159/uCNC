#include "../../../../cnc_config.h"
#ifdef ESP8266
#ifdef MCU_HAS_SPI
#include <Arduino.h>
#include <SPI.h>
#include "esp_peri.h"
extern "C"
{
	void esp8266_spi_init(void)
	{
		SPI.begin();
		SPI.setFrequency(SPI_FREQ);
		SPI.setDataMode(SPI_MODE);
	}
}

#endif
#endif