# µCNC Tool Backend Context

Terms used across the µCNC tool layer (`uCNC/src/hal/tools/`) and its governing document (`.claude/skills/ucnc-tool/BACKEND_GUIDE.md`). This glossary is the authoritative vocabulary; deviations in new backends or docs should be corrected to it.

## Layers

**Tool HAL**:
The interface contract between µCNC core and the tool subsystem: the `tool_t` struct surface declared in `tool.h`, and the management functions (`tool_init`, `tool_change`, `tool_stop`, `tool_set_speed`, `tool_get_speed`, `tool_range_speed`, `tool_set_coolant`, `tool_pid_update`, `tool_get_mode`, `tool_set_mode`, `tool_reset_mode`) implemented in `tool.c`.
_Avoid_: tool manager, tool interface

**Tool backend**:
The per-tool implementation for one machine tool type (e.g. spindle PWM, laser PWM, laser PPI, plasma THC, VFD Modbus, pen servo, embroidery stepper): a single `.c` file that defines a `const tool_t` struct populated with static callback functions. Lives in `uCNC/src/hal/tools/tools/` or inline in the user project.
_Avoid_: tool driver, tool plugin

**Tool palette**:
The set of up to 16 tools assigned to `TOOL1` through `TOOL16` in `cnc_hal_config.h`. Each `TOOLx` macro points to a `const tool_t` symbol. Tools must be assigned sequentially with no gaps. `TOOL_COUNT` sets the number of defined tools; when `TOOL_COUNT == 1` the single tool is always active; when `TOOL_COUNT > 1`, M6 tool changes are enabled.
_Avoid_: tool table, tool magazine

## Tool struct slots

**`tool_t`**:
The struct type representing a tool. All function pointers may be NULL if unused.

| Slot | Type | Purpose |
|---|---|---|
| `startup_code` | `tool_func` | Runs when the tool is loaded (init, set mode flags, configure pins) |
| `shutdown_code` | `tool_func` | Runs when the tool is unloaded (cleanup, restore modes) |
| `pid_update` | `tool_func` | Periodic PID control loop called from main loop |
| `range_speed` | `tool_range_speed_func` | Converts between G-code S-value and tool IO value (bidirectional) |
| `get_speed` | `tool_get_speed_func` | Returns current tool speed (setpoint or sensor feedback) |
| `set_speed` | `tool_set_speed_func` | Sets the tool speed/power at the IO level |
| `set_coolant` | `tool_coolant_func` | Enables/disables flood and mist coolant |

## Tool lifecycle functions

**`tool_init`**:
Called during controller initialization. For a single-tool config, runs `startup_code` immediately. For multi-tool configs, calls `tool_change(g_settings.default_tool)` to load the default tool.

**`tool_change`**:
Stops the current tool, invokes the ATC unmount hook, runs the current tool's `shutdown_code`, copies the new tool's `tool_t` struct from ROM to RAM via `memcpy`, runs its `startup_code`, and invokes the ATC mount hook.

**`tool_stop`**:
Stops the tool: calls `tool_set_speed(0)` then `tool_set_coolant(0)`.

**`tool_set_speed`**:
Sets the tool IO speed. Receives the value already converted by `range_speed`. A positive value means forward (M3), negative means reverse (M4), zero means stop.

**`tool_get_speed`**:
Returns the current tool speed. Default returns the setpoint; a tool may override to return true RPM from a sensor.

**`tool_range_speed`**:
Bidirectional converter. `conv=0`: G-code S-value → tool IO value (e.g. 0–10000 RPM → 0–255 PWM). `conv=1`: tool IO value → S-value.

**`tool_set_coolant`**:
Receives a bitmask: `COOLANT_MASK` (0x02) for flood, `MIST_MASK` (0x01) for mist.

## Tool modes

Defined in `parser.h`:

| Mode | Value | Description |
|---|---|---|
| `SPINDLE_MODE` | 0 | Standard spindle (default if no tool sets a mode) |
| `PWM_VARPOWER_MODE` | 1 | PWM variable power — tool power scales with M4 dynamic power |
| `PPI_MODE` | 2 | Pulse Per Inch modulation |
| `PPI_VARPOWER_MODE` | 4 | PPI with variable pulse width |
| `PLASMA_THC_MODE` | 8 | Plasma THC (torch height control) |
| `EMBROIDERY_MODE` | 16 | Embroidery stepper mode |

A tool sets its mode in `startup_code` via `tool_set_mode()` and restores the default mode in `shutdown_code` via `tool_reset_mode()`. The user-configured default mode is `$32`.

## Coolant and IO

**`COOLANT_MASK`** (value `0x02`):
Bit position for flood coolant in the coolant bitmask.

**`MIST_MASK`** (value `0x01`):
Bit position for mist coolant in the coolant bitmask.

**`SET_SPINDLE(PWM, DIR, value, dir_value)`**:
Macro that sets direction pin then PWM value atomically. Used by spindle-type tools.

**`SET_LASER(PWM, value)`**:
Macro that sets a PWM value without direction. Used by laser-type tools.

**`SET_COOLANT(FLOOD, MIST, mask)`**:
Macro that sets flood and mist coolant pins from a bitmask. Accepts `UNDEF_PIN` for unused channels.

## Pin configuration pattern

Each tool backend defines configurable pin macros with `#ifndef` defaults:

```c
#ifndef SPINDLE_PWM
#define SPINDLE_PWM PWM0
#endif
#ifndef SPINDLE_PWM_DIR
#define SPINDLE_PWM_DIR DOUT0
#endif
```

All pin references are wrapped in `#if ASSERT_PIN(PIN_NAME)` to compile out when the pin is not connected. `UNDEF_PIN` indicates an unused pin.

## Built-in tools

Defined in `uCNC/src/hal/tools/tools/`:

- **spindle_pwm**: PWM + direction pin + coolant + optional PID
- **laser_pwm**: PWM-only laser with configurable frequency and minimum power
- **laser_ppi**: Pulse-per-inch laser with PPI/PPI-PW/PPI-mixed modes and extra M-codes
- **spindle_relay**: Relay-based on/off spindle with direction + coolant
- **spindle_besc**: BESC-controlled brushless motor via servo signal + relay
- **pen_servo**: Pen up/down via servo signal with configurable positions
- **vfd_pwm**: VFD speed control via PWM with analog RPM feedback + PID
- **vfd_modbus**: VFD speed control via Modbus RTU (softuart), supports multiple VFD brands
- **plasma_thc**: Plasma cutter with torch height control (up/down inputs, arc-ok, voltage divider)
- **embroidery_stepper**: Embroidery machine with stitch synchronization

## Extended settings (PID)

Tools with PID control register extended settings for Kp, Ki, Kd using `DECL_EXTENDED_SETTING`. PID setting IDs are tool-specific (e.g. `SPINDLE_PWM_PID_SETTING_ID 300`, `VFD_PWM_PID_SETTING_ID 304`). The PID structures are declared inside an `#if defined(ENABLE_TOOL_PID_CONTROLLER)` guard.

## ATC hooks

**`HOOK(tool_atc_unmount)`**:
Invoked before unloading the current tool during `tool_change`. Can reject the change by returning a non-OK status.

**`HOOK(tool_atc_mount)`**:
Invoked after loading the new tool during `tool_change`. Can reject by returning a non-OK status.

Both hooks are only compiled when `ENABLE_ATC_HOOKS` is defined and `TOOL_COUNT > 1`.