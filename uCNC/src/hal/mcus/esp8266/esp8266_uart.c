/*
	Name: esp8266_uart.c
	Description: Implements UART for ESP8266.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 04-02-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (MCU == MCU_ESP8266)
#include "ets_sys.h"
#include "uart_register.h"
#include <uart.h>
#include <esp8266_peri.h>

#ifdef MCU_HAS_UART
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
uint8_t mcu_uart_getc(void)
{
	uint8_t c = 0;
	BUFFER_TRY_DEQUEUE(uart_rx, &c);
	return c;
}

uint8_t mcu_uart_available(void)
{
	return BUFFER_READ_AVAILABLE(uart_rx);
}

void mcu_uart_clear(void)
{
	BUFFER_CLEAR(uart_rx);
}

void mcu_uart_putc(uint8_t c)
{
	while (!BUFFER_TRY_ENQUEUE(uart_tx, &c))
	{
		mcu_uart_flush();
	}
}

void mcu_uart_flush(void)
{
	uint8_t c = 0;

	while (BUFFER_TRY_DEQUEUE(uart_tx, &c))
	{
		while (((USS(UART_PORT) >> USTXC) & UART_TXFIFO_CNT) >= 0x7f)
		{
			esp_yield();
		}
		USF(UART_PORT) = c;
	}
}

/**
 * ISR implementation (not used)
 * The ESP8266 has a UART FIFO. Should be enough to coupled with the Software buffer and the mcu task loop
 * */
void IRAM_ATTR mcu_uart_isr(void *arg)
{
	uint32_t usis = USIS(UART_PORT);
	uint8_t c = 0;

	if (usis & ((1 << UIFF) | (1 << UITO)))
	{
		while ((USS(UART_PORT) >> USRXC) & 0xFF)
		{
			// system_soft_wdt_feed();
#ifndef DETACH_UART_FROM_MAIN_PROTOCOL
			c = (uint8_t)USF(UART_PORT);
			if (mcu_com_rx_cb(c))
			{
				if (!BUFFER_TRY_ENQUEUE(uart_rx, &c))
				{
					USIC(UART_PORT) = usis;
					STREAM_OVF(c);
				}
			}
#else
			mcu_uart_rx_cb((uint8_t)USF(UART_PORT));
#endif
		}
	}

	if (usis & ((1 << UIOF) | (1 << UIFR) | (1 << UIPE)))
	{
		USIC(UART_PORT) = usis;
		STREAM_OVF(c);
	}

	USIC(UART_PORT) = usis;
}

static void mcu_uart_process()
{
#ifdef MCU_HAS_UART
	uint32_t timeout = 1;
	__TIMEOUT_MS__(timeout)
	{
		if (!((USS(UART_PORT) >> USRXC) & 0xFF))
		{
			break;
		}
#ifndef DETACH_UART_FROM_MAIN_PROTOCOL
		uint8_t c = (uint8_t)USF(UART_PORT);
		if (mcu_com_rx_cb(c))
		{
			if (!BUFFER_TRY_ENQUEUE(uart_rx, &c))
			{
				STREAM_OVF(c);
			}
		}
#else
		mcu_uart_rx_cb((uint8_t)USF(UART_PORT));
#endif
	}
#endif
}
#endif

#ifdef MCU_HAS_UART2
#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart2_rx, RX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart2_tx, UART2_TX_BUFFER_SIZE);
uint8_t mcu_uart2_getc(void)
{
	uint8_t c = 0;
	BUFFER_TRY_DEQUEUE(uart2_rx, &c);
	return c;
}

uint8_t mcu_uart2_available(void)
{
	return BUFFER_READ_AVAILABLE(uart2_rx);
}

void mcu_uart2_clear(void)
{
	BUFFER_CLEAR(uart2_rx);
}

void mcu_uart2_putc(uint8_t c)
{
	while (!BUFFER_TRY_ENQUEUE(uart2_tx, &c))
	{
		mcu_uart2_flush();
	}
}

void mcu_uart2_flush(void)
{
	uint8_t c = 0;
	while (BUFFER_TRY_DEQUEUE(uart2_tx, &c))
	{
		while (((USS(UART2_PORT) >> USTXC) & UART_TXFIFO_CNT) >= 0x7f)
		{
			esp_yield();
		}

		USF(UART2_PORT) = c;
	}
}

/**
 * ISR implementation (not used)
 * The ESP8266 has a UART2 FIFO. Should be enough to coupled with the Software buffer and the mcu task loop
 * */
void IRAM_ATTR mcu_uart2_isr(void *arg)
{
	uint32_t usis = USIS(UART2_PORT);

	if (usis & ((1 << UIFF) | (1 << UITO)))
	{
		while ((USS(UART2_PORT) >> USRXC) & 0xFF)
		{
			// system_soft_wdt_feed();
#ifndef DETACH_UART2_FROM_MAIN_PROTOCOL
			uint8_t c = (uint8_t)USF(UART2_PORT);
			if (mcu_com_rx_cb(c))
			{
				if (!BUFFER_TRY_ENQUEUE(uart2_rx, &c))
				{
					USIC(UART2_PORT) = usis;
					STREAM_OVF(c);
				}
			}
#else
			mcu_uart2_rx_cb((uint8_t)USF(UART2_PORT));
#endif
		}
	}

	if (usis & ((1 << UIOF) | (1 << UIFR) | (1 << UIPE)))
	{
		USIC(UART2_PORT) = usis;
		STREAM_OVF(c);
	}

	USIC(UART2_PORT) = usis;
}

static void mcu_uart2_process()
{
#ifdef MCU_HAS_UART2
	uint32_t timeout = 1;
	__TIMEOUT_MS__(timeout)
	{
		if (!((USS(UART2_PORT) >> USRXC) & 0xFF))
		{
			break;
		}
#ifndef DETACH_UART2_FROM_MAIN_PROTOCOL
		uint8_t c = (uint8_t)USF(UART2_PORT);
		if (mcu_com_rx_cb(c))
		{
			if (!BUFFER_TRY_ENQUEUE(uart2_rx, &c))
			{
				STREAM_OVF(c);
			}
		}
#else
		mcu_uart2_rx_cb((uint8_t)USF(UART2_PORT));
#endif
	}
#endif
}
#endif

void mcu_uart_dotasks(void)
{
#ifdef MCU_HAS_UART
	mcu_uart_process();
#endif
#ifdef MCU_HAS_UART2
	mcu_uart2_process();
#endif
}

void mcu_uart_init(void)
{
#ifdef MCU_HAS_UART
	ETS_UART_INTR_DISABLE();
	mcu_config_input(RX);
	mcu_config_input(TX);
	mcu_config_af(RX, SPECIAL);
#if UART_PORT == 0
	mcu_config_af(TX, (TX_BIT == 1) ? FUNCTION_0 : FUNCTION_4);
#else
	mcu_config_af(TX, SPECIAL);
#endif
#ifndef UART_PIN_SWAP
	IOSWAP &= ~(1 << IOSWAPU0);
#else
	IOSWAP |= (1 << IOSWAPU0);
#endif
	USD(UART_PORT) = (ESP8266_CLOCK / BAUDRATE);
	USC0(UART_PORT) = UART_8N1;
	USC1(UART_PORT) = 0;
	USIC(UART_PORT) = 0xffff;
	USIE(UART_PORT) = 0;
	/*With ISR*/
	// USC1(UART_PORT) = (16 << UCFFT) | (1 << UCTOE) | (16 << UCTOT);
	// USIC(UART_PORT) = 0xffff;
	// USIE(UART_PORT) = (1 << UIFF) | (1 << UIOF) | (1 << UIFR) | (1 << UIPE) | (1 << UITO);
	// ETS_UART_INTR_ATTACH(mcu_uart_isr, NULL);
	// ETS_UART_INTR_ENABLE();
#endif
}

void mcu_uart2_init(void)
{
#ifdef MCU_HAS_UART2
	ETS_UART_INTR_DISABLE();
	mcu_config_input(RX2);
	mcu_config_input(TX2);
	mcu_config_af(RX2, SPECIAL);
#if UART_PORT == 0
	mcu_config_af(TX2, (TX2_BIT == 1) ? FUNCTION_0 : FUNCTION_4);
#else
	mcu_config_af(TX2, SPECIAL);
#endif
#ifndef UART2_PIN_SWAP
	IOSWAP &= ~(1 << IOSWAPU0);
#else
	IOSWAP |= (1 << IOSWAPU0);
#endif
	USD(UART2_PORT) = (ESP8266_CLOCK / BAUDRATE2);
	USC0(UART2_PORT) = UART_8N1;
	USC1(UART2_PORT) = 0;
	USIC(UART2_PORT) = 0xffff;
	USIE(UART2_PORT) = 0;
#endif
}

#endif