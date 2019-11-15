#include "config.h"

#include "mcu.h"
#include "gcode.h"
#include "report.h"
#include "error.h"
#include "utils.h"
#include "planner.h"
#include "settings.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

#define MUL10(X) (((X<<2) + X)<<1)

GCODE_PARSER_STATE g_gcparser_state;
float g_gcode_coord_sys[COORD_SYS_COUNT][AXIS_COUNT];
float g_gcode_offset[AXIS_COUNT];
float gcode_max_feed;

/*
	Parses a string to number (real)
	If the number is an integer the isinteger flag is set
	The string pointer is also advanced to the next position
*/
bool gcode_parse_float(char **str, float *value, bool *isinteger)
{
    bool isnegative = false;
    bool isfloat = false;
    uint32_t intval = 0;
    uint8_t fpcount = 0;
    bool result = false;

    if (**str == '-')
    {
        isnegative = true;
        (*str)++;
    }
    else if (**str == '+')
    {
        (*str)++;
    }

    for(;;)
    {
        uint8_t digit = (uint8_t)(**str - '0');
        if (digit <= 9)
        {
            intval = MUL10(intval) + digit;
            if (isfloat)
            {
                fpcount++;
            }

            result = true;
        }
        else if (**str == '.' && !isfloat)
        {
            isfloat = true;
        }
        else
        {
            break;
        }

        (*str)++;
        result = true;
    }
    
    *value = intval;
 
    do
    {
        if(fpcount>=3)
        {
            *value *= 0.001f;
            fpcount -= 3;
        }

        if(fpcount>=2)
        {
            *value *= 0.01f;
            fpcount -= 2;
        }

        if(fpcount>=1)
        {
            *value *= 0.1f;
            fpcount -= 1;
        }

    } while (fpcount !=0 );
    

    *isinteger = !isfloat;
    
    if(isnegative)
    {
    	*value = -*value;
	}
	
    return result;

}

/*
	parses comments as defined in the RS274
	Suports nested comments
	If comment is not closed returns an error
*/
void gcode_parse_comment()
{
	uint8_t comment_nest = 1;
	for(;;)
	{
		while(mcu_peek()=='\0');
		
		char c = mcu_peek();
		switch(c)
		{
			case '(':
				comment_nest++;
				break;
			case ')':
				comment_nest--;
				if(comment_nest == 0)
				{
					mcu_getc();
					return;
				}
				break;
			case '\n':
			case '\r':
				report_error(GCODE_INVALID_COMMENT);
                return;
		}
		
		mcu_getc();
	}	
}

/*
	STEP 1
	Fetches the next line from the mcu communication buffer and preprocesses the string
	In the preprocess these steps are executed
		1. Whitespaces are removed
		2. Comments are parsed (nothing is done besides parsing for now)
		3. All letters are changed to upper-case	
*/
void gcode_fetch_frombuffer(char *str)
{
	if(mcu_peek() != 0)
	{
		for(;;)
		{
			char c = mcu_getc();	
			switch(c)
			{
				case ' ':
				case '\t':
					//ignore whitechars
					break;
				case '(':
					gcode_parse_comment();
					mcu_printfp(PSTR("comment"));
					break;
				case '\n':
				case '\r':
					*str = '\0';
					return;
				default:
					if(c>='a' && c<='z')
					{
						c -= 32;
					}
					*str = c;
					str++;
					break;
			}
		}
	}
}

/*
	STEP 2
	Parse the hole string and updates the values of the parser new state
    In this step the parser will check if:
    	1. There is a valid word character followed by a number
    	2. If there is no modal groups or word repeating violations
    	3. For words N, M, T and L check if the value is an integer
    	4. If N is in the beginning of the line
    	
    After that the parser has to perform the following checks:
    	1. At least a G or M command must exist in a line
    	2. Words F, N, R and S must be positive (H, P, Q and T not implemented)
    	3. Motion codes must have at least one axis declared
*/
void gcode_parse_line(char* str, GCODE_PARSER_STATE *new_state)
{
    float word_val = 0.0;
    char word = '\0';
    uint8_t code = 0;
    uint8_t subcode = 0;
    uint8_t wordcount = 0;
    uint8_t mwords = 0;
    uint8_t axis_mask = 0x3F;

    //flags optimized for 8 bits CPU
    uint8_t group0 = 0;
    uint8_t group1 = 0;
    uint8_t word0 = 0;
    uint8_t word1 = 0;
    uint8_t word2 = 0;
    
    for(;;)
    {
        word = *str;
        if(word == '\0')
        {
            break;
        }

        str++;
        bool isinteger = false;
        if(!gcode_parse_float(&str, &word_val, &isinteger))
        {
            report_error(GCODE_BAD_NUMBER_FORMAT);
            return ;
        }

        switch(word)
        {
            case 'G':
                code = (uint8_t)(word_val);
                switch(code)
                {
                    //motion codes
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                        if(CHECKFLAG(group0,GCODE_GROUP_MOTION))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group0,GCODE_GROUP_MOTION);
                        new_state->groups.motion = code;
                        break;
                    //unsuported
                    case 38://check if 38.2
                        if(CHECKFLAG(group0,GCODE_GROUP_MOTION))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group0,GCODE_GROUP_MOTION);
                        subcode = (uint8_t)round((word_val - code) * 100.0f);
                        if(subcode == 20)
                        {
                            report_error(GCODE_UNSUPORTED_COMMAND);
                        }      
                        else
                        {
                            report_error(GCODE_UNKNOWN_GCOMMAND);
                        }
                        return;
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
                        if(CHECKFLAG(group0,GCODE_GROUP_MOTION))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group0,GCODE_GROUP_MOTION);
                        code -= 75;
                        new_state->groups.motion = code;
                        break;
                    case 17:
                    case 18:
                    case 19:
                        if(CHECKFLAG(group0,GCODE_GROUP_PLANE))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group0,GCODE_GROUP_PLANE);
                        code -= 17;
                        new_state->groups.plane = code;
                        break;
                    case 90:
                    case 91:
                        if(CHECKFLAG(group0,GCODE_GROUP_DISTANCE))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group0,GCODE_GROUP_DISTANCE);
                        code -= 90;
                        new_state->groups.distance_mode = code;
                        break;
                    case 93:
                    case 94:
                        if(CHECKFLAG(group0,GCODE_GROUP_FEEDRATE))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group0,GCODE_GROUP_FEEDRATE);
                        code -= 93;
                        new_state->groups.feedrate_mode = code;
                        break;
                    case 20:
                    case 21:
                        if(CHECKFLAG(group0,GCODE_GROUP_UNITS))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group0,GCODE_GROUP_UNITS);  
                        code -= 20;
                        new_state->groups.units = code;
                        break;
                    case 40:
                    case 41:
                    case 42:
                        if(CHECKFLAG(group0,GCODE_GROUP_CUTTERRAD))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group0,GCODE_GROUP_CUTTERRAD);
                        code -= 40;
                        new_state->groups.cutter_radius_compensation = code;
                        break;
                    case 43:
                        if(CHECKFLAG(group0,GCODE_GROUP_TOOLLENGTH))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group0,GCODE_GROUP_TOOLLENGTH);
                        new_state->groups.tool_length_offset = 0;
                        break;
                    case 49:
                        if(CHECKFLAG(group0,GCODE_GROUP_TOOLLENGTH))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group0,GCODE_GROUP_TOOLLENGTH);
                        new_state->groups.tool_length_offset = 1;
                        break;
                    case 98:
                    case 99:
                        if(CHECKFLAG(group0,GCODE_GROUP_RETURNMODE))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group0,GCODE_GROUP_RETURNMODE);
                        code -= 98;
                        new_state->groups.return_mode = code;
                        break;
                    case 54:
                    case 55:
                    case 56:
                    case 57:
                    case 58:
                    case 59:
                        if(CHECKFLAG(group1,GCODE_GROUP_COORDSYS))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group1,GCODE_GROUP_COORDSYS);
                        //59.X unsupported
                        if(code == 59)
                        {
                            subcode = (uint8_t)round((word_val - code) * 100.0f);
                            switch(subcode)
                            {
                                case 10:
                                case 20:
                                case 30:
                                    report_error(GCODE_UNSUPORTED_COMMAND);
                                    return;
                                default:
                                    report_error(GCODE_UNKNOWN_GCOMMAND);
                                    return;
                            }
                        }
                        code -= 54;
                        new_state->groups.coord_system = code;
                        break;
                    case 61:
                        if(CHECKFLAG(group1,GCODE_GROUP_PATH))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group1,GCODE_GROUP_PATH);
                        new_state->groups.path_mode = 0;
                        break;
                    case 64:
                        if(CHECKFLAG(group1,GCODE_GROUP_PATH))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group1,GCODE_GROUP_PATH);
                        new_state->groups.path_mode = 1;
                        break;
                    case 4:
                    case 10:
                    case 28:
                    case 30:
                    case 53:
                    case 92:
                        //convert code within 4 bits without 
                        //4 = 4
                        //10 = 1
                        //28 = 2
                        //30 = 3
                        //53 = 5
                        //92 = 9
                        //92.1 = 10
                        //92.2 = 11
                        //92.3 = 12
                        subcode = (uint8_t)round((word_val - code) * 10.0f);
                        if(code >= 10)
                        {
                            code /= 10;
                            code += subcode;
                        }

                        if(CHECKFLAG(group1,GCODE_GROUP_NONMODAL))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group1,GCODE_GROUP_NONMODAL);
                        new_state->groups.nonmodal = code;
                        break;
                    
                    default:
                        report_error(GCODE_UNKNOWN_GCOMMAND);
                        return ;
                }
                break;

            case 'M':
            	if(!isinteger)
	            {
	                report_error(GCODE_UNKNOWN_MCOMMAND);
	                return;
	            }
	            
	            //counts number of M commands
	            mwords++;
                code = (uint8_t)(word_val);
                switch(code)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 30:
                    case 60:
                        if(CHECKFLAG(group1,GCODE_GROUP_STOPPING))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group1,GCODE_GROUP_STOPPING);
                        if(code >= 10)
                        {
                            code /= 10;
                        }
                        new_state->groups.stopping = code;
                        break;
                    case 3:
                    case 4:
                    case 5:
                        if(CHECKFLAG(group1,GCODE_GROUP_SPINDLE))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group1,GCODE_GROUP_SPINDLE);
                        code -= 3;
                        new_state->groups.spindle_turning = code;
                        break;
                    case 7:
                        new_state->groups.coolant |= 1;
                        break;
                    case 8:
                        new_state->groups.coolant |= 2;
                        break;
                    case 9:
                        new_state->groups.coolant = 0;
                        break;
                    case 48:
                    case 49:
                        if(CHECKFLAG(group1,GCODE_GROUP_ENABLEOVER))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETFLAG(group1,GCODE_GROUP_ENABLEOVER);
                        code -= 48;
                        new_state->groups.feed_speed_override = code;
                        break;
                    default:
                    	report_error(GCODE_UNKNOWN_MCOMMAND);
	                	return;
                }
            break;
        case 'N':
            if(CHECKFLAG(word2, GCODE_WORD_N))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            if(!isinteger || wordcount!=0)
            {
                report_error(GCODE_INVALID_LINE_NUMBER);
                return;
            }

            new_state->linenum = trunc(word_val);
        case 'X':
            if(CHECKFLAG(word0, GCODE_WORD_X))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word0, GCODE_WORD_X);
            new_state->words.xyzabc[0] = word_val;
            break;
        case 'Y':
            if(CHECKFLAG(word0, GCODE_WORD_Y))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word0, GCODE_WORD_Y);
            new_state->words.xyzabc[1] = word_val;
            break;
        case 'Z':
            if(CHECKFLAG(word0, GCODE_WORD_Z))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word0, GCODE_WORD_Z);
            new_state->words.xyzabc[2] = word_val;
            break;
        case 'A':
            if(CHECKFLAG(word0, GCODE_WORD_A))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word0, GCODE_WORD_A);
            new_state->words.xyzabc[3] = word_val;
            break;
        case 'B':
            if(CHECKFLAG(word0, GCODE_WORD_B))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word0, GCODE_WORD_B);
            new_state->words.xyzabc[4] = word_val;
            break;
        case 'C':
            if(CHECKFLAG(word0, GCODE_WORD_C))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word0, GCODE_WORD_C);
            new_state->words.xyzabc[5] = word_val;
            break;
        case 'D':
            if(CHECKFLAG(word0, GCODE_WORD_D))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word0, GCODE_WORD_D);
            new_state->words.d = word_val;
            break;
        case 'F':
            if(CHECKFLAG(word0, GCODE_WORD_F))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word0, GCODE_WORD_F);
            new_state->words.f = word_val;
            break;
        case 'H':
            if(CHECKFLAG(word1, GCODE_WORD_H))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word1, GCODE_WORD_H);
            new_state->words.h = word_val;
            break;
        case 'I':
            if(CHECKFLAG(word1, GCODE_WORD_I))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word1, GCODE_WORD_I);
            new_state->words.ijk[0] = word_val;
            break;
        case 'J':
            if(CHECKFLAG(word1, GCODE_WORD_J))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word1, GCODE_WORD_J);
            new_state->words.ijk[1] = word_val;
            break;
        case 'K':
            if(CHECKFLAG(word1, GCODE_WORD_K))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word1, GCODE_WORD_K);
            new_state->words.ijk[2] = word_val;
            break;
        case 'L':
            if(CHECKFLAG(word1, GCODE_WORD_L))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }
            
            if(!isinteger)
            {
                report_error(GCODE_VALUE_NOT_INTEGER);
                return;
            }

            SETFLAG(word1, GCODE_WORD_L);
            new_state->words.l= word_val;
            break;
        case 'P':
            if(CHECKFLAG(word1, GCODE_WORD_P))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word1, GCODE_WORD_P);
            new_state->words.p = word_val;
            break;
        case 'Q':
            if(CHECKFLAG(word1, GCODE_WORD_Q))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word1, GCODE_WORD_Q);
            new_state->words.q = word_val;
            break;
        case 'R':
            if(CHECKFLAG(word1, GCODE_WORD_R))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word1, GCODE_WORD_R);
            new_state->words.r = word_val;
            break;
        case 'S':
            if(CHECKFLAG(word2, GCODE_WORD_S))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETFLAG(word2, GCODE_WORD_S);
            new_state->words.s = word_val;
            break;
        case 'T':
            if(CHECKFLAG(word2, GCODE_WORD_T))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }
            
            if(!isinteger)
            {
                report_error(GCODE_VALUE_NOT_INTEGER);
                return;
            }

            SETFLAG(word2, GCODE_WORD_T);
            new_state->words.t = word_val;
            break;
        default:
            break;

        }
        wordcount++;
    }
    
    //The string is parsed
    //Starts to validate the string parameters
    
    //At least a G or M command must exist in a line
	if(group0==0 && group1 == 0)
    {
        report_error(GCODE_MISSING_COMMAND);
        return;
    }
    
    //Line number must be positive
    if(new_state->linenum<0)
    {
    	report_error(GCODE_INVALID_LINE_NUMBER);
        return;
	}
	
	//Words F, R and S must be positive
	//Words H, P, Q, and T are not implemented
	if(new_state->words.f<0.0f || new_state->words.r<0.0f|| new_state->words.s<0.0f)
    {
    	report_error(GCODE_VALUE_IS_NEGATIVE);
        return;
	}
	
	//check if axis are definined in motion commands
	if(new_state->groups.motion == 2 || new_state->groups.motion == 3)
	{
		switch(new_state->groups.plane)
		{
			case 0: //XY
				axis_mask = 0x02;
				break;
			case 1: //XZ
				axis_mask = 0x05;
				break;
			case 2: //YZ
				axis_mask = 0x06;
				break;
			default:
				axis_mask = 0x3F;
				break;
		}
	}
	
	//G0, G1, G2 and G3
	if(new_state->groups.motion <= 3)
	{
		if((word0 & axis_mask) == 0)
		{
			report_error(GCODE_UNDEFINED_AXIS);
        	return;
		}
	}
	
	//future
	//check if T is negative and smaller than max tool slots 
}

/*
	STEP 3
	In this step the interpreter does all remaining checks.
	The command is then executed has defined by the RS274 instruction
	All coordinates are converted to machine absolute coordinates before sent to the motion controller
*/
void gcode_execute_line(GCODE_PARSER_STATE *new_state)
{	
	float axis[AXIS_COUNT];
	float feed = 0;
	float spindle = 0;
	uint8_t coord_sys = COORD_SYS_COUNT;
	
	//resets axis
	memset(&axis, 0, sizeof(float)*AXIS_COUNT);
	
	//has close has possible to the RS274 document (part 3.8)
	
	//set feed (given the feedmode selected)
	
	if(new_state->groups.feedrate_mode!=0)
	{
		feed = new_state->words.f;
	}
	else
	{
		feed = (1.0f / new_state->words.f);
	}
	//internally uCNC works in mm/s and not mm/min
	feed /= 60.0f;
	
	//set spindle speed
	spindle = new_state->words.s;
	
	//select tool (not implemented)
	//coolant mode already defined in new_state->groups
	//overrides (not implemented)
	//dwell (not implemented)
	//set active plane (G17, G18, G19).
	//set length units (G20, G21).
	if(new_state->groups.units == 0) //convert inches to mm
	{
		#ifdef AXIS_X
			new_state->words.xyzabc[AXIS_X] *= 25.4f;
		#endif
		#ifdef AXIS_Y
			new_state->words.xyzabc[AXIS_Y] *= 25.4f;
		#endif
		#ifdef AXIS_Z
			new_state->words.xyzabc[AXIS_Z] *= 25.4f;
		#endif
		#ifdef AXIS_A
			new_state->words.xyzabc[AXIS_A] *= 25.4f;
		#endif
		#ifdef AXIS_B
			new_state->words.xyzabc[AXIS_B] *= 25.4f;
		#endif
		#ifdef AXIS_C
			new_state->words.xyzabc[AXIS_C] *= 25.4f;
		#endif
	}
	
	//cutter radius compensation on or off (G40, G41, G42)
	//cutter length compensation on or off (G43, G49)
	//coordinate system selection (G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3).
	if(new_state->groups.nonmodal != 5) //if no G53 (absolute coordinate system) was issued load coordinate system
	{
		#ifdef AXIS_X
			axis[AXIS_X] = g_gcode_coord_sys[new_state->groups.coord_system][AXIS_X];
		#endif
		#ifdef AXIS_Y
			axis[AXIS_Y] = g_gcode_coord_sys[new_state->groups.coord_system][AXIS_Y];
		#endif
		#ifdef AXIS_Z
			axis[AXIS_Z] = g_gcode_coord_sys[new_state->groups.coord_system][AXIS_Z];
		#endif
		#ifdef AXIS_A
			axis[AXIS_A] = g_gcode_coord_sys[new_state->groups.coord_system][AXIS_A];
		#endif
		#ifdef AXIS_B
			axis[AXIS_B] = g_gcode_coord_sys[new_state->groups.coord_system][AXIS_B];
		#endif
		#ifdef AXIS_C
			axis[AXIS_C] = g_gcode_coord_sys[new_state->groups.coord_system][AXIS_C];
		#endif
	}
	//set path control mode (G61, G61.1, G64)
	//set distance mode (G90, G91).

	if(new_state->groups.distance_mode == 1) //if relative initialize with current absolute position
	{
		#ifdef AXIS_X
			axis[AXIS_X] += g_gcparser_state.words.xyzabc[AXIS_X];
		#endif
		#ifdef AXIS_Y
			axis[AXIS_Y] += g_gcparser_state.words.xyzabc[AXIS_Y];
		#endif
		#ifdef AXIS_Z
			axis[AXIS_Z] += g_gcparser_state.words.xyzabc[AXIS_Z];
		#endif
		#ifdef AXIS_A
			axis[AXIS_A] += g_gcparser_state.words.xyzabc[AXIS_A];
		#endif
		#ifdef AXIS_B
			axis[AXIS_B] += g_gcparser_state.words.xyzabc[AXIS_B];
		#endif
		#ifdef AXIS_C
			axis[AXIS_C] += g_gcparser_state.words.xyzabc[AXIS_C];
		#endif
	}
	//set retract mode (G98, G99).
	//home (G28, G30) or
	//change coordinate system data (G10) or
	//set new coordinate system offset
	//implement later
	/*
	#ifdef AXIS_X
		g_gcode_coord_sys[new_state->groups.coord_system][AXIS_X] = new_state->words.xyzabc[AXIS_X];
	#endif
	#ifdef AXIS_Y
		g_gcode_coord_sys[new_state->groups.coord_system][AXIS_Y] = new_state->words.xyzabc[AXIS_Y];
	#endif
	#ifdef AXIS_Z
		g_gcode_coord_sys[new_state->groups.coord_system][AXIS_Z] = new_state->words.xyzabc[AXIS_Z];
	#endif
	#ifdef AXIS_A
		g_gcode_coord_sys[new_state->groups.coord_system][AXIS_A] = new_state->words.xyzabc[AXIS_A];
	#endif
	#ifdef AXIS_B
		g_gcode_coord_sys[new_state->groups.coord_system][AXIS_B] = new_state->words.xyzabc[AXIS_B];
	#endif
	#ifdef AXIS_C
		g_gcode_coord_sys[new_state->groups.coord_system][AXIS_C] = new_state->words.xyzabc[AXIS_C];
	#endif
	*/
		
	//set axis offsets (G92, G92.1, G92.2, G94).
	//set new coordinate system offset
	switch(new_state->groups.nonmodal)
	{
		case 9:
			#ifdef AXIS_X
				g_gcode_offset[AXIS_X] = new_state->words.xyzabc[AXIS_X];
			#endif
			#ifdef AXIS_Y
				g_gcode_offset[AXIS_Y] = new_state->words.xyzabc[AXIS_Y];
			#endif
			#ifdef AXIS_Z
				g_gcode_offset[AXIS_Z] = new_state->words.xyzabc[AXIS_Z];
			#endif
			#ifdef AXIS_A
				g_gcode_offset[AXIS_A] = new_state->words.xyzabc[AXIS_A];
			#endif
			#ifdef AXIS_B
				g_gcode_offset[AXIS_B] = new_state->words.xyzabc[AXIS_B];
			#endif
			#ifdef AXIS_C
				g_gcode_offset[AXIS_C] = new_state->words.xyzabc[AXIS_C];
			#endif
			break;
		case 10:
			memset(&g_gcode_offset,0, sizeof(float)*AXIS_COUNT);
			break;
	}
	
	
	//perform motion (G0 to G3, G80 to G89), as modified (possibly) by G53.
	#ifdef AXIS_X
		axis[AXIS_X] += new_state->words.xyzabc[AXIS_X] + g_gcode_offset[AXIS_X];
	#endif
	#ifdef AXIS_Y
		axis[AXIS_Y] += new_state->words.xyzabc[AXIS_Y] + g_gcode_offset[AXIS_Y];
	#endif
	#ifdef AXIS_Z
		axis[AXIS_Z] += new_state->words.xyzabc[AXIS_Z] + g_gcode_offset[AXIS_Z];
	#endif
	#ifdef AXIS_A
		axis[AXIS_A] += new_state->words.xyzabc[AXIS_A] + g_gcode_offset[AXIS_A];
	#endif
	#ifdef AXIS_B
		axis[AXIS_B] += new_state->words.xyzabc[AXIS_A] + g_gcode_offset[AXIS_B];
	#endif
	#ifdef AXIS_C
		axis[AXIS_C] += new_state->words.xyzabc[AXIS_C] + g_gcode_offset[AXIS_C];
	#endif
	
	switch(new_state->groups.motion)
	{
		case 0:
			feed = gcode_max_feed;
			planner_add_line(axis, feed);
			break;
		case 1:
			planner_add_line(axis, feed);
			break;
	}
	
	//stop (M0, M1, M2, M30, M60).
	
	//if everything went ok updates the interpreter state
	memcpy(&g_gcparser_state, new_state, sizeof(GCODE_PARSER_STATE));
}

/*
	Initializes the gcode parser 
*/
void gcode_init()
{
	memset(&g_gcparser_state, 0, sizeof(GCODE_PARSER_STATE));
	for(uint8_t i = 0; i < COORD_SYS_COUNT; i++)
	{
		for(uint8_t j = 0; j < AXIS_COUNT; j++)
		{
			g_gcode_coord_sys[i][j] = 0;
		}
	}
	
	g_gcparser_state.groups.units = 1; //default units mm
	g_gcparser_state.groups.feedrate_mode = 1; //default units/m
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		gcode_max_feed = (gcode_max_feed <= g_settings.max_speed[i]) ? g_settings.max_speed[i] : gcode_max_feed;
	}
}

/*
	Parse the next gcode line available in the buffer and send it to the motion controller
*/
void gcode_parse_nextline()
{
	//nothing to be done
	if(mcu_peek() == 0)
	{
		return;
	}
	
	char gcode_line[GCODE_PARSER_BUFFER_SIZE];
	GCODE_PARSER_STATE next_state = {};
	//next state will be the same as previous except for nonmodal group (is set with 0)
	memcpy(&next_state, &g_gcparser_state, sizeof(GCODE_PARSER_STATE));
    next_state.groups.nonmodal = 0;
	gcode_fetch_frombuffer(&gcode_line[0]);
	gcode_parse_line(&gcode_line[0], &next_state);
	gcode_execute_line(&next_state);
	
}

void gcode_print_states()
{
	mcu_printfp(PSTR("GCode parser active states: "));
	mcu_printfp(PSTR("G%u "), 20 + g_gcparser_state.groups.units);
	mcu_printfp(PSTR("G%u "), 17 + g_gcparser_state.groups.plane);
	mcu_printfp(PSTR("G%u\n"), 90 + g_gcparser_state.groups.distance_mode);
}
