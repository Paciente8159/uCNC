---
name: ucnc-tool
description: Use for creating, modifying, porting, or diagnosing the µCNC Tool HAL, tool backends, tool selection, ATC lifecycle, speed conversion, coolant integration, or tool-specific PID and mode behavior under uCNC/src/hal/tools/.
disable-model-invocation: false
---

# µCNC tool work

Treat the current source, compile-time configuration, and tests as authoritative. The
documents in this skill explain intent and established patterns; they do not override
`tool.h`, `tool.c`, or an affected backend.

## Route the task

Before editing tool-layer code, read:

- `uCNC/src/hal/tools/README.md`
- `uCNC/src/hal/tools/tool.h`
- the affected backend and the nearest backend with the same control mechanism

Then load only the material needed for the task:

- **New backend or substantial port:** read `BACKEND_GUIDE.md`.
- **Lifecycle, ATC, stop, mode, or fallback behavior:** read
  `uCNC/src/hal/tools/tool.c`; inspect the hook declarations and callers involved.
- **PWM, direction, or coolant macros:** read
  `uCNC/src/hal/tools/tool_helper.h` and the relevant IO HAL definitions.
- **PID or measured-speed feedback:** read `uCNC/src/modules/pid.h`, a current
  PID-enabled backend, and the extended-setting declarations it follows.
- **Tool palette or feature configuration:** inspect the relevant parts of
  `uCNC/cnc_hal_config.h`, `uCNC/cnc_config.h`, and
  `uCNC/src/cnc_hal_config_helper.h` rather than relying on a cached macro list.
- **Tool terminology or documentation:** read `CONTEXT.md`.
- **Behavioral tests:** read `test/README.md` and the closest fixture. Current
  starting points include `test_grbl_spindle` and `test_domain2_f_tools`.

## Contract anchors

Confirm these against the current source while working:

- `tool_set_speed()` receives a signed tool-IO value: positive and negative encode
  direction; zero stops the output.
- `tool_range_speed()` normalizes the value to a non-negative magnitude before it
  calls a backend converter. A `NULL` converter means identity conversion.
- Callback slots are optional. Preserve the manager's fallback behavior when a slot
  is `NULL`.
- In a multi-tool change, the manager stops the current tool, invokes ATC unmount,
  runs shutdown, selects and starts the new backend, then invokes ATC mount. Inspect
  the early-return behavior before changing error handling.
- A backend that sets a temporary tool mode owns restoring the configured mode during
  shutdown. A tool change does not perform an unconditional mode reset.
- Single-tool initialization and multi-tool selection take different paths; verify
  both when changing shared lifecycle behavior.

## Safety and completion

Account for the output state at initialization, zero speed, controller stop, tool
change, and failed external communication. Check conversion bounds, division by zero,
integer width, and callback execution cost on constrained targets.

Finish when every affected compile-time configuration is identified, the narrowest
relevant fixture or target build passes, and remaining hardware-only timing or
electrical coverage is named explicitly. Emulator tests do not establish
hardware-in-the-loop behavior.
