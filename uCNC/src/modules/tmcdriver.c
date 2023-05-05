/*
	Name: tmcdriver.c
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
#include "tmc.h"
#include <stdint.h>
#include <float.h>

#ifdef ENABLE_TMC_DRIVERS

#define TMC_UARTBAUD 38400
#define TMC_UART_TIMEOUT 100

// driver communications declarations
// UART
#define TMC1_STEPPER_DECL(CHANNEL) SOFTUART(tmc##CHANNEL##_uart, TMC_UARTBAUD, STEPPER##CHANNEL##_UART_TX, STEPPER##CHANNEL##_UART_RX)
// SPI
#define TMC2_STEPPER_DECL(CHANNEL) SOFTSPI(tmc##CHANNEL_spi, 1000000UL, 0, STEPPER##CHANNEL##_SPI_DO, STEPPER##CHANNEL##_SPI_DI, STEPPER##CHANNEL##_SPI_CLK)
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
			data[i] = softuart_getc(&tmc##CHANNEL##_uart, TMC_UART_TIMEOUT); \
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
			data[i] = softspi_xmit(&tmc##CHANNEL##_spi, data[i]);            \
		}                                                                    \
		mcu_set_output(STEPPER##CHANNEL##_SPI_CS);                           \
	}
#define _TMC_STEPPER_RW(TYPE, CHANNEL) TMC##TYPE##_STEPPER_RW(CHANNEL)
#define TMC_STEPPER_RW(TYPE, CHANNEL) _TMC_STEPPER_RW(TYPE, CHANNEL)

typedef struct
{
	float rms_current;
	float rsense;
	float ihold_mul;
	uint8_t ihold_delay;
	int16_t mstep;
	bool step_interpolation;
	uint32_t stealthchop_threshold;
	int32_t stallguard_threshold;
} tmc_driver_setting_t;

#ifdef STEPPER0_HAS_TMC
TMC_STEPPER_DECL(STEPPER0_TMC_INTERFACE, 0);
TMC_STEPPER_RW(STEPPER0_TMC_INTERFACE, 0);
tmc_driver_t tmc0_driver;
tmc_driver_setting_t tmc0_settings;
#endif
#ifdef STEPPER1_HAS_TMC
TMC_STEPPER_DECL(STEPPER1_TMC_INTERFACE, 1);
TMC_STEPPER_RW(STEPPER1_TMC_INTERFACE, 1);
tmc_driver_t tmc1_driver;
tmc_driver_setting_t tmc1_settings;
#endif
#ifdef STEPPER2_HAS_TMC
TMC_STEPPER_DECL(STEPPER2_TMC_INTERFACE, 2);
TMC_STEPPER_RW(STEPPER2_TMC_INTERFACE, 2);
tmc_driver_t tmc2_driver;
tmc_driver_setting_t tmc2_settings;
#endif
#ifdef STEPPER3_HAS_TMC
TMC_STEPPER_DECL(STEPPER3_TMC_INTERFACE, 3);
TMC_STEPPER_RW(STEPPER3_TMC_INTERFACE, 3);
tmc_driver_t tmc3_driver;
tmc_driver_setting_t tmc3_settings;
#endif
#ifdef STEPPER4_HAS_TMC
TMC_STEPPER_DECL(STEPPER4_TMC_INTERFACE, 4);
TMC_STEPPER_RW(STEPPER4_TMC_INTERFACE, 4);
tmc_driver_t tmc4_driver;
tmc_driver_setting_t tmc4_settings;
#endif
#ifdef STEPPER5_HAS_TMC
TMC_STEPPER_DECL(STEPPER5_TMC_INTERFACE, 5);
TMC_STEPPER_RW(STEPPER5_TMC_INTERFACE, 5);
tmc_driver_t tmc5_driver;
tmc_driver_setting_t tmc5_settings;
#endif
#ifdef STEPPER6_HAS_TMC
TMC_STEPPER_DECL(STEPPER6_TMC_INTERFACE, 6);
TMC_STEPPER_RW(STEPPER6_TMC_INTERFACE, 6);
tmc_driver_t tmc6_driver;
tmc_driver_setting_t tmc6_settings;
#endif
#ifdef STEPPER7_HAS_TMC
TMC_STEPPER_DECL(STEPPER7_TMC_INTERFACE, 7);
TMC_STEPPER_RW(STEPPER7_TMC_INTERFACE, 7);
tmc_driver_t tmc7_driver;
tmc_driver_setting_t tmc7_settings;
#endif

#define TMC_STEPPER_PORT(PORT) &tmc##PORT

void tmcdriver_update(tmc_driver_t *driver, tmc_driver_setting_t *driver_settings)
{
	tmc_init(driver);
	tmc_set_current(driver, driver_settings->rms_current, driver_settings->rsense, driver_settings->ihold_mul, driver_settings->ihold_delay);
	tmc_set_microstep(driver, driver_settings->mstep);
	tmc_set_stealthchop(driver, driver_settings->stealthchop_threshold);
	tmc_set_stepinterpol(driver, driver_settings->step_interpolation);
	switch (driver->type)
	{
	case 2209:
	case 2130:
		tmc_set_stallguard(driver, driver_settings->stallguard_threshold);
		break;
	}
}

void tmcdriver_config(void)
{
#ifdef STEPPER0_HAS_TMC
	tmcdriver_update(&tmc0_driver, &tmc0_settings);
#endif
#ifdef STEPPER1_HAS_TMC
	tmcdriver_update(&tmc1_driver, &tmc1_settings);
#endif
#ifdef STEPPER2_HAS_TMC
	tmcdriver_update(&tmc2_driver, &tmc2_settings);
#endif
#ifdef STEPPER3_HAS_TMC
	tmcdriver_update(&tmc3_driver, &tmc3_settings);
#endif
#ifdef STEPPER4_HAS_TMC
	tmcdriver_update(&tmc4_driver, &tmc4_settings);
#endif
#ifdef STEPPER5_HAS_TMC
	tmcdriver_update(&tmc5_driver, &tmc5_settings);
#endif
#ifdef STEPPER6_HAS_TMC
	tmcdriver_update(&tmc6_driver, &tmc6_settings);
#endif
#ifdef STEPPER7_HAS_TMC
	tmcdriver_update(&tmc7_driver, &tmc7_settings);
#endif
}

bool tmcdriver_config_handler(void *args)
{
	tmcdriver_config();
	return EVENT_CONTINUE;
}

#ifdef ENABLE_MAIN_LOOP_MODULES
CREATE_EVENT_LISTENER(cnc_reset, tmcdriver_config_handler);
#endif

/*custom gcode commands*/
#if defined(ENABLE_PARSER_MODULES)
// this ID must be unique for each code
#define M350 EXTENDED_MCODE(350)
// this ID must be unique for each code
#define M906 EXTENDED_MCODE(906)
// this ID must be unique for each code
#define M913 EXTENDED_MCODE(913)
// this ID must be unique for each code
#define M914 EXTENDED_MCODE(914)
// this ID must be unique for each code
#define M920 EXTENDED_MCODE(920)

bool m350_parse(void *args);
bool m350_exec(void *args);
bool m906_parse(void *args);
bool m906_exec(void *args);
bool m913_parse(void *args);
bool m913_exec(void *args);
bool m914_parse(void *args);
bool m914_exec(void *args);
bool m920_parse(void *args);
bool m920_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, m350_parse);
CREATE_EVENT_LISTENER(gcode_exec, m350_exec);
CREATE_EVENT_LISTENER(gcode_parse, m906_parse);
CREATE_EVENT_LISTENER(gcode_exec, m906_exec);
CREATE_EVENT_LISTENER(gcode_parse, m913_parse);
CREATE_EVENT_LISTENER(gcode_exec, m913_exec);
CREATE_EVENT_LISTENER(gcode_parse, m914_parse);
CREATE_EVENT_LISTENER(gcode_exec, m914_exec);
CREATE_EVENT_LISTENER(gcode_parse, m920_parse);
CREATE_EVENT_LISTENER(gcode_exec, m920_exec);

// this just parses and acceps the code
bool m350_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M' && ptr->value == 350)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M350;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m350_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended == M350)
	{
		itp_sync();
		if (!ptr->cmd->words)
		{
			int32_t val = 0;
			// if no additional args then print the
			protocol_send_string(__romstr__("[MICROSTEPS:"));
			val = 0;
			serial_putc('X');
#ifdef STEPPER0_HAS_TMC
			val = tmc_get_microstep(&tmc0_driver);
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = 0;
			serial_putc('Y');
#ifdef STEPPER1_HAS_TMC
			val = tmc_get_microstep(&tmc1_driver);
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = 0;
			serial_putc('Z');
#ifdef STEPPER2_HAS_TMC
			val = tmc_get_microstep(&tmc2_driver);
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = 0;
			serial_putc('A');
#ifdef STEPPER3_HAS_TMC
			val = tmc_get_microstep(&tmc3_driver);
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = 0;
			serial_putc('B');
#ifdef STEPPER4_HAS_TMC
			val = tmc_get_microstep(&tmc4_driver);
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = 0;
			serial_putc('C');
#ifdef STEPPER5_HAS_TMC
			val = tmc_get_microstep(&tmc5_driver);
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = 0;
			serial_putc('I');
#ifdef STEPPER6_HAS_TMC
			val = tmc_get_microstep(&tmc6_driver);
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = 0;
			serial_putc('J');
#ifdef STEPPER7_HAS_TMC
			val = tmc_get_microstep(&tmc7_driver);
#endif
			serial_print_flt(val);
			serial_putc(']');
			protocol_send_string(MSG_EOL);
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_X))
		{
#ifdef STEPPER0_HAS_TMC
			tmc0_settings.mstep = (uint8_t)ptr->words->xyzabc[0];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Y))
		{
#ifdef STEPPER1_HAS_TMC
			tmc1_settings.mstep = (uint8_t)ptr->words->xyzabc[1];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Z))
		{
#ifdef STEPPER2_HAS_TMC
			tmc2_settings.mstep = (uint8_t)ptr->words->xyzabc[2];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A))
		{
#ifdef STEPPER3_HAS_TMC
			tmc3_settings.mstep = (uint8_t)ptr->words->xyzabc[3];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_B))
		{
#ifdef STEPPER4_HAS_TMC
			tmc4_settings.mstep = (uint8_t)ptr->words->xyzabc[4];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_C))
		{
#ifdef STEPPER5_HAS_TMC
			tmc5_settings.mstep = (uint8_t)ptr->words->xyzabc[5];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_I))
		{
#ifdef STEPPER6_HAS_TMC
			tmc6_settings.mstep = (uint8_t)ptr->words->ijk[0];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_J))
		{
#ifdef STEPPER7_HAS_TMC
			tmc7_settings.mstep = (uint8_t)ptr->words->ijk[1];
#endif
		}

		tmcdriver_config();

		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

// this just parses and acceps the code
bool m906_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M' && ptr->value == 906.0f)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M906;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m906_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended == M906)
	{
		itp_sync();
		if (!ptr->cmd->words)
		{
			float val;
			// if no additional args then print the
			protocol_send_string(__romstr__("[STEPPER CURRENT:"));
			val = -1;
			serial_putc('X');
#ifdef STEPPER0_HAS_TMC
			val = tmc_get_current(&tmc0_driver, STEPPER0_RSENSE);
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = -1;
			serial_putc('Y');
#ifdef STEPPER1_HAS_TMC
			val = tmc_get_current(&tmc1_driver, STEPPER1_RSENSE);
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = -1;
			serial_putc('Z');
#ifdef STEPPER2_HAS_TMC
			val = tmc_get_current(&tmc2_driver, STEPPER2_RSENSE);
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = -1;
			serial_putc('A');
#ifdef STEPPER3_HAS_TMC
			val = tmc_get_current(&tmc3_driver, STEPPER3_RSENSE);
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = -1;
			serial_putc('B');
#ifdef STEPPER4_HAS_TMC
			val = tmc_get_current(&tmc4_driver, STEPPER4_RSENSE);
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = -1;
			serial_putc('C');
#ifdef STEPPER5_HAS_TMC
			val = tmc_get_current(&tmc5_driver, STEPPER5_RSENSE);
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = -1;
			serial_putc('I');
#ifdef STEPPER6_HAS_TMC
			val = tmc_get_current(&tmc6_driver, STEPPER6_RSENSE);
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = -1;
			serial_putc('J');
#ifdef STEPPER7_HAS_TMC
			val = tmc_get_current(&tmc7_driver, STEPPER7_RSENSE);
#endif
			serial_print_flt(val);
			serial_putc(']');
			protocol_send_string(MSG_EOL);
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_X))
		{
#ifdef STEPPER0_HAS_TMC
			tmc0_settings.rms_current = ptr->words->xyzabc[0];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Y))
		{
#ifdef STEPPER1_HAS_TMC
			tmc1_settings.rms_current = ptr->words->xyzabc[1];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Z))
		{
#ifdef STEPPER2_HAS_TMC
			tmc2_settings.rms_current = ptr->words->xyzabc[2];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A))
		{
#ifdef STEPPER3_HAS_TMC
			tmc3_settings.rms_current = ptr->words->xyzabc[3];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_B))
		{
#ifdef STEPPER4_HAS_TMC
			tmc4_settings.rms_current = ptr->words->xyzabc[4];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_C))
		{
#ifdef STEPPER5_HAS_TMC
			tmc5_settings.rms_current = ptr->words->xyzabc[5];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_I))
		{
#ifdef STEPPER6_HAS_TMC
			tmc6_settings.rms_current = ptr->words->ijk[0];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_J))
		{
#ifdef STEPPER7_HAS_TMC
			tmc7_settings.rms_current = ptr->words->ijk[1];
#endif
		}

		tmcdriver_config();
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

// this just parses and acceps the code
bool m913_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M' && ptr->value == 913.0f)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M913;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m913_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended == M913)
	{
		itp_sync();
		if (!ptr->cmd->words)
		{
			int32_t val;
			// if no additional args then print the
			protocol_send_string(__romstr__("[STEPPER HYBRID THRESHOLD:"));
			val = -1;
			serial_putc('X');
#ifdef STEPPER0_HAS_TMC
			val = tmc_get_stealthchop(&tmc0_driver);
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -1;
			serial_putc('Y');
#ifdef STEPPER1_HAS_TMC
			val = tmc_get_stealthchop(&tmc1_driver);
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -1;
			serial_putc('Z');
#ifdef STEPPER2_HAS_TMC
			val = tmc_get_stealthchop(&tmc2_driver);
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -1;
			serial_putc('A');
#ifdef STEPPER3_HAS_TMC
			val = tmc_get_stealthchop(&tmc3_driver);
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -1;
			serial_putc('B');
#ifdef STEPPER4_HAS_TMC
			val = tmc_get_stealthchop(&tmc4_driver);
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -1;
			serial_putc('C');
#ifdef STEPPER5_HAS_TMC
			val = tmc_get_stealthchop(&tmc5_driver);
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -1;
			serial_putc('I');
#ifdef STEPPER6_HAS_TMC
			val = tmc_get_stealthchop(&tmc6_driver);
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -1;
			serial_putc('J');
#ifdef STEPPER7_HAS_TMC
			val = tmc_get_stealthchop(&tmc7_driver);
#endif
			serial_print_int(val);
			serial_putc(']');
			protocol_send_string(MSG_EOL);
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_X))
		{
#ifdef STEPPER0_HAS_TMC
			tmc0_settings.stealthchop_threshold = (uint32_t)ptr->words->xyzabc[0];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Y))
		{
#ifdef STEPPER1_HAS_TMC
			tmc1_settings.stealthchop_threshold = (uint32_t)ptr->words->xyzabc[1];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Z))
		{
#ifdef STEPPER2_HAS_TMC
			tmc2_settings.stealthchop_threshold = (uint32_t)ptr->words->xyzabc[2];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A))
		{
#ifdef STEPPER3_HAS_TMC
			tmc3_settings.stealthchop_threshold = (uint32_t)ptr->words->xyzabc[3];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_B))
		{
#ifdef STEPPER4_HAS_TMC
			tmc4_settings.stealthchop_threshold = (uint32_t)ptr->words->xyzabc[4];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_C))
		{
#ifdef STEPPER5_HAS_TMC
			tmc5_settings.stealthchop_threshold = (uint32_t)ptr->words->xyzabc[5];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_I))
		{
#ifdef STEPPER6_HAS_TMC
			tmc6_settings.stealthchop_threshold = (uint32_t)ptr->words->ijk[0];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_J))
		{
#ifdef STEPPER7_HAS_TMC
			tmc7_settings.stealthchop_threshold = (uint32_t)ptr->words->ijk[1];
#endif
		}

		tmcdriver_config();
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

// this just parses and acceps the code
bool m914_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M' && ptr->value == 914.0f)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M914;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m914_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended == M914)
	{
		itp_sync();
		if (!ptr->cmd->words)
		{
			int32_t val;
			// if no additional args then print the
			protocol_send_string(__romstr__("[STEPPER STALL SENSITIVITY:"));
			val = -255;
			serial_putc('X');
#ifdef STEPPER0_HAS_TMC
			val = tmc_get_stallguard(&tmc0_driver);
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -255;
			serial_putc('Y');
#ifdef STEPPER1_HAS_TMC
			val = tmc_get_stallguard(&tmc1_driver);
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -255;
			serial_putc('Z');
#ifdef STEPPER2_HAS_TMC
			val = tmc_get_stallguard(&tmc2_driver);
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -255;
			serial_putc('A');
#ifdef STEPPER3_HAS_TMC
			val = tmc_get_stallguard(&tmc3_driver);
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -255;
			serial_putc('B');
#ifdef STEPPER4_HAS_TMC
			val = tmc_get_stallguard(&tmc4_driver);
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -255;
			serial_putc('C');
#ifdef STEPPER5_HAS_TMC
			val = tmc_get_stallguard(&tmc5_driver);
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -255;
			serial_putc('I');
#ifdef STEPPER6_HAS_TMC
			val = tmc_get_stallguard(&tmc6_driver);
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -255;
			serial_putc('J');
#ifdef STEPPER7_HAS_TMC
			val = tmc_get_stallguard(&tmc7_driver);
#endif
			serial_print_int(val);
			serial_putc(']');
			protocol_send_string(MSG_EOL);
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_X))
		{
#ifdef STEPPER0_HAS_TMC
			tmc0_settings.stallguard_threshold = (int32_t)ptr->words->xyzabc[0];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Y))
		{
#ifdef STEPPER1_HAS_TMC
			tmc1_settings.stallguard_threshold = (int16_t)ptr->words->xyzabc[1];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Z))
		{
#ifdef STEPPER2_HAS_TMC
			tmc2_settings.stallguard_threshold = (int16_t)ptr->words->xyzabc[2];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A))
		{
#ifdef STEPPER3_HAS_TMC
			tmc3_settings.stallguard_threshold = (int16_t)ptr->words->xyzabc[3];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_B))
		{
#ifdef STEPPER4_HAS_TMC
			tmc4_settings.stallguard_threshold = (int16_t)ptr->words->xyzabc[4];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_C))
		{
#ifdef STEPPER5_HAS_TMC
			tmc5_settings.stallguard_threshold = (int16_t)ptr->words->xyzabc[5];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_I))
		{
#ifdef STEPPER6_HAS_TMC
			tmc6_settings.stallguard_threshold = (int16_t)ptr->words->ijk[0];
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_J))
		{
#ifdef STEPPER7_HAS_TMC
			tmc7_settings.stallguard_threshold = (int16_t)ptr->words->ijk[1];
#endif
		}

		tmcdriver_config();
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

// this just parses and acceps the code
bool m920_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M' && ptr->value == 920.0f)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M920;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m920_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended == M920)
	{
		if (!CHECKFLAG(ptr->cmd->words, GCODE_ALL_AXIS | GCODE_IJK_AXIS))
		{
			*(ptr->error) = STATUS_TMC_CMD_MISSING_ARGS;
			return EVENT_HANDLED;
		}

		int8_t wordreg = -1;
		uint16_t wordval = 0;
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_L))
		{
			wordreg = (int8_t)ptr->words->l;
			if (wordreg > 1 || wordreg < 0)
			{
				*(ptr->error) = STATUS_INVALID_STATEMENT;
				return EVENT_HANDLED;
			}
			wordval = ptr->words->s;
		}

		uint32_t reg;

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_X))
		{
			protocol_send_string(__romstr__("[TMCREG X:"));
			reg = (uint32_t)ptr->words->xyzabc[0];
			serial_print_int(reg);
			serial_putc(',');
#ifdef STEPPER0_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc0_driver, (uint8_t)ptr->words->xyzabc[0]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc0_driver, (uint8_t)ptr->words->xyzabc[0], reg);
			}
			reg = tmc_read_register(&tmc0_driver, (uint8_t)ptr->words->xyzabc[0]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			serial_print_int(reg);
			serial_putc(']');
			protocol_send_string(MSG_EOL);
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Y))
		{
			protocol_send_string(__romstr__("[TMCREG Y:"));
			reg = (uint32_t)ptr->words->xyzabc[1];
			serial_print_int(reg);
			serial_putc(',');
#ifdef STEPPER1_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc1_driver, (uint8_t)ptr->words->xyzabc[1]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc1_driver, (uint8_t)ptr->words->xyzabc[1], reg);
			}
			reg = tmc_read_register(&tmc1_driver, (uint8_t)ptr->words->xyzabc[1]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			serial_print_int(reg);
			serial_putc(']');
			protocol_send_string(MSG_EOL);
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Z))
		{
			protocol_send_string(__romstr__("[TMCREG Z:"));
			reg = (uint32_t)ptr->words->xyzabc[2];
			serial_print_int(reg);
			serial_putc(',');
#ifdef STEPPER2_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc2_driver, (uint8_t)ptr->words->xyzabc[2]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc2_driver, (uint8_t)ptr->words->xyzabc[2], reg);
			}
			reg = tmc_read_register(&tmc2_driver, (uint8_t)ptr->words->xyzabc[2]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			serial_print_int(reg);
			serial_putc(']');
			protocol_send_string(MSG_EOL);
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A))
		{
			protocol_send_string(__romstr__("[TMCREG A:"));
			reg = (uint32_t)ptr->words->xyzabc[3];
			serial_print_int(reg);
			serial_putc(',');
#ifdef STEPPER3_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc3_driver, (uint8_t)ptr->words->xyzabc[3]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc3_driver, (uint8_t)ptr->words->xyzabc[3], reg);
			}
			reg = tmc_read_register(&tmc3_driver, (uint8_t)ptr->words->xyzabc[3]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			serial_print_int(reg);
			serial_putc(']');
			protocol_send_string(MSG_EOL);
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_B))
		{
			protocol_send_string(__romstr__("[TMCREG B:"));
			reg = (uint32_t)ptr->words->xyzabc[4];
			serial_print_int(reg);
			serial_putc(',');
#ifdef STEPPER4_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc4_driver, (uint8_t)ptr->words->xyzabc[4]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc4_driver, (uint8_t)ptr->words->xyzabc[4], reg);
			}
			reg = tmc_read_register(&tmc4_driver, (uint8_t)ptr->words->xyzabc[4]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			serial_print_int(reg);
			serial_putc(']');
			protocol_send_string(MSG_EOL);
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_C))
		{
			protocol_send_string(__romstr__("[TMCREG C:"));
			reg = (uint32_t)ptr->words->xyzabc[5];
			serial_print_int(reg);
			serial_putc(',');
#ifdef STEPPER5_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc5_driver, (uint8_t)ptr->words->xyzabc[5]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc5_driver, (uint8_t)ptr->words->xyzabc[5], reg);
			}
			reg = tmc_read_register(&tmc5_driver, (uint8_t)ptr->words->xyzabc[5]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			serial_print_int(reg);
			serial_putc(']');
			protocol_send_string(MSG_EOL);
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_I))
		{
			protocol_send_string(__romstr__("[TMCREG I:"));
			reg = (uint32_t)ptr->words->ijk[0];
			serial_print_int(reg);
			serial_putc(',');
#ifdef STEPPER6_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc6_driver, (uint8_t)ptr->words->ijk[0]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc6_driver, (uint8_t)ptr->words->ijk[0], reg);
			}
			reg = tmc_read_register(&tmc6_driver, (uint8_t)ptr->words->ijk[0]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			serial_print_int(reg);
			serial_putc(']');
			protocol_send_string(MSG_EOL);
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_J))
		{
			protocol_send_string(__romstr__("[TMCREG J:"));
			reg = (uint32_t)ptr->words->ijk[1];
			serial_print_int(reg);
			serial_putc(',');
#ifdef STEPPER7_HAS_TMC
			if (wordreg >= 0)
			{
				reg = tmc_read_register(&tmc7_driver, (uint8_t)ptr->words->ijk[1]);
				switch (wordreg)
				{
				case 0:
					reg &= 0xFFFF0000;
					reg |= (((uint32_t)wordval));
					break;
				case 1:
					reg &= 0x0000FFFF;
					reg |= (((uint32_t)wordval) << 16);
					break;
				}
				tmc_write_register(&tmc7_driver, (uint8_t)ptr->words->ijk[1], reg);
			}
			reg = tmc_read_register(&tmc7_driver, (uint8_t)ptr->words->ijk[1]);
#else
			reg = 0xFFFFFFFFUL;
#endif
			serial_print_int(reg);
			serial_putc(']');
			protocol_send_string(MSG_EOL);
		}

		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

DECL_MODULE(tmcdriver)
{
#if ASSERT_PIN(STEPPER0_SPI_CS)
	mcu_set_output(STEPPER0_SPI_CS);
#endif
#if ASSERT_PIN(STEPPER1_SPI_CS)
	mcu_set_output(STEPPER1_SPI_CS);
#endif
#if ASSERT_PIN(STEPPER2_SPI_CS)
	mcu_set_output(STEPPER2_SPI_CS);
#endif
#if ASSERT_PIN(STEPPER3_SPI_CS)
	mcu_set_output(STEPPER3_SPI_CS);
#endif
#if ASSERT_PIN(STEPPER4_SPI_CS)
	mcu_set_output(STEPPER4_SPI_CS);
#endif
#if ASSERT_PIN(STEPPER5_SPI_CS)
	mcu_set_output(STEPPER5_SPI_CS);
#endif
#if ASSERT_PIN(STEPPER6_SPI_CS)
	mcu_set_output(STEPPER6_SPI_CS);
#endif
#if ASSERT_PIN(STEPPER7_SPI_CS)
	mcu_set_output(STEPPER7_SPI_CS);
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_reset, tmcdriver_config_handler);
#else
#error "Main loop extensions are not enabled. TMC configurations will not work."
#endif
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, m350_parse);
	ADD_EVENT_LISTENER(gcode_exec, m350_exec);
	ADD_EVENT_LISTENER(gcode_parse, m906_parse);
	ADD_EVENT_LISTENER(gcode_exec, m906_exec);
	ADD_EVENT_LISTENER(gcode_parse, m913_parse);
	ADD_EVENT_LISTENER(gcode_exec, m913_exec);
	ADD_EVENT_LISTENER(gcode_parse, m914_parse);
	ADD_EVENT_LISTENER(gcode_exec, m914_exec);
	ADD_EVENT_LISTENER(gcode_parse, m920_parse);
	ADD_EVENT_LISTENER(gcode_exec, m920_exec);
#else
#warning "Parser extensions are not enabled. M350, M906, M913, M914 and M920 code extensions will not work."
#endif

#ifdef STEPPER0_HAS_TMC
	tmc0_driver.type = STEPPER0_DRIVER_TYPE;
	tmc0_driver.slave = 0;
	tmc0_driver.init = NULL;
	tmc0_driver.rw = &tmc0_rw;
	tmc0_settings.rms_current = STEPPER0_CURRENT_MA;
	tmc0_settings.rsense = STEPPER0_RSENSE;
	tmc0_settings.ihold_mul = STEPPER0_HOLD_MULT;
	tmc0_settings.ihold_delay = 5;
	tmc0_settings.mstep = STEPPER0_MICROSTEP;
	tmc0_settings.stealthchop_threshold = STEPPER0_STEALTHCHOP_THERSHOLD;
	tmc0_settings.step_interpolation = STEPPER0_ENABLE_INTERPLATION;
	switch (STEPPER0_DRIVER_TYPE)
	{
	case 2209:
		tmc0_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc0_settings.stallguard_threshold = 63;
		break;
	}
#endif
#ifdef STEPPER1_HAS_TMC
	tmc1_driver.type = STEPPER1_DRIVER_TYPE;
	tmc1_driver.slave = 0;
	tmc1_driver.init = NULL;
	tmc1_driver.rw = &tmc1_rw;
	tmc1_settings.rms_current = STEPPER1_CURRENT_MA;
	tmc1_settings.rsense = STEPPER1_RSENSE;
	tmc1_settings.ihold_mul = STEPPER1_HOLD_MULT;
	tmc1_settings.ihold_delay = 5;
	tmc1_settings.mstep = STEPPER1_MICROSTEP;
	tmc1_settings.stealthchop_threshold = STEPPER1_STEALTHCHOP_THERSHOLD;
	tmc1_settings.step_interpolation = STEPPER1_ENABLE_INTERPLATION;
	switch (STEPPER1_DRIVER_TYPE)
	{
	case 2209:
		tmc1_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc1_settings.stallguard_threshold = 63;
		break;
	}
#endif
#ifdef STEPPER2_HAS_TMC
	tmc2_driver.type = STEPPER2_DRIVER_TYPE;
	tmc2_driver.slave = 0;
	tmc2_driver.init = NULL;
	tmc2_driver.rw = &tmc2_rw;
	tmc2_settings.rms_current = STEPPER2_CURRENT_MA;
	tmc2_settings.rsense = STEPPER2_RSENSE;
	tmc2_settings.ihold_mul = STEPPER2_HOLD_MULT;
	tmc2_settings.ihold_delay = 5;
	tmc2_settings.mstep = STEPPER2_MICROSTEP;
	tmc2_settings.stealthchop_threshold = STEPPER2_STEALTHCHOP_THERSHOLD;
	tmc2_settings.step_interpolation = STEPPER2_ENABLE_INTERPLATION;
	switch (STEPPER2_DRIVER_TYPE)
	{
	case 2209:
		tmc2_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc2_settings.stallguard_threshold = 63;
		break;
	}
#endif
#ifdef STEPPER3_HAS_TMC
	tmc3_driver.type = STEPPER3_DRIVER_TYPE;
	tmc3_driver.slave = 0;
	tmc3_driver.init = NULL;
	tmc3_driver.rw = &tmc3_rw;
	tmc3_settings.rms_current = STEPPER3_CURRENT_MA;
	tmc3_settings.rsense = STEPPER3_RSENSE;
	tmc3_settings.ihold_mul = STEPPER3_HOLD_MULT;
	tmc3_settings.ihold_delay = 5;
	tmc3_settings.mstep = STEPPER3_MICROSTEP;
	tmc3_settings.stealthchop_threshold = STEPPER3_STEALTHCHOP_THERSHOLD;
	tmc3_settings.step_interpolation = STEPPER3_ENABLE_INTERPLATION;
	switch (STEPPER3_DRIVER_TYPE)
	{
	case 2209:
		tmc3_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc3_settings.stallguard_threshold = 63;
		break;
	}
#endif
#ifdef STEPPER4_HAS_TMC
	tmc4_driver.type = STEPPER4_DRIVER_TYPE;
	tmc4_driver.slave = 0;
	tmc4_driver.init = NULL;
	tmc4_driver.rw = &tmc4_rw;
	tmc4_settings.rms_current = STEPPER4_CURRENT_MA;
	tmc4_settings.rsense = STEPPER4_RSENSE;
	tmc4_settings.ihold_mul = STEPPER4_HOLD_MULT;
	tmc4_settings.ihold_delay = 5;
	tmc4_settings.mstep = STEPPER4_MICROSTEP;
	tmc4_settings.stealthchop_threshold = STEPPER4_STEALTHCHOP_THERSHOLD;
	tmc4_settings.step_interpolation = STEPPER4_ENABLE_INTERPLATION;
	switch (STEPPER4_DRIVER_TYPE)
	{
	case 2209:
		tmc4_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc4_settings.stallguard_threshold = 63;
		break;
	}
#endif
#ifdef STEPPER5_HAS_TMC
	tmc5_driver.type = STEPPER5_DRIVER_TYPE;
	tmc5_driver.slave = 0;
	tmc5_driver.init = NULL;
	tmc5_driver.rw = &tmc5_rw;
	tmc5_settings.rms_current = STEPPER5_CURRENT_MA;
	tmc5_settings.rsense = STEPPER5_RSENSE;
	tmc5_settings.ihold_mul = STEPPER5_HOLD_MULT;
	tmc5_settings.ihold_delay = 5;
	tmc5_settings.mstep = STEPPER5_MICROSTEP;
	tmc5_settings.stealthchop_threshold = STEPPER5_STEALTHCHOP_THERSHOLD;
	tmc5_settings.step_interpolation = STEPPER5_ENABLE_INTERPLATION;
	switch (STEPPER5_DRIVER_TYPE)
	{
	case 2209:
		tmc5_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc5_settings.stallguard_threshold = 63;
		break;
	}
#endif
#ifdef STEPPER6_HAS_TMC
	tmc6_driver.type = STEPPER6_DRIVER_TYPE;
	tmc6_driver.slave = 0;
	tmc6_driver.init = NULL;
	tmc6_driver.rw = &tmc6_rw;
	tmc6_settings.rms_current = STEPPER6_CURRENT_MA;
	tmc6_settings.rsense = STEPPER6_RSENSE;
	tmc6_settings.ihold_mul = STEPPER6_HOLD_MULT;
	tmc6_settings.ihold_delay = 6;
	tmc6_settings.mstep = STEPPER6_MICROSTEP;
	tmc6_settings.stealthchop_threshold = STEPPER6_STEALTHCHOP_THERSHOLD;
	tmc6_settings.step_interpolation = STEPPER6_ENABLE_INTERPLATION;
	switch (STEPPER6_DRIVER_TYPE)
	{
	case 2209:
		tmc6_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc6_settings.stallguard_threshold = 63;
		break;
	}
#endif
#ifdef STEPPER7_HAS_TMC
	tmc7_driver.type = STEPPER7_DRIVER_TYPE;
	tmc7_driver.slave = 0;
	tmc7_driver.init = NULL;
	tmc7_driver.rw = &tmc7_rw;
	tmc7_settings.rms_current = STEPPER7_CURRENT_MA;
	tmc7_settings.rsense = STEPPER7_RSENSE;
	tmc7_settings.ihold_mul = STEPPER7_HOLD_MULT;
	tmc7_settings.ihold_delay = 7;
	tmc7_settings.mstep = STEPPER7_MICROSTEP;
	tmc7_settings.stealthchop_threshold = STEPPER7_STEALTHCHOP_THERSHOLD;
	tmc7_settings.step_interpolation = STEPPER7_ENABLE_INTERPLATION;
	switch (STEPPER7_DRIVER_TYPE)
	{
	case 2209:
		tmc7_settings.stallguard_threshold = 0;
		break;
	case 2130:
		tmc7_settings.stallguard_threshold = 63;
		break;
	}
#endif
}

#endif
