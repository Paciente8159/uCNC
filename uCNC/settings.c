/*
	Name: setting.c - uCNC settings functionalities
	Copyright: 2019 João Martins
	Author: João Martins
	Date: Nov/2019
	Description: uCNC is a free cnc controller software designed to be flexible and
	portable to several	microcontrollers/architectures.
	uCNC is a FREE SOFTWARE under the terms of the GPLv3 (see <http://www.gnu.org/licenses/>).
*/

#include "config.h"
#include "defaults.h"
#include "settings.h"
#include "mcudefs.h"
#include "mcu.h"
#include "grbl_interface.h"
#include "protocol.h"

//if settings struct is changed this version has to change too
#define SETTINGS_VERSION "V01"

settings_t g_settings;

const uint8_t __rom__ crc7_table[256] = {
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
	0x46, 0x4f, 0x54, 0x5d, 0x62, 0x6b, 0x70, 0x79
};

const settings_t __rom__ defaults = {\
	.version = SETTINGS_VERSION,
	#ifdef AXIS_X
	.step_per_mm[AXIS_X] = DEFAULT_X_STEP_PER_MM,
	.max_feed_rate[AXIS_X] = DEFAULT_X_MAX_FEED,
	.acceleration[AXIS_X] = DEFAULT_X_ACCEL,
	.max_distance[AXIS_X] = DEFAULT_X_MAX_DIST,
	#endif
	#ifdef AXIS_Y
	.step_per_mm[AXIS_Y] = DEFAULT_Y_STEP_PER_MM,
	.max_feed_rate[AXIS_Y] = DEFAULT_Y_MAX_FEED,
	.acceleration[AXIS_Y] = DEFAULT_Y_ACCEL,
	.max_distance[AXIS_Y] = DEFAULT_Y_MAX_DIST,
	#endif
	#ifdef AXIS_Z
	.step_per_mm[AXIS_Z] = DEFAULT_Z_STEP_PER_MM,
	.max_feed_rate[AXIS_Z] = DEFAULT_Z_MAX_FEED,
	.acceleration[AXIS_Z] = DEFAULT_Z_ACCEL,
	.max_distance[AXIS_Z] = DEFAULT_Z_MAX_DIST,
	#endif
	#ifdef AXIS_A
	.step_per_mm[AXIS_A] = DEFAULT_A_STEP_PER_MM,
	.max_feed_rate[AXIS_A] = DEFAULT_A_MAX_FEED,
	.acceleration[AXIS_A] = DEFAULT_A_ACCEL,
	.max_distance[AXIS_A] = DEFAULT_A_MAX_DIST,
	#endif
	#ifdef AXIS_B
	.step_per_mm[AXIS_B] = DEFAULT_B_STEP_PER_MM,
	.max_feed_rate[AXIS_B] = DEFAULT_B_MAX_FEED,
	.acceleration[AXIS_B] = DEFAULT_B_ACCEL,
	.max_distance[AXIS_B] = DEFAULT_B_MAX_DIST,
	#endif
	#ifdef AXIS_C
	.step_per_mm[AXIS_C] = DEFAULT_C_STEP_PER_MM,
	.max_feed_rate[AXIS_C] = DEFAULT_C_MAX_FEED,
	.acceleration[AXIS_C] = DEFAULT_C_ACCEL,
	.max_distance[AXIS_C] = DEFAULT_C_MAX_DIST,
	#endif

	.step_enable_invert = DEFAULT_STEP_ENA_INV,
	.step_invert_mask = DEFAULT_STEP_INV_MASK,
    .dir_invert_mask = DEFAULT_DIR_INV_MASK,
    .homing_dir_invert_mask = DEFAULT_HOMING_DIR_INV_MASK,
    .homing_fast_feed_rate = DEFAULT_HOMING_FAST,
  	.homing_slow_feed_rate = DEFAULT_HOMING_SLOW,
  	.homing_offset = DEFAULT_HOMING_OFFSET,
	.arc_tolerance = DEFAULT_ARC_TOLERANCE,
	.tool_count = DEFAULT_TOOL_COUNT,
	.limits_invert_mask = DEFAULT_LIMIT_INV_MASK,
	.status_report_mask = DEFAULT_STATUS_MASK,
	.control_invert_mask = DEFAULT_CONTROL_INV_MASK,
	.max_step_rate = DEFAULT_MAX_STEP_RATE,
	.soft_limits_enabled = true,
	.hard_limits_enabled = true,
	.homing_enabled = true
	};


/*static uint8_t settings_crc7 (uint8_t  crc, uint8_t* pc, int len)
{
    do
    {
    	crc = *(uint8_t*)rom_read_byte(&crc7_table[crc ^ *pc++]);
	} while (--len);
	
    return crc;
}*/

uint8_t settings_init()
{
	return settings_load(SETTINGS_ADDRESS_OFFSET, (uint8_t*) &g_settings, sizeof(settings_t));
}

uint8_t settings_load(uint16_t address, uint8_t* __ptr, uint16_t size)
{
	uint8_t crc = 0;
	for(uint16_t i = size; i !=0; )
	{
		i--;
		__ptr[i] = mcu_eeprom_getc(i + address);
		crc = *(uint8_t*)rom_read_byte(&crc7_table[crc ^ __ptr[i]]);
	}
	
	return crc;
}

void settings_reset()
{
	settings_t* ptr = (settings_t*)&g_settings;
	const settings_t* defptr = &defaults;
	uint8_t size = sizeof(settings_t);
	
	rom_memcpy(ptr, defptr, size);
	settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t*)ptr, size);
}

void settings_save(uint16_t address, const uint8_t* __ptr, uint16_t size)
{
	for(uint16_t i = size; i !=0; )
	{
		i--;
		mcu_eeprom_putc(i + address, __ptr[i]);
	}
}

uint8_t settings_change(uint8_t setting, float value)
{
	uint8_t result = 0;
	uint8_t value8 = (uint8_t)value;
	uint8_t value16 = (uint16_t)value;
	bool value1 = (value!=0);

	if(value < 0)
	{
		return STATUS_NEGATIVE_VALUE;
	}
	
	switch(setting)
	{
		case 0:
			if(value > F_PULSE_MAX)
			{
				return STATUS_MAX_STEP_RATE_EXCEEDED;
			}
			g_settings.max_step_rate = value16;
			break;
		case 2:
			g_settings.step_invert_mask = value8;
			break;
		case 3:
			g_settings.dir_invert_mask = value8;
			break;
		case 4:
			g_settings.step_enable_invert = value1;
			break;
		case 5:
			g_settings.limits_invert_mask = value8;
			break;
		case 7:
			g_settings.control_invert_mask = value8;
			break;
		case 10:
			g_settings.status_report_mask = value8;
			break;
		case 12:
			g_settings.arc_tolerance = value;
			break;
		case 20:
			if(!g_settings.homing_enabled)
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
		case 27:
			g_settings.homing_offset = value;
			break;
		#if(AXIS_COUNT > 0)
		case 100:
			g_settings.step_per_mm[0] = value;
			break;
		case 110:
			g_settings.max_feed_rate[0] = value;
			break;
		case 120:
			g_settings.acceleration[0] = value;
			break;
		case 130:
			g_settings.max_distance[0] = value;
			break;
		#endif
		#if(AXIS_COUNT > 1)
		case 101:
			g_settings.step_per_mm[1] = value;
			break;
		case 111:
			g_settings.max_feed_rate[1] = value;
			break;
		case 121:
			g_settings.acceleration[1] = value;
			break;
		case 131:
			g_settings.max_distance[1] = value;
			break;
		#endif
		#if(AXIS_COUNT > 2)
		case 102:
			g_settings.step_per_mm[2] = value;
			break;
		case 112:
			g_settings.max_feed_rate[2] = value;
			break;
		case 122:
			g_settings.acceleration[2] = value;
			break;
		case 132:
			g_settings.max_distance[2] = value;
			break;
		#endif
		#if(AXIS_COUNT > 3)
		case 103:
			g_settings.step_per_mm[3] = value;
			break;
		case 113:
			g_settings.max_feed_rate[3] = value;
			break;
		case 123:
			g_settings.acceleration[3] = value;
			break;
		case 133:
			g_settings.max_distance[3] = value;
			break;
		#endif
		#if(AXIS_COUNT > 4)
		case 104:
			g_settings.step_per_mm[4] = value;
			break;
		case 114:
			g_settings.max_feed_rate[4] = value;
			break;
		case 124:
			g_settings.acceleration[4] = value;
			break;
		case 134:
			g_settings.max_distance[4] = value;
			break;
		#endif
		#if(AXIS_COUNT > 5)
		case 105:
			g_settings.step_per_mm[5] = value;
			break;
		case 115:
			g_settings.max_feed_rate[5] = value;
			break;
		case 125:
			g_settings.acceleration[5] = value;
			break;
		case 135:
			g_settings.max_distance[5] = value;
			break;
		#endif
		default:
			return STATUS_INVALID_STATEMENT;
	}
	
	settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t*)&g_settings, sizeof(settings_t));
	return result;
}

void settings_print()
{
	protocol_printf(MSG_SETTING_INT, 0, g_settings.max_step_rate);
	protocol_printf(MSG_SETTING_INT, 2, g_settings.step_invert_mask);
	protocol_printf(MSG_SETTING_INT, 3, g_settings.dir_invert_mask);
	protocol_printf(MSG_SETTING_INT, 4, g_settings.step_enable_invert);
	protocol_printf(MSG_SETTING_INT, 5, g_settings.limits_invert_mask);
	protocol_printf(MSG_SETTING_INT, 7, g_settings.control_invert_mask);
	protocol_printf(MSG_SETTING_INT, 10, g_settings.status_report_mask);
	protocol_printf(MSG_SETTING_FLT, 12, g_settings.arc_tolerance);
	protocol_printf(MSG_SETTING_INT, 20, g_settings.soft_limits_enabled);
	protocol_printf(MSG_SETTING_INT, 21, g_settings.hard_limits_enabled);
	protocol_printf(MSG_SETTING_INT, 22, g_settings.homing_enabled);
	protocol_printf(MSG_SETTING_INT, 23, g_settings.homing_dir_invert_mask);
	protocol_printf(MSG_SETTING_FLT, 24, g_settings.homing_slow_feed_rate);
	protocol_printf(MSG_SETTING_FLT, 25, g_settings.homing_fast_feed_rate);
	protocol_printf(MSG_SETTING_FLT, 27, g_settings.homing_offset);
	
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		protocol_printf(MSG_SETTING_FLT, 100 + i , g_settings.step_per_mm[i]);
	}
	
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		protocol_printf(MSG_SETTING_FLT, 110 + i , g_settings.max_feed_rate[i]);
	}
	
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		protocol_printf(MSG_SETTING_FLT, 120 + i , g_settings.acceleration[i]);
	}
	
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		protocol_printf(MSG_SETTING_FLT, 130 + i , g_settings.max_distance[i]);
	}
}
