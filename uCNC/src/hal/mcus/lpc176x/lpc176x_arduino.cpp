/*
	Name: lpc176x_usb.cpp
	Description: Integrates Arduino LPC176x CDC into µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 06-05-2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifdef ARDUINO_ARCH_LPC176X

#include <usb/usb.h>
#include <usb/usbcfg.h>
#include <usb/usbhw.h>
#include <usb/usbcore.h>
#include <usb/cdc.h>
#include <usb/cdcuser.h>
#include <usb/mscuser.h>
#include <CDCSerial.h>
#include <usb/mscuser.h>
#include <stdint.h>

extern "C"
{
#include "../../../cnc.h"
#ifdef USE_ARDUINO_CDC
	void lpc176x_usb_dotasks(void)
	{
		MSC_RunDeferredCommands();
	}

	bool lpc176x_usb_available(void)
	{
		return (UsbSerial.available() > 0);
	}

	uint8_t lpc176x_usb_getc(void)
	{
		return (uint8_t)UsbSerial.read();
	}

	void lpc176x_usb_putc(uint8_t c)
	{
		char a = (char)c;
		UsbSerial.write(&a, 1);
	}

	void lpc176x_usb_write(uint8_t* ptr, uint8_t len)
	{
		UsbSerial.write((char*)ptr, len);
	}

	void lpc176x_usb_init(void)
	{
		USB_Init();			// USB Initialization
		USB_Connect(false); // USB clear connection
		USB_Connect(true);
		uint32_t usb_timeout = mcu_millis() + 2000;
		while (!USB_Configuration && usb_timeout > mcu_millis())
		{
			mcu_delay_us(50);
			MSC_RunDeferredCommands();
#if ASSERT_PIN(ACTIVITY_LED)
			io_toggle_output(ACTIVITY_LED); // Flash quickly during USB initialization
#endif
		}
		UsbSerial.begin(BAUDRATE);
// 		// BUFFER_CLEAR(usb_rx);
	}

#endif

#if defined(MCU_HAS_I2C) && defined(USE_ARDUINO_WIRE)
#include <Wire.h>

#if I2C_ADDRESS!=0
#error "I2C slave mode not supported"
#endif

extern "C"
{
#if (I2C_ADDRESS != 0)
	static uint8_t mcu_i2c_buffer_len;
	static uint8_t mcu_i2c_buffer[I2C_SLAVE_BUFFER_SIZE];
	void lpc176x_i2c_onreceive(int len)
	{
		uint8_t l = I2C_REG.readBytes(mcu_i2c_buffer, len);
		mcu_i2c_slave_cb(mcu_i2c_buffer, &l);
		mcu_i2c_buffer_len = l;
	}

	void lpc176x_i2c_onrequest(void)
	{
		I2C_REG.write(mcu_i2c_buffer, mcu_i2c_buffer_len);
	}

#endif

	void mcu_i2c_config(uint32_t frequency)
	{
#if I2C_ADDRESS == 0
		I2C_REG.begin();
#else
		I2C_REG.onReceive(lpc176x_i2c_onreceive);
		I2C_REG.onRequest(lpc176x_i2c_onrequest);
		I2C_REG.begin(I2C_ADDRESS);
#endif
	}

	uint8_t mcu_i2c_send(uint8_t address, uint8_t *data, uint8_t datalen, bool release, uint32_t ms_timeout)
	{
		I2C_REG.beginTransmission(address);
		I2C_REG.write(data, datalen);
		return (I2C_REG.endTransmission() == 0) ? I2C_OK : I2C_NOTOK;
	}

	uint8_t mcu_i2c_receive(uint8_t address, uint8_t *data, uint8_t datalen, uint32_t ms_timeout)
	{
		if (I2C_REG.requestFrom(address, datalen) == datalen)
		{
			while(datalen--){
				*data = (uint8_t)I2C_REG.read();
				data++;
			}
			
			return I2C_OK;
		}

		return I2C_NOTOK;
	}
}
#endif
}
#endif
