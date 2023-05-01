/*
	Name: graphic_lcd.c
	Description: Graphic LCD module for µCNC using u8g2 lib.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 08-09-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include <clib/u8g2.h>
#include <clib/u8x8.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "../system_menu.h"

#ifndef UCNC_MODULE_VERSION_1_5_0_PLUS
#error "This module is not compatible with the current version of µCNC"
#endif

// used with graphic_lcd module

#ifndef U8X8_MSG_GPIO_SPI_CLOCK_PIN
// #define U8X8_MSG_GPIO_SPI_CLOCK_PIN DOUT8
#endif
#ifndef U8X8_MSG_GPIO_SPI_DATA_PIN
// #define U8X8_MSG_GPIO_SPI_DATA_PIN DOUT9
#endif
#ifndef U8X8_MSG_GPIO_CS_PIN
// #define U8X8_MSG_GPIO_CS_PIN DOUT10
#endif

// #define U8X8_MSG_GPIO_MENU_SELECT_PIN DIN21
// #define U8X8_MSG_GPIO_MENU_NEXT_PIN DIN22
// #define U8X8_MSG_GPIO_MENU_PREV_PIN DIN23

#ifndef GRAPHIC_LCD_MAX_LINES
#define GRAPHIC_LCD_MAX_LINES 5
#endif

#ifndef GRAPHIC_LCD_REFRESH
#define GRAPHIC_LCD_REFRESH 1000
#endif

#define GRAPHIC_LCD_SELECT 1
#define GRAPHIC_LCD_NEXT 2
#define GRAPHIC_LCD_PREV 4
#define GRAPHIC_LCD_UP 8
#define GRAPHIC_LCD_DOWN 16
#define GRAPHIC_LCD_HOME 32

static u8g2_t graphiclcd_u8g2;
#define U8G2 ((u8g2_t *)&graphiclcd_u8g2)

#define LCDWIDTH u8g2_GetDisplayWidth(U8G2)
#define LCDHEIGHT u8g2_GetDisplayHeight(U8G2)
#define FONTHEIGHT (u8g2_GetAscent(U8G2) - u8g2_GetDescent(U8G2))
#define ALIGN_CENTER(t) ((LCDWIDTH - u8g2_GetUTF8Width(U8G2, t)) / 2)
#define ALIGN_RIGHT(t) (LCDWIDTH - u8g2_GetUTF8Width(U8G2, t))
#define ALIGN_LEFT 0
#define JUSTIFY_CENTER ((LCDHEIGHT + FONTHEIGHT) / 2)
#define JUSTIFY_BOTTOM (LCDHEIGHT + u8g2_GetDescent(U8G2))
#define JUSTIFY_TOP u8g2_GetAscent(U8G2)
#define TEXT_WIDTH(t) u8g2_GetUTF8Width(U8G2, t)

/**
 *
 * can also be done via hardware SPI and I2C libraries of µCNC
 * but is not needed
 *
 * */

// #ifdef MCU_HAS_SPI
// #include "../softspi.h"
// uint8_t u8x8_byte_ucnc_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
// {
// 	uint8_t *data;
// 	switch (msg)
// 	{
// 	case U8X8_MSG_BYTE_SEND:
// 		data = (uint8_t *)arg_ptr;
// 		while (arg_int > 0)
// 		{
// 			softspi_xmit(NULL, (uint8_t)*data);
// 			data++;
// 			arg_int--;
// 		}
// 		break;
// 	case U8X8_MSG_BYTE_INIT:
// 		mcu_set_output(U8X8_MSG_GPIO_CS_PIN);
// 		break;
// 	case U8X8_MSG_BYTE_SET_DC:
// 		u8x8_gpio_SetDC(u8x8, arg_int);
// 		break;
// 	case U8X8_MSG_BYTE_START_TRANSFER:
//      softspi_config(u8x8->display_info->spi_mode, u8x8->display_info->sck_clock_hz);
// 		/* SPI mode has to be mapped to the mode of the current controller, at least Uno, Due, 101 have different SPI_MODEx values */
// 		u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);
// 		u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->post_chip_enable_wait_ns, NULL);
// 		break;
// 	case U8X8_MSG_BYTE_END_TRANSFER:
// 		u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->pre_chip_disable_wait_ns, NULL);
// 		u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
// 		break;
// 	default:
// 		return 0;
// 	}
// 	return 1;
// }
// #endif

#include "../softi2c.h"
#ifndef MCU_HAS_I2C
#ifndef U8X8_MSG_GPIO_I2C_CLOCK_PIN
#define U8X8_MSG_GPIO_I2C_CLOCK_PIN DIN30
#endif
#ifndef U8X8_MSG_GPIO_I2C_DATA_PIN
#define U8X8_MSG_GPIO_I2C_DATA_PIN DIN31
#endif
SOFTI2C(graphic_i2c, 100000UL, U8X8_MSG_GPIO_I2C_CLOCK_PIN, U8X8_MSG_GPIO_I2C_DATA_PIN)
#endif

uint8_t u8x8_byte_ucnc_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
#ifdef MCU_HAS_I2C
	softi2c_port_t *i2c_port = NULL;
#else
	softi2c_port_t *i2c_port = &graphic_i2c;
#endif
	static uint8_t i2c_buffer[32]; /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
	static uint8_t i2c_buffer_offset = 0;

	switch (msg)
	{
	case U8X8_MSG_BYTE_SEND:
		memcpy(&i2c_buffer[i2c_buffer_offset], arg_ptr, arg_int);
		i2c_buffer_offset += arg_int;
		break;
	case U8X8_MSG_BYTE_INIT:
		/* add your custom code to init i2c subsystem */
		break;
	case U8X8_MSG_BYTE_SET_DC:
		/* ignored for i2c */
		break;
	case U8X8_MSG_BYTE_START_TRANSFER:
		i2c_buffer_offset = 0;
		break;
	case U8X8_MSG_BYTE_END_TRANSFER:
		softi2c_send(i2c_port, u8x8_GetI2CAddress(u8x8) >> 1, i2c_buffer, i2c_buffer_offset);
		i2c_buffer_offset = 0;
		break;
	default:
		return 0;
	}

	return 1;
}

uint8_t u8x8_gpio_and_delay_ucnc(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	switch (msg)
	{
	case U8X8_MSG_GPIO_AND_DELAY_INIT: // called once during init phase of u8g2/u8x8
		break;						   // can be used to setup pins
	case U8X8_MSG_DELAY_NANO:		   // delay arg_int * 1 nano second
		while (arg_int--)
			;
		break;
	case U8X8_MSG_DELAY_100NANO: // delay arg_int * 100 nano seconds
		while (arg_int--)
			mcu_delay_100ns();
		break;
	case U8X8_MSG_DELAY_10MICRO: // delay arg_int * 10 micro seconds
		while (arg_int--)
			mcu_delay_us(10);
		break;
	case U8X8_MSG_DELAY_MILLI: // delay arg_int * 1 milli second
		cnc_delay_ms(arg_int);
		break;
	case U8X8_MSG_DELAY_I2C: // arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
		if (arg_int == 1)
		{
			mcu_delay_us(5);
		}
		else
		{
			mcu_delay_us(1);
		}
		break; // arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
#if U8X8_MSG_GPIO_SPI_CLOCK_PIN == UNDEF_PIN
	case U8X8_MSG_GPIO_D0: // D0 or SPI clock pin: Output level in arg_int
#else
	case U8X8_MSG_GPIO_SPI_CLOCK:
#endif
#if U8X8_MSG_GPIO_SPI_CLOCK_PIN != UNDEF_PIN
		if (arg_int)
		{
			mcu_set_output(U8X8_MSG_GPIO_SPI_CLOCK_PIN);
		}
		else
		{
			mcu_clear_output(U8X8_MSG_GPIO_SPI_CLOCK_PIN);
		}
#endif
		break;
#ifndef U8X8_MSG_GPIO_SPI_DATA_PIN
	case U8X8_MSG_GPIO_D1: // D1 or SPI data pin: Output level in arg_int
#else
	case U8X8_MSG_GPIO_SPI_DATA:
#endif
#if U8X8_MSG_GPIO_SPI_DATA_PIN != UNDEF_PIN
		if (arg_int)
		{
			mcu_set_output(U8X8_MSG_GPIO_SPI_DATA_PIN);
		}
		else
		{
			mcu_clear_output(U8X8_MSG_GPIO_SPI_DATA_PIN);
		}
#endif
		break;
	case U8X8_MSG_GPIO_D2: // D2 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D2_PIN
		io_set_output(U8X8_MSG_GPIO_D2_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D3: // D3 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D3_PIN
		io_set_output(U8X8_MSG_GPIO_D3_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D4: // D4 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D4_PIN
		io_set_output(U8X8_MSG_GPIO_D4_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D5: // D5 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D5_PIN
		io_set_output(U8X8_MSG_GPIO_D5_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D6: // D6 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D6_PIN
		io_set_output(U8X8_MSG_GPIO_D6_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D7: // D7 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D7_PIN
		io_set_output(U8X8_MSG_GPIO_D7_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_E: // E/WR pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_E_PIN
		io_set_output(U8X8_MSG_GPIO_E_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_CS: // CS (chip select) pin: Output level in arg_int
#if U8X8_MSG_GPIO_CS_PIN != UNDEF_PIN
		if (arg_int)
		{
			mcu_set_output(U8X8_MSG_GPIO_CS_PIN);
		}
		else
		{
			mcu_clear_output(U8X8_MSG_GPIO_CS_PIN);
		}
#endif
		break;
	case U8X8_MSG_GPIO_DC: // DC (data/cmd, A0, register select) pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_DC_PIN
		io_set_output(U8X8_MSG_GPIO_DC_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_RESET: // Reset pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_RESET_PIN
		io_set_output(U8X8_MSG_GPIO_RESET_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_CS1: // CS1 (chip select) pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_CS1_PIN
		io_set_output(U8X8_MSG_GPIO_CS1_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_CS2: // CS2 (chip select) pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_CS2_PIN
		io_set_output(U8X8_MSG_GPIO_CS2_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_I2C_CLOCK: // arg_int=0: Output low at I2C clock pin
#if U8X8_MSG_GPIO_I2C_CLOCK_PIN != UNDEF_PIN
		if (arg_int)
		{
			mcu_config_input(U8X8_MSG_GPIO_I2C_CLOCK_PIN);
			mcu_config_pullup(U8X8_MSG_GPIO_I2C_CLOCK_PIN);
			u8x8_SetGPIOResult(u8x8, mcu_get_input(U8X8_MSG_GPIO_I2C_CLOCK_PIN));
		}
		else
		{
			mcu_config_output(U8X8_MSG_GPIO_I2C_CLOCK_PIN);
			mcu_clear_output(U8X8_MSG_GPIO_I2C_CLOCK_PIN);
			u8x8_SetGPIOResult(u8x8, 0);
		}
#endif
		break;					 // arg_int=1: Input dir with pullup high for I2C clock pin
	case U8X8_MSG_GPIO_I2C_DATA: // arg_int=0: Output low at I2C data pin
#if U8X8_MSG_GPIO_I2C_DATA_PIN != UNDEF_PIN
		if (arg_int)
		{
			mcu_config_input(U8X8_MSG_GPIO_I2C_DATA_PIN);
			mcu_config_pullup(U8X8_MSG_GPIO_I2C_DATA_PIN);
			u8x8_SetGPIOResult(u8x8, mcu_get_input(U8X8_MSG_GPIO_I2C_DATA_PIN));
		}
		else
		{
			mcu_config_output(U8X8_MSG_GPIO_I2C_DATA_PIN);
			mcu_clear_output(U8X8_MSG_GPIO_I2C_DATA_PIN);
			u8x8_SetGPIOResult(u8x8, 0);
		}
#endif
		break; // arg_int=1: Input dir with pullup high for I2C data pin
	case U8X8_MSG_GPIO_MENU_SELECT:
#ifdef U8X8_MSG_GPIO_MENU_SELECT_PIN
		u8x8_SetGPIOResult(u8x8, io_get_pinvalue(U8X8_MSG_GPIO_MENU_SELECT_PIN));
#else
		u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
#endif
		break;
	case U8X8_MSG_GPIO_MENU_NEXT:
#ifdef U8X8_MSG_GPIO_MENU_NEXT_PIN
		u8x8_SetGPIOResult(u8x8, io_get_pinvalue(U8X8_MSG_GPIO_MENU_NEXT_PIN));
#else
		u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
#endif
		break;
	case U8X8_MSG_GPIO_MENU_PREV:
#ifdef U8X8_MSG_GPIO_MENU_PREV_PIN
		u8x8_SetGPIOResult(u8x8, io_get_pinvalue(U8X8_MSG_GPIO_MENU_PREV_PIN));
#else
		u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
#endif
		break;
	case U8X8_MSG_GPIO_MENU_HOME:
#ifdef U8X8_MSG_GPIO_MENU_HOME_PIN
		u8x8_SetGPIOResult(u8x8, io_get_pinvalue(U8X8_MSG_GPIO_MENU_HOME_PIN));
#else
		u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
#endif
		break;
	default:
		u8x8_SetGPIOResult(u8x8, 1); // default return value
		break;
	}
	return 1;
}

#ifndef GRAPHIC_LCD_ENCODER_BTN
#define GRAPHIC_LCD_ENCODER_BTN DIN21
#endif
#ifndef GRAPHIC_LCD_ENCODER_ENC1
#define GRAPHIC_LCD_ENCODER_ENC1 DIN22
#endif
#ifndef GRAPHIC_LCD_ENCODER_ENC2
#define GRAPHIC_LCD_ENCODER_ENC2 DIN23
#endif
// reads inputs and returns a mask with a pin state transition (only rising or only falling)
uint8_t graphic_lcd_rotary_encoder_control(void)
{
	static uint8_t last_pin_state = 0;
	static uint8_t last_rot_transition = 0;
	uint8_t pin_state = 0;

// rotation encoder
#ifndef GRAPHIC_LCD_INVERT_ENCODER_DIR
	pin_state |= io_get_pinvalue(GRAPHIC_LCD_ENCODER_ENC1) ? 2 : 0;
	pin_state |= io_get_pinvalue(GRAPHIC_LCD_ENCODER_ENC2) ? 4 : 0;
#else
	pin_state |= io_get_pinvalue(GRAPHIC_LCD_ENCODER_ENC1) ? 4 : 0;
	pin_state |= io_get_pinvalue(GRAPHIC_LCD_ENCODER_ENC2) ? 2 : 0;
#endif

	switch (pin_state)
	{
	case 6:
		last_rot_transition = 0;
		pin_state = 0;
		break;
	case 4:
		pin_state = (last_rot_transition == 0) ? 2 : 0;
		last_rot_transition = 1;
		break;
	case 2:
		pin_state = (last_rot_transition == 0) ? 4 : 0;
		last_rot_transition = 2;
		break;
	default:
		last_rot_transition = 3;
		pin_state = 0;
		break;
	}

	pin_state |= io_get_pinvalue(GRAPHIC_LCD_ENCODER_BTN) ? 1 : 0;

	pin_state = ~pin_state;

	uint8_t pin_diff = last_pin_state ^ pin_state;
	if (pin_diff)
	{
		last_pin_state = pin_state;
		return (pin_diff & pin_state);
	}

	return 0;
}

// static bool graphic_lcd_current_menu_active;
#ifdef ENABLE_MAIN_LOOP_MODULES
uint8_t graphic_lcd_update(void *args, bool *handled)
{
	switch (graphic_lcd_rotary_encoder_control())
	{
	case 0:
		// no action needed to go idle
		system_menu_action(SYSTEM_MENU_ACTION_NONE);
		break;
	case GRAPHIC_LCD_SELECT:
		system_menu_action(SYSTEM_MENU_ACTION_SELECT);
		break;
	case GRAPHIC_LCD_NEXT:
		system_menu_action(SYSTEM_MENU_ACTION_NEXT);
		break;
	case GRAPHIC_LCD_PREV:
		system_menu_action(SYSTEM_MENU_ACTION_PREV);
		break;
	}

	// render menu
	system_menu_render();

	return STATUS_OK;
}

CREATE_EVENT_LISTENER(cnc_io_dotasks, graphic_lcd_update);
#endif

DECL_MODULE(graphic_lcd)
{
	system_menu_init();
// u8g2_Setup_st7920_s_128x64_f(U8G2, U8G2_R0, u8x8_byte_4wire_sw_spi, u8x8_gpio_and_delay_ucnc);
#if (BOARD == BOARD_VIRTUAL)
	u8g2_SetupBuffer_SDL_128x64(U8G2, &u8g2_cb_r0);
#else
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(U8G2, U8G2_R0, /*u8x8_byte_sw_i2c*/ u8x8_byte_ucnc_hw_i2c, u8x8_gpio_and_delay_ucnc);
#endif
	u8g2_InitDisplay(U8G2); // send init sequence to the display, display is in sleep mode after this,
	u8g2_ClearDisplay(U8G2);
	u8g2_SetPowerSave(U8G2, 0); // wake up display
	u8g2_FirstPage(U8G2);
	// clear
	u8g2_ClearBuffer(U8G2);
	u8g2_SetFont(U8G2, u8g2_font_6x12_tr);
	u8g2_NextPage(U8G2);

// adds the display loop
#ifdef ENABLE_MAIN_LOOP_MODULES
	// ADD_EVENT_LISTENER(cnc_reset, graphic_lcd_start);
	ADD_EVENT_LISTENER(cnc_io_dotasks, graphic_lcd_update);
#else
#warning "Main loop extensions are not enabled. SD card will not work."
#endif
}

// system menu overrides

void system_menu_render_startup(void)
{
	u8g2_ClearBuffer(U8G2);
	char buff[SYSTEM_MENU_MAX_STR_LEN];
	rom_strcpy(buff, __romstr__("µCNC"));
	u8g2_ClearBuffer(U8G2);
	u8g2_SetFont(U8G2, u8g2_font_9x15_t_symbols);
	u8g2_DrawUTF8X2(U8G2, (LCDWIDTH / 2 - u8g2_GetUTF8Width(U8G2, buff)), JUSTIFY_CENTER - FONTHEIGHT / 2, buff);
	rom_strcpy(buff, __romstr__(("v" CNC_VERSION)));
	u8g2_SetFont(U8G2, u8g2_font_6x12_tr);
	u8g2_DrawStr(U8G2, ALIGN_CENTER(buff), JUSTIFY_CENTER + FONTHEIGHT, buff);
	u8g2_SendBuffer(U8G2);
	u8g2_NextPage(U8G2);
}

void system_menu_render_idle(void)
{
	u8g2_ClearBuffer(U8G2);
	// starts from the bottom up

	// coordinates
	char buff[SYSTEM_MENU_MAX_STR_LEN];
	uint8_t y = JUSTIFY_BOTTOM;

	float axis[MAX(AXIS_COUNT, 3)];
	int32_t steppos[STEPPER_COUNT];
	itp_get_rt_position(steppos);
	kinematics_apply_forward(steppos, axis);
	kinematics_apply_reverse_transform(axis);

#if (AXIS_COUNT >= 4)
	memset(buff, 0, 32);
	buff[0] = 'A';
	system_menu_flt_to_str(&buff[1], axis[3]);
	u8g2_DrawStr(U8G2, ALIGN_LEFT, y, buff);

#if (AXIS_COUNT >= 5)
	buff[0] = 'B';
	system_menu_flt_to_str(&buff[1], axis[4]);
	u8g2_DrawStr(U8G2, ALIGN_CENTER(buff), y, buff);
#endif
#if (AXIS_COUNT >= 6)
	buff[0] = 'C';
	system_menu_flt_to_str(&buff[1], axis[5]);
	u8g2_DrawStr(U8G2, ALIGN_RIGHT(buff), y, buff);
#endif
	y -= (FONTHEIGHT + 3);
#endif

	memset(buff, 0, 32);
	u8g2_DrawLine(U8G2, 0, y - FONTHEIGHT - 1, LCDWIDTH, y - FONTHEIGHT - 1);

#if (AXIS_COUNT >= 1)
	buff[0] = 'X';
	system_menu_flt_to_str(&buff[1], axis[0]);
	u8g2_DrawStr(U8G2, ALIGN_LEFT, y, buff);
#endif
#if (AXIS_COUNT >= 2)
	buff[0] = 'Y';
	system_menu_flt_to_str(&buff[1], axis[1]);
	u8g2_DrawStr(U8G2, ALIGN_CENTER(buff), y, buff);
#endif
#if (AXIS_COUNT >= 3)
	buff[0] = 'Z';
	system_menu_flt_to_str(&buff[1], axis[2]);
	u8g2_DrawStr(U8G2, ALIGN_RIGHT(buff), y, buff);
#endif

	memset(buff, 0, 32);
	y -= (FONTHEIGHT + 3);

	// units, feed and tool
	if (g_settings.report_inches)
	{
		rom_strcpy(buff, __romstr__("IN F "));
	}
	else
	{
		rom_strcpy(buff, __romstr__("MM F "));
	}

	// Realtime feed
	system_menu_flt_to_str(&buff[5], itp_get_rt_feed());
	u8g2_DrawStr(U8G2, ALIGN_LEFT, y, buff);
	memset(buff, 0, 32);

	// Tool
	char tool[5];
	uint8_t modalgroups[14];
	uint16_t feed;
	uint16_t spindle;
	uint8_t coolant;
	parser_get_modes(modalgroups, &feed, &spindle, &coolant);
	rom_strcpy(tool, __romstr__(" T "));
	system_menu_int_to_str(&tool[3], modalgroups[11]);
	// Realtime tool speed
	rom_strcpy(buff, __romstr__("S "));
	system_menu_int_to_str(&buff[2], tool_get_speed());
	strcat(buff, tool);
	u8g2_DrawStr(U8G2, ALIGN_RIGHT(buff), y, buff);
	memset(buff, 0, 32);
	u8g2_DrawLine(U8G2, 0, y - FONTHEIGHT - 1, LCDWIDTH, y - FONTHEIGHT - 1);

	y -= (FONTHEIGHT + 3);

	// system status
	uint8_t i;

	rom_strcpy(buff, __romstr__("St:"));
	uint8_t state = cnc_get_exec_state(0xFF);
	uint8_t filter = 0x80;
	while (!(state & filter) && filter)
	{
		filter >>= 1;
	}

	state &= filter;
	if (cnc_has_alarm())
	{
		rom_strcpy(&buff[3], MSG_STATUS_ALARM);
	}
	else if (mc_get_checkmode())
	{
		rom_strcpy(&buff[3], MSG_STATUS_CHECK);
	}
	else
	{
		switch (state)
		{
		case EXEC_DOOR:
			rom_strcpy(&buff[3], MSG_STATUS_DOOR);
			break;
		case EXEC_KILL:
		case EXEC_UNHOMED:
			rom_strcpy(&buff[3], MSG_STATUS_ALARM);
			break;
		case EXEC_HOLD:
			rom_strcpy(&buff[3], MSG_STATUS_HOLD);
			break;
		case EXEC_HOMING:
			rom_strcpy(&buff[3], MSG_STATUS_HOME);
			break;
		case EXEC_JOG:
			rom_strcpy(&buff[3], MSG_STATUS_JOG);
			break;
		case EXEC_RUN:
			rom_strcpy(&buff[3], MSG_STATUS_RUN);
			break;
		default:
			rom_strcpy(&buff[3], MSG_STATUS_IDLE);
			break;
		}
	}
	u8g2_DrawStr(U8G2, ALIGN_LEFT, y, buff);
	memset(buff, 0, 32);

	uint8_t controls = io_get_controls();
	uint8_t limits = io_get_limits();
	uint8_t probe = io_get_probe();
	rom_strcpy(buff, __romstr__("Sw:"));
	i = 3;
	if (CHECKFLAG(controls, (ESTOP_MASK | SAFETY_DOOR_MASK | FHOLD_MASK)) || CHECKFLAG(limits, LIMITS_MASK) || probe)
	{
		if (CHECKFLAG(controls, ESTOP_MASK))
		{
			buff[i++] = 'R';
		}

		if (CHECKFLAG(controls, SAFETY_DOOR_MASK))
		{
			buff[i++] = 'D';
		}

		if (CHECKFLAG(controls, FHOLD_MASK))
		{
			buff[i++] = 'H';
		}

		if (probe)
		{
			buff[i++] = 'P';
		}

		if (CHECKFLAG(limits, LIMIT_X_MASK))
		{
			buff[i++] = 'X';
		}

		if (CHECKFLAG(limits, LIMIT_Y_MASK))
		{
			buff[i++] = 'Y';
		}

		if (CHECKFLAG(limits, LIMIT_Z_MASK))
		{
			buff[i++] = 'Z';
		}

		if (CHECKFLAG(limits, LIMIT_A_MASK))
		{
			buff[i++] = 'A';
		}

		if (CHECKFLAG(limits, LIMIT_B_MASK))
		{
			buff[i++] = 'B';
		}

		if (CHECKFLAG(limits, LIMIT_C_MASK))
		{
			buff[i++] = 'C';
		}
	}
	u8g2_DrawStr(U8G2, (LCDWIDTH >> 1), y, buff);
	u8g2_NextPage(U8G2);
}

static uint8_t y_coord;

void system_menu_render_header(const char *__s)
{
	u8g2_ClearBuffer(U8G2);
	char buff[SYSTEM_MENU_MAX_STR_LEN];
	rom_strcpy(buff, __s);
	u8g2_DrawStr(U8G2, ALIGN_CENTER(buff), JUSTIFY_TOP + 1, buff);
	u8g2_DrawLine(U8G2, 0, FONTHEIGHT + 2, LCDWIDTH, FONTHEIGHT + 2);
	y_coord = 2;
}

void system_menu_render_nav_back(bool is_hover)
{
	u8g2_SetDrawColor(U8G2, 1);
	uint8_t mode = U8G2_BTN_BW1;
	if (is_hover)
	{
		mode |= U8G2_BTN_INV;
	}

	u8g2_DrawButtonUTF8(U8G2, ALIGN_RIGHT("X") - 2, FONTHEIGHT - 1, mode, TEXT_WIDTH("X") + 2, 2, 1, "X");
}

void system_menu_item_render_label(uint8_t render_flags, const char *label)
{
	y_coord += FONTHEIGHT + 1;
	uint8_t y = y_coord;
	if (label)
	{
		if (render_flags & SYSTEM_MENU_MODE_EDIT)
		{
			y_coord += FONTHEIGHT + 1;
			y = y_coord;
			u8g2_SetDrawColor(U8G2, 1);
			u8g2_DrawStr(U8G2, ALIGN_CENTER(label), y + JUSTIFY_TOP + 1, label);
			return;
		}

		u8g2_SetDrawColor(U8G2, 1);
		if (render_flags & SYSTEM_MENU_MODE_SELECT)
		{
			u8g2_DrawBox(U8G2, ALIGN_LEFT, y, LCDWIDTH, FONTHEIGHT + 1);
			u8g2_SetDrawColor(U8G2, 0);
		}
		u8g2_DrawStr(U8G2, ALIGN_LEFT, y + JUSTIFY_TOP + 1, label);
	}
}

void system_menu_item_render_arg(uint8_t render_flags, const char *value)
{
	if (value)
	{
		uint8_t y = y_coord;

		if (render_flags & SYSTEM_MENU_MODE_EDIT)
		{
			y_coord += 2 * (FONTHEIGHT + 1);
			uint8_t start_pos = ALIGN_CENTER(value);
			u8g2_DrawStr(U8G2, start_pos, y_coord + JUSTIFY_TOP + 1, value);
			y_coord += FONTHEIGHT;
			y = y_coord;
			char *dot = strchr(value, '.');
			uint8_t base_pos = start_pos + TEXT_WIDTH(value);
			int8_t mult = g_system_menu.current_multiplier + 1;
			if (dot && mult > 3)
			{
				// jump the comma position
				mult++;
			}

			if (mult > 0)
			{
				base_pos -= 6 * mult;
				if (render_flags & SYSTEM_MENU_MODE_MODIFY)
				{
					u8g2_DrawTriangle(U8G2, base_pos, y - FONTHEIGHT, base_pos + 6, y - FONTHEIGHT, base_pos + 3, y - FONTHEIGHT - 4);
					u8g2_DrawTriangle(U8G2, base_pos + 1, y, base_pos + 6, y, base_pos + 3, y + 3);
				}
				else
				{
					u8g2_DrawBox(U8G2, base_pos + 1, y, 5, 2);
				}
			}

			return;
		}

		u8g2_DrawStr(U8G2, ALIGN_RIGHT(value), y + JUSTIFY_TOP + 1, value);
	}
}

void system_menu_render_footer(void)
{
	u8g2_SetDrawColor(U8G2, 1);
	u8g2_NextPage(U8G2);
}

bool system_menu_render_menu_item_filter(uint8_t item_index)
{
	static uint8_t menu_top = 0;
	uint8_t current_index = MAX(0, g_system_menu.current_index);

	uint8_t top = menu_top;
	if ((top + GRAPHIC_LCD_MAX_LINES) <= current_index)
	{
		// advance menu top
		menu_top = top = (current_index - GRAPHIC_LCD_MAX_LINES + 1);
	}

	if (top > current_index)
	{
		// rewind menu top
		menu_top = top = current_index;
	}

	return ((top <= item_index) && (item_index < (top + GRAPHIC_LCD_MAX_LINES)));
}
