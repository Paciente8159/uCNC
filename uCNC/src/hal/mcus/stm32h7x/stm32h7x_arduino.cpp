

#ifdef STM32H7xx
#ifdef __cplusplus
extern "C"
{
#endif
#include "../../../cnc.h"
#ifdef __cplusplus
}
#endif

#if (MCU == MCU_STM32H7X)
#define _ARDUINO_PIN_NAME_(BIT, PORT) P##PORT##_##BIT
#define _ARDUINO_PIN_NAME(BIT, PORT) _ARDUINO_PIN_NAME_(BIT, PORT)
#define ARDUINO_PIN_NAME(PIN) (uint32_t)_ARDUINO_PIN_NAME(PIN##_BIT, PIN##_PORT)

#ifdef USE_ARDUINO_I2C_LIBRARY
#undef I2C_OK
#ifdef MCU_HAS_I2C
#include <Arduino.h>
#include <Wire.h>
TwoWire arduino_i2c = TwoWire(ARDUINO_PIN_NAME(I2C_DATA), ARDUINO_PIN_NAME(I2C_CLK));

extern "C"
{
	void mcu_i2c_config(uint32_t frequency)
	{
		arduino_i2c.begin(ARDUINO_PIN_NAME(I2C_DATA), ARDUINO_PIN_NAME(I2C_CLK));
		arduino_i2c.setClock(frequency);
	}

	uint8_t mcu_i2c_send(uint8_t address, uint8_t *data, uint8_t datalen, bool release, uint32_t ms_timeout)
	{
		arduino_i2c.setTimeout(ms_timeout);
		arduino_i2c.beginTransmission(address);
		arduino_i2c.write(data, datalen);
		arduino_i2c.endTransmission(release);
		return 0;
	}

	uint8_t mcu_i2c_receive(uint8_t address, uint8_t *data, uint8_t datalen, uint32_t ms_timeout)
	{
		arduino_i2c.requestFrom(address, datalen);
		return arduino_i2c.readBytes(data, datalen);
	}
}
#endif
#endif

#ifdef USE_ARDUINO_SPI_LIBRARY
#ifdef MCU_HAS_SPI
#include <Arduino.h>
#include <SPI.h>

SPIClass arduino_spi = SPIClass(ARDUINO_PIN_NAME(SPI_SDO), ARDUINO_PIN_NAME(SPI_SDI), ARDUINO_PIN_NAME(SPI_CLK), NC);

#ifdef __cplusplus
extern "C"
{
#endif

	void mcu_spi_config(spi_config_t config, uint32_t frequency)
	{
		arduino_spi.begin();
		mcu_spi_start(config, frequency);
		mcu_spi_stop();
	}

	uint8_t mcu_spi_xmit(uint8_t c)
	{
		return arduino_spi.transfer(c, false);
	}

	void mcu_spi_start(spi_config_t config, uint32_t frequency)
	{
		arduino_spi.beginTransaction(SPISettings(frequency, MSBFIRST, config.mode));
	}

	void mcu_spi_stop(void)
	{
		arduino_spi.endTransaction();
	}

	bool mcu_spi_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
	{
		arduino_spi.transfer((const void *)out, (void *)in, len);
		return false;
	}

#ifdef __cplusplus
}
#endif

#endif
#endif

#endif
#endif