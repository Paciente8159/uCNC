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

#include <stdbool.h>
#include "config.h"

#ifdef LIMIT_X
#define LIMITEN_LIMIT_X (1<<(LIMIT_X - LIMIT_X))
#ifdef LIMIT_X_ISR
#define LIMITISR_LIMIT_X (1<<(LIMIT_X - LIMIT_X))
#else
#define LIMITISR_LIMIT_X 0
#endif
#else
#define LIMITEN_LIMIT_X 0
#define LIMITISR_LIMIT_X 0
#endif
#ifdef LIMIT_Y
#define LIMITEN_LIMIT_Y (1<<(LIMIT_Y - LIMIT_X))
#ifdef LIMIT_Y_ISR
#define LIMITISR_LIMIT_Y (1<<(LIMIT_Y - LIMIT_X))
#else
#define LIMITISR_LIMIT_Y 0
#endif
#else
#define LIMITEN_LIMIT_Y 0
#define LIMITISR_LIMIT_Y 0
#endif
#ifdef LIMIT_Z
#define LIMITEN_LIMIT_Z (1<<(LIMIT_Z - LIMIT_X))
#ifdef LIMIT_Z_ISR
#define LIMITISR_LIMIT_Z (1<<(LIMIT_Z - LIMIT_X))
#else
#define LIMITISR_LIMIT_Z 0
#endif
#else
#define LIMITEN_LIMIT_Z 0
#define LIMITISR_LIMIT_Z 0
#endif
#ifdef LIMIT_X2
#define LIMITEN_LIMIT_X2 (1<<(LIMIT_X2 - LIMIT_X))
#ifdef LIMIT_X2_ISR
#define LIMITISR_LIMIT_X2 (1<<(LIMIT_X2 - LIMIT_X))
#else
#define LIMITISR_LIMIT_X2 0
#endif
#else
#define LIMITEN_LIMIT_X2 0
#define LIMITISR_LIMIT_X2 0
#endif
#ifdef LIMIT_Y2
#define LIMITEN_LIMIT_Y2 (1<<(LIMIT_Y2 - LIMIT_X))
#ifdef LIMIT_Y2_ISR
#define LIMITISR_LIMIT_Y2 (1<<(LIMIT_Y2 - LIMIT_X))
#else
#define LIMITISR_LIMIT_Y2 0
#endif
#else
#define LIMITEN_LIMIT_Y2 0
#define LIMITISR_LIMIT_Y2 0
#endif
#ifdef LIMIT_Z2
#define LIMITEN_LIMIT_Z2 (1<<(LIMIT_Z2 - LIMIT_X))
#ifdef LIMIT_Z2_ISR
#define LIMITISR_LIMIT_Z2 (1<<(LIMIT_Z2 - LIMIT_X))
#else
#define LIMITISR_LIMIT_Z2 0
#endif
#else
#define LIMITEN_LIMIT_Z2 0
#define LIMITISR_LIMIT_Z2 0
#endif
#ifdef LIMIT_A
#define LIMITEN_LIMIT_A (1<<(LIMIT_A - LIMIT_X))
#ifdef LIMIT_A_ISR
#define LIMITISR_LIMIT_A (1<<(LIMIT_A - LIMIT_X))
#else
#define LIMITISR_LIMIT_A 0
#endif
#else
#define LIMITEN_LIMIT_A 0
#define LIMITISR_LIMIT_A 0
#endif
#ifdef LIMIT_B
#define LIMITEN_LIMIT_B (1<<(LIMIT_B - LIMIT_X))
#ifdef LIMIT_B_ISR
#define LIMITISR_LIMIT_B (1<<(LIMIT_B - LIMIT_X))
#else
#define LIMITISR_LIMIT_B 0
#endif
#else
#define LIMITEN_LIMIT_B 0
#define LIMITISR_LIMIT_B 0
#endif
#ifdef LIMIT_C
#define LIMITEN_LIMIT_C (1<<(LIMIT_C - LIMIT_X))
#ifdef LIMIT_C_ISR
#define LIMITISR_LIMIT_C (1<<(LIMIT_C - LIMIT_X))
#else
#define LIMITISR_LIMIT_C 0
#endif
#else
#define LIMITEN_LIMIT_C 0
#define LIMITISR_LIMIT_C 0
#endif
#ifdef PROBE
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
#ifdef ESTOP
#define CONTROLEN_ESTOP (1<<(ESTOP - ESTOP))
#ifdef ESTOP_ISR
#define CONTROLISR_ESTOP (1<<(ESTOP - ESTOP))
#else
#define CONTROLISR_ESTOP 0
#endif
#else
#define CONTROLEN_ESTOP 0
#define CONTROLISR_ESTOP 0
#endif
#ifdef SAFETY_DOOR
#define CONTROLEN_SAFETY_DOOR (1<<(SAFETY_DOOR - ESTOP))
#ifdef SAFETY_DOOR_ISR
#define CONTROLISR_SAFETY_DOOR (1<<(SAFETY_DOOR - ESTOP))
#else
#define CONTROLISR_SAFETY_DOOR 0
#endif
#else
#define CONTROLEN_SAFETY_DOOR 0
#define CONTROLISR_SAFETY_DOOR 0
#endif
#ifdef FHOLD
#define CONTROLEN_FHOLD (1<<(FHOLD - ESTOP))
#ifdef FHOLD_ISR
#define CONTROLISR_FHOLD (1<<(FHOLD - ESTOP))
#else
#define CONTROLISR_FHOLD 0
#endif
#else
#define CONTROLEN_FHOLD 0
#define CONTROLISR_FHOLD 0
#endif
#ifdef CS_RES
#define CONTROLEN_CS_RES (1<<(CS_RES - ESTOP))
#ifdef CS_RES_ISR
#define CONTROLISR_CS_RES (1<<(CS_RES - ESTOP))
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

//ISR
void io_limits_isr(void);
void io_controls_isr(void);
void io_probe_isr(void);

//inputs
bool io_check_boundaries(float *axis);
uint8_t io_get_limits(void);
uint8_t io_get_controls(void);
void io_enable_probe(void);
void io_disable_probe(void);
bool io_get_probe(void);
void io_set_homing_limits_filter(uint8_t filter_mask);

//outputs
void io_set_steps(uint8_t mask);
void io_toggle_steps(uint8_t mask);
void io_set_dirs(uint8_t mask);

void io_enable_steps(void);

#endif
