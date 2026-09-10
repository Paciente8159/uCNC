# µCNC tool vocabulary

Use these terms when tool-layer documentation or discussion needs a shared vocabulary.
The current declarations and behavior in `tool.h` and `tool.c` remain authoritative.

## Layers

**Tool HAL** — The contract between the µCNC core and tool subsystem: `tool_t` plus
the management functions implemented in `tool.c`.

**Tool backend** — One hardware-specific implementation that exposes a `const tool_t`
symbol and its callbacks. Built-in backends live under
`uCNC/src/hal/tools/tools/`; a project may supply its own implementation.

**Tool palette** — The configured set of backend symbols assigned to `TOOL1` through
`TOOL16`. `TOOL_COUNT` controls how many entries participate in the build. Inspect
`cnc_hal_config.h` and the configuration helper for the current registration rules.

## Speed values

**Setpoint** — The requested G-code speed or power in user-facing units.

**Tool-IO value** — The hardware-level command passed to `set_speed`. Its magnitude
controls output; its sign carries direction; zero means stop.

**Measured speed** — Optional backend feedback returned by `get_speed`. When the
callback is absent, the Tool HAL reports the converted setpoint.

**Speed conversion** — The optional `range_speed` mapping between a non-negative
setpoint magnitude and a non-negative tool-IO magnitude. The Tool HAL uses identity
conversion when this callback is absent.

## Lifecycle

**Startup** — Backend initialization performed when that backend becomes active. A
single-tool configuration starts directly during `tool_init`; a multi-tool
configuration starts through `tool_change`.

**Shutdown** — Backend cleanup performed while switching away from an active backend.
It owns restoration of backend-specific temporary state.

**Tool stop** — Setting tool speed and coolant to zero. It is distinct from backend
shutdown.

**ATC hooks** — Optional multi-tool unmount and mount extension points surrounding
backend replacement. Inspect `tool.c` for ordering, status propagation, and partial
failure behavior.

## Modes and coolant

**Tool mode** — Planner/interpolator behavior selected temporarily by a backend.
Backends that override it restore the configured mode during shutdown.

**Coolant mask** — The flood and mist bitmask passed to `set_coolant`. Inspect
`tool_helper.h` for current mask definitions and helper behavior rather than copying
numeric values into documentation.
