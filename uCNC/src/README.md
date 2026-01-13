<p align="center">
<img src="https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true">
</p>


# µCNC
µCNC - Universal CNC firmware for microcontrollers

## module.c and module.h
These files contain initialization code for all modules that extend µCNC functionalities, like custom modules.

_**Jump to section**_
* [Adding custom modules to µCNC](#adding-custom-modules-to-µcnc)
   * [µCNC existing events/delegates](#µcnc-existing-eventsdelegates)
   * [µCNC existing hooks](#µcnc-existing-hooks)
   * [modules.h and events](#modulesh-and-events)
   * [Creating a new custom event listener](#creating-a-new-custom-event-listener)
   * [Creating a new custom event](#creating-a-new-custom-event)

# Adding custom modules to µCNC

__IMPORTANT NOTE__: _Version 1.7 implemented breaking changes to modules declarations. Please check the modules releases to get the right modules for your version [repository](https://github.com/Paciente8159/uCNC-modules)_

__NOTE__: _Version 1.4.6 implemented changes to module initialization. Also additional modules were moved to a new [repository](https://github.com/Paciente8159/uCNC-modules)_

µCNC has implemented a module system that allows the user to perform custom actions that get executed in an event/delegate fashion style similar to what is done with C#. Multiple callbacks functions can be attached to the same event.
These modules can be quite useful and perform several things like adding custom custom gcodes to perform actions, or modifying IO states if a given condition is verified.
µCNC already has a few useful modules like PID controller, Encoder module, TMC drivers support and custom G/M code support.
Version 1.8 also introduces the concept of simple hooks. These hooks are simple function pointers that execute the assigned callback at that time. They are usually used inside interrupt service routines and provide a way to extend ISR's with small extra code blocks to perform certain tasks. Unlike events, simple hooks can only run a single callback. The active callback is the last one to attach to any give hook.

## µCNC existing events/delegates

Without having to modify core code inside µCNC it is possible to listen to several already existing events. Here is a list of current events:

__NOTE__: Not all event hooks might be listed here. To find all available event hooks declarations, do a search on all files (on VSCode in Windows it's Ctrl+Shift+F) of the project of `DECL_EVENT_HANDLER`. You can also search for the `EVENT_INVOKE` to see what argument is being passed to the event handler.

| Event name | Argument | Enable option | Description |
| --- | --- | --- | --- |
| gcode_parse | gcode_parse_args_t* | ENABLE_PARSER_MODULES | Fires when a gcode command was not captured/understood by the core parser is trying to parse the code. Arg is a pointer to a gcode_parse_args_t struct |
| gcode_exec | gcode_exec_args_t* | ENABLE_PARSER_MODULES | Fires when a gcode command that was not captured/understood by the core parser is intercepted by gcode_parse event an recognized as an extended gcode command. Arg is a pointer to a gcode_exec_args_t struct |
| gcode_exec_modifier | gcode_exec_args_t* | ENABLE_PARSER_MODULES | Fires prior to gcode execution phase and can be used to modify the parsed command or execute custom actions or issue motions before gcode execution. Arg is a pointer to a gcode_exec_args_t struct |
| gcode_before_motion | gcode_exec_args_t* | ENABLE_PARSER_MODULES | Fires before a motion group command is executed (G0, G1, G2, etc...). Arg is a pointer to a gcode_exec_args_t struct |
| gcode_after_motion | gcode_exec_args_t* | ENABLE_PARSER_MODULES | Fires after a motion group command is executed (G0, G1, G2, etc...). Arg is a pointer to a gcode_exec_args_t struct |
| grbl_cmd | grbl_cmd_args_t* | ENABLE_PARSER_MODULES | Fires when a custom/unknown '$' grbl type command is received. Arg is a pointer to a grbl_cmd_args_t struct |
| parse_token | NULL | ENABLE_PARSER_MODULES | Fires when a custom/unknown token/uint8_t is received for further processing |
| parser_get_modes | uint8_t * | ENABLE_PARSER_MODULES | Fires when $G command is issued and the active modal states array is being requested (can be used to modify the active modes with extended gcodes). Arg is a pointer to an uint8_t array with the parser motion groups current values |
| parser_reset | NULL | ENABLE_PARSER_MODULES | Fires on parser reset |
| cnc_reset | NULL | ENABLE_MAIN_LOOP_MODULES | Fires when µCNC resets |
| cnc_dotasks | NULL | ENABLE_MAIN_LOOP_MODULES | Fires on the main loop running. Any repeating task should be hooked here |
| cnc_io_dotasks | NULL | ENABLE_MAIN_LOOP_MODULES | Fires on the main IO loop running. This is similar to cnc_dotasks, the main difference is that this task will run also during delays |
| cnc_stop | NULL | ENABLE_MAIN_LOOP_MODULES | Fires when a halt/stop condition is triggered |
| cnc_parse_cmd_error | NULL | ENABLE_MAIN_LOOP_MODULES | Fires when an invalid command is received |
| cnc_alarm | NULL | ENABLE_MAIN_LOOP_MODULES | Fires when an alarm is triggered |
| settings_extended_change | setting_args_t* | ENABLE_SETTINGS_MODULES | Fires when a $ setting is changed. Arg is a pointer to a setting_args_t struct identifying the changed setting id and value |
| settings_extended_load | NULL | ENABLE_SETTINGS_MODULES | Fires when the base settings ($) are loaded from memory. Arg is a pointer to a settings_args_t struct |
| settings_extended_save | NULL | ENABLE_SETTINGS_MODULES | Fires when the base settings ($) are saved into memory. Arg is a pointer to a settings_args_t struct |
| settings_extended_erase | NULL | ENABLE_SETTINGS_MODULES | Fires when the base settings are erased/reset ($). This will only be triggered when the base address of the settings is targeted |
| proto_status | NULL | - | Fires when printing the status message |
| proto_cnc_settings | NULL | ENABLE_SETTINGS_MODULES | Fires when printing settings values |
| proto_cnc_info | NULL | ENABLE_SYSTEM_INFO | Fires when printing response to $I command |
| proto_pins_states | NULL | ENABLE_IO_MODULES | Fires when $P command is printing the pins states |
| proto_gcode_modes | NULL | ENABLE_PARSER_MODULES | Fires when $G printing modals states message |
| input_change | uint8_t* | ENABLE_IO_MODULES | Fires when a generic input (DIN0-7) pin changes state. Args is an uint8_t array with 2 values. The first is the inputs current values mask and the second a maks with the inputs that changed since the last time |
| probe_enable | NULL | ENABLE_IO_MODULES | Fires when the probe is enabled |
| probe_disable | NULL | ENABLE_IO_MODULES | Fires when the probe is disabled |
| mc_home_axis_start | homing_status_t* | ENABLE_MOTION_CONTROL_MODULES | Fires once per axis when homing motion is starting. Pointer to homing_status_t struct with homming information |
| mc_home_axis_finish | homing_status_t* | ENABLE_MOTION_CONTROL_MODULES | Fires once per axis when homing motion is finnished |
| mc_line_segment | motion_data_t* | ENABLE_MOTION_CONTROL_MODULES | Fires when a line segment is about to be sent from the motion control to the planner. Arg is a pointer to a motion_data_t struct with the current motion block data |

Each of these events exposes a delegate and and event that have the following naming convention:

`<event_name>_delegate` -> type of a function pointer that defines the function prototype to be called by the event handler 
`<event_name>_event` -> pointer to the event that contains all the subscribers that are going to be called by the event handler
`event_<event_name>_handler` -> the event handler that is called inside the core code and calls and executes every subscriber callback

For example `cnc_dotasks` exposes `cnc_dotasks_delegate` and `cnc_dotasks_event` that will enable to create an attach a listener that gets called when the event executed inside the core code. This is done by the call to `event_<event_name>_handler` inside `cnc_dotasks` main loop function.

All delegates are of type `bool (*delegate_function_pointer) (void* args)`. All functions should reply with a `EVENT_CONTINUE`(false) or `EVENT_HANDLED`(true) to the handler. By default the event handler propagates the event to all the registered listeners until one of them replies with `EVENT_HANDLED`, at which point the handler prevents further propagation and resumes execution.

In most events the input argument is `NULL`. The exceptions are:

#### gcode_parse

**Input args:**

gcode_parse_args_t* - Pointer to a struct of type gcode_parse_args_t. The gcode_parse_args_t struct is defined like this:

```
typedef struct gcode_parse_args_
{
	uint8_t word; // This is the GCode word being parsed (usually 'G' or 'M')
	uint8_t code; // This is the GCode word argument converted to uint8_t format (example 98)
	uint8_t error; // The parser current error code (STATUS_GCODE_EXTENDED_UNSUPPORTED)
	float value; // This is the actual GCode word argument parsed as float. Useful if code has mantissa or is bigger then 255 (example 98.1)
	parser_state_t *new_state; // The current parser state (struct)
	parser_words_t *words; // The current parser words argument values (struct)
	parser_cmd_explicit_t *cmd; // The current parser GCode command active words/groups
} gcode_parse_args_t;
```

#### gcode_exec, gcode_exec_modifier, gcode_before_motion and gcode_after_motion

**Input args:**

gcode_exec_args_t* - Pointer to a struct of type gcode_exec_args_t. The gcode_exec_args_t struct is defined like this:

```
typedef struct gcode_exec_args_
{
	parser_state_t *new_state; // The current parser state (struct)
	parser_words_t *words; // The current parser words argument values (struct)
	parser_cmd_explicit_t *cmd; // The current parser GCode command active words/groups
} gcode_exec_args_t;
```

#### grbl_cmd

**Input args:**

grbl_cmd_args_t* - Pointer to a struct of type grbl_cmd_args_t. The grbl_cmd_args_t struct is defined like this:

```
typedef struct grbl_cmd_args_
{
	uint8_t *error; // current parser error state
	uint8_t *cmd; // pointer to the command string
	uint8_t len; // command string length
	uint8_t next_char; // next uint8_t to be read
} grbl_cmd_args_t;
```

#### parser_get_modes

**Input args:**

uint8_t* - An array with the parser current active modal states

#### input_change

**Input args:**

uint8_t* - An array with the DIN7- states. `array[0]` contains the mask value of pins DIN7(MSB) to DIN0(LSB). `array[1]` contains the mask value of all pins DIN7(MSB) to DIN0(LSB) that changed state since the last time.

#### mc_home_axis_start and mc_home_axis_finnish

**Input args:**

homing_status_t* - Pointer to a struct of type homing_status_t. The homing_status_t struct is defined like this:

```
typedef struct homing_status_ {
	uint8_t axis; // current axis homming (mask)
	uint8_t axis_limit; // current axis homming limit switch enabled (mask)
	uint8_t status; // status of the motion (defaults to STATUS_OK)
} homing_status_t;
```

#### mc_line_segment

**Input args:**

motion_data_t* - Pointer to a struct of type motion_data_t. The motion_data_t struct is defined like this:

```
typedef struct
{
#ifdef GCODE_PROCESS_LINE_NUMBERS
	uint32_t line; // gcode line number
#endif
	step_t steps[STEPPER_COUNT]; // step count for each linear actuator
	uint8_t dirbits; // step dir mask
#ifdef ENABLE_LINACT_PLANNER
	uint32_t full_steps; // number of steps of all linear actuators
#endif
	float feed; // programmed feedrate in steps per second
	float max_feed; // rapid motion feedrate in steps per second
	float max_accel; // maximum acceleration in steps per second^2
	float feed_conversion; // step to units convertion constant
	float cos_theta; // angle between current and previous motion
	uint8_t main_stepper; // dominant step linear actuator for this motion
	uint16_t spindle; // spindle speed and direction
	uint16_t dwell; // dwell in milliseconds
	uint8_t motion_mode; // motion mode mask
	motion_flags_t motion_flags; // motion type flags
} motion_data_t;
```

## µCNC existing hooks

These are the list of available hooks inside 

__NOTE__: Not all simple hooks may be listed here. To find all available simple hooks declarations, do a search on all files (on VSCode in Windows it's Ctrl+Shift+F) of the project of `DECL_HOOK`. You can also search for the `HOOK_INVOKE` to see what argument is being passed to the simple hook handler.

| Event name | Argument | Enable option | Description |
| --- | --- | --- | --- |
| itp_rt_pre_stepbits | int*, int* | ENABLE_RT_SYNC_MOTIONS | Fires when the next computed step bits and dirs have been computed to be output. Args are a pointer the stepbit var and a pointer to a dirbit var |
| itp_rt_stepbits | int, int | ENABLE_RT_SYNC_MOTIONS | Fires when the setpbits have been output to the IO. Args are the stepbit mask value and the step ISR flags value |
| encoder_index | void | ENCODER_COUNT | Fires when the index of the specialized rpm encoder is triggered. Has no args |

## modules.h and events 

`src/module.h` exposes a few handy macros that make event listeners, or custom events creations easy.

### event creation

To create custom event handlers these macros are available:

`DECL_EVENT_HANDLER(name)`

`WEAK_EVENT_HANDLER(name)`

`DEFAULT_EVENT_HANDLER(name)`

To create custom event listeners these macros are available:

`CREATE_EVENT_LISTENER(name, handler)`

`ADD_EVENT_LISTENER(name, handler)`

There are also a few other usefull macros that make module initialization and declaration easier. These are:

`DECL_MODULE(name)`

`LOAD_MODULE(name)`

As mentioned all events run a default calback handler that runs through all the listeners or shortens the execution if one of the listeners responds with a `EVENT_HANDLED`.

This behaviour can be modified with a custom callback function that can be declared using:

```
OVERRIDE_EVENT_HANDLER(name){
	// your custom code here
}
```
This is usefull if you want to to make a listener the only one that is able to listen to an event or bypass the event handler processing with some custom code.

## Creating a new custom event listener

Creating a new event listener is easy. All current modules are placed inside the `src/modules` directory but it's not mandatory. It's just a matter of having them organized.
If you place them in other places the only difference is the `#includes` directives should be updated since they use relative paths.

Also new modules should be initialized inside the `load_modules` function in module.c. This is not mandatory. It's a matter of keeping code organized and insure that the function calls are done in the correct order. Due to Arduino IDE limitations (not supporting folders) in the way you can access and modify source that is not in the same directory as the .ino file, different ways of including custom code can be adopted to make it work.

Let's look at an example in the Arduino IDE environment by creating and attaching and event listener to `cnc_dotasks`.

Add a new file .c to uCNC directory (same has uCNC.ino) and paste this code

```
// include cnc.h relative path
#include "src/cnc.h"

// preprocessor check if module is enabled
#ifdef ENABLE_MAIN_LOOP_MODULES

// custom code listener callback implementation
// all callbacks must be of type bool (*function_pointer) (void*)
bool my_custom_code(void* args)
{
    // do something

	// in this case we will return EVENT_CONTINUE to allow event to propagate to other listeners
	return EVENT_CONTINUE;
}

// create a listener of type cnc_dotasks
CREATE_EVENT_LISTENER(cnc_dotasks, my_custom_code);
#endif

// you only need to add the event listener to the event handler
// you can either call the ADD_EVENT_LISTENER inside the main µCNC initialization code 
// or you can create a module initialization function and call it inside the main µCNC initialization code. The advantage of the latest is that you can have multiple event listeners you want to attach to multiple events and perform other loading actions and that way, you perform this in one call.
// Here is an example

DECL_MODULE(my_custom_module){
#ifdef ENABLE_MAIN_LOOP_MODULES
	// attach the listener
	ADD_EVENT_LISTENER(cnc_dotasks, my_custom_code);
#endif
}
```

This created the event listener that is of type `cnc_dotasks`. The only thing left to do is to attach it so that it gets called when `cnc_dotasks` is fired.

In the uCNC.ino it's just a matter of adding the listener to the event like this:

Open uCNC.ino

```
#include "src/cnc.h"

int main(void)
{
    //initializes all systems
    cnc_init();

    // add the listener to cnc_dotasks
    // ADD_EVENT_LISTENER(cnc_dotasks, my_custom_code);

	// or instead of calling the listener you can call the module initializer that does that task like this
	LOAD_MODULE(my_custom_module);

    for (;;)
    {
        cnc_run();
    }
}
```

That's it. Your custom function will run inside the main loop.

## Creating a new custom event

Creating custom events inside the core code is also easy.

First you need to declare the event inside one of the .h core code files

For example lets create an event to be fired when µCNC enters alarm any alarm state, that will be called inside function `void cnc_alarm(int8_t code)` in `cnc.c`

We start by declaring a new event called *cnc_alarm* (in our example inside `cnc.h`) like this

```
DECL_EVENT_HANDLER(cnc_alarm);
```

Now (in our example inside `cnc.c`) we need to add the event callback that will check if it as event listeners and call them one by one. To create a default event listener we just need to add this code:

```
WEAK_EVENT_HANDLER(cnc_alarm)
{
	DEFAULT_EVENT_HANDLER(cnc_alarm);
}

```

*Again remember. You can make a module take hover this event callback (disabling all existing listeners of this event by creating and override callback with `OVERRIDE_EVENT_HANDLER` like showned above).*

The only step left is to actually add the callback inside the core code to be executed. In this case let's call it inside our `void cnc_alarm(int8_t code)` function like this:

```
void cnc_alarm(int8_t code)
{
	cnc_set_exec_state(EXEC_KILL);
	cnc_stop();
	cnc_state.alarm = code;
#ifdef ENABLE_IO_ALARM_DEBUG
	proto_print(MSG_FEEDBACK_START);
	proto_print(__romstr__("LIMITS:"));
	proto_print_int(io_alarm_limits);
	proto_print(__romstr__("|CONTROLS:"));
	proto_print_int(io_alarm_controls);
	proto_print(MSG_FEEDBACK_END);
#endif

	// we add our callback here
	// in this callback we will pass to the listeners our error code
	// since all event callback argument is of type void* (void pointer) we send the pointer to the error code like this

	EVENT_INVOKE(cnc_alarm, &code);
}

```

That's it. Every time there is an alarm, the event handler will run and execute the listener callbacks.
