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

#define SETTINGS_OK
#define SETTINGS_READ_ERROR 1
#define SETTINGS_WRITE_ERROR 2

	typedef struct
	{
		uint8_t version[3];
		float max_step_rate;
// step delay not used
#ifdef ENABLE_STEPPERS_DISABLE_TIMEOUT
		uint16_t step_disable_timeout;
#endif
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

		KINEMATICS_VARS_DECL /*KINEMATICS VARS DECLARATIONS*/

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
#ifdef H_MAPPING_EEPROM_STORE_ENABLED
		float hmap_x;
		float hmap_y;
		float hmap_x_offset;
		float hmap_y_offset;
		float hmap_offsets[H_MAPING_ARRAY_SIZE];
#endif
	} settings_t;

// settings base address
#ifndef SETTINGS_ADDRESS_OFFSET
#define SETTINGS_ADDRESS_OFFSET 0
#endif
#ifndef SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET
#define SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET (SETTINGS_ADDRESS_OFFSET + sizeof(settings_t) + 1)
#endif
// coordinate systems base addresses
#ifndef DISABLE_HOME_SUPPORT
#define ADDITIONAL_COORDINATES 2
#else
#define ADDITIONAL_COORDINATES 0
#endif
#ifdef DISABLE_COORD_SYS_SUPPORT
#define TOTAL_COORDINATE_SYSTEMS (1 + ADDITIONAL_COORDINATES)
#else
#define TOTAL_COORDINATE_SYSTEMS (COORD_SYS_COUNT + ADDITIONAL_COORDINATES)
#endif
#define PARSER_PARAM_SIZE (sizeof(float) * AXIS_COUNT)																								// parser parameters array size
#define PARSER_PARAM_ADDR_OFFSET (PARSER_PARAM_SIZE + 1)																							// parser parameters array size + 1 crc byte
#define G28HOME (COORD_SYS_COUNT)																																			// G28 index
#define G30HOME (COORD_SYS_COUNT + 1)																																	// G30 index
#define G92OFFSET (COORD_SYS_COUNT + ADDITIONAL_COORDINATES)																					// G92 index
#define PARSER_CORDSYS_ADDRESS (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET)														// 1st coordinate system offset eeprom address (G54)
#define G28ADDRESS (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (PARSER_PARAM_ADDR_OFFSET * G28HOME)) // G28 coordinate offset eeprom address
#define G30ADDRESS (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (PARSER_PARAM_ADDR_OFFSET * G30HOME)) // G28 coordinate offset eeprom address
#ifdef G92_STORE_NONVOLATILE
#define G92ADDRESS (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (PARSER_PARAM_ADDR_OFFSET * G92OFFSET)) // G92 coordinate offset eeprom address
#endif

#ifndef STARTUP_BLOCKS_COUNT
#define STARTUP_BLOCKS_COUNT 2
#endif
#ifndef STARTUP_BLOCK_SIZE
#define STARTUP_BLOCK_SIZE RX_BUFFER_SIZE
#endif
#ifndef STARTUP_BLOCK0_ADDRESS_OFFSET
#ifdef G92_STORE_NONVOLATILE
#define STARTUP_BLOCK0_ADDRESS_OFFSET (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (PARSER_PARAM_ADDR_OFFSET * (TOTAL_COORDINATE_SYSTEMS + 1)))
#else
#define STARTUP_BLOCK0_ADDRESS_OFFSET (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (PARSER_PARAM_ADDR_OFFSET * TOTAL_COORDINATE_SYSTEMS))
#endif
#endif
#define STARTUP_BLOCK_ADDRESS_OFFSET(NBLOCK) (STARTUP_BLOCK0_ADDRESS_OFFSET + (NBLOCK * RX_BUFFER_SIZE))

#ifndef MODULES_SETTINGS_ADDRESS_OFFSET
#define MODULES_SETTINGS_ADDRESS_OFFSET STARTUP_BLOCK_ADDRESS_OFFSET(STARTUP_BLOCKS_COUNT)
#endif

#ifndef ENABLE_SETTINGS_MODULES
	typedef uint8_t setting_offset_t;
#else
typedef uint16_t setting_offset_t;
#endif

#define SETTING_TYPE(T) (T << 5)
#define SETTING_TYPE_MASK(T) (T & (0x03 << 5))
#define SETTING_ARRAY 0x80
#define SETTING_ARRCNT(X) (X & 0x1F)
#define SETTING_TYPE_BOOL SETTING_TYPE(1)
#define SETTING_TYPE_UINT8 SETTING_TYPE(2)
#define SETTING_TYPE_UINT16 SETTING_TYPE(3)
#define SETTING_TYPE_FLOAT SETTING_TYPE(0)

	typedef struct setting_id_
	{
		setting_offset_t id;
		void *memptr;
		uint8_t type;
	} setting_id_t;

#ifndef DISABLE_SAFE_SETTINGS
	extern uint8_t g_settings_error;
#endif
	extern settings_t g_settings;
	extern const setting_id_t g_settings_id_table[];

	/**
	 * Overridable callbacks to implement external NVM mechanisms
	 */
	void nvm_start_read(uint16_t address);
	void nvm_start_write(uint16_t address);
	uint8_t nvm_getc(uint16_t address);
	void nvm_putc(uint16_t address, uint8_t c);
	void nvm_end_read(void);
	void nvm_end_write(void);

	void settings_init(void);
	// Assumes that no structure being saved is bigger than 255 bytes
	uint8_t settings_load(uint16_t address, uint8_t *__ptr, uint16_t size);
	void settings_save(uint16_t address, uint8_t *__ptr, uint16_t size);
	void settings_reset(bool erase_startup_blocks);
	uint8_t settings_change(setting_offset_t id, float value);
	void settings_erase(uint16_t address, uint8_t *__ptr, uint16_t size);
	bool settings_check_startup_gcode(uint16_t address);
	uint16_t settings_register_external_setting(uint16_t size);
	uint8_t settings_count(void);

#if (defined(ENABLE_SETTINGS_MODULES) || defined(BOARD_HAS_CUSTOM_SYSTEM_COMMANDS))
	// event_settings_extended_load_handler
	DECL_EVENT_HANDLER(settings_extended_load);
	// event_settings_extended_save_handler
	DECL_EVENT_HANDLER(settings_extended_save);
	// event_settings_extended_erase_handler
	DECL_EVENT_HANDLER(settings_extended_erase);
	// event_settings_extended_change_handler
	typedef struct setting_args_
	{
		uint16_t id;
		float value;
	} setting_args_t;
	DECL_EVENT_HANDLER(settings_extended_change);
#endif

/**
 *
 * Settings quick/help macros
 * These allow custom settings setup of simple settings
 *
 * **/
#define __DECL_EXTENDED_SETTING__(ID, var, type, count, print_cb)                    \
	static uint32_t set##ID##_settings_address;                                        \
	bool set##ID##_settings_load(void *args)                                           \
	{                                                                                  \
		settings_load(set##ID##_settings_address, (uint8_t *)var, sizeof(type) * count); \
		return EVENT_CONTINUE;                                                           \
	}                                                                                  \
	bool set##ID##_settings_save(void *args)                                           \
	{                                                                                  \
		settings_save(set##ID##_settings_address, (uint8_t *)var, sizeof(type) * count); \
		return EVENT_CONTINUE;                                                           \
	}                                                                                  \
	bool set##ID##_settings_change(void *args)                                         \
	{                                                                                  \
		setting_args_t *set = (setting_args_t *)args;                                    \
		type *ptr = var;                                                                 \
		if (set->id >= ID && set->id < (ID + count))                                     \
		{                                                                                \
			ptr[set->id - ID] = (type)set->value;                                          \
			return EVENT_HANDLED;                                                          \
		}                                                                                \
		return EVENT_CONTINUE;                                                           \
	}                                                                                  \
	bool set##ID##_settings_erase(void *args)                                          \
	{                                                                                  \
		memset(var, 0, sizeof(type) * count);                                            \
		return EVENT_CONTINUE;                                                           \
	}                                                                                  \
	bool set##ID##_proto_cnc_settings(void *args)                                      \
	{                                                                                  \
		type *ptr = var;                                                                 \
		for (uint8_t i = 0; i < count; i++)                                              \
		{                                                                                \
			print_cb(ID + i, ptr[i]);                                                      \
		}                                                                                \
		return EVENT_CONTINUE;                                                           \
	}                                                                                  \
	CREATE_EVENT_LISTENER(settings_extended_load, set##ID##_settings_load);            \
	CREATE_EVENT_LISTENER(settings_extended_save, set##ID##_settings_save);            \
	CREATE_EVENT_LISTENER(settings_extended_change, set##ID##_settings_change);        \
	CREATE_EVENT_LISTENER(settings_extended_erase, set##ID##_settings_erase);          \
	CREATE_EVENT_LISTENER(proto_cnc_settings, set##ID##_proto_cnc_settings)
#define DECL_EXTENDED_SETTING(ID, var, type, count, print_cb) __DECL_EXTENDED_SETTING__(ID, var, type, count, print_cb)

#define __DECL_EXTENDED_STRING_SETTING__(ID, var, count)                               \
	static uint32_t set##ID##_settings_address;                                          \
	bool set##ID##_settings_load(void *args)                                             \
	{                                                                                    \
		settings_load(set##ID##_settings_address, (uint8_t *)var, sizeof(char) * count);   \
		return EVENT_CONTINUE;                                                             \
	}                                                                                    \
	bool set##ID##_settings_save(void *args)                                             \
	{                                                                                    \
		return EVENT_CONTINUE;                                                             \
	}                                                                                    \
	bool set##ID##_settings_change(void *args)                                           \
	{                                                                                    \
		setting_args_t *set = (setting_args_t *)args;                                      \
		if (set->id == ID)                                                                 \
		{                                                                                  \
			settings_load(set##ID##_settings_address, (uint8_t *)var, sizeof(char) * count); \
			for (uint8_t i = 0; i < count; i++)                                              \
			{                                                                                \
				char c = grbl_stream_getc();                                                   \
				if (c == EOL || c == '\n')                                                     \
				{                                                                              \
					var[i] = EOL;                                                                \
					break;                                                                       \
				}                                                                              \
				var[i] = c;                                                                    \
			}                                                                                \
			settings_save(set##ID##_settings_address, (uint8_t *)var, sizeof(char) * count); \
			return EVENT_HANDLED;                                                            \
		}                                                                                  \
		return EVENT_CONTINUE;                                                             \
	}                                                                                    \
	bool set##ID##_settings_erase(void *args)                                            \
	{                                                                                    \
		memset(var, 0, sizeof(char) * count);                                              \
		settings_save(set##ID##_settings_address, (uint8_t *)var, sizeof(char) * count);   \
		return EVENT_CONTINUE;                                                             \
	}                                                                                    \
	bool set##ID##_proto_cnc_settings(void *args)                                        \
	{                                                                                    \
		memset(var, 0, sizeof(char) * count);                                              \
		settings_load(set##ID##_settings_address, (uint8_t *)var, sizeof(char) * count);   \
		proto_putc('$');                                                                   \
		proto_printf("%ld", ID);                                                           \
		proto_putc('=');                                                                   \
		for (uint8_t i = 0; i < count; i++)                                                \
		{                                                                                  \
			char c = var[i];                                                                 \
			if (c < 20 || c > 127)                                                           \
			{                                                                                \
				proto_print(MSG_EOL);                                                          \
				return EVENT_CONTINUE;                                                         \
			}                                                                                \
			proto_putc(c);                                                                   \
		}                                                                                  \
		return EVENT_CONTINUE;                                                             \
	}                                                                                    \
	CREATE_EVENT_LISTENER(settings_extended_load, set##ID##_settings_load);              \
	CREATE_EVENT_LISTENER(settings_extended_save, set##ID##_settings_save);              \
	CREATE_EVENT_LISTENER(settings_extended_change, set##ID##_settings_change);          \
	CREATE_EVENT_LISTENER(settings_extended_erase, set##ID##_settings_erase);            \
	CREATE_EVENT_LISTENER(proto_cnc_settings, set##ID##_proto_cnc_settings)

#define DECL_EXTENDED_STRING_SETTING(ID, var, count) __DECL_EXTENDED_STRING_SETTING__(ID, var, count)

#define __EXTENDED_SETTING_ADDRESS__(ID) set##ID##_settings_address
#define EXTENDED_SETTING_ADDRESS(ID) __EXTENDED_SETTING_ADDRESS__(ID)

#define __EXTENDED_SETTING_INIT__(ID, var)                                        \
	static bool set##ID##_init = false;                                             \
	if (!set##ID##_init)                                                            \
	{                                                                               \
		set##ID##_settings_address = settings_register_external_setting(sizeof(var)); \
		ADD_EVENT_LISTENER(settings_extended_load, set##ID##_settings_load);          \
		ADD_EVENT_LISTENER(settings_extended_save, set##ID##_settings_save);          \
		ADD_EVENT_LISTENER(settings_extended_change, set##ID##_settings_change);      \
		ADD_EVENT_LISTENER(settings_extended_erase, set##ID##_settings_erase);        \
		ADD_EVENT_LISTENER(proto_cnc_settings, set##ID##_proto_cnc_settings);         \
		set##ID##_init = true;                                                        \
	}

#define EXTENDED_SETTING_INIT(ID, var) __EXTENDED_SETTING_INIT__(ID, var)

#ifdef __cplusplus
}
#endif

#endif
