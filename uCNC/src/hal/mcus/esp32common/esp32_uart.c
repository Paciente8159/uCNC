/*
	Name: esp32_uart.c
	Description: Implements the µCNC uart shim for ESP32, ESP32S3 and ESP32C3.

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

#include "../../../cnc.h"

#if (ESP32)
#include "driver/uart.h"

#ifdef MCU_HAS_UART
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);

void mcu_uart_init(void)
{
	const uart_config_t uartconfig = {
			.baud_rate = BAUDRATE,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
			.source_clk = UART_SCLK_APB};
	// We won't use a buffer for sending data.
	uart_param_config(UART_PORT, &uartconfig);
	uart_set_pin(UART_PORT, TX_BIT, RX_BIT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void mcu_uart_start(void)
{
	uart_driver_install(UART_PORT, MAX((RX_BUFFER_SIZE * 2), (UART_FIFO_LEN + 1)), MAX((UART_TX_BUFFER_SIZE * 2), (UART_FIFO_LEN + 1)), 0, NULL, 0);
}

void mcu_uart_dotasks(void)
{
	uint8_t rxdata[RX_BUFFER_SIZE];
	uint16_t rxlen = uart_read_bytes(UART_PORT, rxdata, RX_BUFFER_CAPACITY, 0);
	for (uint16_t i = 0; i < rxlen; i++)
	{
		uint8_t c = (uint8_t)rxdata[i];
#ifndef DETACH_UART_FROM_MAIN_PROTOCOL
		if (mcu_com_rx_cb(c))
		{
			if (BUFFER_FULL(uart_rx))
			{
				c = OVF;
			}

			BUFFER_ENQUEUE(uart_rx, &c);
		}
#else
		mcu_uart_rx_cb(c);
#endif
	}
}

uint8_t mcu_uart_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(uart_rx, &c);
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
	while (BUFFER_FULL(uart_tx))
	{
		mcu_uart_flush();
	}
	BUFFER_ENQUEUE(uart_tx, &c);
}

void mcu_uart_flush(void)
{
	while (!BUFFER_EMPTY(uart_tx))
	{
		uint8_t tmp[UART_TX_BUFFER_SIZE + 1];
		uint8_t *p = tmp;
		memset(tmp, 0, sizeof(tmp));
		uint8_t r;

		BUFFER_READ(uart_tx, tmp, UART_TX_BUFFER_SIZE, r);
		while (r)
		{
			int sent = uart_write_bytes(UART_PORT, p, r);
			if (sent < 0)
			{
				break; /*error*/
			}
			else
			{
				p += sent;
				r -= sent;
			}
		}
	}
}

#endif

#ifdef MCU_HAS_UART2
#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart2_rx, RX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart2_tx, UART2_TX_BUFFER_SIZE);
void mcu_uart2_init()
{
	const uart_config_t uartconfig = {
			.baud_rate = BAUDRATE2,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
			.source_clk = UART_SCLK_APB};
	// We won't use a buffer for sending data.
	uart_param_config(UART2_PORT, &uartconfig);
	uart_set_pin(UART2_PORT, TX2_BIT, RX2_BIT, UART2_PIN_NO_CHANGE, UART2_PIN_NO_CHANGE);
}

void mcu_uart2_start()
{
	uart_driver_install(UART2_PORT, MAX((RX2_BUFFER_SIZE * 2), (UART_FIFO_LEN + 1)), MAX((UART2_TX_BUFFER_SIZE * 2), (UART_FIFO_LEN + 1)), 0, NULL, 0);
}

void mcu_uart2_dotasks()
{
	uint8_t rxdata[RX_BUFFER_SIZE];
	uint16_t rxlen = uart_read_bytes(UART2_PORT, rxdata, RX_BUFFER_CAPACITY, 0);
	for (uint16_t i = 0; i < rxlen; i++)
	{
		uint8_t c = (uint8_t)rxdata[i];
#ifndef DETACH_UART2_FROM_MAIN_PROTOCOL
		if (mcu_com_rx_cb(c))
		{
			if (BUFFER_FULL(uart2_rx))
			{
				c = OVF;
			}

			BUFFER_ENQUEUE(uart2_rx, &c);
		}
#else
		mcu_uart2_rx_cb(c);
#endif
	}
}

uint8_t mcu_uart2_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(uart2_rx, &c);
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
	while (BUFFER_FULL(uart2_tx))
	{
		mcu_uart2_flush();
	}
	BUFFER_ENQUEUE(uart2_tx, &c);
}

void mcu_uart2_flush(void)
{
	while (!BUFFER_EMPTY(uart2_tx))
	{
		uint8_t tmp[UART2_TX_BUFFER_SIZE + 1];
		uint8_t *p = tmp;
		memset(tmp, 0, sizeof(tmp));
		uint8_t r;

		BUFFER_READ(uart2_tx, tmp, UART2_TX_BUFFER_SIZE, r);
		while (r)
		{
			int sent = uart_write_bytes(UART2_PORT, p, r);
			if (sent < 0)
			{
				break; /*error*/
			}
			else
			{
				p += sent;
				r -= sent;
			}
		}
	}
}
#endif

#endif