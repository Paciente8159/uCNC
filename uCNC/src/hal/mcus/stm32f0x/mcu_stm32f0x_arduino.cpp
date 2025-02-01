

extern "C"
{
#include "../../../cnc.h"
}

#if (MCU == MCU_STM32F0X)
#ifdef USE_ARDUINO_I2C_LIBRARY
#undef I2C_OK
#ifdef MCU_HAS_I2C
#include <Arduino.h>
#include <Wire.h>

#define _ARDUINO_PIN_NAME_(BIT, PORT) P##PORT##_##BIT
#define _ARDUINO_PIN_NAME(BIT, PORT) _ARDUINO_PIN_NAME_(BIT, PORT)
#define ARDUINO_PIN_NAME(PIN) (uint32_t)_ARDUINO_PIN_NAME(PIN##_BIT, PIN##_PORT)
TwoWire arduino_i2c = TwoWire(ARDUINO_PIN_NAME(I2C_DATA), ARDUINO_PIN_NAME(I2C_CLK));

extern "C"
{
void mcu_i2c_config(uint32_t frequency){
	arduino_i2c.begin(ARDUINO_PIN_NAME(I2C_DATA), ARDUINO_PIN_NAME(I2C_CLK));
	arduino_i2c.setClock(frequency);
}

uint8_t mcu_i2c_send(uint8_t address, uint8_t *data, uint8_t datalen, bool release, uint32_t ms_timeout){
	arduino_i2c.setTimeout(ms_timeout);
	arduino_i2c.beginTransmission(address);
	arduino_i2c.write(data, datalen);
	arduino_i2c.flush();
	arduino_i2c.endTransmission(release);
	return 0;
}

uint8_t mcu_i2c_receive(uint8_t address, uint8_t *data, uint8_t datalen, uint32_t ms_timeout){
	arduino_i2c.readBytes(data, datalen);
	return 0;
}

}
#endif
#endif

#endif