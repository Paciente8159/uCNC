/*
	Name: graphic_display.c
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

#include "graphic_display.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "../system_menu.h"

#if (UCNC_MODULE_VERSION < 10801 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

// used with graphic_display module
#ifndef GRAPHIC_DISPLAY_BEEP
#define GRAPHIC_DISPLAY_BEEP DOUT7
#endif

#ifndef GRAPHIC_DISPLAY_MAX_LINES
#define GRAPHIC_DISPLAY_MAX_LINES 5
#endif

#ifndef GRAPHIC_DISPLAY_REFRESH
#define GRAPHIC_DISPLAY_REFRESH 1000
#endif

#define GRAPHIC_DISPLAY_SELECT 1
#define GRAPHIC_DISPLAY_NEXT 2
#define GRAPHIC_DISPLAY_PREV 4
#define GRAPHIC_DISPLAY_UP 8
#define GRAPHIC_DISPLAY_DOWN 16
#define GRAPHIC_DISPLAY_HOME 32

static int16_t display_width;
static int16_t display_height;
static uint8_t display_max_lines;
static uint8_t display_char_width;

static uint8_t graphic_display_str_lines(const char *__s, int16_t *max_len);
static uint8_t graphic_display_str_line_len(const char *__s);

/**
 *
 * can also be done via hardware SPI and I2C ports of µCNC
 * but is not needed
 *
 * */
#if (GRAPHIC_DISPLAY_INTERFACE & (GRAPHIC_DISPLAY_SW_SPI | GRAPHIC_DISPLAY_HW_SPI))
#include "../softspi.h"
#if (GRAPHIC_DISPLAY_INTERFACE == GRAPHIC_DISPLAY_SW_SPI)
// temporary result of reading non existing read pin
#define io0_get_input 0
SOFTSPI(graphic_spi, 1000000UL, 0, GRAPHIC_DISPLAY_SPI_MOSI, GRAPHIC_DISPLAY_SPI_MISO, GRAPHIC_DISPLAY_SPI_CLOCK)
// delete temporary definition
#undef io0_get_input
#define graphic_display_port ((void *)&graphic_spi)
#else
#define graphic_display_port NULL
#endif
#endif

#if (GRAPHIC_DISPLAY_INTERFACE & (GRAPHIC_DISPLAY_SW_I2C | GRAPHIC_DISPLAY_HW_I2C))
#include "../softi2c.h"
#if (GRAPHIC_DISPLAY_INTERFACE == GRAPHIC_DISPLAY_SW_I2C)
SOFTI2C(graphic_i2c, 100000UL, GRAPHIC_DISPLAY_I2C_CLOCK, GRAPHIC_DISPLAY_I2C_DATA)
#define graphic_display_port ((void *)&graphic_i2c)
#else
#define graphic_display_port NULL
#endif
#endif

#ifndef GRAPHIC_DISPLAY_ENCODER_BTN
#define GRAPHIC_DISPLAY_ENCODER_BTN DIN16
#endif
#ifndef GRAPHIC_DISPLAY_ENCODER_ENC1
#define GRAPHIC_DISPLAY_ENCODER_ENC1 DIN17
#endif
#ifndef GRAPHIC_DISPLAY_ENCODER_ENC2
#define GRAPHIC_DISPLAY_ENCODER_ENC2 DIN18
#endif
#ifndef GRAPHIC_DISPLAY_ENCODER_DEBOUNCE_MS
#define GRAPHIC_DISPLAY_ENCODER_DEBOUNCE_MS 200
#endif

static int8_t graphic_display_rotary_encoder_counter;
static int8_t graphic_display_rotary_encoder_pressed;

// reads inputs and returns a mask with a pin state transition
uint8_t graphic_display_rotary_encoder_control(void)
{
	if (graphic_display_rotary_encoder_pressed != 0)
	{
		graphic_display_rotary_encoder_pressed = 0;
		return GRAPHIC_DISPLAY_SELECT;
	}

	if (graphic_display_rotary_encoder_counter > 0)
	{
		graphic_display_rotary_encoder_counter = 0;
		return GRAPHIC_DISPLAY_NEXT;
	}

	if (graphic_display_rotary_encoder_counter < 0)
	{
		graphic_display_rotary_encoder_counter = 0;
		return GRAPHIC_DISPLAY_PREV;
	}

	return 0;
}

// static bool graphic_display_current_menu_active;
#ifdef ENABLE_MAIN_LOOP_MODULES
bool graphic_display_start(void *args)
{
	// clear
	system_menu_render_startup();

	return false;
}
CREATE_EVENT_LISTENER(cnc_reset, graphic_display_start);

bool graphic_display_alarm(void *args)
{
	// renders the alarm
	system_menu_render();

	return EVENT_CONTINUE;
}

bool graphic_display_rotary_encoder_control_sample(void *args)
{
	static uint8_t last_pin_state = 0;
	static uint8_t last_rot_transition = 0;
	uint8_t pin_state = 0;
	static uint32_t long_press_timeout = 0;
	// btn debounce
	static uint32_t short_press_timeout = 0;

// rotation encoder
#ifndef GRAPHIC_DISPLAY_INVERT_ENCODER_DIR
	pin_state |= io_get_pinvalue(GRAPHIC_DISPLAY_ENCODER_ENC1) ? 2 : 0;
	pin_state |= io_get_pinvalue(GRAPHIC_DISPLAY_ENCODER_ENC2) ? 4 : 0;
#else
	pin_state |= io_get_pinvalue(GRAPHIC_DISPLAY_ENCODER_ENC1) ? 4 : 0;
	pin_state |= io_get_pinvalue(GRAPHIC_DISPLAY_ENCODER_ENC2) ? 2 : 0;
#endif

	int8_t counter = graphic_display_rotary_encoder_counter;
	uint8_t last_rot = last_rot_transition;

	switch (pin_state)
	{
	case 6:
		last_rot_transition = 0;
		break;
	case 4:
		if (last_rot == 0 || last_rot == 1)
		{
			last_rot_transition = 1;
		}
		else
		{
			last_rot_transition = 5;
		}
		break;
	case 2:
		if (last_rot == 0 || last_rot == 2)
		{
			last_rot_transition = 2;
		}
		else
		{
			last_rot_transition = 5;
		}
		break;
	default:
		if (last_rot == 1)
		{
			if (counter < 127)
			{
				graphic_display_rotary_encoder_counter++;
			}
		}

		if (last_rot == 2)
		{
			if (counter > -127)
			{
				graphic_display_rotary_encoder_counter--;
			}
		}

		last_rot_transition = 3;
		break;
	}

	pin_state = !io_get_pinvalue(GRAPHIC_DISPLAY_ENCODER_BTN) ? 1 : 0;
	io_set_pinvalue(GRAPHIC_DISPLAY_BEEP, pin_state);
	// if btn is pressed
	if ((pin_state & 1))
	{
		uint32_t long_press = long_press_timeout;
		if (long_press && long_press < mcu_millis())
		{
			// forces a soft reset
			cnc_call_rt_command(0x18);
			long_press_timeout = 0;
		}
	}
	else
	{
		// resets long press timer
		long_press_timeout = 0;
	}

	uint8_t pin_diff = last_pin_state ^ pin_state;
	if (pin_diff)
	{
		// if btn is pressed (1st transition)
		if ((pin_state & 1))
		{
			// set soft reset timeout (5s)
			long_press_timeout = mcu_millis() + 5000;
		}
		last_pin_state = pin_state;
		if (pin_diff & pin_state)
		{
			uint32_t short_press = short_press_timeout;
			if (short_press < mcu_millis())
			{
				short_press_timeout = mcu_millis() + GRAPHIC_DISPLAY_ENCODER_DEBOUNCE_MS;
				graphic_display_rotary_encoder_pressed++;
			}
		}
	}

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_io_dotasks, graphic_display_rotary_encoder_control_sample);

bool graphic_display_update(void *args)
{
	static bool running = false;

	if (!running)
	{
		running = true;
		uint8_t action = SYSTEM_MENU_ACTION_NONE;
		switch (graphic_display_rotary_encoder_control())
		{
		case GRAPHIC_DISPLAY_SELECT:
			action = SYSTEM_MENU_ACTION_SELECT;
			// prevent double click
			graphic_display_rotary_encoder_pressed = 0;
			break;
		case GRAPHIC_DISPLAY_NEXT:
			action = SYSTEM_MENU_ACTION_NEXT;
			break;
		case GRAPHIC_DISPLAY_PREV:
			action = SYSTEM_MENU_ACTION_PREV;
			break;
		}

		system_menu_action(action);

		cnc_dotasks();
		// render menu
		system_menu_render();
		cnc_dotasks();

		running = false;
	}

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, graphic_display_update);
CREATE_EVENT_LISTENER(cnc_alarm, graphic_display_update);
#endif

#ifdef DECL_SERIAL_STREAM
DECL_BUFFER(uint8_t, graphic_stream_buffer, 32);
static uint8_t graphic_display_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(graphic_stream_buffer, &c);
	return c;
}

uint8_t graphic_display_available(void)
{
	return BUFFER_READ_AVAILABLE(graphic_stream_buffer);
}

void graphic_display_clear(void)
{
	BUFFER_CLEAR(graphic_stream_buffer);
}

DECL_SERIAL_STREAM(graphic_stream, graphic_display_getc, graphic_display_available, graphic_display_clear, NULL, NULL);

uint8_t system_menu_send_cmd(const char *__s)
{
	// if machine is running rejects the command
	if (cnc_get_exec_state(EXEC_RUN | EXEC_JOG) == EXEC_RUN)
	{
		return STATUS_SYSTEM_GC_LOCK;
	}

	uint8_t len = strlen(__s);
	uint8_t w;

	if (BUFFER_WRITE_AVAILABLE(graphic_stream_buffer) < len)
	{
		return STATUS_STREAM_FAILED;
	}

	BUFFER_WRITE(graphic_stream_buffer, (void *)__s, len, w);

	return STATUS_OK;
}

#endif

DECL_MODULE(graphic_display)
{
	display_driver_t *display_driver = (display_driver_t *)&gd_ssd1306_128x64_i2c;
	// display_driver_t *display_driver = (display_driver_t *)&gd_ili9341_240x320_spi;
	gd_init(display_driver, graphic_display_port);
	display_width = display_driver->width;
	display_height = display_driver->height;
	display_max_lines = gd_display_max_lines();
#ifdef DECL_SERIAL_STREAM
	serial_stream_register(&graphic_stream);
#endif

	// STARTS SYSTEM MENU MODULE
	system_menu_init();
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_reset, graphic_display_start);
	ADD_EVENT_LISTENER(cnc_alarm, graphic_display_update);
	ADD_EVENT_LISTENER(cnc_dotasks, graphic_display_update);
	ADD_EVENT_LISTENER(cnc_io_dotasks, graphic_display_rotary_encoder_control_sample);
#else
#warning "Main loop extensions are not enabled. Graphic display card will not work."
#endif
}

static void io_states_str(char *buff)
{
	uint8_t controls = io_get_controls();
	uint8_t limits = io_get_limits();
	uint8_t probe = io_get_probe();
	rom_strcpy(buff, __romstr__("Sw:"));
	uint8_t i = 3;
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

		if (CHECKFLAG(limits, LINACT0_LIMIT_MASK))
		{
			buff[i++] = 'X';
		}

		if (CHECKFLAG(limits, LINACT1_LIMIT_MASK))
		{

#if ((AXIS_COUNT == 2) && defined(USE_Y_AS_Z_ALIAS))
			buff[i++] = 'Z';
#else
			buff[i++] = 'Y';
#endif
		}

		if (CHECKFLAG(limits, LINACT2_LIMIT_MASK))
		{
			buff[i++] = 'Z';
		}

		if (CHECKFLAG(limits, LINACT3_LIMIT_MASK))
		{
			buff[i++] = 'A';
		}

		if (CHECKFLAG(limits, LINACT4_LIMIT_MASK))
		{
			buff[i++] = 'B';
		}

		if (CHECKFLAG(limits, LINACT5_LIMIT_MASK))
		{
			buff[i++] = 'C';
		}
	}
}

// system menu overrides

void system_menu_render_startup(void)
{
	gd_draw_startup();
	// reset menu on actual alarm reset or soft reset
	if (cnc_get_exec_state(EXEC_INTERLOCKING_FAIL) || cnc_has_alarm())
	{
		system_menu_reset();
	}
}

void system_menu_render_idle(void)
{
	gd_clear();
	// starts from the bottom up

	// coordinates
	char buff[SYSTEM_MENU_MAX_STR_LEN];
	int8_t line = display_max_lines - 1;
	int16_t y = gd_get_line_top(line);
	memset(buff, 0, 32);

	float axis[MAX(AXIS_COUNT, 3)];
	int32_t steppos[STEPPER_COUNT];
	itp_get_rt_position(steppos);
	kinematics_apply_forward(steppos, axis);
	kinematics_apply_reverse_transform(axis);

#if (AXIS_COUNT >= 5)
	buff[0] = 'B';
	system_menu_flt_to_str(&buff[1], axis[4]);
	gd_draw_string(0, y, buff);

#if (AXIS_COUNT >= 6)
	buff[0] = 'C';
	system_menu_flt_to_str(&buff[1], axis[5]);
	gd_draw_string(display_width >> 1, y, buff);
#endif
	memset(buff, 0, 32);
	gd_draw_h_line(y);

	y = gd_get_line_top(--line);
#endif

	cnc_dotasks();

#if (AXIS_COUNT >= 3)
	buff[0] = 'Z';
	system_menu_flt_to_str(&buff[1], axis[2]);
	gd_draw_string(0, y, buff);

#if (AXIS_COUNT >= 4)
	memset(buff, 0, 32);
	buff[0] = 'A';
	system_menu_flt_to_str(&buff[1], axis[3]);
	gd_draw_string(display_width >> 1, y, buff);
#endif
	memset(buff, 0, 32);
	gd_draw_h_line(y);

	y = gd_get_line_top(--line);
#endif

	cnc_dotasks();

	{
		cnc_dotasks();
	}

#if (AXIS_COUNT >= 1)
	buff[0] = 'X';
	system_menu_flt_to_str(&buff[1], axis[0]);
	gd_draw_string(0, y, buff);
#if (AXIS_COUNT >= 2)
	buff[0] = 'Y';
	system_menu_flt_to_str(&buff[1], axis[1]);
	gd_draw_string(display_width >> 1, y, buff);
#endif
	memset(buff, 0, 32);
	gd_draw_h_line(y);
	y = gd_get_line_top(--line);
#endif

	cnc_dotasks();

	// units, feed and tool
	if (g_settings.report_inches)
	{
		rom_strcpy(buff, __romstr__("IN F"));
	}
	else
	{
		rom_strcpy(buff, __romstr__("MM F"));
	}

	// Realtime feed
	system_menu_flt_to_str(&buff[4], itp_get_rt_feed());
	gd_draw_string(0, y, buff);
	memset(buff, 0, 32);

	// Tool
	char tool[5];
	uint8_t modalgroups[14];
	uint16_t feed;
	uint16_t spindle;
	parser_get_modes(modalgroups, &feed, &spindle);
	rom_strcpy(tool, __romstr__(" T"));
	system_menu_int_to_str(&tool[2], modalgroups[11]);
	// Realtime tool speed
	rom_strcpy(buff, __romstr__("S"));
	system_menu_int_to_str(&buff[1], tool_get_speed());
	strcat(buff, tool);
	gd_draw_string(gd_str_align_end(buff), y, buff);
	memset(buff, 0, 32);
	gd_draw_h_line(y);
	y = gd_get_line_top(--line);

	cnc_dotasks();

	// system status
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

	gd_draw_string(0, y, buff);
	memset(buff, 0, 32);
	io_states_str(buff);
	gd_draw_string((display_width >> 1), y, buff);

	cnc_dotasks();

	gd_flush();
}

static int8_t item_line;

void system_menu_render_header(const char *__s)
{
	gd_clear();
	gd_draw_string(gd_str_align_center(__s), 0, __s);
	gd_draw_h_line(gd_get_line_top(1));
	item_line = 0;
}

void system_menu_render_nav_back(bool is_hover)
{
	gd_draw_button(display_width, 0, "X", -1, is_hover, false);
}

void system_menu_item_render_label(uint8_t render_flags, const char *label)
{
	int8_t line = item_line + 1;
	int16_t y = gd_get_line_top(line);
	item_line = line;
	if (label)
	{
		cnc_dotasks();
		if (render_flags & SYSTEM_MENU_MODE_EDIT)
		{
			gd_draw_string(gd_str_align_center(label), gd_get_line_top((display_max_lines >> 1) - 1), label);
			return;
		}
		gd_draw_button(0, y, label, display_width, (render_flags & SYSTEM_MENU_MODE_SELECT), true);
	}
}

void system_menu_item_render_arg(uint8_t render_flags, const char *value)
{
	if (value)
	{
		cnc_dotasks();

		int16_t y = 0;
		int16_t fh = gd_font_height();
		int16_t bt_y = fh + gd_half_padding() + 1;
		int16_t tri_off = (fh >> 1) + 1;
		if (render_flags & SYSTEM_MENU_MODE_EDIT)
		{
			int16_t start_pos = gd_str_align_center(value);
			y = gd_get_line_top((display_max_lines >> 1)) + (fh >> 1);
			bt_y += y;
			gd_draw_string(start_pos, y, value);

			if (g_system_menu.current_multiplier >= 0)
			{
				int8_t mult = (g_system_menu.current_multiplier + 1);
				char *c = (char *)&value[strlen(value) - 1];
				while (mult--)
				{
					if (*c == '.')
					{
						*c = 0;
						c--;
					}
					*c = 0;
					if (c == value)
					{
						mult--;
						break;
					}
					c--;
					
				}

				mult = strlen(value) - mult;

				uint8_t cw = display_char_width;
				if (!cw)
				{
					cw = gd_str_width("0");
					display_char_width = cw;
				}
				int16_t base_pos = start_pos;
				base_pos += (mult > 0) ? gd_str_width(value) + 1 : (mult - 1) * cw;

				if (render_flags & SYSTEM_MENU_MODE_MODIFY)
				{
					gd_draw_triangle(base_pos, y, base_pos + cw, y, base_pos + 2, y - tri_off, false);
					gd_draw_triangle(base_pos, bt_y, base_pos + cw, bt_y, base_pos + 2, bt_y + tri_off - 2, false);
				}
				else
				{
					gd_draw_rectangle_fill(base_pos, bt_y, cw, 2, false);
				}
			}

			return;
		}
		y = gd_get_line_top(item_line);
		bt_y += y;
		int16_t base_x = gd_str_align_end(value);

		if (CHECKFLAG(render_flags, (SYSTEM_MENU_MODE_SELECT | SYSTEM_MENU_MODE_SIMPLE_EDIT)) == (SYSTEM_MENU_MODE_SELECT | SYSTEM_MENU_MODE_SIMPLE_EDIT))
		{
			gd_draw_triangle(base_x - 1, bt_y - tri_off, base_x - tri_off, bt_y, base_x - tri_off, y + gd_half_padding(), true);
		}

		gd_draw_string_inv(base_x, y, value, (render_flags & SYSTEM_MENU_MODE_SELECT));
	}
}

void system_menu_render_footer(void)
{
	gd_flush();
}

bool system_menu_render_menu_item_filter(uint8_t item_index)
{
	static uint8_t menu_top = 0;
	uint8_t current_index = MAX(0, g_system_menu.current_index);
	uint8_t max_lines = display_max_lines - 1;

	uint8_t top = menu_top;
	if ((top + max_lines) <= current_index)
	{
		// advance menu top
		menu_top = top = (current_index - max_lines + 1);
	}

	if (top > current_index)
	{
		// rewind menu top
		menu_top = top = current_index;
	}

	return ((top <= item_index) && (item_index < (top + max_lines)));
}

void system_menu_render_modal_popup(const char *__s)
{
	int16_t w = 0;
	uint8_t lines = graphic_display_str_lines(__s, &w);
	w = (lines != 1) ? (w * gd_str_width("Z")) : (gd_str_width(__s));
	w += 6;

	gd_draw_rectangle_fill(5, 5, (display_width - 10), (display_height - 5), true);
	gd_draw_rectangle(5, 5, (display_width - 10), (display_height - 5));

	uint8_t line = ((display_max_lines - lines) >> 1);
	do
	{
		char buffer[SYSTEM_MENU_MAX_STR_LEN];
		uint8_t len = graphic_display_str_line_len(__s);
		memcpy(buffer, __s, len);
		buffer[len] = 0;
		__s += len;
		gd_draw_string(gd_str_align_center(buffer), gd_get_line_top(line++), buffer);
	} while (--lines);

	gd_flush();
}

static uint8_t graphic_display_str_lines(const char *__s, int16_t *max_len)
{
	uint8_t lines = 1;
	uint8_t chars_per_line = display_width / gd_str_width("Z");

	uint8_t chars = 0;
	while (*__s)
	{
		chars++;
		if (*__s == '\n' || (chars >= chars_per_line))
		{
			*max_len = MAX(*max_len, chars);
			chars = 0;
			lines++;
		}
		__s++;
	}

	return lines;
}

static uint8_t graphic_display_str_line_len(const char *__s)
{
	uint8_t chars = 0;
	uint8_t chars_per_line = display_width / gd_str_width("Z");
	while (*__s)
	{
		chars++;
		if (*__s == '\n' || (chars >= chars_per_line))
		{
			return chars;
		}
		__s++;
	}

	return chars;
}

// define this way so it can be translated
// this defaults to english
#ifndef STR_USER_NEEDS_SYSTEM_RESET_1
#define STR_USER_NEEDS_SYSTEM_RESET_1 "Press btn for 5s"
#endif

#ifndef STR_USER_NEEDS_SYSTEM_RESET_2
#define STR_USER_NEEDS_SYSTEM_RESET_2 "to reset"
#endif

void system_menu_render_alarm(void)
{
	gd_clear();
	// coordinates
	char buff[SYSTEM_MENU_MAX_STR_LEN];
	rom_strcpy(buff, __romstr__("ALARM "));
	uint8_t alarm = cnc_get_alarm();
	system_menu_int_to_str(&buff[6], alarm);
	gd_draw_rectangle_fill(0, 0, display_width, gd_line_height(), false);
	gd_draw_string_inv(gd_str_align_center(buff), 0, buff, true);

	memset(buff, 0, SYSTEM_MENU_MAX_STR_LEN);

	switch (alarm)
	{
	case 1:
		rom_strcpy(buff, __romstr__(STR_ALARM_1));
		break;
	case 2:
		rom_strcpy(buff, __romstr__(STR_ALARM_2));
		break;
	case 3:
		rom_strcpy(buff, __romstr__(STR_ALARM_3));
		break;
	case 4:
		rom_strcpy(buff, __romstr__(STR_ALARM_4));
		break;
	case 5:
		rom_strcpy(buff, __romstr__(STR_ALARM_5));
		break;
	case 6:
		rom_strcpy(buff, __romstr__(STR_ALARM_6));
		break;
	case 7:
		rom_strcpy(buff, __romstr__(STR_ALARM_7));
		break;
	case 8:
		rom_strcpy(buff, __romstr__(STR_ALARM_8));
		break;
	case 9:
		rom_strcpy(buff, __romstr__(STR_ALARM_9));
		break;
	case 10:
		rom_strcpy(buff, __romstr__(STR_ALARM_10));
		break;
	case 11:
		rom_strcpy(buff, __romstr__(STR_ALARM_11));
		break;
	case 12:
		rom_strcpy(buff, __romstr__(STR_ALARM_12));
		break;
	case 13:
		rom_strcpy(buff, __romstr__(STR_ALARM_13));
		break;
	default:
		rom_strcpy(buff, __romstr__(STR_ALARM_0));
		break;
	}

	gd_draw_string(gd_str_align_center(buff), gd_get_line_top(1), buff);
	memset(buff, 0, SYSTEM_MENU_MAX_STR_LEN);
	io_states_str(buff);
	gd_draw_string(gd_str_align_center(buff), gd_get_line_top(2), buff);
	rom_strcpy(buff, __romstr__(STR_USER_NEEDS_SYSTEM_RESET_1));
	gd_draw_string(gd_str_align_center(buff), gd_get_line_top(3), buff);
	rom_strcpy(buff, __romstr__(STR_USER_NEEDS_SYSTEM_RESET_2));
	gd_draw_string(gd_str_align_center(buff), gd_get_line_top(4), buff);
	gd_flush();
}
