#include "gcparser.h"
#include "report.h"
#include "error.h"
#include "utils.h"
#include <math.h>

#define MUL10(X) (((X<<2) + X)<<1)

static GCODE_PARSER_STATE g_gcparser_state;

bool gcparser_parse_float(char *str, float *value)
{
    bool isnegative = true;
    bool isfloat = true;
    uint32_t intval = 0;
    uint8_t fpcount = 0;
    bool result = false;

    if (*str == '-')
    {
        isnegative = true;
        str++;
    }
    else if (*str == '+')
    {
        str++;
    }

    for(;;)
    {
        uint8_t digit = (uint8_t)(*str - '0');
        if (digit <= 9)
        {
            intval = MUL10(intval) + digit;
            if (isfloat)
            {
                fpcount++;
            }

            result = true;
        }
        else if (*str == '.' && !isfloat)
        {
            isfloat = true;
        }
        else
        {
            break;
        }

        str++;
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
    

    return result;

}

void gcparser_parse_line(char* str)
{
    GCODE_PARSER_STATE new_state;
    float word_val = 0.0;
    char word = '\0';
    uint8_t code = 0;
    uint8_t subcode = 0;

    //flags optimized for 8 bits CPU
    uint8_t group0 = 0;
    uint8_t group1 = 0;
    uint8_t word0 = 0;
    uint8_t word1 = 0;
    uint8_t word2 = 0;
    
    memcpy(&new_state, &g_gcparser_state, sizeof(GCODE_PARSER_STATE));
    new_state.groups.nonmodal = 0;

    //Step 1
    //Parse the hole string.
    //In this step the parser will check if:
    //  1. There is a valid word character followed by a number (no white spaces in between)
    //  2. If there is no modal groups or word repeating violations

    for(;;)
    {
        word = *str;
        if(word == '\0')
        {
            break;
        }

        str++;
        if(!gcparser_parse_float(str, &word_val))
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
                        if(CHECKBIT(group0,GCODE_GROUP_MOTION))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group0,GCODE_GROUP_MOTION);
                        g_gcparser_state.groups.motion = code;
                        break;
                    //unsuported
                    case 38://check if 38.2
                        if(CHECKBIT(group0,GCODE_GROUP_MOTION))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group0,GCODE_GROUP_MOTION);
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
                        if(CHECKBIT(group0,GCODE_GROUP_MOTION))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group0,GCODE_GROUP_MOTION);
                        code -= 75;
                        g_gcparser_state.groups.motion = code;
                        break;
                    case 17:
                    case 18:
                    case 19:
                        if(CHECKBIT(group0,GCODE_GROUP_PLANE))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group0,GCODE_GROUP_PLANE);
                        code -= 17;
                        g_gcparser_state.groups.plane = code;
                        break;
                    case 90:
                    case 91:
                        if(CHECKBIT(group0,GCODE_GROUP_DISTANCE))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group0,GCODE_GROUP_DISTANCE);
                        code -= 90;
                        g_gcparser_state.groups.distance_mode = code;
                        break;
                    case 93:
                    case 94:
                        if(CHECKBIT(group0,GCODE_GROUP_FEEDRATE))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group0,GCODE_GROUP_FEEDRATE);
                        code -= 93;
                        g_gcparser_state.groups.feedrate_mode = code;
                        break;
                    case 20:
                    case 21:
                        if(CHECKBIT(group0,GCODE_GROUP_UNITS))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group0,GCODE_GROUP_UNITS);  
                        code -= 20;
                        g_gcparser_state.groups.units = code;
                        break;
                    case 40:
                    case 41:
                    case 42:
                        if(CHECKBIT(group0,GCODE_GROUP_CUTTERRAD))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group0,GCODE_GROUP_CUTTERRAD);
                        code -= 40;
                        g_gcparser_state.groups.cutter_radius_compensation = code;
                        break;
                    case 43:
                        if(CHECKBIT(group0,GCODE_GROUP_TOOLLENGTH))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group0,GCODE_GROUP_TOOLLENGTH);
                        g_gcparser_state.groups.tool_length_offset = 0;
                        break;
                    case 49:
                        if(CHECKBIT(group0,GCODE_GROUP_TOOLLENGTH))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group0,GCODE_GROUP_TOOLLENGTH);
                        g_gcparser_state.groups.tool_length_offset = 1;
                        break;
                    case 98:
                    case 99:
                        if(CHECKBIT(group0,GCODE_GROUP_RETURNMODE))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group0,GCODE_GROUP_RETURNMODE);
                        code -= 98;
                        g_gcparser_state.groups.return_mode = code;
                        break;
                    case 54:
                    case 55:
                    case 56:
                    case 57:
                    case 58:
                    case 59:
                        if(CHECKBIT(group1,GCODE_GROUP_COORDSYS))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group1,GCODE_GROUP_COORDSYS);
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
                        g_gcparser_state.groups.coord_system = code;
                        break;
                    case 61:
                        if(CHECKBIT(group1,GCODE_GROUP_PATH))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group1,GCODE_GROUP_PATH);
                        g_gcparser_state.groups.path_mode = 0;
                        break;
                    case 64:
                        if(CHECKBIT(group1,GCODE_GROUP_PATH))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group1,GCODE_GROUP_PATH);
                        g_gcparser_state.groups.path_mode = 1;
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
                        //53 = 4
                        //92 = 9
                        if(code >= 10)
                        {
                            code /= 10;
                        }

                        if(CHECKBIT(group1,GCODE_GROUP_NONMODAL))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group1,GCODE_GROUP_NONMODAL);
                        g_gcparser_state.groups.nonmodal = code;
                        break;
                    
                    default:
                        report_error(GCODE_UNKNOWN_GCOMMAND);
                        return ;
                }
                break;

            case 'M':
                code = (uint8_t)(word_val);
                switch(code)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 30:
                    case 60:
                        if(CHECKBIT(group1,GCODE_GROUP_STOPPING))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group1,GCODE_GROUP_STOPPING);
                        if(code >= 10)
                        {
                            code /= 10;
                        }
                        g_gcparser_state.groups.stopping = code;
                        break;
                    case 3:
                    case 4:
                    case 5:
                        if(CHECKBIT(group1,GCODE_GROUP_SPINDLE))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group1,GCODE_GROUP_SPINDLE);
                        code -= 3;
                        g_gcparser_state.groups.spindle_turning = code;
                        break;
                    case 7:
                        g_gcparser_state.groups.coolant |= 1;
                        break;
                    case 8:
                        g_gcparser_state.groups.coolant |= 2;
                        break;
                    case 9:
                        g_gcparser_state.groups.coolant = 0;
                        break;
                    case 48:
                    case 49:
                        if(CHECKBIT(group1,GCODE_GROUP_ENABLEOVER))
                        {
                            report_error(GCODE_MODAL_GROUP_VIOLATION);
                            return;
                        }

                        SETBIT(group1,GCODE_GROUP_ENABLEOVER);
                        code -= 48;
                        g_gcparser_state.groups.feed_speed_override = code;
                        break;
                    default:
                        break;
                }
            break;
        case 'X':
            if(CHECKBIT(word0, GCODE_WORD_X))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word0, GCODE_WORD_X);
            g_gcparser_state.words.xyzabc[0] = word_val;
            break;
        case 'Y':
            if(CHECKBIT(word0, GCODE_WORD_Y))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word0, GCODE_WORD_Y);
            g_gcparser_state.words.xyzabc[1] = word_val;
            break;
        case 'Z':
            if(CHECKBIT(word0, GCODE_WORD_Z))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word0, GCODE_WORD_Z);
            g_gcparser_state.words.xyzabc[2] = word_val;
            break;
        case 'A':
            if(CHECKBIT(word0, GCODE_WORD_A))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word0, GCODE_WORD_A);
            g_gcparser_state.words.xyzabc[3] = word_val;
            break;
        case 'B':
            if(CHECKBIT(word0, GCODE_WORD_B))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word0, GCODE_WORD_B);
            g_gcparser_state.words.xyzabc[4] = word_val;
            break;
        case 'C':
            if(CHECKBIT(word0, GCODE_WORD_C))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word0, GCODE_WORD_C);
            g_gcparser_state.words.xyzabc[5] = word_val;
            break;
        case 'D':
            if(CHECKBIT(word0, GCODE_WORD_D))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word0, GCODE_WORD_D);
            g_gcparser_state.words.d = word_val;
            break;
        case 'F':
            if(CHECKBIT(word0, GCODE_WORD_F))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word0, GCODE_WORD_F);
            g_gcparser_state.words.f = word_val;
            break;
        case 'H':
            if(CHECKBIT(word1, GCODE_WORD_H))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word1, GCODE_WORD_H);
            g_gcparser_state.words.h = word_val;
            break;
        case 'I':
            if(CHECKBIT(word1, GCODE_WORD_I))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word1, GCODE_WORD_I);
            g_gcparser_state.words.ijk[0] = word_val;
            break;
        case 'J':
            if(CHECKBIT(word1, GCODE_WORD_J))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word1, GCODE_WORD_J);
            g_gcparser_state.words.ijk[1] = word_val;
            break;
        case 'K':
            if(CHECKBIT(word1, GCODE_WORD_K))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word1, GCODE_WORD_K);
            g_gcparser_state.words.ijk[2] = word_val;
            break;
        case 'L':
            if(CHECKBIT(word1, GCODE_WORD_L))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word1, GCODE_WORD_L);
            g_gcparser_state.words.l= word_val;
            break;
        case 'P':
            if(CHECKBIT(word1, GCODE_WORD_P))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word1, GCODE_WORD_P);
            g_gcparser_state.words.p = word_val;
            break;
        case 'Q':
            if(CHECKBIT(word1, GCODE_WORD_Q))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word1, GCODE_WORD_Q);
            g_gcparser_state.words.q = word_val;
            break;
        case 'R':
            if(CHECKBIT(word1, GCODE_WORD_R))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word1, GCODE_WORD_R);
            g_gcparser_state.words.r = word_val;
            break;
        case 'S':
            if(CHECKBIT(word2, GCODE_WORD_S))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word2, GCODE_WORD_S);
            g_gcparser_state.words.s = word_val;
            break;
        case 'T':
            if(CHECKBIT(word2, GCODE_WORD_T))
            {
                report_error(GCODE_WORD_REPEATED);
                return;
            }

            SETBIT(word2, GCODE_WORD_T);
            g_gcparser_state.words.t = word_val;
            break;
        default:
            break;

        }

        //Step 2
        //In this step the parser will check for invalid values according to the RS274NGC v3
        //  1. At least a G or M command must exist in a line
        //  2. Words F, H, N, P, Q, R, S and T must be positive
        //  3. Word N must be an integer
        //  4. Motion codes must have at least one axis declared

        if(group0==0 || group1 == 0)
        {
            report_error(GCODE_WORD_REPEATED);
            return;
        }
    }
}