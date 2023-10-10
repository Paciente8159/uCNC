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

extern "C"
{
#include "../../../cnc.h"
#ifdef USE_ARDUINO_CDC
	DECL_BUFFER(uint8_t, usb_rx, RX_BUFFER_SIZE);

	void mcu_usb_dotasks(void)
	{
		MSC_RunDeferredCommands();
		while (UsbSerial.available() > 0)
		{
#ifndef DETACH_USB_FROM_MAIN_PROTOCOL
			uint8_t c = (uint8_t)UsbSerial.read();
			if (mcu_com_rx_cb(c))
			{
				if (BUFFER_FULL(usb_rx))
				{
					c = OVF;
				}

				*(BUFFER_NEXT_FREE(usb_rx)) = c;
				BUFFER_STORE(usb_rx);
			}

#else
			mcu_usb_rx_cb((uint8_t)UsbSerial.read());
#endif
		}
	}

	void mcu_usb_init(void)
	{
		USB_Init();			// USB Initialization
		USB_Connect(false); // USB clear connection
		USB_Connect(true);
		uint32_t usb_timeout = mcu_millis() + 2000;
		while (!USB_Configuration && usb_timeout > mcu_millis())
		{
			mcu_delay_us(50);
			mcu_usb_dotasks();
#if ASSERT_PIN(ACTIVITY_LED)
			io_toggle_output(ACTIVITY_LED); // Flash quickly during USB initialization
#endif
		}
		UsbSerial.begin(BAUDRATE);
	}

#ifndef USB_TX_BUFFER_SIZE
#define USB_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, usb_tx, USB_TX_BUFFER_SIZE);

	void mcu_usb_flush(void)
	{

		while (!BUFFER_EMPTY(usb_tx))
		{
			uint8_t tmp[USB_TX_BUFFER_SIZE];
			uint8_t r;

			BUFFER_READ(usb_tx, tmp, USB_TX_BUFFER_SIZE, r);
#ifdef MCU_HAS_USB
			UsbSerial.write((char *)tmp, r);
			UsbSerial.flushTX();
#endif
		}
	}

	void mcu_usb_putc(uint8_t c)
	{
		while (BUFFER_FULL(usb_tx))
		{
			mcu_usb_flush();
		}
		BUFFER_ENQUEUE(usb_tx, &c);
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

#endif
}
#endif
