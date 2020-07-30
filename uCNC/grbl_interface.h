/*
	Name: grbl_interface.h
	Description: µCNC definitions and standard codes used by Grbl.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07/12/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef GRBL_INTERFACE_H
#define GRBL_INTERFACE_H

#include "config.h"
#include "build.h"

//Defines Grbl realtime ascii codes
#define CMD_CODE_RESET 0x18
#define CMD_CODE_FEED_HOLD 0x21
#define CMD_CODE_REPORT 0x3F
#define CMD_CODE_CYCLE_START 0x7E
#define CMD_CODE_SAFETY_DOOR 0x84
#define CMD_CODE_JOG_CANCEL 0x85
#define CMD_CODE_FEED_100 0x90
#define CMD_CODE_FEED_INC_COARSE 0x91
#define CMD_CODE_FEED_DEC_COARSE 0x92
#define CMD_CODE_FEED_INC_FINE 0x93
#define CMD_CODE_FEED_DEC_FINE 0x94
#define CMD_CODE_RAPIDFEED_100 0x95
#define CMD_CODE_RAPIDFEED_OVR1 0x96
#define CMD_CODE_RAPIDFEED_OVR2 0x97
#define CMD_CODE_SPINDLE_100 0x99
#define CMD_CODE_SPINDLE_INC_COARSE 0x9A
#define CMD_CODE_SPINDLE_DEC_COARSE 0x9B
#define CMD_CODE_SPINDLE_INC_FINE 0x9C
#define CMD_CODE_SPINDLE_DEC_FINE 0x9D
#define CMD_CODE_SPINDLE_TOGGLE 0x9E
#define CMD_CODE_COOL_FLD_TOGGLE 0xA0
#define CMD_CODE_COOL_MST_TOGGLE 0xA1

// Define Grbl status codes. Valid values (0-255)
#define STATUS_OK 0
#define STATUS_EXPECTED_COMMAND_LETTER 1
#define STATUS_BAD_NUMBER_FORMAT 2
#define STATUS_INVALID_STATEMENT 3
#define STATUS_NEGATIVE_VALUE 4
#define STATUS_SETTING_DISABLED 5
#define STATUS_SETTING_STEP_PULSE_MIN 6 //
#define STATUS_SETTING_READ_FAIL 7
#define STATUS_IDLE_ERROR 8
#define STATUS_SYSTEM_GC_LOCK 9
#define STATUS_SOFT_LIMIT_ERROR 10
#define STATUS_OVERFLOW 11 //
#define STATUS_MAX_STEP_RATE_EXCEEDED 12
#define STATUS_CHECK_DOOR 13
#define STATUS_LINE_LENGTH_EXCEEDED 14 //
#define STATUS_TRAVEL_EXCEEDED 15
#define STATUS_INVALID_JOG_COMMAND 16
#define STATUS_SETTING_DISABLED_LASER 17 //
#define STATUS_GCODE_UNSUPPORTED_COMMAND 20
#define STATUS_GCODE_MODAL_GROUP_VIOLATION 21
#define STATUS_GCODE_UNDEFINED_FEED_RATE 22
#define STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER 23
#define STATUS_GCODE_AXIS_COMMAND_CONFLICT 24 //
#define STATUS_GCODE_WORD_REPEATED 25
#define STATUS_GCODE_NO_AXIS_WORDS 26
#define STATUS_GCODE_INVALID_LINE_NUMBER 27
#define STATUS_GCODE_VALUE_WORD_MISSING 28
#define STATUS_GCODE_UNSUPPORTED_COORD_SYS 29
#define STATUS_GCODE_G53_INVALID_MOTION_MODE 30
#define STATUS_GCODE_AXIS_WORDS_EXIST 31
#define STATUS_GCODE_NO_AXIS_WORDS_IN_PLANE 32
#define STATUS_GCODE_INVALID_TARGET 33
#define STATUS_GCODE_ARC_RADIUS_ERROR 34
#define STATUS_GCODE_NO_OFFSETS_IN_PLANE 35
#define STATUS_GCODE_UNUSED_WORDS 36 //
#define STATUS_GCODE_G43_DYNAMIC_AXIS_ERROR 37 //
#define STATUS_GCODE_MAX_VALUE_EXCEEDED 38 //
//additional codes
#define STATUS_BAD_COMMENT_FORMAT 39
#define STATUS_INVALID_TOOL 40
#define STATUS_FEED_NOT_SET 41
#define STATUS_CRITICAL_FAIL 255

//special Grbl system commands return codes
//These are not error codes but codes to print requested reports after parsing a grbl command
#define GRBL_SEND_SYSTEM_SETTINGS 128
#define GRBL_SEND_COORD_SYSTEM 129
#define GRBL_SEND_PARSER_MODES	130
#define GRBL_SEND_STARTUP_BLOCKS 131
#define GRBL_SEND_CHECKMODE_ON 132
#define GRBL_SEND_CHECKMODE_OFF 133
#define GRBL_SEND_SETTINGS_RESET 134
#define GRBL_UNLOCK 135
#define GRBL_HOME 136
#define GRBL_HELP 137


#define EXEC_ALARM_RESET					  0
// Grbl alarm codes. Valid values (1-255). Zero is reserved.
#define EXEC_ALARM_HARD_LIMIT                 1
#define EXEC_ALARM_SOFT_LIMIT                 2
#define EXEC_ALARM_ABORT_CYCLE                3
#define EXEC_ALARM_PROBE_FAIL_INITIAL         4
#define EXEC_ALARM_PROBE_FAIL_CONTACT         5
#define EXEC_ALARM_HOMING_FAIL_RESET          6
#define EXEC_ALARM_HOMING_FAIL_DOOR           7
#define EXEC_ALARM_HOMING_FAIL_PULLOFF        8
#define EXEC_ALARM_HOMING_FAIL_APPROACH       9
#define EXEC_ALARM_HOMING_FAIL_DUAL_APPROACH  10
#define EXEC_ALARM_HOMING_FAIL_LIMIT_ACTIVE   11

//formated messages
#define MSG_OK __romstr__("ok\r\n")
#define MSG_ERROR __romstr__("error:")
#define MSG_ALARM __romstr__("ALARM:")
#define MSG_ECHO __romstr__("[echo:")
#define MSG_STARTUP_START "uCNC "
#define MSG_STARTUP_END " ['$' for help]\r\n"
#define MSG_STARTUP __romstr__(MSG_STARTUP_START VERSION MSG_STARTUP_END)
#define MSG_HELP __romstr__("[HLP:$$ $# $G $I $N $x=val $Nx=line $J=line $C $X $H ~ ! ? ctrl-x]\r\n")

//Non query feedback messages
#define MSG_START __romstr__("[MSG:")
#define MSG_END __romstr__("]\r\n")
#define MSG_FEEDBACK_1 __romstr__("Reset to continue")
#define MSG_FEEDBACK_2 __romstr__("'$H'|'$X' to unlock")
#define MSG_FEEDBACK_3 __romstr__("Caution: Unlocked")
#define MSG_FEEDBACK_4 __romstr__("Enabled")
#define MSG_FEEDBACK_5 __romstr__("Disabled")
#define MSG_FEEDBACK_6 __romstr__("Check Door")
#define MSG_FEEDBACK_7 __romstr__("Check Limits")
#define MSG_FEEDBACK_8 __romstr__("Pgm End")
#define MSG_FEEDBACK_9 __romstr__("Restoring defaults")
#define MSG_FEEDBACK_10 __romstr__("Restoring spindle")
//#define MSG_FEEDBACK_11 __romstr__("Sleeping") not implemented
/*NEW*/
#define MSG_FEEDBACK_12 __romstr__("Check Emergency stop")

//#define MSG_INT "%d"
//#define MSG_FLT "%0.3f"
//#define MSG_FLT_IMPERIAL "%0.5f"

#endif
