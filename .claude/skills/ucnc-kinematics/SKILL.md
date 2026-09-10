---
name: ucnc-kinematics
description: Use when the user asks to create, modify, port, debug, or troubleshoot a kinematics backend (kinematic_*.c, kinematic_*.h, settings vars, homing strategy, forward/inverse transforms) or any code under uCNC/src/hal/kinematics/.
disable-model-invocation: true
---

Before any coding task within the kinematics layer (`uCNC/src/hal/kinematics/`) or any task that involves kinematic backend code (forward/inverse kinematics, homing strategy, coordinate transforms, settings variables, boundary checks, axis/stepper configuration, or registration in `kinematics.h` / `kinematicdefs.h`), read these two files in full:

1. **`BACKEND_GUIDE.md`** (this directory) — normative kinematics HAL contract and recommended backend-authoring patterns. Covers directory layout, the function surface contract, settings variable integration, axis/stepper configuration, homing patterns, motion-by-segments strategy, boundary checking, and the new-kinematic registration checklist.

2. **`CONTEXT.md`** (this directory) — authoritative vocabulary for the kinematics layer: layers (Kinematics HAL, kinematic backend), transform types (forward/inverse, coordinate-to-steps/steps-to-coordinates), settings macros (`KINEMATICS_VARS_DECL`, `KINEMATICS_VARS_DEFAULTS_INIT`, `KINEMATICS_VARS_SETTINGS_INIT`, `KINEMATICS_VARS_SYSTEM_MENU_INIT`), homing patterns, and registration contracts.

After reading both files, return to the task. Do not start editing, generating, or porting any kinematic backend code before reading them.