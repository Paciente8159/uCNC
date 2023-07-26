<p align="center">
<img src="https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true">
</p>


# µCNC
µCNC - Universal CNC firmware for microcontrollers

# Adding custom tools to µCNC

µCNC tool is a bit different from modules but it's equally straight forward.
tool.h file sets a struct with a set of function pointer that can be defined to create a custom tool. In the end you just need to add the tool in cnc_hal_config.h file by attributing it to a TOOL<x>.

## µCNC tool structure

The tool struct is like this:

```
	typedef void (*tool_func)(void);
	typedef int16_t (*tool_range_speed_func)(int16_t);
	typedef uint16_t (*tool_get_speed_func)(void);
	typedef void (*tool_set_speed_func)(int16_t);
	typedef void (*tool_coolant_func)(uint8_t);

	typedef struct
	{
		tool_func startup_code;			   /*runs any custom code after the tool is loaded*/
		tool_func shutdown_code;		   /*runs any custom code before the tool is unloaded*/
		tool_func pid_update;			   /*runs de PID update code needed to keep the tool at the desired speed/power*/
		tool_range_speed_func range_speed; /*converts core speed to tool speed*/
		tool_get_speed_func get_speed;	   /*gets the tool speed/power (converts from tool speed to core speed)*/
		tool_set_speed_func set_speed;	   /*sets the speed/power of the tool*/
		tool_coolant_func set_coolant;	   /*enables/disables the coolant*/
	} tool_t;
```

## µCNC creating a dummy tool

This is an example for creating a dummy tool. Again all current tools are inside the `src/hal/tool/tools` directory but it's not mandatory. It's just a matter of having them organized.

Add a new file .c to uCNC directory (same has uCNC.ino) and paste this code

__NOTE:__ You don't need to define all functions for the tool. Just the ones you need on your tool.

```
//include cnc.h relative path
#include "src/cnc.h"

//may be needed (contains the definition of bool)
#include <stdbool.h>

static void dummy_tool_startup_code(void)
{
    // run code on tool loading
    // can be anything. Configuring IO, special motions, changing states, etc...
}

static void dummy_tool_shutdown_code(void)
{
    // run code on tool unloading
    // can be anything. Configuring IO, special motions, changing states, etc...
}

static void dummy_tool_set_speed(int16_t value)
{
    // if M3 the value will be positive
    // if M4 the value will be negative
    // a value of 0 is for an absolute stop

    // this value is received in tool IO units (for PWM should be between -255(inverse direction-M4) and 255(forward direction-M3))

    // with this value some operation must be done in order to update the tool
}

static int16_t dummy_tool_range_speed(int16_t value)
{
    // in this function you do all your calculations to convert from GCode S speed to tool IO control speed (to PWM value or other for example)

    // if M3 the value will be positive
    // if M4 the value will be negative

    // do something to value and return the converted value, that can be either a convertion formula or a lookup table

    return value;
}

static uint16_t dummy_tool_get_speed(void)
{
   // this can return the tool real speed value is some sort of sensor is present
   // this should return the value of the sensor converted to S units
   return 0;
}

static void dummy_tool_pid_update(void)
{
   // you can do anything here
   // for example read the tool sensor and based on that modify the tool speed or the axis speed to match sync motions
}

// declares the tool in ROM
// all unused function pointer must be initialized to NULL to prevent unexpected behavior

// this is an example

const tool_t dummy_tool = {
	.startup_code = &dummy_tool_startup_code, // pointer to startup function
	.shutdown_code = NULL,					  // not used (empty pointer)
	.pid_update = &dummy_tool_pid_update,	  // pointer to pid function
	.range_speed = &dummy_tool_range_speed,	  // pointer to range function
	.get_speed = &dummy_tool_get_speed,		  // pointer to custom get_speed function
	.set_speed = &dummy_tool_set_speed,		  // pointer to set_speed function
	.set_coolant = NULL						  // not used (empty pointer)
};

```

That's it.
The only thing left is to add the tool in cnc_hal_config.h
Lets say you have setup 5 tools in total and this will be TOOL5

Open uCNC.ino
add this to cnc_hal_config.h

`#define TOOL5 dummy_tool`

That's it. You can use your tool via `M6 T5` command.
