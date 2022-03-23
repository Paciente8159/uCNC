/*
    Name: tmc.c
    Description: TMC driver support µCNC.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 20-03-2022

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#include "../cnc.h"
#include "softuart.h"
#include "tmc/tmcdriver.h"

#define TMC_UARTBAUD 9600

// driver communications declarations
// UART
#define TMC1_STEPPER_DECL(CHANNEL) SOFTUART(tmc##CHANNEL##_uart, TMC_UARTBAUD, STEPPER##CHANNEL##_UART_TX, STEPPER##CHANNEL##_UART_TX)
// SPI
#define TMC2_STEPPER_DECL(CHANNEL) SOFTSPI(tmc##CHANNEL_spi)
#define _TMC_STEPPER_DECL(TYPE, CHANNEL) TMC##TYPE##_STEPPER_DECL(CHANNEL)
#define TMC_STEPPER_DECL(TYPE, CHANNEL) _TMC_STEPPER_DECL(TYPE, CHANNEL)

// driver communications init
// UART
#define TMC1_STEPPER_INIT(CHANNEL) softuart_init(&tmc##CHANNEL##_uart)
// SPI
#define TMC2_STEPPER_INIT(CHANNEL) softspi_init(&tmc##CHANNEL##_spi)
#define _TMC_STEPPER_INIT(TYPE, CHANNEL) TMC##TYPE##_STEPPER_INIT(CHANNEL)
#define TMC_STEPPER_INIT(TYPE, CHANNEL) _TMC_STEPPER_INIT(TYPE, CHANNEL)

// driver communications read/write
// UART
#define TMC1_STEPPER_RW(CHANNEL)                                             \
    static void tmc##CHANNEL##_rw(uint8_t *data, uint8_t wlen, uint8_t rlen) \
    {                                                                        \
        for (uint8_t i = 0; i < wlen; i++)                                   \
        {                                                                    \
            softuart_putc(&tmc##CHANNEL##_uart, data[i]);                    \
        }                                                                    \
        for (uint8_t i = 0; i < rlen; i++)                                   \
        {                                                                    \
            data[i] = softuart_getc(&tmc##CHANNEL##_uart);                   \
        }                                                                    \
    }
// SPI
#define TMC2_STEPPER_RW(CHANNEL)                                             \
    static void tmc##CHANNEL##_rw(uint8_t *data, uint8_t wlen, uint8_t rlen) \
    {                                                                        \
        mcu_clear_output(STEPPER##CHANNEL##_SPI_CS);                         \
        for (uint8_t i = 0; i < wlen; i++)                                   \
        {                                                                    \
            data[i] = softspi_xmit(&tmc##CHANNEL##_uart, data[i]);           \
        }                                                                    \
        mcu_set_output(STEPPER##CHANNEL##_SPI_CS);                           \
    }
#define _TMC_STEPPER_RW(TYPE, CHANNEL) TMC##TYPE##_STEPPER_RW(CHANNEL)
#define TMC_STEPPER_RW(TYPE, CHANNEL) _TMC_STEPPER_RW(TYPE, CHANNEL)

#ifdef STEPPER0_HAS_TMC
TMC_STEPPER_DECL(STEPPER0_TMC_INTERFACE, 0);
TMC_STEPPER_RW(STEPPER0_TMC_INTERFACE, 0);
#endif

#define TMC_STEPPER_PORT(PORT) &tmc##PORT

void tmcdrivers_init(void)
{
#ifdef STEPPER0_HAS_TMC
    TMC_STEPPER_INIT(STEPPER0_TMC_INTERFACE, 0);
    tmc_driver_t tmc0_driver = {.type = STEPPER0_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc0_rw};
    tmc_init(&tmc0_driver);
    tmc_set_current(&tmc0_driver, STEPPER0_CURRENT_MA, STEPPER0_RSENSE, STEPPER0_HOLD_MULT);
    tmc_set_microstep(&tmc0_driver, STEPPER0_MICROSTEP);
    tmc_set_stealthshop(&tmc0_driver, STEPPER0_STEALTHCHOP_THERSHOLD);
    tmc_set_stepinterpol(&tmc0_driver, STEPPER0_ENABLE_INTERPLATION);
#endif
}

/*custom gcode commands*/
#if defined(ENABLE_PARSER_MODULES) && defined(STEPPER0_HAS_TMC)
// this ID must be unique for each code
#define M350 1350

uint8_t m350_parse(unsigned char c, uint8_t word, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
uint8_t m350_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);

CREATE_LISTENER(gcode_parse_delegate, m350_parse);
CREATE_LISTENER(gcode_exec_delegate, m350_exec);

// this just parses and acceps the code
uint8_t m350_parse(unsigned char word, uint8_t code, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (word == 'M' && value == 350)
    {
        if (cmd->group_extended != 0)
        {
            // there is a collision of custom gcode commands (only one per line can be processed)
            return STATUS_GCODE_MODAL_GROUP_VIOLATION;
        }
        // tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
        cmd->group_extended = M350;
        return STATUS_OK;
    }

    // if this is not catched by this parser, just send back the error so other extenders can process it
    return error;
}

// this actually performs 2 steps in 1 (validation and execution)
uint8_t m350_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (cmd->group_extended == M350)
    {
        itp_sync();
        if (!cmd->words)
        {
            int32_t val = -1;
            // if no additional args then print the
            serial_print_str(__romstr__("[MICROSTEPS:"));
            serial_putc('X');
#ifdef STEPPER0_HAS_TMC
            tmc_driver_t tmc0_driver = {.type = STEPPER0_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc0_rw};
            val = tmc_get_microstep(&tmc0_driver);
#endif
            serial_print_int(val);
            serial_putc(',');
            serial_putc('Y');
            val = -1;
#ifdef STEPPER1_HAS_TMC
            val = tmc_get_microstep(&tmc1_driver);
#endif
            serial_print_int(val);
            serial_putc(',');
            serial_putc('Z');
            val = -1;
#ifdef STEPPER2_HAS_TMC
            val = tmc_get_microstep(&tmc2_driver);
#endif
            serial_print_int(val);
            serial_putc(',');
            serial_putc('A');
            val = -1;
#ifdef STEPPER3_HAS_TMC
            val = tmc_get_microstep(&tmc3_driver);
#endif
            serial_print_int(val);
            serial_putc(',');
            serial_putc('B');
            val = -1;
#ifdef STEPPER4_HAS_TMC
            val = tmc_get_microstep(&tmc4_driver);
#endif
            serial_print_int(val);
            serial_putc(',');
            serial_putc('C');
            val = -1;
#ifdef STEPPER5_HAS_TMC
            val = tmc_get_microstep(&tmc5_driver);
#endif
            serial_print_int(val);
            serial_putc(',');
            serial_putc('I');
            val = -1;
#ifdef STEPPER6_HAS_TMC
            val = tmc_get_microstep(&tmc6_driver);
#endif
            serial_print_int(val);
            serial_putc(',');
            serial_putc('J');
            val = -1;
#ifdef STEPPER7_HAS_TMC
            val = tmc_get_microstep(&tmc7_driver);
#endif
            serial_putc(']');
            serial_print_str(MSG_EOL);
        }

#ifdef STEPPER0_HAS_TMC
        if (CHECKFLAG(cmd->words, GCODE_WORD_X))
        {
            tmc_driver_t tmc0_driver = {.type = STEPPER0_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc0_rw};
            tmc_set_microstep(&tmc0_driver, (uint8_t)words->xyzabc[0]);
        }
#endif
#ifdef STEPPER1_HAS_TMC
        if (CHECKFLAG(cmd->words, GCODE_WORD_Y))
        {
            tmc_set_microstep(&tmc1_driver, (uint8_t)words->xyzabc[1]);
        }
#endif
#ifdef STEPPER2_HAS_TMC
        if (CHECKFLAG(cmd->words, GCODE_WORD_Z))
        {
            tmc_set_microstep(&tmc2_driver, (uint8_t)words->xyzabc[2]);
        }
#endif
#ifdef STEPPER3_HAS_TMC
        if (CHECKFLAG(cmd->words, GCODE_WORD_A))
        {
            tmc_set_microstep(&tmc3_driver, (uint8_t)words->xyzabc[3]);
        }
#endif
#ifdef STEPPER4_HAS_TMC
        if (CHECKFLAG(cmd->words, GCODE_WORD_B))
        {
            tmc_set_microstep(&tmc4_driver, (uint8_t)words->xyzabc[4]);
        }
#endif
#ifdef STEPPER5_HAS_TMC
        if (CHECKFLAG(cmd->words, GCODE_WORD_C))
        {
            tmc_set_microstep(&tmc5_driver, (uint8_t)words->xyzabc[5]);
        }
#endif
#ifdef STEPPER6_HAS_TMC
        if (CHECKFLAG(cmd->words, GCODE_WORD_I))
        {
            tmc_set_microstep(&tmc6_driver, (uint8_t)words->ijk[0]);
        }
#endif
#ifdef STEPPER7_HAS_TMC
        if (CHECKFLAG(cmd->words, GCODE_WORD_J))
        {
            tmc_set_microstep(&tmc7_driver, (uint8_t)words->ijk[1]);
        }
#endif
        return STATUS_OK;
    }

    return STATUS_GOCDE_EXTENDED_UNSUPPORTED;
}

#endif
