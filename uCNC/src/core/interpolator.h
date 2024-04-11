/*
	Name: interpolator.h
	Description: Function declarations for the stepper interpolator.

		TODO: Create an S-curve interpolator

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 13/10/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

// Itp update flags
#define ITP_NOUPDATE 0
#define ITP_UPDATE_ISR 1
#define ITP_UPDATE_TOOL 2
#define ITP_UPDATE (ITP_UPDATE_ISR | ITP_UPDATE_TOOL)
#define ITP_ACCEL 4
#define ITP_CONST 8
#define ITP_DEACCEL 16
#define ITP_SYNC 32
#define ITP_BACKLASH 64

#define ITP_STEP_MODE_STARTUP 0
#define ITP_STEP_MODE_DEFAULT 1
#define ITP_STEP_MODE_REALTIME 2
#define ITP_STEP_MODE_SYNC 4

	// contains data of the block being executed by the pulse routine
	// this block has the necessary data to execute the Bresenham line algorithm
	typedef struct itp_blk_
	{
#ifdef STEP_ISR_SKIP_MAIN
		uint8_t main_stepper;
#endif
#ifdef STEP_ISR_SKIP_IDLE
		uint8_t idle_axis;
#endif
		uint8_t dirbits;
		step_t steps[STEPPER_COUNT];
		step_t total_steps;
		step_t errors[STEPPER_COUNT];
#ifdef GCODE_PROCESS_LINE_NUMBERS
		uint32_t line;
#endif
	} itp_block_t;

	// contains data of the block segment being executed by the pulse and integrator routines
	// the segment is a fragment of the motion defined in the block
	// this also contains the acceleration/deacceleration info
	typedef struct pulse_sgm_
	{
		itp_block_t *block;
		uint16_t remaining_steps;
		uint16_t timer_counter;
		uint16_t timer_prescaller;
#if (DSS_MAX_OVERSAMPLING != 0)
		int8_t next_dss;
#endif
#if TOOL_COUNT > 0
		int16_t spindle;
#endif
		float feed;
		uint8_t flags;
	} itp_segment_t;

	void itp_init(void);
	void itp_run(void);
	void itp_update(void);
	void itp_stop(void);
	void itp_stop_tools(void);
	void itp_clear(void);
	void itp_get_rt_position(int32_t *position);
	void itp_sync_rt_position(int32_t *position);
	int32_t itp_get_rt_position_index(int8_t index);
	void itp_reset_rt_position(float *origin);
	float itp_get_rt_feed(void);
	bool itp_is_empty(void);
	uint8_t itp_sync(void);
	itp_segment_t *itp_get_rt_segment();
	uint8_t itp_set_step_mode(uint8_t mode);

	void itp_sync_spindle(void);
	void itp_start(bool is_synched);
#ifdef ENABLE_MULTI_STEP_HOMING
	void itp_lock_stepper(uint8_t lockmask);
#endif
#ifdef GCODE_PROCESS_LINE_NUMBERS
	uint32_t itp_get_rt_line_number(void);
#endif
#ifdef ENABLE_RT_SYNC_MOTIONS
	// extern volatile int32_t itp_sync_step_counter;
	void itp_update_feed(float feed);
	bool itp_sync_ready(void);
	DECL_HOOK(itp_rt_pre_stepbits, uint8_t *, uint8_t *);
	DECL_HOOK(itp_rt_stepbits, uint8_t, uint8_t);
#endif

#ifdef __cplusplus
}
#endif

#endif
