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
 * Uncomment this feature to enable tool length compensation
 */
#if (!defined(AXIS_TOOL) && defined(AXIS_Z))
#define AXIS_TOOL AXIS_Z
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
 * Uncomment this feature to enable up to 2 dual drive axis
 * NOTE: If Laser PPI is enabled one of the stepper drivers position will be used by the laser controller
 * Usually that is STEPPER<AXIS_COUNT> so if AXIS_COUNT=3, STEPPER3 will be used by laser PPI
 */
// #define ENABLE_DUAL_DRIVE_AXIS
#ifdef ENABLE_DUAL_DRIVE_AXIS
// defines the first dual drive capable axis
// #define DUAL_DRIVE0_AXIS X
// by default this will be rewired to STEPPER6 (if available on the board)
// this can be uncommented to re-wire to an available (unused stepper other then 6)
// #define DUAL_DRIVE0_STEPPER 6
//  #define DUAL_DRIVE0_ENABLE_SELFSQUARING

// defines the first second drive capable axis
// #define DUAL_DRIVE1_AXIS Y
// by default this will be rewired to STEPPER7 (if available on the board)
// this can be uncommented to re-wire to an available (unused stepper other then 7)
// #define DUAL_DRIVE1_STEPPER 7
// #define DUAL_DRIVE1_ENABLE_SELFSQUARING
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
 * Enables Plasma THC capabilities
 *
 * **/
//  #define ENABLE_PLASMA_THC

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

// Assigns an output to an blinking led (1Hz rate)
#define ACTIVITY_LED DOUT31

/*
	Sets the number of encoders to be used (max of 8)
*/
#define ENCODERS 0
/**
 * To use the encoder counter 2 definitions are needed
 * ENCx_PULSE -> must be set to an input PIN with interrupt on change enabled capabilities
 * ENCx_DIR -> a regular input PIN that detects the direction of the encoding step
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

// // Encoder mode
// #define ENC0_PULSE DIN0
// #define ENC0_DIR DIN8

// #define ENC1_PULSE DIN1
// #define ENC1_DIR DIN9

// #define ENC2_PULSE DIN2
// #define ENC2_DIR DIN10

// Assign encoders to steppers
// #define STEP0_ENCODER ENC0
// #define STEP1_ENCODER ENC1
// #define STEP2_ENCODER ENC2

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

/*
	Sets the number of PID controllers to be used
*/
#define PID_CONTROLLERS 0

#if PID_CONTROLLERS > 0
#include "src/modules/pid.h"

	/**
	 * To use PID you need to set the number o PID controllers.
	 * PID0 is hardwired to run the tool PID (defined or not). That being said if you need a PID for any other purpose other than the tool the number of PID controllers
	 * to be enabled must be greater then 1.
	 *
	 * µCNC will run each PID in a timed slot inside the rtc timer scheduler like this.
	 * Let's say you have enabled 3 PID controllers. At each RTC call of the scheduller it will run the current PID controller in a ring loop
	 *
	 * |--RTC+0--|--RTC+1--|--RTC+2--|--RTC+3--|--RTC+4--|--RTC+5--|--RTC+6--|..etc..
	 * |--PID 0--|--PID 1--|--PID 2--|--PID 0--|--PID 1--|--PID 2--|--PID 0--|..etc..
	 *
	 * REMEMBER PID0 is hardwired to the tool PID. If the tool PID is not defined for the current tool it will simply do nothing.
	 *
	 *
	 * To use the PID controller 3 definitions are needed
	 * PIDx_DELTA() -> sets the function that gets the error between the setpoint and the current value for x PID controller
	 * PIDx_OUTPUT(X) -> sets the output after calculating the pid corrected value for x PID controller
	 * PIDx_STOP() -> runs this function on any halt or emergency stop of the machine
	 *
	 * For example
	 *
	 * #define PID1_DELTA() (my_setpoint - io_get_analog(ANA0))
	 * #define PID1_OUTPUT(X) (io_set_pwm(PWM0, X))
	 * #define PID1_STOP() (io_set_pwm(PWM0, 0))
	 *
	 * An optional configuration is the sampling rate of the PID update. By default the sampling rate is 125Hz.
	 * To reduce the sampling rate a 125/PIDx_FREQ_DIV can be defined between 1 (125Hz) and 250 (0.5Hz)
	 *
	 * You can but you should not define PID for tools. Tools have a dedicated PID that can be customized for each tool. Check the tool HAL for this.
	 *
	 * */
	// here is an example on how to add an PID controller to the spindle
	// this exemple assumes that the spindle speed is feedback via an analog pin
	// reference to io_get_spindle defined in io_control
	//  	extern uint8_t io_get_spindle(void);
	//  #define SPINDLE_SPEED ANALOG0
	//  #define PID1_DELTA() (io_get_spindle() - io_get_analog(SPINDLE_SPEED))
	//  #define PID1_OUTPUT(X) (io_set_pwm(SPINDLE_PWM, X))
	//  #define PID1_STOP() (io_set_pwm(PWM0, 0))
	//  //optional
	//  #define PID1_FREQ_DIV 50
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
 * Trinamic drivers
 *
 * */
// interface types
#define TMC_UART 1
#define TMC_SPI 2

// uncomment to enable trinamic driver
// #define STEPPER0_HAS_TMC
#ifdef STEPPER0_HAS_TMC
#define STEPPER0_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER0_TMC_INTERFACE TMC_UART
#if (STEPPER0_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER0_UART_TX DOUT20
#define STEPPER0_UART_RX DIN20
#elif (STEPPER0_TMC_INTERFACE == TMC_SPI)
#define STEPPER0_SPI_SDO DOUT29
#define STEPPER0_SPI_SDI DIN29
#define STEPPER0_SPI_CLK DOUT30
#define STEPPER0_SPI_CS DOUT20
#endif
// basic parameters
#define STEPPER0_CURRENT_MA 800
#define STEPPER0_MICROSTEP 16
#define STEPPER0_RSENSE 0.11
#define STEPPER0_HOLD_MULT 0.7
#define STEPPER0_STEALTHCHOP_THERSHOLD 0
#define STEPPER0_ENABLE_INTERPLATION true
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER0_STALL_SENSITIVITY 10
#endif
// uncomment to enable trinamic driver
// #define STEPPER1_HAS_TMC
#ifdef STEPPER1_HAS_TMC
#define STEPPER1_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER1_TMC_INTERFACE TMC_UART
#if (STEPPER1_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER1_UART_TX DOUT21
#define STEPPER1_UART_RX DIN21
#elif (STEPPER1_TMC_INTERFACE == TMC_SPI)
#define STEPPER1_SPI_SDO DOUT29
#define STEPPER1_SPI_SDI DIN29
#define STEPPER1_SPI_CLK DOUT30
#define STEPPER1_SPI_CS DOUT21
#endif
// basic parameters
#define STEPPER1_CURRENT_MA 800
#define STEPPER1_MICROSTEP 16
#define STEPPER1_RSENSE 0.11
#define STEPPER1_HOLD_MULT 0.7
#define STEPPER1_STEALTHCHOP_THERSHOLD 0
#define STEPPER1_ENABLE_INTERPLATION true
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER1_STALL_SENSITIVITY 10
#endif
// uncomment to enable trinamic driver
// #define STEPPER2_HAS_TMC
#ifdef STEPPER2_HAS_TMC
#define STEPPER2_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER2_TMC_INTERFACE TMC_UART
#if (STEPPER2_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER2_UART_TX DOUT22
#define STEPPER2_UART_RX DIN22
#elif (STEPPER2_TMC_INTERFACE == TMC_SPI)
#define STEPPER2_SPI_SDO DOUT29
#define STEPPER2_SPI_SDI DIN29
#define STEPPER2_SPI_CLK DOUT30
#define STEPPER2_SPI_CS DOUT22
#endif
// basic parameters
#define STEPPER2_CURRENT_MA 800
#define STEPPER2_MICROSTEP 16
#define STEPPER2_RSENSE 0.11
#define STEPPER2_HOLD_MULT 0.7
#define STEPPER2_STEALTHCHOP_THERSHOLD 0
#define STEPPER2_ENABLE_INTERPLATION true
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER2_STALL_SENSITIVITY 10
#endif
// uncomment to enable trinamic driver
// #define STEPPER3_HAS_TMC
#ifdef STEPPER3_HAS_TMC
#define STEPPER3_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER3_TMC_INTERFACE TMC_UART
#if (STEPPER3_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER3_UART_TX DOUT23
#define STEPPER3_UART_RX DIN23
#elif (STEPPER3_TMC_INTERFACE == TMC_SPI)
#define STEPPER3_SPI_SDO DOUT29
#define STEPPER3_SPI_SDI DIN29
#define STEPPER3_SPI_CLK DOUT30
#define STEPPER3_SPI_CS DOUT23
#endif
// basic parameters
#define STEPPER3_CURRENT_MA 800
#define STEPPER3_MICROSTEP 16
#define STEPPER3_RSENSE 0.11
#define STEPPER3_HOLD_MULT 0.7
#define STEPPER3_STEALTHCHOP_THERSHOLD 0
#define STEPPER3_ENABLE_INTERPLATION true
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER3_STALL_SENSITIVITY 10
#endif
// uncomment to enable trinamic driver
// #define STEPPER4_HAS_TMC
#ifdef STEPPER4_HAS_TMC
#define STEPPER4_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER4_TMC_INTERFACE TMC_UART
#if (STEPPER4_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER4_UART_TX DOUT24
#define STEPPER4_UART_RX DIN24
#elif (STEPPER4_TMC_INTERFACE == TMC_SPI)
#define STEPPER4_SPI_SDO DOUT29
#define STEPPER4_SPI_SDI DIN29
#define STEPPER4_SPI_CLK DOUT30
#define STEPPER4_SPI_CS DOUT24
#endif
// basic parameters
#define STEPPER4_CURRENT_MA 800
#define STEPPER4_MICROSTEP 16
#define STEPPER4_RSENSE 0.11
#define STEPPER4_HOLD_MULT 0.7
#define STEPPER4_STEALTHCHOP_THERSHOLD 0
#define STEPPER4_ENABLE_INTERPLATION true
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER4_STALL_SENSITIVITY 10
#endif
// uncomment to enable trinamic driver
// #define STEPPER5_HAS_TMC
#ifdef STEPPER5_HAS_TMC
#define STEPPER5_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER5_TMC_INTERFACE TMC_UART
#if (STEPPER5_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER5_UART_TX DOUT25
#define STEPPER5_UART_RX DIN25
#elif (STEPPER5_TMC_INTERFACE == TMC_SPI)
#define STEPPER5_SPI_SDO DOUT29
#define STEPPER5_SPI_SDI DIN29
#define STEPPER5_SPI_CLK DOUT30
#define STEPPER5_SPI_CS DOUT25
#endif
// basic parameters
#define STEPPER5_CURRENT_MA 800
#define STEPPER5_MICROSTEP 16
#define STEPPER5_RSENSE 0.11
#define STEPPER5_HOLD_MULT 0.7
#define STEPPER5_STEALTHCHOP_THERSHOLD 0
#define STEPPER5_ENABLE_INTERPLATION true
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER5_STALL_SENSITIVITY 10
#endif
// uncomment to enable trinamic driver
// #define STEPPER6_HAS_TMC
#ifdef STEPPER6_HAS_TMC
#define STEPPER6_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER6_TMC_INTERFACE TMC_UART
#if (STEPPER6_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER6_UART_TX DOUT26
#define STEPPER6_UART_RX DIN26
#elif (STEPPER6_TMC_INTERFACE == TMC_SPI)
#define STEPPER6_SPI_SDO DOUT29
#define STEPPER6_SPI_SDI DIN29
#define STEPPER6_SPI_CLK DOUT30
#define STEPPER6_SPI_CS DOUT26
#endif
// basic parameters
#define STEPPER6_CURRENT_MA 800
#define STEPPER6_MICROSTEP 16
#define STEPPER6_RSENSE 0.11
#define STEPPER6_HOLD_MULT 0.7
#define STEPPER6_STEALTHCHOP_THERSHOLD 0
#define STEPPER6_ENABLE_INTERPLATION true
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER6_STALL_SENSITIVITY 10
#endif
// uncomment to enable trinamic driver
// #define STEPPER7_HAS_TMC
#ifdef STEPPER7_HAS_TMC
#define STEPPER7_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER7_TMC_INTERFACE TMC_UART
#if (STEPPER7_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER7_UART_TX DOUT27
#define STEPPER7_UART_RX DIN27
#elif (STEPPER7_TMC_INTERFACE == TMC_SPI)
#define STEPPER7_SPI_SDO DOUT29
#define STEPPER7_SPI_SDI DIN29
#define STEPPER7_SPI_CLK DOUT30
#define STEPPER7_SPI_CS DOUT27
#endif
// basic parameters
#define STEPPER7_CURRENT_MA 800
#define STEPPER7_MICROSTEP 16
#define STEPPER7_RSENSE 0.11
#define STEPPER7_HOLD_MULT 0.7
#define STEPPER7_STEALTHCHOP_THERSHOLD 0
#define STEPPER7_ENABLE_INTERPLATION true
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER7_STALL_SENSITIVITY 10
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

#ifdef __cplusplus
}
#endif

#endif
