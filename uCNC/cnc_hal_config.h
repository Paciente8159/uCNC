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
 * The HAL config file is were the IO pins are "connected" to
 * alias name to be used inside µCNC
 *
 */

/*
	Uncomment this feature to enable tool length compensation
*/
#if (!defined(AXIS_TOOL) && defined(AXIS_Z))
#define AXIS_TOOL AXIS_Z
#endif

/*
	Uncomment this feature to enable up to 2 dual drive axis
*/
//#define ENABLE_DUAL_DRIVE_AXIS
#ifdef ENABLE_DUAL_DRIVE_AXIS
// defines the first dual drive capable step output
//#define DUAL_DRIVE_AXIS0 X
//#define DUAL_DRIVE_AXIS1 Y
#endif

	/**
	Tool definition
	For any given tool the respective macro TOOLx (x from 1 to 16) must be created
*/

// assign the tools from 1 to 16
#define TOOL1 spindle1
	//#define TOOL2 laser1

#if DOUT31 >= 0
#define LED DOUT31
#endif

// these modules must be enabled to use encoders
#if defined(ENABLE_IO_MODULES) && defined(ENABLE_INTERPOLATOR_MODULES) && defined(ENABLE_MAIN_LOOP_MODULES) && (defined(ENABLE_PROTOCOL_MODULES) || !defined(ENABLE_EXTRA_SYSTEM_CMDS))
/*
	Sets the number of encoders to be used (max of 8)
*/
#define ENCODERS 0

/**
 * To use the encoder counter 2 definitions are needed
 * ENCx_PULSE -> must be set to an input PIN with interrupt on change enabled capabilities
 * ENCx_DIR -> a regular input PIN that detects the direction of the encoding step
 * Defined encoders must match the number of encoders and numeral defined above.
 * For example if ENCODERS is set to 2 it expects to find the definitions for ENC0 and ENC1. Number skipping is not allowed (exemple Set ENC0 and ENC2 but not ENC1)
 * */
// #define ENC0_PULSE DIN0
// #define ENC0_DIR DIN8
#endif

// these modules must be enabled to use pid
#if defined(ENABLE_MAIN_LOOP_MODULES) && defined(ENABLE_SETTINGS_MODULES)
/*
	Sets the number of PID controllers to be used
*/
#define PID_CONTROLLERS 0

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
	 * #define PID1_DELTA() (my_setpoint - mcu_get_analog(ANA0))
	 * #define PID1_OUTPUT(X) (mcu_set_pwm(PWM0, X))
	 * #define PID1_STOP() (mcu_set_pwm(PWM0, 0))
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
	//  #define PID1_DELTA() (io_get_spindle() - mcu_get_analog(SPINDLE_SPEED))
	//  #define PID1_OUTPUT(X) (mcu_set_pwm(SPINDLE_PWM, X))
	//  #define PID1_STOP() (mcu_set_pwm(PWM0, 0))
	//  //optional
	//  #define PID1_FREQ_DIV 50
#endif

#if defined(ENABLE_MOTION_MODULES)
// uncomment to enable BLTOUCH PROBE
#define ENABLE_BLTOUCH_PROBE
// BLTOUCH uses SERVO0 pin of µCNC by default
// the servo pin can be changed here
// #define BLTOUCH_PROBE_SERVO SERVO0
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
//#define STEPPER0_HAS_TMC
#ifdef STEPPER0_HAS_TMC
#define STEPPER0_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER0_TMC_INTERFACE TMC_UART
#if (STEPPER0_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER0_UART_TX DOUT23
#define STEPPER0_UART_RX DIN24
#elif (STEPPER0_TMC_INTERFACE == TMC_SPI)
#define STEPPER0_SPI_DO DOUT13
#define STEPPER0_SPI_DI DIN13
#define STEPPER0_SPI_CLK DOUT14
#define STEPPER0_SPI_CS DOUT23
#endif
// basic parameters
#define STEPPER0_CURRENT_MA 800
#define STEPPER0_MICROSTEP 16
#define STEPPER0_RSENSE 0.11
#define STEPPER0_HOLD_MULT 0.7
#define STEPPER0_STEALTHCHOP_THERSHOLD 0
#define STEPPER0_ENABLE_INTERPLATION true
#endif
// uncomment to enable trinamic driver
//#define STEPPER1_HAS_TMC
#ifdef STEPPER1_HAS_TMC
#define STEPPER1_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER1_TMC_INTERFACE TMC_UART
#if (STEPPER1_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER1_UART_TX DOUT24
#define STEPPER1_UART_RX DIN25
#elif (STEPPER1_TMC_INTERFACE == TMC_SPI)
#define STEPPER1_SPI_DO DOUT13
#define STEPPER1_SPI_DI DIN13
#define STEPPER1_SPI_CLK DOUT14
#define STEPPER1_SPI_CS DOUT24
#endif
// basic parameters
#define STEPPER1_CURRENT_MA 800
#define STEPPER1_MICROSTEP 16
#define STEPPER1_RSENSE 0.11
#define STEPPER1_HOLD_MULT 0.7
#define STEPPER1_STEALTHCHOP_THERSHOLD 0
#define STEPPER1_ENABLE_INTERPLATION true
#endif
// uncomment to enable trinamic driver
//#define STEPPER2_HAS_TMC
#ifdef STEPPER2_HAS_TMC
#define STEPPER2_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER2_TMC_INTERFACE TMC_UART
#if (STEPPER2_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER2_UART_TX DOUT25
#define STEPPER2_UART_RX DIN26
#elif (STEPPER2_TMC_INTERFACE == TMC_SPI)
#define STEPPER2_SPI_DO DOUT13
#define STEPPER2_SPI_DI DIN13
#define STEPPER2_SPI_CLK DOUT14
#define STEPPER2_SPI_CS DOUT25
#endif
// basic parameters
#define STEPPER2_CURRENT_MA 800
#define STEPPER2_MICROSTEP 16
#define STEPPER2_RSENSE 0.11
#define STEPPER2_HOLD_MULT 0.7
#define STEPPER2_STEALTHCHOP_THERSHOLD 0
#define STEPPER2_ENABLE_INTERPLATION true
#endif
// uncomment to enable trinamic driver
//#define STEPPER3_HAS_TMC
#ifdef STEPPER3_HAS_TMC
#define STEPPER3_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER3_TMC_INTERFACE TMC_UART
#if (STEPPER3_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER3_UART_TX DOUT26
#define STEPPER3_UART_RX DIN27
#elif (STEPPER3_TMC_INTERFACE == TMC_SPI)
#define STEPPER3_SPI_DO DOUT13
#define STEPPER3_SPI_DI DIN13
#define STEPPER3_SPI_CLK DOUT14
#define STEPPER3_SPI_CS DOUT26
#endif
// basic parameters
#define STEPPER3_CURRENT_MA 800
#define STEPPER3_MICROSTEP 16
#define STEPPER3_RSENSE 0.11
#define STEPPER3_HOLD_MULT 0.7
#define STEPPER3_STEALTHCHOP_THERSHOLD 0
#define STEPPER3_ENABLE_INTERPLATION true
#endif
// uncomment to enable trinamic driver
//#define STEPPER4_HAS_TMC
#ifdef STEPPER4_HAS_TMC
#define STEPPER4_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER4_TMC_INTERFACE TMC_UART
#if (STEPPER4_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER4_UART_TX DOUT27
#define STEPPER4_UART_RX DIN28
#elif (STEPPER4_TMC_INTERFACE == TMC_SPI)
#define STEPPER4_SPI_DO DOUT13
#define STEPPER4_SPI_DI DIN13
#define STEPPER4_SPI_CLK DOUT14
#define STEPPER4_SPI_CS DOUT27
#endif
// basic parameters
#define STEPPER4_CURRENT_MA 800
#define STEPPER4_MICROSTEP 16
#define STEPPER4_RSENSE 0.11
#define STEPPER4_HOLD_MULT 0.7
#define STEPPER4_STEALTHCHOP_THERSHOLD 0
#define STEPPER4_ENABLE_INTERPLATION true
#endif
// uncomment to enable trinamic driver
//#define STEPPER5_HAS_TMC
#ifdef STEPPER5_HAS_TMC
#define STEPPER5_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER5_TMC_INTERFACE TMC_UART
#if (STEPPER5_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER5_UART_TX DOUT28
#define STEPPER5_UART_RX DIN29
#elif (STEPPER5_TMC_INTERFACE == TMC_SPI)
#define STEPPER5_SPI_DO DOUT13
#define STEPPER5_SPI_DI DIN13
#define STEPPER5_SPI_CLK DOUT14
#define STEPPER5_SPI_CS DOUT28
#endif
// basic parameters
#define STEPPER5_CURRENT_MA 800
#define STEPPER5_MICROSTEP 16
#define STEPPER5_RSENSE 0.11
#define STEPPER5_HOLD_MULT 0.7
#define STEPPER5_STEALTHCHOP_THERSHOLD 0
#define STEPPER5_ENABLE_INTERPLATION true
#endif
// uncomment to enable trinamic driver
//#define STEPPER6_HAS_TMC
#ifdef STEPPER6_HAS_TMC
#define STEPPER6_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER6_TMC_INTERFACE TMC_UART
#if (STEPPER6_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER6_UART_TX DOUT29
#define STEPPER6_UART_RX DIN30
#elif (STEPPER6_TMC_INTERFACE == TMC_SPI)
#define STEPPER6_SPI_DO DOUT13
#define STEPPER6_SPI_DI DIN13
#define STEPPER6_SPI_CLK DOUT14
#define STEPPER6_SPI_CS DOUT29
#endif
// basic parameters
#define STEPPER6_CURRENT_MA 800
#define STEPPER6_MICROSTEP 16
#define STEPPER6_RSENSE 0.11
#define STEPPER6_HOLD_MULT 0.7
#define STEPPER6_STEALTHCHOP_THERSHOLD 0
#define STEPPER6_ENABLE_INTERPLATION true
#endif
// uncomment to enable trinamic driver
//#define STEPPER7_HAS_TMC
#ifdef STEPPER7_HAS_TMC
#define STEPPER7_DRIVER_TYPE 2208
// choose the interface type
#define STEPPER7_TMC_INTERFACE TMC_UART
#if (STEPPER7_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#define STEPPER7_UART_TX DOUT30
#define STEPPER7_UART_RX DIN31
#elif (STEPPER7_TMC_INTERFACE == TMC_SPI)
#define STEPPER7_SPI_DO DOUT13
#define STEPPER7_SPI_DI DIN13
#define STEPPER7_SPI_CLK DOUT14
#define STEPPER7_SPI_CS DOUT30
#endif
// basic parameters
#define STEPPER7_CURRENT_MA 800
#define STEPPER7_MICROSTEP 16
#define STEPPER7_RSENSE 0.11
#define STEPPER7_HOLD_MULT 0.7
#define STEPPER7_STEALTHCHOP_THERSHOLD 0
#define STEPPER7_ENABLE_INTERPLATION true
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
#define STEPPER0_MSTEP0 DOUT23
#define STEPPER0_MSTEP1 DOUT15
#endif
// #define STEPPER1_HAS_MSTEP
#ifdef STEPPER1_HAS_MSTEP
#define STEPPER1_MSTEP 3
#define STEPPER1_MSTEP0 DOUT24
#define STEPPER1_MSTEP1 DOUT16
#endif
// #define STEPPER2_HAS_MSTEP
#ifdef STEPPER2_HAS_MSTEP
#define STEPPER2_MSTEP 3
#define STEPPER2_MSTEP0 DOUT25
#define STEPPER2_MSTEP1 DOUT17
#endif
// #define STEPPER3_HAS_MSTEP
#ifdef STEPPER3_HAS_MSTEP
#define STEPPER3_MSTEP 3
#define STEPPER3_MSTEP0 DOUT26
#define STEPPER3_MSTEP1 DOUT18
#endif
// #define STEPPER4_HAS_MSTEP
#ifdef STEPPER4_HAS_MSTEP
#define STEPPER4_MSTEP 3
#define STEPPER4_MSTEP0 DOUT27
#define STEPPER4_MSTEP1 DOUT19
#endif
// #define STEPPER5_HAS_MSTEP
#ifdef STEPPER5_HAS_MSTEP
#define STEPPER5_MSTEP 3
#define STEPPER5_MSTEP0 DOUT28
#define STEPPER5_MSTEP1 DOUT20
#endif
// #define STEPPER6_HAS_MSTEP
#ifdef STEPPER6_HAS_MSTEP
#define STEPPER6_MSTEP 3
#define STEPPER6_MSTEP0 DOUT29
#define STEPPER6_MSTEP1 DOUT21
#endif
// #define STEPPER7_HAS_MSTEP
#ifdef STEPPER7_HAS_MSTEP
#define STEPPER7_MSTEP 3
#define STEPPER7_MSTEP0 DOUT30
#define STEPPER7_MSTEP1 DOUT22
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
#define STEPPER_DIGIPOT_DO DOUT13
#define STEPPER_DIGIPOT_DI DIN13
#define STEPPER_DIGIPOT_CLK DOUT14
#define STEPPER_DIGIPOT_SS DOUT12
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

#ifdef __cplusplus
}
#endif

#endif