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
#define SETTINGS_VERSION \
	{                    \
		'V', '0', '6'    \
	}

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
#if (KINEMATIC == KINEMATIC_LINEAR_DELTA)
		.delta_arm_length = DEFAULT_LIN_DELTA_ARM_LENGTH,
		.delta_armbase_radius = DEFAULT_LIN_DELTA_BASE_RADIUS,
// float delta_efector_height;
#elif (KINEMATIC == KINEMATIC_DELTA)
		.delta_base_radius = DEFAULT_DELTA_BASE_RADIUS,
		.delta_effector_radius = DEFAULT_DELTA_EFFECTOR_RADIUS,
		.delta_bicep_length = DEFAULT_DELTA_BICEP_LENGTH,
		.delta_forearm_length = DEFAULT_DELTA_FOREARM_LENGTH,
		.delta_bicep_homing_angle = DEFAULT_DELTA_BICEP_HOMING_ANGLE,
#elif (KINEMATIC == KINEMATIC_SCARA)
		.scara_arm_length = DEFAULT_SCARA_ARM_LENGTH,
		.scara_forearm_length = DEFAULT_SCARA_FOREARM_LENGTH,
		.scara_arm_homing_angle = DEFAULT_SCARA_ARM_HOMING_ANGLE,
		.scara_forearm_homing_angle = DEFAULT_SCARA_FOREARM_HOMING_ANGLE,
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
#endif

static uint8_t settings_size_crc(uint16_t size, uint8_t crc)
{
	crc = crc7(((uint8_t *)&size)[0], crc);
	return crc7(((uint8_t *)&size)[1], crc);
}

void settings_init(void)
{
	const uint8_t version[3] = SETTINGS_VERSION;
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

uint8_t settings_load(uint16_t address, uint8_t *__ptr, uint16_t size)
{
	DEBUG_STR("EEPROM load @ ");
	DEBUG_INT(address);
	DEBUG_PUTC('\n');

	// settiing address invalid
	if (address == UINT16_MAX)
	{
		return STATUS_SETTING_DISABLED;
	}
#ifdef ENABLE_SETTINGS_MODULES
	bool extended_load __attribute__((__cleanup__(EVENT_HANDLER_NAME(settings_extended_load)))) = (address == SETTINGS_ADDRESS_OFFSET);
	settings_args_t args = {.address = address, .data = __ptr, .size = size};
	// if handled exit
	if (EVENT_INVOKE(settings_load, &args))
	{
		return STATUS_OK;
	}
	// if unable to get settings from external memory tries to get from internal EEPROM
#endif

#ifndef RAM_ONLY_SETTINGS
	uint8_t crc = 0;

	for (uint16_t i = 0; i < size;)
	{
		uint8_t value = mcu_eeprom_getc(address++);
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

	return (crc ^ mcu_eeprom_getc(address));
#else
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
}

void settings_reset(bool erase_startup_blocks)
{
	settings_erase(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, sizeof(settings_t));
	rom_memcpy(&g_settings, &default_settings, sizeof(settings_t));

#if !defined(ENABLE_EXTRA_SYSTEM_CMDS) && !defined(RAM_ONLY_SETTINGS)
	settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
	if (erase_startup_blocks)
	{
		settings_erase(STARTUP_BLOCK0_ADDRESS_OFFSET, NULL, 1);
		settings_erase(STARTUP_BLOCK1_ADDRESS_OFFSET, NULL, 1);
	}
#endif
}

void settings_save(uint16_t address, uint8_t *__ptr, uint16_t size)
{
	DEBUG_STR("EEPROM save @ ");
	DEBUG_INT(address);
	DEBUG_PUTC('\n');

	if (address == UINT16_MAX)
	{
		return;
	}

#ifdef ENABLE_SETTINGS_MODULES
	bool extended_save __attribute__((__cleanup__(EVENT_HANDLER_NAME(settings_extended_save)))) = (address == SETTINGS_ADDRESS_OFFSET);
	settings_args_t args = {.address = address, .data = __ptr, .size = size};
	if (EVENT_INVOKE(settings_save, &args))
	{
		// if the event was handled
		return;
	}
#endif

#ifndef RAM_ONLY_SETTINGS
	uint8_t crc = 0;

	for (uint16_t i = 0; i < size;)
	{
		if (cnc_get_exec_state(EXEC_RUN))
		{
			cnc_dotasks(); // updates buffer before cycling
		}

		uint8_t c = (__ptr != NULL) ? (*(__ptr++)) : ((uint8_t)serial_getc());
		crc = crc7(c, crc);
		mcu_eeprom_putc(address++, c);
		i++;
		if (!__ptr && (c == EOL))
		{
			size = i;
			break;
		}
	}

	crc = settings_size_crc(size, crc);

	mcu_eeprom_putc(address, crc);
	mcu_eeprom_flush();
#endif
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
	uint8_t result = STATUS_OK;
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

		uint8_t setting = (uint8_t)id;
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
#if EMULATE_GRBL_STARTUP > 0
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
			g_settings.report_inches = value1;
			break;
#if S_CURVE_ACCELERATION_LEVEL == -1
		case 14:
			g_settings.s_curve_profile = CLAMP(0, value8, 5);
			break;
#endif
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
#if TOOL_COUNT > 1
		case 80:
			g_settings.default_tool = CLAMP(0, value8, (uint8_t)TOOL_COUNT);
			break;
#endif
#if (KINEMATIC == KINEMATIC_LINEAR_DELTA)
		case 106:
			g_settings.delta_arm_length = value;
			break;
		case 107:
			g_settings.delta_armbase_radius = value;
			break;
			// case 108:
			//     g_settings.delta_efector_height = value;
			//     break;
#elif (KINEMATIC == KINEMATIC_DELTA)
	case 106:
		g_settings.delta_base_radius = value;
		break;
	case 107:
		g_settings.delta_effector_radius = value;
		break;
	case 108:
		g_settings.delta_bicep_length = value;
		break;
	case 109:
		g_settings.delta_forearm_length = value;
		break;
	case 28:
		g_settings.delta_bicep_homing_angle = value;
		break;
#elif (KINEMATIC == KINEMATIC_SCARA)
	case 106:
		g_settings.scara_arm_length = value;
		break;
	case 107:
		g_settings.scara_forearm_length = value;
		break;
	case 28:
		g_settings.scara_arm_homing_angle = value;
		break;
	case 29:
		g_settings.scara_forearm_homing_angle = value;
		break;
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
		}
#ifdef ENABLE_SETTINGS_MODULES
	}
	else
	{
		setting_args_t extended_setting = {.id = id, .value = value};
		if (!EVENT_INVOKE(settings_change, &extended_setting))
		{
			return STATUS_INVALID_STATEMENT;
		}
	}
#endif

#if !defined(ENABLE_EXTRA_SYSTEM_CMDS)
	settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
#endif

	return result;
}

void settings_erase(uint16_t address, uint8_t *__ptr, uint16_t size)
{
	DEBUG_STR("EEPROM erase @ ");
	DEBUG_INT(address);
	DEBUG_PUTC('\n');

	if (address == UINT16_MAX)
	{
		return;
	}

#ifdef ENABLE_SETTINGS_MODULES
	bool extended_erase __attribute__((__cleanup__(EVENT_HANDLER_NAME(settings_extended_erase)))) = (address == SETTINGS_ADDRESS_OFFSET);
	settings_args_t args = {.address = address, .data = __ptr, .size = size};
	if (EVENT_INVOKE(settings_erase, &args))
	{
		// if the event was handled
		return;
	}
#endif

	if (__ptr)
	{
		memset(__ptr, 0, size);
	}

#ifndef RAM_ONLY_SETTINGS
	if (address != SETTINGS_ADDRESS_OFFSET)
	{
		for (uint16_t i = size; i != 0; i--)
		{
			if (cnc_get_exec_state(EXEC_RUN))
			{
				cnc_dotasks(); // updates buffer before cycling
			}
			mcu_eeprom_putc(address++, 0);
		}

		uint8_t crc = settings_size_crc(size, 0);

		// erase crc byte that is next to data
		mcu_eeprom_putc(address, crc);
#if !defined(ENABLE_EXTRA_SYSTEM_CMDS)
		mcu_eeprom_flush();
#endif
	}
#endif
}

bool settings_check_startup_gcode(uint16_t address)
{
	serial_broadcast(true);
	serial_putc('>');
#ifndef RAM_ONLY_SETTINGS
	if (settings_load(address, NULL, UINT16_MAX))
	{
		serial_putc(':');
		protocol_send_error(STATUS_SETTING_READ_FAIL);
		settings_erase(address, NULL, 1);
		return false;
	}

	return true;
#else
	serial_putc(':');
	protocol_send_ok();
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
