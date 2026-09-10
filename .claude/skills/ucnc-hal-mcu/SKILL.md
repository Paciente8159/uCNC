---
name: ucnc-hal-mcu
description: Use when the user asks to create, modify, port, debug, or troubleshoot an MCU backend (mcumap, mcu_*.c, boardmap, timer allocation, peripheral config, pin mapping) or any code under uCNC/src/hal/mcus/.
disable-model-invocation: false
---

Before any coding task within the MCU HAL layer (`uCNC/src/hal/mcus/`) or any task that involves MCU backend code (mcumap, pin aliases, timer allocation, peripheral configuration, ISR wiring, NVM strategy, clock/step-rate setup, feature flags, or boardmap integration), read these two files in full:

1. **`BACKEND_GUIDE.md`** (this directory) — normative MCU HAL contract and recommended backend-authoring patterns. Covers directory layout, the mcumap compile-time contract, the runtime contract, callback rules, timer allocation (ITP/RTC/SERVO/ONESHOT), the macro-or-function duality, communication streams, NVM strategies, testing requirements, and the new-backend registration checklist.

2. **`CONTEXT.md`** (this directory) — authoritative vocabulary for the backend layer: layers (MCU HAL, MCU backend, mcumap, Board HAL, IO HAL), pin categories (friendly name, canonical number, extended pin), construction patterns (feature flag, weak default, macro-or-function duality, access tier, contract change, extension protocol, expected-behavior contract, primary stream, custom shift provider).

After reading both files, return to the task. Do not start editing, generating, or porting any MCU backend code before reading them.