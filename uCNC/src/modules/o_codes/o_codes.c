#include "../../cnc.h"
#include "../file_system.h"
#include <stdint.h>
#include <stdbool.h>

#if defined(ENABLE_PARSER_MODULES) && defined(ENABLE_RS274NGC_EXPRESSIONS)

#if (UCNC_MODULE_VERSION < 11100 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#define OCODE_STACK_DEPTH 16

typedef struct o_code_sub_
{
	fs_file_t *fp;
	uint32_t pos;
} o_code_sub_t;

typedef struct o_code_stack_
{
	uint16_t code;
	uint8_t op;
	float user_vars[RS274NGC_MAX_USER_VARS];
} o_code_stack_t;

o_code_stack_t o_codes_stack[OCODE_STACK_DEPTH];
uint8_t o_codes_stack_index;

static uint8_t o_code_getc(void)
{
	uint8_t c = 0;
	if (fs_running_file)
	{
		int avail = fs_available(fs_running_file);
		if (avail)
		{
			fs_read(fs_running_file, &c, 1);
			avail--;
		}
		// auto close file
		if (!avail)
		{
			fs_close(fs_running_file);
			fs_running_file = NULL;
		}
	}
	return c;
}

static uint8_t o_code_available()
{
	uint8_t avail = 0;
	if (fs_running_file)
	{
		avail = (uint8_t)MIN(255, fs_available(fs_running_file));
	}
	return avail;
}

static void o_code_clear()
{
	if (fs_running_file)
	{
		fs_close(fs_running_file);
	}
}

DECL_GRBL_STREAM(o_code_stream, o_code_getc, o_code_available, o_code_clear, NULL, NULL);

bool o_codes_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'O')
	{
		uint16_t ocode_id = truncf(ptr->value);
		char cmd[16];
		memset(cmd, 0, sizeof(cmd));
		for (uint8_t i = 0; i < 16; i++)
		{
			char c = parser_get_next_preprocessed(true);
			c = TOUPPER(c);
			if (c < 'A' || c > 'Z' || c == ' ')
			{
				break;
			}
			cmd[i] = c;
			parser_get_next_preprocessed(false);
		}

		uint8_t index = o_codes_stack_index;

		if (!strcmp(cmd, "CALL"))
		{
			if (index)
			{
				index++;
			}
			o_codes_stack[index].code = ocode_id;
			parser_copy_user_vars(o_codes_stack[index].user_vars, MAX(sizeof(o_codes_stack[index].user_vars), (RS274NGC_MAX_USER_VARS * sizeof(float))));
			// TODO
			// open file based on ocode number and default drive
			// reset file position
			// create readonlystream
			index++;
		}
		if (!strcmp(cmd, "IF"))
		{
			// TODO
			// parser_get_float and evaluate value
			// run if not 0 then discard all lines until endif is found
			// or discard all lines until next ocode
		}
		if (!strcmp(cmd, "ELSEIF"))
		{
		}
		if (!strcmp(cmd, "ELSE"))
		{
		}
		if (!strcmp(cmd, "RETURN"))
		{
		}
		if (!strcmp(cmd, "WHILE"))
		{
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

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(gcode_parse, o_codes_parse);

DECL_MODULE(o_codes)
{
	o_codes_stack_index = 0;
	ADD_EVENT_LISTENER(gcode_parse, o_codes_parse);
}

#endif