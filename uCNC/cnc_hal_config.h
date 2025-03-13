/*
	Name: cnc_hal_config.h
	Description: Compile time HAL configurations for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 18-06-2021

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef CNC_HAL_CONFIG_H
#define CNC_HAL_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/**
 *
 * The HAL config file is were the advanced HAL settings are defined µCNC.
 * These settings include Pin functions, Tools, Modules, etc
 *
 */

/**
 * Limit switch pins
 * */

// Uncomment to disable limit switch
// If the pin is not defined in the board this will be ignored
// #define LIMIT_X_DISABLE
// #define LIMIT_Y_DISABLE
// #define LIMIT_Z_DISABLE
// #define LIMIT_X2_DISABLE
// #define LIMIT_Y2_DISABLE
// #define LIMIT_Z2_DISABLE
// #define LIMIT_A_DISABLE
// #define LIMIT_B_DISABLE
// #define LIMIT_C_DISABLE

/**
 * Pins weak pullup resistors
 * */

// Uncomment to enable weak pull up resistors for limit switch
// If the pin is not defined in the board this will be ignored
#define LIMIT_X_PULLUP_ENABLE
#define LIMIT_Y_PULLUP_ENABLE
#define LIMIT_Z_PULLUP_ENABLE
#define LIMIT_X2_PULLUP_ENABLE
#define LIMIT_Y2_PULLUP_ENABLE
#define LIMIT_Z2_PULLUP_ENABLE
#define LIMIT_A_PULLUP_ENABLE
#define LIMIT_B_PULLUP_ENABLE
#define LIMIT_C_PULLUP_ENABLE

// Uncomment to enable weak pull up resistor for probe
// If the pin is not defined in the board this will be ignored
// #define PROBE_PULLUP_ENABLE

// Uncomment to enable weak pull up resistors for control pins
// If the pin is not defined in the board this will be ignored
#define ESTOP_PULLUP_ENABLE
#define SAFETY_DOOR_PULLUP_ENABLE
#define FHOLD_PULLUP_ENABLE
#define CS_RES_PULLUP_ENABLE

/**
 * Uncomment this to use Y axis as a Z axis alias on 2 axis machines
 * This allows the Gcode to expect X and Z coordinates in the parser
 * **/
// #define USE_Y_AS_Z_ALIAS

/**
 * Uncomment this feature to enable tool length compensation
 */
#ifndef AXIS_TOOL
#ifdef AXIS_Z
#define AXIS_TOOL AXIS_Z
#elif ((AXIS_COUNT == 2) && defined(USE_Y_AS_Z_ALIAS))
#define AXIS_TOOL AXIS_Y
#endif
#endif

/**
 * Uncomment to disable axis homing
 * Theses settings have no effect on the DELTA kinematic (on X,Y,Z axis)
 */
// #define DISABLE_X_HOMING
// #define DISABLE_Y_HOMING
// #define DISABLE_Z_HOMING
// #define DISABLE_A_HOMING
// #define DISABLE_B_HOMING
// #define DISABLE_C_HOMING

/**
 * Uncomment this feature to enable X and Y homing simultaneously
 */
// #define ENABLE_XY_SIMULTANEOUS_HOMING

/**
 * Rotational axis - disable limits after homing
 * Enable this option if you want to disable the limits for rotation axis to work
 * allowing them to rotatle freely
 *
 */
// #define DISABLE_ROTATIONAL_AXIS_LIMITS_AFTER_HOMING

/**
 * Rotational axis - force relative distances
 * Enable this option if you want the rotation axis to work in relative distance mode only
 * This will mean that no matter if the machine is working in absolute (G90) or relative (G91) coordinates
 * the rotational axis will always calculate the motion in relative distance mode
 *
 */
// #define AXIS_A_FORCE_RELATIVE_MODE
// #define AXIS_B_FORCE_RELATIVE_MODE
// #define AXIS_C_FORCE_RELATIVE_MODE

/**
 * Uncomment this feature to enable multi motor axis
 * NOTE: If Laser PPI is enabled one of the stepper drivers position will be used by the laser controller
 * Usually that is STEPPER<AXIS_COUNT> so if AXIS_COUNT=3, STEPPER3 will be used by laser PPI
 */
// #define ENABLE_MULTI_STEPPER_AXIS
#ifdef ENABLE_MULTI_STEPPER_AXIS

	/**
	 * Configure each multi motor linear actuator.
	 *
	 * AXIS are the motion degrees of freedom of the machine in 3D space
	 * STEP are the stepper controller drivers that are controlled by the the board
	 * LINACT are the linear actuators that drive the machine motion. You can think of linear actuator as a combination of linear guide + motor
	 *
	 * Each AXIS is logically attached to a LINACT.
	 * AXIS_X <-> LINACT0
	 * AXIS_Y <-> LINACT1
	 * etc..
	 *
	 * In some machines the coorelation between AXIS and LINACT is direct. For example cartesian machines the AXIS X motion is the result of motions of the LINACT0, AXIS Y of LINACT1, AXIS Z of LINACT2, etc.
	 * On Core XY kinematics the same logic as the cartesian is applied (regarding AXIS and LINACT logic) although, AXIS X and Y motions are a combination of motions of both LINACT0 and LINAXCT1.
	 * On Delta type machines the same logic as the cartesian is applied (regarding AXIS and LINACT logic) although, AXIS X, Y and Z are all a combination of motions of LINACT0, 1 and 2.
	 *
	 * As stated earlier LINACT is a combination of linear guide + motor. Usually a LINACT is defined as a single stepper motor.
	 * But it can also be composed of multiple stepper motors (2, 3, 4, etc...)
	 *
	 * To enable this just define the LINACTx_IO_MASK as a combination of STEPx_IO_MASK's
	 * **/

	// defines a multi stepper linear actuator LINACT0
	//  #define LINACT0_IO_MASK (STEP0_IO_MASK | STEP5_IO_MASK)

	// defines a second multi stepper linear actuator LINACT1
	//  #define LINACT1_IO_MASK (STEP1_IO_MASK | STEP6_IO_MASK)

	// defines a second multi stepper linear actuator LINACT2
	//  #define LINACT2_IO_MASK (STEP2_IO_MASK | STEP7_IO_MASK)

	// there is no limit to the ammount of STEP IO that can be combined into a LINACT. For example it's possible to assign 4 independent STEP IO to a single LINACT
	// #define LINACT2_IO_MASK (STEP2_IO_MASK | STEP5_IO_MASK | STEP6_IO_MASK | STEP7_IO_MASK)

	/**
	 * SELF SQUARING/AUTOLEVEL AXIS
	 * Limits switches have a mask value that is equivalent to the STEPx that it controls.
	 *
	 * By default these mask values match the corresponding AXIS, LINACT, STEP, etc...
	 *
	 * STEPx			7		6		5		4		3		2		1		0
	 * STEPx_IO_MASK	128		64		32		16		8		4		2		1
	 * AXISx			-		-		C		B		C		Z		Y		X
	 * LINACTx			-		-		5		4		3		2		1		0
	 * LIMITx			-		-		C		B		A		Z&Z2	Y&Y2	X&X2
	 *
	 * LINACT with multiple STEP IO pulse all those IO in sync, but when homing it can stop independently as it hits the correspondent limit until all motors reach the desired home position.
	 * To achieve that each each LIMITx_IO_MASK should be set to the corresponding STEP IO MASK that it controls
	 *
	 * For example to use STEP0 and STEP6 to drive the AXIS_X/LINACT0 you need to configure the correct LINACT0_IO_MASK and then to make LIMIT_X stop STEP0 and LIMIT_X2 stop STEP6 you need
	 * to reassign LIMIT_X2 to STEP6 IO MASK do it like this
	 *
	 * #define LIMIT_X2_IO_MASK STEP6_IO_MASK
	 *
	 * **/

	// #define ENABLE_AXIS_AUTOLEVEL

#ifdef ENABLE_AXIS_AUTOLEVEL

	// Uncomment to modify X2 limit mask value to match the X2 motor
	// #define LIMIT_X2_IO_MASK STEP5_IO_MASK

	// Uncomment to modify Y2 limit mask value to match the Y2 motor
	// #define LIMIT_Y2_IO_MASK STEP6_IO_MASK

	// Uncomment to modify Y2 limit mask value to match the Y2 motor
	// #define LIMIT_Z2_IO_MASK STEP7_IO_MASK

#endif

	/**
	 * Advanced Multi-axis
	 * The advantage of using this mask scheme is that it's possible to defined advanced custom self squaring/planning rigs
	 * and complete reassing of unused axis limits to perform the task
	 *
	 * Let's assume a custom cartesian machine that has 3 axis with 5 motors and 5 limit switches with the following configuration
	 *
	 * Axis X - 1 motor/limit
	 * Axis Y - 2 motors/limits (self squaring)
	 * Axis Z - 3 motors/limits (self leveling - 3 point plane)
	 *
	 * By default, with the machine configured for a 3 axis machine 3 linear actuators are configured with one motor each. By default AXIS_X -> LINACT0 -> STEP0, AXIS_Y-> LINACT1 -> STEP1, etc...
	 * One possibility would be to map the connections (from STEP0 to STEP5) as X,Y,Z,Y2,Z2,Z3.
	 *
	 * Let's assume the user also wants the connections to be made in the order X,Y,Y2,Z,Z2,Z3
	 *
	 *
	 * Axis X does not require any special configuration. Uses the default settings for a single linear actuator with one motor and a limit switch
	 * Axis Y requires a second STEPx output. Because we also want to reorder the connection we must define our custom LINACTx for the remaining axis like this
	 *
	 * // defines a custom LINACT1_IO_MASK to use STP1 and STEP2
	 * #define LINACT1_IO_MASK (STEP1_IO_MASK | STEP2_IO_MASK)
	 * // defines a custom LINACT1_IO_MASK to use STP3, STEP4 and STEP5
	 * #define LINACT2_IO_MASK (STEP3_IO_MASK | STEP4_IO_MASK | STEP5_IO_MASK)
	 *
	 * Then let's configure the limits:
	 *
	 * Limits for AXIS X does not require any special configuration. Uses the default settings for a single linear actuator with one motor and a limit switch (uses LIMIT_X)
	 * Limits for AXIS Y only require Limit Y2 to be reasigned to match STEP2 so in this case
	 *
	 * #define LIMIT_Y2_IO_MASK STEP2_IO_MASK
	 *
	 * And finally limits for AXIS Z will new to be reassigned to the matching STEPx. We will also need a 3rd limit. We will use LIMIT_A since AXIS_A is not used.
	 *
	 * // enable Z selfsquare/selfplane
	 * #define LIMIT_Y_IO_MASK STEP3_IO_MASK
	 * #define LIMIT_Y2_IO_MASK STEP4_IO_MASK
	 * #define LIMIT_A_IO_MASK STEP5_IO_MASK
	 *
	 * There is still a final step that involve reassign LINACT2 limits to include the extra limit (LIMIT A) like this
	 *
	 * #define LINACT2_LIMIT_MASK (LIMIT_Y_IO_MASK | LIMIT_Y2_IO_MASK | LIMIT_A_IO_MASK)
	 *
	 * Also remember that ENABLE_AXIS_AUTOLEVEL should be enabled for this to work
	 *
	 * That is it.
	 *
	 * **/

#endif

	/*
		Tool definition
		For any given tool the respective macro TOOLx (x from 1 to 16) must be created
		Tools must be assigned in sequential order
		That is TOOL1, TOOL2 etc...
		You can't skip tool numbers (for example define TOOL1 and TOOL3 without having a TOOL2)
	*/

/**
 *
 * Enables Laser PPI capabilities
 *
 * **/
#ifdef ENABLE_LASER_PPI
#define LASER_PPI PWM0
// Uncomment to invert the output login on the LASER_PPI pin
// #define INVERT_LASER_PPI_LOGIC
#endif

/**
 *
 * Tool pallete
 * You can assign your tool pallete indexes here
 * Up to 16 tools can be defined
 * M6 command is available if TOOL_COUNT >= 2
 *
 * to set a tool you just need to define which tool will be in which index.
 * For example: Set TOOL1 as laser_pwm
 *
 * #define TOOL1 laser_pwm
 *
 * Tools can be any of the built in tools available in /src/hal/tools/tools/ or you can use your own custom tool.
 *
 * **/
// assign the tools from 1 to 16
#if (TOOL_COUNT >= 1)
// to allow build on virtual emulator
#define TOOL1 spindle_pwm
#endif
#if (TOOL_COUNT >= 2)
#define TOOL2 spindle_pwm
#endif
#if (TOOL_COUNT >= 3)
#define TOOL3 spindle_pwm
#endif
#if (TOOL_COUNT >= 4)
#define TOOL4 spindle_pwm
#endif
#if (TOOL_COUNT >= 5)
#define TOOL5 spindle_pwm
#endif
#if (TOOL_COUNT >= 6)
#define TOOL6 spindle_pwm
#endif
#if (TOOL_COUNT >= 7)
#define TOOL7 spindle_pwm
#endif
#if (TOOL_COUNT >= 8)
#define TOOL8 spindle_pwm
#endif
#if (TOOL_COUNT >= 9)
#define TOOL9 spindle_pwm
#endif
#if (TOOL_COUNT >= 10)
#define TOOL10 spindle_pwm
#endif
#if (TOOL_COUNT >= 11)
#define TOOL11 spindle_pwm
#endif
#if (TOOL_COUNT >= 12)
#define TOOL12 spindle_pwm
#endif
#if (TOOL_COUNT >= 13)
#define TOOL13 spindle_pwm
#endif
#if (TOOL_COUNT >= 14)
#define TOOL14 spindle_pwm
#endif
#if (TOOL_COUNT >= 15)
#define TOOL15 spindle_pwm
#endif
#if (TOOL_COUNT >= 16)
#define TOOL16 spindle_pwm
#endif

// enable RPM encoder for spindle_pwm
// depends on encoders (below)
// #define SPINDLE_PWM_HAS_RPM_ENCODER

// enable RPM encoder for spindle_besc
// depends on encoders (below)
// #define SPINDLE_BESC_HAS_RPM_ENCODER

/**
 * Uncomment to enable PID controller for tools
 * Each tool has it's own PID controller and EEPROM settings
 * Chech the tool file to find the settings for each tool
 *
 * **/
#ifndef ENABLE_TOOL_PID_CONTROLLER
// #define ENABLE_TOOL_PID_CONTROLLER
#endif

/**
 * Set a custom filter that prevents step motions in realtime
 * This can be any expression that can be evaluated as true or false
 * If defined and the expression evaluates to true the ISR will be unable to generate steps until condition cleared
 *
 * In the example bellow if input pin DIN19 is active step ISR will stop generating steps
 * Can be uses for example in sewing machines to prevent motion on needle down detection and avoid damadge to the needle
 *
 * **/
// #define RT_STEP_PREVENT_CONDITION io_get_input(DIN19)

// Assigns an output to an blinking led (1Hz rate)
#define ACTIVITY_LED DOUT31

/*
	Sets the number of encoders to be used (max of 8)
*/
#define ENCODERS 0
/**
 * To use the encoder counter 3 definitions are needed
 * ENCx_PULSE -> must be set to an input PIN with interrupt on change enabled capabilities
 * ENCx_DIR -> a regular input PIN that detects the direction of the encoding step
 * ENCx_PPR -> the encoder Pulses Per Rotation value
 * Defined encoders must match the number of encoders and numeral defined above.
 * For example if ENCODERS is set to 2 it expects to find the definitions for ENC0 and ENC1. Number skipping is not allowed (example Set ENC0 and ENC2 but not ENC1)
 *
 * It's also possible to assign an encoder to a stepper motor by defining the STEPx_ENCODER and the encoder index like this
 * #define STEP0_ENCODER ENC0 //assigns encoder 0 to stepper 0
 *
 * The encoder can work as a simple counter (used in speed encoders) by setting the same HAL pin for both PULSE and DIR functions - Counter mode
 *
 * For encoders to work as STEP encoders ENABLE_MAIN_LOOP_MODULES must be enabled
 *
 * Encoders counting direction can be inverted via mask setting $8
 * */
#if ENCODERS > 0
#include "src/modules/encoder.h"

// Counter mode
// #define ENC0_PULSE DIN7
// #define ENC0_DIR DIN7
// #define ENC0_PPR 16384

// // Encoder mode
#define ENC0_PULSE DOUT15
#define ENC0_DIR DOUT16
#define ENC0_PPR 16384

#define ENC0_TYPE ENC_TYPE_I2C

	// #define ENC1_PULSE DIN1
	// #define ENC1_DIR DIN9

	// #define ENC2_PULSE DIN2
	// #define ENC2_DIR DIN10

	// Assign encoders to steppers
#define STEP0_ENCODER ENC0
// #define STEP1_ENCODER ENC1
// #define STEP2_ENCODER ENC2

// By default encoders are configured as pulse encoders
// But encoders can also be defined as I2C or SSI encoders
// for these types of encoders the PULSE pin is used as the clock communication pin
// the DIR pin is used as the data pin
// I2C or SSI encoders can share the same CLOCK pin for better optimization
// for example to use ENC1 with I2C and ENC2 with SSI

// #define ENC1_TYPE ENC_TYPE_I2C
// #define ENC2_TYPE ENC_TYPE_SSI

// Assign an encoder has an RPM encoder
// #define ENABLE_ENCODER_RPM
#ifdef ENABLE_ENCODER_RPM

// Assign an encoder to work as the RPM encoder
#define RPM_ENCODER ENC0

// Optional set a second encoder pin has an encoder index
// This assumes the index pulse occurs when pulse pin is also triggered
// #define RPM_INDEX_INPUT DIN8

// Resolution of the RPM encoder or Pulses Per Revolution
#define RPM_PPR 24

// uncomment to update tool sync on index pulse only
// instead of updating in every PPR
// #define RPM_SYNC_UPDATE_ON_INDEX_ONLY
#endif
#endif

/**
 *
 * Software emulated communication interfaces
 *
 * */
#ifdef SOFT_SPI_ENABLED
#ifndef SOFT_SPI_CLK
#define SOFT_SPI_CLK DOUT30
#endif
#ifndef SOFT_SPI_SDO
#define SOFT_SPI_SDO DOUT29
#endif
#ifndef SOFT_SPI_SDI
#define SOFT_SPI_SDI DIN29
#endif
#endif

	/**
	 *
	 *
	 * Microstepping pins controlled via MCU (like in RAMBO board)
	 *
	 * Uncomment for each stepper that is controlled this way
	 * The microstepping value is set has a bitmask for <MSTEP2,MSTEP1,MSTEP0>
	 *
	 * For example to set <low,high,low> = 0b010 = set value 2
	 *
	 *
	 * */

// #define STEPPER0_HAS_MSTEP
#ifdef STEPPER0_HAS_MSTEP
#define STEPPER0_MSTEP 3
#define STEPPER0_MSTEP0 DOUT20
#define STEPPER0_MSTEP1 DOUT12
#endif
// #define STEPPER1_HAS_MSTEP
#ifdef STEPPER1_HAS_MSTEP
#define STEPPER1_MSTEP 3
#define STEPPER1_MSTEP0 DOUT21
#define STEPPER1_MSTEP1 DOUT13
#endif
// #define STEPPER2_HAS_MSTEP
#ifdef STEPPER2_HAS_MSTEP
#define STEPPER2_MSTEP 3
#define STEPPER2_MSTEP0 DOUT22
#define STEPPER2_MSTEP1 DOUT14
#endif
// #define STEPPER3_HAS_MSTEP
#ifdef STEPPER3_HAS_MSTEP
#define STEPPER3_MSTEP 3
#define STEPPER3_MSTEP0 DOUT23
#define STEPPER3_MSTEP1 DOUT15
#endif
// #define STEPPER4_HAS_MSTEP
#ifdef STEPPER4_HAS_MSTEP
#define STEPPER4_MSTEP 3
#define STEPPER4_MSTEP0 DOUT24
#define STEPPER4_MSTEP1 DOUT16
#endif
// #define STEPPER5_HAS_MSTEP
#ifdef STEPPER5_HAS_MSTEP
#define STEPPER5_MSTEP 3
#define STEPPER5_MSTEP0 DOUT25
#define STEPPER5_MSTEP1 DOUT17
#endif
// #define STEPPER6_HAS_MSTEP
#ifdef STEPPER6_HAS_MSTEP
#define STEPPER6_MSTEP 3
#define STEPPER6_MSTEP0 DOUT26
#define STEPPER6_MSTEP1 DOUT18
#endif
// #define STEPPER7_HAS_MSTEP
#ifdef STEPPER7_HAS_MSTEP
#define STEPPER7_MSTEP 3
#define STEPPER7_MSTEP0 DOUT27
#define STEPPER7_MSTEP1 DOUT19
#endif

	/**
	 *
	 *
	 * Stepper current set via SPI digital potenciometer (like in RAMBO board)
	 *
	 * Uncomment to enable SPI digital potenciomenter
	 * Set current and channel for each stepper
	 *
	 * */

// #define STEPPER_CURR_DIGIPOT
#ifdef STEPPER_CURR_DIGIPOT
#define STEPPER_DIGIPOT_CLK DOUT30
#define STEPPER_DIGIPOT_SDO DOUT29
#define STEPPER_DIGIPOT_SDI DIN29
#define STEPPER_DIGIPOT_CS DOUT11
#define STEPPER0_DIGIPOT_CHANNEL 4
#define STEPPER0_DIGIPOT_VALUE 135
#define STEPPER1_DIGIPOT_CHANNEL 5
#define STEPPER1_DIGIPOT_VALUE 135
#define STEPPER2_DIGIPOT_CHANNEL 3
#define STEPPER2_DIGIPOT_VALUE 135
#define STEPPER3_DIGIPOT_CHANNEL 0
#define STEPPER3_DIGIPOT_VALUE 135
#define STEPPER4_DIGIPOT_CHANNEL 1
#define STEPPER4_DIGIPOT_VALUE 135
#define STEPPER5_DIGIPOT_CHANNEL -1
#define STEPPER5_DIGIPOT_VALUE 0
#define STEPPER6_DIGIPOT_CHANNEL -1
#define STEPPER6_DIGIPOT_VALUE 0
#define STEPPER7_DIGIPOT_CHANNEL -1
#define STEPPER7_DIGIPOT_VALUE 0
#endif

/**
 * System Menu for displays
 * System Menu manages interfaces and control actions for displays in µCNC
 * **/
// override the default max string buffer length
#ifndef SYSTEM_MENU_MAX_STR_LEN
// #define SYSTEM_MENU_MAX_STR_LEN 32
#endif

	/**
	 * Force the IO direction to be configure before each request
	 * This sets the pin direction before trying to control it
	 * Some MCU like ESP32 require this option to be enabled because the IO by some SDK function calls without realizing
	 */
	// #define FORCE_HAL_IO_DIRECTION_ONREQUEST

#ifdef __cplusplus
}
#endif

#endif
