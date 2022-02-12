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

//if settings struct is changed this version should change too
#define SETTINGS_VERSION "V04"

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
    for (;;)
    {
        if (crc & 0x80)
        {
            crc ^= 0x89;
        }
        if (!--i)
        {
            return (crc);
        }
        crc <<= 1;
    }
}
#endif

const settings_t __rom__ default_settings =
    {
        .version = SETTINGS_VERSION,
#ifdef AXIS_X
        .max_distance[AXIS_X] = DEFAULT_X_MAX_DIST,
#endif
#ifdef AXIS_Y
        .max_distance[AXIS_Y] = DEFAULT_Y_MAX_DIST,
#endif
#ifdef AXIS_Z
        .max_distance[AXIS_Z] = DEFAULT_Z_MAX_DIST,
#endif
#ifdef AXIS_A
        .max_distance[AXIS_A] = DEFAULT_A_MAX_DIST,
#endif
#ifdef AXIS_B
        .max_distance[AXIS_B] = DEFAULT_B_MAX_DIST,
#endif
#ifdef AXIS_C
        .max_distance[AXIS_C] = DEFAULT_C_MAX_DIST,
#endif

#ifdef ENABLE_SKEW_COMPENSATION
        .skew_xy_factor = 0,
#ifndef SKEW_COMPENSATION_XY_ONLY
        .skew_xz_factor = 0,
        .skew_yz_factor = 0,
#endif
#endif

#if STEPPER_COUNT > 0
        .step_per_mm[0] = DEFAULT_0_STEP_PER_MM,
        .max_feed_rate[0] = DEFAULT_0_MAX_FEED,
        .acceleration[0] = DEFAULT_0_ACCEL,
#ifdef ENABLE_BACKLASH_COMPENSATION
        .backlash_steps[0] = 0,
#endif
#endif
#if STEPPER_COUNT > 1
        .step_per_mm[1] = DEFAULT_1_STEP_PER_MM,
        .max_feed_rate[1] = DEFAULT_1_MAX_FEED,
        .acceleration[1] = DEFAULT_1_ACCEL,
#ifdef ENABLE_BACKLASH_COMPENSATION
        .backlash_steps[1] = 0,
#endif
#endif
#if STEPPER_COUNT > 2
        .step_per_mm[2] = DEFAULT_2_STEP_PER_MM,
        .max_feed_rate[2] = DEFAULT_2_MAX_FEED,
        .acceleration[2] = DEFAULT_2_ACCEL,
#ifdef ENABLE_BACKLASH_COMPENSATION
        .backlash_steps[2] = 0,
#endif
#endif
#if STEPPER_COUNT > 3
        .step_per_mm[3] = DEFAULT_3_STEP_PER_MM,
        .max_feed_rate[3] = DEFAULT_3_MAX_FEED,
        .acceleration[3] = DEFAULT_3_ACCEL,
#ifdef ENABLE_BACKLASH_COMPENSATION
        .backlash_steps[3] = 0,
#endif
#endif
#if STEPPER_COUNT > 4
        .step_per_mm[4] = DEFAULT_4_STEP_PER_MM,
        .max_feed_rate[4] = DEFAULT_4_MAX_FEED,
        .acceleration[4] = DEFAULT_4_ACCEL,
#ifdef ENABLE_BACKLASH_COMPENSATION
        .backlash_steps[4] = 0,
#endif
#endif
#if STEPPER_COUNT > 5
        .step_per_mm[5] = DEFAULT_5_STEP_PER_MM,
        .max_feed_rate[5] = DEFAULT_5_MAX_FEED,
        .acceleration[5] = DEFAULT_5_ACCEL,
#ifdef ENABLE_BACKLASH_COMPENSATION
        .backlash_steps[5] = 0,
#endif
#endif
        .laser_mode = 0,
#if PID_CONTROLLERS > 0
        .pid_gain[0][0] = 0,
        .pid_gain[0][1] = 0,
        .pid_gain[0][2] = 0,
#endif
#if PID_CONTROLLERS > 1
        .pid_gain[1][0] = 0,
        .pid_gain[1][1] = 0,
        .pid_gain[1][2] = 0,
#endif
#if PID_CONTROLLERS > 2
        .pid_gain[2][0] = 0,
        .pid_gain[2][1] = 0,
        .pid_gain[2][2] = 0,
#endif
#if PID_CONTROLLERS > 3
        .pid_gain[3][0] = 0,
        .pid_gain[3][1] = 0,
        .pid_gain[3][2] = 0,
#endif
#if PID_CONTROLLERS > 4
        .pid_gain[4][0] = 0,
        .pid_gain[4][1] = 0,
        .pid_gain[4][2] = 0,
#endif
#if PID_CONTROLLERS > 5
        .pid_gain[5][0] = 0,
        .pid_gain[5][1] = 0,
        .pid_gain[5][2] = 0,
#endif
#if PID_CONTROLLERS > 6
        .pid_gain[6][0] = 0,
        .pid_gain[6][1] = 0,
        .pid_gain[6][2] = 0,
#endif
#if PID_CONTROLLERS > 7
        .pid_gain[7][0] = 0,
        .pid_gain[7][1] = 0,
        .pid_gain[7][2] = 0,
#endif
#if TOOL_COUNT > 0
        .default_tool = DEFAULT_STARTUP_TOOL,
        .tool_length_offset[0] = 0,
#endif
#if TOOL_COUNT > 1
        .tool_length_offset[1] = 0,
#endif
#if TOOL_COUNT > 2
        .tool_length_offset[2] = 0,
#endif
#if TOOL_COUNT > 3
        .tool_length_offset[3] = 0,
#endif
#if TOOL_COUNT > 4
        .tool_length_offset[4] = 0,
#endif
#if TOOL_COUNT > 5
        .tool_length_offset[5] = 0,
#endif
#if TOOL_COUNT > 6
        .tool_length_offset[6] = 0,
#endif
#if TOOL_COUNT > 7
        .tool_length_offset[7] = 0,
#endif
#if TOOL_COUNT > 8
        .tool_length_offset[8] = 0,
#endif
#if TOOL_COUNT > 9
        .tool_length_offset[9] = 0,
#endif
#if TOOL_COUNT > 10
        .tool_length_offset[10] = 0,
#endif
#if TOOL_COUNT > 11
        .tool_length_offset[11] = 0,
#endif
#if TOOL_COUNT > 12
        .tool_length_offset[12] = 0,
#endif
#if TOOL_COUNT > 13
        .tool_length_offset[13] = 0,
#endif
#if TOOL_COUNT > 14
        .tool_length_offset[14] = 0,
#endif
#if TOOL_COUNT > 15
        .tool_length_offset[15] = 0,
#endif

        .step_enable_invert = DEFAULT_STEP_ENA_INV,
        .step_invert_mask = DEFAULT_STEP_INV_MASK,
        .dir_invert_mask = DEFAULT_DIR_INV_MASK,
        .probe_invert_mask = DEFAULT_PROBE_INV_MASK,
        .homing_dir_invert_mask = DEFAULT_HOMING_DIR_INV_MASK,
        .homing_fast_feed_rate = DEFAULT_HOMING_FAST,
        .homing_slow_feed_rate = DEFAULT_HOMING_SLOW,
        .homing_offset = DEFAULT_HOMING_OFFSET,
        .g64_angle_factor = DEFAULT_G64_FACTOR,
        .arc_tolerance = DEFAULT_ARC_TOLERANCE,
        .limits_invert_mask = DEFAULT_LIMIT_INV_MASK,
        .status_report_mask = DEFAULT_STATUS_MASK,
        .control_invert_mask = DEFAULT_CONTROL_INV_MASK,
        .max_step_rate = DEFAULT_MAX_STEP_RATE,
        .report_inches = DEFAULT_REPORT_INCHES,
        .soft_limits_enabled = DEFAULT_SOFT_LIMITS_ENABLED,
        .hard_limits_enabled = DEFAULT_HARD_LIMITS_ENABLED,
        .homing_enabled = DEFAULT_HOMING_ENABLED,
        .spindle_max_rpm = DEFAULT_SPINDLE_MAX_RPM,
        .spindle_min_rpm = DEFAULT_SPINDLE_MIN_RPM,
        .debounce_ms = DEFAULT_DEBOUNCE_MS};

//static uint8_t settings_crc;

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
                error = 1; //just set an error
                break;
            }
        }
    }

    if (error)
    {
        settings_reset();
        protocol_send_error(STATUS_SETTING_READ_FAIL);
        protocol_send_cnc_settings();
    }
}

uint8_t settings_load(uint16_t address, uint8_t *__ptr, uint8_t size)
{
    uint8_t crc = 0;

    while (size)
    {
        size--;
        uint8_t value = mcu_eeprom_getc(address++);
        crc = crc7(value, crc);
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
    rom_memcpy(&g_settings, &default_settings, sizeof(settings_t));
#ifndef ENABLE_SETTING_EXTRA_CMDS
    settings_save(SETTINGS_ADDRESS_OFFSET, (const uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
#endif

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

void settings_save(uint16_t address, const uint8_t *__ptr, uint8_t size)
{
    uint8_t crc = 0;

#ifdef ENABLE_DUAL_DRIVE_AXIS
    uint8_t temp_step_inv_mask = g_settings.step_invert_mask;
    g_settings.step_invert_mask &= 63; //sets cloned axis to 0
#endif

    while (size)
    {
        if (cnc_get_exec_state(EXEC_RUN))
        {
            cnc_dotasks(); //updates buffer before cycling
        }

        size--;
        crc = crc7(*__ptr, crc);
        mcu_eeprom_putc(address++, *(__ptr++));
    }

    mcu_eeprom_putc(address, crc);
    mcu_eeprom_flush();

#ifdef ENABLE_DUAL_DRIVE_AXIS
    g_settings.step_invert_mask = temp_step_inv_mask; //restores setting
#endif
}

uint8_t settings_change(uint8_t setting, float value)
{
    uint8_t result = 0;
    uint8_t value8 = (uint8_t)value;
    uint16_t value16 = (uint16_t)value;
    bool value1 = (value != 0);

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
        g_settings.spindle_max_rpm = value;
        break;
    case 31:
        g_settings.spindle_min_rpm = value;
        break;
    case 32:
        g_settings.laser_mode = value8;
        break;
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
    case 40:
        g_settings.default_tool = CLAMP(0, value8, TOOL_COUNT);
        break;
#endif
    default:
        if (setting >= 100 && setting < (100 + STEPPER_COUNT))
        {
            setting -= 100;
            g_settings.step_per_mm[setting] = value;
        }
        else if (setting >= 110 && setting < (110 + STEPPER_COUNT))
        {
            setting -= 110;
            g_settings.max_feed_rate[setting] = value;
        }
        else if (setting >= 120 && setting < (120 + STEPPER_COUNT))
        {
            setting -= 120;
            g_settings.acceleration[setting] = value;
        }
        else if (setting >= 130 && setting < (130 + AXIS_COUNT))
        {
            setting -= 130;
            g_settings.max_distance[setting] = value;
        }
        else if (setting >= 130 && setting < (130 + AXIS_COUNT))
        {
            setting -= 130;
            g_settings.max_distance[setting] = value;
        }
#ifdef ENABLE_BACKLASH_COMPENSATION
        else if (setting >= 140 && setting < (140 + STEPPER_COUNT))
        {
            setting -= 140;
            g_settings.backlash_steps[setting] = value16;
        }
#endif
#if PID_CONTROLLERS > 0
        //kp ki and kd 0 -> 150, 151, 152
        //kp ki and kd 1 -> 154, 155, 156, etc...
        else if (setting >= 150 && setting < (150 + (4 * PID_CONTROLLERS)))
        {
            uint8_t k = setting & 0x03;
            uint8_t pid = (setting >> 2) & 0x03;
            //3 is invalid index
            if (k == 0x03)
            {
                return STATUS_INVALID_STATEMENT;
            }
            g_settings.pid_gain[pid][k] = value;
        }
#endif
#if TOOL_COUNT > 0
        else if (setting > 40 && setting <= 56)
        {
            setting -= 41;
            g_settings.tool_length_offset[setting] = value;
        }
#endif
        else
        {
            return STATUS_INVALID_STATEMENT;
        }
        break;
    }

#ifndef ENABLE_SETTING_EXTRA_CMDS
    settings_save(SETTINGS_ADDRESS_OFFSET, (const uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
#endif

#if PID_CONTROLLERS > 0
    pid_init();
#endif

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
    while (size)
    {
        if (cnc_get_exec_state(EXEC_RUN))
        {
            cnc_dotasks(); //updates buffer before cycling
        }
        mcu_eeprom_putc(address++, EOL);
        size--;
    }

    //erase crc byte that is next to data
    mcu_eeprom_putc(address, EOL);
    mcu_eeprom_flush();
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
        crc = crc7(c, crc);
        mcu_eeprom_putc(address++, (uint8_t)c);
        size--;
    } while (size && c);

    mcu_eeprom_putc(address, crc);
}
