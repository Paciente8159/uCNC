/*
    Name: softspi.c
    Description: A software based UART library for µCNC.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 23-03-2022

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/
#include "softspi.h"

uint8_t softspi_xmit(const softspi_port_t *port_ptr, uint8_t c)
{
    mcu_disable_global_isr();
    softspi_port_t port;
    rom_memcpy(&port, port_ptr, sizeof(softspi_port_t));
    bool clk = false;
    if ((port.spimode & 0x2))
    {
        clk = true;
    }

    port.clk(clk);

    uint8_t counter = 8;
    do
    {
        mcu_delay_us(port.delay);
        if (c & 0x80)
        {
            port.mosi(true);
        }
        else
        {
            port.mosi(false);
        }
        clk = !clk;
        port.clk(clk);
        c <<= 1;
        mcu_delay_us(port.delay);
        c |= port.miso() ? 1 : 0;
        clk = !clk;
        port.clk(clk);
    } while (--counter);
    mcu_enable_global_isr();

    return c;
}
