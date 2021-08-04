/*
	Name: parser.c (version 2)
	Description: Parses Grbl system commands and RS274NGC (GCode) commands
        The RS274NGC parser tries to follow the standard document version 3 as close as possible.
        The parsing is done in 3 steps:
            - tokenization; Converts the command string to a structure with GCode parameters
            - Validation; Validates the command by checking all the parameters (part 3.5 - 3.7 of the document)
            - Execution; Executes the command by the orther set in part 3.8 of the document.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 31/07/2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "cnc.h"
#include "interface/grbl_interface.h"
#include "interface/settings.h"
#include "interface/serial.h"
#include "interface/protocol.h"
#include "core/planner.h"
#include "core/motion_control.h"
#include "core/io_control.h"
#include "core/interpolator.h"
#include "core/parser.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <float.h>

//group masks
#define GCODE_GROUP_MOTION 0x0001
#define GCODE_GROUP_PLANE 0x0002
#define GCODE_GROUP_DISTANCE 0x0004
#define GCODE_GROUP_FEEDRATE 0x0008
#define GCODE_GROUP_UNITS 0x0010
#define GCODE_GROUP_CUTTERRAD 0x0020
#define GCODE_GROUP_TOOLLENGTH 0x0040
#define GCODE_GROUP_RETURNMODE 0x0080
#define GCODE_GROUP_COORDSYS 0x0100
#define GCODE_GROUP_PATH 0x0200
#define GCODE_GROUP_STOPPING 0x0400
#define GCODE_GROUP_TOOLCHANGE 0x0800
#define GCODE_GROUP_SPINDLE 0x1000
#define GCODE_GROUP_COOLANT 0x2000
#define GCODE_GROUP_ENABLEOVER 0x4000
#define GCODE_GROUP_NONMODAL 0x8000

//word masks
#define GCODE_WORD_X 0x0001
#define GCODE_WORD_Y 0x0002
#define GCODE_WORD_Z 0x0004
#define GCODE_WORD_A 0x0008
#define GCODE_WORD_B 0x0010
#define GCODE_WORD_C 0x0020
#define GCODE_WORD_D 0x0040
#define GCODE_WORD_F 0x0080
#define GCODE_WORD_I 0x0100 //matches X axis bit
#define GCODE_WORD_J 0x0200 //matches Y axis bit
#define GCODE_WORD_K 0x0400 //matches Z axis bit
#define GCODE_WORD_L 0x0800
#define GCODE_WORD_P 0x1000
#define GCODE_WORD_R 0x2000
#define GCODE_WORD_S 0x4000
#define GCODE_WORD_T 0x8000
    //H and Q are related to unsupported commands

#if (defined(AXIS_B) | defined(AXIS_C) | defined(GCODE_PROCESS_LINE_NUMBERS))
#define GCODE_WORDS_EXTENDED
#endif

#define GCODE_JOG_INVALID_WORDS (GCODE_WORD_I | GCODE_WORD_J | GCODE_WORD_K | GCODE_WORD_D | GCODE_WORD_L | GCODE_WORD_P | GCODE_WORD_R | GCODE_WORD_T | GCODE_WORD_S)
#define GCODE_ALL_AXIS (GCODE_WORD_X | GCODE_WORD_Y | GCODE_WORD_Z | GCODE_WORD_A | GCODE_WORD_B | GCODE_WORD_C)
#define GCODE_XYPLANE_AXIS (GCODE_WORD_X | GCODE_WORD_Y)
#define GCODE_XZPLANE_AXIS (GCODE_WORD_X | GCODE_WORD_Z)
#define GCODE_YZPLANE_AXIS (GCODE_WORD_Y | GCODE_WORD_Z)
#define GCODE_IJPLANE_AXIS (GCODE_XYPLANE_AXIS << 8)
#define GCODE_IKPLANE_AXIS (GCODE_YZPLANE_AXIS << 8)
#define GCODE_JKPLANE_AXIS (GCODE_YZPLANE_AXIS << 8)
#define GCODE_IJK_AXIS (GCODE_WORD_I | GCODE_WORD_J | GCODE_WORD_K)

#define G0 0
#define G1 1
#define G2 2
#define G3 3
#define G38_2 4
#define G38_3 5
#define G38_4 6
#define G38_5 7
#define G80 8
#define G17 0
#define G18 1
#define G19 2
#define G90 0
#define G91 1
#define G93 0
#define G94 1
#define G20 0
#define G21 1
#define G40 0
#define G41 1
#define G42 2
#define G43_1 0
#define G49 1
#define G98 0
#define G99 1
#define G54 0
#define G55 1
#define G56 2
#define G57 3
#define G58 4
#define G59 5
#define G59_1 6
#define G59_2 7
#define G59_3 8
#define G61 0
#define G61_1 1
#define G64 2
#define G4 1
#define G10 2
#define G28 3
#define G30 4
#define G53 6
#define G92 10
#define G92_1 11
#define G92_2 12
#define G92_3 13

#define M0 1
#define M1 2
#define M2 3
#define M30 4
#define M60 5
#define M3 1
#define M4 2
#define M5 0
#define M6 0
#define M7 MIST_MASK
#define M8 COOLANT_MASK
#define M9 0
#define M48 1
#define M49 0

#define PARSER_PARAM_SIZE (sizeof(float) * AXIS_COUNT)   //parser parameters array size
#define PARSER_PARAM_ADDR_OFFSET (PARSER_PARAM_SIZE + 1) //parser parameters array size + 1 crc byte
#define G28HOME COORD_SYS_COUNT                          //G28 index
#define G30HOME COORD_SYS_COUNT + 1                      //G30 index
#define G92OFFSET COORD_SYS_COUNT + 2                    //G92 index

#define PARSER_CORDSYS_ADDRESS SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET                                //1st coordinate system offset eeprom address (G54)
#define G28ADDRESS (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (PARSER_PARAM_ADDR_OFFSET * G28HOME))   //G28 coordinate offset eeprom address
#define G30ADDRESS (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (PARSER_PARAM_ADDR_OFFSET * G30HOME))   //G28 coordinate offset eeprom address
#define G92ADDRESS (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (PARSER_PARAM_ADDR_OFFSET * G92OFFSET)) //G92 coordinate offset eeprom address

#define NUMBER_UNDEF 0
#define NUMBER_OK 0x20
#define NUMBER_ISFLOAT 0x40
#define NUMBER_ISNEGATIVE 0x80

    //32bytes in total
    typedef struct
    {
        //1byte
        uint8_t motion : 5;
        uint8_t coord_system : 3;
        //1byte
        uint8_t nonmodal : 4; //reset to 0 in every line (non persistent)
        uint8_t plane : 2;
        uint8_t path_mode : 2;
        //1byte
        uint8_t cutter_radius_compensation : 2;
        uint8_t distance_mode : 1;
        uint8_t feedrate_mode : 1;
        uint8_t units : 1;
        uint8_t tool_length_offset : 1;
        uint8_t return_mode : 1;
        uint8_t feed_speed_override : 1;
        //1byte
        uint8_t tool_change : 1;
        uint8_t stopping : 3;
#ifdef USE_SPINDLE
        uint8_t spindle_turning : 2;
#else
    uint8_t : 2; //unused
#endif
#ifdef USE_COOLANT
        uint8_t coolant : 2;
#else
    uint8_t : 2; //unused
#endif
    } parser_groups_t;

    typedef struct
    {
        float tool_length_offset;
        uint8_t coord_system_index;
        float coord_system_offset[AXIS_COUNT];
        float g92_offset[AXIS_COUNT];
        float last_probe_position[AXIS_COUNT];
        uint8_t last_probe_ok;
    } parser_parameters_t;

    typedef struct
    {
        float xyzabc[AXIS_COUNT];
        float ijk[3];
        float d;
        float f;
        float p;
        float r;
#ifdef GCODE_PROCESS_LINE_NUMBERS
        uint32_t n;
#endif
#ifdef USE_SPINDLE
        int16_t s;
#endif
        uint8_t t;
        uint8_t l;
    } parser_words_t;

    typedef struct
    {
        uint16_t groups;
        uint16_t words;
        bool group_0_1_useaxis;
    } parser_cmd_explicit_t;

    typedef struct
    {
        parser_groups_t groups;
        float feedrate;
        uint8_t tool_index;
#ifdef USE_SPINDLE
        int16_t spindle;
#endif
#ifdef GCODE_PROCESS_LINE_NUMBERS
        uint32_t line;
#endif
    } parser_state_t;

    static parser_state_t parser_state;
    static parser_parameters_t parser_parameters;
    static uint8_t parser_wco_counter;
    static float g92permanentoffset[AXIS_COUNT];

    static unsigned char parser_get_next_preprocessed(bool peek);
    FORCEINLINE static uint8_t parser_get_comment(void);
    static uint8_t parser_get_float(float *value);
    FORCEINLINE static uint8_t parser_eat_next_char(unsigned char c);
    FORCEINLINE static uint8_t parser_get_token(unsigned char *word, float *value);
    FORCEINLINE static uint8_t parser_gcode_word(uint8_t code, uint8_t mantissa, parser_state_t *new_state, parser_cmd_explicit_t *cmd);
    FORCEINLINE static uint8_t parser_mcode_word(uint8_t code, uint8_t mantissa, parser_state_t *new_state, parser_cmd_explicit_t *cmd);
    FORCEINLINE static uint8_t parser_letter_word(unsigned char c, float value, uint8_t mantissa, parser_words_t *words, parser_cmd_explicit_t *cmd);
    static uint8_t parse_grbl_exec_code(uint8_t code);
    static uint8_t parser_fetch_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
    static uint8_t parser_validate_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
    FORCEINLINE static uint8_t parser_exec_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
    static uint8_t parser_grbl_command(void);
    FORCEINLINE static uint8_t parser_gcode_command(void);
    FORCEINLINE static void parser_discard_command(void);
    static void parser_reset();

    /*
	Initializes the gcode parser
*/
    void parser_init(void)
    {
#ifdef FORCE_GLOBALS_TO_0
        memset(&parser_state, 0, sizeof(parser_state_t));
        memset(&parser_parameters, 0, sizeof(parser_parameters_t));
#endif
        parser_parameters_load();
        parser_reset();
    }

    uint8_t parser_read_command(void)
    {
        uint8_t error = STATUS_OK;
        unsigned char c = serial_peek();

        if (c == '$')
        {
            error = parser_grbl_command();

            if (error >= GRBL_SYSTEM_CMD)
            {
                if (error != GRBL_JOG_CMD)
                {
                    return parse_grbl_exec_code(error);
                }
            }
            else
            {

                return error;
            }
        }

        if (error == GRBL_JOG_CMD)
        {
            if (cnc_get_exec_state(~EXEC_JOG))
            {
                return STATUS_SYSTEM_GC_LOCK;
            }
        }
        else if (cnc_get_exec_state(~(EXEC_RUN | EXEC_HOLD | EXEC_RESUMING)))
        {
            return STATUS_SYSTEM_GC_LOCK;
        }

        return parser_gcode_command();
    }

    void parser_get_modes(uint8_t *modalgroups, uint16_t *feed, uint16_t *spindle, uint8_t *coolant)
    {
        modalgroups[0] = (parser_state.groups.motion < 8) ? parser_state.groups.motion : (72 + parser_state.groups.motion);
        modalgroups[1] = parser_state.groups.plane + 17;
        modalgroups[2] = parser_state.groups.distance_mode + 90;
        modalgroups[3] = parser_state.groups.feedrate_mode + 93;
        modalgroups[4] = parser_state.groups.units + 20;
        modalgroups[5] = ((parser_state.groups.tool_length_offset == G49) ? 49 : 43);
        modalgroups[6] = parser_state.groups.coord_system + 54;
        modalgroups[7] = parser_state.groups.path_mode + 61;
#ifdef USE_SPINDLE
        modalgroups[8] = ((parser_state.groups.spindle_turning == M5) ? 5 : (2 + parser_state.groups.spindle_turning));
        *spindle = (uint16_t)ABS(parser_state.spindle);
#else
    modalgroups[8] = 5;
#endif
#ifdef USE_COOLANT
        *coolant = parser_state.groups.coolant;
        modalgroups[9] = (parser_state.groups.coolant == M9) ? 9 : MIN(parser_state.groups.coolant + 6, 8);
#else

    modalgroups[9] = 9;
#endif
        modalgroups[10] = 49 - parser_state.groups.feed_speed_override;
#ifdef USE_TOOL_CHANGER
        modalgroups[11] = parser_state.tool_index;
#else
    modalgroups[11] = 1;
#endif
        *feed = (uint16_t)parser_state.feedrate;
    }

    void parser_get_coordsys(uint8_t system_num, float *axis)
    {
        switch (system_num)
        {
        case 255:
            memcpy(axis, parser_parameters.last_probe_position, sizeof(parser_parameters.last_probe_position));
            break;
        case 254:
            memcpy(axis, (float *)&parser_parameters.tool_length_offset, sizeof(float));
            break;
        case 28:
            settings_load(G28ADDRESS, (uint8_t *)axis, PARSER_PARAM_SIZE);
            break;
        case 30:
            settings_load(G30ADDRESS, (uint8_t *)axis, PARSER_PARAM_SIZE);
            break;
        case 92:
            memcpy(axis, parser_parameters.g92_offset, sizeof(parser_parameters.g92_offset));
            break;
        default:
            settings_load(PARSER_CORDSYS_ADDRESS + (system_num * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)axis, PARSER_PARAM_SIZE);
            break;
        }
    }

    uint8_t parser_get_probe_result(void)
    {
        return parser_parameters.last_probe_ok;
    }

    void parser_parameters_reset(void)
    {
        //erase all parameters for G54..G59.x coordinate systems
        memset(parser_parameters.coord_system_offset, 0, sizeof(parser_parameters.coord_system_offset));
        for (uint8_t i = 0; i < COORD_SYS_COUNT; i++)
        {
            settings_erase(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (i * PARSER_PARAM_ADDR_OFFSET), PARSER_PARAM_SIZE);
        }

        //erase G92
        settings_erase(G92ADDRESS, PARSER_PARAM_SIZE);
    }

    bool parser_get_wco(float *axis)
    {
        if (!parser_wco_counter)
        {
            for (uint8_t i = AXIS_COUNT; i != 0;)
            {
                i--;
                axis[i] = parser_parameters.g92_offset[i] + parser_parameters.coord_system_offset[i];
            }

#ifdef AXIS_Z
//axis[AXIS_Z] += parser_parameters.tool_length_offset;
#endif
            parser_wco_counter = STATUS_WCO_REPORT_MIN_FREQUENCY;
            return true;
        }

        parser_wco_counter--;
        return false;
    }

    void parser_sync_probe(void)
    {
        itp_get_rt_position(parser_parameters.last_probe_position);
    }

    static uint8_t parser_grbl_command(void)
    {
        //if not IDLE
        if (cnc_get_exec_state(EXEC_RUN))
        {
            parser_discard_command();
            return STATUS_IDLE_ERROR;
        }

        serial_getc(); //eat $
        unsigned char c = serial_getc();
        uint16_t block_address;
        uint8_t error = STATUS_OK;
        switch (c)
        {
        case '$':
            return (!parser_eat_next_char(EOL)) ? GRBL_SEND_SYSTEM_SETTINGS : STATUS_INVALID_STATEMENT;
        case '#':
            return (!parser_eat_next_char(EOL)) ? GRBL_SEND_COORD_SYSTEM : STATUS_INVALID_STATEMENT;
            error = GRBL_SEND_COORD_SYSTEM;
        case 'H':
            return (!parser_eat_next_char(EOL)) ? GRBL_HOME : STATUS_INVALID_STATEMENT;
        case 'X':
            return (!parser_eat_next_char(EOL)) ? GRBL_UNLOCK : STATUS_INVALID_STATEMENT;
        case 'G':
            return (!parser_eat_next_char(EOL)) ? GRBL_SEND_PARSER_MODES : STATUS_INVALID_STATEMENT;
        case 'C':
            return (!parser_eat_next_char(EOL)) ? GRBL_TOGGLE_CHECKMODE : STATUS_INVALID_STATEMENT;
        case 'J':
            if (parser_eat_next_char('='))
            {
                return STATUS_INVALID_JOG_COMMAND;
            }
            if (cnc_get_exec_state(EXEC_ALLACTIVE) & !cnc_get_exec_state(EXEC_JOG)) //Jog only allowed in IDLE or JOG mode
            {
                return STATUS_IDLE_ERROR;
            }
            cnc_set_exec_state(EXEC_JOG);
            return GRBL_JOG_CMD;
        case 'N':
            c = serial_getc();
            switch (c)
            {
            case '0':
            case '1':
                block_address = (!(c - '0') ? STARTUP_BLOCK0_ADDRESS_OFFSET : STARTUP_BLOCK1_ADDRESS_OFFSET);
                if (parser_eat_next_char('='))
                {
                    return STATUS_INVALID_STATEMENT;
                }
                break;
            case EOL:
                return GRBL_SEND_STARTUP_BLOCKS;
            default:
                return STATUS_INVALID_STATEMENT;
            }
            c = 'N';
            break;
        case 'R':
            if (parser_eat_next_char('S'))
            {
                return STATUS_INVALID_STATEMENT;
            }
            if (parser_eat_next_char('T'))
            {
                return STATUS_INVALID_STATEMENT;
            }
            if (parser_eat_next_char('='))
            {
                return STATUS_INVALID_STATEMENT;
            }
            break;
        case EOL:
            return GRBL_HELP;
        }

        parser_state_t next_state = {0};
        parser_words_t words = {0};
        parser_cmd_explicit_t cmd = {0};
        switch (c)
        {
        case 'R':
            c = serial_getc();
            switch (c)
            {
            case '$':
                settings_reset();
                break;
            case '#':
                parser_parameters_reset();
                break;
            case '*':
                settings_reset();
                parser_parameters_reset();
                break;
            default:
                return STATUS_INVALID_STATEMENT;
            }
            return ((!parser_eat_next_char(EOL)) ? GRBL_SEND_SETTINGS_RESET : STATUS_INVALID_STATEMENT);
        case 'N':
            error = parser_fetch_command(&next_state, &words, &cmd);
            if (error)
            {
                return error;
            }
            error = parser_validate_command(&next_state, &words, &cmd);
            if (error)
            {
                return error;
            }
            //everything ok reverts string and saves it
            do
            {
                serial_ungetc();
            } while (serial_peek() != '=');
            serial_getc();
            settings_save_startup_gcode(block_address);
            break;
        default:
            if (c >= '0' && c <= '9') //settings
            {
                float val = 0;
                uint8_t setting_num = 0;
                serial_ungetc();
                error = parser_get_float(&val);
                if (!error)
                {
                    return STATUS_INVALID_STATEMENT;
                }

                if ((error & NUMBER_ISFLOAT) || val > 255 || val < 0)
                {
                    return STATUS_INVALID_STATEMENT;
                }

                setting_num = (uint8_t)val;
                //eat '='
                if (parser_eat_next_char('='))
                {
                    return STATUS_INVALID_STATEMENT;
                }

                val = 0;
                if (!parser_get_float(&val))
                {
                    return STATUS_BAD_NUMBER_FORMAT;
                }

                if (parser_eat_next_char(EOL))
                {
                    return STATUS_INVALID_STATEMENT;
                }

                return settings_change(setting_num, val);
            }
            return STATUS_INVALID_STATEMENT;
        }

        return error;
    }

    static uint8_t parse_grbl_exec_code(uint8_t code)
    {

        switch (code)
        {
        case GRBL_SEND_SYSTEM_SETTINGS:
            protocol_send_cnc_settings();
            return STATUS_OK;
        case GRBL_SEND_COORD_SYSTEM:
            protocol_send_gcode_coordsys();
            return STATUS_OK;
        case GRBL_SEND_PARSER_MODES:
            protocol_send_gcode_modes();
            return STATUS_OK;
        case GRBL_SEND_STARTUP_BLOCKS:
            protocol_send_start_blocks();
            return STATUS_OK;
        case GRBL_TOGGLE_CHECKMODE:
            if (mc_toogle_checkmode())
            {
                protocol_send_feedback(MSG_FEEDBACK_4);
            }
            else
            {
                cnc_stop();
                cnc_alarm(EXEC_ALARM_RESET);
                protocol_send_feedback(MSG_FEEDBACK_5);
            }
            return STATUS_OK;
        case GRBL_SEND_SETTINGS_RESET:
            protocol_send_feedback(MSG_FEEDBACK_9);
            return STATUS_OK;
        case GRBL_UNLOCK:
            cnc_unlock();
            if (cnc_get_exec_state(EXEC_DOOR))
            {
                return STATUS_CHECK_DOOR;
            }
            protocol_send_feedback(MSG_FEEDBACK_3);
            return STATUS_OK;
        case GRBL_HOME:
            if (!g_settings.homing_enabled)
            {
                return STATUS_SETTING_DISABLED;
            }

            cnc_unlock();
            if (cnc_get_exec_state(EXEC_DOOR))
            {
                return STATUS_CHECK_DOOR;
            }

            cnc_home();
            return STATUS_OK;
        case GRBL_HELP:
            protocol_send_string(MSG_HELP);
            return STATUS_OK;
        default:
            return code;
        }

        return STATUS_OK;
    }

    /*
	STEP 1
	Fetches the next line from the mcu communication buffer and preprocesses the string
	In the preprocess these steps are executed
		1. Whitespaces are ignored
		2. Comments are parsed (nothing is done besides parsing for now)
		3. All letters are upper-cased
		4. Checks number formats in all words
		5. Checks for modal groups and words collisions
*/
    static uint8_t parser_fetch_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
    {
        uint8_t error = STATUS_OK;
        uint8_t wordcount = 0;
        for (;;)
        {
            unsigned char word = 0;
            float value = 0;
            //this flushes leading white chars and also takes care of processing comments
            parser_get_next_preprocessed(true);
#ifdef ECHO_CMD
            if (!wordcount)
            {
                protocol_send_string(MSG_ECHO);
            }
#endif
            error = parser_get_token(&word, &value);
            if (error)
            {
                parser_discard_command();
#ifdef ECHO_CMD
                protocol_send_string(MSG_END);
#endif
                return error;
            }
            uint8_t code = (uint8_t)floorf(value);
            //check mantissa
            uint8_t mantissa = (uint8_t)roundf((value - code) * 100.0f);

            switch (word)
            {
            case EOL:
#ifdef ECHO_CMD
                protocol_send_string(MSG_END);
#endif
                return STATUS_OK;
            case 'G':
                error = parser_gcode_word(code, mantissa, new_state, cmd);
                break;
            case 'M':
                error = parser_mcode_word(code, mantissa, new_state, cmd);
                break;
            default:
                if (word == 'N' && wordcount != 0)
                {
                    error = STATUS_GCODE_INVALID_LINE_NUMBER;
                    break;
                }
                error = parser_letter_word(word, value, mantissa, words, cmd);
                break;
            }

            if (error)
            {
                parser_discard_command();
#ifdef ECHO_CMD
                protocol_send_string(MSG_END);
#endif
                return error;
            }

            wordcount++;
        }
        //Never should reach
        return STATUS_CRITICAL_FAIL;
    }

    /*
	STEP 2
	Validadates command by checking for errors on all G/M Codes
		RS274NGC v3 - 3.5 G Codes
		RS274NGC v3 - 3.6 Input M Codes
		RS274NGC v3 - 3.7 Other Input Codes
*/

    static uint8_t parser_validate_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
    {
        //only alow groups 3, 6 and modal G53
        if (cnc_get_exec_state(EXEC_JOG))
        {
            if (cmd->groups & ~(GCODE_GROUP_DISTANCE | GCODE_GROUP_UNITS | GCODE_GROUP_NONMODAL))
            {
                return STATUS_INVALID_JOG_COMMAND;
            }

            //if nonmodal other than G53
            if (new_state->groups.nonmodal != 0 && new_state->groups.nonmodal != G53)
            {
                return STATUS_INVALID_JOG_COMMAND;
            }

            if (cmd->words & GCODE_JOG_INVALID_WORDS)
            {
                return STATUS_INVALID_JOG_COMMAND;
            }

            new_state->groups.motion = 1;
            SETFLAG(cmd->groups, GCODE_GROUP_MOTION);
        }

        //RS274NGC v3 - 3.5 G Codes
        //group 0 - non modal (incomplete)
        if ((cmd->groups & GCODE_GROUP_NONMODAL))
        {
            switch (new_state->groups.nonmodal)
            {
            case G10:
                //G10
                //if no P or L is present
                if (!(cmd->words & (GCODE_WORD_P | GCODE_WORD_L)))
                {
                    return STATUS_GCODE_VALUE_WORD_MISSING;
                }
                //L is not 2
                if (words->l != 2)
                {
                    return STATUS_GCODE_UNSUPPORTED_COMMAND;
                }
                //P is not between 1 and N of coord systems
                if (words->p != 28 && words->p != 30)
                {
                    if (words->p > COORD_SYS_COUNT)
                    {
                        return STATUS_GCODE_UNSUPPORTED_COORD_SYS;
                    }
                }
            case G92:
                if (!CHECKFLAG(cmd->words, GCODE_ALL_AXIS))
                {
                    return STATUS_GCODE_NO_AXIS_WORDS;
                }
                break;
            case G53:
                //G53
                //if no G0 or G1 not active
                if (new_state->groups.motion > G1)
                {
                    return STATUS_GCODE_G53_INVALID_MOTION_MODE;
                }
                break;
            }
        }

        //group 1 - motion (incomplete)
        //TODO
        //81...89 Canned cycles
        if (CHECKFLAG(cmd->groups, GCODE_GROUP_MOTION))
        {
            switch (new_state->groups.motion)
            {
            case G0:    //G0
            case G1:    //G1
            case G38_2: //G38.2
            case G38_3: //G38.3
            case G38_4: //G38.4
            case G38_5: //G38.5
                if (!CHECKFLAG(cmd->words, GCODE_ALL_AXIS))
                {
                    return STATUS_GCODE_NO_AXIS_WORDS;
                }
                break;
            case G2:
            case G3:
                switch (new_state->groups.plane)
                {
                case G17:
                    if (!CHECKFLAG(cmd->words, GCODE_XYPLANE_AXIS))
                    {
                        return STATUS_GCODE_NO_AXIS_WORDS_IN_PLANE;
                    }

                    if (!CHECKFLAG(cmd->words, (GCODE_IJPLANE_AXIS | GCODE_WORD_R)))
                    {
                        return STATUS_GCODE_NO_OFFSETS_IN_PLANE;
                    }
                    break;
                case G18:
                    if (!CHECKFLAG(cmd->words, GCODE_XZPLANE_AXIS))
                    {
                        return STATUS_GCODE_NO_AXIS_WORDS_IN_PLANE;
                    }

                    if (!CHECKFLAG(cmd->words, (GCODE_IKPLANE_AXIS | GCODE_WORD_R)))
                    {
                        return STATUS_GCODE_NO_OFFSETS_IN_PLANE;
                    }
                    break;
                case G19:
                    if (!CHECKFLAG(cmd->words, GCODE_YZPLANE_AXIS))
                    {
                        return STATUS_GCODE_NO_AXIS_WORDS_IN_PLANE;
                    }

                    if (!CHECKFLAG(cmd->words, (GCODE_JKPLANE_AXIS | GCODE_WORD_R)))
                    {
                        return STATUS_GCODE_NO_OFFSETS_IN_PLANE;
                    }
                    break;
                }
                break;
            case G80: //G80 and
                if (CHECKFLAG(cmd->words, GCODE_ALL_AXIS) && !cmd->group_0_1_useaxis)
                {

                    return STATUS_GCODE_AXIS_WORDS_EXIST;
                }

                break;
            default: //G81..G89 canned cycles (not implemented yet)
                break;
            }

            //group 5 - feed rate mode
            if (new_state->groups.motion >= G1 && new_state->groups.motion <= G3)
            {
                if (!CHECKFLAG(cmd->words, GCODE_WORD_F))
                {
                    if (new_state->groups.feedrate_mode == G93)
                    {
                        return STATUS_GCODE_UNDEFINED_FEED_RATE;
                    }

                    if (new_state->feedrate == 0)
                    {
                        return STATUS_GCODE_UNDEFINED_FEED_RATE;
                    }
                }
                else
                {
                    if (words->f <= 0)
                    {
                        return STATUS_GCODE_UNDEFINED_FEED_RATE;
                    }
                }
            }
        }
        //group 2 - plane selection (nothing to be checked)
        //group 3 - distance mode (nothing to be checked)

        //group 6 - units (nothing to be checked)
        //group 7 - cutter radius compensation (not implemented yet)
        //group 8 - tool length offset
        if ((new_state->groups.tool_length_offset == G43_1) && CHECKFLAG(cmd->groups, GCODE_GROUP_TOOLLENGTH))
        {
            if (!CHECKFLAG(cmd->words, GCODE_WORD_Z))
            {
                return STATUS_GCODE_AXIS_WORDS_EXIST;
            }
        }
//group 10 - return mode in canned cycles (not implemented yet)
//group 12 - coordinate system selection (nothing to be checked)
//group 13 - path control mode (nothing to be checked)

//RS274NGC v3 - 3.6 Input M Codes
//group 4 - stopping (nothing to be checked)
//group 6 - tool change(not implemented yet)
//group 7 - spindle turning (nothing to be checked)
//group 8 - coolant (nothing to be checked)
//group 9 - enable/disable feed and speed override switches (not implemented)

//RS274NGC v3 - 3.7 Other Input Codes
//Words S and T must be positive
#ifdef USE_SPINDLE
        if (words->s < 0)
        {
            return STATUS_NEGATIVE_VALUE;
        }
#endif
#ifdef USE_TOOL_CHANGER
        if (words->t < 0)
        {
            return STATUS_NEGATIVE_VALUE;
        }
        if (words->t > g_settings.tool_count)
        {
            return STATUS_INVALID_TOOL;
        }
#endif
        return STATUS_OK;
    }

    /*
	STEP 3
	Executes the command
		Follows the RS274NGC v3 - 3.8 Order of Execution
	All coordinates are converted to machine absolute coordinates before sent to the motion controller
*/
    static uint8_t parser_exec_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
    {
        float axis[AXIS_COUNT];
        //plane selection
        uint8_t a = 0;
        uint8_t b = 0;
        uint8_t offset_a = 0;
        uint8_t offset_b = 0;
        float radius;
        //pointer to planner position
        float planner_last_pos[AXIS_COUNT];
        motion_data_t block_data = {0};

        //stoping from previous command M2 or M30 command
        if (new_state->groups.stopping && !CHECKFLAG(cmd->groups, GCODE_GROUP_STOPPING))
        {
            if (new_state->groups.stopping == 3 || new_state->groups.stopping == 4)
            {
                return STATUS_PROGRAM_ENDED;
            }

            new_state->groups.stopping = 0;
        }

#ifdef GCODE_PROCESS_LINE_NUMBERS
        block_data.line = words->n;
#endif

        mc_get_position(planner_last_pos);

        //RS274NGC v3 - 3.8 Order of Execution
        //1. comment (ignored - already filtered)
        //2. set feed mode
        block_data.motion_mode = MOTIONCONTROL_MODE_FEED; //default mode
        if (new_state->groups.feedrate_mode == G93)
        {
            block_data.motion_mode |= MOTIONCONTROL_MODE_INVERSEFEED;
        }
        //3. set feed rate (µCNC works in units per second and not per minute)
        if (CHECKFLAG(cmd->words, GCODE_WORD_F))
        {
            if (new_state->groups.feedrate_mode != G93)
            {
                new_state->feedrate = words->f; // * MIN_SEC_MULT;
            }
        }

//4. set spindle speed
#ifdef USE_SPINDLE
        if (CHECKFLAG(cmd->words, GCODE_WORD_S))
        {
            new_state->spindle = words->s;
            block_data.update_tools = (parser_state.spindle != new_state->spindle);
        }
#endif
//5. select tool
#ifdef USE_TOOL_CHANGER
        if (CHECKFLAG(cmd->words, GCODE_WORD_T))
        {
            new_state->tool_index = words->t;
        }
#else
    new_state->tool_index = 1; //tool is allways 1
#endif
//6. change tool (not implemented yet)
//7. spindle on/off
#ifdef USE_SPINDLE
        switch (new_state->groups.spindle_turning)
        {
        case M3:
            block_data.spindle = new_state->spindle;
            break;
        case M4:
            block_data.spindle = -new_state->spindle;
            break;
        case M5:
            block_data.spindle = 0;
            break;
        }

        //spindle speed or direction was changed (force a safety dwell to let the spindle change speed and continue)
        if (CHECKFLAG(cmd->words, GCODE_WORD_S) || CHECKFLAG(cmd->groups, GCODE_GROUP_SPINDLE))
        {
            block_data.update_tools = true;
#ifdef LASER_MODE
            if (!g_settings.laser_mode)
            {
#endif
                block_data.dwell = (uint16_t)roundf(DELAY_ON_SPINDLE_SPEED_CHANGE * 1000);
#ifdef LASER_MODE
            }
#endif
        }
#endif
//8. coolant on/off
#ifdef USE_COOLANT
        if (CHECKFLAG(cmd->groups, GCODE_GROUP_COOLANT))
        {
            block_date.update_tools = true;
        }
        block_data.coolant = new_state->groups.coolant;
//moving to planner
//parser_update_coolant(new_state->groups.coolant);
#endif

        //9. overrides
        if ((new_state->groups.feed_speed_override == M48) != planner_get_overrides())
        {
            planner_toggle_overrides();
        }

        //10. dwell
        if (new_state->groups.nonmodal == G4)
        {
            //calc dwell in time in 10ms increments
            block_data.dwell = MAX(block_data.dwell, (uint16_t)roundf(words->p));
            new_state->groups.nonmodal = 0;
        }

        //after all spindle, overrides, coolant and dwells are set
        //execute sync if dwell is present
        if (block_data.dwell)
        {
            mc_dwell(&block_data);
        }

        //11. set active plane (G17, G18, G19)
        switch (new_state->groups.plane)
        {
#if (defined(AXIS_X) && defined(AXIS_Y))
        case 0:
            a = AXIS_X;
            b = AXIS_Y;
            offset_a = 0;
            offset_b = 1;
            break;
#endif
#if (defined(AXIS_X) && defined(AXIS_Z))
        case 1:
            a = AXIS_X;
            b = AXIS_Z;
            offset_a = 0;
            offset_b = 2;
            break;
#endif
#if (defined(AXIS_Y) && defined(AXIS_Z))
        case 2:
            a = AXIS_Y;
            b = AXIS_Z;
            offset_a = 1;
            offset_b = 2;
            break;
#endif
        }

        //12. set length units (G20, G21).
        if (new_state->groups.units == G20) //all internal state variables must be converted to mm
        {
            for (uint8_t i = AXIS_COUNT; i != 0;)
            {
                i--;
                words->xyzabc[i] *= INCH_MM_MULT;
            }

            //check if any i, j or k words were used
            if (CHECKFLAG(cmd->words, GCODE_IJK_AXIS))
            {
                words->ijk[0] *= INCH_MM_MULT;
                words->ijk[1] *= INCH_MM_MULT;
                words->ijk[2] *= INCH_MM_MULT;
            }

            //if normal feed mode convert to mm/s
            if (CHECKFLAG(cmd->words, GCODE_WORD_F) && (new_state->groups.feedrate_mode != G93))
            {
                new_state->feedrate *= INCH_MM_MULT;
            }

            if (CHECKFLAG(cmd->words, GCODE_WORD_R))
            {
                words->r *= INCH_MM_MULT;
            }
        }

//13. cutter radius compensation on or off (G40, G41, G42) (not implemented yet)
//14. cutter length compensation on or off (G43.1, G49)
#ifdef AXIS_TOOL
        if ((new_state->groups.tool_length_offset == 1) && CHECKFLAG(cmd->groups, GCODE_GROUP_TOOLLENGTH))
        {
            parser_parameters.tool_length_offset = words->xyzabc[AXIS_Z];
            CLEARFLAG(cmd->words, GCODE_WORD_Z);
            words->xyzabc[AXIS_TOOL] = 0; //resets parameter so it it doen't do anything else
        }
#endif
        //15. coordinate system selection (G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3) (OK nothing to be done)
        if (CHECKFLAG(cmd->groups, GCODE_GROUP_COORDSYS))
        {
            parser_parameters.coord_system_index = new_state->groups.coord_system;
            settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (parser_parameters.coord_system_index * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)&parser_parameters.coord_system_offset[0], PARSER_PARAM_SIZE);
            parser_wco_counter = 0;
        }
        //16. set path control mode (G61, G61.1, G64)
        switch (new_state->groups.path_mode)
        {
        case 1:
            block_data.motion_mode |= PLANNER_MOTION_EXACT_STOP;
            break;
        case 3:
            block_data.motion_mode |= PLANNER_MOTION_CONTINUOUS;
            break;
        }

        //17. set distance mode (G90, G91) (OK nothing to be done)

        //18. set retract mode (G98, G99)  (not implemented yet)
        //19. home (G28, G30) or change coordinate system data (G10) or set axis offsets (G92, G92.1, G92.2, G92.3)
        //	or also modifies target if G53 is active. These are executed after calculating intemediate targets (G28 ad G30)
        //set the initial feedrate to the maximum value
        block_data.feed = FLT_MAX;
        //limit feed to the maximum possible feed
        if (!CHECKFLAG(block_data.motion_mode, MOTIONCONTROL_MODE_INVERSEFEED))
        {
            block_data.feed = MIN(block_data.feed, new_state->feedrate);
        }
        else
        {
            block_data.feed = new_state->feedrate;
        }

        //if non-modal is executed
        uint8_t index = 255;
        uint16_t address = 0;
        uint8_t error = 0;
        switch (new_state->groups.nonmodal)
        {
        case G10: //G10
            index = ((uint8_t)words->p) ? words->p : (parser_parameters.coord_system_index + 1);
            switch (index)
            {
            case 28:
                address = G28ADDRESS;
                index = G28HOME;
                break;
            case 30:
                address = G30ADDRESS;
                index = G30HOME;
                break;
            default:
                index--;
                address = SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (index * PARSER_PARAM_ADDR_OFFSET);
                break;
            }
            break;
        case G92_1: //G92.1
            memset(g92permanentoffset, 0, sizeof(g92permanentoffset));
            //continue
        case G92_2: //G92.2
            memset(parser_parameters.g92_offset, 0, sizeof(parser_parameters.g92_offset));
            parser_wco_counter = 0;
            new_state->groups.nonmodal = 0; //this command is compatible with motion commands
            break;
        case G92_3: //G92.3
            memcpy(parser_parameters.g92_offset, g92permanentoffset, sizeof(g92permanentoffset));
            parser_wco_counter = 0;
            new_state->groups.nonmodal = 0; //this command is compatible with motion commands
            break;
        }

        memcpy(axis, words->xyzabc, sizeof(axis));

        // check from were to read the previous values for the axis array
        if (index != 255)
        {
            if (settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (index * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)&planner_last_pos[0], PARSER_PARAM_SIZE))
            {
                memset(planner_last_pos, 0, sizeof(planner_last_pos));
            }
        }
        else
        {
            //if by any reason this is a nomotion command or world coordinates are used skip this
            switch (new_state->groups.nonmodal)
            {
            case G53:
            case G28:
            case G30:
                break;
            default:
                if ((new_state->groups.distance_mode == G90))
                {
                    for (uint8_t i = AXIS_COUNT; i != 0;)
                    {
                        i--;
                        axis[i] += parser_parameters.coord_system_offset[i] + parser_parameters.g92_offset[i];
                    }
#ifdef AXIS_TOOL
                    axis[AXIS_TOOL] += parser_parameters.tool_length_offset;
#endif
                }
                else
                {
                    for (uint8_t i = AXIS_COUNT; i != 0;)
                    {
                        i--;
                        axis[i] += planner_last_pos[i];
                    }
                }
                break;
            }
        }

//for all not explicitly declared axis retain their position
#ifdef AXIS_X
        if (!CHECKFLAG(cmd->words, GCODE_WORD_X))
        {
            axis[AXIS_X] = planner_last_pos[AXIS_X];
        }
#endif
#ifdef AXIS_Y
        if (!CHECKFLAG(cmd->words, GCODE_WORD_Y))
        {
            axis[AXIS_Y] = planner_last_pos[AXIS_Y];
        }
#endif
#ifdef AXIS_Z
        if (!CHECKFLAG(cmd->words, GCODE_WORD_Z))
        {
            axis[AXIS_Z] = planner_last_pos[AXIS_Z];
        }
#endif
#ifdef AXIS_A
        if (!CHECKFLAG(cmd->words, GCODE_WORD_A))
        {
            axis[AXIS_A] = planner_last_pos[AXIS_A];
        }
#endif
#ifdef AXIS_B
        if (!CHECKFLAG(cmd->words, GCODE_WORD_B))
        {
            axis[AXIS_B] = planner_last_pos[AXIS_B];
        }
#endif
#ifdef AXIS_C
        if (!CHECKFLAG(cmd->words, GCODE_WORD_C))
        {
            axis[AXIS_C] = planner_last_pos[AXIS_C];
        }
#endif

        //stores G10 L2 command in the right address
        if (index <= G30HOME)
        {
            settings_save(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (index * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)&axis[0], PARSER_PARAM_SIZE);
            if (index == parser_parameters.coord_system_index)
            {
                memcpy(parser_parameters.coord_system_offset, axis, sizeof(parser_parameters.coord_system_offset));
            }
            parser_wco_counter = 0;
        }

#ifdef LASER_MODE
        //laser disabled in nonmodal moves
        if (g_settings.laser_mode && new_state->groups.nonmodal)
        {
            block_data.spindle = 0;
        }
#endif

        switch (new_state->groups.nonmodal)
        {
        case G28: //G28
        case G30: //G30
            block_data.feed = FLT_MAX;
            if (CHECKFLAG(cmd->words, GCODE_ALL_AXIS))
            {
                error = mc_line(axis, &block_data);
                if (error)
                {
                    return error;
                }
            }

            if (new_state->groups.nonmodal == G28)
            {
                settings_load(G28ADDRESS, (uint8_t *)&axis, PARSER_PARAM_SIZE);
            }
            else
            {
                settings_load(G30ADDRESS, (uint8_t *)&axis, PARSER_PARAM_SIZE);
            }
            error = mc_line((float *)&axis, &block_data);
            {
                return error;
            }
            break;
        case G92: //G92
            for (uint8_t i = AXIS_COUNT; i != 0;)
            {
                i--;
                parser_parameters.g92_offset[i] = -(axis[i] - planner_last_pos[i] - parser_parameters.g92_offset[i]);
            }
            memcpy(g92permanentoffset, parser_parameters.g92_offset, sizeof(g92permanentoffset));
            //settings_save(G92ADDRESS, (uint8_t *)&parser_parameters.g92_offset[0], PARSER_PARAM_SIZE);
            parser_wco_counter = 0;
            break;
        case G53:
            new_state->groups.nonmodal = 0; //this command is compatible with motion commands
            break;
        }

        float x, y;
        //20. perform motion (G0 to G3, G80 to G89), as modified (possibly) by G53.
        //only if any axis word was used
        //incomplete (canned cycles not supported)
        if (new_state->groups.nonmodal == 0 && CHECKFLAG(cmd->words, GCODE_ALL_AXIS))
        {
            uint8_t probe_error;

            switch (new_state->groups.motion)
            {
            case G0:
                //rapid move
                block_data.feed = FLT_MAX;
                //continues to send G1 at maximum feed rate
#ifdef LASER_MODE
                //laser disabled in G0
                if (g_settings.laser_mode)
                {
                    block_data.spindle = 0;
                }
#endif
            case G1:
                if (block_data.feed == 0)
                {
                    return STATUS_FEED_NOT_SET;
                }
                error = mc_line(axis, &block_data);
                break;
            case G2:
            case G3:
                if (block_data.feed == 0)
                {
                    return STATUS_FEED_NOT_SET;
                }

                //target points
                x = axis[a] - planner_last_pos[a];
                y = axis[b] - planner_last_pos[b];
                float center_offset_a = words->ijk[offset_a];
                float center_offset_b = words->ijk[offset_b];
                //radius mode
                if (CHECKFLAG(cmd->words, GCODE_WORD_R))
                {
                    if (x == 0 && y == 0)
                    {
                        return STATUS_GCODE_INVALID_TARGET;
                    }

                    float x_sqr = x * x;
                    float y_sqr = y * y;
                    float c_factor = words->r * words->r;
                    c_factor = fast_flt_mul4(c_factor) - x_sqr - y_sqr;

                    if (c_factor < 0)
                    {
                        return STATUS_GCODE_ARC_RADIUS_ERROR;
                    }

                    c_factor = -sqrt((c_factor) / (x_sqr + y_sqr));

                    if (new_state->groups.motion == G3)
                    {
                        c_factor = -c_factor;
                    }

                    radius = words->r;
                    if (words->r < 0)
                    {
                        c_factor = -c_factor;
                        radius = -radius; // Finished with r. Set to positive for mc_arc
                    }

                    // Complete the operation by calculating the actual center of the arc
                    center_offset_a = (x - (y * c_factor));
                    center_offset_b = (y + (x * c_factor));
                    center_offset_a = fast_flt_div2(center_offset_a);
                    center_offset_b = fast_flt_div2(center_offset_b);
                }
                else //offset mode
                {
                    //calculate radius and check if center is within tolerances
                    radius = sqrtf(center_offset_a * center_offset_a + center_offset_b * center_offset_b);
                    //calculates the distance between the center point and the target points and compares with the center offset distance
                    float x1 = x - center_offset_a;
                    float y1 = y - center_offset_b;
                    float r1 = sqrt(x1 * x1 + y1 * y1);
                    if (fabs(radius - r1) > 0.002) //error must not exceed 0.002mm according to the NIST RS274NGC standard
                    {
                        return STATUS_GCODE_INVALID_TARGET;
                    }
                }

                error = mc_arc(axis, center_offset_a, center_offset_b, radius, a, b, (new_state->groups.motion == 2), &block_data);
                break;
            case 4: //G38.2
            case 5: //G38.3
            case 6: //G38.4
            case 7: //G38.5
                probe_error = mc_probe(axis, (new_state->groups.motion > 5), &block_data);
                if (probe_error)
                {
                    parser_parameters.last_probe_ok = 0;
                    if (!(new_state->groups.motion & 0x01))
                    {
                        cnc_alarm(probe_error);
                    }
                }
                parser_parameters.last_probe_ok = 1;
            }
        }

        if (error)
        {
            return error;
        }

        //stop (M0, M1, M2, M30, M60) (not implemented yet).
        bool hold = false;
        bool resetparser = false;
        switch (new_state->groups.stopping)
        {
        case 1: //M0
        case 6: //M60 (pallet change has no effect)
            hold = true;
            break;
        case 2: //M1
#ifdef M1_CONDITION_ASSERT
            hold = M1_CONDITION_ASSERT;
#endif
            break;
        case 3: //M2
        case 4: //M30 (pallet change has no effect)
            hold = true;
            resetparser = true;
            break;
        }

        if (hold)
        {
            mc_pause();
            if (resetparser)
            {
                cnc_stop();
                parser_reset();
                protocol_send_feedback(MSG_FEEDBACK_8);
            }
        }

        //if reached here the execution was not intersected
        //send a spindle and coolant update if needed
        if (block_data.update_tools)
        {
            return mc_update_tools(&block_data);
        }

        return STATUS_OK;
    }

    /*
	Parse the next gcode line available in the buffer and send it to the motion controller
*/
    static uint8_t parser_gcode_command(void)
    {
        uint8_t result = 0;
        //initializes new state
        parser_state_t next_state = {0};
        parser_words_t words = {0};
        parser_cmd_explicit_t cmd = {0};
        //next state will be the same as previous except for nonmodal group (is set with 0)
        memcpy(&next_state, &parser_state, sizeof(parser_state_t));
        next_state.groups.nonmodal = 0; //reset nonmodal

        //fetch command
        result = parser_fetch_command(&next_state, &words, &cmd);
        if (result != STATUS_OK)
        {
            return result;
        }

        //validates command
        result = parser_validate_command(&next_state, &words, &cmd);
        if (result != STATUS_OK)
        {
            return result;
        }

        //executes command
        result = parser_exec_command(&next_state, &words, &cmd);
        if (result != STATUS_OK)
        {
            return result;
        }

        //if is jog motion state is not preserved
        if (!cnc_get_exec_state(EXEC_JOG))
        {
            //if everything went ok updates the parser modal groups and position
            memcpy(&parser_state, &next_state, sizeof(parser_state_t));
        }

        return result;
    }

    /*
	Parses a string to number (real)
	If the number is an integer the isinteger flag is set
	The string pointer is also advanced to the next position
*/
    static uint8_t parser_get_float(float *value)
    {
        uint32_t intval = 0;
        uint8_t fpcount = 0;
        uint8_t result = NUMBER_UNDEF;

        unsigned char c = parser_get_next_preprocessed(true);

        *value = 0;

        if (c == '-' || c == '+')
        {
            if (c == '-')
            {
                result |= NUMBER_ISNEGATIVE;
            }
#ifdef ECHO_CMD
            serial_putc(serial_getc());
#else
        serial_getc();

#endif
            c = parser_get_next_preprocessed(true);
        }

        for (;;)
        {
            uint8_t digit = (uint8_t)c - 48;
            if (digit <= 9)
            {
                intval = fast_int_mul10(intval) + digit;
                if (fpcount)
                {
                    fpcount++;
                }

                result |= NUMBER_OK;
            }
            else if (c == '.' && !fpcount)
            {
                fpcount++;
                result |= NUMBER_ISFLOAT;
            }
            else if (c == ' ')
            {
                //ignore white chars in the middle of numbers
            }
            else
            {
                if (!(result & NUMBER_OK))
                {
                    return NUMBER_UNDEF;
                }
                break;
            }

#ifdef ECHO_CMD
            serial_putc(serial_getc());
#else
        serial_getc();

#endif
            c = parser_get_next_preprocessed(true);
        }

        *value = (float)intval;
        if (fpcount)
        {
            fpcount--;
        }

        do
        {
            if (fpcount >= 2)
            {
                *value *= 0.01f;
                fpcount -= 2;
            }

            if (fpcount >= 1)
            {
                *value *= 0.1f;
                fpcount -= 1;
            }

        } while (fpcount != 0);

        if (result & NUMBER_ISNEGATIVE)
        {
            *value = -*value;
        }

        return result;
    }

    /*
	Parses comments almost as defined in the RS274NGC
	To be compatible with Grbl it accepts bad format comments
	On error returns false otherwise returns true
*/
    static uint8_t parser_get_comment(void)
    {
        uint8_t msg_parser = 0;
        for (;;)
        {
            unsigned char c = serial_peek();
            switch (c)
            {
                //case '(':	//error under RS274NGC (commented for Grbl compatibility)
            case ')': //OK
                serial_getc();
#ifdef PROCESS_COMMENTS
                if (msg_parser == 4)
                {
                    protocol_send_string(MSG_END);
                }
#endif
                return STATUS_OK;
            case EOL: //error under RS274NGC
                return STATUS_BAD_COMMENT_FORMAT;
            case OVF:
                //return ((c==')') ? true : false); //under RS274NGC (commented for Grbl compatibility)
                return STATUS_OVERFLOW;
            case ' ':
                break;
#ifdef PROCESS_COMMENTS
            default:
                switch (msg_parser)
                {
                case 0:
                    msg_parser = (c == 'M' | c == 'm') ? 1 : 0xFF;
                    break;
                case 1:
                    msg_parser = (c == 'S' | c == 's') ? 2 : 0xFF;
                    break;
                case 2:
                    msg_parser = (c == 'G' | c == 'g') ? 3 : 0xFF;
                    break;
                case 3:
                    msg_parser = (c == ',') ? 4 : 0xFF;
                    protocol_send_string(MSG_START);
                    break;
                case 4:
                    serial_putc(c);
                    break;
                }
                break;
#endif
            }

            serial_getc();
        }

        return STATUS_BAD_COMMENT_FORMAT; //never reached here
    }

    static uint8_t parser_eat_next_char(unsigned char c)
    {
        return ((serial_getc() == c) ? STATUS_OK : STATUS_INVALID_STATEMENT);
    }

    static uint8_t parser_get_token(unsigned char *word, float *value)
    {
        unsigned char c = serial_getc();

        //if other char starts tokenization
        if (c > 'Z')
        {
            c -= 32; //uppercase
        }

        *word = c;
        switch (c)
        {
        case EOL: //EOL
            return STATUS_OK;
        case OVF:
            return STATUS_OVERFLOW;
        default:
#ifdef ECHO_CMD
            serial_putc(c);
#endif
            if (c < 'A' || c > 'Z') //invalid recognized char
            {
                return STATUS_EXPECTED_COMMAND_LETTER;
            }

            if (!parser_get_float(value))
            {
                return STATUS_BAD_NUMBER_FORMAT;
            }
            break;
        }

        return STATUS_OK;
    }

    static uint8_t parser_gcode_word(uint8_t code, uint8_t mantissa, parser_state_t *new_state, parser_cmd_explicit_t *cmd)
    {
        uint16_t new_group = cmd->groups;

        if (mantissa)
        {
            switch (code)
            {
                //codes with possible mantissa
            case 38:
            case 43:
            case 59:
            case 61:
            case 92:
                break;
            default:
                return STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER;
            }
        }
        else
        {
            mantissa = 255; //if code should not have mantissa set it to a undefined value
        }

        switch (code)
        {
            //motion codes
        case 38: //check if 38.x
        case 0:
        case 1:
        case 2:
        case 3:
        case 80:

            switch (mantissa)
            {
            case 255:
                break;
            case 20:
                code = 4;
                break;
            case 30:
                code = 5;
                break;
            case 40:
                code = 6;
                break;
            case 50:
                code = 7;
                break;
            default:
                return STATUS_GCODE_UNSUPPORTED_COMMAND;
            }
            if (code == 80)
            {
                code -= 72;
            }
            else if (cmd->group_0_1_useaxis)
            {
                return STATUS_GCODE_MODAL_GROUP_VIOLATION;
            }
            else
            {
                cmd->group_0_1_useaxis = true;
            }

            new_group |= GCODE_GROUP_MOTION;
            new_state->groups.motion = code;
            break;
        case 17:
        case 18:
        case 19:
            new_group |= GCODE_GROUP_PLANE;
            code -= 17;
            new_state->groups.plane = code;
            break;
        case 90:
        case 91:
            new_group |= GCODE_GROUP_DISTANCE;
            new_state->groups.distance_mode = code - 90;
            break;
        case 93:
        case 94:
            new_group |= GCODE_GROUP_FEEDRATE;
            new_state->groups.feedrate_mode = code - 93;
            break;
        case 20:
        case 21:
            new_group |= GCODE_GROUP_UNITS;
            new_state->groups.units = code - 20;
            break;
        case 40:
        case 41:
        case 42:
            new_group |= GCODE_GROUP_CUTTERRAD;
            new_state->groups.cutter_radius_compensation = code - 40;
            break;
        case 43: //doesn't support G43 but G43.1 (takes Z coordinate input has offset)
        case 49:
            if (code == 43)
            {
                switch (mantissa)
                {
                case 10:
                    break;
                default:
                    return STATUS_GCODE_UNSUPPORTED_COMMAND;
                }
            }
            new_state->groups.tool_length_offset = ((code == 49) ? G49 : G43_1);
            new_group |= GCODE_GROUP_TOOLLENGTH;
            break;
        case 98:
        case 99:
            new_group |= GCODE_GROUP_RETURNMODE;
            new_state->groups.return_mode = code - 98;
            break;
        case 54:
        case 55:
        case 56:
        case 57:
        case 58:
        case 59:
            new_group |= GCODE_GROUP_COORDSYS;
            code -= 54;
            switch (mantissa)
            {
            case 255:
                break;
            case 10:
                code += 1;
                break;
            case 20:
                code += 2;
                break;
            case 30:
                code += 3;
                break;
            default:
                return STATUS_GCODE_UNSUPPORTED_COMMAND;
            }

            if (code > COORD_SYS_COUNT)
            {
                return STATUS_GCODE_UNSUPPORTED_COORD_SYS;
            }
            new_state->groups.coord_system = code;
            break;
        case 61:
        case 64:
            code -= 61;
            switch (mantissa)
            {
            case 255:
                break;
            case 10:
                code += 1;
                break;
            default:
                return STATUS_GCODE_UNSUPPORTED_COMMAND;
            }
            new_group |= GCODE_GROUP_PATH;
            new_state->groups.path_mode = code;
            break;
        //de following nonmodal colide with motion groupcodes
        case 10:
        case 28:
        case 30:
        case 92:
            if (cmd->group_0_1_useaxis)
            {
                return STATUS_GCODE_MODAL_GROUP_VIOLATION;
            }
            cmd->group_0_1_useaxis = true;
        case 4:
        case 53:

            //convert code within 4 bits without
            //4 = 1
            //10 = 2
            //28 = 3
            //30 = 4
            //53 = 6
            //92 = 10
            //92.1 = 11
            //92.2 = 12
            //92.3 = 13
            code = (uint8_t)floorf(code * 0.10001f);
            switch (mantissa)
            {
            case 255:
                break;
            case 30:
                code++;
            case 20:
                code++;
            case 10:
                code++;
                break;
            default:
                return STATUS_GCODE_UNSUPPORTED_COMMAND;
            }

            new_group |= GCODE_GROUP_NONMODAL;
            new_state->groups.nonmodal = code + 1;
            break;
        default:
            return STATUS_GCODE_UNSUPPORTED_COMMAND;
        }

        if (new_group == cmd->groups)
        {
            return STATUS_GCODE_MODAL_GROUP_VIOLATION;
        }

        cmd->groups = new_group;
        return STATUS_OK;
    }

    static uint8_t parser_mcode_word(uint8_t code, uint8_t mantissa, parser_state_t *new_state, parser_cmd_explicit_t *cmd)
    {
        uint16_t new_group = cmd->groups;

        if (mantissa)
        {
            return STATUS_GCODE_UNSUPPORTED_COMMAND;
        }

        switch (code)
        {
        case 60:
            code = 5;
        case 30:
            code = (code & 1) ? 5 : 3;
        case 0:
        case 1:
        case 2:
            new_group |= GCODE_GROUP_STOPPING;
            new_state->groups.stopping = code + 1;
            break;
#ifdef USE_SPINDLE
        case 3:
        case 4:
        case 5:
            new_group |= GCODE_GROUP_SPINDLE;
            code = (code == 5) ? M5 : code - 2;
            new_state->groups.spindle_turning = code;
            break;
#endif
        case 6:
            new_group |= GCODE_GROUP_TOOLCHANGE;
            new_state->groups.tool_change = M6;
            break;
#ifdef USE_COOLANT
#ifdef COOLANT_MIST
        case 7:
#endif
#ifdef M7_SAME_AS_M8
        case 7:
#endif
        case 8:
            cmd->groups |= GCODE_GROUP_COOLANT; //word overlapping allowed
#ifdef COOLANT_MIST
            new_state->groups.coolant |= ((code == 8) ? M8 : M7);
#else
            new_state->groups.coolant |= M8;
#endif
            return STATUS_OK;
        case 9:
            cmd->groups |= GCODE_GROUP_COOLANT;
            new_state->groups.coolant = M9;
            return STATUS_OK;
#endif
        case 48:
        case 49:
            new_group |= GCODE_GROUP_ENABLEOVER;
            code = (code == 48) ? M48 : M49;
            new_state->groups.feed_speed_override = code;
            break;
        default:
            return STATUS_GCODE_UNSUPPORTED_COMMAND;
        }

        if ((new_group & ~GCODE_GROUP_COOLANT) == (cmd->groups & ~GCODE_GROUP_COOLANT))
        {
            return STATUS_GCODE_MODAL_GROUP_VIOLATION;
        }

        cmd->groups = new_group;
        return STATUS_OK;
    }

    static uint8_t parser_letter_word(unsigned char c, float value, uint8_t mantissa, parser_words_t *words, parser_cmd_explicit_t *cmd)
    {
        uint16_t new_group = cmd->groups;

        switch (c)
        {
        case 'N':
            //doesn't need processing (if it fails to be in the begining of the line throws error)
            break;
#ifdef AXIS_X
        case 'X':
            cmd->words |= GCODE_WORD_X;
            words->xyzabc[AXIS_X] = value;
            break;
#endif
#ifdef AXIS_Y
        case 'Y':
            cmd->words |= GCODE_WORD_Y;
            words->xyzabc[AXIS_Y] = value;
            break;
#endif
#ifdef AXIS_Z
        case 'Z':
            cmd->words |= GCODE_WORD_Z;
            words->xyzabc[AXIS_Z] = value;
            break;
#endif
#ifdef AXIS_A
        case 'A':
#ifdef GCODE_ACCEPT_WORD_E
        case 'E':
#endif
            cmd->words |= GCODE_WORD_A;
            words->xyzabc[AXIS_A] = value;
            break;
#endif
#ifdef AXIS_B
        case 'B':
            cmd->words |= GCODE_WORD_B;
            words->xyzabc[AXIS_B] = value;
            break;
#endif
#ifdef AXIS_C
        case 'C':
            cmd->words |= GCODE_WORD_C;
            words->xyzabc[AXIS_C] = value;
            break;
#endif
        case 'D':
            cmd->words |= GCODE_WORD_D;
            words->d = value;
            break;
        case 'F':
            cmd->words |= GCODE_WORD_F;
            words->f = value;
            break;
#ifdef AXIS_X
        case 'I':
            cmd->words |= GCODE_WORD_I;
            words->ijk[AXIS_X] = value;
            break;
#endif
#ifdef AXIS_Y
        case 'J':
            cmd->words |= GCODE_WORD_J;
            words->ijk[AXIS_Y] = value;
            break;
#endif
#ifdef AXIS_Z
        case 'K':
            cmd->words |= GCODE_WORD_K;
            words->ijk[AXIS_Z] = value;
            break;
#endif
        case 'L':
            cmd->words |= GCODE_WORD_L;

            if (mantissa)
            {
                return STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER;
            }

            words->l = (uint8_t)truncf(value);
            break;
        case 'P':
            cmd->words |= GCODE_WORD_P;

            if (value < 0)
            {
                return STATUS_NEGATIVE_VALUE;
            }

            words->p = value;
            break;
        case 'R':
            cmd->words |= GCODE_WORD_R;
            words->r = value;
            break;
#ifdef USE_SPINDLE
        case 'S':
            cmd->words |= GCODE_WORD_S;
            if (value < 0)
            {
                return STATUS_NEGATIVE_VALUE;
            }
            words->s = (uint16_t)trunc(value);
            break;
#endif
        case 'T':
            cmd->words |= GCODE_WORD_T;
            if (mantissa)
            {
                return STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER;
            }

            if (value < 0)
            {
                return STATUS_NEGATIVE_VALUE;
            }

            words->t = (uint8_t)trunc(value);
            break;
        default:
            if (c >= 'A' && c <= 'Z') //invalid recognized char
            {
#ifdef IGNORE_UNDEFINED_AXIS
                if (c <= 'C' || c >= 'X') //ignore undefined axis chars
                {
                    return STATUS_OK;
                }
#endif
                return STATUS_GCODE_UNUSED_WORDS;
            }
            return STATUS_INVALID_STATEMENT;
        }

        return STATUS_OK;
    }

    static unsigned char parser_get_next_preprocessed(bool peek)
    {
        unsigned char c = serial_peek();

        while (c == ' ' || c == '(')
        {
            serial_getc();
            if (c == '(')
            {
                parser_get_comment();
            }
            c = serial_peek();
        }

        if (!peek)
        {
            serial_getc();
        }

        return c;
    }

    static void parser_discard_command(void)
    {
        unsigned char c = '@';
#ifdef ECHO_CMD
        serial_putc(c);
#endif
        do
        {
            c = serial_getc();
#ifdef ECHO_CMD
            serial_putc(c);
#endif
        } while (c != EOL);
    }

    static void parser_reset(void)
    {
        parser_state.groups.coord_system = G54;               //G54
        parser_state.groups.plane = G17;                      //G17
        parser_state.groups.feed_speed_override = M48;        //M48
        parser_state.groups.cutter_radius_compensation = G40; //G40
        parser_state.groups.distance_mode = G90;              //G90
        parser_state.groups.feedrate_mode = G94;              //G94
        parser_state.groups.tool_length_offset = G49;         //G49
#ifdef USE_COOLANT
        parser_state.groups.coolant = M9; //M9
#endif
#ifdef USE_SPINDLE
        parser_state.groups.spindle_turning = M5; //M5
#endif
        parser_state.groups.motion = G1;                                               //G1
        parser_state.groups.units = G21;                                               //G21
        memset(parser_parameters.g92_offset, 0, sizeof(parser_parameters.g92_offset)); //G92.2
    }

    //loads parameters
    //loads G92 offset
    //loads G54 coordinate system
    //also checks all other coordinate systems and homing positions
    void parser_parameters_load(void)
    {
        const uint8_t size = PARSER_PARAM_SIZE;

        //loads G92
        if (settings_load(G92ADDRESS, (uint8_t *)&parser_parameters.g92_offset, PARSER_PARAM_SIZE))
        {
            memset(parser_parameters.g92_offset, 0, sizeof(parser_parameters.g92_offset));
            settings_erase(G92ADDRESS, PARSER_PARAM_SIZE);
        }

        for (uint8_t i = 1; i < G92OFFSET; i++)
        {
            if (settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (i * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)&parser_parameters.coord_system_offset, PARSER_PARAM_SIZE))
            {
                settings_erase(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (i * PARSER_PARAM_ADDR_OFFSET), PARSER_PARAM_SIZE);
            }
        }

        //load G54
        if (settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET, (uint8_t *)&parser_parameters.coord_system_offset, PARSER_PARAM_SIZE))
        {
            memset(parser_parameters.coord_system_offset, 0, sizeof(parser_parameters.coord_system_offset));
            settings_erase(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET, PARSER_PARAM_SIZE);
        }
    }

#ifdef __cplusplus
}
#endif
