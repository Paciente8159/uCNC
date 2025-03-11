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

#ifndef DISABLE_SAFE_SETTINGS
uint8_t g_settings_error;
#endif

// if settings struct is changed this version should change too
#define SETTINGS_VERSION \
	{                      \
			'V', '0', '7'}

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
				.max_step_rate = (1000000.0f / F_STEP_MAX),
#ifdef ENABLE_STEPPERS_DISABLE_TIMEOUT
				.step_disable_timeout = 0,
#endif
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
#if S_CURVE_ACCELERATION_LEVEL == -1
				.s_curve_profile = 0,
#endif
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
				.laser_ppi = DEFAULT_LASER_PPI,
				.laser_ppi_uswidth = DEFAULT_LASER_PPI_USWIDTH,
				.laser_ppi_mixmode_ppi = 0.25,
				.laser_ppi_mixmode_uswidth = 0.75,
#endif
				.step_per_mm = DEFAULT_STEP_PER_MM_PER_AXIS,
				.max_feed_rate = DEFAULT_MAX_FEED_PER_AXIS,
				.acceleration = DEFAULT_ACCEL_PER_AXIS,
				.max_distance = DEFAULT_MAX_DIST_PER_AXIS,
#if TOOL_COUNT > 0
#if TOOL_COUNT > 1
				.default_tool = DEFAULT_STARTUP_TOOL,
#endif
				.tool_length_offset = DEFAULT_ARRAY(TOOL_COUNT, 0),
#endif
				KINEMATICS_VARS_DEFAULTS_INIT /*KINEMATICS DEFAULTS*/

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
				.encoders_resolution = DEFAULT_ARRAY(ENCODERS, 1),
#endif
};

const setting_id_t __rom__ g_settings_id_table[] = {
		{.id = 0, .memptr = &g_settings.max_step_rate, .type = SETTING_TYPE_FLOAT},
#ifdef ENABLE_STEPPERS_DISABLE_TIMEOUT
		{.id = 1, .memptr = &g_settings.step_disable_timeout, .type = SETTING_TYPE_UINT16},
#endif
		{.id = 2, .memptr = &g_settings.step_invert_mask, .type = SETTING_TYPE_UINT8},
		{.id = 3, .memptr = &g_settings.dir_invert_mask, .type = SETTING_TYPE_UINT8},
		{.id = 4, .memptr = &g_settings.step_enable_invert, .type = SETTING_TYPE_BOOL},
		{.id = 5, .memptr = &g_settings.limits_invert_mask, .type = SETTING_TYPE_UINT8},
		{.id = 6, .memptr = &g_settings.probe_invert_mask, .type = SETTING_TYPE_BOOL},
		{.id = 7, .memptr = &g_settings.control_invert_mask, .type = SETTING_TYPE_UINT8},
#if ENCODERS > 0
		{.id = 8, .memptr = &g_settings.encoders_pulse_invert_mask, .type = SETTING_TYPE_UINT8},
		{.id = 9, .memptr = &g_settings.encoders_dir_invert_mask, .type = SETTING_TYPE_UINT8},
#endif
		{.id = 10, .memptr = &g_settings.status_report_mask, .type = SETTING_TYPE_UINT8},
		{.id = 11, .memptr = &g_settings.g64_angle_factor, .type = SETTING_TYPE_FLOAT},
		{.id = 12, .memptr = &g_settings.arc_tolerance, .type = SETTING_TYPE_FLOAT},
		{.id = 13, .memptr = &g_settings.report_inches, .type = SETTING_TYPE_BOOL},
#if S_CURVE_ACCELERATION_LEVEL == -1
		{.id = 14, .memptr = &g_settings.s_curve_profile, .type = SETTING_TYPE_UINT8},
#endif
		{.id = 20, .memptr = &g_settings.soft_limits_enabled, .type = SETTING_TYPE_BOOL},
		{.id = 21, .memptr = &g_settings.hard_limits_enabled, .type = SETTING_TYPE_BOOL},
		{.id = 22, .memptr = &g_settings.homing_enabled, .type = SETTING_TYPE_BOOL},
		{.id = 23, .memptr = &g_settings.homing_dir_invert_mask, .type = SETTING_TYPE_UINT8},
		{.id = 24, .memptr = &g_settings.homing_slow_feed_rate, .type = SETTING_TYPE_FLOAT},
		{.id = 25, .memptr = &g_settings.homing_fast_feed_rate, .type = SETTING_TYPE_FLOAT},
		{.id = 26, .memptr = &g_settings.debounce_ms, .type = SETTING_TYPE_UINT16},
		{.id = 27, .memptr = &g_settings.homing_offset, .type = SETTING_TYPE_FLOAT},
		{.id = 30, .memptr = &g_settings.spindle_max_rpm, .type = SETTING_TYPE_UINT16},
		{.id = 31, .memptr = &g_settings.spindle_min_rpm, .type = SETTING_TYPE_UINT16},
		{.id = 32, .memptr = &g_settings.laser_mode, .type = SETTING_TYPE_UINT8},
#ifdef ENABLE_LASER_PPI
		{.id = 33, .memptr = &g_settings.step_per_mm[0], .type = SETTING_TYPE_FLOAT},
		{.id = 34, .memptr = &g_settings.laser_ppi_uswidth, .type = SETTING_TYPE_UINT16},
		{.id = 35, .memptr = &g_settings.laser_ppi_mixmode_ppi, .type = SETTING_TYPE_FLOAT},
		{.id = 36, .memptr = &g_settings.laser_ppi_mixmode_uswidth, .type = SETTING_TYPE_FLOAT},
#endif
#ifdef ENABLE_SKEW_COMPENSATION
		{.id = 37, .memptr = &g_settings.skew_xy_factor, .type = SETTING_TYPE_FLOAT},
#ifndef SKEW_COMPENSATION_XY_ONLY
		{.id = 38, .memptr = &g_settings.skew_xz_factor, .type = SETTING_TYPE_FLOAT},
		{.id = 39, .memptr = &g_settings.skew_yz_factor, .type = SETTING_TYPE_FLOAT},
#endif
#endif
#if TOOL_COUNT > 1
		{.id = 80, .memptr = &g_settings.default_tool, .type = SETTING_TYPE_UINT8},
#endif
#if TOOL_COUNT > 0
		{.id = 81, .memptr = &g_settings.tool_length_offset, .type = SETTING_TYPE_FLOAT | SETTING_ARRAY | SETTING_ARRCNT(TOOL_COUNT)},
#endif
		KINEMATICS_VARS_SETTINGS_INIT /*KINEMATICS INITIALIZATION*/

		{.id = 100, .memptr = &g_settings.step_per_mm, .type = SETTING_TYPE_FLOAT | SETTING_ARRAY | SETTING_ARRCNT(STEPPER_COUNT)},
		{.id = 110, .memptr = &g_settings.max_feed_rate, .type = SETTING_TYPE_FLOAT | SETTING_ARRAY | SETTING_ARRCNT(STEPPER_COUNT)},
		{.id = 120, .memptr = &g_settings.acceleration, .type = SETTING_TYPE_FLOAT | SETTING_ARRAY | SETTING_ARRCNT(STEPPER_COUNT)},
		{.id = 130, .memptr = &g_settings.max_distance, .type = SETTING_TYPE_FLOAT | SETTING_ARRAY | SETTING_ARRCNT(AXIS_COUNT)},
#ifdef ENABLE_BACKLASH_COMPENSATION
		{.id = 140, .memptr = &g_settings.backlash_steps, .type = SETTING_TYPE_UINT16 | SETTING_ARRAY | SETTING_ARRCNT(AXIS_TO_STEPPERS)},
#endif
#if ENCODERS
		{.id = 150, .memptr = &g_settings.encoders_resolution, .type = SETTING_TYPE_FLOAT | SETTING_ARRAY | SETTING_ARRCNT(ENCODERS)},
#endif
};

#ifdef ENABLE_SETTINGS_MODULES
// event_settings_extended_load_handler
WEAK_EVENT_HANDLER(settings_extended_load)
{
	if (!*((bool *)args))
	{
		return EVENT_CONTINUE;
	}

	DEFAULT_EVENT_HANDLER(settings_extended_load);
}

// event_settings_extended_save_handler
WEAK_EVENT_HANDLER(settings_extended_save)
{
	if (!*((bool *)args))
	{
		return EVENT_CONTINUE;
	}
	DEFAULT_EVENT_HANDLER(settings_extended_save);
}

// event_settings_extended_erase_handler
WEAK_EVENT_HANDLER(settings_extended_erase)
{
	if (!*((bool *)args))
	{
		return EVENT_CONTINUE;
	}
	DEFAULT_EVENT_HANDLER(settings_extended_erase);
}

// event_settings_extended_change_handler
WEAK_EVENT_HANDLER(settings_extended_change)
{
	DEFAULT_EVENT_HANDLER(settings_extended_change);
}
#endif

static uint8_t settings_size_crc(uint16_t size, uint8_t crc)
{
	crc = crc7(((uint8_t *)&size)[0], crc);
	return crc7(((uint8_t *)&size)[1], crc);
}

void __attribute__((weak)) nvm_start_read(uint16_t address) {}
void __attribute__((weak)) nvm_start_write(uint16_t address) {}
uint8_t __attribute__((weak)) nvm_getc(uint16_t address) { return mcu_eeprom_getc(address); }
void __attribute__((weak)) nvm_putc(uint16_t address, uint8_t c) { mcu_eeprom_putc(address, c); }
void __attribute__((weak)) nvm_end_read(void) {}
void __attribute__((weak)) nvm_end_write(void) { mcu_eeprom_flush(); }

void settings_init(void)
{

#ifdef FORCE_GLOBALS_TO_0
#ifndef DISABLE_SAFE_SETTINGS
	g_settings_error = SETTINGS_OK;
#endif
#endif
	uint8_t error = settings_load(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));

	if (error)
	{
#ifdef DISABLE_SAFE_SETTINGS
		settings_reset(true);
#endif
		proto_error(STATUS_SETTING_READ_FAIL);
		proto_cnc_settings();
	}
}

uint8_t settings_load(uint16_t address, uint8_t *__ptr, uint16_t size)
{
#ifdef RAM_ONLY_SETTINGS
	DBGMSG("Default settings @ %u", address);
	if (address == SETTINGS_ADDRESS_OFFSET)
	{
		rom_memcpy(&g_settings, &default_settings, sizeof(settings_t));
	}
	else
	{
		size = MAX(size, 1);
		memset(__ptr, 0, size);
	}
	return 0; // loads defaults
#endif

	DBGMSG("EEPROM load @ %u", address);

	// settiing address invalid
	if (address == UINT16_MAX)
	{
		return STATUS_SETTING_DISABLED;
	}

	uint8_t crc = 0;
	bool is_machine_settings = (address == SETTINGS_ADDRESS_OFFSET);

#ifdef ENABLE_SETTINGS_MODULES
	bool extended_load __attribute__((__cleanup__(EVENT_HANDLER_NAME(settings_extended_load)))) = is_machine_settings;
#endif

	nvm_start_read(address);
	for (uint16_t i = 0; i < size;)
	{
		uint8_t value = nvm_getc(address++);
		crc = crc7(value, crc);
		if (__ptr)
		{
			*(__ptr++) = value;
		}

		i++;
		if (!__ptr && (value == EOL))
		{
			size = i;
			break;
		}
	}

	crc = settings_size_crc(size, crc);
	crc ^= nvm_getc(address);

	nvm_end_read();

#ifndef DISABLE_SAFE_SETTINGS
	if (crc)
	{
		g_settings_error |= SETTINGS_READ_ERROR;
	}
#endif

	if (is_machine_settings)
	{
		const uint8_t version[3] = SETTINGS_VERSION;
		if ((g_settings.version[0] != version[0]) || (g_settings.version[1] != version[1]) || (g_settings.version[2] != version[2]))
		{
			return 1; // return error
		}
	}

	return crc;
}

void settings_reset(bool erase_startup_blocks)
{
	settings_erase(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, sizeof(settings_t));

	if (erase_startup_blocks)
	{
		for (uint8_t i = 0; i < STARTUP_BLOCKS_COUNT; i++)
		{
			settings_erase(STARTUP_BLOCK_ADDRESS_OFFSET(i), NULL, 1);
		}
	}
}

void settings_save(uint16_t address, uint8_t *__ptr, uint16_t size)
{
#ifdef RAM_ONLY_SETTINGS
	return;
#endif

	DBGMSG("EEPROM save @ %u", address);

	if (address == UINT16_MAX)
	{
#ifndef DISABLE_SAFE_SETTINGS
		g_settings_error |= SETTINGS_WRITE_ERROR;
#endif
		return;
	}

#ifdef ENABLE_SETTINGS_MODULES
	bool extended_save __attribute__((__cleanup__(EVENT_HANDLER_NAME(settings_extended_save)))) = (address == SETTINGS_ADDRESS_OFFSET);
#endif

	uint8_t crc = 0;
	nvm_start_write(address);
	for (uint16_t i = 0; i < size;)
	{
		if (cnc_get_exec_state(EXEC_RUN))
		{
			cnc_dotasks(); // updates buffer before cycling
		}

		uint8_t c = (__ptr != NULL) ? (*(__ptr++)) : ((uint8_t)grbl_stream_getc());
		crc = crc7(c, crc);
		nvm_putc(address++, c);
		i++;
		if (!__ptr && (c == EOL))
		{
			size = i;
			break;
		}
	}

	crc = settings_size_crc(size, crc);

	nvm_putc(address, crc);
	nvm_end_write();
}

bool settings_allows_negative(setting_offset_t id)
{
#if TOOL_COUNT > 0
	if (id > 80 && id <= (80 + TOOL_COUNT))
	{
		return true;
	}
#endif
#ifdef ENABLE_SKEW_COMPENSATION
	if (id >= 37 && id <= 39)
	{
		return true;
	}
#endif
	return false;
}

uint8_t settings_change(setting_offset_t id, float value)
{
	uint8_t result = STATUS_INVALID_STATEMENT;
	uint16_t value16 = (uint16_t)CLAMP(0, value, INT16_MAX);
	uint8_t value8 = (uint8_t)MIN(value16, UINT8_MAX);

	bool value1 = (value8 != 0);

#ifdef ENABLE_SETTINGS_MODULES
	if (id < 256)
	{
#endif
		if (value < 0 && !settings_allows_negative(id))
		{
			return STATUS_NEGATIVE_VALUE;
		}

		// id 0 conversion from us to frequency
		if (id == 0)
		{
			if (value < (1000000.0f / F_STEP_MAX))
			{
				return STATUS_MAX_STEP_RATE_EXCEEDED;
			}
		}

		uint8_t count = settings_count();
		for (uint8_t i = 0; i < count; i++)
		{
			setting_id_t s = {0};
			uint8_t max = 1;
			rom_memcpy(&s, &g_settings_id_table[i], sizeof(setting_id_t));
			if (s.type & SETTING_ARRAY)
			{
				max = SETTING_ARRCNT(s.type);
			}
			if ((uint8_t)id >= s.id && (uint8_t)id < (s.id + max))
			{
				switch (SETTING_TYPE_MASK(s.type))
				{
				case SETTING_TYPE_BOOL:
					((bool *)s.memptr)[(uint8_t)id - s.id] = value1;
					break;
				case SETTING_TYPE_UINT8:
					((uint8_t *)s.memptr)[(uint8_t)id - s.id] = value8;
					break;
				case SETTING_TYPE_UINT16:
					((uint16_t *)s.memptr)[(uint8_t)id - s.id] = value16;
					break;
				default: // default is float
					((float *)s.memptr)[(uint8_t)id - s.id] = value;
					break;
				}

				result = STATUS_OK;
			}
		}

#ifdef ENABLE_SETTINGS_MODULES
	}
	else
	{
		setting_args_t extended_setting = {.id = id, .value = value};
		if (!EVENT_INVOKE(settings_extended_change, &extended_setting))
		{
			return STATUS_INVALID_STATEMENT;
		}
		result = STATUS_OK;
	}
#endif

#if !defined(ENABLE_EXTRA_SETTINGS_CMDS)
	settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
#endif

	return result;
}

/**
 * Erases settings
 * If the address points to the SETTINGS_ADDRESS_OFFSET reloads default values and invokes extended settings erase event
 * Coordinate systems and ofsetts are set to 0
 * Startup blocks are set to 0
 */
void settings_erase(uint16_t address, uint8_t *__ptr, uint16_t size)
{
	DBGMSG("EEPROM erase @ %u", address);
	uint8_t empty_startup_block = 0;

	if (address == UINT16_MAX)
	{
		return;
	}

	bool is_machine_settings = (address == SETTINGS_ADDRESS_OFFSET);
	// checks if it's a valid pointer (startup blocks are a special case that uses a NULL pointer)
	if (__ptr)
	{
		if (is_machine_settings) // loads the defaults for settings
		{
			rom_memcpy(&g_settings, &default_settings, sizeof(settings_t));
		}
		else
		{
			memset(__ptr, 0, size);
		}
	}
	else
	{
		__ptr = &empty_startup_block;
		size = 1;
	}

#ifdef ENABLE_SETTINGS_MODULES
	// propagates the erase event to all extended settings
	EVENT_INVOKE(settings_extended_erase, &is_machine_settings);
#endif

#ifdef ENABLE_EXTRA_SETTINGS_CMDS
	if (!is_machine_settings)
	{
#endif
		// store the settings
		settings_save(address, __ptr, size);
#ifdef ENABLE_EXTRA_SETTINGS_CMDS
	}
#endif
}

bool settings_check_startup_gcode(uint16_t address)
{
	grbl_stream_start_broadcast();
	proto_putc('>');
#ifndef RAM_ONLY_SETTINGS
	if (settings_load(address, NULL, UINT16_MAX))
	{
		proto_putc(':');
		proto_error(STATUS_SETTING_READ_FAIL);
		settings_erase(address, NULL, 1);
		return false;
	}

	return true;
#else
	proto_putc(':');
	proto_error(0);
	return false;
#endif
}

uint16_t settings_register_external_setting(uint16_t size)
{
#if (defined(ENABLE_SETTINGS_MODULES) || defined(BOARD_HAS_CUSTOM_SYSTEM_COMMANDS))
	static uint16_t setting_offset = MODULES_SETTINGS_ADDRESS_OFFSET;
	uint16_t new_offset = setting_offset;
	setting_offset += size + 1; // include crc
	return new_offset;
#else
#warning "External/extension settings storing is disabled"
	return UINT16_MAX;
#endif
}

uint8_t settings_count(void)
{
	return sizeof(g_settings_id_table) / sizeof(setting_id_t);
}
