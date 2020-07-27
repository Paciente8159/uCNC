/*
	Name: parser.c
	Description: Parses Grbl system commands and RS274NGC (GCode) commands
        The RS274NGC parser tries to follow the standard document version 3 as close as possible.
        The parsing is done in 3 steps:
            - Tockenization; Converts the command string to a structure with GCode parameters
            - Validation; Validates the command by checking all the parameters (part 3.5 - 3.7 of the document)
            - Execution; Executes the command by the orther set in part 3.8 of the document.

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

#include "config.h"
#include "mcu.h"
#include "grbl_interface.h"
#include "utils.h"
#include "settings.h"
#include "serial.h"
#include "protocol.h"
#include "planner.h"
#include "motion_control.h"
#include "io_control.h"
#include "interpolator.h"
#include "cnc.h"
#include "parser.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <float.h>

//group0 offsets
#define GCODE_GROUP_MOTION 1
#define GCODE_GROUP_PLANE 2
#define GCODE_GROUP_DISTANCE 4
#define GCODE_GROUP_FEEDRATE 8
#define GCODE_GROUP_UNITS 16
#define GCODE_GROUP_CUTTERRAD 32
#define GCODE_GROUP_TOOLLENGTH 64
#define GCODE_GROUP_RETURNMODE 128
//group1 offsets
#define GCODE_GROUP_COORDSYS 1
#define GCODE_GROUP_PATH 2
#define GCODE_GROUP_STOPPING 4
#define GCODE_GROUP_TOOLCHANGE 8
#define GCODE_GROUP_SPINDLE 16
#define GCODE_GROUP_COOLANT 32
#define GCODE_GROUP_ENABLEOVER 64
#define GCODE_GROUP_NONMODAL 128
//word0 offsets
#define GCODE_WORD_X 1
#define GCODE_WORD_Y 2
#define GCODE_WORD_Z 4
#define GCODE_WORD_A 8
#define GCODE_WORD_B 16
#define GCODE_WORD_C 32
#define GCODE_WORD_D 64
#define GCODE_WORD_F 128
//word1 offsets
#define GCODE_WORD_I GCODE_WORD_X //matches X axis bit
#define GCODE_WORD_J GCODE_WORD_Y //matches Y axis bit
#define GCODE_WORD_K GCODE_WORD_Z //matches Z axis bit
#define GCODE_WORD_H 8
#define GCODE_WORD_L 16
#define GCODE_WORD_P 32
#define GCODE_WORD_Q 64
#define GCODE_WORD_R 128
//word2 offsets
#define GCODE_WORD_S 1
#define GCODE_WORD_T 2
#define GCODE_WORD_N 4

#define GCODE_MAIN_AXIS (GCODE_WORD_X | GCODE_WORD_Y | GCODE_WORD_Z)
#define GCODE_ALL_AXIS (GCODE_MAIN_AXIS | GCODE_WORD_A | GCODE_WORD_B | GCODE_WORD_C)
#define GCODE_XYPLANE_AXIS (GCODE_WORD_X | GCODE_WORD_Y)
#define GCODE_XZPLANE_AXIS (GCODE_WORD_X | GCODE_WORD_Z)
#define GCODE_YZPLANE_AXIS (GCODE_WORD_Y | GCODE_WORD_Z)

#define PARSER_PARAM_SIZE (sizeof(float) * AXIS_COUNT) //parser parameters array size
#define PARSER_PARAM_ADDR_OFFSET (PARSER_PARAM_SIZE + 1) //parser parameters array size + 1 crc byte
#define G28HOME COORD_SYS_COUNT //G28 index
#define G30HOME COORD_SYS_COUNT + 1 //G30 index
#define G92OFFSET COORD_SYS_COUNT + 2 //G92 index

#define PARSER_CORDSYS_ADDRESS SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET //1st coordinate system offset eeprom address (G54)
#define G28ADDRESS (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (PARSER_PARAM_ADDR_OFFSET * G28HOME)) //G28 coordinate offset eeprom address
#define G30ADDRESS (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (PARSER_PARAM_ADDR_OFFSET * G30HOME)) //G28 coordinate offset eeprom address
#define G92ADDRESS (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (PARSER_PARAM_ADDR_OFFSET * G92OFFSET)) //G92 coordinate offset eeprom address

typedef struct
{
    //group1
    uint8_t motion : 5;
    uint8_t plane : 2;
    uint8_t distance_mode : 1;
    uint8_t feedrate_mode : 1;

    uint8_t units : 1;
    uint8_t cutter_radius_compensation : 2;
    uint8_t tool_length_offset : 1;
    uint8_t return_mode : 1;
    uint8_t coord_system : 3;

    uint8_t path_mode : 2;
    uint8_t stopping : 3;
    uint8_t tool_change : 1;
#ifdef USE_SPINDLE
    uint8_t spindle_turning : 2;
#endif
#ifdef USE_COOLANT
    uint8_t coolant : 2;
#endif
    uint8_t feed_speed_override : 1;
    uint8_t nonmodal : 4; //reset to 0 in every line (non persistent)
    //uint8_t : 1;		  //unused
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
    float d;
    float f;
    float h;
    float ijk[3];
    uint8_t l;
    float p;
    float q;
    float r;
#ifdef GCODE_PROCESS_LINE_NUMBERS
    uint32_t n;
#endif
#ifdef USE_SPINDLE
    int16_t s;
#endif
    uint8_t t;
} parser_words_t;

typedef struct
{
    parser_groups_t groups;
    float feedrate;
#ifdef USE_SPINDLE
	int16_t spindle;
#endif
	uint8_t tool_index;
} parser_state_t;

static parser_state_t parser_state;
static parser_parameters_t parser_parameters;
//contains all bitflags for the used groups/words on the parsed command
static uint8_t parser_group0;
static uint8_t parser_group1;
static uint8_t parser_word0;
static uint8_t parser_word1;
static uint8_t parser_word2;

static uint8_t parser_wco_counter;

static void parser_reset();
static bool parser_get_comment();
static bool parser_get_float(float *value, bool *isinteger);
static uint8_t parser_fetch_command(parser_state_t *new_state, parser_words_t *words);
static uint8_t parser_validate_command(parser_state_t *new_state, parser_words_t *words);
static uint8_t parser_exec_command(parser_state_t *new_state, parser_words_t *words);
static void parser_load_parameters();
static uint8_t parser_eat_next_char(unsigned char c);
static void parser_discard_command();

/*
	Initializes the gcode parser
*/
void parser_init(void)
{
#ifdef FORCE_GLOBALS_TO_0
    memset(&parser_state, 0, sizeof(parser_state_t));
    memset(&parser_parameters, 0, sizeof(parser_parameters_t));
#endif
    parser_load_parameters();
    parser_reset();
}

/*
	Parse the next gcode line available in the buffer and send it to the motion controller
*/
uint8_t parser_gcode_command(void)
{
    uint8_t result = 0;
    //initializes new state
    parser_state_t next_state = {};
    parser_words_t words = {};
    //next state will be the same as previous except for nonmodal group (is set with 0)
    memcpy(&next_state, &parser_state, sizeof(parser_state_t));
    next_state.groups.nonmodal = 0; //reset nonmodal

    //fetch command
    result = parser_fetch_command(&next_state, &words);
    if (result != STATUS_OK)
    {
        parser_discard_command();//if error during fetch discard the rest of the line
        return result;
    }

    //validates command
    result = parser_validate_command(&next_state, &words);
    if (result != STATUS_OK)
    {
        return result;
    }

    //executes command
    result = parser_exec_command(&next_state, &words);
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

uint8_t parse_grbl_error_code(uint8_t code)
{
    switch(code)
    {
        case GRBL_SEND_SYSTEM_SETTINGS:
            protocol_send_ucnc_settings();
            break;
        case GRBL_SEND_COORD_SYSTEM:
            protocol_send_gcode_coordsys();
            break;
        case GRBL_SEND_PARSER_MODES:
            protocol_send_gcode_modes();
            break;
        case GRBL_SEND_STARTUP_BLOCKS:
            protocol_send_start_blocks();
            break;
        case GRBL_SEND_CHECKMODE_ON:
            protocol_send_string(MSG_FEEDBACK_4);
            break;
        case GRBL_SEND_CHECKMODE_OFF:
                cnc_stop();
                cnc_alarm(EXEC_ALARM_RESET);
                protocol_send_string(MSG_FEEDBACK_5);
            break;
        case GRBL_SEND_SETTINGS_RESET:
            protocol_send_string(MSG_FEEDBACK_9);
            break;
        case GRBL_UNLOCK:
            cnc_unlock();
            if(cnc_get_exec_state(EXEC_DOOR))
            {
                return STATUS_CHECK_DOOR;
            }
            protocol_send_string(MSG_FEEDBACK_3);
            break;
        case GRBL_HOME:
            if (!g_settings.homing_enabled)
            {
                return STATUS_SETTING_DISABLED;
                break;
            }

            cnc_unlock();
            if(cnc_get_exec_state(EXEC_DOOR))
            {
                return STATUS_CHECK_DOOR;
            }

            cnc_home();
            break;
        case GRBL_HELP:
            protocol_send_string(MSG_HELP);
            break;
        default:
            return code;
    }

    return STATUS_OK;
}

uint8_t parser_grbl_command(void)
{
    //if not IDLE
    if (cnc_get_exec_state(EXEC_RUN))
    {
        parser_discard_command();
        return STATUS_IDLE_ERROR;
    }
    serial_getc();
    unsigned char c = serial_peek();
	uint16_t block_address;
    uint8_t error = STATUS_OK;
    switch(c)
    {
        case '$':
        case '#':
        case 'H':
        case 'X':
        case 'G':
        case 'C':
            serial_getc();
            error = parser_eat_next_char(EOL);
            break;
        case 'J':
            serial_getc();
            if(parser_eat_next_char('='))
            {
                error = STATUS_INVALID_JOG_COMMAND;
                break;
            }
            if(cnc_get_exec_state(EXEC_ALLACTIVE)) //Jog only allowed in IDLE
            {
                error = STATUS_IDLE_ERROR;
                break;
            }
            break;
        case 'N':
        	serial_getc();
        	c = serial_getc();
        	switch(c)
        	{
        		case '0':
        		case '1':
        			block_address = (!(c - '0') ? STARTUP_BLOCK0_ADDRESS_OFFSET : STARTUP_BLOCK1_ADDRESS_OFFSET);
        			if(parser_eat_next_char('='))
        			{
        				error = STATUS_INVALID_STATEMENT;
					}
					break;
				case EOL:
					return GRBL_SEND_STARTUP_BLOCKS;
				default:
					error = STATUS_INVALID_STATEMENT;
					break;
			}
			c = 'N';
			break;			
        case 'R':
            serial_getc();
            if(parser_eat_next_char('S'))
            {
                break;
            }
            if(parser_eat_next_char('T'))
            {
                break;
            }
            if(parser_eat_next_char('='))
            {
                break;
            }
            break;
        case EOL:
            serial_getc();
            return GRBL_HELP;
    }

    if (error)
    {
        parser_discard_command(); //discards the rest of the line
        return error;
    }

    error = STATUS_OK;
    parser_state_t next_state = {};
	parser_words_t words = {};
    switch (c)
    {
        case '$':
            return GRBL_SEND_SYSTEM_SETTINGS;
        case '#':
            return GRBL_SEND_COORD_SYSTEM;
        case 'H':
            return GRBL_HOME;
            break;
        case 'X':
            return GRBL_UNLOCK;
        case 'G':
            return GRBL_SEND_PARSER_MODES;
        case 'R':
            c = serial_peek();
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
                    error = STATUS_INVALID_STATEMENT;
                    break;
            }
            serial_getc();
            error = parser_eat_next_char(EOL);
            return ((!error) ? GRBL_SEND_SETTINGS_RESET : error);
        case 'J': //jog command
            /*
            		The jog command is parsed like an emulated G1 command without changing the parser state
            		1. Fetch the command
            		2. Validates the command (in jog mode)
            		3. Executes it as a G1 command
            	*/

            cnc_set_exec_state(EXEC_JOG);
            return parser_gcode_command();
        case 'N':
        	error = parser_fetch_command(&next_state, &words);
        	if(error)
        	{
        		break;
			}
			error = parser_validate_command(&next_state, &words);
        	if(error)
        	{
        		break;
			}
			//everything ok reverts string and saves it
		    do
		    {
		        serial_ungetc();
		    } while (serial_peek() != '=');
		    serial_getc();
			settings_save_startup_gcode(block_address);
            break;
        case 'C':
            //toggles motion control check mode
            if (mc_toogle_checkmode())
            {
                return GRBL_SEND_CHECKMODE_ON;
            }
            else
            {
                return GRBL_SEND_CHECKMODE_OFF;
            }
            break;
        default:
            if (c >= '0' && c <= '9') //settings
            {
                float val = 0;
                uint8_t setting_num = 0;
                bool isinteger = false;

                parser_get_float(&val, &isinteger);

                if (!isinteger || val > 255 || val < 0)
                {
                    error = STATUS_INVALID_STATEMENT;
                    break;
                }

                setting_num = (uint8_t)val;
                //eat '='
                if (parser_eat_next_char('='))
                {
                    error = STATUS_INVALID_STATEMENT;
                    break;
                }

                val = 0;
                isinteger = false;
                parser_get_float(&val, &isinteger);

                if (parser_eat_next_char(EOL))
                {
                    error = STATUS_INVALID_STATEMENT;
                }

                return settings_change(setting_num, val);
            }
            error =  STATUS_INVALID_STATEMENT;
            break;
    }

    if (error)
    {
        parser_discard_command();
    }
    return error;
}

void parser_get_modes(uint8_t *modalgroups, uint16_t *feed, uint16_t *spindle)
{
    modalgroups[0] = parser_state.groups.motion;
    modalgroups[1] = parser_state.groups.plane + 17;
    modalgroups[2] = parser_state.groups.distance_mode + 90;
    modalgroups[3] = parser_state.groups.units + 20;
    modalgroups[4] = parser_state.groups.coord_system + 54;
#ifdef USE_SPINDLE
    modalgroups[5] = parser_state.groups.spindle_turning + 3;
    *spindle = (uint16_t)ABS(parser_state.spindle);
#endif
#ifdef USE_COOLANT
    modalgroups[6] = 9 - parser_state.groups.coolant;
#endif
    modalgroups[7] = parser_state.groups.feed_speed_override + 48;
#ifdef USE_TOOL_CHANGER
    modalgroups[8] = parser_state.tool_index;
#endif
    *feed = (uint16_t)parser_state.feedrate;
}

void parser_get_coordsys(uint8_t system_num, float* axis)
{
    switch (system_num)
    {
        case 255:
        	memcpy(axis, parser_parameters.last_probe_position, sizeof(parser_parameters.last_probe_position));
            break;
        case 254:
        	memcpy(axis, (float*)&parser_parameters.tool_length_offset, sizeof(float));
            break;
        case 28:
            settings_load(G28ADDRESS, (uint8_t*)axis, PARSER_PARAM_SIZE);
            break;
        case 30:
            settings_load(G30ADDRESS, (uint8_t*)axis, PARSER_PARAM_SIZE);
            break;
        case 92:
            memcpy(axis, parser_parameters.g92_offset, sizeof(parser_parameters.g92_offset));
            break;
        default:
            settings_load(PARSER_CORDSYS_ADDRESS + (system_num * PARSER_PARAM_ADDR_OFFSET), (uint8_t*)axis, PARSER_PARAM_SIZE);
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
    memset(&parser_parameters.coord_system_offset, 0, sizeof(parser_parameters.coord_system_offset));
    for(uint8_t i = 0; i < COORD_SYS_COUNT; i++)
    {
        settings_erase(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (i * PARSER_PARAM_ADDR_OFFSET), PARSER_PARAM_SIZE);
	}

    //erase G92
    settings_erase(G92ADDRESS, PARSER_PARAM_SIZE);
}

bool parser_get_wco(float *axis)
{
    if(!parser_wco_counter)
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

#ifdef USE_COOLANT
void parser_toogle_coolant(uint8_t state)
{
    parser_update_coolant(parser_state.groups.coolant ^ state);
}

void parser_update_coolant(uint8_t state)
{
    parser_state.groups.coolant = state;

    switch (parser_state.groups.coolant)
    {
        case 0: //off
            mcu_clear_output(COOLANT_FLOOD);
			mcu_clear_output(COOLANT_MIST);
            break;
        case 1: //flood
            mcu_clear_output(COOLANT_MIST);
            mcu_set_output(COOLANT_FLOOD);
            break;
        case 2: //mist
            mcu_clear_output(COOLANT_FLOOD);
            mcu_set_output(COOLANT_MIST);
            break;
        case 3: //flood and mist
            mcu_set_output(COOLANT_FLOOD);
            mcu_set_output(COOLANT_MIST);
            break;
    }
}
#endif

/*
	Parses a string to number (real)
	If the number is an integer the isinteger flag is set
	The string pointer is also advanced to the next position
*/
bool parser_get_float(float *value, bool *isinteger)
{
    bool isnegative = false;
    bool isfloat = false;
    uint32_t intval = 0;
    uint8_t fpcount = 0;
    bool result = false;

    char c = serial_peek();

    *value = 0;

    if (c == '-' || c == '+')
    {
        isnegative = (c == '-');
        serial_getc();
    }

    for (;;)
    {
        c = serial_peek();
        uint8_t digit = (uint8_t)c - 48;
        if (digit <= 9)
        {
            intval = fast_mult10(intval) + digit;
            if (isfloat)
            {
                fpcount++;
            }

            result = true;
        }
        else if (c == '.' && !isfloat)
        {
            isfloat = true;
        }
        else
        {
            if (!result)
            {
                return result;
            }
            break;
        }

        serial_getc();
    }

    *value = (float)intval;

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

    }
    while (fpcount != 0);

    *isinteger = !isfloat;

    if (isnegative)
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
bool parser_get_comment(void)
{
    uint8_t msg_parser = 0;
    for(;;)
    {
        unsigned char c = serial_peek();
        switch(c)
        {
                //case '(':	//error under RS274NGC (commented for Grbl compatibility)
            case ')':	//OK
                serial_getc();
            case EOL:	//error under RS274NGC (ignore for Grbl compatibility)
            case OVF:
#ifdef PROCESS_COMMENTS
                if(msg_parser==4)
                {
                    protocol_send_string(MSG_END);
                }
#endif
                //return ((c==')') ? true : false); //under RS274NGC (commented for Grbl compatibility)
                return true;
#ifdef PROCESS_COMMENTS
            default:
                switch(msg_parser)
                {
                    case 0:
                        msg_parser = (c=='M') ? 1 : 0xFF;
                        break;
                    case 1:
                        msg_parser = (c=='S') ? 2 : 0xFF;
                        break;
                    case 2:
                        msg_parser = (c=='G') ? 3 : 0xFF;
                        break;
                    case 3:
                        msg_parser = (c==',') ? 4 : 0xFF;
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

    return false;//never reached here
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
uint8_t parser_fetch_command(parser_state_t *new_state, parser_words_t *words)
{
    bool hasnumber = false;
    float word_val = 0.0;
    unsigned char word = EOL;
    uint8_t code = 0;
    uint8_t wordcount = 0;
    uint8_t mwords = 0;

    //flags for used words and groups
    parser_group0 = 0;
    parser_group1 = 0;
    parser_word0 = 0;
    parser_word1 = 0;
    parser_word2 = 0;

    for (;;)
    {
        uint8_t *word_group = &parser_group0;
        uint8_t word_group_val = 0;

        word = serial_getc();
        bool isinteger = false;

        switch (word)
        {
            case EOL: //EOL
                return STATUS_OK;
            case OVF:
                return STATUS_OVERFLOW;
            case ' ': //eat white space
                word_group = NULL;
                break;
            case '(':
                word_group = NULL;
                if(!parser_get_comment())
                {
                    return STATUS_BAD_COMMENT_FORMAT;//error code
                }
                break;
            default:
                if (word < 'A' || word > 'Z') //invalid recognized char
                {
                    return STATUS_EXPECTED_COMMAND_LETTER;
                }

                if (!parser_get_float(&word_val, &isinteger))
                {
                    return STATUS_BAD_NUMBER_FORMAT;
                }
                break;
        }

        switch (word)
        {
            case 'G':
                code = (uint8_t)floorf(word_val);
                //check mantissa
                uint8_t mantissa = 0;
                if (!isinteger)
                {
                    switch (code)
                    {
                            //codes with possible mantissa
                        case 38:
                        case 43:
                        case 59:
                        case 61:
                        case 92:
                            mantissa = (uint8_t)roundf((word_val - code) * 100.0f);
                            break;
                        default:
                            return STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER;
                            break;
                    }
                }

                switch (code)
                {
                        //motion codes
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 38: //check if 38.2
                    case 80:
                    case 81:
                    case 82:
                    case 83:
                    case 84:
                    case 85:
                    case 86:
                    case 87:
                    case 88:
                    case 89:
                        if (code == 38)
                        {
                            code = 4;
                            switch (mantissa)
                            {
                                case 50:
                                    code++;
                                case 40:
                                    code++;
                                case 30:
                                    code++;
                                case 20:
                                    break;
                                default:
                                    return STATUS_INVALID_STATEMENT;
                            }
                        }
                        else if (code >= 80)
                        {
                            code -= 72;
                            // Canned cycles not supported
                            return STATUS_GCODE_UNSUPPORTED_COMMAND;
                        }
                        else if (!isinteger)
                        {
                            return STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER;
                        }

                        word_group_val = GCODE_GROUP_MOTION;
                        new_state->groups.motion = code;
                        break;
                    case 17:
                    case 18:
                    case 19:
                        word_group_val = GCODE_GROUP_PLANE;
                        code -= 17;
                        new_state->groups.plane = code;
                        break;
                    case 90:
                    case 91:
                        word_group_val = GCODE_GROUP_DISTANCE;
                        code -= 90;
                        new_state->groups.distance_mode = code;
                        break;
                    case 93:
                    case 94:
                        word_group_val = GCODE_GROUP_FEEDRATE;
                        code -= 93;
                        new_state->groups.feedrate_mode = code;
                        break;
                    case 20:
                    case 21:
                        word_group_val = GCODE_GROUP_UNITS;
                        code -= 20;
                        new_state->groups.units = code;
                        break;
                    case 40:
                    case 41:
                    case 42:
                        word_group_val = GCODE_GROUP_CUTTERRAD;
                        code -= 40;
                        new_state->groups.cutter_radius_compensation = code;
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
                                    return STATUS_INVALID_STATEMENT;
                            }
                        }
                        new_state->groups.tool_length_offset = ((code == 49) ? 0 : 1);
                        word_group_val = GCODE_GROUP_TOOLLENGTH;
                        break;
                    case 98:
                    case 99:
                        word_group_val = GCODE_GROUP_RETURNMODE;
                        code -= 98;
                        new_state->groups.return_mode = code;
                        break;
                    case 54:
                    case 55:
                    case 56:
                    case 57:
                    case 58:
                    case 59:
                        word_group = &parser_group1;
                        word_group_val = GCODE_GROUP_COORDSYS;
                        code -= 54;
                        switch (mantissa)
                        {
                            case 0:
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
                        switch(mantissa)
                        {
                            case 0:
                                break;
                            case 10:
                                code += 1;
                                break;
                            default:
                                return STATUS_GCODE_UNSUPPORTED_COMMAND;
                        }
                        word_group = &parser_group1;
                        word_group_val = GCODE_GROUP_PATH;
                        new_state->groups.path_mode = code;
                        break;
                    case 4:
                    case 53:
                    case 10:
                    case 28:
                    case 30:
                    case 92:
                        //convert code within 4 bits without
                        //4 = 0
                        //10 = 1
                        //28 = 2
                        //30 = 3
                        //53 = 5
                        //92 = 9
                        //92.1 = 10
                        //92.2 = 11
                        //92.3 = 12
                        code = (uint8_t)floorf(word_val * 0.10001f);
                        switch (mantissa)
                        {
                            case 0:
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

                        word_group = &parser_group1;
                        word_group_val = GCODE_GROUP_NONMODAL;
                        new_state->groups.nonmodal = code;
                        break;

                    default:
                        return STATUS_GCODE_UNSUPPORTED_COMMAND;
                }
                break;

            case 'M':
                if (!isinteger)
                {
                    return STATUS_GCODE_UNSUPPORTED_COMMAND;
                }
                word_group = &parser_group1;
                //counts number of M commands
                mwords++;
                code = (uint8_t)(word_val);
                switch (code)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 30:
                    case 60:
                        word_group_val = GCODE_GROUP_STOPPING;
                        if (code >= 10)
                        {
                            code /= 10;
                        }
                        new_state->groups.stopping = code;
                        break;
#ifdef USE_SPINDLE
                    case 3:
                    case 4:
                    case 5:
                        word_group_val = GCODE_GROUP_SPINDLE;
                        code -= 3;
                        new_state->groups.spindle_turning = code;
                        break;
#endif
                    case 6:
                        word_group_val = GCODE_GROUP_TOOLCHANGE;
                        new_state->groups.tool_change = 1;
                        break;
#ifdef USE_COOLANT
                    case 7:
                    case 8:
                        SETFLAG(parser_group1, GCODE_GROUP_COOLANT);
                        word_group_val = 0;
                        new_state->groups.coolant |= (9 - code);
                        break;
                    case 9:
                        TOGGLEFLAG(parser_group1, GCODE_GROUP_COOLANT);
                        word_group_val = 0;
                        new_state->groups.coolant = 0;
                        break;
#endif
                    case 48:
                    case 49:
                        word_group_val = GCODE_GROUP_ENABLEOVER;
                        code -= 48;
                        new_state->groups.feed_speed_override = code;
                        break;
                    default:
                        return STATUS_GCODE_UNSUPPORTED_COMMAND;
                }
                break;
            case 'N':
#ifdef GCODE_PROCESS_LINE_NUMBERS
                word_group = &parser_word2;
                word_group_val = GCODE_WORD_N;
                if (!isinteger || wordcount != 0 || word_val < 0)
                {
                    return STATUS_GCODE_INVALID_LINE_NUMBER;
                }

                words->n = trunc(word_val);
#else
                word_group = NULL;
#endif
                break;
#ifdef AXIS_X
            case 'X':
                word_group = &parser_word0;
                word_group_val = GCODE_WORD_X;
                words->xyzabc[0] = word_val;
                break;
#endif
#ifdef AXIS_Y
            case 'Y':
                word_group = &parser_word0;
                word_group_val = GCODE_WORD_Y;
                words->xyzabc[1] = word_val;
                break;
#endif
#ifdef AXIS_Z
            case 'Z':
                word_group = &parser_word0;
                word_group_val = GCODE_WORD_Z;
                words->xyzabc[2] = word_val;
                break;
#endif
#ifdef AXIS_A
            case 'A':
#ifdef GCODE_ACCEPT_WORD_E
            case 'E':
#endif
                word_group = &parser_word0;
                word_group_val = GCODE_WORD_A;
                words->xyzabc[3] = word_val;
                break;
#endif
#ifdef AXIS_B
            case 'B':
                word_group = &parser_word0;
                word_group_val = GCODE_WORD_B;
                words->xyzabc[4] = word_val;
                break;
#endif
#ifdef AXIS_C
            case 'C':
                word_group = &parser_word0;
                word_group_val = GCODE_WORD_C;
                words->xyzabc[5] = word_val;
                break;
#endif
            case 'D':
                word_group = &parser_word0;
                word_group_val = GCODE_WORD_D;
                words->d = word_val;
                break;
            case 'F':
                word_group = &parser_word0;
                word_group_val = GCODE_WORD_F;
                words->f = word_val;
                break;
            case 'H':
                word_group = &parser_word1;
                word_group_val = GCODE_WORD_H;
                words->h = word_val;
                break;
#ifdef AXIS_X
            case 'I':
                word_group = &parser_word1;
                word_group_val = GCODE_WORD_I;
                words->ijk[0] = word_val;
                break;
#endif
#ifdef AXIS_Y
            case 'J':
                word_group = &parser_word1;
                word_group_val = GCODE_WORD_J;
                words->ijk[1] = word_val;
                break;
#endif
#ifdef AXIS_Z
            case 'K':
                word_group = &parser_word1;
                word_group_val = GCODE_WORD_K;
                words->ijk[2] = word_val;
                break;
#endif
            case 'L':
                word_group = &parser_word1;
                word_group_val = GCODE_WORD_L;

                if (!isinteger)
                {
                    return STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER;
                }

                words->l = (uint8_t)floorf(word_val);
                break;
            case 'P':
                word_group = &parser_word1;
                word_group_val = GCODE_WORD_P;

                if (word_val < 0)
                {
                    return STATUS_NEGATIVE_VALUE;
                }

                words->p = word_val;
                break;
            case 'Q':
                word_group = &parser_word1;
                word_group_val = GCODE_WORD_Q;
                words->q = word_val;
                break;
            case 'R':
                word_group = &parser_word1;
                word_group_val = GCODE_WORD_R;
                words->r = word_val;
                break;
#ifdef USE_SPINDLE
            case 'S':
                word_group = &parser_word2;
                word_group_val = GCODE_WORD_S;
                words->s = (uint16_t)roundf(word_val);

                break;
#endif
            case 'T':
                word_group = &parser_word2;
                word_group_val = GCODE_WORD_T;

                if (!isinteger)
                {
                    return STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER;
                }

                words->t = (uint8_t)floorf(word_val);
                break;
            default:
                if (word >= 'A' && word <= 'Z') //invalid recognized char
                {
                    return STATUS_GCODE_UNSUPPORTED_COMMAND;
                }
                break;
        }

        if(word_group != NULL)
        {
            wordcount++;
            if(word_group_val)
            {
                TOGGLEFLAG(*word_group, word_group_val);
            }
            else
            {
                word_group_val = GCODE_GROUP_COOLANT;
            }
            if (!CHECKFLAG(*word_group, word_group_val))
            {
                if (word_group == &parser_group0 || word_group == &parser_group1)
                {
                    return STATUS_GCODE_MODAL_GROUP_VIOLATION;
                }
                return STATUS_GCODE_WORD_REPEATED;
            }
        }
    }

    //The string is read and all tockens are identified
    //The command can now be checked for validation before being executed
    return STATUS_OK;
}

/*
	STEP 2
	Validadates command by checking for errors on all G/M Codes
		RS274NGC v3 - 3.5 G Codes
		RS274NGC v3 - 3.6 Input M Codes
		RS274NGC v3 - 3.7 Other Input Codes
*/

uint8_t parser_validate_command(parser_state_t *new_state, parser_words_t *words)
{
    //only alow groups 3, 6 and modal G53
    if (cnc_get_exec_state(EXEC_JOG))
    {
        if (parser_group0 & ~(GCODE_GROUP_DISTANCE | GCODE_GROUP_UNITS))
        {
            return STATUS_INVALID_JOG_COMMAND;
        }

        if (parser_group1 & ~GCODE_GROUP_NONMODAL)
        {
            return STATUS_INVALID_JOG_COMMAND;
        }

        //if nonmodal other than G53
        if (new_state->groups.nonmodal != 0 && new_state->groups.nonmodal != 5)
        {
            return STATUS_INVALID_JOG_COMMAND;
        }

        if ((parser_word0 & GCODE_WORD_D) || parser_word1 || (parser_word2 & GCODE_WORD_N))
        {
            return STATUS_INVALID_JOG_COMMAND;
        }
    }

    if (cnc_get_exec_state(EXEC_JOG)) // if is jog emulates a G1 motion code
    {
        //if any motion is declared the jog is invalid
        if ((parser_group0 & GCODE_GROUP_MOTION))
        {
            return STATUS_INVALID_JOG_COMMAND;
        }

        //emulates G1
        new_state->groups.motion = 1;
        SETFLAG(parser_group0, GCODE_GROUP_MOTION);
    }

    //RS274NGC v3 - 3.5 G Codes
    //group 0 - non modal (incomplete)
    //TODO
    //4 Dwell
    if ((parser_group1 & GCODE_GROUP_NONMODAL))
    {
        switch (new_state->groups.nonmodal)
        {
            case 1:
                //G10
                //if no P or L is present
                if (!(parser_word1 & GCODE_WORD_P) || !(parser_word1 & GCODE_WORD_L))
                {
                    return STATUS_GCODE_VALUE_WORD_MISSING;
                }
                //L is not 2
                if (words->l != 2)
                {
                    return STATUS_GCODE_UNSUPPORTED_COMMAND;
                }
                //P is not between 1 and N� of coord systems
                if (words->p < 1 || words->p > COORD_SYS_COUNT)
                {
                    return STATUS_GCODE_UNSUPPORTED_COORD_SYS;
                }
                break;
            case 5:
                //G53
                //if no G0 or G1 not active
                if (new_state->groups.motion > 1)
                {
                    return STATUS_GCODE_G53_INVALID_MOTION_MODE;
                }
                break;
        }
    }

    //group 1 - motion (incomplete)
    //TODO
    //38.2 probing
    //81...89 Canned cycles
    if (CHECKFLAG(parser_group0,GCODE_GROUP_MOTION))
    {
        switch (new_state->groups.motion)
        {
            case 0: //G0
            case 1: //G1
            case 4: //G38.2
            case 5: //G38.3
            case 6: //G38.4
            case 7: //G38.5
                if (!CHECKFLAG(parser_word0, GCODE_ALL_AXIS))
                {
                    return STATUS_GCODE_NO_AXIS_WORDS;
                }
                break;
            case 2:
            case 3:
                switch (new_state->groups.plane)
                {
                    case 0:
                        if (!CHECKFLAG(parser_word0, GCODE_XYPLANE_AXIS))
                        {
                            return STATUS_GCODE_NO_AXIS_WORDS_IN_PLANE;
                        }

                        if (!CHECKFLAG(parser_word1, GCODE_XYPLANE_AXIS) && !CHECKFLAG(parser_word1, GCODE_WORD_R))
                        {
                            return STATUS_GCODE_NO_OFFSETS_IN_PLANE;
                        }
                        break;
                    case 1:
                        if (!CHECKFLAG(parser_word0, GCODE_XZPLANE_AXIS))
                        {
                            return STATUS_GCODE_NO_AXIS_WORDS_IN_PLANE;
                        }

                        if (!CHECKFLAG(parser_word1, GCODE_XZPLANE_AXIS) && !CHECKFLAG(parser_word1, GCODE_WORD_R))
                        {
                            return STATUS_GCODE_NO_OFFSETS_IN_PLANE;
                        }
                        break;
                    case 2:
                        if (!CHECKFLAG(parser_word0, GCODE_YZPLANE_AXIS))
                        {
                            return STATUS_GCODE_NO_AXIS_WORDS_IN_PLANE;
                        }

                        if (!CHECKFLAG(parser_word1, GCODE_YZPLANE_AXIS) && !CHECKFLAG(parser_word1, GCODE_WORD_R))
                        {
                            return STATUS_GCODE_NO_OFFSETS_IN_PLANE;
                        }
                        break;
                }
                break;
            case 8: //G80 and
                if (CHECKFLAG(parser_word0, GCODE_ALL_AXIS))
                {
                    return STATUS_GCODE_AXIS_WORDS_EXIST;
                }
                break;
            default: //G81..G89 canned cycles (not implemented yet)
                break;
        }

//group 5 - feed rate mode
        if (new_state->groups.motion >= 1 && new_state->groups.motion <= 3)
        {
        	if(!CHECKFLAG(parser_word0, GCODE_WORD_F))
        	{
        		if (new_state->groups.feedrate_mode == 0)
	            {
	                return STATUS_GCODE_UNDEFINED_FEED_RATE;
	            }
	
	            if(new_state->feedrate == 0)
	            {
	                return STATUS_GCODE_UNDEFINED_FEED_RATE;
	            }
			}
            else
            {
            	if(words->f == 0)
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
    if ((new_state->groups.tool_length_offset == 1) && CHECKFLAG(parser_group0, GCODE_GROUP_TOOLLENGTH))
    {
        if (!CHECKFLAG(parser_word0, GCODE_WORD_Z))
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
uint8_t parser_exec_command(parser_state_t *new_state, parser_words_t *words)
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
    planner_block_data_t block_data = {};

    #ifdef GCODE_PROCESS_LINE_NUMBERS
    block_data.line = words->n;
    #endif

    planner_get_position(planner_last_pos);

    //RS274NGC v3 - 3.8 Order of Execution
    //1. comment (ignored - already filtered)
    //2. set feed mode (not implemented yet)
    block_data.motion_mode = PLANNER_MOTION_MODE_FEED;
    if (new_state->groups.feedrate_mode == 0)
    {
        if (new_state->groups.motion >= 1 && new_state->groups.motion <= 3)
        {
            block_data.motion_mode = PLANNER_MOTION_MODE_INVERSEFEED;
        }
    }
    //3. set feed rate (µCNC works in units per second and not per minute)
    if (CHECKFLAG(parser_word0, GCODE_WORD_F))
    {
        if (new_state->groups.feedrate_mode != 0)
        {
            new_state->feedrate = words->f * MIN_SEC_MULT;
        }
    }
    
	words->f = new_state->feedrate;
    
//4. set spindle speed
#ifdef USE_SPINDLE
    if (CHECKFLAG(parser_word2, GCODE_WORD_S))
    {
        new_state->spindle = words->s;
    }
    else
    {
        words->s = new_state->spindle;
    }
#endif
//5. select tool (not implemented yet)
#ifdef USE_TOOL_CHANGER
    //new_state->tool_index = words->t;
#else
    new_state->tool_index = 1; //tool is allways 1
#endif
//6. change tool (not implemented yet)
//7. spindle on/off
#ifdef USE_SPINDLE
    switch (new_state->groups.spindle_turning)
    {
        case 0:
            block_data.spindle = new_state->spindle;
            break;
        case 1:
            block_data.spindle = -new_state->spindle;
            break;
        case 2:
            block_data.spindle = 0;
            break;
    }

    //spindle speed or direction was changed (force a safety dwell to let the spindle change speed and continue)
    if (CHECKFLAG(parser_word2, GCODE_WORD_S) || CHECKFLAG(parser_group1, GCODE_GROUP_SPINDLE))
    {
        block_data.dwell = (uint16_t)roundf(DELAY_ON_SPINDLE_SPEED_CHANGE * 10.0);
    }
#endif
//8. coolant on/off
#ifdef USE_COOLANT
    parser_update_coolant(new_state->groups.coolant);
#endif

    //9. overrides
    if((new_state->groups.feed_speed_override==0) != planner_get_overrides())
    {
        planner_toggle_overrides();
    }

    //10. dwell
    if (CHECKFLAG(parser_group1, GCODE_GROUP_NONMODAL) && !new_state->groups.nonmodal)
    {
        //calc dwell in time in 10ms increments
        block_data.dwell = MAX(block_data.dwell, (uint16_t)roundf(words->p * 10.0));
    }

    //11. set active plane (G17, G18, G19)
    switch (new_state->groups.plane)
    {
        #if(defined(AXIS_X) && defined(AXIS_Y))
        case 0:
            a = AXIS_X;
            b = AXIS_Y;
            offset_a = 0;
            offset_b = 1;
            break;
        #endif
        #if(defined(AXIS_X) && defined(AXIS_Z))
        case 1:
            a = AXIS_X;
            b = AXIS_Z;
            offset_a = 0;
            offset_b = 2;
            break;
        #endif
        #if(defined(AXIS_Y) && defined(AXIS_Z))
        case 2:
            a = AXIS_Y;
            b = AXIS_Z;
            offset_a = 1;
            offset_b = 2;
            break;
        #endif
    }

    //12. set length units (G20, G21).
    if (new_state->groups.units == 0) //all internal state variables must be converted to mm
    {
        for (uint8_t i = AXIS_COUNT; i != 0;)
        {
            i--;
            words->xyzabc[i] *= 25.4f;
        }

        //check if any i, j or k words were used
        if (CHECKFLAG(parser_word1, GCODE_MAIN_AXIS))
        {
            for (uint8_t i = 3; i != 0;)
            {
                i--;
                words->ijk[i] *= 25.4f;
            }
        }

        //if normal feed mode convert to mm/s
        if (CHECKFLAG(block_data.motion_mode,PLANNER_MOTION_MODE_FEED))
        {
            words->f *= 25.4f;
        }

        if (CHECKFLAG(parser_word1, GCODE_WORD_R))
        {
            words->r *= 25.4f;
        }
    }

    //13. cutter radius compensation on or off (G40, G41, G42) (not implemented yet)
    //14. cutter length compensation on or off (G43.1, G49)
    #ifdef AXIS_TOOL
    if ((new_state->groups.tool_length_offset == 1) && CHECKFLAG(parser_group0, GCODE_GROUP_TOOLLENGTH))
    {
        parser_parameters.tool_length_offset = words->xyzabc[AXIS_Z];
        CLEARFLAG(parser_word0, GCODE_WORD_Z);
        words->xyzabc[AXIS_TOOL] = 0; //resets parameter so it it doen't do anything else
    }
    #endif
    //15. coordinate system selection (G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3) (OK nothing to be done)
    if (CHECKFLAG(parser_group1, GCODE_GROUP_COORDSYS))
    {
    	parser_parameters.coord_system_index = new_state->groups.coord_system;
    	settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (parser_parameters.coord_system_index * PARSER_PARAM_ADDR_OFFSET), (uint8_t*)&parser_parameters.coord_system_offset, PARSER_PARAM_SIZE);
        parser_wco_counter = 0;
    }
    //16. set path control mode (G61, G61.1, G64)
    switch(new_state->groups.path_mode)
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
    if (CHECKFLAG(block_data.motion_mode,PLANNER_MOTION_MODE_FEED))
    {
        block_data.feed = MIN(block_data.feed, words->f);
    }
    else
    {
        block_data.feed = words->f;
    }

    //if non-modal is executed
    if (CHECKFLAG(parser_group1, GCODE_GROUP_NONMODAL))
    {
        uint8_t index = 0;
        uint8_t error = 0;
        switch (new_state->groups.nonmodal)
        {
            case 0:											  //G4
                if (!CHECKFLAG(parser_word0, GCODE_ALL_AXIS)) //if no axis was issued then no motion group is going to be executed and execute dwell
                {
                    return mc_dwell(block_data);
                }
                break;
            case 1: //G10
                index = (uint8_t)words->p;
                switch(index)
                {
                    case 28:
                        settings_save(G28ADDRESS, (uint8_t *)&words->xyzabc, PARSER_PARAM_SIZE);
                        break;
                    case 30:
                        settings_save(G28ADDRESS, (uint8_t *)&words->xyzabc, PARSER_PARAM_SIZE);
                        break;
                    default:
                        index--;
                        for (uint8_t i = AXIS_COUNT; i != 0;)
                        {
                            i--;
                            parser_parameters.coord_system_offset[i] = words->xyzabc[i];
                        }
                        settings_save(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (index * PARSER_PARAM_ADDR_OFFSET), (uint8_t*)&parser_parameters.coord_system_offset, PARSER_PARAM_SIZE);
                        break;
                }
                CLEARFLAG(parser_word0, GCODE_ALL_AXIS);
                break;
            case 2: //G28
            case 3: //G30
                error = mc_line((float *)&words->xyzabc, block_data);
                if (error)
                {
                    return error;
                }

                if (new_state->groups.nonmodal == 2)
                {
                    settings_load(G28ADDRESS, (uint8_t*)&axis, PARSER_PARAM_SIZE);
                }
                else
                {
                    settings_load(G30ADDRESS, (uint8_t*)&axis, PARSER_PARAM_SIZE);
                }
                error = mc_line((float *)&axis, block_data);
                {
                    return error;
                }
                CLEARFLAG(parser_word0, GCODE_ALL_AXIS);
                break;
            case 5: //G53
                memcpy(&axis, &words->xyzabc, sizeof(axis));
                break;
            case 9: //G92
                for (uint8_t i = AXIS_COUNT; i != 0;)
                {
                    i--;
                    parser_parameters.g92_offset[i] = planner_last_pos[i] - parser_parameters.coord_system_offset[i] - words->xyzabc[i];
                }
				settings_save(G92ADDRESS, (uint8_t *)&parser_parameters.g92_offset, PARSER_PARAM_SIZE);
                parser_wco_counter = 0;
                CLEARFLAG(parser_word0, GCODE_ALL_AXIS);
                break;
            case 10: //G92.1
                memset(&parser_parameters.g92_offset, 0, sizeof(parser_parameters.g92_offset));
                settings_erase(G92ADDRESS, PARSER_PARAM_SIZE);
                parser_wco_counter = 0;
                break;
            case 11: //G92.2
                memset(&parser_parameters.g92_offset, 0, sizeof(parser_parameters.g92_offset));
                parser_wco_counter = 0;
                break;
            case 12: //G92.3
                settings_load(G92ADDRESS, (uint8_t *)&parser_parameters.g92_offset, PARSER_PARAM_SIZE);
                parser_wco_counter = 0;
                break;
        }
    }

    if (new_state->groups.nonmodal != 5) //if not modified by G53
    {
        memset(&axis, 0, sizeof(axis));

        if ((new_state->groups.distance_mode == 0))
        {
            for (uint8_t i = AXIS_COUNT; i != 0;)
            {
                i--;
                axis[i] = words->xyzabc[i] + parser_parameters.coord_system_offset[i] + parser_parameters.g92_offset[i];
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
                axis[i] = planner_last_pos[i] + words->xyzabc[i];
            }
        }

//for all not explicitly declared axis retain their position
#ifdef AXIS_X
        if (!CHECKFLAG(parser_word0, GCODE_WORD_X))
        {
            axis[AXIS_X] = planner_last_pos[AXIS_X];
        }
#endif
#ifdef AXIS_Y
        if (!CHECKFLAG(parser_word0, GCODE_WORD_Y))
        {
            axis[AXIS_Y] = planner_last_pos[AXIS_Y];
        }
#endif
#ifdef AXIS_Z
        if (!CHECKFLAG(parser_word0, GCODE_WORD_Z))
        {
            axis[AXIS_Z] = planner_last_pos[AXIS_Z];
        }
#endif
#ifdef AXIS_A
        if (!CHECKFLAG(parser_word0, GCODE_WORD_A))
        {
            axis[AXIS_A] = planner_last_pos[AXIS_A];
        }
#endif
#ifdef AXIS_B
        if (!CHECKFLAG(parser_word0, GCODE_WORD_B))
        {
            axis[AXIS_B] = planner_last_pos[AXIS_B];
        }
#endif
#ifdef AXIS_C
        if (!CHECKFLAG(parser_word0, GCODE_WORD_C))
        {
            axis[AXIS_C] = planner_last_pos[AXIS_C];
        }
#endif
    }

    float x, y;
    //20. perform motion (G0 to G3, G80 to G89), as modified (possibly) by G53.
    //incomplete (canned cycles not supported)
    //if at least one axis was present in the command execute the active motion group
    if (CHECKFLAG(parser_word0, GCODE_ALL_AXIS))
    {
        uint8_t probe_error;

        switch (new_state->groups.motion)
        {
            case 0:
                //rapid move
                block_data.feed = FLT_MAX;
                //continues to send G1 at maximum feed rate
            case 1:
                if (block_data.feed == 0)
                {
                    return STATUS_FEED_NOT_SET;
                }
                return mc_line(axis, block_data);
            case 2:
            case 3:
                if (block_data.feed == 0)
                {
                    return STATUS_FEED_NOT_SET;
                }

                //target points
                x = axis[a] - planner_last_pos[a];
                y = axis[b] - planner_last_pos[b];

                //radius mode
                if (CHECKFLAG(parser_word1, GCODE_WORD_R))
                {
                    if (x == 0 && y == 0)
                    {
                        return STATUS_GCODE_INVALID_TARGET;
                    }

                    float x_sqr = x * x;
                    float y_sqr = y * y;
                    float c_factor = 4.0 * words->r * words->r - x_sqr - y_sqr;

                    if (c_factor < 0)
                    {
                        return STATUS_GCODE_ARC_RADIUS_ERROR;
                    }

                    c_factor = -sqrt((c_factor) / (x_sqr + y_sqr));

                    if (new_state->groups.motion == 3)
                    {
                        c_factor = -c_factor;
                    }

                    if (words->r < 0)
                    {
                        c_factor = -c_factor;
                        words->r = -words->r; // Finished with r. Set to positive for mc_arc
                    }

                    // Complete the operation by calculating the actual center of the arc
                    words->ijk[offset_a] = 0.5 * (x - (y * c_factor));
                    words->ijk[offset_b] = 0.5 * (y + (x * c_factor));
                    radius = words->r;
                }
                else //offset mode
                {
                    //calculate radius and check if center is within tolerances
                    radius = sqrtf(words->ijk[offset_a] * words->ijk[offset_a] + words->ijk[offset_b] * words->ijk[offset_b]);
                    //calculates the distance between the center point and the target points and compares with the center offset distance
                    float x1 = x - words->ijk[offset_a];
                    float y1 = y - words->ijk[offset_b];
                    float r1 = sqrt(x1 * x1 + y1 * y1);
                    if (fabs(radius - r1) > 0.002) //error must not exceed 0.002mm
                    {
                        return STATUS_GCODE_INVALID_TARGET;
                    }
                }

                return mc_arc(axis, words->ijk[offset_a], words->ijk[offset_b], radius, a, b, (new_state->groups.motion == 2), block_data);
            case 4: //G38.2
            case 5: //G38.3
            case 6: //G38.4
            case 7: //G38.5
                probe_error = mc_probe(axis, (new_state->groups.motion > 5), block_data);
                if (probe_error)
                {
                    parser_parameters.last_probe_ok = 0;
                    if (!(new_state->groups.motion & 0x01))
                    {
                        cnc_alarm(probe_error);
                    }
                    return STATUS_OK;
                }
                parser_parameters.last_probe_ok = 1;
                return STATUS_OK;
        }
    }

    //stop (M0, M1, M2, M30, M60) (not implemented yet).
    switch (new_state->groups.stopping)
    {
        case 0: //M0
            break;
        case 1: //M1
            break;
        case 2: //M2
        case 3: //M30
            //reset to initial states
            parser_reset();
            protocol_send_string(MSG_FEEDBACK_8);
            break;
        case 6: //M60
            break;
    }

    //if reached here the execution was not intersected
    //spindle and coolant must be updated
    if (CHECKFLAG(parser_word2, GCODE_WORD_S) || CHECKFLAG(parser_group1, GCODE_GROUP_SPINDLE | GCODE_GROUP_COOLANT))
    {
        return mc_spindle_coolant(block_data);
    }

    return STATUS_OK;
}

void parser_reset(void)
{
    parser_state.groups.coord_system = 0;											//G54
    parser_state.groups.plane = 0;												  	//G17
    parser_state.groups.feed_speed_override = 0;								  	//M48
    parser_state.groups.cutter_radius_compensation = 0;							  	//M40
    parser_state.groups.distance_mode = 1;										 	//G91
    parser_state.groups.feedrate_mode = 1;										  	//G94
    parser_state.groups.tool_length_offset = 0;										//G49
#ifdef USE_COOLANT
    parser_state.groups.coolant = 0; 												//M9
#endif
#ifdef USE_SPINDLE
    parser_state.groups.spindle_turning = 2; 										//M5
#endif
    parser_state.groups.motion = 1; 												//G1
    parser_state.groups.units = 1; 													//G21
    memset(&parser_parameters.g92_offset, 0, sizeof(parser_parameters.g92_offset));	//G92.2
}

//loads parameters
//loads G92 offset
//loads G54 coordinate system
//also checks all other coordinate systems and homing positions
void parser_load_parameters(void)
{
    const uint8_t size = PARSER_PARAM_SIZE;

    //loads G92
    if(settings_load(G92ADDRESS, (uint8_t *)&parser_parameters.g92_offset, PARSER_PARAM_SIZE))
    {
        memset(&parser_parameters.g92_offset, 0, sizeof(parser_parameters.g92_offset));
        settings_erase(G92ADDRESS, PARSER_PARAM_SIZE);
    }

    for(uint8_t i = 1; i < G92OFFSET; i++)
    {
        if(settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (i * PARSER_PARAM_ADDR_OFFSET), (uint8_t*)&parser_parameters.coord_system_offset, PARSER_PARAM_SIZE))
        {
            settings_erase(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (i * PARSER_PARAM_ADDR_OFFSET), PARSER_PARAM_SIZE);
        }
    }

    //load G54
    if(settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET, (uint8_t*)&parser_parameters.coord_system_offset, PARSER_PARAM_SIZE))
    {
        memset(&parser_parameters.coord_system_offset, 0, sizeof(parser_parameters.coord_system_offset));
        settings_erase(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET, PARSER_PARAM_SIZE);
    }
}

void parser_discard_command(void)
{
    unsigned char c = '@';
    #ifdef ECHO_CMD
    serial_putc(c);
    #endif
    do
    {
        
        c = serial_getc();
    } while(c != EOL);
}

uint8_t parser_eat_next_char(unsigned char c)
{
    if (serial_peek() != c)
    {
        return STATUS_INVALID_STATEMENT;
    }

    serial_getc();
    return STATUS_OK;
}
