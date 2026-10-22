/**
 *       Name: interpolator.h
 *       Description: Function declarations for the stepper interpolator.
 *
 *               TODO: Create an S-curve interpolator
 *
 *       Copyright: Copyright (c) João Martins
 *       Author: João Martins
 *       Date: 13/10/2019
 *
 *       µCNC is free software: you can redistribute it and/or modify
 *       it under the terms of the GNU General Public License as published by
 *       the Free Software Foundation, either version 3 of the License, or
 *       (at your option) any later version. Please see
 *  <http://www.gnu.org/licenses/>
 *
 *       µCNC is distributed WITHOUT ANY WARRANTY;
 *       Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A
 *  PARTICULAR PURPOSE. See the	GNU General Public License for more details.
 */

#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

// sets the sample frequency for the Riemann sum integral
#ifndef INTERPOLATOR_FREQ
#define INTERPOLATOR_FREQ 100
#endif

  /**
   * Both structs below cross the IPC link as they are (see itp_push_blk and
   * itp_push_sgm), and the two sides of a split are separate builds from
   * different boardmaps. So a field may not be conditional on an option a
   * boardmap is free to set: the side that left it out would shift every field
   * after it on the other side of the link, and the frame would still be well
   * formed, so only the values in it would be wrong.
   *
   * DISABLE_ITP_STEP_GEN_OPTIMIZATIONS is one of those - the UNO boardmap
   * applies it, and it is worth having: it takes the work of stepping an idle
   * stepper out of the ISR. So what keeps the two sides in step is not the
   * fields being unconditional but the split turning the option off for both of
   * them, which cnc_hal_config_helper.h does for any IPC_MODE other than
   * IPC_SINGLE_CORE. The assertions below hold that in place.
   * */

  // contains data of the block being executed by the pulse routine
  // this block has the necessary data to execute the Bresenham line algorithm
  typedef struct itp_blk_
  {
    uint8_t dirbits;
    step_t steps[STEPPER_COUNT];
    step_t total_steps;
    step_t errors[STEPPER_COUNT];
#ifdef GCODE_PROCESS_LINE_NUMBERS
    uint32_t line;
#endif
#ifndef DISABLE_ITP_STEP_GEN_OPTIMIZATIONS
    uint8_t idle_steppers;
#endif
  } __attribute__((packed, aligned(1))) itp_block_t;

  // contains data of the block segment being executed by the pulse and
  // integrator routines the segment is a fragment of the motion defined in the
  // block this also contains the acceleration/deacceleration info
  typedef struct pulse_sgm_
  {
    uint16_t remaining_steps;
#ifndef DISABLE_ITP_STEP_GEN_OPTIMIZATIONS
    uint8_t main_stepper;
#endif
    uint16_t timer_counter;
    uint16_t timer_prescaller;
#if (DSS_MAX_OVERSAMPLING != 0)
    int8_t next_dss;
#ifdef ENABLE_RT_SYNC_MOTIONS
    uint8_t dss_level;
#endif
#endif
#if TOOL_COUNT > 0
    int16_t spindle;
#endif
    float feed;
    uint8_t flags;
    // kept last so that every field above has an architecture independent offset.
    // It is never transmitted over IPC (the RT side fills it in), and its size is
    // architecture dependent (2 bytes on AVR, 4 on 32bit MCUs)
    itp_block_t *block;
  } __attribute__((packed, aligned(1))) itp_segment_t;

  /**
   * What actually crosses the link is every field of the two structs above up to
   * the trailing pointer, which each side fills in for itself. Spelled out from
   * the motion configuration the two sides have to share anyway, so that a
   * build fails if a field of that part is ever made conditional on something
   * they are free to disagree on (see the note above the structs).
   *
   * Only a split build has a link, so only a split build has this to get wrong:
   * the check is that the two sides agree, which is not a question a single core
   * build has to answer. Scoped by the same test the note above relies on, so
   * that it applies exactly where that option is turned off.
   * */
#ifdef GCODE_PROCESS_LINE_NUMBERS
#define ITP_IPC_LINE_SIZE (sizeof(uint32_t))
#else
#define ITP_IPC_LINE_SIZE 0
#endif
#if (DSS_MAX_OVERSAMPLING != 0) && !defined(ENABLE_RT_SYNC_MOTIONS)
#define ITP_IPC_DSS_SIZE (sizeof(int8_t))
#elif (DSS_MAX_OVERSAMPLING != 0)
#define ITP_IPC_DSS_SIZE (sizeof(int8_t) + sizeof(uint8_t))
#else
#define ITP_IPC_DSS_SIZE 0
#endif
#if TOOL_COUNT > 0
#define ITP_IPC_SPINDLE_SIZE (sizeof(int16_t))
#else
#define ITP_IPC_SPINDLE_SIZE 0
#endif
// dirbits and idle_steppers, then steps, total_steps and errors
#define ITP_IPC_BLOCK_SIZE                                               \
  (sizeof(uint8_t) * 2 + (sizeof(step_t) * ((STEPPER_COUNT * 2) + 1)) + \
   ITP_IPC_LINE_SIZE)
// remaining_steps, timer_counter and timer_prescaller, then main_stepper,
// next_dss, spindle, feed and flags
#define ITP_IPC_SEGMENT_SIZE                                       \
  (sizeof(uint16_t) * 3 + sizeof(uint8_t) * 2 + ITP_IPC_DSS_SIZE + \
   ITP_IPC_SPINDLE_SIZE + sizeof(float))

#if (IPC_MODE != IPC_SINGLE_CORE)
  typedef char ipc_itp_block_layout_fits
      [(sizeof(itp_block_t) == ITP_IPC_BLOCK_SIZE) ? 1 : -1];
  typedef char ipc_itp_segment_layout_fits
      [(offsetof(itp_segment_t, block) == ITP_IPC_SEGMENT_SIZE) ? 1 : -1];
#endif

  /**
   * Payload of ITP_RESET_RT_POSITION.
   *
   * The machine coordinates are converted to steps by the NonRT side, so that
   * the RT side does not need step_per_mm (nor the kinematics) to reset its
   * position. The coordinates are still sent when steppers have encoders,
   * because the encoder positions are reset from them.
   * */
  typedef struct
  {
    int32_t steps[STEPPER_COUNT];
#if STEPPERS_ENCODERS_MASK != 0
    float origin[AXIS_COUNT];
#endif
  } __attribute__((packed, aligned(1))) itp_reset_rt_position_t;

  void itp_init(void);
  void itp_run(void);
  void itp_update(void);
  void itp_stop(void);
  void itp_stop_tools(void);
  void itp_clear(void);
  void itp_get_rt_position(int32_t *position);
  void itp_sync_rt_position(int32_t *position);
  int32_t itp_get_rt_position_index(int8_t index);
  // resets the RT step position from a machine coordinate origin. The conversion
  // to steps is done here (see itp_reset_rt_position_t)
  void itp_reset_rt_position(float *origin);
  // resets the RT step position from an already converted step position. Owned by
  // the RT side: called by itp_reset_rt_position when there is no IPC split, and
  // by the ITP_RESET_RT_POSITION request otherwise
  void itp_set_rt_position(int32_t *steps, float *origin);
  float itp_get_rt_feed(void);
  bool itp_is_empty(void);
  uint8_t itp_sync(void);
  itp_segment_t *itp_get_rt_segment();
  uint8_t itp_set_step_mode(uint8_t mode);
  // IPC split seam
  bool itp_is_full(void);
  void itp_push_blk(itp_block_t *blk);
  void itp_push_sgm(bool advance_block, itp_segment_t *sgm);
  void itp_step_rate_convert(float frequency, uint16_t *ticks,
                             uint16_t *prescaller);

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
#define ITP_BLOCK_CONTINUOUS \
  0                        // normal mode. Don't care about block ID. dispatch blocck as soon as
                           // available
#define ITP_BLOCK_SINGLE 1 // will run one single block at time.
#define ITP_BLOCK_BURST \
  2 // will run as many blocks as in queue.
    // #define ITP_BLOCK_READY 128
  // modifies the
  void itp_set_block_mode(uint8_t mode);
  void itp_inc_block_id(void);
  void itp_update_feed(float step_frequency);
  bool itp_sync_ready(void);
  // deprecate to make ISR leaner
  // DECL_HOOK(itp_rt_pre_stepbits, uint8_t *, uint8_t *);
  DECL_HOOK(itp_rt_stepbits, uint8_t, uint8_t);
#ifndef DISABLE_RT_STEP_PREVENT_CONDITION
  typedef bool (*itp_rt_step_prevent_t)(void);
  extern itp_rt_step_prevent_t itp_rt_step_prevent_cb;
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
