/*
	This is and example of how to add a custom $ setting extension to µCNC

	All extended settings minimum address is 256 and maximum is 65535

	There is a new simplified way of creating extensions
	Please check the end of this file

*/

#include "../../cnc.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_SETTINGS_MODULES

#if (UCNC_MODULE_VERSION < 10800 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

// dummy setting $1000
#define DUMMY_SETTING_ID 1000
// this will keep the offset address of the stored settings
static uint32_t dummy_settings_address;

// dummy setting
// can be anything float, struct, etc...
static uint32_t dummy_setting;

bool extend_settings_load(void *args)
{
	// load setting from NVM

	// can be done by simply read from NVM
	settings_load(dummy_settings_address, &dummy_setting, sizeof(dummy_setting));

	// or in alternative use any custom method for loading the setting
	// if a read error occurs it can be represent signaled to force the user to reconfigure all the machine settings
	// the settings error global variable must be signaled like this:
	
	// g_settings_error |= SETTINGS_READ_ERROR;

	return EVENT_CONTINUE;
}

bool extend_settings_save(void *args)
{
	// save setting to NVM
	// can be done by simply write to NVM
	settings_save(dummy_settings_address, &dummy_setting, sizeof(dummy_setting));

	// to signal a write error you can set the settings error global variable
	// the settings error global variable must be signaled like this:
	
	// g_settings_error |= SETTINGS_WRITE_ERROR;

	// allow other settings to be stored in eeprom
	return EVENT_CONTINUE;
}

bool extend_settings_change(void *args)
{
	setting_args_t *set = (setting_args_t *)args;
	if (set->id == DUMMY_SETTING_ID)
	{
		// if matches setting id intercepts the call
		// load from disk
		dummy_setting = (uint32_t)set->value;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

bool extend_settings_erase(void *args)
{
	settings_args_t *set = (settings_args_t *)args;
	// if reseting main settings load this one too
	if (set->address == SETTINGS_ADDRESS_OFFSET)
	{
		// reset
		dummy_setting = 0;
	}

	// prevent propagation
	// to enable event propagation return EVENT_CONTINUE
	return EVENT_HANDLED;
}

uint8_t extend_protocol_send_cnc_settings(void *args)
{
	proto_gcode_setting_line_int(DUMMY_SETTING_ID, dummy_setting);
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(settings_extended_load, extend_settings_load);
CREATE_EVENT_LISTENER(settings_extended_save, extend_settings_save);
CREATE_EVENT_LISTENER(settings_extended_change, extend_settings_change);
CREATE_EVENT_LISTENER(settings_extended_erase, extend_settings_erase);
CREATE_EVENT_LISTENER(proto_cnc_settings, extend_protocol_send_cnc_settings);

#endif

DECL_MODULE(dummy)
{
#ifdef ENABLE_SETTINGS_MODULES
	dummy_settings_address = settings_register_external_setting(sizeof(dummy_setting));
	ADD_EVENT_LISTENER(settings_load, extend_settings_load);
	ADD_EVENT_LISTENER(settings_save, extend_settings_save);
	ADD_EVENT_LISTENER(settings_extended_change, extend_settings_change);
	ADD_EVENT_LISTENER(settings_extended_erase, extend_settings_erase);
	ADD_EVENT_LISTENER(proto_cnc_settings, extend_protocol_send_cnc_settings);
	// since settings are loaded before the modules are initialized, it's recommended to manually trigger the load event since this will not fire again naturally
	extend_settings_load(NULL);
#else
// just a warning in case you disabled the MAIN_LOOP option on build
#warning "Settings extensions are not enabled. Your module will not work."
#endif
}

/**
 *
 * ALTERNATIVE SIMPLIFIED WAY WITH NEW SETTINGS MACROS
 *
 * The example below does the same thing as the code above
 *
 * **/

// dummy setting $1000
#define DUMMY_SETTING_ID 1000
// this will keep the offset address of the stored settings
static uint32_t dummy_settings_address;

// declarate the setting with
//  args are:
//  ID - setting ID
//  var - pointer to var
//  type - type of var
//  count - var array length (if single var set to 1)
//  print_cb - protocol setting print callback

DECL_EXTENDED_SETTING(DUMMY_SETTING_ID, &dummy_settings_address, uint32_t, 1, proto_gcode_setting_line_int);

// then just call the initializator from any point in the code
// the initialization will run only once
// ID - setting ID
// var - var as argument (not the pointer)
void some_function_running_in_the_code_loop()
{
	EXTENDED_SETTING_INIT(DUMMY_SETTING_ID, dummy_settings_address);
}
