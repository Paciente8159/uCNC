# µCNC Kinematics Backend Authoring Guide

This document combines the normative µCNC **Kinematics HAL contract** with recommended
backend-authoring patterns. The keywords below distinguish the two:

- **Required**: observable behavior or API/ABI compatibility that every kinematic backend
  must preserve. A deviation is a contract change and requires communication with the project.
- **Recommended**: the common implementation or layout. A backend may deviate
  when its machine geometry requires it and should document the reason.
- **Example**: an observed implementation, not a requirement.

The function signatures in `kinematic.h` are normative; implementation examples in this
document remain examples. Directory layout, section ordering, and implementation techniques
are authoring guidance unless explicitly labeled **Required**.

Reference backends: `kinematic_cartesian`, `kinematic_corexy`, `kinematic_linear_delta`,
`kinematic_delta`, `kinematic_scara`, `kinematic_rtheta`, `kinematic_dummy`.

It is written for kinematics backend authors. Terminology is pinned in
`.claude/skills/ucnc-kinematics/CONTEXT.md`; contract-level decisions live in `docs/adr/`.

## Authoring workflow

1. **Understand the machine geometry.** Draw the kinematic chain: how many degrees of
   freedom, which axes map to which actuators, what transformations are needed between
   Cartesian space and actuator space. Identify whether the kinematics is linear
   (Cartesian, CoreXY) or non-linear (delta, SCARA, polar), and whether motion-by-segments
   is needed.

2. **Choose implementation boundaries.** Decide which files to create following §1.2.
   Identify which of the seven function slots need real work vs. defaults/no-ops.
   Plan custom settings variables and their `$` ID range.

3. **Prove the vertical slice.** Write `kinematics_init`, `kinematics_apply_inverse`,
   `kinematics_apply_forward`, and `kinematics_home`. Verify with a known target →
   steps → coordinates round-trip.

4. **Add boundary checking and transforms.** Implement `kinematics_check_boundaries`
   and any custom `kinematics_apply_transform` / `kinematics_apply_reverse_transform`.

5. **Register and validate.** Add the kinematic ID to `kinematics.h`, add the include
   branch to `kinematicdefs.h`, add default values to `defaults.h` if needed, configure a
   board to use the new kinematic, build, and test.

## 1. Big picture: where the kinematic backend fits

The kinematics layer sits between the motion planner and the stepper control. Its
responsibilities are:

- **Coordinate transform**: convert Cartesian world coordinates to actuator step positions
  (inverse kinematics) and back (forward kinematics).
- **Homing**: define the sequence and strategy for calibrating axes against limit switches.
- **Boundary checking**: enforce software limits in the machine's natural coordinate space.
- **Coordinate correction**: apply optional transforms (skew compensation, tool offsets)
  before inverse kinematics and after forward kinematics.

The controller calls `kinematics_coordinates_to_steps()` and
`kinematics_steps_to_coordinates()` — these wrappers in `kinematic.c` apply skew
compensation and the optional transform functions around the core forward/inverse pair.
New backends implement the core functions and let the wrappers compose them.

### 1.1 Layers

```
┌─────────────────────────────────────────────────────┐
│                   Motion control                      │
│  (mc_line, mc_arc, mc_home_axis, mc_sync_position)   │
├─────────────────────────────────────────────────────┤
│         Kinematics wrappers (kinematic.c)             │
│  coordinates_to_steps  │  steps_to_coordinates        │
│  (skew + transform)    │  (unskew + reverse)          │
├────────────┬────────────┴────────────┬───────────────┤
│ inverse    │ forward                 │ home          │
│ kinematics │ kinematics              │ strategy      │
├────────────┴────────────────────────┴───────────────┤
│              Per-kinematic backend                     │
│  (kinematic_<type>.c + kinematic_<type>.h)             │
└─────────────────────────────────────────────────────┘
```

### 1.2 File/directory layout

A kinematic backend consists of two files:

```
uCNC/src/hal/kinematics/
├── kinematic.h                  # Interface contract (shared, do not modify casually)
├── kinematics.h                 # Kinematic type IDs (add new ID here)
├── kinematicdefs.h              # Axis definitions + compile-time dispatch (add branch here)
├── kinematic.c                  # Wrappers + weak defaults (shared)
├── kinematic_cartesian.h        # Per-kinematic header
├── kinematic_cartesian.c        # Per-kinematic implementation
├── kinematic_corexy.h
├── kinematic_corexy.c
├── kinematic_scara.h
├── kinematic_scara.c
├── kinematic_delta.h
├── kinematic_delta.c
├── kinematic_linear_delta.h
├── kinematic_linear_delta.c
├── kinematic_rtheta.h
├── kinematic_rtheta.c
├── kinematic_dummy.h
├── kinematic_dummy.c
└── uCNC kinematics.pdf          # Reference document
```

A new backend adds `kinematic_<type>.h` and `kinematic_<type>.c`.

## 2. Implementation contract

Every kinematic backend must implement all functions declared in `kinematic.h`.
The two transform functions have weak no-op defaults in `kinematic.c` and may be
overridden when needed.

### 2.1 `kinematics_init`

**Signature**: `void kinematics_init(void)`

**Required**: Initializes the kinematic system. Must compute any derived geometry
constants (arm lengths, angle factors, radii, distance limits) from settings variables,
and call `mc_sync_position()` to synchronize the planner position with the kinematic state.

**Recommended pattern**: Read per-kinematic settings from `g_settings`, precompute
derived values to avoid recomputation at runtime, then sync.

*Example (SCARA)*:
```c
void kinematics_init(void) {
  scara_arm_angle_fact[0] = 2.0f * M_PI / g_settings.step_per_mm[0];
  scara_arm_angle_fact[1] = 2.0f * M_PI / g_settings.step_per_mm[1];
  scara_max_distance_to_center_sqr =
      g_settings.scara_arm_length + g_settings.scara_forearm_length;
  scara_max_distance_to_center_sqr *= scara_max_distance_to_center_sqr;
  scara_min_distance_to_center_sqr =
      g_settings.scara_arm_length - g_settings.scara_forearm_length;
  scara_min_distance_to_center_sqr *= scara_min_distance_to_center_sqr;
  mc_sync_position();
}
```

### 2.2 `kinematics_apply_inverse`

**Signature**: `void kinematics_apply_inverse(float *axis, int32_t *steps)`

**Required**: Converts Cartesian world coordinates (`axis[AXIS_X]` .. `axis[AXIS_C]`)
to actuator step positions (`steps[0]` .. `steps[STEPPER_COUNT-1]`).

- Input `axis` values are in mm (or degrees for rotary axes).
- Output `steps` values are integer step counts.
- For linear (Cartesian/CoreXY) backends, this is a simple scaling by
  `g_settings.step_per_mm[i]` per axis.
- For non-linear backends, this involves solving the inverse kinematic equations.
- **Must not** modify the `axis` array (the caller owns it).

*Example (Cartesian)*: not needed — the default is handled by `kinematic_cartesian.c`.

### 2.3 `kinematics_apply_forward`

**Signature**: `void kinematics_apply_forward(int32_t *steps, float *axis)`

**Required**: Converts actuator step positions back to Cartesian world coordinates.
The inverse of `kinematics_apply_inverse`.

- For linear backends: `axis[i] = steps[i] / g_settings.step_per_mm[i]`.
- For non-linear backends: solve the forward kinematic chain.
- **Must not** modify the `steps` array.

### 2.4 `kinematics_home`

**Signature**: `uint8_t kinematics_home(void)`

**Required**: Executes the homing sequence. Defines the order in which axes home and
any custom homing motions.

**Homning pattern**:
1. Home each axis group using `mc_home_axis(mask, limit_mask)` in the desired order.
2. After all axes are homed, compute the homing position in steps and call
   `kinematics_apply_forward(steps_homing, target)` to get the Cartesian homing position.
3. Call `itp_reset_rt_position(target)` to set the reference position.
4. Call `mc_sync_position()` to synchronize the planner.
5. Unless `ENABLE_GRBL_STYLE_HOMING` is set, call `mc_home_motion_pulloff(255, true)`.

**Returns**: `STATUS_OK` on success, or an error code on failure (axis home error).

*Homing error flags*: `KINEMATIC_HOMING_ERROR_X` (1), `KINEMATIC_HOMING_ERROR_Y` (2),
`KINEMATIC_HOMING_ERROR_XY` (3), `KINEMATIC_HOMING_ERROR_Z` (4),
`KINEMATIC_HOMING_ERROR_A` (8), `KINEMATIC_HOMING_ERROR_B` (16),
`KINEMATIC_HOMING_ERROR_C` (32).

*Recommended homing order*: Z first (retract probe), then X/Y (or the primary planar
axes), then A/B/C (rotary axes). Guard each axis group with `#if AXIS_<n>_HOMING_MASK != 0`
so the homing sequence is compilable with fewer axes.

### 2.5 `kinematics_apply_transform`

**Signature**: `void kinematics_apply_transform(float *axis)`

**Default**: Weak no-op in `kinematic.c`.

**Required only when**: the kinematic needs to apply a coordinate transformation
before inverse kinematics (e.g., tool length offset, custom mapping).
Called by `kinematics_coordinates_to_steps` for normal (non-homing) moves.

### 2.6 `kinematics_apply_reverse_transform`

**Signature**: `void kinematics_apply_reverse_transform(float *axis)`

**Default**: Weak no-op in `kinematic.c`.

**Required only when**: the kinematic needs to reverse the transform applied by
`kinematics_apply_transform`. Called by `kinematics_steps_to_coordinates` for
normal (non-homing) moves.

### 2.7 `kinematics_coordinates_to_steps`

**Signature**: `void kinematics_coordinates_to_steps(float *axis, int32_t *steps)`

**Note**: Implemented in `kinematic.c`. Not overridden by backends. This wrapper:
1. Copies the axis array (to avoid modifying the caller's data).
2. During non-homing: calls `kinematics_apply_transform`, then applies skew compensation
   (if `ENABLE_SKEW_COMPENSATION` is defined).
3. Calls `kinematics_apply_inverse`.

### 2.8 `kinematics_steps_to_coordinates`

**Signature**: `void kinematics_steps_to_coordinates(int32_t *steps, float *axis)`

**Note**: Implemented in `kinematic.c`. Not overridden by backends. This wrapper:
1. Calls `kinematics_apply_forward`.
2. During non-homing: reverses skew compensation, then calls
   `kinematics_apply_reverse_transform`.

### 2.9 `kinematics_check_boundaries`

**Signature**: `bool kinematics_check_boundaries(float *axis)`

**Required**: Checks whether the target position is inside software limits.

- Return `true` if within boundaries, `false` if outside.
- If soft limits are disabled (`!g_settings.soft_limits_enabled`), return `true`.
- During homing (`cnc_get_exec_state(EXEC_HOMING)`), return `true`.
- **Recommended**: Use the natural constraint space of the kinematic. For non-Cartesian
  kinematics, check radial distance, angular range, or reachable workspace.

*Example (SCARA)*:
```c
bool kinematics_check_boundaries(float *axis) {
  if (!g_settings.soft_limits_enabled || cnc_get_exec_state(EXEC_HOMING)) {
    return true;
  }
  float distance_to_center_sqr =
      axis[AXIS_X] * axis[AXIS_X] + axis[AXIS_Y] * axis[AXIS_Y];
  if (distance_to_center_sqr < scara_min_distance_to_center_sqr ||
      distance_to_center_sqr > scara_max_distance_to_center_sqr) {
    return false;
  }
  // remaining axes: check against g_settings.max_distance[i]
  for (uint8_t i = AXIS_COUNT; i != 2;) {
    i--;
    if (g_settings.max_distance[i]) {
      float value = axis[i];
      // apply homing direction inversion if SET_ORIGIN_AT_HOME_POS
      if (value > g_settings.max_distance[i] || value < 0) {
        return false;
      }
    }
  }
  return true;
}
```

## 3. Settings variables integration

Kinematic-specific settings are declared via three macros in the `kinematic_<type>.h`
header, and optionally a fourth for the system menu.

### 3.1 `KINEMATICS_VARS_DECL`

Declares fields inside the `g_settings` struct (see `grbl_settings.h:89`).

**Required format**: Each variable is a complete field declaration.

```c
#define KINEMATICS_VARS_DECL          \
  float my_kin_var_float;            \
  uint8_t my_kin_var_uint8;          \
  float my_kin_var_array[3];
```

### 3.2 `KINEMATICS_VARS_DEFAULTS_INIT`

Initializes fields with C designated initializer syntax. Reference default values
from macros defined in `defaults.h`.

```c
#define KINEMATICS_VARS_DEFAULTS_INIT                    \
  .my_kin_var_float = DEFAULT_MY_KIN_VAR_FLOAT,         \
  .my_kin_var_uint8 = DEFAULT_MY_KIN_VAR_UINT8,         \
  .my_kin_var_array = {0, 1, 2},
```

Add the corresponding `DEFAULT_*` macros in `uCNC/src/interface/defaults.h`:

```c
#ifndef DEFAULT_MY_KIN_VAR_FLOAT
#define DEFAULT_MY_KIN_VAR_FLOAT 1.5f
#endif
#ifndef DEFAULT_MY_KIN_VAR_UINT8
#define DEFAULT_MY_KIN_VAR_UINT8 3
#endif
```

### 3.3 `KINEMATICS_VARS_SETTINGS_INIT`

Integrates variables into the `$` command settings system. Each entry maps a
setting ID to a variable in `g_settings`.

**Available setting types**:
- `SETTING_TYPE_FLOAT` — 32-bit float
- `SETTING_TYPE_UINT8` — unsigned 8-bit
- `SETTING_TYPE_UINT16` — unsigned 16-bit
- `SETTING_TYPE_BOOL` — boolean
- `SETTING_ARRAY | SETTING_ARRCNT(n)` — array flag, combined with base type

**ID convention**:
- IDs 28-29: homing parameters (angles, distances)
- IDs 106+: kinematic geometry parameters (lengths, radii, ratios)
- See existing kinematics for the allocated ID range.

```c
#define KINEMATICS_VARS_SETTINGS_INIT                       \
  {.id = 28, .memptr = &g_settings.my_kin_var_float,       \
   .type = SETTING_TYPE_FLOAT},                             \
  {.id = 29, .memptr = &g_settings.my_kin_var_uint8,       \
   .type = SETTING_TYPE_UINT8},
```

### 3.4 `KINEMATICS_VARS_SYSTEM_MENU_INIT`

Optional. Registers variables in the system menu for display-based editing.

```c
#define KINEMATICS_VARS_SYSTEM_MENU_INIT                                    \
  DECL_MENU_VAR(SYSTEM_MENU_ID_HOMING, s28, STR_HOME_ANG,                  \
                &g_settings.my_kin_var_float, VAR_TYPE_FLOAT);              \
  DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s106, STR_KIN_PARAM,    \
                &g_settings.my_kin_var_array[0], VAR_TYPE_FLOAT);
```

Define string constants (`STR_*`) in the kinematic header for display labels.

## 4. Axis and stepper configuration

### 4.1 `AXIS_COUNT`

Default: 3 (X, Y, Z). Can be overridden by the kinematic header or user config.

Range: 0 to 6 (X, Y, Z, A, B, C).

Set in the kinematic header only when the kinematic requires a specific minimum:

```c
#if AXIS_COUNT < 3
#error "MyKinematic expects at least 3 axes"
#endif
```

### 4.2 `AXIS_TO_STEPPERS` and `STEPPER_COUNT`

By default `STEPPER_COUNT` = `AXIS_TO_STEPPERS` = `AXIS_COUNT`. Override
`AXIS_TO_STEPPERS` when extra actuators are needed (e.g., laser PPI requires
an additional stepper).

### 4.3 Axis indices

Defined in `kinematicdefs.h`:

| Axis | Index | Constant |
|------|-------|----------|
| X    | 0     | `AXIS_X` |
| Y    | 1     | `AXIS_Y` |
| Z    | 2     | `AXIS_Z` |
| A    | 3     | `AXIS_A` |
| B    | 4     | `AXIS_B` |
| C    | 5     | `AXIS_C` |

Only defined when `AXIS_COUNT > N`.

## 5. Motion-by-segments

Non-linear kinematics (delta, SCARA, RTheta) require motion to be subdivided into
small linear segments to preserve accuracy. Enable by adding to the kinematic header:

```c
#define KINEMATICS_MOTION_BY_SEGMENTS
#define KINEMATICS_MOTION_SEGMENT_SIZE 1.0f  // override default if needed
```

When defined, the motion control layer (`motion_control.c`) breaks linear G-code
moves into segments of at most `KINEMATICS_MOTION_SEGMENT_SIZE` mm. Each segment
is planned as an independent motion.

**Recommended**: Define for any kinematic where `kinematics_apply_inverse` is
non-linear (i.e., the actuator position is not a linear function of Cartesian position).

## 6. Kinematic type identification

Every kinematic header must define:

```c
#define KINEMATIC_TYPE_STR "XX"
```

This string identifies the kinematic type in protocol output. Use 1-3 characters:

| Kinematic | Type string |
|-----------|-------------|
| Cartesian | `"C"` |
| CoreXY    | `"CXY"` |
| Linear Delta | `"LD"` |
| Delta     | `"D"` |
| SCARA     | `"SC"` |
| RTheta    | `"RT"` |
| Dummy     | `"DMY"` |

## 7. Additional kinematic identification flags

When needed for compile-time dispatch, define a flag in the header:

```c
#define IS_MYKINEMATIC_KINEMATICS
```

This allows other subsystems (motion control, planner) to conditionally adapt
behavior. Examples: `IS_SCARA_KINEMATICS`, `IS_DELTA_KINEMATICS`,
`IS_RTHETA_KINEMATICS`.

## 8. Registration checklist

To add a new kinematic type, complete all steps:

- [ ] **Add kinematic ID** in `uCNC/src/hal/kinematics/kinematics.h`:
      ```c
      #define KINEMATIC_MYTYPE 6
      ```
- [ ] **Create kinematic header** `uCNC/src/hal/kinematics/kinematic_mytype.h`:
      - `KINEMATIC_TYPE_STR`
      - `KINEMATICS_VARS_DECL`, `KINEMATICS_VARS_DEFAULTS_INIT`,
        `KINEMATICS_VARS_SETTINGS_INIT`
      - Optionally `KINEMATICS_VARS_SYSTEM_MENU_INIT`
      - Optionally `KINEMATICS_MOTION_BY_SEGMENTS` and `KINEMATICS_MOTION_SEGMENT_SIZE`
      - Optionally `IS_MYTYPE_KINEMATICS` flag
- [ ] **Create kinematic implementation** `uCNC/src/hal/kinematics/kinematic_mytype.c`:
      - Implement all 7 functions from `kinematic.h` (the transform pair may be no-ops)
      - Guard with `#if (KINEMATIC == KINEMATIC_MYTYPE)`
- [ ] **Add include branch** in `uCNC/src/hal/kinematics/kinematicdefs.h`:
      ```c
      #elif (KINEMATIC == KINEMATIC_MYTYPE)
      #include "kinematic_mytype.h"
      ```
- [ ] **Add default values** in `uCNC/src/interface/defaults.h`:
      ```c
      #if (!defined(DEFAULT_MY_KIN_VAR))
      #define DEFAULT_MY_KIN_VAR 100
      #endif
      ```
- [ ] **Build and test**: configure a board to use the new kinematic, build for the
      emulator and at least one hardware target, verify forward/inverse round-trip.

## 9. Implementation file structure

The `.c` file should follow this structure:

```c
#include "../../cnc.h"

#if (KINEMATIC == KINEMATIC_MYTYPE)

#include <math.h>   // if needed

// Static variables for derived geometry
static float my_derived_constant;

void kinematics_init(void) {
  // Compute derived values from settings
  mc_sync_position();
}

void kinematics_apply_inverse(float *axis, int32_t *steps) {
  // Inverse kinematics equations
}

void kinematics_apply_forward(int32_t *steps, float *axis) {
  // Forward kinematics equations
}

uint8_t kinematics_home(void) {
  // Homing sequence
  // ...
  return STATUS_OK;
}

void kinematics_apply_transform(float *axis) {
  // Optional: coordinate transform before inverse
}

void kinematics_apply_reverse_transform(float *axis) {
  // Optional: reverse transform after forward
}

bool kinematics_check_boundaries(float *axis) {
  // Boundary checking in the natural constraint space
  if (!g_settings.soft_limits_enabled || cnc_get_exec_state(EXEC_HOMING)) {
    return true;
  }
  // ... per-axis checks ...
  return true;
}

#endif
```

All code must be guarded by `#if (KINEMATIC == KINEMATIC_MYTYPE)` — this ensures
only the selected kinematic is compiled, saving flash on constrained targets.

## 10. Testing recommendations

- **Round-trip test**: Pick a Cartesian position, convert to steps via
  `kinematics_apply_inverse`, convert back via `kinematics_apply_forward`, verify
  the original position is recovered within floating-point precision.
- **Homing test**: Verify the homing sequence homes axes in the correct order and
  sets the reference position correctly.
- **Boundary test**: Test positions inside and outside the workspace.
- **Segment test** (non-linear kinematics): Verify that a long linear move is
  subdivided into segments shorter than `KINEMATICS_MOTION_SEGMENT_SIZE`.
- **Settings round-trip**: Set a kinematic parameter via its `$` ID, verify the
  value is stored and used by the kinematics.

Existing tests live under `test/`; add a `test_kinematic_<type>` fixture when
creating a new kinematic.

## 11. Existing backends reference

### Cartesian (`KINEMATIC_CARTESIAN`)
- Header: `kinematic_cartesian.h` — minimal, only `KINEMATIC_TYPE_STR "C"`
- Implementation: 1:1 axis-to-stepper mapping, no custom settings
- Homing: standard Grbl-style per-axis homing

### CoreXY (`KINEMATIC_COREXY`)
- Header: `kinematic_corexy.h` — defines `COREXY_AXIS_XY` (default),
  `COREXY_AXIS_XZ`, or `COREXY_AXIS_YZ`
- Implementation: belt-driven H-bot kinematics on the selected plane
- Settings: none beyond standard

### Linear Delta (`KINEMATIC_LINEAR_DELTA`)
- Header: `kinematic_linear_delta.h` — `KINEMATICS_MOTION_BY_SEGMENTS`,
  settings: arm length ($106), base radius ($107)
- Implementation: linear delta (linear actuators on towers)

### Delta (`KINEMATIC_DELTA`)
- Header: `kinematic_delta.h` — `KINEMATICS_MOTION_BY_SEGMENTS`,
  settings: base radius ($106), effector radius ($107), bicep length ($108),
  forearm length ($109), bicep homing angle ($28)
- Implementation: rotary delta with bicep/forearm arms

### SCARA (`KINEMATIC_SCARA`)
- Header: `kinematic_scara.h` — `KINEMATICS_MOTION_BY_SEGMENTS`,
  settings: arm length ($106), forearm length ($107), arm homing angle ($28),
  forearm homing angle ($29). Optional `MP_SCARA` fixed-motor variant.
- Implementation: serial SCARA (standard or MP variant)

### RTheta (`KINEMATIC_RTHETA`)
- Header: `kinematic_rtheta.h` — `KINEMATICS_MOTION_BY_SEGMENTS`,
  settings: theta reduction ratio ($106), arm length ($107), homing angle ($28),
  homing distance ($29)
- Implementation: rotary theta (polar) with rotating base and radial arm

### Dummy (`KINEMATIC_DUMMY`)
- Minimal stub for testing. No custom settings, no motion-by-segments.