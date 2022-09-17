/*
	Name: softuart.h
	Description: A software based UART library for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 06-03-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef SOFTUART_H
#define SOFTUART_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc.h"
#include <stdint.h>
#include <stdbool.h>

	typedef struct softuart_port_
	{
		void (*wait)(void);
		void (*waithalf)(void);
		void (*tx)(bool);
		bool (*rx)(void);
	} softuart_port_t;

#define UART_DELAY(x) (5000000UL / x)
#define SOFTUART_TIMEOUT 20
#define SOFTUART(NAME, BAUD, TXPIN, RXPIN)                              \
	void NAME##_tx(bool state)                                          \
	{                                                                   \
		if (state)                                                      \
		{                                                               \
			mcu_set_output(TXPIN);                                      \
		}                                                               \
		else                                                            \
		{                                                               \
			mcu_clear_output(TXPIN);                                    \
		}                                                               \
	}                                                                   \
	bool NAME##_rx(void)                                                \
	{                                                                   \
		return mcu_get_input(RXPIN);                                    \
	}                                                                   \
	void NAME##_wait(void) { mcu_delay_us((UART_DELAY(BAUD))); }          \
	void NAME##_waithalf(void) { mcu_delay_us((UART_DELAY(BAUD)) >> 1); } \
	__attribute__((used)) softuart_port_t NAME = {.wait = &NAME##_wait, .waithalf = &NAME##_waithalf, .tx = &NAME##_tx, .rx = &NAME##_rx};

	void softuart_putc(softuart_port_t *port, uint8_t c);
	int16_t softuart_getc(softuart_port_t *port, uint32_t ms_timeout);

#ifdef __cplusplus
}
#endif

#endif