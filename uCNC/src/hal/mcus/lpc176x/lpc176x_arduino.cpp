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
}
#endif
