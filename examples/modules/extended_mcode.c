/*
	This is and example of how to add a custom MCode extension to µCNC
	Extension GCode/MCodes can be added simply by adding 2 event listeners to the core parser

	GCode/MCode execution is done in 3 steps:
	  - Parsing - the code is read from the stream and it's parsed generating a new machine state
	  - Validation - the new machine is validated against some NIST/RS274 rules for incompatibilities
	  - Execution - The new state is executed and the active machine state is updated

	Every unrecognized GCode/MCode can be intercepted before being discarded as an invalid command
	There are 2 events that can be hooked to do this. One at the parsing stage and other at the execution stage.
	Since there is no event for validation, this step should be done inside one of the other (parsing or execution) events.

*/

#include "../../cnc.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief	GCode/MCode extensions depend on the ENABLE_PARSER_MODULES option
 * 			Check if this option is defined or not
 */
#ifdef ENABLE_PARSER_MODULES

/**
 * @brief	Check if your current module is up to date with the current core version of module
 */
#if (UCNC_MODULE_VERSION < 10800 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

/**
 * @brief	Choose and ID from 1 to 999 for you custom G or M code
 * 			This ID must be unique for each code
 * 			Use the EXTENDED_GCODE or EXTENDED_MCODE to define the correct ID code to be used by the extended code group inside the machine state parser
 */
#define M999 EXTENDED_MCODE(999)

/**
 * @brief Create a function to parse your custom MCode. All event functions are declared as uint8_t <function>(void* args, bool* handle)
 *
 * @param args		is a pointer to a set of arguments to be passed to the event handler. In the case of the gcode_parse event it's a struct of type gcode_parse_args_t define the following way
 * 					typedef struct gcode_parse_args_
 *					{
 *						unsigned char word;			// Type of command word ('G' or 'M' usually)
 *						uint8_t code;				// Code of the command this is the code of the command. Beware that this is limited from 0 to 255. For G/M code with higher values or subcodes  (ex: 97.4) this value is ignored argument 'value' of the struct
 *						uint8_t *error;				// The current error code of the parsing stage
 *						float value;				// The value of the code. If the intercepted word command was M97.4, value is 97.4
 *						parser_state_t *new_state;	// The new parser state of the machine
 *						parser_words_t *words;		// The struct with the valid words values
 *						parser_cmd_explicit_t *cmd;	// The struct with command groups/words flags and exetended commands code
 *					} gcode_parse_args_t;
 * @return bool 	a boolean that tells the handler if the event should continue to propagate through additional listeners or is handled by the current listener an should stop propagation
 */
bool mycustom_parser(void *args)
{
	// this is just to cast the void* args to the used struct by the parse event
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	// if the word command M99 (word-M value-999) then it's our code
	if (ptr->word == 'M' && ptr->value == 999)
	{
		// you should always check if there is already another extended command active in this line of GCODE. µCNC should only execute one extended command by GCODE line.
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			// this modified the error code
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			// because this MCode is our desired code we can prevent further parsing event propagation to other listeners
			// this avoids errors due to modifications of the return error code and speeds up execution
			return EVENT_HANDLED;
		}

		// tells the gcode validation and execution functions this is custom code M999 (ID must be unique)
		ptr->cmd->group_extended = M999;
		// return status code error OK
		*(ptr->error) = STATUS_OK;
		// again prevents event propagation to speed up the code since this callback handled the event
		return EVENT_HANDLED;
	}

	// if this is not catched by this parser, just send back the error so other modules can process it
	return EVENT_CONTINUE;
}

/**
 * @brief 	Create an event listener object an attach our custom code parser handler.
 * 			in this case we are adding a listener to the 'gcode_parse' EVENT
 *
 */
CREATE_EVENT_LISTENER(gcode_parse, mycustom_parser);

/**
 * @brief Create a function to execute your custom MCode. Again all event functions are declared as uint8_t <function>(void* args, bool* handle)
 *
 * @param args		is a pointer to a set of arguments to be passed to the event handler. In the case of the gcode_exec event it's a struct of type gcode_exec_args_t define the following way
 * 					typedef struct gcode_exec_args_
 *					{
						uint8_t* error;				// the current error code status
						parser_state_t *new_state;  // The new parser state of the machine
						parser_words_t *words;      // The struct with the valid words values
						parser_cmd_explicit_t *cmd; // The struct with command groups/words flags and exetended commands code
						float *target;				// the new target position after excuting this code
						motion_data_t *block_data;  // the block data to be sent to the motion control
 *					} gcode_exec_args_t;
 *  * @return bool 	a boolean that tells the handler if the event should continue to propagate through additional listeners or is handled by the current listener an should stop propagation
 */
bool mycustom_execution(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	// checks if the extended command that is being executed is our custom M999 command
	if (ptr->cmd->group_extended == M999)
	{
		// because this MCode is our desired code we can prevent further parsing event propagation to other listeners
		// this avoids errors due to modifications of the return error code and speeds up execution
		// return status code error OK
		*(ptr->error) = STATUS_OK;
		// again prevents event propagation to speed up the code since this callback handled the event
		return EVENT_HANDLED;
	}

	// if this is not catched by this exec callback, just send back so other modules can process it
	return EVENT_CONTINUE;
}

/**
 * @brief 	Create an event listener object an attach our custom code executer handler.
 * 			in this case we are adding a listener to the 'gcode_exec' EVENT
 *
 */
CREATE_EVENT_LISTENER(gcode_exec, mycustom_execution);

#endif

/**
 * @brief 	Declarates a new module and adds the event listeners.
 * 			Again this should check the if the appropriate module option is enabled
 * 			To add this module you just neet to call LOAD_MODULE(mycustom_parse_module); from inside the core code
 */
DECL_MODULE(mycustom_parse_module)
{
#ifdef ENABLE_PARSER_MODULES
	// Makes the event handler 'mycustom_parser' listen to the event 'gcode_parse'
	ADD_EVENT_LISTENER(gcode_parse, mycustom_parser);
	// Makes the event handler 'mycustom_execution' listen to the event 'gcode_exec'
	ADD_EVENT_LISTENER(gcode_exec, mycustom_execution);
#else
// just a warning in case you disabled the PARSER_MODULES option on build
#warning "Parser extensions are not enabled. M999 code extension will not work."
#endif
}
