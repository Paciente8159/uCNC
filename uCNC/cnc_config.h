/*
		Name: cnc_config.h
		Description: Compile time configurations for µCNC.

		Copyright: Copyright (c) João Martins
		Author: João Martins
		Date: 19/09/2019

		µCNC is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version. Please see
   <http://www.gnu.org/licenses/>

		µCNC is distributed WITHOUT ANY WARRANTY;
		Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A
   PARTICULAR PURPOSE. See the	GNU General Public License for more details.
*/

#ifndef CNC_CONFIG_H
#define CNC_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

	/**
	 * Serial COM
	 * Defines the serial COM baud rate
	 * Uses 1 start bit + 8 bit + 1 stop bit (no parity)
	 * */

#ifndef BAUDRATE
#define BAUDRATE 115200
#endif

#ifndef ENABLE_WIFI
// #define ENABLE_WIFI
#endif

#ifndef ENABLE_BLUETOOTH
// #define ENABLE_BLUETOOTH
#endif

	/**
	 * Choose the board
	 * Check boards.h for list of available/supported boards
	 * */

#ifndef BOARD
#define BOARD BOARD_UNO
#endif

// optional name to override default board name build info (if option enabled)
#ifndef BOARD_NAME
// #define BOARD_NAME "My custom board"
#endif

	/**
	 * Kinematic
	 *
	 * Defines axis count
	 * Defines the machine kinematics (cartesian, corexy, delta, custom, ...)
	 * For custom/advanced configurations go to the specified kinematics header file
	 * This does not take in account any dual drive axis. For example a X Y Y2 Z machine is still a 3 axis machine
	 * */

#ifndef AXIS_COUNT
#define AXIS_COUNT 3
#endif

#ifndef KINEMATIC
#define KINEMATIC KINEMATIC_CARTESIAN
#endif

	/**
	 * Defines the number of supported coordinate systems supported by µCNC
	 * Can be any value between 1 and 9
	 * */

#define COORD_SYS_COUNT 6

	/**
	 * Uncomment to enable G92 storing on non volatile memory
	 * If disabled G92 will be stored in RAM only. Soft-reset will not erase stored value.
	 * */
	// #define G92_STORE_NONVOLATILE

	/**
	 * Number of segments of an arc computed with aprox. of sin/cos math
	 * operation before performing a full calculation
	 * */

#define N_ARC_CORRECTION 16

	/**
	 * Echo received commands.
	 * Uncomment to enable. Only necessary to debug communication problems
	 * */

	// #define ECHO_CMD

	/**
	 * Debug command parsing time
	 * Uncomment to enable. This measures the time it takes to execute a command line and place it in the planner
	 * */

	// #define ENABLE_PARSING_TIME_DEBUG

	/**
	 * Override default configuration settings. Use _PER_AXIS parameters to
	 * define different settings for each axis.
	 */

	// #define DEFAULT_DIR_INV_MASK 0
	// #define DEFAULT_LIMIT_INV_MASK 0
	// #define DEFAULT_SOFT_LIMITS_ENABLED 0
	// #define DEFAULT_HARD_LIMITS_ENABLED 0
	// #define DEFAULT_HOMING_ENABLED 0
	// #define DEFAULT_HOMING_DIR_INV_MASK 0
	// #define DEFAULT_HOMING_FAST 50
	// #define DEFAULT_HOMING_SLOW 10
	// #define DEFAULT_HOMING_OFFSET 2
	// #define DEFAULT_STEP_PER_MM 200
	// #define DEFAULT_STEP_PER_MM_PER_AXIS {200, 200, 200}
	// #define DEFAULT_MAX_FEED 500
	// #define DEFAULT_MAX_FEED_PER_AXIS {500, 500, 500}
	// #define DEFAULT_ACCEL 10
	// #define DEFAULT_ACCEL_PER_AXIS {10, 10, 10}
	// #define DEFAULT_MAX_DIST 200
	// #define DEFAULT_MAX_DIST_PER_AXIS {200, 200, 50}
	// #define DEFAULT_ARC_TOLERANCE 0.002
	// #define DEFAULT_DEBOUNCE_MS 250

#if defined(KINEMATIC_DELTA)
	// #define DEFAULT_LIN_DELTA_ARM_LENGTH 230
	// #define DEFAULT_LIN_DELTA_BASE_RADIUS 115
#endif

#if defined(KINEMATIC_LINEAR_DELTA)
	// #define DEFAULT_DELTA_BICEP_LENGTH 100
	// #define DEFAULT_DELTA_FOREARM_LENGTH 300
	// #define DEFAULT_DELTA_EFFECTOR_RADIUS 24
	// #define DEFAULT_DELTA_BASE_RADIUS 75
	// #define DEFAULT_DELTA_BICEP_HOMING_ANGLE 0
#endif

	/**
	 * Sets/limits the number of tools to be used
	 * The tool and tool order are configured in the cnc_hal_config.h
	 * */

#ifndef TOOL_COUNT
#define TOOL_COUNT 1
#endif

#if TOOL_COUNT > 0

/**
 * Override the default startup tool
 */
// #define DEFAULT_STARTUP_TOOL 1

/**
 * Enable or disable coolant
 * */
// #define ENABLE_COOLANT

/**
 * On speed change waits for the tool to reach the designated speed
 * The maximum error is a value between 0-0% and 100-100%
 * 0% error will only continue if the tool reaches the exact programed speed
 * 100% will continue regardless of the current value
 * This will be ignored on laser mode
 * */
// #define TOOL_WAIT_FOR_SPEED
#ifdef TOOL_WAIT_FOR_SPEED
#define TOOL_WAIT_FOR_SPEED_MAX_ERROR 5
#endif

/**
 * Number of seconds of delay before motions restart after releasing from a
 * hold or after setting a new spindle speed This is used by spindle to
 * ensure spindle gets up to speed in motions
 * This will be ignored on laser mode
 * */
#define DELAY_ON_RESUME_SPINDLE 4
#define DELAY_ON_SPINDLE_SPEED_CHANGE 1
// define coolant delay at restart
#define DELAY_ON_RESUME_COOLANT 1
// uncomment to make M7 act as M8
// #define M7_SAME_AS_M8
#endif

/**
 * Uncomment to enable laser PPI feature
 * Laser PPI requires the MCU to support ONESHOT timeout
 * Also note that enabling LASER PPI will use an internal STEPPER motor for calculations
 * Usually this will occupy STEPPER with index AXIS_COUNT
 * For example on a 3 axis machine (uses steppers 0 to 2) laser ppi will use stepper 3 index
 * This should be taken in account with pin mapping and dual axis config
 * */
// #define ENABLE_LASER_PPI

/**
 * Override default laser configuration settings.
 */

// #define DEFAULT_LASER_PPI 254
// #define DEFAULT_LASER_PPI_USWIDTH 1500

/**
 *
 * Enables Plasma THC capabilities
 *
 * **/
//  #define ENABLE_PLASMA_THC

/**
 * Feed overrides increments and percentage ranges
 * */
#define FEED_OVR_MAX 200
#define FEED_OVR_MIN 10
#define FEED_OVR_COARSE 10
#define FEED_OVR_FINE 1

	/**
	 * Rapid feed overrides percentages
	 * */

#define RAPID_FEED_OVR1 50
#define RAPID_FEED_OVR2 25

	/**
	 * Spindle speed overrides increments percentages and ranges
	 * */

#define SPINDLE_OVR_MAX 200
#define SPINDLE_OVR_MIN 10
#define SPINDLE_OVR_COARSE 10
#define SPINDLE_OVR_FINE 1

/**
 * G-code options
 */

/**
 * ignores undefined axis in gcode instead of sending an error
 * */
#define IGNORE_UNDEFINED_AXIS

/**
 * accept G0 and G1 without explicit target
 * */
#define IGNORE_G0_G1_MISSING_AXIS_WORDS

/**
 * processes and displays the currently executing gcode numbered line
 * */

// #define GCODE_PROCESS_LINE_NUMBERS
#ifdef GCODE_PROCESS_LINE_NUMBERS
// uncomment this line to ignore the value in the N parameter and count real
// text lines
// #define GCODE_COUNT_TEXT_LINES
#endif

	/**
	 * processes comment as defined in the RS274NGC
	 * */

	// #define PROCESS_COMMENTS

	/**
	 * Enables RS274NGC canned cycles
	 * */

	// #define ENABLE_CANNED_CYCLES

	/**
	 * accepts the E word (currently is processed has A)
	 * */

	// #define GCODE_ACCEPT_WORD_E

	/**
	 * Shrink µCNC
	 * It's possible to shrink µCNC by disable some core features:
	 *   - arc support (G2,G3,G17,G18,G19)
	 *   - probing (G38.x and if enabled G39,G39.x)
	 *   - coordinate systems (G55 to G59.3, G54 remains active) and home  and non volatile storage for these systems
	 *   - home commands (G28 and G30)
	 *   - disable G10. This affects coordinate systems and home as it's not possible to define them.
	 */
	// #define DISABLE_ARC_SUPPORT
	// #define DISABLE_PROBING_SUPPORT
	// #define DISABLE_COORD_SYS_SUPPORT
	// #define DISABLE_HOME_SUPPORT
	// #define DISABLE_G10_SUPPORT
	// #define DISABLE_PATH_MODES

	/**
	 * enable step counting on sync motion command (needed for some Gcode extensions like G33)
	 * */
	// #define ENABLE_RT_SYNC_MOTIONS

	/**
	 * enable motion control and planner highjacking
	 * this unlocks funtions to perform a full planner copy and restore
	 * this requires some memory since the full planned contents must be stored and also the motion control reference position
	 * */
	// #define ENABLE_MOTION_CONTROL_PLANNER_HIJACKING

	/**
	 * Uncomment to enable module extensions
	 * */
	// #define ENABLE_MAIN_LOOP_MODULES
	// #define ENABLE_IO_MODULES
	// #define ENABLE_PARSER_MODULES
	// #define ENABLE_MOTION_CONTROL_MODULES

	/**
	 * Settings extensions are enabled by default
	 * Uncomment to disable this extension.
	 * Some option might override this (like ENABLE_TOOL_PID_CONTROLLER)
	 * */
// #define DISABLE_SETTINGS_MODULES

/**
 * Report specific options
 * */
#define STATUS_WCO_REPORT_MIN_FREQUENCY 30
#define STATUS_OVR_REPORT_MIN_FREQUENCY STATUS_WCO_REPORT_MIN_FREQUENCY - 1

// enables automatic status report sending
// this value sets the millisecond interval of the reports
// values bellow 100ms have no effect
#define STATUS_AUTOMATIC_REPORT_INTERVAL 0

/**
 *
 * Enable this option to set home has your machine origin.
 * When a machine homes each axis is set to 0 or max_axis_distance (settings $13x) depending on if the home direction invert mask is turned on or off (settting $23)
 * In practice $23 sets if the machine homes towards the origin (default) or away from the origin (inverted)
 * After homing the machine coordinate system is set in a way that the workable volume has always positive coordinates.
 * By enabling this option after homing the machine will set the homing position has it's origin.
 * Because of this the machine coordinate system might be offset to negative dimensions in some axis.
 * */

// #define SET_ORIGIN_AT_HOME_POS

/**
 *
 * Enable this option to allow the $H to be used to perform a software homing.
 * Software homing will only work if hardware limits are disabled.
 * This will apply the not execute the homing search motions but it will still
 * execute the pull-off motion before reset the coordinate system
 *
 * */

// #define ALLOW_SOFTWARE_HOMING

/**
 * Enable this option to modify the behavior of software limits
 * By default a motion that travels beyond software limits makes the controller send an alarm and halts the program
 * You can modify this behavior to make the controller send an error and continue
 * or set the machine into an hold and wait for the user to allow it to continue
 * **/

// #define MODIFY_SOFT_LIMIT_TO_ERROR
#ifdef MODIFY_SOFT_LIMIT_TO_ERROR
// uncomment this to allow motion to continue
// otherwise it will put the machine in hold until the user allows it to continue
// #define ALLOW_MOTION_TO_CONTINUE
#endif

	/**
	 * If the type of machine supports skew and needs skew correction
	 *  (defined in the specified kinematics_xxx.h file)
	 * */

// #define ENABLE_SKEW_COMPENSATION
#ifdef ENABLE_SKEW_COMPENSATION
// uncomment to correct only in the xy axis
// #define SKEW_COMPENSATION_XY_ONLY
#endif

/**
 * Uncomment to enable surface height mapping compensation
 * This enables G39 gcode and is useful for PCB milling and similar jobs
 * It uses a 9 point matrix and bilinear interpolation to compensate for Z height deformations
 * To map a region do G39 X<left bottom corner> Y<left bottom corner> Z<max-depth> I<X region offset> J<Y region offset>
 * G39.1 will disable HMAP
 * G39.2 will re-enable it
 *
 * It's an error if:
 *  - I and J are missing
 *  - I or J are negative
 *  - Z is missing
 *  - cutter radius compensation is active (not implemented)
 *
 * The map will not be stored in memory and will be reset on any of the following conditions
 *  - a hardware or software reset
 *  - a homing command
 **/
// #define ENABLE_G39_H_MAPPING
#ifdef ENABLE_G39_H_MAPPING
// set the grid size factor
// this sets the size of the Hmap -> H_MAPING_GRID_FACTOR ^ 2
// the minimum value is 2 (4 points) and the maximum is 6 (36 points)
#define H_MAPING_GRID_FACTOR 3
#endif

	/**
	 * Changes the planner acceleration profile generation from axis driven to
	 * linear actuator driven
	 * */

	// #define ENABLE_LINACT_PLANNER
#ifdef ENABLE_LINACT_PLANNER
	// uncomment to do a stop and start if any of the linear actuators is at a
	// still state or changes direction
	// #define ENABLE_LINACT_COLD_START
#endif

	/**
	 * If the type of machine need backlash compensation configure here
	 * */

	// #define ENABLE_BACKLASH_COMPENSATION

	/**
	 * Uncomment these to enable step ISR calculation strategies (uses more
	 * memory) STEP_ISR_SKIP_MAIN - carries the information about the main
	 * stepper (performs a step in every ISR tick) and skips calculations
	 * STEP_ISR_SKIP_IDLE - carries the information about the idle steppers
	 * (performs 0 steps in the ISR tick) and skips calculations
	 * */

#define STEP_ISR_SKIP_MAIN
#define STEP_ISR_SKIP_IDLE

	/**
	 * Sets the maximum number of step doubling loops carried by the DSS (Dynamic
	 * Step Spread) algorithm (Similar to Grbl AMASS). The DSS algorithm allows
	 * to spread steps by over sampling bresenham line algorithm at lower
	 * frequencies and reduce vibrations of the stepper motors Value should range
	 * from 0 to 3. With a value o 0 the DSS will be disabled.
	 * */

#define DSS_MAX_OVERSAMPLING 3
#define DSS_CUTOFF_FREQ 500

	/**
	 * Modifies the bresenham algorithm to use a 16-version (experimental).
	 * This uses less memory, faster ISR stepping, but increases motion and
	 * planner calculations since line segments are divided into smaller
	 * segments.
	 * */

	// #define BRESENHAM_16BIT

	/**
	 * Performs motions with variable acceleration (trapezoidal speed profile
	 * with rounded speed transition between accel/deaccel and constant speed)
	 * instead of constant acceleration (trapezoidal speed profile)
	 *
	 * -1 - selectable via setting $14
	 *  0 - disabled
	 *  1 - mild profile (smaller mid slope and higher initial and exit slopes)
	 *  2 - medium profile (medium mid slope and medium initial and exit slopes)
	 *  3 - stron profile (high mid slope and medium initial and exit slopes)
	 *  4 - agressive (higher mid slope and smaller initial and exit slopes - uses bezier 5th order)
	 *  5 - agressive2 (higher mid slope and smaller initial and exit slopes - uses tanh curve)
	 *
	 * */

#define S_CURVE_ACCELERATION_LEVEL 0

	/**
	 * Forces pin pooling for all limits and control pins (with or without
	 * interrupts)
	 * */

	// #define FORCE_SOFT_POLLING

	/**
	 * Runs a check for state change inside the scheduler. This is a failsafe
	 * check to pin ISR checking The value sets the frequency of this safety
	 * check that is executed every 2^(CTRL_SCHED_CHECK) milliseconds. A
	 * negative value will disable this feature. The maximum is 7
	 * */

#define CTRL_SCHED_CHECK 4

/**
 * Uncomment to invert Emergency stop button
 * */
#ifndef INVERT_EMERGENCY_STOP
// #define INVERT_EMERGENCY_STOP
#endif

	/**
	 * Disable/enable all control, limits or/and probing input pins. This
	 * helps to reduce code size if features are not needed
	 * */
#ifndef DISABLE_ALL_CONTROLS
#define DISABLE_ALL_CONTROLS
#endif
#ifndef DISABLE_ALL_LIMITS
// #define DISABLE_ALL_LIMITS
#endif
#ifndef DISABLE_PROBE
// #define DISABLE_PROBE
#endif

	/**
	 *
	 * Uncomment to store the state of the limits, controls and probe states that tiggered and alarm
	 * This is useful to debug momentary faults
	 *
	 */
	// #define ENABLE_IO_ALARM_DEBUG

	/**
	 * Modifies the startup message to emulate Grbl (required by some programs so
	 * that uCNC is recognized a Grbl protocol controller device)
	 * 0 - disables
	 * 1 - partially emulates the startup message and prints unused settings to improve compatibility
	 * 2 - full emulation of the grbl startup and info messages (this also makes command $IE available to print the firmware information in extended format)
	 * */

#define EMULATE_GRBL_STARTUP 1

	/**
	 *
	 * Enables $I Grbl info command on µCNC.
	 *
	 * */

#define ENABLE_SYSTEM_INFO

	/**
	 * Enables additional core grbl system commands
	 * For settings allows settings to only be stored in EEPROM/Flash explicitly
	 * on special command This makes that all $<setting-id>=<setting-value>
	 * commands are only performed in SRAM and not stored directly to
	 * EEPROM/Flash A few commands are added: $SS - Settings store - records
	 * settings from SRAM to EEPROM/Flash $SL - Settings load - Loads settings
	 * from EEPROM/Flash to SRAM $SR - Settings reset - Reloads the default value
	 * settings from ROM to SRAM
	 *
	 * For pin diagnostics enables command $P
	 * */

	// #define ENABLE_EXTRA_SYSTEM_CMDS
	// uncomment o translate pins names when printing pins states with $P command
	// #define ENABLE_PIN_TRANSLATIONS

	/**
	 * Compilation specific options
	 * */

	/**
	 * forces all global variables are set to 0 at start up
	 * this should not be necessary since the linker usually performs this task
	 * */

	// #define FORCE_GLOBALS_TO_0

	/**
	 * saves a little program memory bytes but much more slow CRC check
	 * */

#define CRC_WITHOUT_LOOKUP_TABLE

	/**
	 * This uses RAM only settings
	 * Storing is disabled and the defaults will be loaded at each power up
	 * This is useful if you don't have EEPROM/FLASH storage or the divide read/write maximum cycle count is low to prevent damage
	 * */
	// #define RAM_ONLY_SETTINGS

	/**
	 * EXPERIMENTAL! Uncomment to enable fast math macros to reduce the number of
	 * required cpu cycles needed for a few math operations (mainly on 8-bit
	 * processors) This will affect the feed rate precision in about ~5%. Output
	 * binary will be bigger. No fast math macros are and should be used in
	 * functions that calculate coordinates to avoid positional errors except
	 * multiply and divide by powers of 2 macros
	 * */

	// #define ENABLE_FAST_MATH

/**
 *
 * HAL offsets
 *
 * */
#ifndef PWM_PINS_OFFSET
#define PWM_PINS_OFFSET 25
#endif
#ifndef SERVO_PINS_OFFSET
#define SERVO_PINS_OFFSET 41
#endif
#ifndef DOUT_PINS_OFFSET
#define DOUT_PINS_OFFSET 47
#endif
#ifndef ANALOG_PINS_OFFSET
#define ANALOG_PINS_OFFSET 114
#endif
#ifndef DIN_PINS_OFFSET
#define DIN_PINS_OFFSET 130
#endif

#ifdef __cplusplus
}
#endif

#endif
