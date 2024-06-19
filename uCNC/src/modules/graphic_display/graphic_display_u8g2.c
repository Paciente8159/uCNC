/*
	Name: graphic_display_u8g2.c
	Description: Graphic LCD module for µCNC using u8g2 lib.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 15-06-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#ifndef GRAPHIC_DISPLAY_LIB

#include <clib/u8g2.h>
#include <clib/u8x8.h>
#include "graphic_display.h"
#include "../softspi.h"
#include "../softi2c.h"
#include "../system_menu.h"

static u8g2_t graphiclcd_u8g2;
#define U8G2 ((u8g2_t *)&graphiclcd_u8g2)
static void *graphic_port;
static int8_t graphic_last_line_offset;

uint8_t u8x8_byte_ucnc_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	cnc_dotasks();

	uint8_t *data;
	switch (msg)
	{
	case U8X8_MSG_BYTE_SEND:
		data = (uint8_t *)arg_ptr;
		while (arg_int > 0)
		{
			softspi_xmit((softspi_port_t *)graphic_port, (uint8_t)*data);
			data++;
			arg_int--;
			cnc_dotasks();
		}
		break;
	case U8X8_MSG_BYTE_INIT:
		u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
		break;
	case U8X8_MSG_BYTE_SET_DC:
		u8x8_gpio_SetDC(u8x8, arg_int);
		break;
	case U8X8_MSG_BYTE_START_TRANSFER:
		softspi_config((softspi_port_t *)graphic_port, u8x8->display_info->spi_mode, u8x8->display_info->sck_clock_hz);
		/* SPI mode has to be mapped to the mode of the current controller, at least Uno, Due, 101 have different SPI_MODEx values */
		u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);
		u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->post_chip_enable_wait_ns, NULL);
		break;
	case U8X8_MSG_BYTE_END_TRANSFER:
		u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->pre_chip_disable_wait_ns, NULL);
		u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
		break;
	default:
		return 0;
	}
	return 1;
}

uint8_t u8x8_byte_ucnc_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
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
#if (UCNC_MODULE_VERSION < 10808)
		softi2c_send((softi2c_port_t *)graphic_port, u8x8_GetI2CAddress(u8x8) >> 1, i2c_buffer, i2c_buffer_offset, true);
#else
		softi2c_send((softi2c_port_t *)graphic_port, u8x8_GetI2CAddress(u8x8) >> 1, i2c_buffer, i2c_buffer_offset, true, 20);
#endif
		i2c_buffer_offset = 0;
		break;
	default:
		return 0;
	}

	return 1;
}

uint8_t u8x8_gpio_and_delay_ucnc(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	cnc_dotasks();

	switch (msg)
	{
	case U8X8_MSG_GPIO_AND_DELAY_INIT: // called once during init phase of u8g2/u8x8
		break;													 // can be used to setup pins
	case U8X8_MSG_DELAY_NANO:					 // delay arg_int * 1 nano second
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
		break;							 // arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
	case U8X8_MSG_GPIO_D0: // D0 or SPI clock pin: Output level in arg_int
#if GRAPHIC_DISPLAY_SPI_CLOCK != UNDEF_PIN
		io_set_pinvalue(GRAPHIC_DISPLAY_SPI_CLOCK, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D1: // D1 or SPI data pin: Output level in arg_int
#if GRAPHIC_DISPLAY_SPI_DATA != UNDEF_PIN
		io_set_pinvalue(GRAPHIC_DISPLAY_SPI_DATA, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D2: // D2 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D2_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_D2_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D3: // D3 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D3_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_D3_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D4: // D4 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D4_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_D4_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D5: // D5 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D5_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_D5_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D6: // D6 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D6_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_D6_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D7: // D7 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D7_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_D7_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_E: // E/WR pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_E_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_E_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_CS: // CS (chip select) pin: Output level in arg_int
#if GRAPHIC_DISPLAY_SPI_CS != UNDEF_PIN
		io_set_pinvalue(GRAPHIC_DISPLAY_SPI_CS, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_DC: // DC (data/cmd, A0, register select) pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_DC_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_DC_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_RESET: // Reset pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_RESET_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_RESET_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_CS1: // CS1 (chip select) pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_CS1_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_CS1_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_CS2: // CS2 (chip select) pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_CS2_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_CS2_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_I2C_CLOCK: // arg_int=0: Output low at I2C clock pin
#if GRAPHIC_DISPLAY_I2C_CLOCK != UNDEF_PIN
		if (arg_int)
		{
			mcu_config_input(GRAPHIC_DISPLAY_I2C_CLOCK);
			mcu_config_pullup(GRAPHIC_DISPLAY_I2C_CLOCK);
			u8x8_SetGPIOResult(u8x8, mcu_get_input(GRAPHIC_DISPLAY_I2C_CLOCK));
		}
		else
		{
			mcu_config_output(GRAPHIC_DISPLAY_I2C_CLOCK);
			mcu_clear_output(GRAPHIC_DISPLAY_I2C_CLOCK);
			u8x8_SetGPIOResult(u8x8, 0);
		}
#endif
		break;										 // arg_int=1: Input dir with pullup high for I2C clock pin
	case U8X8_MSG_GPIO_I2C_DATA: // arg_int=0: Output low at I2C data pin
#if GRAPHIC_DISPLAY_I2C_DATA != UNDEF_PIN
		if (arg_int)
		{
			mcu_config_input(GRAPHIC_DISPLAY_I2C_DATA);
			mcu_config_pullup(GRAPHIC_DISPLAY_I2C_DATA);
			u8x8_SetGPIOResult(u8x8, mcu_get_input(GRAPHIC_DISPLAY_I2C_DATA));
		}
		else
		{
			mcu_config_output(GRAPHIC_DISPLAY_I2C_DATA);
			mcu_clear_output(GRAPHIC_DISPLAY_I2C_DATA);
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

int16_t __attribute__((weak)) gd_font_height(void)
{
	return 9; // u8g2_GetAscent(U8G2) - u8g2_GetDescent(U8G2);
}

void __attribute__((weak)) gd_init(display_driver_t *driver, void *port_interface)
{
	graphic_port = port_interface;
	driver->init();
	u8g2_InitDisplay(U8G2); // send init sequence to the display, display is in sleep mode after this,
	u8g2_ClearDisplay(U8G2);
	u8g2_SetPowerSave(U8G2, 0); // wake up display
	u8g2_FirstPage(U8G2);
	graphic_last_line_offset = -1;
	//	driver->width = u8g2_GetDisplayWidth(U8G2);
	//	driver->height = u8g2_GetDisplayHeight(U8G2);
}

void __attribute__((weak)) gd_clear()
{
	u8g2_ClearBuffer(U8G2);
}

void __attribute__((weak)) gd_flush()
{
	u8g2_SendBuffer(U8G2);
}

void __attribute__((weak)) gd_draw_startup(void)
{
	u8g2_ClearBuffer(U8G2);
	char buff[SYSTEM_MENU_MAX_STR_LEN];
	rom_strcpy(buff, __romstr__("µCNC"));
	u8g2_ClearBuffer(U8G2);
	u8g2_SetFont(U8G2, u8g2_font_9x15_t_symbols);
	u8g2_DrawUTF8X2(U8G2, (u8g2_GetDisplayWidth(U8G2) / 2 - u8g2_GetUTF8Width(U8G2, buff)), gd_str_justify_center(), buff);
	rom_strcpy(buff, __romstr__(("v" CNC_VERSION)));
	u8g2_SetFont(U8G2, u8g2_font_6x12_tr);
	u8g2_DrawStr(U8G2, gd_str_align_center(buff), (u8g2_GetDisplayHeight(U8G2) + u8g2_GetDescent(U8G2)), buff);
	u8g2_SendBuffer(U8G2);
}

void __attribute__((weak)) gd_draw_h_line(int16_t y0)
{
	u8g2_DrawHLine(U8G2, 0, y0, u8g2_GetDisplayWidth(U8G2));
}

void __attribute__((weak)) gd_draw_rectangle(int16_t x0, int16_t y0, int16_t w, int16_t h)
{
	u8g2_DrawFrame(U8G2, x0, y0, w, h);
}

void __attribute__((weak)) gd_draw_rectangle_fill(int16_t x0, int16_t y0, int16_t w, int16_t h, bool invert)
{
	if (invert)
	{
		u8g2_SetDrawColor(U8G2, 0);
	}
	u8g2_DrawBox(U8G2, x0, y0, w, h);
	if (invert)
	{
		u8g2_SetDrawColor(U8G2, 1);
	}
}

	int16_t __attribute__((weak)) gd_half_padding(void){
		return 1;
	}

void __attribute__((weak)) gd_draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool invert)
{
	if (invert)
	{
		u8g2_SetDrawColor(U8G2, 0);
	}
	u8g2_DrawTriangle(U8G2, x0, y0, x1, y1, x2, y2);
	if (invert)
	{
		u8g2_SetDrawColor(U8G2, 1);
	}
}

void __attribute__((weak)) gd_draw_string(int16_t x0, int16_t y0, const char *s)
{
	u8g2_DrawStr(U8G2, x0, y0 + u8g2_GetAscent(U8G2) + 2, s);
}

void __attribute__((weak)) gd_draw_string_inv(int16_t x0, int16_t y0, const char *s, bool invert)
{
	if (invert)
	{
		u8g2_SetDrawColor(U8G2, 0);
	}
	u8g2_DrawStr(U8G2, x0, y0 + u8g2_GetAscent(U8G2) + 2, s);
	if (invert)
	{
		u8g2_SetDrawColor(U8G2, 1);
	}
}

void __attribute__((weak)) gd_draw_button(int16_t x0, int16_t y0, const char *s, int16_t minw, bool invert, bool frameless)
{
	y0 += u8g2_GetAscent(U8G2) + 2;
	uint8_t mode = (!frameless) ? U8G2_BTN_BW1 : U8G2_BTN_BW0;
	mode |= (!invert) ? 0 : U8G2_BTN_INV;
	if (minw < 0)
	{
		minw = ABS(minw);
		x0 -= MAX(gd_str_width(s) + 5, minw);
	}

	u8g2_DrawButtonUTF8(U8G2, x0, y0, mode, minw, 1, 1, s);
}

int16_t __attribute__((weak)) gd_str_width(const char *s)
{
	return u8g2_GetUTF8Width(U8G2, s);
}

int16_t __attribute__((weak)) gd_line_height(void)
{
	return (gd_font_height() + 3);
}

int16_t __attribute__((weak)) gd_str_align_start(const char *s)
{
	return 0;
}

int16_t __attribute__((weak)) gd_str_align_center(const char *s)
{
	return ((u8g2_GetDisplayWidth(U8G2) - gd_str_width(s)) >> 1);
}

int16_t __attribute__((weak)) gd_str_align_end(const char *s)
{
	return (u8g2_GetDisplayWidth(U8G2) - gd_str_width(s));
}

int16_t __attribute__((weak)) gd_str_justify_start(void)
{
	return u8g2_GetAscent(U8G2);
}

int16_t __attribute__((weak)) gd_str_justify_center(void)
{
	return (u8g2_GetDisplayHeight(U8G2) + gd_font_height()) / 2;
}

int16_t __attribute__((weak)) gd_str_justify_end(void)
{
	return u8g2_GetDisplayHeight(U8G2) + u8g2_GetDescent(U8G2);
}

int16_t __attribute__((weak)) gd_get_line_top(int8_t line)
{
	// line height plus padding
	int8_t offset = graphic_last_line_offset;
	if (offset < 0)
	{
		offset = u8g2_GetDisplayHeight(U8G2) - (gd_font_height() + 3) * floor(u8g2_GetDisplayHeight(U8G2) / (gd_font_height() + 3));
		graphic_last_line_offset = offset;
	}
	return line * (gd_font_height() + 3) + offset;
}

/**
 * Create some U8G2 display drivers
 */

void ssd1306_128x64_i2c_init()
{
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(U8G2, U8G2_R0, u8x8_byte_ucnc_hw_i2c, u8x8_gpio_and_delay_ucnc);
}

const display_driver_t gd_ssd1306_128x64_i2c = {
		.width = 128,
		.height = 60,
		.init = &ssd1306_128x64_i2c_init};

void st7920_128x64_spi_init()
{
	u8g2_Setup_st7920_s_128x64_f(U8G2, U8G2_R0, u8x8_byte_ucnc_hw_spi, u8x8_gpio_and_delay_ucnc);
}

const display_driver_t gd_st7920_128x64_spi = {
		.width = 128,
		.height = 64,
		.init = &st7920_128x64_spi_init};

void virtual_sdl_init()
{
	u8g2_SetupBuffer_SDL_128x64(U8G2, U8G2_R0);
}

const display_driver_t gd_virtual_sdl = {
		.width = 128,
		.height = 64,
		.init = &virtual_sdl_init};

#endif
