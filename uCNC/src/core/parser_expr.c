/*
	Name: parser_expr.c
	Description: Parses RS274NGC expressions and subrotines (O-Codes) commands

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 24/10/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"

#include <stdint.h>
#include <math.h>
#include <string.h>
#include <float.h>

/**
 *
 *
 * RS274NGC expressions extensions
 * This includes:
 *  - numbered parameters
 *  - O Codes
 *
 *
 */
#ifdef ENABLE_RS274NGC_EXPRESSIONS
char parser_backtrack;
extern float g_parser_num_params[RS274NGC_MAX_USER_VARS];
#define STRLEN(s) (sizeof(s) / sizeof(s[0]))
#define STRCMP(sram, srom) rom_strcmp(sram, __romstr__(srom))
#ifdef ENABLE_NAMED_PARAMETERS
// carefull this should be at least as big as the longer named parameter
#define NAMED_PARAM_MAX_LEN 24
typedef struct named_param_
{
	const char *const name;
	uint16_t id;
} named_param_t;
#define DECL_NAMED_PARAM(name) static const char gp##name[] __rom__ = #name
#define NAMED_PARAM(p_name, p_id) {.name = gp##p_name, .id = p_id}
DECL_NAMED_PARAM(_vmajor);
DECL_NAMED_PARAM(_vminor);
DECL_NAMED_PARAM(_line);
DECL_NAMED_PARAM(_motion_mode);
DECL_NAMED_PARAM(_plane);
DECL_NAMED_PARAM(_ccomp);
DECL_NAMED_PARAM(_metric);
DECL_NAMED_PARAM(_imperial);
DECL_NAMED_PARAM(_absolute);
DECL_NAMED_PARAM(_incremental);
DECL_NAMED_PARAM(_inverse_time);
DECL_NAMED_PARAM(_units_per_minute);
DECL_NAMED_PARAM(_units_per_rev);
DECL_NAMED_PARAM(_coord_system);
DECL_NAMED_PARAM(_tool_offset);
DECL_NAMED_PARAM(_retract_r_plane);
DECL_NAMED_PARAM(_retract_old_z);
DECL_NAMED_PARAM(_spindle_rpm_mode);
DECL_NAMED_PARAM(_spindle_css_mode);
DECL_NAMED_PARAM(_ijk_absolute_mode);
DECL_NAMED_PARAM(_lathe_diameter_mode);
DECL_NAMED_PARAM(_lathe_radius_mode);
DECL_NAMED_PARAM(_spindle_on);
DECL_NAMED_PARAM(_spindle_cw);
DECL_NAMED_PARAM(_mist);
DECL_NAMED_PARAM(_flood);
DECL_NAMED_PARAM(_speed_override);
DECL_NAMED_PARAM(_feed_override);
DECL_NAMED_PARAM(_adaptive_feed);
DECL_NAMED_PARAM(_feed_hold);
DECL_NAMED_PARAM(_feed);
DECL_NAMED_PARAM(_rpm);
DECL_NAMED_PARAM(_x);
DECL_NAMED_PARAM(_y);
DECL_NAMED_PARAM(_z);
DECL_NAMED_PARAM(_a);
DECL_NAMED_PARAM(_b);
DECL_NAMED_PARAM(_c);
DECL_NAMED_PARAM(_u);
DECL_NAMED_PARAM(_v);
DECL_NAMED_PARAM(_w);
DECL_NAMED_PARAM(_abs_x);
DECL_NAMED_PARAM(_abs_y);
DECL_NAMED_PARAM(_abs_z);
DECL_NAMED_PARAM(_abs_a);
DECL_NAMED_PARAM(_abs_b);
DECL_NAMED_PARAM(_abs_c);
DECL_NAMED_PARAM(_current_tool);
DECL_NAMED_PARAM(_current_pocket);
DECL_NAMED_PARAM(_selected_tool);
DECL_NAMED_PARAM(_selected_pocket);
DECL_NAMED_PARAM(_value);
DECL_NAMED_PARAM(_value_returned);
DECL_NAMED_PARAM(_task);
DECL_NAMED_PARAM(_call_level);
DECL_NAMED_PARAM(_remap_level);
static const named_param_t named_params[] __rom__ = {
		NAMED_PARAM(_vmajor, 6001),
		NAMED_PARAM(_vminor, 6002),
		NAMED_PARAM(_line, 6003),
		NAMED_PARAM(_motion_mode, 6010),
		NAMED_PARAM(_plane, 6011),
		NAMED_PARAM(_ccomp, 6012),
		NAMED_PARAM(_metric, 6013),
		NAMED_PARAM(_imperial, 6014),
		NAMED_PARAM(_absolute, 6015),
		NAMED_PARAM(_incremental, 6016),
		NAMED_PARAM(_inverse_time, 6017),
		NAMED_PARAM(_units_per_minute, 6018),
		NAMED_PARAM(_units_per_rev, 6019),
		NAMED_PARAM(_coord_system, 6020),
		NAMED_PARAM(_tool_offset, 6021),
		NAMED_PARAM(_retract_r_plane, 6022),
		NAMED_PARAM(_retract_old_z, 6023),
		NAMED_PARAM(_spindle_rpm_mode, 6024),
		NAMED_PARAM(_spindle_css_mode, 6025),
		NAMED_PARAM(_ijk_absolute_mode, 6026),
		NAMED_PARAM(_lathe_diameter_mode, 6027),
		NAMED_PARAM(_lathe_radius_mode, 6028),
		NAMED_PARAM(_spindle_on, 6030),
		NAMED_PARAM(_spindle_cw, 6031),
		NAMED_PARAM(_mist, 6032),
		NAMED_PARAM(_flood, 6033),
		NAMED_PARAM(_speed_override, 6040),
		NAMED_PARAM(_feed_override, 6041),
		NAMED_PARAM(_adaptive_feed, 6042),
		NAMED_PARAM(_feed_hold, 6043),
		NAMED_PARAM(_feed, 6044),
		NAMED_PARAM(_rpm, 6045),
		NAMED_PARAM(_x, 5420),
		NAMED_PARAM(_y, 5421),
		NAMED_PARAM(_z, 5422),
		NAMED_PARAM(_a, 5423),
		NAMED_PARAM(_b, 5424),
		NAMED_PARAM(_c, 5425),
		NAMED_PARAM(_u, 5426),
		NAMED_PARAM(_v, 5427),
		NAMED_PARAM(_w, 5428),
		NAMED_PARAM(_abs_x, 6050),
		NAMED_PARAM(_abs_y, 6051),
		NAMED_PARAM(_abs_z, 6052),
		NAMED_PARAM(_abs_a, 6053),
		NAMED_PARAM(_abs_b, 6054),
		NAMED_PARAM(_abs_c, 6055),
		NAMED_PARAM(_current_tool, 5400),
		NAMED_PARAM(_current_pocket, 6060),
		NAMED_PARAM(_selected_tool, 6061),
		NAMED_PARAM(_selected_pocket, 6062),
		NAMED_PARAM(_value, 6100),
		NAMED_PARAM(_value_returned, 6101),
		NAMED_PARAM(_task, 6110),
		NAMED_PARAM(_call_level, 6111),
		NAMED_PARAM(_remap_level, 6112)};
#endif
#ifdef ENABLE_O_CODES
#include "../modules/file_system.h"
#ifndef OCODE_PARSER_STACK_DEPTH
#define OCODE_PARSER_STACK_DEPTH MAX_PARSER_STACK_DEPTH
#endif
#ifndef OCODE_CONTEXT_STACK_DEPTH
#define OCODE_CONTEXT_STACK_DEPTH 10
#endif
#define OCODE_CONTEXT_SIZE (sizeof(float) * MIN(RS274NGC_MAX_USER_VARS, 30))
#ifndef OCODE_DRIVE
#define OCODE_DRIVE 'C'
#endif

#define O_CODE_OP_DISCARD 0x80
#define O_CODE_OP_OPTIONS_MASK 0x03
#define O_CODE_BASE_TYPE(x) (x & ~(O_CODE_OP_DISCARD | O_CODE_OP_OPTIONS_MASK))
#define O_CODE_OP_CALL 1
#define O_CODE_OP_SUB 2
#define O_CODE_OP_IF 4
#define O_CODE_OP_IF_FOUND (O_CODE_OP_IF | 1)
#define O_CODE_OP_IF_DISCARD (O_CODE_OP_IF | O_CODE_OP_DISCARD)
#define O_CODE_OP_WHILE 8
#define O_CODE_OP_DO_WHILE (O_CODE_OP_WHILE | 1)
#define O_CODE_OP_WHILE_DISCARD (O_CODE_OP_WHILE | O_CODE_OP_DISCARD)
#define O_CODE_OP_WHILE_BREAK (O_CODE_OP_WHILE | O_CODE_OP_DISCARD | 3)
#define O_CODE_OP_REPEAT 16
#define O_CODE_OP_REPEAT_DISCARD (O_CODE_OP_REPEAT | O_CODE_OP_DISCARD)

#define STATUS_O_CODE_SUBROTINE 100
#define STATUS_OCODE_ERROR_FILE_NOT_FOUND 101
#define STATUS_OCODE_ERROR_INVALID_NUMBER 102
#define STATUS_OCODE_ERROR_INVALID_OPERATION 103
#define STATUS_OCODE_ERROR_INVALID_EXPRESSION 104
#define STATUS_OCODE_ERROR_STACK_UNDERFLOW 105
#define STATUS_OCODE_ERROR_STACK_OVERFLOW 106

#define STATUS_OCODE_ERROR_MIN STATUS_OCODE_ERROR_FILE_NOT_FOUND
#define STATUS_OCODE_ERROR_MAX STATUS_OCODE_ERROR_STACK_OVERFLOW

static fs_file_t *o_code_file;
static uint32_t o_code_file_pos;
static bool o_code_file_changed;
bool o_code_returned;
float o_code_return_value;

typedef struct o_code_stack_
{
	uint16_t code;
	uint8_t op;
	uint32_t pos;
	int32_t loop;
	uint8_t context_index;
} o_code_stack_t;

static o_code_stack_t o_code_stack[OCODE_PARSER_STACK_DEPTH];
uint8_t o_code_stack_index;
uint8_t o_code_stack_context_index;
float o_code_stack_context_vars[OCODE_CONTEXT_STACK_DEPTH][MIN(RS274NGC_MAX_USER_VARS, 30)];
#define O_CODE_FILE_CLOSE 1
#define O_CODE_FILE_CLOSE_ALL 2
#endif

/**
 *
 *
 * Numbered parameters and expressions parsing
 *
 *
 */
#define OP_LEVEL0 (0 << 5)
#define OP_LEVEL1 (1 << 5)
#define OP_LEVEL2 (2 << 5)
#define OP_LEVEL3 (3 << 5)
#define OP_LEVEL4 (4 << 5)
#define OP_LEVEL5 (5 << 5)
#define OP_LEVEL6 (6 << 5)
#define OP_LEVEL7 (7 << 5)
#define OP_LEVEL(X) (X & (7 << 5))

#define OP_INVALID 0

#define OP_AND (OP_LEVEL0 | 1)
#define OP_OR (OP_LEVEL0 | 2)
#define OP_XOR (OP_LEVEL0 | 3)

#define OP_EQ (OP_LEVEL1 | 1)
#define OP_NE (OP_LEVEL1 | 2)
#define OP_GT (OP_LEVEL1 | 3)
#define OP_GE (OP_LEVEL1 | 4)
#define OP_LT (OP_LEVEL1 | 5)
#define OP_LE (OP_LEVEL1 | 6)

#define OP_ADD (OP_LEVEL2 | 1)
#define OP_SUB (OP_LEVEL2 | 2)

#define OP_MUL (OP_LEVEL3 | 1)
#define OP_DIV (OP_LEVEL3 | 2)
#define OP_MOD (OP_LEVEL3 | 3)

#define OP_POW (OP_LEVEL4 | 1)

#define OP_NEG (OP_LEVEL5 | 1)
#define OP_SQRT (OP_LEVEL5 | 5)
#define OP_COS (OP_LEVEL5 | 10)
#define OP_SIN (OP_LEVEL5 | 11)
#define OP_TAN (OP_LEVEL5 | 12)
#define OP_ACOS (OP_LEVEL5 | 13)
#define OP_ASIN (OP_LEVEL5 | 14)
#define OP_ATAN (OP_LEVEL5 | 15)
#define OP_ATAN_DIV (OP_LEVEL5 | 16)
#define OP_EXP (OP_LEVEL5 | 17)
#define OP_LN (OP_LEVEL5 | 18)
#define OP_ABS (OP_LEVEL5 | 19)
#define OP_FIX (OP_LEVEL5 | 20)
#define OP_FUP (OP_LEVEL5 | 21)
#define OP_ROUND (OP_LEVEL5 | 22)
#define OP_EXISTS (OP_LEVEL5 | 23)

#define OP_PARSER_VAR (OP_LEVEL6 | 1)
#define OP_EXPR_END (OP_LEVEL6 | 2)
#define OP_EXPR_START (OP_LEVEL6 | 3)

#define OP_WORD 201

#define OP_REAL 203
#define OP_NAMED_PARAM 204
#define OP_ASSIGN 252
#define OP_ENDLINE 253

bool parser_assert_op(parser_stack_t stack, float rhs)
{
	int val = 0;
	switch (stack.op)
	{
	case OP_PARSER_VAR:
		if (rhs < 1 || floorf(rhs) > rhs)
		{
			return false;
		}

		val = (int)rhs;

		if (val > RS274NGC_MAX_USER_VARS && val < 5000)
		{
			return false;
		}

		switch (val)
		{
		case 5061:
		case 5062:
		case 5063:
		case 5064:
		case 5065:
		case 5066:
			if (val - 5061 >= AXIS_COUNT)
			{
				return false;
			}
		}
	}

	return true;
}

float parser_exec_op(parser_stack_t stack, float rhs)
{
	switch (stack.op)
	{
	case OP_ADD:
		return stack.lhs + rhs;
	case OP_SUB:
		return stack.lhs - rhs;
	case OP_MUL:
		return stack.lhs * rhs;
	case OP_DIV:
		return stack.lhs / rhs;
	case OP_POW:
		return powf(stack.lhs, rhs);
	case OP_SQRT:
		return sqrtf(rhs);
	case OP_MOD:
		return (float)((int64_t)stack.lhs % (int64_t)rhs);
	case OP_AND:
		return (float)((int64_t)stack.lhs & (int64_t)rhs);
	case OP_OR:
		return (float)((int64_t)stack.lhs | (int64_t)rhs);
	case OP_XOR:
		return (float)((int64_t)stack.lhs ^ (int64_t)rhs);
	case OP_EQ:
		return (float)((int64_t)stack.lhs == (int64_t)rhs);
	case OP_NE:
		return (float)((int64_t)stack.lhs != (int64_t)rhs);
	case OP_GT:
		return (float)((int64_t)stack.lhs > (int64_t)rhs);
	case OP_GE:
		return (float)((int64_t)stack.lhs >= (int64_t)rhs);
	case OP_LT:
		return (float)((int64_t)stack.lhs < (int64_t)rhs);
	case OP_LE:
		return (float)((int64_t)stack.lhs <= (int64_t)rhs);
	case OP_COS:
		return cosf(rhs * DEG_RAD_MULT);
	case OP_SIN:
		return sinf(rhs * DEG_RAD_MULT);
	case OP_TAN:
		return tanf(rhs * DEG_RAD_MULT);
	case OP_ACOS:
		return RAD_DEG_MULT * acosf(rhs);
	case OP_ASIN:
		return RAD_DEG_MULT * asinf(rhs);
	// case OP_ATAN:
	// 	return rhs;
	case OP_ATAN_DIV:
		// special atan case
		return RAD_DEG_MULT * atan2f(stack.lhs, rhs);
	case OP_EXP:
		return expf(rhs);
	case OP_LN:
		return logf(rhs);
	case OP_ABS:
		return fabs(rhs);
	case OP_FIX:
		return floorf(rhs);
	case OP_FUP:
		return ceilf(rhs);
	case OP_ROUND:
		return roundf(rhs);
	case OP_NEG:
		return -rhs;
	case OP_EXISTS:
		if (rhs < 1 || rhs > RS274NGC_MAX_USER_VARS || floorf(rhs) > rhs)
		{
			return 0;
		}
		return 1;
	case OP_PARSER_VAR:
		return parser_get_parameter((int)rhs);
	default:
		return rhs;
	}
}

/**
 *
 * This determines the operation that will be performed in an expression symbol
 *
 *
 */

static FORCEINLINE uint8_t parser_get_operation(uint8_t stack_depth, parser_stack_t *stack)
{
	bool can_call_unary_func = (stack_depth <= 1) ? true : (stack[stack_depth - 1].op <= OP_NEG || stack[stack_depth - 1].op == OP_EXPR_START);
	char c = (char)parser_get_next_preprocessed(true);
	c = TOUPPER(c);
	uint8_t result = OP_INVALID;
	uint8_t i = 0;

	if ((c >= '0' && c <= '9') || c == '.')
	{
		parser_backtrack = 0;
		return OP_REAL;
	}
#ifdef ENABLE_NAMED_PARAMETERS
	if (c == '<' && stack_depth && stack[stack_depth - 1].op == OP_PARSER_VAR)
	{
		return OP_NAMED_PARAM;
	}
#endif
	else if (c < 'A' || c > 'Z')
	{
		if (!c)
		{
			return OP_ENDLINE;
		}

		switch (c)
		{
		case '=':
			return OP_ENDLINE;
			break;
		case '[':
			result = OP_EXPR_START;
			break;
		case ']':
			parser_backtrack = 0;
			parser_get_next_preprocessed(false);
			return OP_EXPR_END;
		case '#':
			parser_backtrack = 0;
			parser_get_next_preprocessed(false);
			return OP_PARSER_VAR;
		case '*':
			parser_get_next_preprocessed(false);
			c = (char)parser_get_next_preprocessed(true);

			if (c != '*')
			{
				parser_backtrack = c;
				return OP_MUL;
			}

			result = OP_POW;
			break;
		case '/':
			result = OP_DIV;
			break;
		case '-':
			result = OP_SUB;

			if (parser_backtrack == '-')
			{
				result = OP_NEG;
			}
			break;
		case '+':
			result = OP_ADD;
			break;
		default:
			return OP_INVALID;
		}

		parser_get_next_preprocessed(false);
		parser_backtrack = (char)parser_get_next_preprocessed(true);
		return result;
	}
	else if (!can_call_unary_func) // if can't do unary checks for possible binary op
	{
		char peek = 0;
		char peek2 = 0;
		switch (c)
		{
		case 'A':
			peek = 'N';
			break;
		case 'M':
			peek = 'O';
			break;
		case 'O':
			peek = 'R';
			break;
		case 'X':
			peek = 'O';
			break;
		case 'E':
			peek = 'Q';
			break;
		case 'N':
			peek = 'E';
			break;
		case 'G':
			peek = 'T';
			peek2 = 'E';
			break;
		case 'L':
			peek = 'T';
			peek2 = 'E';
			break;
		default:
			return OP_WORD;
		}
		parser_backtrack = parser_get_next_preprocessed(false);
		char p = TOUPPER(parser_get_next_preprocessed(true));
		if (peek != p && peek2 != p)
		{
			return OP_WORD;
		}

		i = 1;
	}

	char str[7];
	memset(str, 0, sizeof(str));
	parser_backtrack = c;
	str[0] = c;

	for (; i < 7; i++)
	{
		c = parser_get_next_preprocessed(true);
		c = TOUPPER(c);
		if (c < 'A' || c > 'Z')
		{
			break;
		}
		parser_get_next_preprocessed(false);
		str[i] = (char)c;
	}

	if (strlen(str) == 1)
	{
		return OP_WORD;
	}

	parser_backtrack = 0;

	if (!STRCMP(str, "MOD"))
	{
		return OP_MOD;
	}
	if (!STRCMP(str, "AND"))
	{
		return OP_AND;
	}
	if (!STRCMP(str, "OR"))
	{
		return OP_OR;
	}
	if (!STRCMP(str, "XOR"))
	{
		return OP_XOR;
	}
	if (!STRCMP(str, "EQ"))
	{
		return OP_EQ;
	}
	if (!STRCMP(str, "NE"))
	{
		return OP_NE;
	}
	if (!STRCMP(str, "GT"))
	{
		return OP_GT;
	}
	if (!STRCMP(str, "GE"))
	{
		return OP_GE;
	}
	if (!STRCMP(str, "LT"))
	{
		return OP_LT;
	}
	if (!STRCMP(str, "LE"))
	{
		return OP_LE;
	}

	if (c != '[')
	{
		return OP_INVALID;
	}

	if (!STRCMP(str, "SQRT"))
	{
		return OP_SQRT;
	}
	if (!STRCMP(str, "COS"))
	{
		return OP_COS;
	}
	if (!STRCMP(str, "SIN"))
	{
		return OP_SIN;
	}
	if (!STRCMP(str, "TAN"))
	{
		return OP_TAN;
	}
	if (!STRCMP(str, "ACOS"))
	{
		return OP_ACOS;
	}
	if (!STRCMP(str, "ASIN"))
	{
		return OP_ASIN;
	}
	if (!STRCMP(str, "ATAN"))
	{
		return OP_ATAN;
	}
	if (!STRCMP(str, "EXP"))
	{
		return OP_EXP;
	}
	if (!STRCMP(str, "LN"))
	{
		return OP_LN;
	}
	if (!STRCMP(str, "ABS"))
	{
		return OP_ABS;
	}
	if (!STRCMP(str, "FIX"))
	{
		return OP_FIX;
	}
	if (!STRCMP(str, "FUP"))
	{
		return OP_FUP;
	}
	if (!STRCMP(str, "ROUND"))
	{
		return OP_ROUND;
	}
	if (!STRCMP(str, "EXISTS"))
	{
		return OP_EXISTS;
	}

	return OP_INVALID;
}

uint8_t parser_get_float(float *value)
{
	uint8_t result = NUMBER_UNDEF;
	float rhs = 0;
	// initializes the stack
	uint8_t stack_depth = 1;
	parser_stack_t stack[MAX_PARSER_STACK_DEPTH];
	memset(stack, 0, sizeof(stack));
	stack[0].op = OP_ASSIGN;

	for (;;)
	{
		uint8_t op = parser_get_operation(stack_depth, stack);

		switch (op)
		{
		case OP_EXPR_START:
		case OP_PARSER_VAR:
			stack[stack_depth].op = op;
			stack_depth++;
			break;
		case OP_INVALID:
			if (stack_depth <= 1)
			{
				return NUMBER_UNDEF;
			}
			__FALL_THROUGH__
		case OP_ENDLINE:
		case OP_WORD:
		case OP_EXPR_END:
			while (stack_depth > 1)
			{
				stack_depth--;
				op = stack[stack_depth].op;
				if (!parser_assert_op(stack[stack_depth], rhs))
				{
					return NUMBER_UNDEF;
				}
				rhs = parser_exec_op(stack[stack_depth], rhs);
				stack[stack_depth].op = 0;
				if (op == OP_EXPR_START)
				{
					break;
				}
				if (!stack_depth && op != OP_EXPR_START)
				{
					return NUMBER_UNDEF;
				}
			}
			break;
#ifdef ENABLE_NAMED_PARAMETERS
		case OP_NAMED_PARAM:
			result = parser_get_namedparam_id(&rhs);
			break;
#endif
		case OP_REAL:
			result = prt_atof((void *)parser_get_next_preprocessed, NULL, &rhs);
			break;
		default:
			while (stack_depth)
			{
				if (OP_LEVEL(stack[stack_depth - 1].op) < OP_LEVEL(op) || stack[stack_depth - 1].op >= OP_EXPR_START)
				{
					break;
				}
				// atan must be preceded by a div
				if (stack[stack_depth - 1].op == OP_ATAN)
				{
					if (op != OP_DIV)
					{
						return NUMBER_UNDEF;
					}
					op = OP_ATAN_DIV;
				}
				stack_depth--;
				if (!parser_assert_op(stack[stack_depth], rhs))
				{
					return NUMBER_UNDEF;
				}
				rhs = parser_exec_op(stack[stack_depth], rhs);
				stack[stack_depth].op = 0;
			}
			stack[stack_depth].op = op;
			stack[stack_depth].lhs = rhs;
			rhs = 0;
			stack_depth++;
			break;
		}

		// the index of a user var was reduced. Can replace value
		while (OP_PARSER_VAR == stack[stack_depth - 1].op && op != OP_PARSER_VAR)
		{
			rhs = parser_exec_op(stack[--stack_depth], rhs);
		}

		// stack processed
		// can return value
		if (stack_depth <= 1)
		{
			*value = parser_exec_op(stack[0], rhs);
			result |= (rhs < 0) ? NUMBER_ISNEGATIVE : 0;
			result |= (floorf(rhs) != rhs) ? NUMBER_ISFLOAT : 0;
			return result;
		}

		result = NUMBER_OK;
	}

	return result;
}

/**
 *
 *
 * O Codes parsing extension
 *
 *
 */
#ifdef ENABLE_O_CODES

static uint8_t o_code_getc(void)
{
	if (o_code_file)
	{
		if (o_code_file_changed)
		{
			o_code_file_changed = false;
			return EOL;
		}
		if (fs_available(o_code_file))
		{
			uint8_t c = 0;
			fs_read(o_code_file, &c, 1);
			return c;
		}
	}
	return FILE_EOF; // end of file marker
}

static uint8_t o_code_file_flush(void)
{
	if (!o_code_stack_index)
	{
		grbl_stream_change(NULL);
	}

	return FILE_EOF; // end of file marker
}

DECL_GRBL_STREAM(o_code_stream, o_code_getc, NULL, NULL, NULL, NULL);

static void o_code_open(uint8_t index)
{
	if (o_code_stack[index].op == O_CODE_OP_CALL || o_code_stack[index].op == O_CODE_OP_SUB)
	{
		if (o_code_file)
		{
			fs_close(o_code_file);
			o_code_file = NULL;
		}

		char o_subrotine[32];
		memset(o_subrotine, 0, sizeof(o_subrotine));
		str_sprintf(o_subrotine, "/%c/o%d.nc", OCODE_DRIVE, o_code_stack[index].code);
		o_code_file = fs_open(o_subrotine, "r");
		if (!o_code_file)
		{
			return;
		}
		o_code_stack[index].op = O_CODE_OP_SUB;
		// reload file and rewind stack
		fs_seek(o_code_file, o_code_file_pos);

		o_code_file_changed = true;
#ifdef ENABLE_O_CODES_VERBOSE
		grbl_stream_readonly(o_code_getc, NULL, NULL);
#else
		grbl_stream_change(&o_code_stream);
#endif
	}
}

static uint8_t o_code_entry_point(uint8_t index)
{
	// find previous sub entry point
	while (o_code_stack[index].op != O_CODE_OP_SUB && index)
	{
		o_code_stack[index].code = 0;
		o_code_stack[index].op = 0;
		index--;
	}

	return index;
}

static uint8_t o_code_close(uint8_t index)
{
	// close file
	if (index)
	{
		if (o_code_file)
		{
			fs_close(o_code_file);
			o_code_file = NULL;
		}

		index = o_code_entry_point(index);
	}

	return index;
}

bool o_code_end_subrotine(void)
{
	uint8_t index = o_code_stack_index;
	if (index)
	{
		index = o_code_close(index);
		// restore user vars
		memcpy(g_parser_num_params, o_code_stack_context_vars[o_code_stack[index].context_index], OCODE_CONTEXT_SIZE);
		o_code_stack_context_index = o_code_stack[index].context_index;

		// restore file pointer and mark entry for (re)call
		o_code_file_pos = o_code_stack[index].pos;
		memset(&o_code_stack[index], 0, sizeof(o_code_stack_t));
	}

	if (index)
	{
		// find previous sub entry point
		index = o_code_entry_point(index);
		// top sub not exited yet
		o_code_open(index);

		return true;
	}

	// clear and close all
	if (o_code_file)
	{
		fs_close(o_code_file);
		o_code_file = NULL;
	}
	memset(o_code_stack, 0, sizeof(o_code_stack));
	o_code_stack_index = 0;
	o_code_stack_context_index = 0;
	// grbl_stream_change(NULL);
	grbl_stream_readonly(o_code_file_flush, NULL, NULL);
	return false;
}

static bool o_code_seek(uint32_t pos)
{
	if (o_code_file)
	{
		fs_seek(o_code_file, pos - 1);
		grbl_stream_readonly(o_code_getc, NULL, NULL);
		return true;
	}
	return false;
}

uint8_t o_code_validate(uint8_t op, uint16_t ocode_id, bool is_new)
{
	for (uint8_t index = o_code_stack_index; index != 0;)
	{
		index--;
		if (o_code_stack[index].code == ocode_id)
		{
			// checks if the ocode and the operation type match
			// it's an error if the ocode id is related to another operation
			return ((O_CODE_BASE_TYPE(o_code_stack[index].op) & op) && is_new) ? 0 : (index + 1);
		}
		if (o_code_stack[index].op == O_CODE_OP_SUB)
		{
			return (is_new) ? (index + 1) : 0;
		}
	}

	return 0;
}

#ifdef PROCESS_COMMENTS
extern bool g_mute_comment_output;
#endif
static void o_code_discard(void)
{
#ifdef PROCESS_COMMENTS
	g_mute_comment_output = true;
#endif
	while (parser_get_next_preprocessed(true) != 'O')
	{
		parser_discard_command();
	}
#ifdef PROCESS_COMMENTS
	g_mute_comment_output = false;
#endif
}

static void FORCEINLINE o_code_word_error(uint8_t *error)
{
	if (*error >= STATUS_OCODE_ERROR_MIN && *error <= STATUS_OCODE_ERROR_MAX)
	{
		// Error in o-code, do not continue execution and empty the stack.
		if (o_code_file)
		{
			fs_close(o_code_file);
			o_code_file = NULL;
		}
		if (o_code_stack_index)
		{
			for (uint16_t i = 0; i < RS274NGC_MAX_USER_VARS; i++)
			{
				parser_set_parameter(i + 1, o_code_stack_context_vars[0][i]);
			}
		}
		memset(o_code_stack, 0, sizeof(o_code_stack));
		o_code_stack_index = 0;
		o_code_file_pos = 0;
		//		grbl_stream_change(NULL);
		grbl_stream_readonly(o_code_file_flush, NULL, NULL);
	}
}

uint8_t parser_ocode_word(uint16_t code, parser_state_t *new_state, parser_cmd_explicit_t *cmd)
{
	uint8_t error __attribute__((__cleanup__(o_code_word_error))) = STATUS_GCODE_UNSUPPORTED_COMMAND;
	float op_arg = 0;
	uint8_t op_arg_error = NUMBER_UNDEF;
	char o_cmd[16];
	memset(o_cmd, 0, sizeof(o_cmd));
	uint32_t loop_ret = 0;

	uint8_t index = o_code_stack_index;

	uint16_t ocode_id = code;

	// this checks if the code is to be discarded or not after come conditional
	if (index && ((o_code_stack[index - 1].op == O_CODE_OP_IF) || (o_code_stack[index - 1].op & O_CODE_OP_DISCARD)) && o_code_stack[index - 1].code != ocode_id)
	{
		// keep discarding
		o_code_discard();
		error = STATUS_OK;
		return error;
	}

	for (uint8_t i = 0; i < 16; i++)
	{
		char c = parser_get_next_preprocessed(true);
		c = TOUPPER(c);
		if (c < 'A' || c > 'Z')
		{
			break;
		}
		o_cmd[i] = c;
		parser_get_next_preprocessed(false);
	}

	if (o_code_file)
	{
		loop_ret = o_code_file->file_info.size - fs_available(o_code_file);
	}
	op_arg_error = parser_get_float(&op_arg);

	/**
	 * Sub rotine call
	 */
	if (!STRCMP(o_cmd, "CALL"))
	{
		// store user vars
		uint8_t i_context = o_code_stack_context_index++;
		memcpy(&o_code_stack_context_vars[i_context], g_parser_num_params, OCODE_CONTEXT_SIZE);

		// check if file exists
		char o_subrotine[32];
		memset(o_subrotine, 0, sizeof(o_subrotine));
		str_sprintf(o_subrotine, "/%c/o%d.nc", OCODE_DRIVE, ocode_id);
		fs_file_info_t finfo;
		fs_finfo(o_subrotine, &finfo);
		if (!finfo.size)
		{
			error = STATUS_OCODE_ERROR_INVALID_OPERATION;
			return error;
		}

		// call parameters
		uint16_t arg_i = 0;
		while (op_arg_error == NUMBER_OK)
		{
			// load args
			g_parser_num_params[arg_i++] = op_arg;
			op_arg_error = parser_get_float(&op_arg);
		}

		//		parser_get_next_preprocessed(false);

		// store operation in the stack
		o_code_stack[index].code = ocode_id;
		o_code_stack[index].op = O_CODE_OP_CALL;
		o_code_stack[index].context_index = i_context;

		// workaround to ftell
		o_code_stack[index].pos = (o_code_file) ? (o_code_file->file_info.size - fs_available(o_code_file)) : 0;
		o_code_open(index);

		o_code_stack_index++;
		error = STATUS_OK;
		return error;
	}

	/**
	 * If, elseif and else
	 */
	if (!STRCMP(o_cmd, "IF") || !STRCMP(o_cmd, "ELSEIF") || !STRCMP(o_cmd, "ELSE") || !STRCMP(o_cmd, "ENDIF"))
	{
		uint8_t type = strlen(o_cmd); // If=2 ElseIf=6 Else=4
		error = STATUS_OK;
		switch (type)
		{
		case 2:
		case 6:
			if (op_arg_error != NUMBER_OK)
			{
				error = STATUS_OCODE_ERROR_INVALID_OPERATION;
				return error;
			}
			break;
		}

		if (!o_code_validate(O_CODE_OP_IF, ocode_id, (type == 2) ? true : false))
		{
			error = STATUS_OCODE_ERROR_INVALID_OPERATION;
			return error;
		}

		index--;
		switch (type)
		{
		case 2:
			index = o_code_stack_index++;
			o_code_stack[index].code = ocode_id;
			o_code_stack[index].op = O_CODE_OP_IF;
			break;
		case 4:
			op_arg = 1; // else is always 1
			break;
		}

		switch (type)
		{
		case 5:
			if (index)
			{
				o_code_stack[index].code = 0;
				o_code_stack[index].op = 0;
				o_code_stack_index--;
				error = STATUS_OK;
			}
			else
			{
				error = STATUS_OCODE_ERROR_STACK_UNDERFLOW;
			}
			break;
		default:
			if ((o_code_stack[index].op == O_CODE_OP_IF) && op_arg) // condition met
			{
				o_code_stack[index].op = O_CODE_OP_IF_FOUND; // signals that the condition was met
			}
			else
			{
				if (o_code_stack[index].op == O_CODE_OP_IF_FOUND) // discard all remaining code
				{
					o_code_stack[index].op = O_CODE_OP_IF_DISCARD;
				}
				o_code_discard();
			}
			break;
		}

		error = STATUS_OK;
		return error;
	}

	if (!STRCMP(o_cmd, "RETURN") || !STRCMP(o_cmd, "ENDSUB"))
	{
		for (uint8_t index = o_code_stack_index; index != 0;)
		{
			index--;
			if (o_code_stack[index].op == O_CODE_OP_SUB)
			{
				bool found = (o_code_stack[index].code == ocode_id);
				o_code_stack_index = index;
				o_code_end_subrotine();
				if (found)
				{
					error = STATUS_OK;
					return error;
				}
				break;
			}
		}

		error = STATUS_OCODE_ERROR_INVALID_OPERATION;
		return error;
	}

	/**
	 * continue and break
	 */
	if (!STRCMP(o_cmd, "CONTINUE") || !STRCMP(o_cmd, "BREAK"))
	{
		uint8_t type = strlen(o_cmd);
		index = o_code_validate(O_CODE_OP_REPEAT, ocode_id, false);
		// tries repeat
		if (!index)
		{
			// if not repeat try while
			index = o_code_validate(O_CODE_OP_WHILE, ocode_id, false);
		}

		// none of the above were found. send error
		if (!index)
		{
			error = STATUS_OCODE_ERROR_INVALID_OPERATION;
			return error;
		}

		// revert and clean stack
		for (uint8_t i = o_code_stack_index; i != index;)
		{
			i--;
			memset(&o_code_stack[i], 0, sizeof(o_code_stack_t));
		}
		o_code_stack_index = index--;
		// if repeat
		if (O_CODE_BASE_TYPE(o_code_stack[index].op) == O_CODE_OP_REPEAT)
		{
			o_code_stack[index].op = O_CODE_OP_REPEAT_DISCARD;
			if (type == 5)
			{
				o_code_stack[index].loop = 0; // forces brake
			}
		}
		else
		{ // else is while
			o_code_stack[index].op = (type == 5) ? O_CODE_OP_WHILE_BREAK : O_CODE_OP_WHILE_DISCARD;
		}
		// start command discard
		o_code_discard();
		error = STATUS_OK;
		return error;
	}

	/**
	 * do while and while
	 */
	if (!STRCMP(o_cmd, "DO") || !STRCMP(o_cmd, "WHILE") || !STRCMP(o_cmd, "ENDWHILE"))
	{
		uint8_t type = strlen(o_cmd);

		index = o_code_validate(O_CODE_OP_WHILE, ocode_id, false);
		if (index)
		{
			index--;
		}

		switch (type)
		{
		case 8:
			if (!index)
			{
				error = STATUS_OCODE_ERROR_INVALID_OPERATION;
				return error;
			}

			if (O_CODE_BASE_TYPE(o_code_stack[index].op) == O_CODE_OP_WHILE && ocode_id == o_code_stack[index].code)
			{
				switch (o_code_stack[index].op)
				{
				case O_CODE_OP_DO_WHILE:
					error = STATUS_OCODE_ERROR_INVALID_NUMBER;
					return error;
				case O_CODE_OP_WHILE_DISCARD:
					// clear the discard and continue
					o_code_stack[index].op = O_CODE_OP_WHILE;
					__FALL_THROUGH__
				case O_CODE_OP_WHILE:
					if (!o_code_seek(o_code_stack[index].loop))
					{
						error = STATUS_OCODE_ERROR_FILE_NOT_FOUND;
						return error;
					}
					// re-eval the arg
					op_arg_error = parser_get_float(&op_arg);
					if (!op_arg)
					{
						// mark to exit loop
						o_code_stack[index].op = O_CODE_OP_WHILE_BREAK;
						o_code_discard();
					}
					error = STATUS_OK;
					return error;
				case O_CODE_OP_WHILE_BREAK:
					o_code_stack[index].code = 0;
					o_code_stack[index].op = 0;
					o_code_stack[index].pos = 0;
					o_code_stack[index].loop = 0;
					o_code_stack_index--;
					error = STATUS_OK;
					return error;
				default:
					error = STATUS_OCODE_ERROR_INVALID_NUMBER;
					return error;
				}
			}
			{
				error = STATUS_OCODE_ERROR_INVALID_NUMBER;
				return error;
			}

			if (o_code_file && o_code_stack[index].op != O_CODE_OP_WHILE_DISCARD)
			{
				// save position
				if (o_code_file)
				{
					loop_ret = o_code_file->file_info.size - fs_available(o_code_file); // backtrack 2 chars to catch the newline
				}
				// rewind
				if (!o_code_seek(o_code_stack[index].loop))
				{
					error = STATUS_OCODE_ERROR_FILE_NOT_FOUND;
					return error;
				}
				// re-eval the arg
				op_arg_error = parser_get_float(&op_arg);
				if (op_arg)
				{
					break;
				}
			}
			// restore position and continue
			if (!o_code_seek(loop_ret))
			{
				error = STATUS_OCODE_ERROR_FILE_NOT_FOUND;
				return error;
			}
			o_code_stack[index].code = 0;
			o_code_stack[index].op = 0;
			o_code_stack[index].pos = 0;
			o_code_stack[index].loop = 0;
			o_code_stack_index--;
			break;
		case 5:
			if (!index)
			{
				index = o_code_validate(O_CODE_OP_WHILE, ocode_id, true);
				if (!index)
				{
					error = STATUS_OCODE_ERROR_INVALID_NUMBER;
					return error;
				}
				// is a while loop
				index = o_code_stack_index++;
				o_code_stack[index].code = ocode_id;
				o_code_stack[index].pos = loop_ret; // o_code_file->file_info.size - fs_available(o_code_file);
				if (op_arg)
				{
					o_code_stack[index].op = O_CODE_OP_WHILE;
				}
				else
				{
					o_code_stack[index].op = O_CODE_OP_WHILE_BREAK;
					o_code_discard();
				}
			}
			else if (O_CODE_BASE_TYPE(o_code_stack[index].op) == O_CODE_OP_WHILE && ocode_id == o_code_stack[index].code)
			{
				// it's a do while
				switch (o_code_stack[index].op)
				{
				case O_CODE_OP_WHILE_DISCARD:
					// clear the discard and continue
					o_code_stack[index].op = O_CODE_OP_DO_WHILE;
					__FALL_THROUGH__
				case O_CODE_OP_DO_WHILE:
					if (op_arg)
					{
						// rewind
						if (!o_code_seek(o_code_stack[index].pos))
						{
							error = STATUS_OCODE_ERROR_FILE_NOT_FOUND;
							return error;
						}
						error = STATUS_OK;
						return error;
					}
					__FALL_THROUGH__
				case O_CODE_OP_WHILE_BREAK:
					o_code_stack[index].code = 0;
					o_code_stack[index].op = 0;
					o_code_stack[index].pos = 0;
					o_code_stack[index].loop = 0;
					o_code_stack_index--;
					error = STATUS_OK;
					return error;
				default:
					error = STATUS_OCODE_ERROR_INVALID_NUMBER;
					return error;
				}
			}
			else
			{
				// it's an invalid condition
				error = STATUS_OCODE_ERROR_INVALID_NUMBER;
				return error;
			}

			o_code_stack[index].loop = loop_ret; // point to the loop condition eval expression
			break;
		case 2:
			if (index)
			{
				error = STATUS_OCODE_ERROR_INVALID_NUMBER;
				return error;
			}
			// is a do while loop
			index = o_code_stack_index++;
			o_code_stack[index].code = ocode_id;
			o_code_stack[index].op = O_CODE_OP_DO_WHILE;
			o_code_stack[index].pos = loop_ret; // point to the start of the loop
			o_code_stack[index].loop = 0;
			break;
		}

		error = STATUS_OK;
		return error;
	}
	/**
	 * repeat
	 */
	if (!STRCMP(o_cmd, "REPEAT") || !STRCMP(o_cmd, "ENDREPEAT"))
	{
		uint8_t type = strlen(o_cmd);

		if (!o_code_validate(O_CODE_OP_REPEAT, ocode_id, (type == 9) ? false : true))
		{
			error = STATUS_OCODE_ERROR_INVALID_OPERATION;
			return error;
		}

		index--;
		switch (type)
		{
		case 6:
			index = o_code_stack_index++;
			o_code_stack[index].code = ocode_id;
			o_code_stack[index].pos = o_code_file->file_info.size - fs_available(o_code_file) + 1; // doesn't care about the re-eval de arg
			o_code_stack[index].loop = (int32_t)truncf(op_arg);
			break;
		}

		if (o_code_stack[index].loop)
		{
			o_code_stack[index].loop--;
			// clear possible discard
			o_code_stack[index].op = O_CODE_OP_REPEAT;
			// always jump to the loop start before eval
			// this makes the algorithm simpler
			if (!o_code_seek(o_code_stack[index].pos))
			{
				error = STATUS_OCODE_ERROR_FILE_NOT_FOUND;
				return error;
			}
		}
		else
		{
			o_code_stack[index].code = 0;
			o_code_stack[index].op = 0;
			o_code_stack[index].pos = 0;
			o_code_stack[index].loop = 0;
			o_code_stack_index--;
		}

		error = STATUS_OK;
		return error;
	}

	return error;
}
#endif

#ifdef ENABLE_NAMED_PARAMETERS
uint8_t parser_get_namedparam_id(float *value)
{
	char namedparam[NAMED_PARAM_MAX_LEN];
	bool valid = false;
	unsigned char c = parser_get_next_preprocessed(true);
	c = TOUPPER(c);
	if (c == '<')
	{
		parser_get_next_preprocessed(false);
		for (uint8_t i = 0; i < NAMED_PARAM_MAX_LEN; i++)
		{
			c = parser_get_next_preprocessed(true);
			if (c == EOL)
			{
				break;
			}
			if (c == '>')
			{
				parser_get_next_preprocessed(false);
				namedparam[i] = 0;
				valid = true;
				break;
			}
			namedparam[i] = parser_get_next_preprocessed(false);
		}

		if (valid)
		{
			for (uint8_t i = 0; i < sizeof(named_params) / sizeof(named_param_t); i++)
			{
				if (!rom_strcmp(namedparam, (const char *)rom_strptr(&(named_params[i].name))))
				{
					named_param_t p = {0};
					rom_memcpy(&p, &named_params[i], sizeof(named_param_t));
					*value = (float)p.id;
					return NUMBER_OK;
				}
			}
		}
	}

	return NUMBER_UNDEF;
}
#endif

#endif
