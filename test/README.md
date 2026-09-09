# Grbl conformity tests (Windows emulator)

These Unity fixtures exercise the default µCNC three-axis Cartesian build as a
Grbl 1.1 black box. Commands enter through the test stream and most assertions
are made against the resulting Grbl serial transcript. Motion, spindle, probe,
and limit fixtures additionally inspect the virtual MCU state where the serial
protocol cannot prove a physical output transition.

Each `test_grbl_*` directory is compiled and launched by PlatformIO as a
separate native Windows process. This isolates controller globals, planner
state, timers, streams, and RAM-only settings between fixture groups.

Run the complete baseline suite with:

```powershell
C:\Users\JCEM\.platformio\penv\Scripts\pio.exe test -e EMULATOR_WINDOWS_TEST
```

Run one fixture with, for example:

```powershell
C:\Users\JCEM\.platformio\penv\Scripts\pio.exe test `
  -e EMULATOR_WINDOWS_TEST `
  -f test_grbl_motion
```

## Fixture coverage

- `test_domain1_a_lexical`: all 24 D1-A preprocessing and numeric lexical cases.
- `test_domain1_b_supported`: all 53 D1-B supported-command cases, capability-gated where required.
- `test_domain1_c_rejection`: all 26 D1-C unsupported-command and modal-conflict cases.
- `test_domain1_d_parameters`: all 23 D1-D missing, invalid, and unused-parameter cases.
- `test_domain1_e_modal`: all 16 D1-E modal-report cases.
- `test_grbl_protocol`: startup, status, help, modal, coordinate, and settings reports.
- `test_grbl_motion`: absolute, incremental, linear, and arc endpoint validation.
- `test_grbl_spindle`: M3/M4/M5 modal behavior and virtual PWM/direction outputs.

## Domain 2 motion conformance

Domain 2 fixtures execute motion in a fresh default three-axis Cartesian emulator and validate
reported `MPos` endpoints, sampled line/arc geometry, coordinate transforms, probing, stored
positions, tool outputs, program flow, and feed constraints:

- `test_domain2_a_linear`: status reporting, G0/G1, absolute/incremental modes, units, and G80.
- `test_domain2_b_arcs`: G2/G3 IJK and radius arcs in G17/G18/G19 plus helical motion.
- `test_domain2_c_coordinates`: G54-G59, G10 L2/L20, G53, G92, and G92.1.
- `test_domain2_d_stored_tool`: G28/G30 stored positions, intermediate legs, G43.1, and G49.
- `test_domain2_e_probing`: G38.2-G38.5 contact/release and failure semantics.
- `test_domain2_f_tools`: spindle PWM/direction, coolant capability behavior, and program end.
- `test_domain2_g_program_feed`: dwell, pause/resume, status feed, and maximum-rate constraints.

Trajectory assertions use status samples rather than fixed completion delays. Receipt of `ok`
only acknowledges that Grbl accepted a line; every motion assertion synchronizes on `<Idle>`.

## Domain 3 controller-state conformance

Domain 3 uses a persistent controller service loop within each fresh PlatformIO fixture and tests
serial framing, status content, settings, Check/Sleep/startup modes, jogging, realtime state
transitions, overrides, physical control inputs, limits, probe/homing alarms, recovery, and the
state-dependent command acceptance matrix:

- `test_domain3_a_interface`: reset/query responses, status framing, partial lines, and realtime interleaving.
- `test_domain3_b_settings`: standard setting read/write coverage, validation, and dependencies.
- `test_domain3_c_modes`: Check, unlock, Alarm recovery, Sleep, and startup blocks.
- `test_domain3_d_jog`: validation, locking, cancellation, queueing, and modal isolation.
- `test_domain3_e_realtime`: hold/resume, reset, safety door, and physical hold/start inputs.
- `test_domain3_f_overrides`: feed, rapid, spindle, and coolant realtime overrides.
- `test_domain3_g_limits_alarms`: hard/soft limits and alarms 1-5 with command locking.
- `test_domain3_h_homing`: disabled homing and homing alarms 6, 7, and 9.
- `test_domain3_i_state_matrix`: Idle/Run/Hold/Jog/Alarm/Check command acceptance and pin reporting.
- `test_grbl_inputs`: probing and hard-limit input behavior.
- `test_grbl_jog_realtime`: jog state and realtime jog cancellation.
- `test_grbl_feed_hold`: realtime feed hold and cycle-start/resume.

The five Domain 1 fixtures execute all 142 cases from D1-A through D1-E. Every
case begins with a soft reset and a fresh Check-mode session. Responses use
exact Grbl status codes, rejected commands are checked for modal-state
preservation, and every mismatch is reported with its specification case ID.
Table-driven mismatches are aggregated so one non-conforming command does not
prevent the remaining cases in that section from executing.

## Current scope

This is the default-configuration baseline, not a claim of mathematical
exhaustiveness. Configuration-dependent features such as homing (disabled by
default), safety-door parking, mist coolant, extra axes, and alternative tools
need separate build profiles before their expected behavior can be asserted.
Physical timing/electrical characteristics also remain hardware-in-the-loop
tests rather than emulator tests.
