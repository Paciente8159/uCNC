/*
		Name: settings.h
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

#ifndef SETTINGS_H
#define SETTINGS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../module.h"
#include <stdint.h>
#include <stdbool.h>

	typedef struct
	{
		uint8_t version[3];
		float max_step_rate;
		// step delay not used
		uint8_t step_invert_mask;
		uint8_t dir_invert_mask;
		uint8_t step_enable_invert;
		uint8_t limits_invert_mask;
		bool probe_invert_mask;
		uint8_t status_report_mask;
		uint8_t control_invert_mask;
		// value must be set between -1.0 and 1.0 If set to 0.0 is the same as exact path mode (G61) and -1.0 is the same as exact stop mode (G61.1)
		float g64_angle_factor;
		// juntion deviation is automatic and always on
		float arc_tolerance;
		bool report_inches;
#if S_CURVE_ACCELERATION_LEVEL == -1
		uint8_t s_curve_profile;
#endif
		bool soft_limits_enabled;
		bool hard_limits_enabled;
		bool homing_enabled;
		uint16_t debounce_ms;
		uint8_t homing_dir_invert_mask;
		float homing_fast_feed_rate;
		float homing_slow_feed_rate;
		// debouncing not used
		float homing_offset;
		int16_t spindle_max_rpm;
		int16_t spindle_min_rpm;
		uint8_t laser_mode;
#ifdef ENABLE_LASER_PPI
		uint16_t laser_ppi;
		uint16_t laser_ppi_uswidth;
		float laser_ppi_mixmode_ppi;
		float laser_ppi_mixmode_uswidth;
#endif
		float step_per_mm[STEPPER_COUNT];
		float max_feed_rate[STEPPER_COUNT];
		float acceleration[STEPPER_COUNT];
		float max_distance[AXIS_COUNT];
#if TOOL_COUNT > 0
#if TOOL_COUNT > 1
		uint8_t default_tool;
#endif
		float tool_length_offset[TOOL_COUNT];
#endif
#if (KINEMATIC == KINEMATIC_LINEAR_DELTA)
		float delta_arm_length;
		float delta_armbase_radius;
		// float delta_efector_height;
#elif (KINEMATIC == KINEMATIC_DELTA)
	float delta_base_radius;
	float delta_effector_radius;
	float delta_bicep_length;
	float delta_forearm_length;
	float delta_bicep_homing_angle;
#elif (KINEMATIC == KINEMATIC_SCARA)
	float scara_arm_length;
	float scara_forearm_length;
	float scara_arm_homing_angle;
	float scara_forearm_homing_angle;
#endif
#ifdef ENABLE_BACKLASH_COMPENSATION
		uint16_t backlash_steps[AXIS_TO_STEPPERS];
#endif
#ifdef ENABLE_SKEW_COMPENSATION
		float skew_xy_factor;
#ifndef SKEW_COMPENSATION_XY_ONLY
		float skew_xz_factor;
		float skew_yz_factor;
#endif
#endif
#if ENCODERS > 0
		uint8_t encoders_pulse_invert_mask;
		uint8_t encoders_dir_invert_mask;
#endif
	} settings_t;

#ifndef SETTINGS_ADDRESS_OFFSET
#define SETTINGS_ADDRESS_OFFSET 0
#endif
#ifndef SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET
#define SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET (SETTINGS_ADDRESS_OFFSET + sizeof(settings_t) + 1)
#endif
#ifndef STARTUP_BLOCK0_ADDRESS_OFFSET
#define STARTUP_BLOCK0_ADDRESS_OFFSET (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (((AXIS_COUNT * sizeof(float)) + 1) * (COORD_SYS_COUNT + 3)))
#endif
#ifndef STARTUP_BLOCK1_ADDRESS_OFFSET
#define STARTUP_BLOCK1_ADDRESS_OFFSET (STARTUP_BLOCK0_ADDRESS_OFFSET + RX_BUFFER_SIZE)
#endif
#ifndef MODULES_SETTINGS_ADDRESS_OFFSET
#define MODULES_SETTINGS_ADDRESS_OFFSET (STARTUP_BLOCK1_ADDRESS_OFFSET + RX_BUFFER_SIZE)
#endif

#ifndef ENABLE_SETTINGS_MODULES
	typedef uint8_t setting_offset_t;
#else
typedef uint16_t setting_offset_t;
#endif

	extern settings_t g_settings;

	void settings_init(void);
	// Assumes that no structure being saved is bigger than 255 bytes
	uint8_t settings_load(uint16_t address, uint8_t *__ptr, uint16_t size);
	void settings_save(uint16_t address, uint8_t *__ptr, uint16_t size);
	void settings_reset(bool erase_startup_blocks);
	uint8_t settings_change(setting_offset_t id, float value);
	void settings_erase(uint16_t address, uint8_t *__ptr, uint16_t size);
	bool settings_check_startup_gcode(uint16_t address);
	uint16_t settings_register_external_setting(uint16_t size);

#if (defined(ENABLE_SETTINGS_MODULES) || defined(BOARD_HAS_CUSTOM_SYSTEM_COMMANDS))
	// event_settings_change_handler
	typedef struct setting_args_
	{
		uint16_t id;
		float value;
	} setting_args_t;
	DECL_EVENT_HANDLER(settings_change);
	typedef struct settings_args_
	{
		uint16_t address;
		uint8_t *data;
		uint8_t size;
	} settings_args_t;
	// event_settings_load_handler
	DECL_EVENT_HANDLER(settings_load);
	// event_settings_save_handler
	DECL_EVENT_HANDLER(settings_save);
	// event_settings_save_extended_handler
	DECL_EVENT_HANDLER(settings_extended_save);
	// event_settings_erase_handler
	DECL_EVENT_HANDLER(settings_erase);
#endif

/**
 *
 * Settings quick/help macros
 * These allow custom settings setup of simple settings
 *
 * **/
#define DECL_EXTENDED_SETTING(ID, var, type, count, print_cb)                                      \
	static uint32_t set##ID##_settings_address;                                                    \
	bool set##ID##_settings_load(void *args)                                                       \
	{                                                                                              \
		settings_args_t *set = (settings_args_t *)args;                                            \
		if (set->address == SETTINGS_ADDRESS_OFFSET || set->address == set##ID##_settings_address) \
		{                                                                                          \
			settings_load(set##ID##_settings_address, (uint8_t *)var, sizeof(type) * count);       \
			return (set->address == set##ID##_settings_address) ? EVENT_HANDLED : EVENT_CONTINUE;  \
		}                                                                                          \
		return EVENT_CONTINUE;                                                                     \
	}                                                                                              \
	bool set##ID##_settings_save(void *args)                                                       \
	{                                                                                              \
		settings_args_t *set = (settings_args_t *)args;                                            \
		if (set->address == SETTINGS_ADDRESS_OFFSET || set->address == set##ID##_settings_address) \
		{                                                                                          \
			settings_save(set##ID##_settings_address, (uint8_t *)var, sizeof(type) * count);       \
			return (set->address == set##ID##_settings_address) ? EVENT_HANDLED : EVENT_CONTINUE;  \
		}                                                                                          \
		return EVENT_CONTINUE;                                                                     \
	}                                                                                              \
	bool set##ID##_settings_change(void *args)                                                     \
	{                                                                                              \
		setting_args_t *set = (setting_args_t *)args;                                              \
		type *ptr = var;                                                                           \
		if (set->id >= ID && set->id <= (ID + count))                                              \
		{                                                                                          \
			ptr[set->id - ID] = (type)set->value;                                                  \
			return EVENT_HANDLED;                                                                  \
		}                                                                                          \
		return EVENT_CONTINUE;                                                                     \
	}                                                                                              \
	bool set##ID##_settings_erase(void *args)                                                      \
	{                                                                                              \
		settings_args_t *set = (settings_args_t *)args;                                            \
		if (set->address == SETTINGS_ADDRESS_OFFSET || set->address == set##ID##_settings_address) \
		{                                                                                          \
			memset(var, 0, sizeof(type) * count);                                                  \
			return (set->address == set##ID##_settings_address) ? EVENT_HANDLED : EVENT_CONTINUE;  \
		}                                                                                          \
		return EVENT_HANDLED;                                                                      \
	}                                                                                              \
	bool set##ID##_protocol_send_cnc_settings(void *args)                                          \
	{                                                                                              \
		type *ptr = var;                                                                           \
		for (uint8_t i = 0; i < count; i++)                                                        \
		{                                                                                          \
			print_cb(ID + i, ptr[i]);                                                              \
		}                                                                                          \
		return EVENT_CONTINUE;                                                                     \
	}                                                                                              \
	CREATE_EVENT_LISTENER(settings_load, set##ID##_settings_load);                                 \
	CREATE_EVENT_LISTENER(settings_save, set##ID##_settings_save);                                 \
	CREATE_EVENT_LISTENER(settings_change, set##ID##_settings_change);                             \
	CREATE_EVENT_LISTENER(settings_erase, set##ID##_settings_erase);                               \
	CREATE_EVENT_LISTENER(protocol_send_cnc_settings, set##ID##_protocol_send_cnc_settings)

#define DECL_EXTENDED_STRING_SETTING(ID, var, count)                                               \
	static uint32_t set##ID##_settings_address;                                                    \
	bool set##ID##_settings_load(void *args)                                                       \
	{                                                                                              \
		settings_args_t *set = (settings_args_t *)args;                                            \
		if (set->address == SETTINGS_ADDRESS_OFFSET || set->address == set##ID##_settings_address) \
		{                                                                                          \
			settings_load(set##ID##_settings_address, (uint8_t *)var, sizeof(char) * count);       \
			return (set->address == set##ID##_settings_address) ? EVENT_HANDLED : EVENT_CONTINUE;  \
		}                                                                                          \
		return EVENT_CONTINUE;                                                                     \
	}                                                                                              \
	bool set##ID##_settings_save(void *args)                                                       \
	{                                                                                              \
		settings_args_t *set = (settings_args_t *)args;                                            \
		if (set->address == SETTINGS_ADDRESS_OFFSET || set->address == set##ID##_settings_address) \
		{                                                                                          \
			settings_save(set##ID##_settings_address, (uint8_t *)var, sizeof(char) * count);       \
			return (set->address == set##ID##_settings_address) ? EVENT_HANDLED : EVENT_CONTINUE;  \
		}                                                                                          \
		return EVENT_CONTINUE;                                                                     \
	}                                                                                              \
	bool set##ID##_settings_change(void *args)                                                     \
	{                                                                                              \
		setting_args_t *set = (setting_args_t *)args;                                              \
		if (set->id == ID)                                                                         \
		{                                                                                          \
			for (uint8_t i = 0; i < count; i++)                                                    \
			{                                                                                      \
				char c = serial_getc();                                                            \
				if (c == EOL || c == '\n')                                                         \
				{                                                                                  \
					var[i] = EOL;                                                                  \
					break;                                                                         \
				}                                                                                  \
				var[i] = c;                                                                        \
			}                                                                                      \
			return EVENT_HANDLED;                                                                  \
		}                                                                                          \
		return EVENT_CONTINUE;                                                                     \
	}                                                                                              \
	bool set##ID##_settings_erase(void *args)                                                      \
	{                                                                                              \
		settings_args_t *set = (settings_args_t *)args;                                            \
		if (set->address == SETTINGS_ADDRESS_OFFSET || set->address == set##ID##_settings_address) \
		{                                                                                          \
			memset(var, 0, sizeof(char) * count);                                                  \
			return (set->address == set##ID##_settings_address) ? EVENT_HANDLED : EVENT_CONTINUE;  \
		}                                                                                          \
		return EVENT_HANDLED;                                                                      \
	}                                                                                              \
	bool set##ID##_protocol_send_cnc_settings(void *args)                                          \
	{                                                                                              \
		settings_args_t *set = (settings_args_t *)args;                                            \
		serial_putc('$');                                                                          \
		serial_print_int(ID);                                                                      \
		serial_putc('=');                                                                          \
		for (uint8_t i = 0; i < count; i++)                                                        \
		{                                                                                          \
			char c = var[i];                                                                       \
			if (c < 20 || c > 127)                                                                 \
			{                                                                                      \
				protocol_send_string(MSG_EOL);                                                     \
				return EVENT_CONTINUE;                                                             \
			}                                                                                      \
			serial_putc(c);                                                                        \
		}                                                                                          \
		return EVENT_CONTINUE;                                                                     \
	}                                                                                              \
	CREATE_EVENT_LISTENER(settings_load, set##ID##_settings_load);                                 \
	CREATE_EVENT_LISTENER(settings_save, set##ID##_settings_save);                                 \
	CREATE_EVENT_LISTENER(settings_change, set##ID##_settings_change);                             \
	CREATE_EVENT_LISTENER(settings_erase, set##ID##_settings_erase);                               \
	CREATE_EVENT_LISTENER(protocol_send_cnc_settings, set##ID##_protocol_send_cnc_settings)

#define EXTENDED_SETTING_ADDRESS(ID) set##ID##_settings_address

#define EXTENDED_SETTING_INIT(ID, var)                                                        \
	static bool set##ID##_init = false;                                                       \
	if (!set##ID##_init)                                                                      \
	{                                                                                         \
		set##ID##_settings_address = settings_register_external_setting(sizeof(var));         \
		ADD_EVENT_LISTENER(settings_load, set##ID##_settings_load);                           \
		ADD_EVENT_LISTENER(settings_save, set##ID##_settings_save);                           \
		ADD_EVENT_LISTENER(settings_change, set##ID##_settings_change);                       \
		ADD_EVENT_LISTENER(settings_erase, set##ID##_settings_erase);                         \
		ADD_EVENT_LISTENER(protocol_send_cnc_settings, set##ID##_protocol_send_cnc_settings); \
		set##ID##_init = true;                                                                \
	}

#ifdef __cplusplus
}
#endif

#endif
