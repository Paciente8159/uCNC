/*
	Name: esp32_uart.c
	Description: Implements the µCNC USB shim for all ESP32 variants.
	Uses Arduino

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 11-09-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#if defined(ESP32)
#include <Arduino.h>

extern "C"
{
#include "../../../cnc.h"

#if defined(MCU_HAS_USB) && defined(USE_ARDUINO_CDC)
	void mcu_usb_init(void)
	{
		Serial.begin(BAUDRATE);
	}

	void mcu_usb_dotasks(void)
	{
		while (Serial.available())
		{
			esp_task_wdt_reset();
#ifndef DETACH_USB_FROM_MAIN_PROTOCOL
			uint8_t c = Serial.read();
			if (mcu_com_rx_cb(c))
			{
				if (BUFFER_FULL(usb_rx))
				{
					STREAM_OVF(c);
				}

				BUFFER_ENQUEUE(usb_rx, &c);
			}
#else
			mcu_usb_rx_cb((uint8_t)SerialBT.read());
#endif
		}
	}

#ifndef USB_TX_BUFFER_SIZE
#define USB_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, usb_tx, USB_TX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, usb_rx, RX_BUFFER_SIZE);

	uint8_t mcu_usb_getc(void)
	{
		uint8_t c = 0;
		BUFFER_DEQUEUE(usb_rx, &c);
		return c;
	}

	uint8_t mcu_usb_available(void)
	{
		return BUFFER_READ_AVAILABLE(usb_rx);
	}

	void mcu_usb_clear(void)
	{
		BUFFER_CLEAR(usb_rx);
	}

	void mcu_usb_putc(uint8_t c)
	{
		while (BUFFER_FULL(usb_tx))
		{
			mcu_usb_flush();
		}
		BUFFER_ENQUEUE(usb_tx, &c);
	}

	void mcu_usb_flush(void)
	{
		while (!BUFFER_EMPTY(usb_tx))
		{
			uint8_t tmp[USB_TX_BUFFER_SIZE + 1];
			memset(tmp, 0, sizeof(tmp));
			uint8_t r;

			BUFFER_READ(usb_tx, tmp, USB_TX_BUFFER_SIZE, r);
			Serial.write(tmp, r);
			Serial.flush();
		}
	}

#endif
}
#endif
