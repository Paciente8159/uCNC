# µCNC tool backend authoring guide

Use this guide when creating a backend or substantially changing how one controls its
hardware. The live contract is defined by `uCNC/src/hal/tools/tool.h` and
`uCNC/src/hal/tools/tool.c`. Read both, then use an existing backend with the closest
control mechanism as the implementation pattern.

## Design the boundary first

Before choosing callbacks, establish:

- the actuator and control mechanism: PWM, digital output, servo, stepper, or a
  communication protocol;
- whether direction is meaningful and how zero must affect every output;
- the units and bounds of the G-code setpoint, hardware command, and measured value;
- whether feedback, coolant, temporary tool modes, settings, or custom M-codes are
  required;
- what initialization, shutdown, timeout, and communication-failure states are safe;
- which boards and compile-time feature combinations can build the backend.

The result should make the zero-output state and failure behavior unambiguous before
code is written.

## Implement the current callback contract

Inspect `tool_t` in `tool.h`; do not reproduce its declaration in a backend. Use
designated initializers and set unused slots explicitly to `NULL`, following the local
style.

The usual responsibilities are:

| Callback | Backend responsibility |
|---|---|
| `startup_code` | Configure required hardware and settings; establish the active mode and safe initial output. |
| `shutdown_code` | Leave outputs safe and undo backend-owned temporary state, including tool modes. |
| `pid_update` | Perform a bounded periodic feedback update when tool PID support is enabled. |
| `range_speed` | Convert a non-negative magnitude between G-code and hardware units. Omit it when identity conversion is correct. |
| `get_speed` | Return measured speed or power in the units expected by callers. Omit it to use the manager's setpoint fallback. |
| `set_speed` | Apply a signed hardware command; sign carries direction and zero means stop. |
| `set_coolant` | Apply the coolant mask, or remain `NULL` when the backend owns no coolant behavior. |

Do not assume every speed-capable backend needs `range_speed`: the manager preserves
the value when that callback is `NULL`. Conversely, a converter receives magnitude,
not direction, because `tool_range_speed()` applies `ABS()` before dispatch.

## Follow a mechanism-specific neighbor

Choose the reference by behavior, not by tool name alone:

- PWM and direction: `tools/spindle_pwm.c` or `tools/vfd_pwm.c`
- PWM variable power: `tools/laser_pwm.c`
- digital forward/reverse: `tools/spindle_relay.c`
- servo output: `tools/pen_servo.c` or `tools/spindle_besc.c`
- protocol-controlled equipment: `tools/vfd_modbus.c`
- motion-synchronized behavior: `tools/laser_ppi.c`, `tools/plasma_thc.c`, or
  `tools/embroidery_stepper.c`

Read the chosen file in full. Search the current configuration helper and build files
for its feature flag and source inclusion; do not infer either from the filename.

## Pins and IO

Use the repository's canonical pin names and established override pattern. Give a pin
an `#ifndef` default only when users are expected to override it. Guard optional pin
access with the appropriate `ASSERT_PIN` form and inspect the IO HAL when extended
pins or nonstandard access are involved.

`SET_SPINDLE`, `SET_LASER`, and `SET_COOLANT` are convenience macros in
`tool_helper.h`. They express an operation sequence; they do not by themselves create
an atomic section. Use the project's atomic facilities only when shared-state or
interrupt behavior actually requires them.

## Speed conversion

Define the three quantities explicitly:

1. G-code setpoint, such as RPM or requested power.
2. Hardware command, such as PWM duty or a device register value.
3. Measured feedback, if any.

Clamp at the layer that owns the valid range and preserve zero exactly. Check maximum
and minimum settings before division, use widths that cover intermediate values, and
document unavoidable quantization. Test representative endpoints and at least one
interior value in both conversion directions when a converter is present.

Direction belongs in the signed command passed to `set_speed`; `range_speed` handles
only magnitude.

## Lifecycle and modes

For `TOOL_COUNT == 1`, `tool_init()` calls the configured backend's startup callback
directly. For a multi-tool configuration, initialization selects the configured
default through `tool_change()`.

Before modifying tool changes, inspect the exact sequence and failure paths in
`tool.c`. In particular, an ATC unmount failure occurs after the current tool is
stopped, while an ATC mount failure occurs after the new backend has started. Do not
invent rollback behavior without treating it as a contract and machine-safety change.

A backend that calls `tool_set_mode()` must restore the configured mode with
`tool_reset_mode()` when it shuts down. The manager does not reset modes on every
change.

## PID and feedback

Copy the current pattern from a PID-enabled backend and `uCNC/src/modules/pid.h`.
Keep setting identifiers unique, validate that the configured sample period has usable
integer precision, and guard all related declarations and initializer fields with the
same feature expression.

Keep `pid_update` bounded and non-blocking. Distinguish the programmed setpoint from
measured feedback, and ensure the PID output is converted exactly once before it
reaches the hardware.

## Communication-backed tools

Define timeout, retry, and error-state behavior explicitly. A controller callback must
not introduce unbounded waits into the main loop. Decide whether loss of communication
stops the tool, holds the controller, reports stale feedback, or follows another
existing project policy, and verify that choice against the closest backend and its
callers.

## Registration

Expose one `const tool_t` symbol and assign that symbol to a sequential `TOOL1` through
`TOOL16` entry. Derive `TOOL_COUNT`, feature flags, and source inclusion from the
current configuration rather than this guide. Registration is complete only when the
selected build actually links the backend symbol.

Minimal shape:

```c
const tool_t my_tool = {
    .startup_code = &startup_code,
    .shutdown_code = &shutdown_code,
    .pid_update = NULL,
    .range_speed = &range_speed,
    .get_speed = NULL,
    .set_speed = &set_speed,
    .set_coolant = NULL};
```

Adjust every slot to the hardware contract; the example is a struct shape, not a
universal backend design.

## Verification

Build the configuration that selects the backend. Exercise the applicable behavior:

- startup and safe initial output;
- positive, negative, zero, minimum, maximum, and out-of-range speed commands;
- conversion round trips within the documented quantization error;
- stop, program end, alarm-related stop paths, and tool change;
- coolant and temporary mode restoration;
- feedback loss, timeout, and PID behavior when present.

For the default emulator, start with `test_grbl_spindle` or
`test_domain2_f_tools`; add configuration-specific coverage for alternative tools.
Report target builds separately from hardware-in-the-loop validation.
