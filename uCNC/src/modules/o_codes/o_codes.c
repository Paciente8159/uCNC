#include "../../cnc.h"
#include "../file_system.h"
#include <stdint.h>
#include <stdbool.h>

#if defined(ENABLE_PARSER_MODULES) && defined(ENABLE_RS274NGC_EXPRESSIONS)

#if (UCNC_MODULE_VERSION < 11100 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#define OCODE_STACK_DEPTH 16
#define OCODE_DRIVE 'C'

#define FILE_EOF 3

#define OP_CALL 0
#define OP_SUB 1
#define OP_IF 2
#define OP_IF_FOUND 4
#define OP_DO_WHILE 8
#define OP_WHILE 16
#define OP_REPEAT 32

#define STATUS_OCODE_ERROR_FILE_NOT_FOUND 100
#define STATUS_OCODE_ERROR_INVALID_NUMBER 101
#define STATUS_OCODE_ERROR_INVALID_OPERATION 102
#define STATUS_OCODE_ERROR_INVALID_EXPRESSION 103
#define STATUS_OCODE_ERROR_STACK_UNDERFLOW 104
#define STATUS_OCODE_ERROR_STACK_OVERFLOW 105

#define STATUS_OCODE_ERROR_MIN STATUS_OCODE_ERROR_FILE_NOT_FOUND
#define STATUS_OCODE_ERROR_MAX STATUS_OCODE_ERROR_STACK_OVERFLOW

fs_file_t *sub_file;
uint32_t sub_pos;

typedef struct o_code_stack_
{
	uint16_t code;
	uint8_t op;
	uint32_t pos;
	float user_vars[RS274NGC_MAX_USER_VARS];
} o_code_stack_t;

o_code_stack_t o_codes_stack[OCODE_STACK_DEPTH];
uint8_t o_codes_stack_index;
bool o_code_call_has_return;
float o_code_call_return_value;

#define O_CODE_FILE_CLOSE 1
#define O_CODE_FILE_CLOSE_ALL 2
uint8_t o_code_file_finished;

static uint8_t o_code_getc(void)
{
	uint8_t c = 0;
	if (sub_file)
	{
		int avail = fs_available(sub_file);
		if (avail)
		{
			fs_read(sub_file, &c, 1);
		}
		else
		{
			return FILE_EOF; // end of file marker
		}
	}
	return c;
}

DECL_GRBL_STREAM(o_code_stream, o_code_getc, NULL, NULL, NULL, NULL);

// static void o_code_clear()
// {
// 	while (o_codes_call_index)
// 	{
// 		o_code_sub_t *stack = &o_codes_calls[o_codes_call_index - 1];
// 		if (stack->fp)
// 		{
// 			fs_close(stack->fp);
// 			o_codes_call_index--;
// 		}
// 	}

// 	if (o_codes_stack_index)
// 	{
// 		o_code_file_finished |= O_CODE_FILE_CLOSE_ALL;
// 	}
// 	o_codes_stack[0].code = 0;
// }

static void o_code_open(uint8_t index)
{
	if (o_codes_stack[index].op == OP_CALL)
	{
		if (sub_file)
		{
			fs_close(sub_file);
			sub_file = NULL;
		}

		char o_subrotine[32];
		memset(o_subrotine, 0, sizeof(o_subrotine));
		str_sprintf(o_subrotine, "/%c/o%d.gcode", OCODE_DRIVE, o_codes_stack[index].code);
		sub_file = fs_open(o_subrotine, "r");
		if (!sub_file)
		{
			return;
		}
		fs_seek(sub_file, sub_pos);
		o_codes_stack[index].op = OP_SUB;
		grbl_stream_change(&o_code_stream);
	}
}

static uint8_t o_code_close(uint8_t index)
{
	// close file
	if (index)
	{
		if (sub_file)
		{
			fs_close(sub_file);
			sub_file = NULL;
		}

		// find previous sub entry point
		while (o_codes_stack[index].op != OP_SUB && index)
		{
			o_codes_stack[index].code = 0;
			o_codes_stack[index].op = 0;
			index--;
		}

		// restore file pointer and mark entry for (re)call
		sub_pos = o_codes_stack[index].pos;
		o_codes_stack[index].op = OP_CALL;
		o_codes_stack_index = index;
	}

	return index;
}

bool o_codes_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	float op_arg = 0;
	uint8_t op_arg_error = NUMBER_UNDEF;
	char cmd[16];
	memset(cmd, 0, sizeof(cmd));

	uint8_t index = o_codes_stack_index;

	if (ptr->word == 'O')
	{
		uint16_t ocode_id = truncf(ptr->value);

		// in an if clause that failed to met the condition it must keep discarding code until a valid O code of the same ID is found
		if (index && (o_codes_stack[index - 1].op == OP_IF) && o_codes_stack[index - 1].code != ocode_id)
		{
			// keep discarding
			while (parser_get_next_preprocessed(true) != 'O')
			{
				parser_discard_command();
			}
			*ptr->error = STATUS_OK;
			goto o_code_return_label;
		}

		for (uint8_t i = 0; i < 16; i++)
		{
			char c = parser_get_next_preprocessed(true);
			c = TOUPPER(c);
			if (c < 'A' || c > 'Z')
			{
				break;
			}
			cmd[i] = c;
			parser_get_next_preprocessed(false);
		}

		op_arg_error = parser_get_float(&op_arg);

		if (!strcmp(cmd, "CALL"))
		{
			// store user vars
			for (uint16_t i = 0; i < RS274NGC_MAX_USER_VARS; i++)
			{
				o_codes_stack[index].user_vars[i] = ptr->new_state->user_vars[i];
			}
			
			// check if file exists
			char o_subrotine[32];
			memset(o_subrotine, 0, sizeof(o_subrotine));
			str_sprintf(o_subrotine, "/%c/o%d.gcode", OCODE_DRIVE, ocode_id);
			fs_file_info_t finfo;
			fs_finfo(o_subrotine, &finfo);
			if (!finfo.size)
			{
				*ptr->error = STATUS_OCODE_ERROR_INVALID_OPERATION;
				goto o_code_return_label;
			}

			// call parameters
			uint16_t arg_i = 0;
			while (op_arg_error == NUMBER_OK)
			{
				// load args
				ptr->new_state->user_vars[arg_i++] = op_arg;
				op_arg_error = parser_get_float(&op_arg);
			}
		
			// store operation in the stack
			o_codes_stack[index].code = ocode_id;
			o_codes_stack[index].op = OP_CALL;
			// workaround to ftell
			o_codes_stack[index].pos = (sub_file) ? (sub_file->file_info.size - fs_available(sub_file)) : 0;
			
			o_codes_stack_index++;
			*ptr->error = STATUS_OK;
			goto o_code_return_label;
		}

		if (!strcmp(cmd, "IF") || !strcmp(cmd, "ELSEIF") || !strcmp(cmd, "ELSE"))
		{
			uint8_t type = strlen(cmd); // If=2 ElseIf=6 Else=4
			*ptr->error = STATUS_OK;
			switch (type)
			{
			case 2:
			case 6:
				if (op_arg_error != NUMBER_OK)
				{
					*(ptr->error) = STATUS_OCODE_ERROR_INVALID_OPERATION;
					goto o_code_return_label;
				}
				break;
			}

			switch (type)
			{
			case 2:
				for (uint8_t codes = index - 1; codes != 0; codes--)
				{
					if (o_codes_stack[codes].code == ocode_id)
					{
						*(ptr->error) = STATUS_OCODE_ERROR_INVALID_NUMBER;
						goto o_code_return_label;
					}
					if (o_codes_stack[codes].op == OP_SUB)
					{
						break;
					}
				}
				break;
			case 4:
			case 6:
				if (index--)
				{
					if (o_codes_stack[index].code != ocode_id || !(o_codes_stack[index].op & OP_IF))
					{
						*(ptr->error) = STATUS_OCODE_ERROR_INVALID_OPERATION;
						goto o_code_return_label;
					}
				}
				break;
			}

			switch (type)
			{
			case 2:
				o_codes_stack[index].code = ocode_id;
				o_codes_stack[index].op = OP_IF;
				o_codes_stack_index++;
				__FALL_THROUGH__
			case 6:
				// discard code if eval is false until a new O code is found
				if (!op_arg)
				{
					while (parser_get_next_preprocessed(true) != 'O')
					{
						parser_discard_command();
					}
					break;
				}
				__FALL_THROUGH__
			case 4:
				if (!(o_codes_stack[index].op & OP_IF_FOUND)) // condition not yet met
				{
					o_codes_stack[index].op |= OP_IF_FOUND; // signals that the condition was met
				}
				else{
					while (parser_get_next_preprocessed(true) != 'O')
					{
						parser_discard_command();
					}
				}
				break;
			}

			*(ptr->error) = STATUS_OK;
			goto o_code_return_label;
		}
		if (!strcmp(cmd, "ENDIF"))
		{
			if (index--)
			{
				if (o_codes_stack[index].code != ocode_id || !(o_codes_stack[index].op & OP_IF))
				{
					*(ptr->error) = STATUS_OCODE_ERROR_INVALID_OPERATION;
					goto o_code_return_label;
				}

				o_codes_stack[index].code = 0;
				o_codes_stack[index].op = 0;
				o_codes_stack_index--;
				*ptr->error = STATUS_OK;
			}
			else
			{
				*(ptr->error) = STATUS_OCODE_ERROR_STACK_UNDERFLOW;
			}

			goto o_code_return_label;
		}
		if (!strcmp(cmd, "DO"))
		{
			o_codes_stack[index].code = ocode_id;
			o_codes_stack[index].op = OP_DO_WHILE;
			o_codes_stack_index++;
		}
		if (!strcmp(cmd, "WHILE"))
		{
			if (index)
			{
				if (o_codes_stack[index - 1].code == ocode_id || !(o_codes_stack[index].op & OP_IF))
				{
					*(ptr->error) = STATUS_OCODE_ERROR_INVALID_OPERATION;
					goto o_code_return_label;
				}

				o_codes_stack_index--;
				goto o_code_return_label;
			}

			goto o_code_return_label;
		}
		if (!strcmp(cmd, "ENDWHILE"))
		{
		}
		if (!strcmp(cmd, "CONTINUE"))
		{
		}
		if (!strcmp(cmd, "REPEAT"))
		{
		}
		if (!strcmp(cmd, "ENDREPEAT"))
		{
		}
	}

	if (ptr->word == FILE_EOF || !strcmp(cmd, "RETURN"))
	{
		o_code_call_has_return = (op_arg_error == NUMBER_OK);
		o_code_call_return_value = op_arg;

		if (index)
		{
			index = o_code_close(index);
			// restore user vars
			for (uint16_t i = 0; i < RS274NGC_MAX_USER_VARS; i++)
			{
				ptr->new_state->user_vars[i] = o_codes_stack[index].user_vars[i];
			}
		}

		if (index)
		{
			// top sub not exited yet
			o_code_open(index);
		}

		*(ptr->error) = STATUS_OK;
		goto o_code_return_label;
	}
	return EVENT_CONTINUE;

o_code_return_label:
	if (*(ptr->error) >= STATUS_OCODE_ERROR_MIN && *(ptr->error) <= STATUS_OCODE_ERROR_MAX)
	{
		// Error in o-code, do not continue execution and empty the stack.
		//		o_code_clear();
		grbl_stream_change(NULL);
	}
	return EVENT_HANDLED;
}

CREATE_EVENT_LISTENER(gcode_parse, o_codes_parse);

bool o_codes_call(void *args)
{
	uint8_t index = o_codes_stack_index;
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (index)
	{
		index--;
		if (o_codes_stack[index].op == OP_CALL)
		{
			sub_pos = 0;
			o_code_open(index);
		}
		if (!sub_file)
		{
			*ptr->error = STATUS_OCODE_ERROR_INVALID_OPERATION;
			return EVENT_CONTINUE;
		}
	}

	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(gcode_exec_modifier, o_codes_call);

bool o_codes_eof_token(void *args)
{
	uint8_t *word = (uint8_t *)args;

	if (*word == FILE_EOF)
	{
		return EVENT_HANDLED;
	}
	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(parse_token, o_codes_eof_token);

DECL_MODULE(o_codes)
{
	o_codes_stack_index = 0;
	ADD_EVENT_LISTENER(gcode_parse, o_codes_parse);
	ADD_EVENT_LISTENER(gcode_exec_modifier, o_codes_call);
	ADD_EVENT_LISTENER(parse_token, o_codes_eof_token);
}

#endif
