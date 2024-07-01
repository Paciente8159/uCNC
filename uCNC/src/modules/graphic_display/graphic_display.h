/*
	Name: graphic_display.h
	Description: Defines the graphic_display interface.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 15-06-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef GRAPHIC_DISPLAY_H
#define GRAPHIC_DISPLAY_H

#ifdef __cplusplus
extern "C"
{
#endif

// #include "../system_menu.h"
#include <stdint.h>
#include <stdbool.h>

#define GRAPHIC_DISPLAY_SW_SPI 1
#define GRAPHIC_DISPLAY_HW_SPI 2
#define GRAPHIC_DISPLAY_SW_I2C 4
#define GRAPHIC_DISPLAY_HW_I2C 8

#ifndef GRAPHIC_DISPLAY_INTERFACE
#define GRAPHIC_DISPLAY_INTERFACE GRAPHIC_DISPLAY_HW_SPI
#endif

#if (GRAPHIC_DISPLAY_INTERFACE == GRAPHIC_DISPLAY_SW_SPI)
#ifndef GRAPHIC_DISPLAY_SPI_CLOCK
#define GRAPHIC_DISPLAY_SPI_CLOCK DOUT4
#endif
#ifndef GRAPHIC_DISPLAY_SPI_DATA
#define GRAPHIC_DISPLAY_SPI_DATA DOUT5
#endif
#define GRAPHIC_DISPLAY_SPI_MOSI GRAPHIC_DISPLAY_SPI_DATA
#ifndef GRAPHIC_DISPLAY_SPI_MISO
#define GRAPHIC_DISPLAY_SPI_MISO DOUT7
#endif
#endif
#ifndef GRAPHIC_DISPLAY_SPI_CS
#define GRAPHIC_DISPLAY_SPI_CS DOUT6
#endif

#if (GRAPHIC_DISPLAY_INTERFACE == GRAPHIC_DISPLAY_SW_I2C)
#ifndef GRAPHIC_DISPLAY_I2C_CLOCK
#define GRAPHIC_DISPLAY_I2C_CLOCK DIN30
#endif
#ifndef GRAPHIC_DISPLAY_I2C_DATA
#define GRAPHIC_DISPLAY_I2C_DATA DIN31
#endif
#endif

	typedef struct display_driver_
	{
		int16_t width;			// returns the width of the display in pixels.
		int16_t height;			// returns the height of the display in pixels.
		void (*init)(void); // initializes the display.
	} display_driver_t;

	void gd_clear();
	void gd_flush();
	void gd_init(display_driver_t *driver, void *port_interface);
	void gd_draw_startup(void);
	void gd_draw_h_line(int16_t y0);
	void gd_draw_rectangle(int16_t x0, int16_t y0, int16_t w, int16_t h);
	void gd_draw_rectangle_fill(int16_t x0, int16_t y0, int16_t w, int16_t h, bool invert);
	void gd_draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool invert);
	void gd_draw_string(int16_t x0, int16_t y0, const char *s);
	void gd_draw_string_inv(int16_t x0, int16_t y0, const char *s, bool invert);
	void gd_draw_button(int16_t x0, int16_t y0, const char *s, int16_t minw, int16_t minh, bool invert, bool frameless);
	int16_t gd_str_width(const char *s);
	int16_t gd_str_align_start(const char *s);
	int16_t gd_str_align_center(const char *s);
	int16_t gd_str_align_end(const char *s);
	int16_t gd_str_justify_center(void);
	int16_t gd_font_height(void);
	int16_t gd_line_height(void);
	int16_t gd_half_padding(void);
	int16_t gd_get_line_top(int8_t line);
	uint8_t gd_display_max_lines(void);

/**
 * Helper macros for declaring a new display driver
 */
#define DECL_DISPLAY(name, w, h)                                                      \
	extern void name##_init();                                                          \
	const display_driver_t gd_##name = {.width = w, .height = h, .init = &name##_init}; \

#define DISPLAY(name) void name##_init()
#define DISPLAY_PTR(name) (display_driver_t *)&gd_##name

#ifdef __cplusplus
}
#endif

#endif
