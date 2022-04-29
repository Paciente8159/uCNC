/*
	Name: io_control.h
	Description: The input control unit for µCNC.
		This is responsible to check all limit switches (both hardware and software), control switches,
		and probe.

		TODO:
			-implement generic inputs
			-implement outputs

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07/12/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef DIGITAL_IO_CONTROL_H
#define DIGITAL_IO_CONTROL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

#if !(LIMIT_X < 0)
#define LIMITEN_LIMIT_X (1 << 0)
#ifdef LIMIT_X_ISR
#define LIMITISR_LIMIT_X (1 << 0)
#else
#define LIMITISR_LIMIT_X 0
#endif
#else
#define LIMITEN_LIMIT_X 0
#define LIMITISR_LIMIT_X 0
#endif
#if !(LIMIT_Y < 0)
#define LIMITEN_LIMIT_Y (1 << 1)
#ifdef LIMIT_Y_ISR
#define LIMITISR_LIMIT_Y (1 << 1)
#else
#define LIMITISR_LIMIT_Y 0
#endif
#else
#define LIMITEN_LIMIT_Y 0
#define LIMITISR_LIMIT_Y 0
#endif
#if !(LIMIT_Z < 0)
#define LIMITEN_LIMIT_Z (1 << 2)
#ifdef LIMIT_Z_ISR
#define LIMITISR_LIMIT_Z (1 << 2)
#else
#define LIMITISR_LIMIT_Z 0
#endif
#else
#define LIMITEN_LIMIT_Z 0
#define LIMITISR_LIMIT_Z 0
#endif
#if !(LIMIT_X2 < 0)
#define LIMITEN_LIMIT_X2 (1 << 3)
#ifdef LIMIT_X2_ISR
#define LIMITISR_LIMIT_X2 (1 << 3)
#else
#define LIMITISR_LIMIT_X2 0
#endif
#else
#define LIMITEN_LIMIT_X2 0
#define LIMITISR_LIMIT_X2 0
#endif
#if !(LIMIT_Y2 < 0)
#define LIMITEN_LIMIT_Y2 (1 << 4)
#ifdef LIMIT_Y2_ISR
#define LIMITISR_LIMIT_Y2 (1 << 4)
#else
#define LIMITISR_LIMIT_Y2 0
#endif
#else
#define LIMITEN_LIMIT_Y2 0
#define LIMITISR_LIMIT_Y2 0
#endif
#if !(LIMIT_Z2 < 0)
#define LIMITEN_LIMIT_Z2 (1 << 5)
#ifdef LIMIT_Z2_ISR
#define LIMITISR_LIMIT_Z2 (1 << 5)
#else
#define LIMITISR_LIMIT_Z2 0
#endif
#else
#define LIMITEN_LIMIT_Z2 0
#define LIMITISR_LIMIT_Z2 0
#endif
#if !(LIMIT_A < 0)
#define LIMITEN_LIMIT_A (1 << 6)
#ifdef LIMIT_A_ISR
#define LIMITISR_LIMIT_A (1 << 6)
#else
#define LIMITISR_LIMIT_A 0
#endif
#else
#define LIMITEN_LIMIT_A 0
#define LIMITISR_LIMIT_A 0
#endif
#if !(LIMIT_B < 0)
#define LIMITEN_LIMIT_B (1 << 7)
#ifdef LIMIT_B_ISR
#define LIMITISR_LIMIT_B (1 << 7)
#else
#define LIMITISR_LIMIT_B 0
#endif
#else
#define LIMITEN_LIMIT_B 0
#define LIMITISR_LIMIT_B 0
#endif
#if !(LIMIT_C < 0)
#define LIMITEN_LIMIT_C (1 << 8)
#ifdef LIMIT_C_ISR
#define LIMITISR_LIMIT_C (1 << 8)
#else
#define LIMITISR_LIMIT_C 0
#endif
#else
#define LIMITEN_LIMIT_C 0
#define LIMITISR_LIMIT_C 0
#endif
#if !(PROBE < 0)
#define PROBEEN_MASK 1
#ifdef PROBE_ISR
#define PROBEISR_MASK 1
#else
#define PROBEISR_MASK 0
#endif
#else
#define PROBEEN_MASK 0
#define PROBEISR_MASK 0
#endif
#if !(ESTOP < 0)
#define CONTROLEN_ESTOP (1 << 0)
#ifdef ESTOP_ISR
#define CONTROLISR_ESTOP (1 << 0)
#else
#define CONTROLISR_ESTOP 0
#endif
#else
#define CONTROLEN_ESTOP 0
#define CONTROLISR_ESTOP 0
#endif
#if !(SAFETY_DOOR < 0)
#define CONTROLEN_SAFETY_DOOR (1 << 1)
#ifdef SAFETY_DOOR_ISR
#define CONTROLISR_SAFETY_DOOR (1 << 1)
#else
#define CONTROLISR_SAFETY_DOOR 0
#endif
#else
#define CONTROLEN_SAFETY_DOOR 0
#define CONTROLISR_SAFETY_DOOR 0
#endif
#if !(FHOLD < 0)
#define CONTROLEN_FHOLD (1 << 2)
#ifdef FHOLD_ISR
#define CONTROLISR_FHOLD (1 << 2)
#else
#define CONTROLISR_FHOLD 0
#endif
#else
#define CONTROLEN_FHOLD 0
#define CONTROLISR_FHOLD 0
#endif
#if !(CS_RES < 0)
#define CONTROLEN_CS_RES (1 << 3)
#ifdef CS_RES_ISR
#define CONTROLISR_CS_RES (1 << 3)
#else
#define CONTROLISR_CS_RES 0
#endif
#else
#define CONTROLEN_CS_RES 0
#define CONTROLISR_CS_RES 0
#endif

#define LIMITEN_MASK (LIMITEN_LIMIT_X | LIMITEN_LIMIT_Y | LIMITEN_LIMIT_Z | LIMITEN_LIMIT_X2 | LIMITEN_LIMIT_Y2 | LIMITEN_LIMIT_Z2 | LIMITEN_LIMIT_A | LIMITEN_LIMIT_B | LIMITEN_LIMIT_C)
#define LIMITISR_MASK (LIMITISR_LIMIT_X | LIMITISR_LIMIT_Y | LIMITISR_LIMIT_Z | LIMITISR_LIMIT_X2 | LIMITISR_LIMIT_Y2 | LIMITISR_LIMIT_Z2 | LIMITISR_LIMIT_A | LIMITISR_LIMIT_B | LIMITISR_LIMIT_C)

#define CONTROLEN_MASK (CONTROLEN_ESTOP | CONTROLEN_SAFETY_DOOR | CONTROLEN_FHOLD | CONTROLEN_CS_RES)
#define CONTROLISR_MASK (CONTROLISR_ESTOP | CONTROLISR_SAFETY_DOOR | CONTROLISR_FHOLD | CONTROLISR_CS_RES)

	// inputs
	uint8_t io_get_limits(void);
	uint8_t io_get_controls(void);
	void io_enable_probe(void);
	void io_disable_probe(void);
	bool io_get_probe(void);
	void io_set_homing_limits_filter(uint8_t filter_mask);

	uint8_t io_get_analog(uint8_t pin);

	// outputs
	void io_set_steps(uint8_t mask);
	void io_toggle_steps(uint8_t mask);
	void io_set_dirs(uint8_t mask);

	void io_set_pwm(uint8_t pin, uint8_t value);
	void io_set_output(uint8_t pin, bool state);

	void io_enable_steppers(uint8_t mask);

	int16_t io_get_pinvalue(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif
