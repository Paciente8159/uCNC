/*
    Name: softspi.h
    Description: A software based SPI library for µCNC.

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

#ifndef SOFTSPI_H
#define SOFTSPI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc.h"
#include <stdint.h>
#include <stdbool.h>

    typedef struct softspi_port_
    {
        uint8_t spimode;
        uint8_t delay;
        void (*clk)(bool);
        void (*mosi)(bool);
        bool (*miso)(void);
    } softspi_port_t;

#define SPIXMITDELAY 2

#define SOFTSPI(NAME, MODE, MOSIPIN, MISOPIN, CLKPIN)         \
    void NAME##_clk(bool state)                               \
    {                                                         \
        if (state)                                            \
        {                                                     \
            mcu_set_output(CLKPIN);                           \
        }                                                     \
        else                                                  \
        {                                                     \
            mcu_clear_output(CLKPIN);                         \
        }                                                     \
    }                                                         \
    void NAME##_mosi(bool state)                              \
    {                                                         \
        if (state)                                            \
        {                                                     \
            mcu_set_output(MOSIPIN);                          \
        }                                                     \
        else                                                  \
        {                                                     \
            mcu_clear_output(MOSIPIN);                        \
        }                                                     \
    }                                                         \
    bool NAME##_miso(void) { return mcu_get_input(MISOPIN); } \
    __attribute__((used)) softspi_port_t NAME = {.spimode = MODE, .delay = SPIXMITDELAY, .clk = &NAME##_clk, .mosi = &NAME##_mosi, .miso = &NAME##_miso};

    uint8_t softspi_xmit(softspi_port_t *port, uint8_t c);

#ifdef __cplusplus
}
#endif

#endif