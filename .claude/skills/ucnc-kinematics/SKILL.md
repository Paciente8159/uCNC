---
name: ucnc-kinematics
description: Use for creating, modifying, porting, or diagnosing µCNC kinematics, including coordinate and step transforms, reachable-workspace checks, segmented nonlinear motion, homing strategies, kinematic settings, and compile-time registration under uCNC/src/hal/kinematics/.
disable-model-invocation: false
---

# µCNC kinematics work

Treat the current source, configuration, and tests as authoritative. The documents in
this skill explain design choices and established patterns; they do not override
`kinematic.h`, `kinematic.c`, the selected backend, or the compile-time dispatch.

## Route the task

Before editing kinematics code, read:

- `uCNC/src/hal/kinematics/README.md`
- `uCNC/src/hal/kinematics/kinematic.h`
- the affected backend and the nearest backend with the same mechanism

Then load only the material needed for the task:

- **New backend or substantial port:** read `BACKEND_GUIDE.md`.
- **Wrapper, skew, coordinate-transform, or position-reporting behavior:** read
  `uCNC/src/hal/kinematics/kinematic.c` and its callers.
- **Registration, axis/stepper count, or feature configuration:** inspect
  `kinematics.h`, `kinematicdefs.h`, `uCNC/cnc_config.h`, and
  `uCNC/src/cnc_hal_config_helper.h`. Derive identifiers and inclusion rules from
  those files rather than a cached list.
- **Settings or system-menu work:** inspect `grbl_settings.h`, `grbl_settings.c`,
  `defaults.h`, and a neighboring kinematic header.
- **Homing or boundary behavior:** inspect `motion_control.c`, the selected
  backend's `kinematics_home` and `kinematics_check_boundaries`, and the active
  limit/origin configuration.
- **Behavioral tests:** read `test/README.md` and the closest motion or homing
  fixture, such as `test_grbl_motion` or `test_domain3_h_homing`.
- **Kinematics terminology or documentation:** read `CONTEXT.md`.

## Contract anchors

Confirm these against the current source while working:

- Backends provide initialization, inverse and forward conversion, homing, and
  boundary behavior. The coordinate-to-steps and steps-to-coordinates wrappers live
  in `kinematic.c`.
- The wrapper owns copying a normal coordinate target, applying optional transforms
  and skew, and invoking inverse kinematics. The reverse wrapper applies forward
  kinematics before unskewing and the optional reverse transform.
- `kinematics_apply_transform` and `kinematics_apply_reverse_transform` have weak
  defaults; override them only when the selected backend needs an additional mapping.
- Boundary checks can be stateful: existing configurations may clamp a jog target or
  adjust a homing target. Preserve the selected backend's caller-visible behavior.
- Homing order, limit masks, pull-off timing, and position synchronization are
  kinematic-specific. Follow a comparable backend instead of applying a universal
  sequence.
- Nonlinear transforms need an explicit policy for unreachable points, singularities,
  rounding, overflow, and branch continuity.

## Safety and completion

Account for coordinate spaces at every boundary: G-code target, transformed machine
position, actuator steps, reported position, and homing reference. Check numerical
domains, step-width limits, soft-limit behavior, and segment size against motion
accuracy and planner cost.

Finish when every affected compile-time configuration is identified, a build actually
selects the changed kinematic, focused transform/boundary/homing behavior is verified,
and remaining hardware-only limit, coupling, timing, or calibration coverage is named
explicitly. Emulator tests do not establish hardware-in-the-loop safety.
