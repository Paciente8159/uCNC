/*
    Name: softuart.c
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
#include "softuart.h"

void softuart_init(const softuart_port_t *port_ptr)
{
    softuart_port_t port;
    rom_memcpy(&port, port_ptr, sizeof(softuart_port_t));
    port.tx(true);
}

void softuart_putc(const softuart_port_t *port_ptr, char c)
{
    mcu_disable_global_isr();
    softuart_port_t port;
    rom_memcpy(&port, port_ptr, sizeof(softuart_port_t));
    port.tx(false);
    mcu_delay_us(port.baud);
    for (uint8_t bits = 0U; bits < 8U; bits++)
    {
        if (c & (1U << bits))
        {
            port.tx(true);
        }
        else
        {
            port.tx(false);
        }
        mcu_delay_us(port.baud);
    }
    port.tx(true);
    mcu_delay_us(port.baud >> 1);
    mcu_enable_global_isr();
}

char softuart_getc(const softuart_port_t *port_ptr)
{
    mcu_disable_global_isr();
    softuart_port_t port;
    rom_memcpy(&port, port_ptr, sizeof(softuart_port_t));
    uint32_t ms = mcu_millis() + SOFTUART_TIMEOUT;
    while (port.rx() && ms < mcu_millis())
        ;
    mcu_delay_us(port.baud + (port.baud >> 1));
    char val = 0;
    for (uint8_t bits = 0U; bits < 8U; bits++)
    {
        bool state = port.rx();
        if (state)
        {
            val = val | (1U << bits);
        }
        mcu_delay_us(port.baud);
    }
    mcu_delay_us((port.baud >> 1));
    mcu_enable_global_isr();
    return val;
}
