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
 * /


/*
	Spindle configurations.
	Uncomment to enable
*/
#ifdef USE_SPINDLE
#ifdef PWM0
#define SPINDLE_PWM PWM0
#endif
#ifdef DOUT0
#define SPINDLE_DIR DOUT0
#endif
#endif

/*
	Define a coolant flood and mist pin
*/
#ifdef USE_COOLANT
#ifdef DOUT1
#define COOLANT_FLOOD DOUT1
#endif
#ifdef DOUT2
#define COOLANT_MIST DOUT2
#endif
#endif

#ifdef DOUT15
#define LED DOUT15
#endif

#ifndef PID_CONTROLLERS
#define PID_CONTROLLERS 0
#endif

#ifndef ENCODERS
#define ENCODERS 0
#endif

#define DIN0 100
#define ENC0_PULSE DIN0

#ifdef __cplusplus
}
#endif

#endif