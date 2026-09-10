# µCNC — Agent Guide

µCNC is an open-source CNC firmware for 8/16/32-bit microcontrollers, written in C with Arduino framework compatibility. Targets AVR, ESP32/ESP32-S3/ESP32-C3, ESP8266, LPC176x, RP2040/RP2350, SAMD21, STM32F0/F1/F4/H7, and a virtual (Windows/Linux) emulator.

---

## Essential Commands

**Run all tests (Linux emulator):**
```sh
pio test -e EMULATOR_LINUX_TEST
```

**Run one test fixture (Windows):**
```powershell
pio test -e EMULATOR_WINDOWS_TEST -f test_grbl_motion
```

**Build for a specific board:**
```sh
pio run -e STM32F4-Blackpill-F401CC
```

**Standalone Makefile builds** (no PlatformIO needed):
```sh
make -C makefiles/stm32f4x BOARD=BOARD_BLACKPILL
make -C makefiles/stm32f1x BOARD=BOARD_BLUEPILL
make -C makefiles/samd21  BOARD=BOARD_MZERO
```

**PlatformIO pinned version** (CI uses `6.1.18`):
```sh
pip install "platformio==6.1.18"
```

---

## Architecture

```
User Config Layer
  uCNC/build_opt.h           — Compile-time defines (e.g. disable unused STM32 HAL modules)
  uCNC/cnc_config.h          — Machine config: AXIS_COUNT, KINEMATIC, TOOL_COUNT, etc.
  uCNC/cnc_hal_config.h      — HAL hardwired connections, multi-motor axes, tool config
  uCNC/boardmap_overrides.h  — Custom pin mapping overrides (stub by default)
  uCNC/cnc_hal_overrides.h   — HAL config overrides (stub by default)
      │
      ▼
Core Layer (uCNC/src/)
  cnc.h / cnc.c              — Main state machine, init, run loop, realtime commands
  module.h / module.c        — Event system: DECL_EVENT_HANDLER / WEAK_EVENT_HANDLER / CREATE_EVENT_LISTENER
  buffer.h / buffer.c        — Lock-free SPSC circular buffer with atomic guards
  atomic.h                   — ATOMIC_CODEBLOCK macro (disable ints, auto-restore via __cleanup__)
  core/
    parser.c / parser.h      — G-code parser (Grbl-compatible)
    parser_expr.c            — Parser expression evaluation
    planner.c / planner.h    — Motion planner (trapezoidal S-curve)
    interpolator.c/.h        — Step pulse interpolator
    motion_control.c/.h      — Motion control commands (G0/G1/G2/G3/etc.)
    io_control.c / io_control.h — I/O control (coolant, spindle, limits, probe)
  interface/
    grbl_protocol.c/.h       — Grbl serial protocol (send-response format)
    grbl_settings.c/.h       — Grbl-compatible $ setting system
    grbl_stream.c/.h         — Stream abstraction (serial/telnet/unit test)
    grbl_print.c/.h          — Formatted output (ftoa, itoa)
    grbl_interface.h         — All Grbl status codes, alarm codes, realtime commands
    defaults.h               — Default setting values
      │
      ▼
HAL Layer (uCNC/src/hal/)
  io_hal.h                   — Pin-level macros: maps STEP0/DIR0/LIMIT_X/SPINDLE/etc. to
                               either mcu_*() GPIO functions or ic74hc595_*() shift-register functions
  boards/                    — Board pin definitions (boardmap_*.h) + PlatformIO board .ini files
  mcus/
    mcu.h / mcu.c            — Abstract interface: mcu_config_input, mcu_set_output, mcu_add_event, etc.
    mcudefs.h                — MCU-specific config (timers, pins)
    mcus.h                   — MCU type enum (MCU_AVR=1, MCU_STM32F1=2, etc.)
    virtual/                 — Virtual MCU for host-based testing
    avr/, esp32/, stm32*/, etc. — Per-MCU ports
  kinematics/
    kinematic.h / kinematic.c — Generic wrapper with skew compensation
    kinematic_cartesian.c     — Cartesian kinematics
    kinematic_corexy.c        — CoreXY kinematics
    kinematic_delta.c         — Delta kinematics
    kinematic_scara.c         — SCARA kinematics
    kinematic_linear_delta.c  — Linear delta kinematics
    kinematic_rtheta.c        — R-Theta kinematics
    kinematic_dummy.c         — Null kinematic (for debugging)
  tools/
    tool.h / tool.c           — tool_t struct with function pointers
    tools/
      spindle_pwm.c           — PWM spindle speed control
      spindle_relay.c         — Relay spindle on/off
      spindle_besc.c          — BESC spindle control
      laser_pwm.c             — Laser PWM power control
      laser_ppi.c             — Laser PPI (pulses per inch)
      plasma_thc.c            — Plasma torch height control
      vfd_modbus.c            — VFD Modbus control
      vfd_pwm.c               — VFD PWM control
      pen_servo.c             — Pen servo control
      embroidery_stepper.c    — Embroidery stepper control
  modules/                   — Optional extension modules (encoders, soft I2C/SPI/UART, PID, Modbus, file system, etc.)
```

**Include ordering** (enforced by `cnc_hal_config_helper.h`):
1. `cnc_build.h` — version, feature flags
2. `hal/mcus/mcus.h` — MCU HAL abstraction
3. `hal/kinematics/kinematics.h` — kinematics definitions
4. `../cnc_config.h` — user machine config
5. `hal/boards/boarddefs.h` — board pin IO + service ISRs
6. `hal/kinematics/kinematicdefs.h` — kinematic selection
7. `hal/tools/tool.h` — tool interface
8. `../cnc_hal_config.h` — HAL wiring
9. `../cnc_hal_overrides.h` — override file
10. `modules/shift_register.h` — IO extender

This ordering is critical — changing it can break half the build targets silently.

---

## Main Control Flow

```
setup() → cnc_init()
  ├── mcu_init()            — MCU clock, GPIO, timer setup
  ├── mcu_io_reset()        — Pin initial state
  ├── io_enable_steppers()  — Steppers disabled at startup
  ├── grbl_stream_init()    — Serial/telnet stream init
  ├── settings_init()       — Load settings from NVM
  ├── cnc_network_init()    — WiFi/BT/telnet init
  ├── mod_init()            — Module init (event listeners register)
  ├── itp_init()            — Interpolator init
  ├── planner_init()        — Motion planner init
  └── tool_init()           — Tool interfaces init

loop() → cnc_run()
  └── cnc_reset() → for(;;)
        ├── cnc_parse_cmd()          — Read & execute G-code
        ├── cnc_dotasks()            — Interpolator step, I/O scan, module events
        └── alarm handling           — Limits, kill, soft-reset
```

Under test (`PIO_UNIT_TESTING`), `cnc_run()` is replaced by controlled stepping:
```
cnc_init() → cnc_unit_test_start() → for each step: cnc_unit_test_run_once()
```
Each `cnc_unit_test_run_once()` call executes exactly one iteration of the main loop.

---

## Module / Event System

Two mechanisms for extending core behavior:

### Event Listeners (linked list)
```c
DECL_EVENT_HANDLER(cnc_reset);           // declares the event type
WEAK_EVENT_HANDLER(cnc_reset) { ... }    // default handler
CREATE_EVENT_LISTENER(cnc_reset, mymod); // registers a listener
ADD_EVENT_LISTENER(cnc_reset, mymod);    // links into chain
EVENT_INVOKE(cnc_reset, args);           // fires the event
```

Events available: `cnc_reset`, `cnc_dotasks`, `cnc_io_dotasks`, `cnc_stop`, `cnc_parse_cmd_error`, `cnc_alarm`.

Lock guards prevent re-entrancy (`LISTENER_HWSPI_LOCK`, `LISTENER_HWI2C_LOCK`, etc.).

### Hooks (simpler single-callback)
```c
DECL_HOOK(name, arg_type1, arg_type2);  // declares typed hook
CREATE_HOOK(name);                       // defines storage
HOOK_ATTACH_CALLBACK(name, my_func);     // set callback
HOOK_INVOKE(name, arg1, arg2);           // call it
HOOK_RELEASE(name);                      // clear callback
```

Hooks have an ISR-safe variant: `HOOK_INVOKE_ISR()`.

### Modules
- `LOAD_MODULE(name)` — calls `name##_init()` at startup
- Module init happens in `mod_init()` (called from `cnc_init()`)
- Add new modules in `module.c` with `#ifdef` guards
- External modules can be downloaded via `ucnc_modules.py` (zip from URL → extracts to `src/modules/`)

---

## IO HAL System (`io_hal.h`)

**Critical pattern**: IO is compiled-time resolved between direct GPIO and shift-register via `ASSERT_PIN_IO()` vs `ASSERT_PIN_EXTENDED()`.

```c
#if ASSERT_PIN_IO(STEP0)
  #define io1_set_output mcu_set_output(STEP0)
#elif ASSERT_PIN_EXTENDED(STEP0)
  #define io1_set_output ic74hc595_set_pin(STEP0)
#endif
```

Each logical signal (STEP0-7, DIR0-7, LIMIT_X/Y/Z, PROBE, SPINDLE, COOLANT, etc.) gets `ioN_*` macros numbered from 1 upward. Adding a new signal means adding to this chain.

**Multi-stepper axes**: `STEP0_MASK` can combine multiple stepper pins (e.g. `STEP0_MASK = STEPPER0_IO_MASK | STEPPER5_IO_MASK`) for ganged/dual-drive axes. Limits should match for auto-squaring.

**Weak symbols** (`__attribute__((weak))`) are used extensively in `mcu.c` for default implementations that per-MCU ports can override.

---

## Testing (Virtual MCU Emulator)

All tests run on the host (Windows/Linux) via a virtual MCU. No hardware needed.

**Test environments:**
- `EMULATOR_WINDOWS_TEST` — MinGW, runs native Windows EXEs
- `EMULATOR_LINUX_TEST` — GCC, runs native Linux ELFs

**Compile-time test flags:**
- `PIO_UNIT_TESTING` — enables the virtual MCU test interface
- `DISABLE_SAFE_SETTINGS` — removes safety checks for test-driven transitions
- `DISABLE_ENDPROGRAM_LOCK` — allows state resets between test cases
- `EMULATE_GRBL_STARTUP=3` — forces Grbl-style startup banner

**Virtual MCU test API:**
- `mcu_unit_test_inject(cmd)` — injects string into RX stream
- `mcu_unit_test_buffer()` — full serial transcript
- `mcu_unit_test_buffer_clear()` — reset transcript
- `mcu_unit_test_advance_time(us)` — advance virtual clock
- `mcu_add_event(us, callback, arg)` — schedule virtual timer event
- `test_io_set() / test_io_set_after() / test_io_set_callback()` — mock IO signals
- `mcu_unit_test_runtime_reset()` — reset runtime state between cases

**Test infrastructure** (`test/common/`):

| Helper | Purpose | Used by |
|---|---|---|
| `grbl_test.h` | Foundational: `grbl_test_start/stop()`, `grbl_test_wait_for()`, `grbl_test_command_expect/ok()`, `grbl_test_realtime_expect()`, `grbl_test_wait_for_state()` | All Grbl-conformity tests |
| `domain1_test.h` | `d1_case_t` table, `D1_CASE_FIXTURE`, `d1_run_current_case()` | D1 lexical/supported/rejection/parameters/modal tests |
| `domain2_test.h` | `d2_sample_t`, `d2_trace_motion()`, `d2_assert_line()`, `d2_assert_arc()` | D2 motion/trajectory tests |
| `domain3_test.h` | `d3_command()`, `d3_status()`, `d3_wait_state()`, `d3_realtime()`, `d3_reset()`, `d3_expect_no_serial_response()` | D3 interface/modes/jog/realtime/overrides/limits/homing tests |

**Test patterns:**

| Pattern | How | When |
|---|---|---|
| **Single-function fixture** | One `RUN_TEST()`, wrapped in `GRBL_PROCESS_FIXTURE` | Simple conformance checks (protocol, motion, feed hold) |
| **Low-level direct API** | `cnc_init()` → `cnc_unit_test_start()` → step with `cnc_unit_test_run_once()` | Testing controller internals (step-by-step execution) |
| **Table-driven D1** | `d1_case_t[]` array, `D1_CASE_FIXTURE` iterates with per-case isolation | G-code lexical/semantic conformance (24-53 cases per fixture) |
| **Trace-sampled motion** | `d2_trace_motion()` records trajectory, validates path geometry | Motion endpoint + interpolation validation |
| **State-machine assertion** | `d3_expect_state()`, `d3_realtime()`, persistent controller loop | Real-time command handling, state transitions |
| **Hardware emulation** | Direct virtual MCU calls (`mcu_micros()`, `mcu_add_event()`) | Timer/clock behavior verification |

**Gotcha**: `setUp()` / `tearDown()` are Unity fixture hooks — they run before/after each `RUN_TEST()`. `grbl_test_start()` calls `cnc_init()` but skips re-init if already initialized (singleton-like behavior). Reset state between cases with `cnc_stop()` + `settings_reset()` + `mcu_unit_test_runtime_reset()`.

**Gotcha**: Trajectory assertions use status samples, not completion delays. Receiving `ok` means the command was accepted, not executed. Always synchronize on `<Idle>` for motion verification.

**Each test directory** (`test/test_*/`) is compiled as a **separate native process** by PlatformIO. This isolates controller globals, planner state, timers, and RAM-only settings between fixture groups.

---

## Kinematics System

Each kinematic is a `.c/.h` pair implementing six functions:
```c
kinematics_apply_inverse(target, joint);          // Cartesian → joint space
kinematics_apply_forward(joint, target);           // Joint → Cartesian space
kinematics_home();                                 // Homing sequence
kinematics_apply_transform(pos, input_is_machine); // Coordinate transform
kinematics_apply_reverse_transform(pos);           // Inverse transform
```

Selected at compile time: `#define KINEMATIC KINEMATIC_CARTESIAN` in `cnc_config.h`.

The wrapper `kinematic.c` adds transparent skew compensation to Cartesian.

**Gotcha**: The default config uses Cartesian. If you add a new kinematic, you must add entries to both `kinematicdefs.h` (the `#if KINEMATIC == KINEMATIC_xxx` chain) and `kinematics.h` (the kinematic type enum).

---

## Tool System

`tool_t` is a struct of function pointers:
```c
typedef struct {
    void (*startup_code)(void);
    void (*shutdown_code)(void);
    uint8_t (*pid_update)(int16_t target, int16_t current);
    uint16_t (*range_speed)(uint16_t rpm);
    uint16_t (*get_speed)(void);
    void (*set_speed)(uint16_t rpm);
    void (*set_coolant)(uint8_t value);
} tool_t;
```

Multiple tools can coexist via `TOOL_COUNT` and are multiplexed by `g_tool_idx` in settings. Each tool indexes into the `TOOL_CONFIG` section of `cnc_hal_config.h`.

---

## Code Style & Conventions

- **Language**: C with `extern "C"` guards for Arduino C++ compatibility
- **Naming**: `snake_case` for functions and variables, `UPPER_CASE` for macros and defines
- **Includes**: No include guards on `.c` files; `.h` files use `#ifndef NAME_H / #define NAME_H / #endif`
- **Error handling**: State-machine based (`cnc_state.exec_state` bitmask), no GOTO
- **Memory**: Minimal dynamic allocation — buffers are statically declared. Most arrays are fixed-size.
- **Inline hints**: `FORCEINLINE` → `__attribute__((always_inline)) inline`
- **Weak defaults**: `__attribute__((weak))` for overridable function implementations
- **Version string**: `CNC_MAJOR_MINOR_VERSION` "1.17" + `CNC_PATCH_VERSION` ".0" in `cnc_build.h`
- **Module version**: `UCNC_MODULE_VERSION` 11700 (numeric, used for compatibility checks)
- **Flash storage**: Abstracted behind `__rom__`, `__romstr__`, `rom_read_byte` macros — per-MCU ports define these to pgm_read_byte (AVR) or direct access (ARM)

---

## Code Rules (Guidelines)

These are aspirational principles, not strict enforcement rules. Prioritize them in order: correctness, resource safety, readability, consistency.

**Formatting:**
- **C99** — write to the C99 standard. No C11/C23 features unless unavoidable for a specific MCU toolchain. Declare variables at block scope, use `//` and `/* */` comments, rely on C99's designated initializers and compound literals where they improve clarity.
- **2-space indent, no tabs** — indent by 2 spaces. Never use tab characters. Configure your editor to convert tabs to spaces.
- **Names** — `snake_case` for all functions, variables, and type names; `UPPER_CASE` for macros, defines, and enum constants. Follow the existing file's naming rhythm.
- **Brace style** — K&R style (opening brace on the same line as the statement) for functions, `if`, `for`, `while`; Allman style (brace on its own line) only where the surrounding file uses it. Match the file.
- **Line length** — aim for 100 characters max. Longer lines are acceptable when they improve readability (e.g., long string literals, complex macro chains).
- **clang-format** — if using clang-format, prefer `BasedOnStyle: LLVM` with the above overrides (2-space indent, no tabs, 100 column limit).

**Principles:**

- **Correctness first** — the machine must not crash or skip steps. A readable wrong answer is still wrong. When in doubt, favor the safer, more defensive path.
- **Resource awareness** — µCNC runs on 8-bit MCUs with KBs of RAM. Every byte and cycle counts. Favor statically-allocated buffers, avoid recursion, and prefer fixed-point math over floating-point where precision allows. Prefer `uint8_t`/`int16_t` over `int` to communicate size intent.
- **Readable over clever** — prefer simple loops, flat conditionals, and straightforward logic over bit tricks or macro wizardry. Code clarity aids debugging on hardware without a debugger.
- **Consistency with surroundings** — match naming, indentation, brace style, and error handling patterns of the file you are editing, even if they differ from your personal preference.
- **Minimal changes, single purpose** — change only what the task requires. Don't fix unrelated formatting, rename symbols, or refactor nearby code. Keeps diffs reviewable and bisectable.
- **Test what you touch** — every change should include or update tests. Fixes require a test that would have caught the regression; new features require basic smoke tests. Use the existing test infrastructure (`test/common/`) and patterns.
- **Prefer built-in infrastructure** — use the project's existing implementations before writing your own:
  - **Buffers/circular queues** → `./uCNC/src/buffer.h` (lock-free SPSC, both macro and generic variants)
  - **Atomic operations** → `./uCNC/src/atomic.h` (`ATOMIC_CODEBLOCK`, atomic guard macros)
  - **Utility macros** → `./uCNC/src/utils.h` (tested helpers for common patterns)
  - These are already tested, match the codebase style, and avoid subtle single-implementation bugs.
- **No dynamic allocation** — never use `malloc`, `calloc`, `realloc`, `free`, or `alloca`. All memory must be statically allocated at compile time. This eliminates fragmentation, OOM crashes, and non-deterministic timing on memory-constrained MCUs.
- **ISRs must be lean** — keep interrupt service routines minimal: set a flag, increment a counter, copy a register. Never call parser, planner, IO, or tool functions from inside an ISR. Use `ATOMIC_CODEBLOCK` for shared data that ISRs and main code both access.
- **Main loop for non-critical work** — all non-time-critical logic (G-code parsing, planning, IO scanning, tool management, serial protocol) belongs in `cnc_run()` or its callees. Don't offload work to timer callbacks or ISRs unless the timing demand is unavoidable.
- **No dead code** — don't leave commented-out blocks, orphaned `#if 0` sections, or unused functions/variables. Delete them. If you need the code later, git history preserves it.
- **Error paths matter** — always consider what happens when a function receives unexpected input, a sensor reads out of range, or memory is exhausted. Return a status code, set an alarm, or fail gracefully — but don't silently proceed with corrupt state.
- **Self-documenting over comments** — prefer descriptive names that make the code speak for itself. A comment should explain *why*, not *what*. Follow the existing project pattern: minimal comments, focused on intent.
- **Single responsibility per function** — if a function does more than its name suggests, split it. This is especially important for motion control, planner, and IO code where state interactions are subtle.

---

## Board / MCU Porting

Adding a new board:
1. Create `boardmap_<board>.h` defining all pin assignments (STEP0..STEP7, DIR0..DIR7, LIMIT_X/Y/Z, PROBE, etc.)
2. Add to `boards_helper.h` — add `BOARD_xxx` enum entry and `BOARD_TO_BOARDMAP` mapping
3. Add PlatformIO `.ini` in the appropriate `hal/boards/<arch>/` directory
4. Optional: create `custom_variants/<board>/` for variant-specific overrides

Adding a new MCU family:
1. Create `mcu_<family>.c` and `mcumap_<family>.h` in `hal/mcus/<family>/`
2. Implement all functions from `mcu.h` interface
3. Add to `mcus.h` enum and `mcus.h` includes
4. Edit `virtual.ini` or the relevant board `.ini` to exclude the new MCU from emulator builds

**Virtual MCU pin numbering** (`mcumap_virtual.h`) uses simple integers (1-200+). `DIO_N` = pin number. PWM and SERVO pins are also generic integers. The `VIRTUAL_MAP` bitfield union maps these to readable/writable bits.

---

## Configuration System

Two-level:
- `cnc_config.h`: **User-facing** — AXIS_COUNT, KINEMATIC, BAUDRATE, TOOL_COUNT, feature enables (laser, parking, limits, homing, etc.)
- `cnc_hal_config.h`: **Hardware wiring** — pin connections, per-axis limit pullup, multi-stepper axes, auto-squaring, tool type (spindle PWM/VFD/laser/plasma), encoder config, PID

Both are in the `uCNC/` directory (not `src/`), treated as the "user modifiable" area.

`build_opt.h` provides raw compiler flags for STM32 HAL module toggling (`-DHAL_TIM_MODULE_DISABLED`).

---

## Gotchas & Non-Obvious Patterns

1. **`pio test` expects test source in the repo root `test/` directory.** Each subdirectory is a separate PlatformIO test executable. The `test_build_src = yes` flag compiles the main source *into each test*, so each test has its own copy of globals.

2. **`grbl_test_start()` is not re-entrant.** It calls `cnc_init()` which is guarded to run only once. To reset between multiple test functions in a single fixture, stop with `cnc_stop()` + reset settings + runtime state manually. Using the `D3_MAIN()` pattern with separate test processes avoids this.

3. **Motion commands must be synchronized on `<Idle>`, not `ok`.** `grbl_test_command_ok()` waits for `ok\r\n` which only confirms G-code acceptance, not motion completion. Use `grbl_test_wait_for_state("<Idle", ...)` for motion synchronization.

4. **Virtual time vs real time.** Tests advance time via `mcu_unit_test_advance_time(us)`. Each test step advances 5000µs (5ms) in `grbl_test_step()`. Test timeouts are in real milliseconds but translated to virtual steps.

5. **Realtime commands are single bytes**, not strings. `!` = feed hold, `~` = cycle start, `?` = status report, `0x18` = soft reset. These bypass the G-code parser and go straight to the realtime command handler.

6. **Check mode (`$C`)** is used extensively in D1 tests. It enters a mode where G-code is parsed and validated but no motion executes. Exit via soft reset.

7. **Alarm codes** are signed: `EXEC_ALARM_NOALARM = 0`, negative values (e.g. `-EXEC_ALARM_HARD_LIMIT`) signal the main loop to clear the alarm after handling, positive values persist.

8. **`boardmap_overrides.h` and `cnc_hal_overrides.h` are stubs** — empty headers users can edit to override pin assignments or HAL config without modifying the main config files.

9. **External modules** can be auto-downloaded via `ucnc_modules.py` by setting `custom_ucnc_modules_url` and `custom_ucnc_modules` in `platformio.ini`. Already-present files are skipped.

10. **The emulator excludes all MCU HAL directories** via `build_src_filter` with `-<src/hal/mcus/avr>`, `-<src/hal/mcus/esp32>`, etc. Only `virtual/` compiles. Adding a new MCU family requires adding its exclusion to the Linux emulator's `build_src_filter`.

11. **`#ifdef PIO_UNIT_TESTING`** gates all test-specific code. In the virtual MCU, this enables the unit test TX buffer, injectable stream, in-memory EEPROM, and test IO signals. Without this flag, the virtual MCU uses real serial ports and file-backed EEPROM.

12. **Buffer implementations** — `buffer.h` provides two styles: macro-based (fast, fixed-size, `DECLARE_BUFFER`/`BUFFER_*`) and function-based (generic, `buffer_t`). Both use lock-free SPSC with atomic guards via `ATOMIC_CODEBLOCK`.

13. **`FORCE_GLOBALS_TO_0`** — if defined, `cnc_init()` memset's the entire state struct to 0 before initialization. This is useful for toolchains that don't zero BSS at startup.