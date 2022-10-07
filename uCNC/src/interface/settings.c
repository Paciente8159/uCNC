/*
	Name: settings.c
	Description: µCNC runtime settings. These functions store settings and other parameters
		in non-volatile memory.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 26/09/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"
#include "defaults.h"

// if settings struct is changed this version should change too
#define SETTINGS_VERSION "V06"

settings_t g_settings;

#ifndef CRC_WITHOUT_LOOKUP_TABLE

const uint8_t __rom__ crc7_table[256] =
	{
		0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f,
		0x48, 0x41, 0x5a, 0x53, 0x6c, 0x65, 0x7e, 0x77,
		0x19, 0x10, 0x0b, 0x02, 0x3d, 0x34, 0x2f, 0x26,
		0x51, 0x58, 0x43, 0x4a, 0x75, 0x7c, 0x67, 0x6e,
		0x32, 0x3b, 0x20, 0x29, 0x16, 0x1f, 0x04, 0x0d,
		0x7a, 0x73, 0x68, 0x61, 0x5e, 0x57, 0x4c, 0x45,
		0x2b, 0x22, 0x39, 0x30, 0x0f, 0x06, 0x1d, 0x14,
		0x63, 0x6a, 0x71, 0x78, 0x47, 0x4e, 0x55, 0x5c,
		0x64, 0x6d, 0x76, 0x7f, 0x40, 0x49, 0x52, 0x5b,
		0x2c, 0x25, 0x3e, 0x37, 0x08, 0x01, 0x1a, 0x13,
		0x7d, 0x74, 0x6f, 0x66, 0x59, 0x50, 0x4b, 0x42,
		0x35, 0x3c, 0x27, 0x2e, 0x11, 0x18, 0x03, 0x0a,
		0x56, 0x5f, 0x44, 0x4d, 0x72, 0x7b, 0x60, 0x69,
		0x1e, 0x17, 0x0c, 0x05, 0x3a, 0x33, 0x28, 0x21,
		0x4f, 0x46, 0x5d, 0x54, 0x6b, 0x62, 0x79, 0x70,
		0x07, 0x0e, 0x15, 0x1c, 0x23, 0x2a, 0x31, 0x38,
		0x41, 0x48, 0x53, 0x5a, 0x65, 0x6c, 0x77, 0x7e,
		0x09, 0x00, 0x1b, 0x12, 0x2d, 0x24, 0x3f, 0x36,
		0x58, 0x51, 0x4a, 0x43, 0x7c, 0x75, 0x6e, 0x67,
		0x10, 0x19, 0x02, 0x0b, 0x34, 0x3d, 0x26, 0x2f,
		0x73, 0x7a, 0x61, 0x68, 0x57, 0x5e, 0x45, 0x4c,
		0x3b, 0x32, 0x29, 0x20, 0x1f, 0x16, 0x0d, 0x04,
		0x6a, 0x63, 0x78, 0x71, 0x4e, 0x47, 0x5c, 0x55,
		0x22, 0x2b, 0x30, 0x39, 0x06, 0x0f, 0x14, 0x1d,
		0x25, 0x2c, 0x37, 0x3e, 0x01, 0x08, 0x13, 0x1a,
		0x6d, 0x64, 0x7f, 0x76, 0x49, 0x40, 0x5b, 0x52,
		0x3c, 0x35, 0x2e, 0x27, 0x18, 0x11, 0x0a, 0x03,
		0x74, 0x7d, 0x66, 0x6f, 0x50, 0x59, 0x42, 0x4b,
		0x17, 0x1e, 0x05, 0x0c, 0x33, 0x3a, 0x21, 0x28,
		0x5f, 0x56, 0x4d, 0x44, 0x7b, 0x72, 0x69, 0x60,
		0x0e, 0x07, 0x1c, 0x15, 0x2a, 0x23, 0x38, 0x31,
		0x46, 0x4f, 0x54, 0x5d, 0x62, 0x6b, 0x70, 0x79};

#define crc7(x, y) rom_read_byte(&crc7_table[x ^ y])
#else
static uint8_t crc7(uint8_t c, uint8_t crc)
{
	uint8_t i = 8;
	crc ^= c;
	for (;;)
	{
		if (crc & 0x80)
		{
			crc ^= 0x89;
		}
		if (!--i)
		{
			return crc;
		}
		crc <<= 1;
	}
}
#endif

const settings_t __rom__ default_settings =
	{
		.version = SETTINGS_VERSION,
		.max_step_rate = F_STEP_MAX,
		.step_invert_mask = DEFAULT_STEP_INV_MASK,
		.dir_invert_mask = DEFAULT_DIR_INV_MASK,
		.step_enable_invert = DEFAULT_STEP_ENA_INV,
		.limits_invert_mask = DEFAULT_LIMIT_INV_MASK,
		.probe_invert_mask = DEFAULT_PROBE_INV_MASK,
		.status_report_mask = DEFAULT_STATUS_MASK,
		.control_invert_mask = DEFAULT_CONTROL_INV_MASK,
		.g64_angle_factor = DEFAULT_G64_FACTOR,
		.arc_tolerance = DEFAULT_ARC_TOLERANCE,
		.report_inches = DEFAULT_REPORT_INCHES,
		.soft_limits_enabled = DEFAULT_SOFT_LIMITS_ENABLED,
		.hard_limits_enabled = DEFAULT_HARD_LIMITS_ENABLED,
		.homing_enabled = DEFAULT_HOMING_ENABLED,
		.debounce_ms = DEFAULT_DEBOUNCE_MS,
		.homing_dir_invert_mask = DEFAULT_HOMING_DIR_INV_MASK,
		.homing_fast_feed_rate = DEFAULT_HOMING_FAST,
		.homing_slow_feed_rate = DEFAULT_HOMING_SLOW,
		.homing_offset = DEFAULT_HOMING_OFFSET,
		.spindle_max_rpm = DEFAULT_SPINDLE_MAX_RPM,
		.spindle_min_rpm = DEFAULT_SPINDLE_MIN_RPM,
		.laser_mode = 0,
#ifdef ENABLE_LASER_PPI
		.laser_ppi_uswidth = 1500,
		.laser_ppi_mixmode_ppi = 0.25,
		.laser_ppi_mixmode_uswidth = 0.75,
#endif
		.step_per_mm = DEFAULT_ARRAY(AXIS_COUNT, DEFAULT_STEP_PER_MM),
		.max_feed_rate = DEFAULT_ARRAY(AXIS_COUNT, DEFAULT_MAX_FEED),
		.acceleration = DEFAULT_ARRAY(AXIS_COUNT, DEFAULT_ACCEL),
		.max_distance = DEFAULT_ARRAY(AXIS_COUNT, DEFAULT_MAX_DIST),
#if TOOL_COUNT > 0
		.default_tool = DEFAULT_STARTUP_TOOL,
		.tool_length_offset = DEFAULT_ARRAY(TOOL_COUNT, 0),
#endif
#if (KINEMATIC == KINEMATIC_DELTA)
		.delta_arm_length = DEFAULT_DELTA_ARM_LENGTH,
		.delta_armbase_radius = DEFAULT_DELTA_BASE_RADIUS,
// float delta_efector_height;
#endif

#ifdef ENABLE_BACKLASH_COMPENSATION
		.backlash_steps = DEFAULT_ARRAY(AXIS_TO_STEPPERS, 0),
#endif
#ifdef ENABLE_SKEW_COMPENSATION
		.skew_xy_factor = 0,
#ifndef SKEW_COMPENSATION_XY_ONLY
		.skew_xz_factor = 0,
		.skew_yz_factor = 0,
#endif
#endif
#if ENCODERS > 0
		.encoders_pulse_invert_mask = 0,
		.encoders_dir_invert_mask = 0,
#endif
#if PID_CONTROLLERS > 0
		.pid_gain[0] = DEFAULT_ARRAY(3, 0),
#endif
#if PID_CONTROLLERS > 1
		.pid_gain[1] = DEFAULT_ARRAY(3, 0),
#endif
#if PID_CONTROLLERS > 2
		.pid_gain[2] = DEFAULT_ARRAY(3, 0),
#endif
#if PID_CONTROLLERS > 3
		.pid_gain[3] = DEFAULT_ARRAY(3, 0),
#endif
#if PID_CONTROLLERS > 4
		.pid_gain[4] = DEFAULT_ARRAY(3, 0),
#endif
#if PID_CONTROLLERS > 5
		.pid_gain[5] = DEFAULT_ARRAY(3, 0),
#endif
#if PID_CONTROLLERS > 6
		.pid_gain[6] = DEFAULT_ARRAY(3, 0),
#endif
#if PID_CONTROLLERS > 7
		.pid_gain[7] = DEFAULT_ARRAY(3, 0),
#endif
};

#ifdef ENABLE_SETTINGS_MODULES
// event_settings_change_handler
WEAK_EVENT_HANDLER(settings_change)
{
	DEFAULT_EVENT_HANDLER(settings_change);
}

// event_settings_load_handler
WEAK_EVENT_HANDLER(settings_load)
{
	DEFAULT_EVENT_HANDLER(settings_load);
}

// event_settings_save_handler
WEAK_EVENT_HANDLER(settings_save)
{
	DEFAULT_EVENT_HANDLER(settings_save);
}

// event_settings_erase_handler
WEAK_EVENT_HANDLER(settings_erase)
{
	DEFAULT_EVENT_HANDLER(settings_erase);
}
#endif

void settings_init(void)
{
	const char version[3] = SETTINGS_VERSION;
	uint8_t error = settings_load(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));

	if (!error)
	{
		for (uint8_t i = 0; i < 3; i++)
		{
			if (g_settings.version[i] != version[i])
			{
				error = 1; // just set an error
				break;
			}
		}
	}

	if (error)
	{
		settings_reset(true);
		protocol_send_error(STATUS_SETTING_READ_FAIL);
		protocol_send_cnc_settings();
	}
}

uint8_t settings_load(uint16_t address, uint8_t *__ptr, uint8_t size)
{
#ifdef ENABLE_SETTINGS_MODULES
	settings_args_t args = {.address = address, .data = __ptr, .size = size};
	if (EVENT_INVOKE(settings_load, &args) == STATUS_EXTERNAL_SETTINGS_OK)
	{
		return 0;
	}
	// if unable to get settings from external memory tries to get from internal EEPROM
#endif
#ifndef RAM_ONLY_SETTINGS
	uint8_t crc = 0;

	while (size)
	{
		size--;
		uint8_t value = mcu_eeprom_getc(address++);
		crc = crc7(value, crc);
		*(__ptr++) = value;
	}

	return (crc) ? (crc ^ mcu_eeprom_getc(address)) : 255;
#else
	return 255; // returns error
#endif
}

void settings_reset(bool erase_startup_blocks)
{
	rom_memcpy(&g_settings, &default_settings, sizeof(settings_t));

#if !defined(ENABLE_EXTRA_SYSTEM_CMDS) && !defined(RAM_ONLY_SETTINGS)
	settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
	if (erase_startup_blocks)
	{
		mcu_eeprom_putc(STARTUP_BLOCK0_ADDRESS_OFFSET, 0);
		mcu_eeprom_putc(STARTUP_BLOCK0_ADDRESS_OFFSET + 1, 0);
		mcu_eeprom_putc(STARTUP_BLOCK1_ADDRESS_OFFSET, 0);
		mcu_eeprom_putc(STARTUP_BLOCK1_ADDRESS_OFFSET + 1, 0);
	}
#endif
}

void settings_save(uint16_t address, uint8_t *__ptr, uint8_t size)
{
#ifdef ENABLE_SETTINGS_MODULES
	settings_args_t args = {.address = address, .data = __ptr, .size = size};
	if (EVENT_INVOKE(settings_save, &args) == STATUS_EXTERNAL_SETTINGS_OK)
	{
		return;
	}
#endif

#ifndef RAM_ONLY_SETTINGS
	uint8_t crc = 0;

	while (size)
	{
		if (cnc_get_exec_state(EXEC_RUN))
		{
			cnc_dotasks(); // updates buffer before cycling
		}

		size--;
		crc = crc7(*__ptr, crc);
		mcu_eeprom_putc(address++, *(__ptr++));
	}

	mcu_eeprom_putc(address, crc);
	mcu_eeprom_flush();
#endif
}

uint8_t settings_change(uint8_t setting, float value)
{
	uint8_t result = 0;
	uint16_t value16 = (uint16_t)CLAMP(0, value, INT16_MAX);
	uint8_t value8 = (uint8_t)MIN(value16, UINT8_MAX);

	bool value1 = (value8 != 0);

	if (value < 0)
	{
		return STATUS_NEGATIVE_VALUE;
	}

	switch (setting)
	{
	case 0:
		value = 1000000.0f / value;
		if (value > F_STEP_MAX)
		{
			return STATUS_MAX_STEP_RATE_EXCEEDED;
		}
		g_settings.max_step_rate = value;
		break;
#ifdef EMULATE_GRBL_STARTUP
	// just adds this for compatibility
	// this setting is not used
	case 1:
		break;
#endif
	case 2:
		g_settings.step_invert_mask = value8;
		break;
	case 3:
		g_settings.dir_invert_mask = value8;
		break;
	case 4:
		g_settings.step_enable_invert = value8;
		break;
	case 5:
		g_settings.limits_invert_mask = value8;
		break;
	case 6:
		g_settings.probe_invert_mask = value1;
		break;
	case 7:
		g_settings.control_invert_mask = (value8 & CONTROLS_MASK);
		break;
#if ENCODERS > 0
	case 8:
		g_settings.encoders_pulse_invert_mask = value8;
		break;
	case 9:
		g_settings.encoders_dir_invert_mask = value8;
		break;
#endif
	case 10:
		g_settings.status_report_mask = value8;
		break;
	case 11:
		g_settings.g64_angle_factor = value;
		break;
	case 12:
		g_settings.arc_tolerance = value;
		break;
	case 13:
		g_settings.report_inches = value;
		break;
	case 20:
		if (!g_settings.homing_enabled && value1)
		{
			return STATUS_SOFT_LIMIT_ERROR;
		}
		g_settings.soft_limits_enabled = value1;
		break;
	case 21:
		g_settings.hard_limits_enabled = value1;
		break;
	case 22:
		g_settings.homing_enabled = value1;
		break;
	case 23:
		g_settings.homing_dir_invert_mask = value8;
		break;
	case 24:
		g_settings.homing_slow_feed_rate = value;
		break;
	case 25:
		g_settings.homing_fast_feed_rate = value;
		break;
	case 26:
		g_settings.debounce_ms = value16;
		break;
	case 27:
		g_settings.homing_offset = value;
		break;
	case 30:
		g_settings.spindle_max_rpm = value16;
		break;
	case 31:
		g_settings.spindle_min_rpm = value16;
		break;
	case 32:
		g_settings.laser_mode = value8;
		break;
#ifdef ENABLE_LASER_PPI
	case 33:
		g_settings.step_per_mm[STEPPER_COUNT - 1] = value;
		break;
	case 34:
		g_settings.laser_ppi_uswidth = value16;
		break;
	case 35:
		g_settings.laser_ppi_mixmode_ppi = value;
		break;
	case 36:
		g_settings.laser_ppi_mixmode_uswidth = value;
		break;
#endif
#ifdef ENABLE_SKEW_COMPENSATION
	case 37:
		g_settings.skew_xy_factor = value;
		break;
#ifndef SKEW_COMPENSATION_XY_ONLY
	case 38:
		g_settings.skew_xz_factor = value;
		break;
	case 39:
		g_settings.skew_yz_factor = value;
		break;
#endif
#endif
#if TOOL_COUNT > 0
	case 80:
		g_settings.default_tool = CLAMP(0, value8, (uint8_t)TOOL_COUNT);
		break;
#endif
#if (KINEMATIC == KINEMATIC_DELTA)
	case 106:
		g_settings.delta_arm_length = value;
		break;
	case 107:
		g_settings.delta_armbase_radius = value;
		break;
		// case 108:
		//     g_settings.delta_efector_height = value;
		//     break;
#endif
	default:
		if (setting >= 100 && setting < (100 + AXIS_COUNT))
		{
			setting -= 100;
			g_settings.step_per_mm[setting] = value;
		}
		else if (setting >= 110 && setting < (110 + AXIS_COUNT))
		{
			setting -= 110;
			g_settings.max_feed_rate[setting] = value;
		}
		else if (setting >= 120 && setting < (120 + AXIS_COUNT))
		{
			setting -= 120;
			g_settings.acceleration[setting] = value;
		}
		else if (setting >= 130 && setting < (130 + AXIS_COUNT))
		{
			setting -= 130;
			g_settings.max_distance[setting] = value;
		}
#ifdef ENABLE_BACKLASH_COMPENSATION
		else if (setting >= 140 && setting < (140 + AXIS_TO_STEPPERS))
		{
			setting -= 140;
			g_settings.backlash_steps[setting] = value16;
		}
#endif
#if PID_CONTROLLERS > 0
		// kp ki and kd 0 -> 41, 42, 43
		// kp ki and kd 1 -> 45, 46, 47, etc...
		else if (setting >= 40 && setting < (40 + (4 * PID_CONTROLLERS)))
		{
			uint8_t k = (setting & 0x03);
			uint8_t pid = (setting >> 2) - 10;
			// 3 is invalid index
			if (k == 0x03)
			{
				return STATUS_INVALID_STATEMENT;
			}
			g_settings.pid_gain[pid][k] = value;
		}
#endif
#if TOOL_COUNT > 0
		else if (setting > 80 && setting <= (80 + TOOL_COUNT))
		{
			setting -= 81;
			g_settings.tool_length_offset[setting] = value;
		}
#endif
		else
		{
			return STATUS_INVALID_STATEMENT;
		}
		break;
	}

#if !defined(ENABLE_EXTRA_SYSTEM_CMDS) && !defined(RAM_ONLY_SETTINGS)
	settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
#endif

#ifdef ENABLE_SETTINGS_MODULES
	EVENT_INVOKE(settings_change, NULL);
#endif

	return result;
}

void settings_erase(uint16_t address, uint8_t size)
{

#ifdef ENABLE_SETTINGS_MODULES
	settings_args_t args = {.address = address, .data = NULL, .size = size};
	if (EVENT_INVOKE(settings_erase, &args) == STATUS_EXTERNAL_SETTINGS_OK)
	{
		return;
	}
#endif
#ifndef RAM_ONLY_SETTINGS
	while (size)
	{
		if (cnc_get_exec_state(EXEC_RUN))
		{
			cnc_dotasks(); // updates buffer before cycling
		}
		mcu_eeprom_putc(address++, EOL);
		size--;
	}

	// erase crc byte that is next to data
	mcu_eeprom_putc(address, EOL);
	mcu_eeprom_flush();
#endif
}

bool settings_check_startup_gcode(uint16_t address)
{
#ifndef RAM_ONLY_SETTINGS
	uint8_t size = (RX_BUFFER_SIZE - 1); // defined in serial.h
	uint8_t crc = 0;
	unsigned char c;
	uint16_t cmd_address = address;

	// pre-checks command valid crc
	do
	{
		c = mcu_eeprom_getc(cmd_address++);
		crc = crc7(c, crc);
		if (!c)
		{
			break;
		}
		size--;
	} while (size);

	if (crc ^ mcu_eeprom_getc(cmd_address))
	{
		serial_putc('>');
		serial_putc(':');
		protocol_send_error(STATUS_SETTING_READ_FAIL);
		settings_erase(address, 1);
		return false;
	}
#endif
	return true;
}

void settings_save_startup_gcode(uint16_t address)
{
#ifndef RAM_ONLY_SETTINGS
	uint8_t size = (RX_BUFFER_SIZE - 1);
	uint8_t crc = 0;
	unsigned char c;
	do
	{
		c = serial_getc();
		crc = crc7(c, crc);
		mcu_eeprom_putc(address++, (uint8_t)c);
		size--;
	} while (size && c);

	mcu_eeprom_putc(address, crc);
#endif
}
