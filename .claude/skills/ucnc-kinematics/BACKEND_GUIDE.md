# µCNC kinematics backend authoring guide

Use this guide when adding a kinematic backend or substantially changing its geometry,
homing, workspace, or actuator mapping. The live contract is defined by
`uCNC/src/hal/kinematics/kinematic.h`, `kinematic.c`, the selected backend, and the
current compile-time dispatch. Read those sources before adapting any example.

## Establish the coordinate contract

Before implementing equations, name the coordinate spaces and their units:

- G-code and machine coordinates;
- coordinates after optional kinematic transforms and skew compensation;
- actuator positions in integer steps;
- any kinematic-specific internal geometry or homing reference;
- reported coordinates reconstructed from actuator positions.

For each conversion, decide the valid workspace, units, precision, rounding, and how
the backend represents an unreachable or invalid result. Consider singularities,
inverse-trigonometric and square-root domains, multiple solution branches, angular
wrapping, and `int32_t` step limits. Make the choice consistent with the nearest
backend and its callers.

## Start from the live interface

Inspect `kinematic.h` rather than copying declarations into the new backend. A backend
normally provides:

| Function | Responsibility |
|---|---|
| `kinematics_init` | Prepare derived geometry or runtime state when the backend needs it. |
| `kinematics_apply_inverse` | Convert the wrapper's transformed world coordinate to actuator steps. |
| `kinematics_apply_forward` | Reconstruct world coordinates from actuator steps. |
| `kinematics_home` | Execute the kinematic-specific homing strategy and establish a valid reference position. |
| `kinematics_check_boundaries` | Apply the selected backend's workspace and soft-limit policy. |

`kinematics_coordinates_to_steps` and `kinematics_steps_to_coordinates` are shared
wrappers in `kinematic.c`. Optional `kinematics_apply_transform` and
`kinematics_apply_reverse_transform` already have weak no-op defaults. Define them
only when the backend needs an additional coordinate mapping beyond the wrappers.

## Choose a comparable backend

Select a reference by geometry and homing behavior:

- direct Cartesian axes: `kinematic_cartesian.c`
- coupled linear actuators: `kinematic_corexy.c`
- parallel linear actuators: `kinematic_linear_delta.c`
- parallel rotary arms: `kinematic_delta.c`
- serial rotary arms: `kinematic_scara.c`
- polar coordinates: `kinematic_rtheta.c`

Read the chosen header and implementation in full. It establishes the useful local
patterns for settings, workspace checks, initialization, segmentation, and homing.
Do not infer an ID, type string, feature flag, or compile guard from a filename;
inspect `kinematics.h` and `kinematicdefs.h` in the current checkout.

## Coordinate conversion

The shared wrapper copies a normal move target before applying optional transforms and
skew compensation, then calls inverse kinematics. In the reverse direction it calls
forward kinematics, unskews, and applies the optional reverse transform. Homing skips
these wrapper modifications.

Implement direct conversion only for the coordinate space the backend owns. Keep
forward and inverse conversions mutually consistent within a tolerance based on step
resolution and geometry, rather than an ideal floating-point comparison. Test known
reference points, limits of the reachable workspace, near-singular points, and a
continuity case across any angular wrap or branch choice.

## Boundaries and segmentation

Check workspace constraints in the coordinate space appropriate to the kinematic. A
Cartesian envelope is not enough for mechanisms with radial, angular, or coupled-arm
constraints. Preserve the active behavior for disabled soft limits, homing, origin
placement, and optional jog clamping; some backends may modify the target in these
cases.

Enable `KINEMATICS_MOTION_BY_SEGMENTS` only when straight Cartesian moves require
subdivision to control actuator-space error. Choose the segment size from acceptable
path error and planner cost, then verify long moves, short moves, and endpoint arrival.
The motion layer creates smaller linear segments; it does not change a line into an
arc.

## Homing is a state transition

Do not apply a universal homing sequence. Determine from a comparable backend:

- which coordinate and actuator limit masks must home together;
- the safe homing order and any pre-home reference needed by the mechanism;
- behavior after each failed `mc_home_axis` call;
- when pull-off occurs and which axes it applies to;
- when `itp_reset_rt_position`, `mc_sync_position`, or other synchronization is
  necessary;
- behavior under the active limit, origin, and Grbl-style-homing configuration.

The completed sequence must leave planner, interpolator, parser, and the physical
mechanism in a mutually consistent position.

## Settings and compile-time composition

Kinematic-specific settings macros are composed into the global settings structure and
registry. Follow a neighboring header and inspect their expansion sites in
`grbl_settings.h`, `grbl_settings.c`, and `defaults.h`. Allocate setting IDs only after
checking the current registry for conflicts; do not rely on a static numeric range.

For a new selectable kinematic, add the required identifier and dispatch entry in the
current kinematics headers, then confirm the selected build includes the new source.
Keep feature guards consistent between the header, implementation, configuration, and
source filters. Do not add an identification flag unless a consuming subsystem needs
it.

## Minimal backend shape

```c
void kinematics_init(void) {
  /* Derive geometry only when this backend needs runtime state. */
}

void kinematics_apply_inverse(float *axis, int32_t *steps) {
  /* World coordinate to actuator steps. */
}

void kinematics_apply_forward(int32_t *steps, float *axis) {
  /* Actuator steps to world coordinate. */
}

uint8_t kinematics_home(void) {
  /* Kinematic-specific homing and position establishment. */
}

bool kinematics_check_boundaries(float *axis) {
  /* Workspace, soft-limit, homing, and jog-clamping policy. */
}
```

The shape is not a universal implementation recipe. Add derived state, transforms,
settings, segmentation, and synchronization only when the mechanism requires them.

## Verification

Build a configuration that selects the changed backend. Verify the applicable cases:

- forward/inverse reference points with a step- and geometry-aware tolerance;
- reachable, unreachable, boundary, and near-singular coordinates;
- branch continuity and angular wrapping where relevant;
- normal moves, long segmented moves, and endpoint accuracy;
- soft limits disabled, homing active, and jog clamping when enabled;
- successful homing, each failure point, pull-off, and post-home position reporting;
- settings persistence and runtime geometry initialization when present.

Use a focused native fixture when the configuration can exercise the backend; existing
default fixtures primarily cover Cartesian motion and homing. Build a representative
target separately, and reserve limit polarity, coupled-actuator behavior, calibration,
and timing for hardware-in-the-loop validation.
