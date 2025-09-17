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

#if defined(ESP32S3) || defined(ESP32C3)
#include <Arduino.h>
#include "USB.h"
#include "USBCDC.h"
#include "esp_task_wdt.h"

#if !ARDUINO_USB_CDC_ON_BOOT
USBCDC USBSerial;
#elif ARDUINO_USB_CDC_ON_BOOT
#define USBSerial Serial
#endif

extern "C"
{
#include "../../../cnc.h"

#if defined(MCU_HAS_USB) && defined(ARDUINO_USB_CDC_ON_BOOT)

#ifndef USB_TX_BUFFER_SIZE
#define USB_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, usb_tx, USB_TX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, usb_rx, RX_BUFFER_SIZE);

	static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
	{
		if (event_base == ARDUINO_USB_EVENTS)
		{
			arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
			switch (event_id)
			{
			case ARDUINO_USB_STARTED_EVENT:
				// Serial.println("USB PLUGGED");
				break;
			case ARDUINO_USB_STOPPED_EVENT:
				// Serial.println("USB UNPLUGGED");
				break;
			case ARDUINO_USB_SUSPEND_EVENT:
				// Serial.printf("USB SUSPENDED: remote_wakeup_en: %u\n", data->suspend.remote_wakeup_en);
				break;
			case ARDUINO_USB_RESUME_EVENT:
				// Serial.println("USB RESUMED");
				break;

			default:
				break;
			}
		}
		else if (event_base == ARDUINO_USB_CDC_EVENTS)
		{
			arduino_usb_cdc_event_data_t *data = (arduino_usb_cdc_event_data_t *)event_data;
			switch (event_id)
			{
			case ARDUINO_USB_CDC_CONNECTED_EVENT:
				// Serial.println("CDC CONNECTED");
				break;
			case ARDUINO_USB_CDC_DISCONNECTED_EVENT:
				// Serial.println("CDC DISCONNECTED");
				break;
			case ARDUINO_USB_CDC_LINE_STATE_EVENT:
				// Serial.printf("CDC LINE STATE: dtr: %u, rts: %u\n", data->line_state.dtr, data->line_state.rts);
				break;
			case ARDUINO_USB_CDC_LINE_CODING_EVENT:
				// Serial.printf(
				// 		"CDC LINE CODING: bit_rate: %lu, data_bits: %u, stop_bits: %u, parity: %u\n", data->line_coding.bit_rate, data->line_coding.data_bits,
				// 		data->line_coding.stop_bits, data->line_coding.parity);
				break;
			case ARDUINO_USB_CDC_RX_EVENT:
				while (USBSerial.available())
				{
					esp_task_wdt_reset();
#ifndef DETACH_USB_FROM_MAIN_PROTOCOL
					uint8_t c = USBSerial.read();
					if (mcu_com_rx_cb(c))
					{
						if (BUFFER_FULL(usb_rx))
						{
							STREAM_OVF(c);
						}

						BUFFER_ENQUEUE(usb_rx, &c);
					}
#else
					mcu_usb_rx_cb((uint8_t)USBSerial.read());
#endif
				}
				break;
			case ARDUINO_USB_CDC_RX_OVERFLOW_EVENT:
				// Serial.printf("CDC RX Overflow of %d bytes", data->rx_overflow.dropped_bytes);
				break;

			default:
				break;
			}
		}
	}

	void mcu_usb_init(void)
	{
		USB.onEvent(usbEventCallback);
		USBSerial.onEvent(usbEventCallback);

		USBSerial.begin();
		USB.begin();
	}

	void mcu_usb_dotasks(void)
	{
		while (USBSerial.available())
		{
			esp_task_wdt_reset();
#ifndef DETACH_USB_FROM_MAIN_PROTOCOL
			uint8_t c = USBSerial.read();
			if (mcu_com_rx_cb(c))
			{
				if (BUFFER_FULL(usb_rx))
				{
					STREAM_OVF(c);
				}

				BUFFER_ENQUEUE(usb_rx, &c);
			}
#else
			mcu_usb_rx_cb((uint8_t)USBSerial.read());
#endif
		}
	}

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
			USBSerial.write(tmp, r);
			USBSerial.flush();
		}
	}

#endif
}
#endif
