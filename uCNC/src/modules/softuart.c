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

void softuart_putc(softuart_port_t *port, char c)
{
    port->tx(false);
    mcu_delay_us(port->baud);
    uint8_t bits = 8;
    do
    {
        if (c & 0x01)
        {
            port->tx(true);
        }
        else
        {
            port->tx(false);
        }
        c >>= 1;
        mcu_delay_us(port->baud);
    } while (--bits);
    port->tx(true);
    mcu_delay_us(port->baud);
}

char softuart_getc(softuart_port_t *port)
{
    uint16_t ms = SOFTUART_TIMEOUT * 1000;
    while (port->rx())
    {
        mcu_delay_us(1);
        if (!ms--)
        {
            return 0xFF;
        }
    }
    mcu_delay_us((port->baud >> 1));
    char val = 0;
    uint8_t bits = 8;
    uint8_t mask = 0x01;
    do
    {
        mcu_delay_us(port->baud);
        if (port->rx())
        {
            val |= mask;
        }
        mask <<= 1;
    } while (--bits);
    mcu_delay_us((port->baud >> 1));
    return val;
}
