# µCNC — Agent Guide

µCNC is CNC firmware for resource-constrained 8/16/32-bit microcontrollers. Most
production code is C compiled as GNU C99; platform adapters may be C++. Arduino,
PlatformIO, standalone Makefiles, many MCU families, and native Windows/Linux
emulators share the same core.

This file contains the workflow and invariants that apply across the repository.
Read subsystem documentation only when the task enters that subsystem.

## Before Editing

1. Identify the smallest subsystem and compile-time configurations affected.
2. Read its public header and the routed documentation below. Treat headers,
   build configuration, and tests as authoritative when prose disagrees.
3. Search for an existing implementation pattern before adding an abstraction.
4. Make a minimal, single-purpose change and preserve the surrounding file's
   style. Do not mix cleanup or bulk formatting into functional work.
5. Verify the narrow behavior first, then build or test the configurations that
   can exercise the changed code.
6. Review the final diff. Report commands run, results, and any relevant target
   or hardware coverage that remains unverified.

## Repository Map

| Path | Responsibility |
|---|---|
| `uCNC/cnc_config.h` | User-facing machine and feature configuration |
| `uCNC/cnc_hal_config.h` | Hardware wiring, tools, motors, limits, and advanced HAL configuration |
| `uCNC/*_overrides.h` | User override points; stubs by default |
| `uCNC/src/cnc.[ch]` | Controller state, initialization, run loop, alarms, and realtime work |
| `uCNC/src/core/` | Parser, planner, interpolator, motion, and I/O control |
| `uCNC/src/interface/` | Grbl protocol, streams, settings, printing, and defaults |
| `uCNC/src/hal/boards/` | Board IDs, boardmaps, and PlatformIO environments |
| `uCNC/src/hal/mcus/` | MCU interface, MCU selection, ports, and virtual MCU |
| `uCNC/src/hal/kinematics/` | Coordinate/step transforms and homing strategies |
| `uCNC/src/hal/tools/` | Tool interface and spindle/laser/plasma/tool implementations |
| `uCNC/src/modules/` | Optional extension modules |
| `test/` | Native Unity fixtures; each `test_*` directory is a separate process |
| `test/common/` | Shared Grbl and Domain 1/2/3 test helpers |
| `makefiles/` | Standalone builds and their platform support files |

The Arduino entry points call `ucnc_init()` and `ucnc_run()`, which normally map
to `cnc_init()` and `cnc_run()`. Keep the `ucnc_*` indirection intact: some
multicore MCU ports override it.

Compile-time configuration is assembled by
`uCNC/src/cnc_hal_config_helper.h`. Its include order is authoritative and must
not be reordered casually. Board selection resolves through
`hal/boards/boards_helper.h`; MCU IDs live in `hal/mcus/mcus.h`, while
MCU-specific definitions are selected by `hal/mcus/mcudefs.h`.

## Task Routing

Read these only when the task takes the corresponding branch:

- **Tests or Grbl conformance:** `test/README.md`, then the relevant helper in
  `test/common/` and a neighboring fixture.
- **Events, hooks, or modules:** `uCNC/src/README.md`, `uCNC/src/module.h`, and
  `uCNC/src/modules/README.md` for module-specific work. Find the actual event
  inventory by searching for `DECL_EVENT_HANDLER`.
- **Kinematics:** `uCNC/src/hal/kinematics/README.md` and
  `uCNC/src/hal/kinematics/kinematic.h`.
- **MCU porting:** `uCNC/src/hal/mcus/README.md`, the nearest existing MCU port,
  `uCNC/src/hal/mcus/mcudefs.h`, and that architecture's board `.ini`.
- **Tools:** `uCNC/src/hal/tools/README.md`, `uCNC/src/hal/tools/tool.h`, and the
  nearest tool implementation.
- **Networking:** the relevant implementation under `uCNC/src/modules/net/`;
  read `uCNC/src/modules/net/socket.md` for socket lifecycle and threading
  constraints.
- **Standalone builds:** the target directory under `makefiles/` and its
  README or Makefile. Do not assume PlatformIO flags transfer unchanged.

## Build and Test Commands

`.github/workflows/` is authoritative for the PlatformIO version and current CI
matrix.

```sh
# All native tests on Linux
pio test -e EMULATOR_LINUX_TEST

# All native tests on Windows
pio test -e EMULATOR_WINDOWS_TEST

# One fixture (works with the matching host emulator environment)
pio test -e EMULATOR_WINDOWS_TEST -f test_grbl_motion

# One firmware target
pio run -e STM32F4-Blackpill-F401CC
```

Standalone examples:

```sh
make -C makefiles/stm32f4x BOARD=BOARD_BLACKPILL
make -C makefiles/stm32f1x BOARD=BOARD_BLUEPILL
make -C makefiles/samd21 BOARD=BOARD_MZERO
```

Choose verification by impact:

| Change | Minimum useful verification |
|---|---|
| Parser, protocol, state machine, or motion behavior | Closest native fixture; add a regression case for a bug fix |
| Shared test helper or virtual MCU | Affected fixture plus neighboring fixtures that use the helper |
| Boardmap or one MCU port | Build the affected PlatformIO environment |
| Shared HAL or compile-time configuration | Native tests plus representative affected board builds |
| Documentation/comment-only | Inspect links, commands, and diff; code tests are not automatically required |

Prefer a targeted fixture during iteration. Run broader coverage when the
change crosses subsystem or configuration boundaries. Hardware timing and
electrical behavior require hardware-in-the-loop validation; never claim the
native emulator proves them.

## Test Invariants

- `PIO_UNIT_TESTING` enables controlled controller stepping and the virtual MCU
  test interfaces. Tests call `cnc_unit_test_start()` and repeatedly call
  `cnc_unit_test_run_once()`; they do not replace the production implementation
  of `cnc_run()`.
- Use `grbl_test_start()` at the beginning of a Grbl fixture case. It initializes
  the controller once per process and resets controller, parser, settings,
  virtual-runtime, and transcript state for each call. Do not duplicate that
  reset sequence in individual tests.
- Each `test/test_*` directory is a separate PlatformIO executable, isolating
  globals, planner state, timers, and RAM-backed settings between fixtures.
- Grbl `ok` acknowledges command acceptance, not motion completion. Motion
  assertions must synchronize on an `<Idle` status before checking endpoints or
  trajectories.
- Test time is virtual. `grbl_test_wait_for()` advances it with
  `mcu_unit_test_advance_time()` and evaluates deadlines through `mcu_millis()`.
  A test that depends on wall-clock sleeps is usually testing the wrong clock.
- Realtime commands are bytes (`!`, `~`, `?`, and `0x18` for soft reset) and
  bypass normal G-code line parsing.
- Domain 1 validates parsing in Check mode, Domain 2 samples motion behavior,
  and Domain 3 exercises persistent controller states. Reuse the corresponding
  helper rather than rebuilding its lifecycle locally.

## Cross-Cutting Engineering Rules

- **Correctness and machine safety come first.** Consider limits, alarms,
  unexpected input, partial state transitions, and lost-position behavior.
- **Keep embedded paths deterministic.** Avoid dynamic allocation, recursion,
  unbounded work, and unnecessary floating-point operations in the core and
  timing-critical MCU paths. Platform-specific filesystem, networking, and
  native-emulator code has established allocation patterns; follow the local
  ownership and failure-handling convention rather than applying a blanket ban.
- **Keep ISRs lean.** Perform only timing-critical register/flag/counter work in
  interrupt context. Defer parsing, planning, protocol output, and other
  non-critical work to the main loop. Protect ISR-shared data with the existing
  atomic facilities.
- **Use existing infrastructure.** Prefer `buffer.h` for queues, `atomic.h` for
  shared-state guards, and `utils.h` for common helpers. Search for a comparable
  MCU, board, kinematic, tool, module, or test before inventing a new pattern.
- **Respect constrained targets.** Communicate storage width intentionally and
  account for RAM, flash, stack, and cycle cost. Do not optimize blindly: retain
  clarity unless measurement or a timing constraint justifies complexity.
- **Preserve public and compile-time behavior.** Changes to headers, macros,
  configuration defaults, event signatures, pin numbering, or source filters
  can affect many targets even when the native emulator passes.
- **Test behavioral changes.** Bug fixes should add or update a case that fails
  before the fix. New behavior needs focused coverage. Do not manufacture a
  code test for prose-only or mechanically unverifiable changes.
- **Keep the diff focused.** Remove newly obsolete code instead of commenting it
  out, but do not delete historical or platform-specific branches merely because
  the active host does not compile them.

## Language and Style

- Production `.c` files use the repository's GNU C99 subset, including existing
  GNU attributes and statement-expression macros where required. Do not add
  C11/C23 dependencies without confirming every affected toolchain supports
  them.
- Platform adapters may be C++. Maintain `extern "C"` boundaries where C APIs
  are consumed from C++.
- In `uCNC/src/`, use 2-space indentation, K&R braces, `snake_case` identifiers,
  and `UPPER_CASE` macros/constants. Other areas contain older styles; preserve
  the local file unless the task is explicitly a formatting migration.
- Aim for readable lines near 100 columns, but do not distort macros, signatures,
  or strings solely to meet a number.
- Comments explain constraints and reasons. Prefer descriptive names for what
  the code does.
- There is no repository-wide clang-format configuration. Never run a bulk
  formatter based on an inferred style.

## Extension and Porting Reminders

- A board addition normally needs a `BOARD_*` ID and matching `BOARDMAP` branch
  in `uCNC/src/hal/boards/boards_helper.h`, a boardmap header, and an environment
  in the appropriate architecture `.ini`. Use the nearest supported board as
  the checklist.
- A new MCU family normally needs an ID in `uCNC/src/hal/mcus/mcus.h`, selection
  in `uCNC/src/hal/mcus/mcudefs.h`, its MCU/map implementation files,
  board/build integration, and exclusion from the Linux virtual emulator's
  `build_src_filter`. Compare every step with a current neighboring port.
- A kinematic addition must be registered in
  `uCNC/src/hal/kinematics/kinematics.h` and
  `uCNC/src/hal/kinematics/kinematicdefs.h` and implement the contract in
  `uCNC/src/hal/kinematics/kinematic.h`. Optional weak defaults and wrapper
  functions are documented there; do not copy signatures from this guide.
- Modules register through the event/hook infrastructure in `uCNC/src/module.h`
  and `uCNC/src/module.c`. External module download behavior is implemented by
  `ucnc_modules.py`; inspect the script before changing archive or overwrite
  semantics.

## Completion Check

Before declaring work complete, ensure that every affected configuration is
either verified or named as unverified, behavioral changes have proportional
tests, no unrelated files were reformatted, and the final diff contains only
the intended change.
