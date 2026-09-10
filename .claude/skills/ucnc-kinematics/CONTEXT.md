# µCNC Kinematics Backend Context

Terms used across the µCNC kinematics layer (`uCNC/src/hal/kinematics/`) and its governing document (`.claude/skills/ucnc-kinematics/BACKEND_GUIDE.md`). This glossary is the authoritative vocabulary; deviations in new backends or docs should be corrected to it.

## Layers

**Kinematics HAL**:
The interface contract between µCNC core and the machine kinematics: the `kinematics_*` function surface declared in `kinematic.h`, with weak defaults in `kinematic.c` for `kinematics_apply_transform` and `kinematics_apply_reverse_transform`.
_Avoid_: motion kernel, kinematics layer

**Kinematic backend**:
The per-kinematics implementation for one machine type (e.g. cartesian, corexy, linear delta, delta, scara, rtheta): a `kinematic_<type>.h` header (type string, settings vars, configuration flags) and a `kinematic_<type>.c` implementation unit. `BACKEND_GUIDE.md` defines the normative kinematics HAL behavior and API rules.
_Avoid_: kinematic driver, kinematics model

**Axis**:
A Cartesian world-coordinate dimension (X, Y, Z, A, B, C) indexed by `AXIS_X` through `AXIS_C`. The number of axes is set by `AXIS_COUNT` (default 3) in `kinematicdefs.h`.
_Avoid_: DOF, dimension

**Stepper / linear actuator**:
A physical motor driving one degree of freedom of the machine. `STEPPER_COUNT` may differ from `AXIS_COUNT` when `AXIS_TO_STEPPERS` is redefined (e.g. laser PPI requires an extra stepper).
_Avoid_: motor, axis motor

## Transform types

**Forward kinematics** (`kinematics_apply_forward`):
Converts step positions (`int32_t *steps`) to world coordinates (`float *axis`). For non-Cartesian kinematics this involves solving the geometric chain from actuator space to Cartesian space.
_Avoid_: FK

**Inverse kinematics** (`kinematics_apply_inverse`):
Converts world coordinates (`float *axis`) to step positions (`int32_t *steps`). For non-Cartesian kinematics this projects a Cartesian target onto actuator space.
_Avoid_: IK

**Coordinate-to-steps** (`kinematics_coordinates_to_steps`):
Wrapper in `kinematic.c` that applies the kinematic forward transform and skew compensation before calling `kinematics_apply_inverse`. Skips transforms during homing.

**Steps-to-coordinates** (`kinematics_steps_to_coordinates`):
Wrapper in `kinematic.c` that calls `kinematics_apply_forward` then reverses skew compensation and applies the reverse kinematic transform. Skips transforms during homing.

**Kinematic transform** (`kinematics_apply_transform` / `kinematics_apply_reverse_transform`):
Optional coordinate modifications applied before inverse and after forward transforms respectively. Weak defaults in `kinematic.c` are no-ops. Cartesian kinematics uses these for skew compensation (wired in the wrappers above); other kinematics may override for additional transformations.

## Homing

**Homing** (`kinematics_home`):
The axis calibration sequence. Axis order is defined by the implementation. Each axis call uses `mc_home_axis()` with the homing mask and the corresponding `LINACT*_LIMIT_MASK`. After homing, the kinematic posts the homing offset via `itp_reset_rt_position()` and `mc_sync_position()`.

**Homing mask** (`AXIS_X_HOMING_MASK`, `AXIS_Y_HOMING_MASK`, etc.):
Compile-time bitmask that may disable homing for an axis when set to 0.

**Pull-off** (`mc_home_motion_pulloff`):
The post-homing retract from the limit switch. Controlled by `ENABLE_GRBL_STYLE_HOMING`; when disabled, a pull-off always occurs.

## Backend construction macros

**`KINEMATIC_TYPE_STR`**:
A short string identifier for the kinematic type (e.g. `"C"` for cartesian, `"SC"` for scara). Defined in the kinematic header. Used for identification in protocol output.

**`KINEMATICS_MOTION_BY_SEGMENTS`**:
Flag that enables segment-by-segment motion for non-linear kinematics. When defined, linear G-code moves are subdivided into arc-like segments of `KINEMATICS_MOTION_SEGMENT_SIZE` to preserve accuracy on the nonlinear actuator path.

**`KINEMATICS_MOTION_SEGMENT_SIZE`**:
The maximum segment length in mm for motion-by-segments subdivision (default 1.0f). Overridable in the kinematic header.

**`KINEMATICS_VARS_DECL`**:
Macro declaring per-kinematics settings variables inside the `g_settings` struct. Each variable is a field with its C type (float, uint8_t, uint16_t, bool). Arrays are supported. Guarded by `#ifndef` so each kinematic defines its own.
_Example_: `float scara_arm_length; float scara_forearm_length;`

**`KINEMATICS_VARS_DEFAULTS_INIT`**:
Macro providing default values for each variable declared in `KINEMATICS_VARS_DECL`, using C designated initializer syntax. Guarded by `#ifndef`.
_Example_: `.scara_arm_length = DEFAULT_SCARA_ARM_LENGTH,`

**`KINEMATICS_VARS_SETTINGS_INIT`**:
Macro integrating each variable into the global settings `$` command system. Each entry is a `{.id, .memptr, .type}` struct. IDs map to `$` commands (e.g. `$28`, `$106`). Arrays use `SETTING_ARRAY | SETTING_ARRCNT(n)`. Guarded by `#ifndef`.
_Example_: `{.id = 106, .memptr = &g_settings.scara_arm_length, .type = SETTING_TYPE_FLOAT},`

**`KINEMATICS_VARS_SYSTEM_MENU_INIT`**:
Macro registering settings in the system menu for display-based editing. Each entry is a `DECL_MENU_VAR()` call targeting `SYSTEM_MENU_ID_KINEMATIC_SETTINGS` or another menu screen. Not guarded by default (commented out in `kinematicdefs.h`).

## Registration

**Kinematic ID**:
A unique numeric constant defined in `kinematics.h` (e.g. `KINEMATIC_CARTESIAN 1`, `KINEMATIC_SCARA 5`). The user selects the kinematic by defining `KINEMATIC` to one of these IDs.

**`kinematicdefs.h` include chain**:
The compile-time dispatch header. Given `KINEMATIC`, it `#include`s the correct `kinematic_<type>.h` header, then unconditionally includes `kinematic.h`. A new kinematic needs a `#elif` branch here.

**Axis index** (`AXIS_X`, `AXIS_Y`, ...):
Defined in `kinematicdefs.h` from `AXIS_COUNT`. Each axis from X (0) through C (5) gets a constant only when `AXIS_COUNT` exceeds its position.