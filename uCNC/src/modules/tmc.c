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

#define TMC_UARTBAUD 38400

// driver communications declarations
// UART
#define TMC1_STEPPER_DECL(CHANNEL) SOFTUART(tmc##CHANNEL##_uart, TMC_UARTBAUD, STEPPER##CHANNEL##_UART_TX, STEPPER##CHANNEL##_UART_TX)
// SPI
#define TMC2_STEPPER_DECL(CHANNEL) SOFTSPI(tmc##CHANNEL_spi)
#define _TMC_STEPPER_DECL(TYPE, CHANNEL) TMC##TYPE##_STEPPER_DECL(CHANNEL)
#define TMC_STEPPER_DECL(TYPE, CHANNEL) _TMC_STEPPER_DECL(TYPE, CHANNEL)

// driver communications read/write
// UART
#define TMC1_STEPPER_RW(CHANNEL)                                             \
    static void tmc##CHANNEL##_rw(uint8_t *data, uint8_t wlen, uint8_t rlen) \
    {                                                                        \
        mcu_disable_global_isr();                                            \
        mcu_config_output(STEPPER##CHANNEL##_UART_TX);                       \
        mcu_set_output(STEPPER##CHANNEL##_UART_TX);                          \
        for (uint8_t i = 0; i < wlen; i++)                                   \
        {                                                                    \
            softuart_putc(&tmc##CHANNEL##_uart, data[i]);                    \
        }                                                                    \
        mcu_config_input(STEPPER##CHANNEL##_UART_TX);                        \
        for (uint8_t i = 0; i < rlen; i++)                                   \
        {                                                                    \
            data[i] = softuart_getc(&tmc##CHANNEL##_uart);                   \
        }                                                                    \
        mcu_config_output(STEPPER##CHANNEL##_UART_TX);                       \
        mcu_enable_global_isr();                                             \
        cnc_delay_ms(10);                                                    \
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
#ifdef STEPPER1_HAS_TMC
TMC_STEPPER_DECL(STEPPER1_TMC_INTERFACE, 1);
TMC_STEPPER_RW(STEPPER1_TMC_INTERFACE, 1);
#endif
#ifdef STEPPER2_HAS_TMC
TMC_STEPPER_DECL(STEPPER2_TMC_INTERFACE, 2);
TMC_STEPPER_RW(STEPPER2_TMC_INTERFACE, 2);
#endif
#ifdef STEPPER3_HAS_TMC
TMC_STEPPER_DECL(STEPPER3_TMC_INTERFACE, 3);
TMC_STEPPER_RW(STEPPER3_TMC_INTERFACE, 3);
#endif
#ifdef STEPPER4_HAS_TMC
TMC_STEPPER_DECL(STEPPER4_TMC_INTERFACE, 4);
TMC_STEPPER_RW(STEPPER4_TMC_INTERFACE, 4);
#endif
#ifdef STEPPER5_HAS_TMC
TMC_STEPPER_DECL(STEPPER5_TMC_INTERFACE, 5);
TMC_STEPPER_RW(STEPPER5_TMC_INTERFACE, 5);
#endif
#ifdef STEPPER6_HAS_TMC
TMC_STEPPER_DECL(STEPPER6_TMC_INTERFACE, 6);
TMC_STEPPER_RW(STEPPER6_TMC_INTERFACE, 6);
#endif
#ifdef STEPPER7_HAS_TMC
TMC_STEPPER_DECL(STEPPER7_TMC_INTERFACE, 7);
TMC_STEPPER_RW(STEPPER7_TMC_INTERFACE, 7);
#endif

#define TMC_STEPPER_PORT(PORT) &tmc##PORT

void tmcdrivers_init(void)
{
#ifdef STEPPER0_HAS_TMC
    tmc_driver_t tmc0_driver = {.type = STEPPER0_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc0_rw};
    tmc_init(&tmc0_driver);
    tmc_set_current(&tmc0_driver, STEPPER0_CURRENT_MA, STEPPER0_RSENSE, STEPPER0_HOLD_MULT);
    tmc_set_microstep(&tmc0_driver, STEPPER0_MICROSTEP);
    tmc_set_stealthchop(&tmc0_driver, STEPPER0_STEALTHCHOP_THERSHOLD);
    tmc_set_stepinterpol(&tmc0_driver, STEPPER0_ENABLE_INTERPLATION);
#endif
#ifdef STEPPER1_HAS_TMC
    tmc_driver_t tmc1_driver = {.type = STEPPER1_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc1_rw};
    tmc_init(&tmc1_driver);
    tmc_set_current(&tmc1_driver, STEPPER1_CURRENT_MA, STEPPER1_RSENSE, STEPPER1_HOLD_MULT);
    tmc_set_microstep(&tmc1_driver, STEPPER1_MICROSTEP);
    tmc_set_stealthchop(&tmc1_driver, STEPPER1_STEALTHCHOP_THERSHOLD);
    tmc_set_stepinterpol(&tmc1_driver, STEPPER1_ENABLE_INTERPLATION);
#endif
#ifdef STEPPER2_HAS_TMC
    tmc_driver_t tmc2_driver = {.type = STEPPER2_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc2_rw};
    tmc_init(&tmc2_driver);
    tmc_set_current(&tmc2_driver, STEPPER2_CURRENT_MA, STEPPER2_RSENSE, STEPPER2_HOLD_MULT);
    tmc_set_microstep(&tmc2_driver, STEPPER2_MICROSTEP);
    tmc_set_stealthchop(&tmc2_driver, STEPPER2_STEALTHCHOP_THERSHOLD);
    tmc_set_stepinterpol(&tmc2_driver, STEPPER2_ENABLE_INTERPLATION);
#endif
#ifdef STEPPER3_HAS_TMC
    tmc_driver_t tmc3_driver = {.type = STEPPER3_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc3_rw};
    tmc_init(&tmc3_driver);
    tmc_set_current(&tmc3_driver, STEPPER3_CURRENT_MA, STEPPER3_RSENSE, STEPPER3_HOLD_MULT);
    tmc_set_microstep(&tmc3_driver, STEPPER3_MICROSTEP);
    tmc_set_stealthchop(&tmc3_driver, STEPPER3_STEALTHCHOP_THERSHOLD);
    tmc_set_stepinterpol(&tmc3_driver, STEPPER3_ENABLE_INTERPLATION);
#endif
#ifdef STEPPER4_HAS_TMC
    tmc_driver_t tmc4_driver = {.type = STEPPER4_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc4_rw};
    tmc_init(&tmc4_driver);
    tmc_set_current(&tmc4_driver, STEPPER4_CURRENT_MA, STEPPER4_RSENSE, STEPPER4_HOLD_MULT);
    tmc_set_microstep(&tmc4_driver, STEPPER4_MICROSTEP);
    tmc_set_stealthchop(&tmc4_driver, STEPPER4_STEALTHCHOP_THERSHOLD);
    tmc_set_stepinterpol(&tmc4_driver, STEPPER4_ENABLE_INTERPLATION);
#endif
#ifdef STEPPER5_HAS_TMC
    tmc_driver_t tmc5_driver = {.type = STEPPER5_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc5_rw};
    tmc_init(&tmc5_driver);
    tmc_set_current(&tmc5_driver, STEPPER5_CURRENT_MA, STEPPER5_RSENSE, STEPPER5_HOLD_MULT);
    tmc_set_microstep(&tmc5_driver, STEPPER5_MICROSTEP);
    tmc_set_stealthchop(&tmc5_driver, STEPPER5_STEALTHCHOP_THERSHOLD);
    tmc_set_stepinterpol(&tmc5_driver, STEPPER5_ENABLE_INTERPLATION);
#endif
#ifdef STEPPER6_HAS_TMC
    tmc_driver_t tmc6_driver = {.type = STEPPER6_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc6_rw};
    tmc_init(&tmc6_driver);
    tmc_set_current(&tmc6_driver, STEPPER6_CURRENT_MA, STEPPER6_RSENSE, STEPPER6_HOLD_MULT);
    tmc_set_microstep(&tmc6_driver, STEPPER6_MICROSTEP);
    tmc_set_stealthchop(&tmc6_driver, STEPPER6_STEALTHCHOP_THERSHOLD);
    tmc_set_stepinterpol(&tmc6_driver, STEPPER6_ENABLE_INTERPLATION);
#endif
#ifdef STEPPER7_HAS_TMC
    tmc_driver_t tmc7_driver = {.type = STEPPER7_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc7_rw};
    tmc_init(&tmc7_driver);
    tmc_set_current(&tmc7_driver, STEPPER7_CURRENT_MA, STEPPER7_RSENSE, STEPPER7_HOLD_MULT);
    tmc_set_microstep(&tmc7_driver, STEPPER7_MICROSTEP);
    tmc_set_stealthchop(&tmc7_driver, STEPPER7_STEALTHCHOP_THERSHOLD);
    tmc_set_stepinterpol(&tmc7_driver, STEPPER7_ENABLE_INTERPLATION);
#endif
}

CREATE_LISTENER(cnc_reset_delegate, tmcdrivers_init);

/*custom gcode commands*/
#if defined(ENABLE_PARSER_MODULES) && defined(ENABLE_TMC_DRIVERS)
// this ID must be unique for each code
#define M350 1350
// this ID must be unique for each code
#define M906 1906
// this ID must be unique for each code
#define M920 1920

uint8_t m350_parse(unsigned char c, uint8_t word, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
uint8_t m350_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
uint8_t m906_parse(unsigned char c, uint8_t word, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
uint8_t m906_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
uint8_t m920_parse(unsigned char c, uint8_t word, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
uint8_t m920_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);

CREATE_LISTENER(gcode_parse_delegate, m350_parse);
CREATE_LISTENER(gcode_exec_delegate, m350_exec);
CREATE_LISTENER(gcode_parse_delegate, m906_parse);
CREATE_LISTENER(gcode_exec_delegate, m906_exec);
CREATE_LISTENER(gcode_parse_delegate, m920_parse);
CREATE_LISTENER(gcode_exec_delegate, m920_exec);

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
            val = -1;
            serial_putc('X');
#ifdef STEPPER0_HAS_TMC
            tmc_driver_t tmc0_driver = {.type = STEPPER0_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc0_rw};
            val = tmc_get_microstep(&tmc0_driver);
#endif
            serial_print_flt(val);
            serial_putc(',');
            val = -1;
            serial_putc('Y');
#ifdef STEPPER1_HAS_TMC
            tmc_driver_t tmc1_driver = {.type = STEPPER1_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc1_rw};
            val = tmc_get_microstep(&tmc1_driver);
#endif
            serial_print_flt(val);
            serial_putc(',');
            val = -1;
            serial_putc('Z');
#ifdef STEPPER2_HAS_TMC
            tmc_driver_t tmc2_driver = {.type = STEPPER2_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc2_rw};
            val = tmc_get_microstep(&tmc2_driver);
#endif
            serial_print_flt(val);
            serial_putc(',');
            val = -1;
            serial_putc('A');
#ifdef STEPPER3_HAS_TMC
            tmc_driver_t tmc3_driver = {.type = STEPPER3_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc3_rw};
            val = tmc_get_microstep(&tmc3_driver);
#endif
            serial_print_flt(val);
            serial_putc(',');
            val = -1;
            serial_putc('B');
#ifdef STEPPER4_HAS_TMC
            tmc_driver_t tmc4_driver = {.type = STEPPER4_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc4_rw};
            val = tmc_get_microstep(&tmc4_driver);
#endif
            serial_print_flt(val);
            serial_putc(',');
            val = -1;
            serial_putc('C');
#ifdef STEPPER5_HAS_TMC
            tmc_driver_t tmc5_driver = {.type = STEPPER5_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc5_rw};
            val = tmc_get_microstep(&tmc5_driver);
#endif
            serial_print_flt(val);
            serial_putc(',');
            val = -1;
            serial_putc('I');
#ifdef STEPPER6_HAS_TMC
            tmc_driver_t tmc6_driver = {.type = STEPPER6_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc6_rw};
            val = tmc_get_microstep(&tmc6_driver);
#endif
            serial_print_flt(val);
            serial_putc(',');
            val = -1;
            serial_putc('J');
#ifdef STEPPER7_HAS_TMC
            tmc_driver_t tmc7_driver = {.type = STEPPER7_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc7_rw};
            val = tmc_get_microstep(&tmc7_driver);
#endif
            serial_print_flt(val);
            serial_putc(']');
            serial_print_str(MSG_EOL);
        }

        if (CHECKFLAG(cmd->words, GCODE_WORD_X))
        {
#ifdef STEPPER0_HAS_TMC
            tmc_driver_t tmc0_driver = {.type = STEPPER0_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc0_rw};
            tmc_set_microstep(&tmc0_driver, (uint8_t)words->xyzabc[0]);
#endif
        }
        if (CHECKFLAG(cmd->words, GCODE_WORD_Y))
        {
#ifdef STEPPER1_HAS_TMC
            tmc_driver_t tmc1_driver = {.type = STEPPER1_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc1_rw};
            tmc_set_microstep(&tmc1_driver, (uint8_t)words->xyzabc[1]);
#endif
        }
        if (CHECKFLAG(cmd->words, GCODE_WORD_Z))
        {
#ifdef STEPPER2_HAS_TMC
            tmc_driver_t tmc2_driver = {.type = STEPPER2_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc2_rw};
            tmc_set_microstep(&tmc2_driver, (uint8_t)words->xyzabc[2]);
#endif
        }
        if (CHECKFLAG(cmd->words, GCODE_WORD_A))
        {
#ifdef STEPPER3_HAS_TMC
            tmc_driver_t tmc3_driver = {.type = STEPPER3_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc3_rw};
            tmc_set_microstep(&tmc3_driver, (uint8_t)words->xyzabc[3]);
#endif
        }
        if (CHECKFLAG(cmd->words, GCODE_WORD_B))
        {
#ifdef STEPPER4_HAS_TMC
            tmc_driver_t tmc4_driver = {.type = STEPPER4_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc4_rw};
            tmc_set_microstep(&tmc4_driver, (uint8_t)words->xyzabc[4]);
#endif
        }
        if (CHECKFLAG(cmd->words, GCODE_WORD_C))
        {
#ifdef STEPPER5_HAS_TMC
            tmc_driver_t tmc5_driver = {.type = STEPPER5_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc5_rw};
            tmc_set_microstep(&tmc5_driver, (uint8_t)words->xyzabc[5]);
#endif
        }
        if (CHECKFLAG(cmd->words, GCODE_WORD_I))
        {
#ifdef STEPPER6_HAS_TMC
            tmc_driver_t tmc6_driver = {.type = STEPPER6_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc6_rw};
            tmc_set_microstep(&tmc6_driver, (uint8_t)words->ijk[0]);
#endif
        }
        if (CHECKFLAG(cmd->words, GCODE_WORD_J))
        {
#ifdef STEPPER7_HAS_TMC
            tmc_driver_t tmc7_driver = {.type = STEPPER7_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc7_rw};
            tmc_set_microstep(&tmc7_driver, (uint8_t)words->ijk[1]);
#endif
        }

        return STATUS_OK;
    }

    return STATUS_GOCDE_EXTENDED_UNSUPPORTED;
}

// this just parses and acceps the code
uint8_t m906_parse(unsigned char word, uint8_t code, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (word == 'M' && value == 906.0f)
    {
        if (cmd->group_extended != 0)
        {
            // there is a collision of custom gcode commands (only one per line can be processed)
            return STATUS_GCODE_MODAL_GROUP_VIOLATION;
        }
        // tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
        cmd->group_extended = M906;
        return STATUS_OK;
    }

    // if this is not catched by this parser, just send back the error so other extenders can process it
    return error;
}

// this actually performs 2 steps in 1 (validation and execution)
uint8_t m906_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (cmd->group_extended == M906)
    {
        itp_sync();
        if (!cmd->words)
        {
            float val;
            // if no additional args then print the
            serial_print_str(__romstr__("[STEPPER CURRENT:"));
            val = -1;
            serial_putc('X');
#ifdef STEPPER0_HAS_TMC
            tmc_driver_t tmc0_driver = {.type = STEPPER0_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc0_rw};
            val = tmc_get_current(&tmc0_driver, STEPPER0_RSENSE);
#endif
            serial_print_flt(val);
            serial_putc(',');
            val = -1;
            serial_putc('Y');
#ifdef STEPPER1_HAS_TMC
            tmc_driver_t tmc1_driver = {.type = STEPPER1_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc1_rw};
            val = tmc_get_current(&tmc1_driver, STEPPER1_RSENSE);
#endif
            serial_print_flt(val);
            serial_putc(',');
            val = -1;
            serial_putc('Z');
#ifdef STEPPER2_HAS_TMC
            tmc_driver_t tmc2_driver = {.type = STEPPER2_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc2_rw};
            val = tmc_get_current(&tmc2_driver, STEPPER2_RSENSE);
#endif
            serial_print_flt(val);
            serial_putc(',');
            val = -1;
            serial_putc('A');
#ifdef STEPPER3_HAS_TMC
            tmc_driver_t tmc3_driver = {.type = STEPPER3_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc3_rw};
            val = tmc_get_current(&tmc3_driver, STEPPER3_RSENSE);
#endif
            serial_print_flt(val);
            serial_putc(',');
            val = -1;
            serial_putc('B');
#ifdef STEPPER4_HAS_TMC
            tmc_driver_t tmc4_driver = {.type = STEPPER4_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc4_rw};
            val = tmc_get_current(&tmc4_driver, STEPPER4_RSENSE);
#endif
            serial_print_flt(val);
            serial_putc(',');
            val = -1;
            serial_putc('C');
#ifdef STEPPER5_HAS_TMC
            tmc_driver_t tmc5_driver = {.type = STEPPER5_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc5_rw};
            val = tmc_get_current(&tmc5_driver, STEPPER5_RSENSE);
#endif
            serial_print_flt(val);
            serial_putc(',');
            val = -1;
            serial_putc('I');
#ifdef STEPPER6_HAS_TMC
            tmc_driver_t tmc6_driver = {.type = STEPPER6_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc6_rw};
            val = tmc_get_current(&tmc6_driver, STEPPER6_RSENSE);
#endif
            serial_print_flt(val);
            serial_putc(',');
            val = -1;
            serial_putc('J');
#ifdef STEPPER7_HAS_TMC
            tmc_driver_t tmc7_driver = {.type = STEPPER7_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc7_rw};
            val = tmc_get_current(&tmc7_driver, STEPPER7_RSENSE);
#endif
            serial_print_flt(val);
            serial_putc(']');
            serial_print_str(MSG_EOL);
        }

        if (CHECKFLAG(cmd->words, GCODE_WORD_X))
        {
#ifdef STEPPER0_HAS_TMC
            tmc_driver_t tmc0_driver = {.type = STEPPER0_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc0_rw};
            tmc_set_current(&tmc0_driver, (uint8_t)words->xyzabc[0], STEPPER0_RSENSE, STEPPER0_HOLD_MULT);
#endif
        }
        if (CHECKFLAG(cmd->words, GCODE_WORD_Y))
        {
#ifdef STEPPER1_HAS_TMC
            tmc_driver_t tmc1_driver = {.type = STEPPER1_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc1_rw};
            tmc_set_current(&tmc1_driver, (uint8_t)words->xyzabc[1], STEPPER1_RSENSE, STEPPER1_HOLD_MULT);
#endif
        }
        if (CHECKFLAG(cmd->words, GCODE_WORD_Z))
        {
#ifdef STEPPER2_HAS_TMC
            tmc_driver_t tmc2_driver = {.type = STEPPER2_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc2_rw};
            tmc_set_current(&tmc2_driver, (uint8_t)words->xyzabc[2], STEPPER2_RSENSE, STEPPER2_HOLD_MULT);
#endif
        }
        if (CHECKFLAG(cmd->words, GCODE_WORD_A))
        {
#ifdef STEPPER3_HAS_TMC
            tmc_driver_t tmc3_driver = {.type = STEPPER3_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc3_rw};
            tmc_set_current(&tmc3_driver, (uint8_t)words->xyzabc[3], STEPPER3_RSENSE, STEPPER3_HOLD_MULT);
#endif
        }
        if (CHECKFLAG(cmd->words, GCODE_WORD_B))
        {
#ifdef STEPPER4_HAS_TMC
            tmc_driver_t tmc4_driver = {.type = STEPPER4_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc4_rw};
            tmc_set_current(&tmc4_driver, (uint8_t)words->xyzabc[4], STEPPER4_RSENSE, STEPPER4_HOLD_MULT);
#endif
        }
        if (CHECKFLAG(cmd->words, GCODE_WORD_C))
        {
#ifdef STEPPER5_HAS_TMC
            tmc_driver_t tmc5_driver = {.type = STEPPER5_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc5_rw};
            tmc_set_current(&tmc5_driver, (uint8_t)words->xyzabc[5], STEPPER5_RSENSE, STEPPER5_HOLD_MULT);
#endif
        }
        if (CHECKFLAG(cmd->words, GCODE_WORD_I))
        {
#ifdef STEPPER6_HAS_TMC
            tmc_driver_t tmc6_driver = {.type = STEPPER6_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc6_rw};
            tmc_set_current(&tmc6_driver, (uint8_t)words->ijk[0], STEPPER6_RSENSE, STEPPER6_HOLD_MULT);
#endif
        }
        if (CHECKFLAG(cmd->words, GCODE_WORD_J))
        {
#ifdef STEPPER7_HAS_TMC
            tmc_driver_t tmc7_driver = {.type = STEPPER7_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc7_rw};
            tmc_set_current(&tmc7_driver, (uint8_t)words->ijk[1], STEPPER7_RSENSE, STEPPER7_HOLD_MULT);
#endif
        }

        return STATUS_OK;
    }

    return STATUS_GOCDE_EXTENDED_UNSUPPORTED;
}

// this just parses and acceps the code
uint8_t m920_parse(unsigned char word, uint8_t code, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (word == 'M' && value == 920.0f)
    {
        if (cmd->group_extended != 0)
        {
            // there is a collision of custom gcode commands (only one per line can be processed)
            return STATUS_GCODE_MODAL_GROUP_VIOLATION;
        }
        // tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
        cmd->group_extended = M920;
        return STATUS_OK;
    }

    // if this is not catched by this parser, just send back the error so other extenders can process it
    return error;
}

// this actually performs 2 steps in 1 (validation and execution)
uint8_t m920_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (cmd->group_extended == M920)
    {
        if (!CHECKFLAG(cmd->words, GCODE_ALL_AXIS | GCODE_IJK_AXIS))
        {
            return STATUS_TMC_CMD_MISSING_ARGS;
        }

        uint32_t reg;

        if (CHECKFLAG(cmd->words, GCODE_WORD_X))
        {
            serial_print_str(__romstr__("[TMCREG X:"));
            reg = (uint32_t)words->xyzabc[0];
            serial_print_int(reg);
            serial_putc(',');
#ifdef STEPPER0_HAS_TMC
            tmc_driver_t tmc0_driver = {.type = STEPPER0_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc0_rw};
            if (CHECKFLAG(cmd->words, GCODE_WORD_R))
            {
                reg = (uint32_t)words->r;
                tmc_write_register(&tmc0_driver, (uint8_t)words->xyzabc[0], reg);
            }
            reg = tmc_read_register(&tmc0_driver, (uint8_t)words->xyzabc[0]);
#else
            reg = 0xFFFFFFFFUL;
#endif
            serial_print_int(reg);
            serial_putc(']');
            serial_print_str(MSG_EOL);
        }

        if (CHECKFLAG(cmd->words, GCODE_WORD_Y))
        {
            serial_print_str(__romstr__("[TMCREG Y:"));
            reg = (uint32_t)words->xyzabc[1];
            serial_print_int(reg);
            serial_putc(',');
#ifdef STEPPER1_HAS_TMC
            tmc_driver_t tmc1_driver = {.type = STEPPER1_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc1_rw};
            reg = tmc_read_register(&tmc1_driver, (uint8_t)words->xyzabc[1]);
#else
            reg = 0xFFFFFFFFUL;
#endif
            serial_print_int(reg);
            serial_putc(']');
            serial_print_str(MSG_EOL);
        }

        if (CHECKFLAG(cmd->words, GCODE_WORD_Z))
        {
            serial_print_str(__romstr__("[TMCREG Z:"));
            reg = (uint32_t)words->xyzabc[2];
            serial_print_int(reg);
            serial_putc(',');
#ifdef STEPPER2_HAS_TMC
            tmc_driver_t tmc2_driver = {.type = STEPPER2_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc2_rw};
            reg = tmc_read_register(&tmc2_driver, (uint8_t)words->xyzabc[2]);
#else
            reg = 0xFFFFFFFFUL;
#endif
            serial_print_int(reg);
            serial_putc(']');
            serial_print_str(MSG_EOL);
        }

        if (CHECKFLAG(cmd->words, GCODE_WORD_A))
        {
            serial_print_str(__romstr__("[TMCREG A:"));
            reg = (uint32_t)words->xyzabc[3];
            serial_print_int(reg);
            serial_putc(',');
#ifdef STEPPER3_HAS_TMC
            tmc_driver_t tmc3_driver = {.type = STEPPER3_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc3_rw};
            reg = tmc_read_register(&tmc3_driver, (uint8_t)words->xyzabc[3]);
#else
            reg = 0xFFFFFFFFUL;
#endif
            serial_print_int(reg);
            serial_putc(']');
            serial_print_str(MSG_EOL);
        }

        if (CHECKFLAG(cmd->words, GCODE_WORD_B))
        {
            serial_print_str(__romstr__("[TMCREG B:"));
            reg = (uint32_t)words->xyzabc[4];
            serial_print_int(reg);
            serial_putc(',');
#ifdef STEPPER4_HAS_TMC
            tmc_driver_t tmc4_driver = {.type = STEPPER4_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc4_rw};
            reg = tmc_read_register(&tmc4_driver, (uint8_t)words->xyzabc[4]);
#else
            reg = 0xFFFFFFFFUL;
#endif
            serial_print_int(reg);
            serial_putc(']');
            serial_print_str(MSG_EOL);
        }

        if (CHECKFLAG(cmd->words, GCODE_WORD_C))
        {
            serial_print_str(__romstr__("[TMCREG C:"));
            reg = (uint32_t)words->xyzabc[5];
            serial_print_int(reg);
            serial_putc(',');
#ifdef STEPPER5_HAS_TMC
            tmc_driver_t tmc5_driver = {.type = STEPPER5_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc5_rw};
            reg = tmc_read_register(&tmc5_driver, (uint8_t)words->xyzabc[5]);
#else
            reg = 0xFFFFFFFFUL;
#endif
            serial_print_int(reg);
            serial_putc(']');
            serial_print_str(MSG_EOL);
        }

        if (CHECKFLAG(cmd->words, GCODE_WORD_I))
        {
            serial_print_str(__romstr__("[TMCREG I:"));
            reg = (uint32_t)words->ijk[0];
            serial_print_int(reg);
            serial_putc(',');
#ifdef STEPPER6_HAS_TMC
            tmc_driver_t tmc6_driver = {.type = STEPPER6_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc6_rw};
            reg = tmc_read_register(&tmc6_driver, (uint8_t)words->ijk[0]);
#else
            reg = 0xFFFFFFFFUL;
#endif
            serial_print_int(reg);
            serial_putc(']');
            serial_print_str(MSG_EOL);
        }

        if (CHECKFLAG(cmd->words, GCODE_WORD_J))
        {
            serial_print_str(__romstr__("[TMCREG J:"));
            reg = (uint32_t)words->ijk[1];
            serial_print_int(reg);
            serial_putc(',');
#ifdef STEPPER7_HAS_TMC
            tmc_driver_t tmc7_driver = {.type = STEPPER7_DRIVER_TYPE, .slave = 0, .init = NULL, .rw = &tmc7_rw};
            reg = tmc_read_register(&tmc7_driver, (uint8_t)words->ijk[1]);
#else
            reg = 0xFFFFFFFFUL;
#endif
            serial_print_int(reg);
            serial_putc(']');
            serial_print_str(MSG_EOL);
        }

        return STATUS_OK;
    }

    return STATUS_GOCDE_EXTENDED_UNSUPPORTED;
}

#endif
