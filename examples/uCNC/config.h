//by defining the CNC_CONFIG_H macro the default µCNC config file will be override
#define CNC_CONFIG_H

/*
	Serial COM
	Defines the serial COM baud rate
	Uses 1 start bit + 8 bit + 1 stop bit (no parity)
*/
#ifndef BAUD
#define BAUD 115200
#endif
//uncomment to enable synchronized TX (used in USB VCP)
//can be used in UART hardware but MCU will be ocuppied while sending every char
//#define ENABLE_SYNC_TX
//can be used in UART hardware but char reading will be poolled instead of interrupt driven
//#define ENABLE_SYNC_RX

/*
	Choose the board
	Check boardss.h for list of available/supported boards
*/
#ifndef BOARD
#define BOARD BOARD_UNO
#endif

/*
	Kinematic
	Defines axis count
	Defines the machine kynematics (cartesian, corexy, delta, custom, ...)
	For custom/advanced configurations go to the specified kynematics header file
*/
#ifndef AXIS_COUNT
#define AXIS_COUNT 3
#endif

#ifndef KINEMATIC
#define KINEMATIC KINEMATIC_CARTESIAN
#endif

/*
	Defines the number of supported coordinate systems supported by µCNC
	Can be any value between 1 and 9
*/
#define COORD_SYS_COUNT 6

/*
	Number of segments of an arc computed with aprox. of sin/cos math operation before performing a full calculation
*/
#define N_ARC_CORRECTION 16

/*
	Echo recieved commands.
	Uncomment to enable. Only necessary to debug communication problems
*/
//#define ECHO_CMD

/*
	Spindle configurations.
	Uncomment to enable
*/
#define USE_SPINDLE
#ifdef USE_SPINDLE
/*
	Number of seconds of delay before motions restart after releasing from a hold or after setting a new spindle speed
	This is used by spindle to ensure spindle gets up to speed in motions
*/
#define DELAY_ON_RESUME 4
#define DELAY_ON_SPINDLE_SPEED_CHANGE 1
//#define LASER_MODE
#endif

/*
	Define a coolant flood and mist pin
*/
#define USE_COOLANT
#ifdef USE_COOLANT
//uncomment to make M7 act as M8
//#define M7_SAME_AS_M8
#endif

/*
	Special definitions used to debug code
*/
//#define __DEBUG__

/*
	Feed overrides increments and percentage ranges
*/
#define FEED_OVR_MAX 200
#define FEED_OVR_MIN 10
#define FEED_OVR_COARSE 10
#define FEED_OVR_FINE 1

/*
	Rapid feed overrides percentages
*/
#define RAPID_FEED_OVR1 50
#define RAPID_FEED_OVR2 25

/*
	Spindle speed overrides increments percentages and ranges
*/
#define SPINDLE_OVR_MAX 200
#define SPINDLE_OVR_MIN 10
#define SPINDLE_OVR_COARSE 10
#define SPINDLE_OVR_FINE 1

/*
	G-code options
*/
//processes and displays the currently executing gcode numbered line
//#define GCODE_PROCESS_LINE_NUMBERS

//processes comment as defined in the RS274NGC
//#define PROCESS_COMMENTS

//accepts the E word (currently is processed has A)
//#define GCODE_ACCEPT_WORD_E

//set factor for countinuos mode (G64)
//value must be set between 0.0 and 1.0 If set to 0.0 is the same as exact path mode (G61)
#define G64_MAX_ANGLE_FACTOR 0.2f

/*
	Report specific options
*/
#define STATUS_WCO_REPORT_MIN_FREQUENCY 30
#define STATUS_OVR_REPORT_MIN_FREQUENCY STATUS_WCO_REPORT_MIN_FREQUENCY - 1

/*
	If the type of machine supports skew and needs skew correction (defined in the specified kinematics_xxx.h file)
*/
#ifdef ENABLE_SKEW_COMPENSATION
//uncomment to correct only in the xy axis
//#define SKEW_COMPENSATION_XY_ONLY
#endif

/*
	Changes the planner acceleration profile generation from axis driven to linear actuator driven
*/
//#define ENABLE_LINACT_PLANNER
#ifdef ENABLE_LINACT_PLANNER
//uncomment to do a stop and start if any of the linear actuators is at a still state or changes direction
//#define ENABLE_LINACT_COLD_START
#endif

/*
	If the type of machine need backlash compensation configure here
*/
//#define ENABLE_BACKLASH_COMPENSATION

/*
	Sets the maximum number of step doubling loops carried by the DSS (Dynamic Step Spread) algorithm (Similar to Grbl AMASS).
	The DSS algorithm allows to spread stepps by over sampling bresenham line algorithm at lower frequencies and reduce vibrations of the stepper motors
	Value should range from 0 to 3. With a value o 0 the DSS will be disabled.
*/
#define DSS_MAX_OVERSAMPLING 0

/*
	Forces pin pooling for all limits and control pins (with or without interrupts)
*/
//#define FORCE_SOFT_POLLING

/*
	Modifies the startup message to emulate Grbl (required by some programs so that uCNC is recognized a Grbl protocol controller device)
*/
//#define EMULATE_GRBL_STARTUP

/*
	Compilation specific options
*/
//ensure all variables are set to 0 at start up
//#define FORCE_GLOBALS_TO_0

//saves a little program memory bytes but much more slow CRC check
//#define CRC_WITHOUT_LOOKUP_TABLE

//EXPERIMENTAL! Uncomment to enable fast math macros to reduce the number of required cpu cycles needed for a few math operations (mainly on 8-bit processors)
//This will affect the feed rate precision in about ~5%. Output binary will be bigger.
//No fast math macros are and shoud be used in functions that calculate coordinates to avoid positional errors except multiply and divide by powers of 2 macros
//#define ENABLE_FAST_MATH
