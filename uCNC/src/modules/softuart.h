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
        const uint8_t baud;
        void (*tx)(bool);
        bool (*rx)(void);
    } softuart_port_t;

#define SOFTBAUD(x) (1000000 / x)
#define SOFTUART_TIMEOUT 20
#define SOFTUART(NAME, BAUD, TXPIN, RXPIN) \
    void NAME##_tx(bool state)             \
    {                                      \
        if (state)                         \
        {                                  \
            mcu_set_output(TXPIN);         \
        }                                  \
        else                               \
        {                                  \
            mcu_clear_output(TXPIN);       \
        }                                  \
    }                                      \
    bool NAME##_rx(void)                   \
    {                                      \
        return mcu_get_input(RXPIN);       \
    }                                      \
    const softuart_port_t __rom__ NAME = {.baud = SOFTBAUD(BAUD), .tx = &NAME##_tx, .rx = &NAME##_rx};

    void softuart_putc(const softuart_port_t *port_ptr, char c);
    char softuart_getc(const softuart_port_t *port_ptr);

#ifdef __cplusplus
}
#endif

#endif