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
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "config.h"
#include "defaults.h"
#include "settings.h"
#include "mcu.h"
#include "serial.h"
#include "grbl_interface.h"
#include "protocol.h"
#include "parser.h"
#include "cnc.h"

//if settings struct is changed this version has to change too
#define SETTINGS_VERSION "V01"

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
    0x46, 0x4f, 0x54, 0x5d, 0x62, 0x6b, 0x70, 0x79
};
#else
static uint8_t crc7(uint8_t c, uint8_t crc)
{
	uint8_t i = 8;
    for (;;)
    {
        if (crc & 0x80)
        {
            crc^=0x89;
        }
        if(!--i)
        {
        	return(crc);
		}
        crc <<= 1;
    }
}
#endif

const settings_t __rom__ default_settings =
{
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
    .probe_invert_mask = DEFAULT_PROBE_INV_MASK,
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
    .report_inches = DEFAULT_REPORT_INCHES,
    .soft_limits_enabled = DEFAULT_SOFT_LIMITS_ENABLED,
    .hard_limits_enabled = DEFAULT_HARD_LIMITS_ENABLED,
    .homing_enabled = DEFAULT_HOMING_ENABLED,
    .spindle_max_rpm = DEFAULT_SPINDLE_MAX_RPM,
    .spindle_min_rpm = DEFAULT_SPINDLE_MIN_RPM
};

//static uint8_t settings_crc;

void settings_init(void)
{
    const char version[3] = SETTINGS_VERSION;
	uint8_t error = settings_load(SETTINGS_ADDRESS_OFFSET, (uint8_t*) &g_settings, (uint8_t)sizeof(settings_t));

    if(!error)
    {
        for(uint8_t i = 0; i < 3; i++)
        {
            if(g_settings.version[i] != version[i])
            {
                error = 1; //just set an error
                break;
            }
        }
    }
	
    if(error)
    {
        settings_reset();
        protocol_send_error(STATUS_SETTING_READ_FAIL);
        protocol_send_ucnc_settings();
    }
}

uint8_t settings_load(uint16_t address, uint8_t* __ptr, uint8_t size)
{
    uint8_t crc = 0;

    while (size)
    {
        size--;
        uint8_t value = mcu_eeprom_getc(address++);
#ifndef CRC_WITHOUT_LOOKUP_TABLE
        crc = rom_read_byte(&crc7_table[value ^ crc]);
#else
        crc = crc7(value, crc);
#endif
        *(__ptr++) = value;
    }

    //fix step invert mask to match mirror step pins
    #ifdef ENABLE_DUAL_DRIVE_AXIS
    #ifdef DUAL_DRIVE_AXIS0
    g_settings.step_invert_mask |= (g_settings.step_invert_mask & STEP_DUAL0) ? 64 : 0;
    #endif
    #ifdef DUAL_DRIVE_AXIS1
    g_settings.step_invert_mask |= (g_settings.step_invert_mask & STEP_DUAL1) ? 128 : 0;
    #endif
    #endif

    return (crc ^ mcu_eeprom_getc(address));
}

void settings_reset(void)
{
	const uint8_t size = sizeof(settings_t);
    uint8_t* __ptr = (uint8_t*)&g_settings;
    rom_memcpy(&g_settings, &default_settings, size);
    settings_save(SETTINGS_ADDRESS_OFFSET, (const uint8_t*)&g_settings, size);

    //fix step invert mask to match mirror step pins
    #ifdef ENABLE_DUAL_DRIVE_AXIS
    #ifdef DUAL_DRIVE_AXIS0
    g_settings.step_invert_mask |= (g_settings.step_invert_mask & STEP_DUAL0) ? 64 : 0;
    #endif
    #ifdef DUAL_DRIVE_AXIS1
    g_settings.step_invert_mask |= (g_settings.step_invert_mask & STEP_DUAL1) ? 128 : 0;
    #endif
    #endif
}

void settings_save(uint16_t address, const uint8_t* __ptr, uint8_t size)
{
    uint8_t crc = 0;

    #ifdef ENABLE_DUAL_DRIVE_AXIS
    uint8_t temp_step_inv_mask = g_settings.step_invert_mask;
    g_settings.step_invert_mask &= 63; //sets cloned axis to 0
    #endif

    while (size)
    {
    	if(cnc_get_exec_state(EXEC_RUN))
    	{
    		cnc_doevents(); //updates buffer before cycling
		}
    	
        size--;
#ifndef CRC_WITHOUT_LOOKUP_TABLE
        crc = rom_read_byte(&crc7_table[*__ptr ^ crc]);
#else
        crc = crc7(*__ptr, crc);
#endif
        mcu_eeprom_putc(address++, *(__ptr++));
    }

    mcu_eeprom_putc(address, crc);

    #ifdef ENABLE_DUAL_DRIVE_AXIS
    g_settings.step_invert_mask = temp_step_inv_mask; //restores setting
    #endif
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
            if(value > F_STEP_MAX)
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
        case 6:
            g_settings.probe_invert_mask = value1;
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
        case 13:
            g_settings.report_inches = value;
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
        case 30:
            g_settings.spindle_max_rpm = value;
            break;
        case 31:
            g_settings.spindle_min_rpm = value;
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

    settings_save(SETTINGS_ADDRESS_OFFSET, (const uint8_t*)&g_settings, (uint8_t)sizeof(settings_t));

    //fix step invert mask to match mirror step pins
    #ifdef ENABLE_DUAL_DRIVE_AXIS
    #ifdef DUAL_DRIVE_AXIS0
    g_settings.step_invert_mask |= (g_settings.step_invert_mask & STEP_DUAL0) ? 64 : 0;
    #endif
    #ifdef DUAL_DRIVE_AXIS1
    g_settings.step_invert_mask |= (g_settings.step_invert_mask & STEP_DUAL1) ? 128 : 0;
    #endif
    #endif
    
    return result;
}

void settings_erase(uint16_t address, uint8_t size)
{
	while(size)
	{
		if(cnc_get_exec_state(EXEC_RUN))
    	{
    		cnc_doevents(); //updates buffer before cycling
		}
		mcu_eeprom_putc(address++, EOL);
		size--;
	}

	//erase crc byte that is next to data
	mcu_eeprom_putc(address, EOL);
}

bool settings_check_startup_gcode(uint16_t address)
{
	uint8_t size = (RX_BUFFER_SIZE - 1); //defined in serial.h
	uint8_t crc = 0;
	unsigned char c;
	uint16_t cmd_address = address;
	
	//pre-checks command valid crc
    do
    {
        c = mcu_eeprom_getc(cmd_address++);
        crc = rom_read_byte(&crc7_table[c ^ crc]);
        if(!c)
        {
        	break;
		}
        size--;
    }while(size);
    
    if(crc ^ mcu_eeprom_getc(cmd_address))
    {
    	serial_putc('>');
    	serial_putc(':');
    	protocol_send_error(STATUS_SETTING_READ_FAIL);
    	settings_erase(address, 1);
    	return false;
	}

    return true;
}

void settings_save_startup_gcode(uint16_t address)
{
	uint8_t size = (RX_BUFFER_SIZE - 1);
	uint8_t crc = 0;
	unsigned char c;
    do
    {
        c = serial_getc();
        crc = rom_read_byte(&crc7_table[c ^ crc]);
        mcu_eeprom_putc(address++, (uint8_t)c);
        size--;
    }while(size && c);
    
    mcu_eeprom_putc(address, crc);
}
