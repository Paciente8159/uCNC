---
name: ucnc-tool
description: Use when the user asks to create, modify, port, debug, or troubleshoot a tool backend (spindle, laser, plasma, pen, VFD, embroidery, tool struct, tool change, coolant, PID, tool modes) or any code under uCNC/src/hal/tools/.
disable-model-invocation: true
---

Before any coding task within the tool layer (`uCNC/src/hal/tools/`) or any task that involves tool backend code (tool struct definition, startup/shutdown sequences, speed control, PWM/spindle/laser/plasma/VFD implementations, tool change logic, coolant control, PID control, tool modes, range/speed conversion, or tool palette configuration in `cnc_hal_config.h`), read these two files in full:

1. **`BACKEND_GUIDE.md`** (this directory) — normative tool HAL contract and recommended backend-authoring patterns. Covers the `tool_t` struct slot-by-slot, the tool lifecycle (init, change, stop, mode), the tool palette registration system (TOOL1–TOOL16), speed range conversion (S-value to IO value and back), coolant integration, PID control integration, tool mode flags, pin configuration macros, and the creation checklist.

2. **`CONTEXT.md`** (this directory) — authoritative vocabulary for the tool layer: layers (Tool HAL, tool backend), the `tool_t` struct, lifecycle functions, tool modes, coolant and mist masks, built-in tool backends, pin configuration patterns, extended settings for PID, and the tool palette registration contract.

After reading both files, return to the task. Do not start editing, generating, or porting any tool backend code before reading them.