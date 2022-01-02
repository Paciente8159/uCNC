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

/**
	Tool definition
	For any given tool the respective macro TOOLx (x from 1 to 16) must be created
*/

//declare the tool to be used
extern const tool_t __rom__ spindle1;
//extern const tool_t __rom__ laser1;

//assign the tools from 1 to 16
#define TOOL1 spindle1
//#define TOOL2 laser1

#if DOUT15 >= 0
#define LED DOUT15
#endif

#ifndef PID_CONTROLLERS
#define PID_CONTROLLERS 0
#endif

#ifndef ENCODERS
#define ENCODERS 0
#endif

/**
 * To use the PID controller 2 definitions are needed
 * PIDx_DELTA() -> sets the function that gets the error between the setpoint and the current value for x PID controller
 * PIDx_OUTPUT(X) -> sets the output after calculating the pid corrected value for x PID controller
 * 
 * For example
 * 
 * #define PID0_DELTA() (my_setpoint - mcu_get_analog(ANA0))
 * #define PID0_OUTPUT(X) (mcu_set_pwm(PWM0, X))
 * 
 * An optional configuration is the sampling rate of the PID update. By default the sampling rate is 125Hz.
 * To reduce the sampling rate a 125/PIDx_FREQ_DIV can be defined between 1 (125Hz) and 250 (0.5Hz)
 * 
 * You can but you should not define PID for tools. Tools have a dedicated PID that can be customized for each tool. Check the tool HAL for this.
 * 
 * */
	//here is an example on how to add an PID controller to the spindle
	//this exemple assumes that the spindle speed is feedback via an analog pin
	//reference to io_get_spindle defined in io_control
	// 	extern uint8_t io_get_spindle(void);
	// #define SPINDLE_SPEED ANALOG0
	// #define PID0_DELTA() (io_get_spindle() - mcu_get_analog(SPINDLE_SPEED))
	// #define PID0_OUTPUT(X) (mcu_set_pwm(SPINDLE_PWM, X))
	// //optional
	// #define PID0_FREQ_DIV 50

	/**
 * To use the encoder counter 2 definitions are needed
 * ENC0_PULSE -> must be set to an input PIN with interrupt on change enabled capabilities
 * ENC0_DIR -> a regular input PIN that detects the direction of the encoding step
 * */
	//#define ENC0_PULSE DIN0
	//#define ENC0_DIR DIN8

#ifdef __cplusplus
}
#endif

#endif