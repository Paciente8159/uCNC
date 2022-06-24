/*
	Name: esp8266_uart.cpp
	Description: Contains all Arduino ESP8266 C++ to C functions used by UART in µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 24-06-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../../cnc_config.h"
#include <Arduino.h>
#include <stdbool.h>

#if (INTERFACE == INTERFACE_USART)

extern "C"
{
	void esp8266_com_init(int baud)
	{
		Serial.begin(baud);
	}

	char esp8266_com_read(void)
	{
		return Serial.read();
	}

	void esp8266_com_write(char c)
	{
		Serial.write(c);
	}

	bool esp8266_com_rx_ready(void)
	{
		return (Serial.available() != 0);
	}

	bool esp8266_com_tx_ready(void)
	{
		return (Serial.availableForWrite() != 0);
	}

	void esp8266_com_flush(void)
	{
		Serial.flush();
	}
}

#endif
