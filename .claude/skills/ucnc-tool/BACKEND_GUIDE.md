# µCNC Tool Backend Authoring Guide

This document combines the normative µCNC **Tool HAL contract** with recommended
backend-authoring patterns. The keywords below distinguish the two:

- **Required**: observable behavior or API/ABI compatibility that every tool backend
  must preserve. A deviation is a contract change and requires communication with the project.
- **Recommended**: the common implementation or layout. A backend may deviate
  when its hardware requires it and should document the reason.
- **Example**: an observed implementation, not a requirement.

The `tool_t` struct signature in `tool.h` is normative; implementation examples in this
document remain examples. File layout, section ordering, and implementation techniques
are authoring guidance unless explicitly labeled **Required**.

Reference backends: `spindle_pwm`, `laser_pwm`, `laser_ppi`, `spindle_relay`,
`spindle_besc`, `pen_servo`, `vfd_pwm`, `vfd_modbus`, `plasma_thc`, `embroidery_stepper`.

It is written for tool backend authors. Terminology is pinned in
`.claude/skills/ucnc-tool/CONTEXT.md`; contract-level decisions live in `docs/adr/`.

## Authoring workflow

1. **Understand the tool hardware.** Identify the type of actuator (spindle, laser, plasma,
   pen, VFD, embroidery), the control signal type (PWM, relay, servo, Modbus, stepper),
   direction control (reversible, unidirectional), feedback (none, analog encoder, VFD readout),
   coolant support, and any special modes or M-codes.

2. **Choose the functions to implement.** Most tools need `set_speed`, `range_speed`,
   `startup_code`, and optionally `set_coolant`, `shutdown_code`, `pid_update`, `get_speed`.
   NULL any unused slots.

3. **Define configurable pin macros.** Use `#ifndef` defaults for every pin so users can
   override them in `cnc_hal_config.h`. Guard pin accesses with `ASSERT_PIN()`.

4. **Implement the tool .c file.** Follow the pattern in §4. Declare the `const tool_t`
   symbol at the end with all slots populated (NULL for unused).

5. **Register and test.** Add `#define TOOL<x> my_tool` to `cnc_hal_config.h` in sequential
   order, set `TOOL_COUNT` accordingly, build, and test.

## 1. Big picture: where the tool backend fits

The tool layer sits between G-code execution and the physical output pins. Its
responsibilities are:

- **Speed control**: convert G-code S-values to hardware-level signals (PWM duty cycle,
  relay on/off, Modbus register value, servo pulse width).
- **Direction control**: map M3 (forward) and M4 (reverse) to direction pin states.
- **Coolant control**: manage flood and mist coolant outputs.
- **Tool change**: support multi-tool configurations with startup/shutdown sequences
  and ATC (automatic tool changer) hooks.
- **PID regulation**: optional closed-loop speed maintenance using sensor feedback.
- **Mode flags**: signal to the planner/interpolator how the tool behaves (laser variable
  power, PPI, plasma THC, embroidery).

```
┌─────────────────────────────────────────────┐
│           G-code execution (parser)           │
│  M3 S1000, M4 S500, M5, M6 T2, M7, M8, M9   │
├─────────────────────────────────────────────┤
│            Tool HAL (tool.c)                  │
│  tool_init, tool_change, tool_stop,           │
│  tool_set_speed, tool_get_speed,              │
│  tool_range_speed, tool_set_coolant           │
├─────────────────────────────────────────────┤
│         Per-tool backend (.c file)            │
│  startup │ shutdown │ set_speed │ range_speed │
│  get_speed │ pid_update │ set_coolant         │
│  + const tool_t symbol                        │
├─────────────────────────────────────────────┤
│            IO HAL (io_hal.h)                  │
│  io_set_pwm, io_set_output, io_clear_output,  │
│  io_config_pwm, io_get_input, ...             │
└─────────────────────────────────────────────┘
```

### 1.1 File/directory layout

```
uCNC/src/hal/tools/
├── tool.h               # tool_t struct and management function declarations
├── tool.c               # Management implementations (init, change, stop, etc.)
├── tool_helper.h         # Convenience macros (SET_SPINDLE, SET_LASER, SET_COOLANT)
├── README.md            # User-facing tool authoring guide
└── tools/
    ├── README.md        # Per-tool documentation
    ├── spindle_pwm.c
    ├── laser_pwm.c
    ├── laser_ppi.c
    ├── spindle_relay.c
    ├── spindle_besc.c
    ├── pen_servo.c
    ├── vfd_pwm.c
    ├── vfd_modbus.c
    ├── plasma_thc.c
    └── embroidery_stepper.c
```

A new tool adds a single `.c` file (to `tools/` or the user project root). No header
is required — the tool identity is entirely captured by the `const tool_t` symbol.

## 2. The `tool_t` struct

```c
typedef struct {
  tool_func startup_code;              // runs on tool load
  tool_func shutdown_code;             // runs on tool unload
  tool_func pid_update;                // periodic PID loop
  tool_range_speed_func range_speed;   // S-value <-> IO value conversion
  tool_get_speed_func get_speed;       // read current speed
  tool_set_speed_func set_speed;       // write speed to hardware
  tool_coolant_func set_coolant;       // set coolant outputs
} tool_t;
```

**Required**: `set_speed` and `range_speed` are needed for any tool that accepts
speed commands. All other slots may be NULL.

**Important**: All seven slots must be explicitly populated. Unused slots must be
set to `NULL`, not omitted. Omitting a field in a C designated initializer zeroes it,
which is equivalent to NULL for a pointer — but explicit NULL is the project convention.

## 3. Callback slot contract

### 3.1 `startup_code`

**Signature**: `static void startup_code(void)`

**When called**: During `tool_change()`, after the new tool's struct is copied to RAM,
before the ATC mount hook.

**Purpose**: One-time initialization when the tool becomes active.

**Common operations**:
1. Configure PWM frequency with `io_config_pwm(PIN, FREQ_HZ)`.
2. Initialize PID controller and load extended settings.
3. Set tool mode with `tool_set_mode(MODE_FLAG)`.
4. Set initial output states.

*Example (laser_pwm)*:
```c
static void startup_code(void) {
  io_config_pwm(LASER_PWM, LASER_FREQ);
  io_set_pwm(LASER_PWM, 0);
  tool_set_mode(PWM_VARPOWER_MODE);
}
```

### 3.2 `shutdown_code`

**Signature**: `static void shutdown_code(void)`

**When called**: During `tool_change()`, before the new tool is loaded, after the
ATC unmount hook.

**Purpose**: Clean up when the tool is deactivated.

**Common operations**:
1. Restore mode with `tool_reset_mode()`.
2. Turn off outputs.

### 3.3 `set_speed`

**Signature**: `static void set_speed(int16_t value)`

**When called**: When M3, M4, M5 are executed or during dynamic power adjustment
(e.g. M4 laser dynamic power).

**Contract**:
- `value > 0`: forward direction (M3)
- `value < 0`: reverse direction (M4)
- `value == 0`: stop (M5)
- The value has already been converted by `range_speed` and is in tool IO units
  (e.g. 0–255 for 8-bit PWM).

**Hardware operations**:
- Set direction pin state based on sign.
- Set PWM/signal magnitude based on absolute value.

*Example (spindle_pwm)*:
```c
static void set_speed(int16_t value) {
  if (value <= 0) {
    io_clear_output(SPINDLE_PWM_DIR);
  } else {
    io_set_output(SPINDLE_PWM_DIR);
  }
  io_set_pwm(SPINDLE_PWM, (uint8_t)ABS(value));
}
```

### 3.4 `range_speed`

**Signature**: `static int16_t range_speed(int16_t value, uint8_t conv)`

**When called**: Before `set_speed` (conv=0) to convert S-value to IO value,
and when reporting speed back (conv=1) to convert IO value to S-value.

**Contract**:
- `conv == 0`: Input is G-code S-value (e.g. 0–10000 RPM for a spindle).
  Output is tool IO value (e.g. 0–255 for 8-bit PWM).
- `conv == 1`: Input is tool IO value. Output is G-code S-value.
- Must be invertible: `range_speed(range_speed(s, 0), 1) ≈ s` (within precision).
- Input and output sign convention: positive = forward, negative = reverse.

*Example (spindle_pwm, linear scaling)*:
```c
static int16_t range_speed(int16_t value, uint8_t conv) {
  if (!conv) {
    value = (int16_t)(255.0f * ((float)value / g_settings.spindle_max_rpm));
  } else {
    value = (int16_t)roundf((1.0f / 255.0f) * value * g_settings.spindle_max_rpm);
  }
  return value;
}
```

*Example (laser_pwm, with minimum power)*:
```c
static int16_t range_speed(int16_t value, uint8_t conv) {
  if (value == 0) return 0;
  if (!conv) {
    value = (int16_t)(LASER_PWM_MIN_VALUE +
                      ((255.0f - LASER_PWM_MIN_VALUE) *
                       ((float)value / g_settings.spindle_max_rpm)));
  } else {
    value = (int16_t)roundf((1.0f / (255.0f - LASER_PWM_MIN_VALUE)) *
                            (value - LASER_PWM_MIN_VALUE) *
                            g_settings.spindle_max_rpm);
  }
  return value;
}
```

### 3.5 `get_speed`

**Signature**: `static uint16_t get_speed(void)`

**When called**: Status reporting (`?` command), probing, or synchronization.

**Default behavior**: If NULL, `tool_get_speed()` returns the setpoint value.
Non-NULL implementations can return true RPM from a sensor.

*Example (spindle_pwm with optional encoder)*:
```c
static uint16_t get_speed(void) {
#ifdef SPINDLE_PWM_RPM_ENCODER
  return encoder_get_rpm(SPINDLE_PWM_RPM_ENCODER);
#else
  return tool_get_setpoint();
#endif
}
```

### 3.6 `pid_update`

**Signature**: `static void pid_update(void)`

**When called**: From the main loop when `ENABLE_TOOL_PID_CONTROLLER` is defined
and the tool provides a non-NULL `pid_update` slot.

**Purpose**: Read the actual tool speed from feedback, compute PID correction,
and update the output.

*Pattern*:
```c
#if defined(ENABLE_TOOL_PID_CONTROLLER)
static pid_data_t my_tool_pid;
DECL_EXTENDED_SETTING(MY_PID_SETTING_ID, my_tool_pid.k, float, 3,
                      proto_gcode_setting_line_flt);

static void pid_update(void) {
  float output = tool_get_setpoint();
  if (output != 0) {
    if (pid_compute(&my_tool_pid, &output, output, get_speed(),
                    HZ_TO_MS(MY_PID_SAMPLE_RATE_HZ))) {
      io_set_pwm(MY_PWM_PIN, range_speed((int16_t)output, 0));
    }
  }
}
#endif
```

**PID registration steps**:
1. Define a `PID_SETTING_ID` (tool-specific, e.g. 300, 304).
2. Define a `PID_SAMPLE_RATE_HZ` (e.g. 125).
3. Declare a `pid_data_t` static variable.
4. Register extended settings with `DECL_EXTENDED_SETTING`.
5. In `startup_code`: call `EXTENDED_SETTING_INIT` and `settings_load`.
6. Set `pid.max` and `pid.min` from `g_settings.spindle_max_rpm` / `spindle_min_rpm`.

### 3.7 `set_coolant`

**Signature**: `static void set_coolant(uint8_t value)`

**When called**: When M7 (mist on), M8 (flood on), M9 (coolant off) are executed.

**Contract**:
- `value & COOLANT_MASK` (0x02): flood on
- `value & MIST_MASK` (0x01): mist on
- `value == 0`: all coolant off

*Example*:
```c
static void set_coolant(uint8_t value) {
#ifdef ENABLE_COOLANT
  SET_COOLANT(MY_COOLANT_FLOOD, MY_COOLANT_MIST, value);
#endif
}
```

Use `UNDEF_PIN` for coolant pins that are not wired:
```c
SET_COOLANT(UNDEF_PIN, MY_MIST_PIN, value);  // only mist connected
```

## 4. Complete tool file template

```c
/*
 * Name: my_tool.c
 * Description: Implements a <type> tool for µCNC.
 */

#include "../../../cnc.h"

#include <math.h>
#include <stdbool.h>

/* ---- Configurable pin defaults ---- */
#ifndef MY_TOOL_PWM
#define MY_TOOL_PWM PWM0
#endif
#ifndef MY_TOOL_DIR
#define MY_TOOL_DIR DOUT0
#endif

#ifdef ENABLE_COOLANT
#ifndef MY_TOOL_COOLANT_FLOOD
#define MY_TOOL_COOLANT_FLOOD DOUT2
#endif
#endif

/* ---- Optional PID ---- */
#if defined(ENABLE_TOOL_PID_CONTROLLER) && !defined(DISABLE_MY_TOOL_PID)
#ifndef MY_TOOL_PID_SAMPLE_RATE_HZ
#define MY_TOOL_PID_SAMPLE_RATE_HZ 125
#endif
#define MY_TOOL_PID_SETTING_ID 300
#include "../../../modules/pid.h"
static pid_data_t my_tool_pid;
DECL_EXTENDED_SETTING(MY_TOOL_PID_SETTING_ID, my_tool_pid.k, float, 3,
                      proto_gcode_setting_line_flt);
#endif

/* ---- Callbacks ---- */
static void startup_code(void) {
#if ASSERT_PIN(MY_TOOL_PWM)
  io_config_pwm(MY_TOOL_PWM, 1000);
#endif

#if defined(ENABLE_TOOL_PID_CONTROLLER) && !defined(DISABLE_MY_TOOL_PID)
  EXTENDED_SETTING_INIT(MY_TOOL_PID_SETTING_ID, my_tool_pid.k);
  settings_load(EXTENDED_SETTING_ADDRESS(MY_TOOL_PID_SETTING_ID),
                (uint8_t *)my_tool_pid.k, sizeof(my_tool_pid.k));
  my_tool_pid.max = g_settings.spindle_max_rpm;
  my_tool_pid.min = g_settings.spindle_min_rpm;
#endif
}

static void shutdown_code(void) {
  // Cleanup on tool unload
}

static void set_speed(int16_t value) {
#if ASSERT_PIN(MY_TOOL_DIR)
  if (value <= 0) {
    io_clear_output(MY_TOOL_DIR);
  } else {
    io_set_output(MY_TOOL_DIR);
  }
#endif
#if ASSERT_PIN(MY_TOOL_PWM)
  io_set_pwm(MY_TOOL_PWM, (uint8_t)ABS(value));
#endif
}

static int16_t range_speed(int16_t value, uint8_t conv) {
  if (!conv) {
    value = (int16_t)(255.0f * ((float)value / g_settings.spindle_max_rpm));
  } else {
    value = (int16_t)roundf((1.0f / 255.0f) * value * g_settings.spindle_max_rpm);
  }
  return value;
}

static uint16_t get_speed(void) {
  return tool_get_setpoint();
}

static void set_coolant(uint8_t value) {
#ifdef ENABLE_COOLANT
  SET_COOLANT(MY_TOOL_COOLANT_FLOOD, UNDEF_PIN, value);
#endif
}

#if defined(ENABLE_TOOL_PID_CONTROLLER) && !defined(DISABLE_MY_TOOL_PID)
static void pid_update(void) {
  float output = tool_get_setpoint();
  if (output != 0) {
    if (pid_compute(&my_tool_pid, &output, output, get_speed(),
                    HZ_TO_MS(MY_TOOL_PID_SAMPLE_RATE_HZ))) {
      io_set_pwm(MY_TOOL_PWM, range_speed((int16_t)output, 0));
    }
  }
}
#endif

/* ---- Tool struct ---- */
const tool_t my_tool = {
    .startup_code = &startup_code,
    .shutdown_code = &shutdown_code,
    .pid_update =
#if defined(ENABLE_TOOL_PID_CONTROLLER) && !defined(DISABLE_MY_TOOL_PID)
        &pid_update,
#else
        NULL,
#endif
    .range_speed = &range_speed,
    .get_speed = &get_speed,
    .set_speed = &set_speed,
    .set_coolant = &set_coolant};
```

## 5. Tool mode flags

A tool that needs special handling in the planner/interpolator must set its mode:

```c
static void startup_code(void) {
  tool_set_mode(PWM_VARPOWER_MODE);  // enable M4 dynamic power scaling
}

static void shutdown_code(void) {
  tool_reset_mode();  // restore default ($32)
}
```

Available modes and their effects:

| Mode | Effect |
|---|---|
| `SPINDLE_MODE` (0) | Standard spindle behavior |
| `PWM_VARPOWER_MODE` (1) | M4 scales power with speed — laser-like dynamic power |
| `PPI_MODE` (2) | Pulse Per Inch modulation for laser marking |
| `PPI_VARPOWER_MODE` (4) | PPI with variable pulse width |
| `PLASMA_THC_MODE` (8) | Torch height control active during cuts |
| `EMBROIDERY_MODE` (16) | Stitch synchronization with stepper motion |

Set via `tool_set_mode()` in startup. The mode is preserved until `tool_reset_mode()`
or the next tool change restores `$32`.

## 6. Pin configuration pattern

**Recommended**: Define every pin with an `#ifndef` guard so users can override:

```c
#ifndef MY_TOOL_PWM
#define MY_TOOL_PWM PWM0
#endif
```

Guard runtime accesses with `ASSERT_PIN()`:
```c
#if ASSERT_PIN(MY_TOOL_PWM)
  io_set_pwm(MY_TOOL_PWM, value);
#endif
```

`ASSERT_PIN` evaluates to false when the pin is `UNDEF_PIN` (255), allowing the
compiler to dead-strip the branch. Tools that require a specific pin should use
`#if ASSERT_PIN(...)` at the point of use.

## 7. Tool palette registration

In `cnc_hal_config.h`:

1. Set `TOOL_COUNT` to the number of tools (1–16):
   ```c
   #define TOOL_COUNT 3
   ```

2. Define each tool in sequential order, no gaps:
   ```c
   #if (TOOL_COUNT >= 1)
   #define TOOL1 spindle_pwm
   #endif
   #if (TOOL_COUNT >= 2)
   #define TOOL2 laser_pwm
   #endif
   #if (TOOL_COUNT >= 3)
   #define TOOL3 my_custom_tool
   #endif
   ```

3. The `M6` tool change command is automatically available when `TOOL_COUNT >= 2`.

**Rules**:
- Tools must be numbered contiguously from 1. Defining TOOL1 and TOOL3 without TOOL2
  is an error.
- `#define TOOL<x> my_symbol` — the symbol must match the `const tool_t` variable name
  in the implementation file.
- `extern const tool_t tool` is automatically declared by `DECL_TOOL()` in `tool.c`.

## 8. Creation checklist

- [ ] **Create the `.c` file** in `uCNC/src/hal/tools/tools/` or alongside the project.
      - `#include "../../../cnc.h"` (adjust relative path as needed)
      - Static callback functions for each needed slot
      - `const tool_t my_tool = {...};` with all slots populated (NULL for unused)
- [ ] **Define configurable pin defaults** with `#ifndef` guards.
- [ ] **Guard variant features** (PD, coolant, mode flags) with appropriate compile-time
      macros (`ENABLE_TOOL_PID_CONTROLLER`, `ENABLE_COOLANT`, etc.).
- [ ] **Set tool mode** in `startup_code` if the tool needs non-default behavior.
- [ ] **Restore tool mode** in `shutdown_code` if `startup_code` changed it.
- [ ] **Wrap PID code** in `#if defined(ENABLE_TOOL_PID_CONTROLLER)`.
- [ ] **Register the tool** in `cnc_hal_config.h`:
      - Add `#define TOOL<x> my_tool` in the correct sequential position
      - Ensure `TOOL_COUNT` includes the new tool
- [ ] **Build and test**: compile for the target board, verify M3/M4/M5/M6/M7/M8/M9
      behavior, test speed range conversion, and (if applicable) PID regulation.

## 9. Testing recommendations

- **Speed round-trip**: Set S-value, verify the IO output value via `range_speed(..., 0)`,
  convert back via `range_speed(..., 1)`, verify the original S-value is recovered.
- **Direction**: Verify M3 produces positive `set_speed` values, M4 negative, M5 zero.
- **Tool change** (multi-tool): Verify `startup_code` runs on load and `shutdown_code`
  on unload. Verify the tool struct is correctly copied from ROM to RAM.
- **Coolant**: Verify M7 sets mist, M8 sets flood, M9 clears both.
- **PID** (if applicable): Verify the PID loop runs at the configured sample rate and
  adjusts output toward the setpoint.
- **Mode flags**: Verify `tool_get_mode()` returns the expected mode after startup
  and is reset after shutdown.

## 10. Existing backends reference

### spindle_pwm
- PWM speed + direction pin + coolant + optional PID ($300–$302)
- `startup_code`: configure PWM at 1kHz, load PID settings
- `range_speed`: linear 0–255 ↔ 0–max_rpm
- `set_speed`: set direction pin then PWM value
- Mode: none (default spindle mode)

### laser_pwm
- PWM-only laser with configurable frequency (default 8kHz) and minimum power
- `startup_code`: configure PWM frequency, set mode to `PWM_VARPOWER_MODE`
- `shutdown_code`: `tool_reset_mode()`
- `range_speed`: scaled with `LASER_PWM_MIN_VALUE` offset
- Guarded with `#if defined(ENABLE_LASER_PWM)`

### laser_ppi
- Pulse-per-inch laser using an additional stepper for PPI signal
- Supports PPI, PPI-PW, and PPI-mixed modes ($32=2,4,6)
- Extra M-codes: M126 (mode), M127 (PPI), M128 (pulse width)
- Guarded with `#if defined(ENABLE_LASER_PPI)`

### spindle_relay
- Simple on/off relay control with direction (FWD/REV) and coolant
- `range_speed`: returns 255 if value > 0, -255 if value < 0, 0 if stopped
- `set_speed`: sets relay on/off based on direction
- `pid_update`: NULL

### spindle_besc
- BESC-controlled brushless via servo signal + power relay
- `range_speed`: scales S-value (0–255) to servo pulse
- Configurable: `SPINDLE_BESC_SERVO`, `SPINDLE_BESC_POWER_RELAY`

### pen_servo
- Pen up/down via servo signal at 50Hz
- Configurable: `PEN_SERVO` pin, `PEN_SERVO_LOW` (down), `PEN_SERVO_HIGH` (up)
- `range_speed`/`set_speed`: toggle between low/high positions
- Mode: sets laser mode for Z-axis motion control during drawing

### vfd_pwm
- PWM speed + direction + analog RPM feedback + PID ($304–$306)
- Same pattern as spindle_pwm but with `VFD_PWM_ANALOG_FEEDBACK` input
- `get_speed` can return true RPM from analog feedback
- Guarded with `#if defined(ENABLE_VFD_PWM)`

### vfd_modbus
- Modbus RTU over softuart, supports Huanyang Type1/2, YL620
- Requires softuart module (`ENABLE_SOFTUART`)
- Complex VFD command table format for different brands
- Communication timeout/retry, error hold, configurable address and baudrate
- Guarded with `#if defined(ENABLE_VFD_MODBUS)`

### plasma_thc
- Plasma torch with THC: up/down sense inputs, arc-ok, voltage divider
- Z-axis stepper control for torch height during cuts
- Virtual pin for THC enable (>64)
- Mode: `PLASMA_THC_MODE`
- Guarded with `#if defined(ENABLE_PLASMA_THC)`

### embroidery_stepper
- Embroidery machine with stitch synchronization
- Uses an additional stepper for needle control
- Mode: `EMBROIDERY_MODE`
- Guarded with `#if defined(ENABLE_EMBROIDERY_STEPPER)`