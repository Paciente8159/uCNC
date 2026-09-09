# µCNC MCU Backend Authoring Guide

This document combines the normative µCNC **MCU HAL contract** with recommended
backend-authoring patterns. The keywords below distinguish the two:

* **Required**: observable behavior or API/ABI compatibility that every backend
  must preserve. A deviation is a contract change and requires an ADR.
* **Recommended**: the common implementation or layout. A backend may deviate
  when its architecture requires it and should document the reason.
* **Example**: an observed implementation, not a requirement.

Section 4's API/behavior rules and the expected-behavior rules in §3.3 are
normative; implementation examples in those sections remain examples. Directory
layout, section ordering, access mechanisms and implementation techniques are
authoring guidance unless explicitly labeled **Required**.

Reference backends: `avr`, `stm32f0x/f1x/f4x/h7x`, `samd21`, `lpc176x`,
`rp2040/rp2350`, `esp8266`, `esp32/esp32c3/esp32s3`, `virtual`.

It is written for backend authors and code generators. The companion document
[`README.md`](README.md) (same folder as BACKEND_GUIDE.md) is the user-facing guide and contains the
canonical pin numbering table; this spec references it as the single source of
truth for pin numbers. Terminology is pinned in `CONTEXT.md` (repo root);
contract-level decisions live in `docs/adr/`.

Existing code is evidence of implementation techniques, not proof that every
backend meets every authoring recommendation. In particular, the C99/vendor-SDK
baseline below is a requirement for **new** backends, not a description of all
current build environments. See §10.1 for the source comparison and limitations.

### Authoring workflow

1. **Define the initial support scope.** List supported parts, packages,
   SDK/device-header versions and build paths. Inventory GPIO, mux routes,
   peripheral instances, timer clocks, IRQ sharing, DMA and flash geometry
   from vendor sources. Record document revision/table or header symbol for
   each hardware fact. Completion: a capability matrix with explicit supported,
   absent, not-yet-implemented and uninvestigated entries for the current scope.
   One chip or a known compatible chip set is a valid starting point; a survey
   of the entire family is not a prerequisite.
2. **Choose implementation boundaries.** Use §2.2 to separate board choices,
   silicon facts, generated aliases and runtime ownership. Select reference
   code per subsystem using §10.1. Completion: a file/dependency plan and a
   resource allocation table covering every enabled peripheral.
3. **Prove the vertical slice.** Register the backend, build the minimal board,
   and bring up GPIO, the clock/tick, one stream and step generation. Completion:
   a linked image and measured signals on the first board.
4. **Parameterize and generate.** Implement the current support scope,
   validate selections (§3.4), then generate the full canonical alias surface
   (§3.5). Completion: alternate board selections require configuration changes
   only within the implemented scope, and unsupported selections produce useful
   diagnostics. Broader chip/instance/route support can be added incrementally.
5. **Validate and report.** Apply §7, recording commands, configurations,
   results and hardware measurements. Completion: each supported feature has
   evidence at the appropriate level; untested combinations remain identified.

---

## 1. Big picture: where the backend fits

The µCNC HAL is composed of three cooperative layers, and all of them must be
consistent for a new MCU to work:

```
core (cnc.c, interpolator.c, ...)
        |
        v
IO HAL  (src/hal/io_hal.h)   <- resolves friendly names (STEP0, DOUT0, ...) to
                                 either an MCU pin or an extender (IC74HC595) pin
        |
        v
MCU HAL (mcumap_<arch>.h + mcu_<arch>.c)   <- THIS SPEC
        |
        v
Board HAL (src/hal/boards/*/boardmap_*.h)  <- maps friendly names to physical pins
```

* The **board HAL** (a `boardmap_<board>.h`) defines the *friendly pin names* and
  the physical location of each pin. For each defined pin it must provide at
  least `<PIN>_BIT` (and `<PIN>_PORT` for port-based MCUs). It also selects the
  MCU: `#define MCU MCU_<ARCH>`.
* The **MCU backend** has two entry files, with optional supporting modules:
  * `mcumap_<arch>.h` — compile-time glue: pin aliases, register access macros,
    feature flags, macro implementations of `mcu_*` calls.
  * `mcu_<arch>.c` — runtime code: ISRs, peripherals init, functions whose
    behavior cannot be a macro.
* The **IO HAL** (`io_hal.h`) consumes exclusively the MCU HAL surface defined in
  `src/hal/mcus/mcu.h` and the values of `DIO<n>` macros set by the mcumap.

### 1.1 Configuration flow (how `MCU` is selected and processed)

1. Build system may pass `-D MCU=MCU_<ARCH>` (and `-D BOARD=BOARD_<NAME>`,
   `-D BOARDMAP="path"`), e.g. `-D MCU=MCU_STM32F1X`.
2. `src/cnc_hal_config_helper.h:30-37` defines the pin assert helpers used
   everywhere by the IO HAL:
   ```c
   #define _EVAL_DIO_(X) DIO##X
   #define EVAL_DIO(X) DIO##X
   #define ASSERT_PIN(X)         (EVAL_DIO(X) != 0)   /* exists (IO or extended)  */
   #define ASSERT_PIN_IO(X)      (EVAL_DIO(X) > 0)    /* direct MCU IO pin        */
   #define ASSERT_PIN_EXTENDED(X) (EVAL_DIO(X) < 0)   /* extender pin (negative)  */
   ```
3. `src/hal/boards/boarddefs.h` includes the selected `BOARDMAP`, which defines
   the friendly pins and normally `#define MCU MCU_<ARCH>`. It then includes
   `uCNC/boardmap_overrides.h` and `pin_mapping_helper.h` before the mcumap.
   Resolve capability flags from these final pin selections, not earlier defaults.
4. `boarddefs.h` then includes `src/hal/mcus/mcudefs.h`, which includes
   `mcus.h` (MCU id list) and then the matching `mcumap_<arch>.h`, and finally
   `mcu.h` (the interface contract). See `mcudefs.h:27-124`.
5. Builds commonly collect `mcu_<arch>.c` files across the tree (makefiles use a
   recursive `rwildcard` over `*.c`; PlatformIO uses `src_dir=uCNC`). Files self-select with
   `#if (MCU == MCU_<ARCH>)` at the top. This is what allows all backends to live
   in the same tree. Source filters can exclude backends (see the Linux virtual
   environment); startup, SDK sources, linker scripts and adapters still need
   explicit build integration.

### 1.2 Registration checklist for a new MCU

A complete new backend normally uses these registration points:

| # | Where | What |
|---|---|---|
| 1 | `src/hal/mcus/mcus.h` | Add `#define MCU_<ARCH> <id>` (see grouping: AVR=1, STM32=10..13, SAMD21=20, LPC176X=30, ESP8266=40, ESP32=50..52, RP=60..61, virtual=1000/2000; pick the next free number in the matching family). |
| 2 | `src/hal/mcus/mcudefs.h` | Add an `#if (MCU == MCU_<ARCH>) ... #include "<arch>/mcumap_<arch>.h" ... #endif` block. Set `CFG_TUSB_MCU`/`CFG_TUSB_OS` here if the MCU uses tinyUSB. |
| 3 | `src/hal/boards/boards_helper.h` | Optional: register a `BOARD_<NAME>` and the matching `BOARDMAP` string. |
| 4 | `src/hal/boards/<family>/boardmap_<board>.h` | New or existing board map defining pins and `#define MCU MCU_<ARCH>`. |
| 5 | `makefiles/<arch>/Makefile` or a PlatformIO env | Build glue: toolchain, flags, `-D MCU=... -D BOARD=...`. Makefiles follow the `rwildcard` pattern (compile the whole `uCNC/` tree; see `makefiles/avr/makefile`). |
| 6 | `src/hal/mcus/<arch>/README.md` | Document the backend following the README skeleton (§2.1). This is required for new backends; older backends are being migrated incrementally. |

---

## 2. Directory and file layout conventions

```
src/hal/mcus/<arch>/
├── README.md              # backend notes (required for new backends)
├── mcu_<arch>.c           # the implementation TU (normally required)
├── mcumap_<arch>.h        # the compile-time map (required)
├── <arch>_*.c/.cpp        # optional: platform glue, extra drivers
│                          #   (e.g. stm32h7x_arduino.cpp, rp2040_lwip.c,
│                          #    esp32_spi.c, ic74hc595.pio + .pio.h)
└── *.ld                   # optional: linker script (e.g. stm32f4x.ld)
```

Rules:

* **Naming**: implementation file is always `mcu_<arch>.c`, map header always
  `mcumap_<arch>.h`, docs always `README.md`.
* **Language and dependency baseline (Required for new backends)**: backend code
  is C99 and builds against the MCU vendor's device headers/startup code or SDK
  without the Arduino framework whenever the vendor kit makes that possible.
  Do not use Arduino pin numbers, board variants, `Arduino.h`, C++ objects, or
  Arduino peripheral libraries as the backend's primary hardware abstraction.
  Prefer operation without an RTOS whenever practical. A vendor SDK is not
  automatically a bare-metal implementation: inspect its execution model and
  transitive dependencies using §5.9.
* **Platform glue** (`<arch>_*.c/.cpp`) is an optional, isolated adapter for a
  facility that genuinely has no practical C99/vendor-SDK implementation.
  Arduino-dependent glue must be behind an explicit opt-in flag, must not leak
  Arduino types into `mcumap_<arch>.h`, `mcu_<arch>.c`, or `mcu.h`, and must
  leave the C99/vendor-SDK build usable. Document why each such dependency is
  necessary. A board package being readily available in Arduino is not, by
  itself, sufficient reason to depend on it.
* **Shared code across variants of the same family** goes in a sibling folder
  included from each mcumap (see `esp32common/`, included at the tail of every
  ESP32 mcumap). Shared files self-select with preprocessor guards
  (`#if (ESP32)`, `#if defined(ESP32S3) || defined(ESP32C3)`, ...).
* Each `.c`/`.cpp` starts with its own guard, e.g.:
  ```c
  #include "../../../cnc.h"
  #if (MCU == MCU_<ARCH>)
  /* ... implementation ... */
  #endif
  ```
  so the whole tree can always be compiled regardless of the selected MCU.

### 2.1 Backend README skeleton

New backend READMEs should follow this fixed skeleton so backends stay
comparable. Existing README files predate this guide and may not yet contain
every section; missing sections are documentation debt, not evidence that the
backend violates the MCU HAL contract.

1. Target chips/packages and tested boards, capability matrix and vendor-source
   provenance. Distinguish implemented, compile-tested and hardware-tested support.
2. Toolchain and build recipes, including at least one C99/vendor-SDK build that
   does not require Arduino when the vendor kit permits it. State the access
   tier used (§5.9), all optional framework flags, and the reason for any
   unavoidable Arduino or C++ dependency.
3. Resource allocation table: timers/IRQs used (ITP/RTC/SERVO/ONESHOT), the
   assigned interrupt priorities, and the priority ordering chosen (§3.3).
   For SDK/framework/RTOS dependencies, include the execution-flow ownership
   record described in §5.9, including callback contexts and scheduling effects.
4. Primary stream per supported board (UART1 vs USB-CDC, §4.4).
5. NVM strategy chosen (§4.5) and any board-layer RAM-only opt-out.
6. Known limitations and any deliberate deviations from this spec, generation
   inputs/recipe (§3.5), and validation matrix/results (§7).

### 2.2 Strategic layout: share mechanisms, isolate silicon differences

Keep `mcumap_<arch>.h` as the compile-time entry point and `mcu_<arch>.c` as
the runtime entry point. Small backends can keep the implementation there;
split files when a subsystem has its own state, dependency or hardware variant.
The following responsibilities matter more than a prescribed file count:

| Responsibility | Owner | Inputs → outputs |
|---|---|---|
| PCB wiring and policy | Boardmap / board overrides | Physical pins, oscillator, peripheral/timer choices, reserved resources and optional features |
| Silicon capabilities | Backend-private part/package definitions | Device selection → available ports, register banks, instances, mux routes, IRQs, memory geometry |
| Configuration resolution | Mcumap or private included header | Final board choices + capabilities → validated register/clock/IRQ/channel tokens |
| Canonical pin aliases | Generated mcumap section or private header | Friendly-pin definitions → canonical IDs and every consumed `DIO<n>_<ATTR>` |
| Peripheral behavior | Runtime C modules | Resolved tokens → init, transfers, ISR dispatch and MCU HAL operations |
| Optional platform services | Guarded adapter modules | SDK/framework API → plain C boundary for USB, networking, storage or startup |

For a larger family, optional names such as `<arch>_caps.h`,
`<arch>_routes.h`, `<arch>_pins.h`, `<arch>_timers.c` and `<arch>_nvm.c`
make these boundaries visible. These are proposed private files, not existing
HAL APIs. Include capabilities before resolving board selections; validate
before exposing feature flags to generic code. Keep includes acyclic. Every
runtime state object, ISR vector and exported function has exactly one owner;
private headers contain declarations, not duplicate storage definitions.

Choose a family boundary around a compatible peripheral programming model.
Within STM32F1xx, differences in available ports, peripheral instances, timers
and routes belong in selection/capability macros where the register model is
compatible. The AVR backend similarly accommodates ATmega328P, ATmega1280 and
ATmega2560 through device definitions and resource selection. STM32F1 and
STM32F4 remain separate backends: their deeper programming-model differences
should not be forced into one driver through increasingly complex macros.

Put compatible variations in explicit capability/route mappings or small private
helpers. A new chip may initially need additional mappings and local exceptions;
that is normal incremental development. Separate runtime implementations where
register semantics or initialization sequences differ substantially (also seen
in SAMD21 TC/TCC handling). Shared vendor or CPU names alone do not establish
the right reuse boundary.

`esp32common/` demonstrates sharing signal generation, UART and task services
while keeping target I2S drivers separate. RP2040/RP2350 demonstrate similar
alarm/PIO algorithms maintained in separate directories; they are candidates
for careful reuse, not evidence that all register banks, GPIO ranges, SDK
assumptions or interrupt primitives are interchangeable.

Keep board names out of runtime driver branches. Use compile-time selection
and narrow private helpers for fixed hardware choices; a runtime table is
appropriate only when selection actually happens at runtime. Retain generated
pin repetitions as data-derived output rather than hand-maintained copies of
driver logic. Guard platform-specific includes as well as implementations so
an unrelated target does not need that SDK installed. Confirm build flags select
exactly one implementation when a native driver and adapter implement the same API.

---

## 3. The `mcumap_<arch>.h` contract (compile-time layer)

The mcumap is the heart of the backend. Stateless operations resolvable at
compile time should normally be macros (see §5). The following stable section
order is recommended because it makes generated and hand-written backends easy
to compare; it is not part of the MCU HAL ABI.

### 3.1 Design for family expansion; implement support incrementally

A new backend may support one chip or a small range of known compatible chips.
It need not cover every port, timer, peripheral instance or route on that chip
or across the family in its first version. Broader support requires further
datasheet/header inspection and validation, often over several development
iterations. Document what works now and what remains unsupported or unknown.

The design objective is to make that expansion local and predictable. Use these
patterns from the beginning, even when only one selection is implemented:

* Keep PCB wiring and resource choices in the boardmap/configuration layer.
  Resolve them into backend tokens instead of repeating the first board's
  literal registers, pins and IRQ names throughout runtime code.
* Use two-stage token expansion for regular register naming, plus explicit
  mappings for exceptions. GPIO port/bit selection, UART instance selection,
  timer/channel selection and IRQ naming should have identifiable resolution
  points. A selector with one currently supported value is a valid first step.
* Expose peripheral resources as configurable selections where the programming
  model allows it. For example, an AVR UART driver should consume resolved
  data/status/control/baud/IRQ tokens so UART0, UART1 and later instances can
  reuse the transfer logic when supported. Apply the same principle to SPI,
  I2C and timers; verify each instance's actual register semantics before reuse.
* Guard device-specific resources and report unsupported selections clearly.
  A documented route can remain unimplemented initially; distinguish that from
  hardware absence. Adding its mapping later should not require redesigning
  the driver or duplicating it for another board.
* Generate the canonical friendly-pin alias blocks independently of the first
  board's wiring. These are a mechanical µCNC interface surface, not a claim
  that all physical pins or peripheral routes have been implemented. The 13
  hardware maps currently contain all 192 canonical `DIO<n>` definitions;
  board selections activate the applicable blocks.

Within already implemented capabilities, a new board should need configuration
changes only. Extending chip, instance or route support may legitimately change
the mcumap and, when necessary, local runtime handling. Review whether the
existing structure makes that extension straightforward, rather than requiring
all future combinations to work without source edits today.

A second or synthetic boardmap is a useful check of this separation when
alternate selections are available. For an initial single-selection backend,
review the macro expansion and driver boundary and document the remaining
coverage. Completion is judged against the current implementation scope and
MCU HAL behavior; exhaustive family coverage is not an acceptance gate.

### 3.2 Canonical section order

1. **Include guard + `extern "C"` wrapper** (`#ifndef MCUMAP_<ARCH>_H`...).
2. **Device/SDK includes** — the vendor register definitions the macros expand
   to (`avr/io.h`, `stm32f4xx.h`, `sam.h`, vendor SDK headers, ...). Keep them
   minimal; STM32 maps include only CMSIS plus the RCC clock header and disable
   unused HAL modules (`-D HAL_TIM_MODULE_DISABLED`...) to save space. Framework
   headers belong only in an explicitly selected adapter (§2 and §5.9).
3. **Clock and step-rate constants**:
   ```c
   #ifndef F_CPU
   #define F_CPU SystemCoreClock   /* or a literal, e.g. 16000000UL */
   #warning "F_CPU not defined as a constant. Cycle-accurate delays/step math may be wrong"
   #endif
   #ifndef F_STEP_MAX
   #define F_STEP_MAX 30000        /* see also F_STEP_MIN, default 4 */
   #endif
   /* extra timer base, if the family uses one:  #define F_TIMERS 4000000UL (SAMD21) */
   ```
4. **Delay constants** (consumed by `mcu.h`'s generic `mcu_delay_cycles`):
   `MCU_CLOCKS_PER_CYCLE` (default 1), `MCU_CYCLES_LOOP_OVERHEAD` and
   `MCU_CYCLES_PER_LOOP` (**#error if missing** in mcu.h). Optionally define a
   custom `mcu_delay_loop(X)` (see AVR inline asm `sbiw/brne`, STM32 `mcu_delay_loop`,
   architecture-specific assembly loops). Check the actual CPU instruction set
   and available counters; a delay mechanism from another ARM core or ESP32
   variant is not automatically portable. A `mcu_nop()` may be provided too.
5. **NVIC/IRQ priority table** (ARM backends): `NVIC_INPUT_IRQ_Pri`,
   `NVIC_ITP_IRQ_Pri`, `NVIC_RTC_IRQ_Pri`, `NVIC_SERVO_IRQ_Pri`,
   `NVIC_ONESHOT_IRQ_Pri`, `NVIC_USB_IRQ_Pri`, ... (see `mcumap_stm32f1x.h:62-70`).
   Recommended ordering (lower numeric value = higher priority, where the
   hardware allows): input-change and ITP at the top, then the RTC tick, then
   servo/oneshot, then comms (UART/USB). The backend README documents the
   priorities actually assigned (§2.1).
6. **ROM/flash string macros** — the `__rom__` / `__romstr__` / `__romarr__` /
   `rom_strptr` / `rom_strcpy` / `rom_strncpy` / `rom_memcpy` / `rom_read_byte` /
   `rom_strcmp` family. AVR defines them to `PROGMEM`/`PSTR`/`pgm_read_*`; other
   architectures leave them as RAM defaults (mcu.h provides those defaults).
7. **Byte/bit operations** (`SETBIT`, `CLEARBIT`, `CHECKBIT`, `TOGGLEBIT`,
   `SETFLAG`, `CLEARFLAG`, `CHECKFLAG`, `TOGGLEFLAG`) — provide or reuse the core
   definitions.
8. **Token-pasting helpers** — common implementation pattern:
   ```c
   #define __helper_ex__(left, mid, right) left##mid##right
   #define __helper__(left, mid, right)    __helper_ex__(left, mid, right)
   #ifndef __indirect__
   #define __indirect__ex__(X, Y) DIO##X##_##Y
   #define __indirect__(X, Y)    __indirect__ex__(X, Y)
   #endif
   ```
   Helpers used to form ISR names must yield bare identifiers. Parenthesize
   expressions at their use sites; wrapping a pasted identifier in parentheses
   prevents using it in some declarations or further token-pasting operations.
   Semantics: `__indirect__(DOUT0, BIT)` first resolves `DOUT0` → `47`, then
   concatenates `DIO47_BIT`. **The mcumap MUST therefore define a
   `DIO<n>_<ATTR>` token for every attribute a pin uses.** This is what converts
   `mcu_set_output(DOUT0)` into a direct register write at compile time.
9. **The DIO pin table** (the autogenerated bulk of the file). Emit a conditional
   block for every canonical friendly pin, whether or not the first board uses
   it; the boardmap definitions decide which blocks become active:
   ```c
   #if (defined(STEP0_PORT) && defined(STEP0_BIT))   /* or only _BIT (RP2040) */
   #define STEP0 1                     /* canonical number, see README table */
   #define DIO1 1
   #define DIO1_BIT (STEP0_BIT)        /* + every other DIO<n>_<ATTR> alias */
   #define STEP0_OUTREG (__outreg__(STEP0_PORT))   /* register + bit tokens */
   ...
   #endif
   ```
   * The **canonical numbers are NOT free**: they are fixed worldwide by the
     table in `README.md` (e.g. `STEP0=1..STEP7=8`, `DIR0=9..DIR7=16`,
     `STEPn_EN=17..24`, `PWM0..15=25..40`, `SERVO0..5=41..46`, `DOUT0..49=47..96`,
     `LIMIT_X..C=100..108`, `PROBE=109`, `ESTOP=110`, `SAFETY_DOOR=111`,
     `FHOLD=112`, `CS_RES=113`, `ANALOG0..15=114..129`, `DIN0..49=130..179`,
     `TX=200 RX=201 USB_DM=202 USB_DP=203 SPI_CLK=204 SPI_SDI=205 SPI_SDO=206
     SPI_CS=207 I2C_CLK=208 I2C_DATA=209 TX2=210 RX2=211 SPI2_*=212..215`).
   * The range `1..215` is **frozen** for compatibility; `216..254` is reserved
     for future official expansion (mainly communications or special
     applications). Extending the table is a cross-cutting change (mcumaps,
     `io_hal.h`, README table, generator) and requires revisiting ADR-0001.
     Negative values belong exclusively to the IO-HAL extender mechanism (§4.6).
   * The IO HAL evaluates `DIO<n>`: `>0` → MCU pin, `<0` → extender, `0` → unused.
   * For port-based MCUs the register tokens are derived with label concatenation
     (`__outreg__(PORT)`→`PORTB` in AVR, `__helper__(GPIO, X, )`→`GPIOA` in STM32).
   * PWM pins additionally get timer/channel tokens (`PWM0_TIMER`, `PWM0_CHANNEL`,
     `PWM0_CCR`, `PWM0_MODE`, `PWM0_PRESCALLER`, clock/enable register macros);
     ANALOG pins get `ANALOG0_CHANNEL`/ADC prescaller tokens.
   * Preserve board-provided definitions such as `STEP0_BIT`; define aliases
     from them, never redefine them in terms of themselves. The table is long
     and mechanical; `docs/mcumap_gen.xlsx` is the existing generation aid.
     Follow §3.5 rather than assuming the workbook proves a new target's routes.
10. **Feature flags, derived from pin/peripheral definitions** (all `#ifndef`-
    friendly, usually computed):
    ```c
    #if (defined(TX) && defined(RX))
    #define MCU_HAS_UART
    #endif
    /* MCU_HAS_UART2, MCU_HAS_USB (USB_DP && USB_DM), MCU_HAS_SPI / MCU_HAS_SPI2,
       MCU_HAS_I2C, MCU_SUPPORTS_I2C_SLAVE (+ I2C_ADDRESS != 0), MCU_HAS_BLUETOOTH,
       MCU_HAS_ONESHOT_TIMER (ONESHOT_TIMER defined), MCU_HAS_DMA, MCU_HAS_FLASHUPDATE */
    ```
    Feature macros gate both generic code (`mcu.c`, `mcu.h`) and the backend's
    own functions, so a minimal board builds a minimal binary.
11. **Peripheral configuration blocks**, one per peripheral, each producing:
    * UART1/2: `COM_UART`/`COM2_UART` (peripheral pointer), `COM_OUTREG/INREG`,
      `MCU_SERIAL_ISR`/`MCU_SERIAL2_ISR` (the IRQ handler *name*),
      `COM_APB/COM_APBEN`, clock source, baud macros; `#error` on unsupported
      pin remaps (e.g. `mcumap_stm32f1x.h:5118`).
    * SPI/SPI2: `SPI_REG`, `SPI_CLOCK`, `SPI_ENREG`, `SPI_FREQ` (default `1000000UL`),
      `SPI_MODE`, optional DMA channel/flag tokens, `SPI_ISR`.
    * I2C: `I2C_REG`, `I2C_ISR`, `I2C_FREQ` (default `400000UL`), `I2C_ADDRESS`,
      `I2C_APBEN`, speed-range tokens.
    * Timer allocator (see §3.3).
12. **IO function macros** — the `mcu_*` "macro-or-function" surface (§4.1).
13. **Time/ISR macros**: `mcu_enable_global_isr`, `mcu_disable_global_isr`,
    `mcu_get_global_isr`, `mcu_in_isr_context`, `mcu_free_micros` (free-running
    fraction of the 1 ms counter), plus `mcu_enable_probe_isr` /
    `mcu_disable_probe_isr` (empty macros when no probe ISR exists).
14. **Stream plumbing**: usually the mcu.h defaults are kept
    (`mcu_getc → mcu_uart_getc` etc.); only override when the primary stream is
    not UART1.
15. **Tail include of family-common headers** (e.g.
    `#include "../esp32common/esp32_common.h"`).

### 3.3 Timer allocation block (critical)

Timing behavior follows the **expected-behavior contract** (CONTEXT.md): there
is no absolute/right way to design the timers — the design depends heavily on
the architecture (compare channels, halved counters, accumulators, alarm pools,
buffered IO, virtual ticks) — as long as the observable behavior is reproduced:
a step pulse with roughly 50% duty per event pair, a monotonic 1 ms tick, and
the callback rules of §4.3. Every backend allocates hardware timing resources
for at least:

| Role | Macro prefix | Purpose |
|---|---|---|
| ITP | `ITP_*` | Step pulse generation (interpolator). Mandatory. |
| RTC | `RTC_*` (or SysTick) | 1 ms tick feeding `mcu_rtc_cb`. Mandatory. |
| SERVO | `SERVO_*` | 50 Hz servo pulse train (compile-time pruned by `SERVOS_MASK > 0`). |
| ONESHOT | `ONESHOT_*` | Optional single-shot timeout → `MCU_HAS_ONESHOT_TIMER`. |

On register-based targets the allocator commonly produces the peripheral pointer, clock
enable bit, the clock source and the **IRQ handler name** via token pasting
(so a single `mcu_<arch>.c` works for any timer choice made by the board):

Illustrative STM32-style tokens (not a portable timer allocator):

```c
#ifndef ITP_TIMER
#define ITP_TIMER 2
#endif
#define MCU_ITP_ISR     __helper__(TIM, ITP_TIMER, _IRQHandler)   /* TIM2_IRQHandler */
#define MCU_ITP_IRQ     __helper__(TIM, ITP_TIMER, _IRQn)         /* TIM2_IRQn     */
#define ITP_TIMER_REG   __helper__(TIM, ITP_TIMER, )              /* TIM2          */
#define ITP_TIMER_APB   __helper__(RCC_APB1ENR_TIM, ITP_TIMER, EN)
#define ITP_TIMER_CLOCK HAL_RCC_GetPCLK1Freq()
```

Resolve the actual timer input clock, bus enable register, counter width and
shared vector for the selected part. A peripheral bus clock is not necessarily
the timer kernel clock; derive prescaler/multiplier behavior from the clock
tree and initialization. The abbreviated example above omits that resolution.

Special cases handled in the same block:
* shared IRQ lines (e.g. `TIM1_UP_TIM10_IRQHandler`, `TIM6_DAC_IRQHandler`,
  `USART3_8_IRQHandler`) — map by timer number with `#if` chains;
* different register families (SAMD21: `TCCn` vs `TCn` chosen by `ITP_TIMER < 3`);
* free-running hardware timer with alarm comparators (RP2040, with
  `irq_set_exclusive_handler(ITP_TIMER_IRQ, mcu_itp_isr)`); RP2040 reuses the
  same alarm pool for RTC/SERVO/ONESHOT via a sorted alarm list;
* RTOS timers or background tasks (ESP32: ESP-IDF timer group + FreeRTOS task);
* virtual backend: software tick emulation.

The step ISR must generate **two alternating events per step period**
(`mcu_step_cb()` then `mcu_step_reset_cb()`); the RTC must produce a 1 ms tick
and call `mcu_rtc_cb(uint32_t millis)`; `mcu_free_micros()` must keep counting
even inside ISRs/atomic sections; **`mcu_dotasks()` MUST NOT be called from the
RTC path** (see the callback-contract section in `README.md`). Observed
mechanisms that reproduce this contract:

| Backend | Step generation | RTC 1 ms tick | Notes |
|---|---|---|---|
| AVR | ITP timer compare A/B for step/reset | Separate `RTC_TIMER`, compare A (default timer 0; ITP default timer 1) | RTC also multiplexes servo pulses; defines `mcu_start_step_reset_timeout()` |
| STM32 F1/F4/F0/H7 | Single timer ISR, `PSC/ARR` halving + `resetstep` toggle | SysTick increments `mcu_runtime_ms`, pends low-priority `PendSV_Handler` | NVIC priority table in the mcumap |
| SAMD21 | TC/TCC compare + toggle | SysTick → PendSV | TCC vs TC selected by timer index |
| LPC176X | `LPC_TIMx` compare | SysTick → PendSV | Needs framework clock |
| RP2040/RP2350 | Hardware alarm slot, ISR re-arms + toggles | Alarm re-enqueued each 1 ms | One alarm pool shared with servo/oneshot |
| ESP32 / S3 / C3 | Timer-group / buffered signal scheduling with shared `esp32common/esp32_signal.c` | FreeRTOS task (`mcu_rtc_task`) + `esp_system_get_time()` | Inspect the selected direct/buffered path and target I2S driver together |
| ESP8266 | `timer1` ISR + **buffered IO** (`out_io_buffer`) | `os_timer` 1 ms | Analyze callback scheduling separately from physical output latency |
| Virtual | Software tick `mcu_gen_step()` at `2×F_STEP_MAX` | `tickcount` sampling | Deterministic under `PIO_UNIT_TESTING` |

Recommended interrupt priority topology (where priorities are programmable,
e.g. ARM NVIC): input-change and ITP at the top, then the RTC tick, then
servo/oneshot, then comms (UART/USB). Follow the `NVIC_*_IRQ_Pri` table pattern
(`mcumap_stm32f1x.h:62-70`) and document the assigned values in the backend
README (§2.1).

### 3.4 Configuration resolution and resource validation

Treat three facts separately: the silicon **has** a peripheral, the backend
**implements** it, and this board **enables** a valid instance/route. Advertise
`MCU_HAS_*` only for the resulting usable configuration. An explicit request
for an unavailable or unimplemented route should fail with a diagnostic;
an unrequested optional feature should compile away. Silicon capability data
belongs to the backend's private definitions, not new public feature flags.

Resolve each selection as a tuple: `(part, package, instance, signal, pin,
mux/remap, channel/pad, clock, IRQ, DMA request)`. Validate combinations, not
just individual integer ranges. Register existence does not prove a pin is
bonded out, output-capable or routable to the chosen instance. GPIO matrices
still have restrictions. ADC channel numbers, timer channels and UART pads are
not interchangeable with GPIO bit numbers.

Maintain a resource table covering ITP, RTC, servo, oneshot, PWM, communications,
DMA, PIO/I2S and SDK-owned resources. Check at least:

* timer/channel collisions, including PWM outputs sharing a frequency/prescaler;
* UART/SPI/I2C selecting the same configurable serial block (e.g. SERCOM);
* shared interrupt vectors: one dispatcher services the enabled pending sources;
* input interrupt routing collisions (e.g. two ports competing for an EXTI line);
* DMA channels/requests and memory accessibility; cache maintenance/alignment
  where required by the selected device and SDK;
* GPIO bank width, valid bit shifts, input-only pins, boot/debug/flash reservations;
* flash/bootloader/NVM region overlap, erase units and minimum program width.

Intentional sharing needs an ownership and scheduling rule; equal resource
numbers alone do not prove a conflict or safe sharing. Diagnostic text should
name the conflicting board selections and permitted alternatives. Defaults
belong behind `#ifndef` and must pass the same validation as explicit choices.

### 3.5 Reproducible generation

Generate mechanical aliases and route/capability tables from reviewed data;
keep peripheral algorithms and ISR state machines in hand-reviewed templates
or runtime modules. A text generator and schema are a recommended improvement,
not tools already supplied by this guide. The existing workbook is a starting
point; record the sheet/template and extraction procedure if using it.

Keep three inputs distinct: the canonical µCNC pin list (README numbering
table), silicon facts with vendor-source provenance, and board selections.
Board selections activate generated blocks; they must not limit which friendly
pin blocks the generator emits. A generation record should contain:

* input paths/revisions and generator/template version;
* target part/package/SDK scope and documented unsupported capabilities;
* exact invocation or workbook procedure, output paths and generated boundaries;
* expected alias attributes per role (GPIO, PWM, ADC, interrupt, communications).

Before accepting output, check canonical name/number equality (including gaps),
duplicate definitions, every consumed `DIO<n>_<ATTR>`, balanced feature guards,
and preservation of board overrides. The 192-block count is a useful check but
cannot detect swapped aliases or incorrect register routes. Preprocess and
compile representative consumers through `cnc.h`/`io_hal.h`, including unused
and extended pins, with the target headers. Merely including the map may leave
broken token-pasting expansions untested. Regenerate twice and require an
unchanged second output. Fix generation inputs/templates rather than editing
generated blocks independently.

For code generation, missing hardware facts are unresolved work: identify the
missing route or register evidence and keep that support claim incomplete.
Never infer register names, IRQ vectors or mux values by renaming another
backend. Validate one representative expansion per peripheral class before
emitting thousands of repetitions.

---

## 4. The `mcu_<arch>.c` contract (runtime layer)

### 4.1 The "macro-or-function" duality (the #1 performance rule)

`mcu.h` declares replaceable IO/time operations inside `#ifndef` guards:

```c
#ifndef mcu_set_output
void mcu_set_output(uint8_t pin);
#endif
```

Consequences, by design:
1. If the mcumap defines `mcu_set_output` as a macro, the header guard suppresses
   the prototype. Matching guards around fallback definitions suppress those
   definitions too. A direct register-write expansion can remove function calls
   and pin switches; the header guard alone does not guard another source file.
2. If the mcumap does **not** define it, a real C function must exist somewhere
   (the backend, or a weak default in `mcu.c`).
3. A macro may expand to a register operation, an inline helper, or an SDK
   function. Macro substitution alone does not guarantee one instruction or
   zero runtime dispatch; inspect optimized output for timing-critical paths.

Writing the IO macros — illustrative STM32F1-style register shapes:

```c
/* direct register write via token pasting */
#define mcu_set_output(diopin)   (__indirect__(diopin, GPIO)->BSRR = (1U << __indirect__(diopin, BIT)))
#define mcu_clear_output(diopin) (__indirect__(diopin, GPIO)->BRR  = (1U << __indirect__(diopin, BIT)))
#define mcu_toggle_output(diopin) (TOGGLEBIT(__indirect__(diopin, GPIO)->ODR, __indirect__(diopin, BIT)))
#define mcu_get_input(diopin)    (CHECKBIT(__indirect__(diopin, GPIO)->IDR, __indirect__(diopin, BIT)))
#define mcu_get_output(diopin)   (CHECKBIT(__indirect__(diopin, GPIO)->ODR, __indirect__(diopin, BIT)))
#define mcu_config_output(diopin) { /* enable clock + set direction via __indirect__(diopin, ...) */}
#define mcu_config_input(diopin)  { ... }
#define mcu_config_pullup(diopin) { ... }
#define mcu_config_input_isr(diopin) { ... }   /* route pin to the input-change ISR */
#define mcu_config_analog(diopin) mcu_config_input(diopin)  /* + ADC channel enable */
#define mcu_config_pwm(diopin, freq) { ... }   /* timer/channel/CCR/mode setup */
#define mcu_set_pwm(diopin, value) { ... }
#define mcu_get_pwm(diopin) ( ... )
#define mcu_get_analog(diopin) ( ... )         /* channel select + read + scale */
```

Use `__indirect__(diopin, <ATTR>)` and the `<PIN>_*` tokens generated in §3.2,
item 9.
With literal addresses and bit numbers, the compiler can remove pin lookup
overhead; the actual loads/stores still cost instructions and code size. Use
the target's atomic set/clear registers where available. Read-modify-write
toggle operations need protection if another execution context writes the same
port. Register names such as `BRR` are not portable across all STM32 devices.

### 4.2 Canonical function and ISR inventory of `mcu_<arch>.c`

The file should implement (or delegate to a macro / a weak default) the complete
surface of `mcu.h`. Canonical inventory, with the implementation notes that make
backends comparable:

The API uses these implementation classes:

| Class | Meaning | Failure mode when omitted |
|---|---|---|
| Required backend function | The backend must provide a definition when the matching feature is enabled. | Link failure is intentional. |
| Replaceable macro/function | The mcumap may define a macro; otherwise a function with the same guarded name must exist. | Compile or link failure unless a default is listed. |
| Weak default | `mcu.c` supplies fallback behavior which a backend may override with a strong symbol. | The documented fallback runs. |
| Optional empty macro | `mcu.h` supplies a no-op because unsupported is valid behavior. | No operation. |
| Feature-gated callback | Exists only when its `MCU_HAS_*`, `DETACH_*`, or related feature condition is true. | Not compiled. |

Do not infer a weak default merely from an `#ifndef` guard. In particular,
`mcu_spi_xmit`, `mcu_spi2_xmit`, the enabled UART/USB/I2C operations, timer
conversion/control, and `mcu_init` are backend requirements unless the mcumap
replaces them with macros.

| Interface (mcu.h) | Notes / pattern |
|---|---|
| `mcu_init(void)` | Clock setup, watch-dog disarm, `mcu_io_init()`, peripheral inits (uart/usb/spi/i2c), pin-change/external-interrupt wiring, `mcu_start_rtc()`/1 ms tick, servo init, `mcu_enable_global_isr()`. |
| `mcu_io_init`, `mcu_io_reset` | **Generic weak defaults exist in `mcu.c`** (`mcu_outputs_init`/`mcu_inputs_init`/`mcu_coms_init` walk direct MCU pins selected by `ASSERT_PIN_IO(...)` and call `mcu_config_output/input/pwm/analog/pullup/input_isr`). Peripheral pins passed to `mcu_config_*` must also be direct MCU pins; extended pins are dispatched by the IO HAL. Override only when the architecture needs extra work; `mcu_io_reset` is the per-board hook for custom power-up states. |
| `mcu_config_*`, `mcu_get/set/clear/toggle_output`, `mcu_get_input` | Macros (see §4.1). |
| `mcu_freq_to_clocks(float, *ticks, *prescaller)` / `mcu_clocks_to_freq` | Clamp to the supported step-rate range and encode/decode an opaque ticks/prescaler pair. Existing backends use floating-point division followed by integer scaling. Derive the encoding from the real clock/divider and event rate, including counter width and rounding; test the minimum/maximum rates and inverse conversion. |
| `mcu_start_itp_isr(ticks, prescaller)` | Initialize the selected step mechanism, clear stale pending events and start step/reset scheduling. Two compare channels are an AVR example, not a requirement for alarms or accumulator designs. |
| `mcu_change_itp_isr(ticks, prescaller)` | Change the rate during motion with defined phase/update behavior and no unintended extra step. |
| `mcu_stop_itp_isr(void)` | Stop further step scheduling and handle pending/reset events consistently with the core. AVR deliberately retains its interrupt mask; STM32 disables the timer interrupt and NVIC line. Follow the selected mechanism's lifecycle. |
| `mcu_start_step_reset_timeout()` | Optional empty macro declared near the step-interpolator API in `mcu.h`. Hook to shorten/re-arm the step pulse or re-enable interrupts right after a step event; called by the interpolator. Only AVR defines it today. |
| `mcu_millis`, `mcu_micros`, `mcu_free_micros` | `mcu_runtime_ms` incremented by the 1 ms tick; `mcu_micros = 1000*ms + free_micros`. |
| `MCU_ITP_ISR` | Alternating `mcu_step_cb()` / `mcu_step_reset_cb()` (see §3.3). |
| RTC/PendSV ISR | `mcu_runtime_ms++; pends low-priority task` → `mcu_rtc_cb(mcu_runtime_ms)`. |
| `mcu_uart_init/getc/available/clear/putc/flush` + `mcu_uart2_*` | Commonly use `DECL_BUFFER` RX/TX buffers initialized before use; SDK-backed ports may delegate buffering. For attached RX: `if (mcu_com_rx_cb(c)) { if (!BUFFER_TRY_ENQUEUE(uart_rx, &c)) { STREAM_OVF(c); } }`. A `false` protocol callback result means a consumed realtime byte, not overflow. Detached RX calls `mcu_uart_rx_cb(c)` instead. |
| `mcu_usb_*` (if `MCU_HAS_USB`) | tinyUSB device (include `<tusb_ucnc.h>`): `mcu_usb_init → tusb_cdc_init`, USB IRQ → `tusb_cdc_isr_handler`, `mcu_dotasks → tusb_cdc_task()` + drain `tusb_cdc_read()` into `mcu_com_rx_cb`. Non-tinyUSB platforms wrap their stack (Arduino `Serial`, `USBCDC`). |
| `mcu_spi_init/config/start/stop/xmit/bulk_transfer` + `mcu_spi2_*` (if `MCU_HAS_SPI[2]`) | `mcu.c` provides weak defaults for init/config/start/stop/bulk transfer. The byte `mcu_spi[_2]_xmit` primitive is required from the backend or mcumap. Bulk transfer rolls over that primitive with `BULK_SPI_TIMEOUT` + `TASK_YIELD()`. `mcu_spi_port`/`mcu_spi2_port` are generic, non-weak function tables `{isbusy, start, xmit, bulk_xmit, stop}` initialized by `mcu.c`; customize behavior by overriding the weak functions or macro substitution points, not by defining a second table. |
| `mcu_i2c_init/config/send/receive` (if `MCU_HAS_I2C`) | Master API with `ms_timeout`; slave support when `MCU_SUPPORTS_I2C_SLAVE && I2C_ADDRESS != 0` calls `mcu_i2c_slave_cb` (weak default in mcu.c). |
| `mcu_set_servo/get_servo` | Body wrapped `#if SERVOS_MASK > 0`; servo pulse train is multiplexed into a shared timer ISR with per-servo `_FRAME` compile-time `#if` pruning (AVR/STM32/SAMD21 pattern). |
| `mcu_config_timeout/start_timeout` (if `MCU_HAS_ONESHOT_TIMER`) | `mcu_timeout_cb` delegate stored globally in `mcu.c`; ISR calls it. `MCU_ONESHOT_ISR`. |
| Input-change ISRs | Per-port/per-line handlers routing to `mcu_limits_changed_cb`, `mcu_controls_changed_cb`, `mcu_probe_changed_cb`, `mcu_inputs_changed_cb`, pruned at compile time by pin bit-masks (AVR `PCINTA_LIMITS_MASK & ...`, STM32 `ALL_EXTIBITMASK`). When ISR-less, the soft-polling path (`FORCE_SOFT_POLLING`) feeds the same callbacks from IO processing. |
| `mcu_eeprom_getc/putc/flush` | Weak stubs in `mcu.c` provide no persistence. Implement storage or explicitly document the opt-out in §4.5; validate the configured address space against real capacity. |
| `mcu_dotasks(void)` | Called from `cnc_run()` (cnc.c): tinyUSB task, port polling, feeding RX into `mcu_com_rx_cb`. Empty OK on ISR-driven designs (AVR). ESP32 variants use it plus FreeRTOS background tasks. |
| `mcu_delay_loop`, `mcu_delay_us/ns/hz/cycles` | Macro or function; cycle-counted base for sub-ms delays. |

Every replaceable function must be `#ifndef`-guarded where a mcumap macro could
replace it (follow the exact guards already present in `mcu.h`), and
feature-gated with the same `MCU_HAS_*`/`SERVOS_MASK` guards used by generic
code. Required functions that are not substitution points do not need such a
guard.

### 4.3 The callback contract

These externs are implemented by core/other TUs; the backend only *calls* them
from ISRs/events. `MCU_CALLBACK`/`MCU_RX_CALLBACK`/`MCU_IO_CALLBACK` are
attribute hooks (e.g. `IRAM_ATTR` on ESP32, empty elsewhere) that the mcumap can
redefine.

| Callback | When to call | Notes |
|---|---|---|
| `mcu_step_cb()` | Each step-pulse rising event | Highest timing requirement. |
| `mcu_step_reset_cb()` | Each step-pulse falling event (half period later) | |
| `mcu_com_rx_cb(uint8_t c)` | Each received byte from any stream | Returns `false` to drop the byte (realtime commands handled internally). Never reentrant. |
| `mcu_rtc_cb(uint32_t millis)` | Every 1 ms tick | Never call `mcu_dotasks()` from inside/around it. |
| `mcu_controls_changed_cb()` | Any control pin (ESTOP/FHOLD/SAFETY_DOOR/CS_RES) changed | |
| `mcu_limits_changed_cb()` | Any limit pin changed | |
| `mcu_probe_changed_cb()` | Probe pin changed (only when probe ISR enabled) | |
| `mcu_inputs_changed_cb()` | Any `DINn` ISR pin changed | |
| `mcu_timeout_cb()` | One-shot timer expired (`MCU_HAS_ONESHOT_TIMER`) | |
| `mcu_i2c_slave_cb(data, len)` | I2C slave transaction (`MCU_SUPPORTS_I2C_SLAVE`) | |
| `mcu_uart/uart2/usb/telnet/bt_rx_cb` | Only when the matching `DETACH_<PORT>_FROM_MAIN_PROTOCOL` is defined | Weak empty defaults in mcu.c. |

Design constraints enforced by the core (see the MCU requirements in
`README.md`):
* Callbacks are **not reentrant**: the same callback must never be entered twice
  concurrently (ISR nesting on the *same* event is forbidden; *different*
  callbacks may nest, e.g. `mcu_step_reset_cb` preempting `mcu_step_cb`).
* No assumptions about ISR priority ordering between callbacks.
* µCNC makes no assumptions about number of CPU cores; on multicore targets the
  backend must add the concurrency safeguards (ESP32 semaphores/atomics, RP2040
  FIFO + core pinning) and may split work across cores (comms on core 0, main
  loop on core 1) as long as memory is shared.

### 4.4 Communication streams and the primary stream

Enabled streams expose the `mcu_<port>_getc/available/clear/putc/flush` shape,
gated by `MCU_HAS_UART`, `MCU_HAS_UART2`, `MCU_HAS_USB`, `MCU_HAS_BLUETOOTH`,
or `ENABLE_SOCKETS` for telnet. Initialization entry points differ by service;
consult `mcu.h` and the selected backend rather than synthesizing a universal
`mcu_<port>_init` API. TX/RX buffering commonly uses `DECL_BUFFER`
(`uart_rx`/`uart_tx`, `uart2_*`, `usb_*`, `bt_*`); SDK-backed streams may use
their own buffers. Initialize buffering before enabling reception.

* **RX path**: the ISR (or the polling loop in `mcu_dotasks`) pushes each byte
  through `mcu_com_rx_cb(c)` (which returns `false` for bytes consumed as
  realtime commands) and enqueues the rest; overflow is reported with
  `STREAM_OVF(c)`.
* **Detached ports**: when `DETACH_<PORT>_FROM_MAIN_PROTOCOL` is defined, the
  port bypasses `mcu_com_rx_cb` and calls the weak `mcu_<port>_rx_cb(c)` hook
  instead.
* **Stream dispatch**: `mcu.h` defaults `mcu_getc/putc/...` to the UART1
  functions; the multistream layer (`grbl_stream.c`) registers every enabled
  port as a `grbl_stream` (flags `STREAM_UART 1`, `STREAM_UART2 2`, `STREAM_USB
  4`, `STREAM_WIFI 8`, `STREAM_BTH 16`, `STREAM_BOARDCAST 255`) and broadcasts
  TX to all of them when broadcasting is active.

**Primary stream preference.** When a board has several ports, the preferred
order for the main protocol is: **UART → USB → WiFi/ethernet → Bluetooth/other**.
The backend either overrides `mcu_getc`/`mcu_putc` (and friends) in the mcumap,
or relies on the `grbl_stream` multistream registration. The backend README
**must state which stream is primary on each supported board** (e.g. UART1 on
bluepill, USB-CDC on RP2040), since it determines the out-of-box console.

### 4.5 Non-volatile storage strategies

Persistence is the baseline UX: settings must survive a reboot. `mcu.h` expects
`mcu_eeprom_getc/putc/flush` over an address space of at least
`NVM_STORAGE_SIZE` (default `0x400`). Real backends override the weak stubs
with one of these strategies (decision table):

| Strategy | Used by | When to pick |
|---|---|---|
| Hardware EEPROM | AVR (`EECR`/`EEAR`/`EEDR` register sequence) | The MCU has dedicated EEPROM; validate its capacity. |
| Flash-page emulation | STM32F1/F0 (inverted-bit pages, `FLASH_CR` sequence), SAMD21 (row erase + `NVMCTRL`), ESP8266 (sector wear-levelling) | No EEPROM, flash budget allows stealing pages at the top of the image; needs a bootloader offset. |
| NVS/Preferences blob | ESP32 (`nvs_set_blob` or Arduino `EEPROM` lib) | An RTOS/SoC NVS service exists; shadow copy + commit on flush. |
| File-based | Virtual (`virtualeeprom` file) | Host platforms. |
| No persistence | LPC176X baseline has no-op writes/zero reads; other paths use `DISABLE_EEPROM_EMULATION` or `RAM_ONLY_SETTINGS` | For new backends, require an explicit board-layer opt-out. No-op storage is not an in-memory EEPROM implementation. |

`RAM_ONLY_SETTINGS` and `DISABLE_EEPROM_EMULATION` are **board-layer** flags
(defined in boardmaps or build flags, e.g. `boardmap_skr3.h`), never set by the
mcumap for new backends. For flash emulation, normally buffer changes in
`mcu_eeprom_putc` and commit in `mcu_eeprom_flush`; dedicated EEPROM may write
directly. Use the flash controller/SDK's required synchronization, not a blanket
interrupt-disable rule around an RTOS NVS call. Document erase/program latency,
execution-from-flash restrictions, interrupt/other-core coordination, reserved
linker/partition space, wear strategy and interrupted-commit behavior. Verify
reboot persistence; RAM-only operation and weak stubs cannot establish it.

### 4.6 IO extenders: soft shift vs custom shift provider

Pin expansion beyond the MCU is handled by the IO HAL and the shift-register
module, not by the backend. The mcumap only provides the `mcu_*` primitives for
real MCU pins; extended pins are encoded as negative `DIO<n>` by the boardmap
and dispatched to `ic74hc595_*`.

Two integration styles exist:

* **Soft shift (default)**: `ic74hc595_*` bit-bangs the shift registers through
two or three ordinary friendly pins (`IC74HC595_CLK_PIN`, `IC74HC595_DATA_PIN`,
`IC74HC595_LATCH_PIN`). No backend support needed.
* **Custom shift provider (optional capability)**: the backend replaces the
soft bit-bang with dedicated hardware. The mcumap defines
`IC74HC595_CUSTOM_SHIFT_IO` and the backend provides the
`MCU_CALLBACK void shift_register_io_pins(void)` function, writing the
daisy-chained 32-bit shadow word whenever the module updates it. Observed
implementations: PIO state machine (RP2040/2350, `ic74hc595.pio` generated
with `pioasm`), I2S + DMA (ESP32 family), soft SPI port (ESP8266). A new
backend may omit this and keep the soft default; if provided, document it in
the README (§2.1).

### 4.7 Extending the MCU HAL surface (extension protocol)

The MCU HAL evolves (recent example: "custom encoder read function hardware
independency", PR #977). Because every mcu.h entry is both a prototype and a
substitution point, growing the surface follows a fixed protocol:

1. **Classify the entry** using the API classes in §4.2. Decide explicitly
   whether unsupported behavior is valid.
2. **Guard replaceable entries**: wrap a macro/function substitution point in
   `mcu.h` inside `#ifndef <name>`.
3. **Prototype**: declare the function inside the guard, or unconditionally for
   a required non-replaceable function.
4. **Default only when safe**: add a weak implementation in `mcu.c` for an
   optional capability with a meaningful fallback. Use an empty macro only when
   a no-op is valid. For mandatory hardware behavior, deliberately omit a
   default so incomplete backends fail at link time.
5. Ship the interface and its first user together.
6. Surface modifications are **contract changes**: they require an ADR
   (`docs/adr/`) and a `BACKEND_GUIDE.md` diff in the same PR.

Backend-private helpers that are not meant to be overridable stay OUT of mcu.h
(e.g. STM32F1's `mcu_config_output_af`, the ESP32 `mcu_gen_*` family): they are
file-local or mcumap-local and follow the backend's own naming.

> Known retro-fits: `mcu_config_analog` and `mcu_config_input_isr` were called
> from generic init (`mcu.c`, `io_hal.h`) without guards, prototypes or
> defaults — they are now part of the guarded surface with weak defaults.
> `mcu_start_step_reset_timeout()` is an optional step-pulse
> hook called by the interpolator; only AVR implements it today.

---

## 5. Efficiency and flexibility rules (the "why" of every pattern)

These are authoring recommendations informed by existing backends; the access
policy explicitly marks requirements for new backends. They are not universal
claims about every current implementation:

1. **Resolve fixed pin operations at compile time.** Prefer register writes
   through `__indirect__`/`__helper__` token pasting or small inline helpers.
   This can remove lookup/call overhead; verify the generated instructions.
2. **mcu.h `#ifndef` guards are the substitution mechanism** — never define a
   macro in the mcumap without the corresponding `#ifndef` guard in mcu.h, and
   never ship a function that shadows a macro (guarded by the same macro).
3. **Compile-time feature pruning**: every peripheral, every pin, every servo is
   compiled out when unused. Backend code must live inside
   `#ifdef MCU_HAS_*`, `#if (SERVOS_MASK > 0)`, `#if ASSERT_PIN_IO(...)` guards.
   Generic code in `mcu.c` already does this per-pin — the backend must expose
   the macros that make it work (`mcu_config_*`, `mcu_set/clear/toggle_output`,
   ...), including for pins the backend itself doesn't know about.
4. **Weak defaults in `mcu.c`, strong overrides in the backend**:
   `mcu_io_init`, `mcu_io_reset`, `mcu_config_analog`,
   `mcu_config_input_isr`, `mcu_eeprom_*`, most `mcu_spi*` operations (but not
   `mcu_spi[_2]_xmit`), detached `mcu_*_rx_cb` hooks, and
   `mcu_i2c_slave_cb` have weak versions. `mcu_delay_loop`, `mcu_com_rx_cb`, and
   the SPI port tables are strong generic symbols. Required hardware operations
   intentionally have no fallback.
5. **Bounded timing work.** Prefer integer arithmetic in the pulse-generation
   path. Frequency conversion APIs accept floats and existing implementations
   divide in floating point; measure their cost if called on a timing-critical
   path. Avoid unbounded polling or SDK calls that can block in step ISRs.
6. **Predictable allocation**: prefer static buffers for motion and ISR state.
   Existing ESP32 DMA drivers allocate DMA-capable memory and the virtual
   backend allocates events dynamically. Where SDK services require allocation,
   do it outside the step path, handle failure and record lifetime/memory needs.
7. **`FORCEINLINE` (`__attribute__((always_inline)) inline`) for hot helpers**
   (`mcu_outputs_init`, servo helpers, `mcu_gen_*`), `static` where possible.
8. **Separate selection from clock measurement**: resource IDs and values used
   in `#if`/token pasting must be preprocessor constants. Clock expressions can
   be runtime values (`SystemCoreClock`, `HAL_RCC_GetPCLK*Freq()` in STM32).
   Use a compile-time frequency where delay expansion requires one, and verify
   it agrees with clock initialization. Board overrides win over defaults.
9. **Tiered peripheral access, bare-metal C99 first** (the "access tier" of
   CONTEXT.md). For new backends this is a selection rule. The reasons include
   performance, but also explicit ownership of execution flow: initialization,
   interrupt entry/exit, callback dispatch and background work should be visible
   and traceable. Prefer non-RTOS implementations whenever practical. Use the
   first viable tier:
   1. direct register access using vendor device/CMSIS headers;
   2. narrowly selected vendor C drivers or SDK facilities when required for
      complex hardware such as clocks, USB, networking, or flash, preferring
      synchronous or explicitly serviced non-RTOS paths;
   3. an RTOS service when the target platform intrinsically requires it;
   4. Arduino framework code or libraries only as a last-resort, explicitly
      opt-in adapter.

   The default backend path must remain C99. C++ is permitted only inside an
   isolated adapter required by a C++-only dependency, with an `extern "C"`
   boundary exposing plain C types to the backend. Framework convenience,
   Arduino examples, or support for only one Arduino board are not sufficient
   justification to choose tier 4. When the controller vendor supplies an SDK
   kit capable of building the firmware, the backend must support that path
   without Arduino. Each backend README records the chosen tier and any
   unavoidable exceptions.

   Inspect what a dependency executes, not just its public API or language.
   SDKs/frameworks can install callbacks, start background services, reserve
   timers, mask interrupts, defer events or invoke work from yield/wait paths.
   A small C wrapper does not remove those effects. Prefer a dependency whose
   required work can be called explicitly from backend initialization or
   `mcu_dotasks()` over one that introduces implicit scheduling and callbacks.

   RTOS integration adds scheduler and context-switch behavior to this analysis.
   Depending on the port, it can own tick/context-switch exceptions, constrain
   interrupt priorities and masking, and request rescheduling on ISR exit.
   Those interactions make the execution path harder to establish; an RTOS is
   not inherently nondeterministic, nor does every interrupt necessarily pass
   through its scheduler. Bare-metal code likewise still requires analysis of
   interrupt nesting, hardware stalls and shared state. The preference is for
   the smallest execution model whose behavior the backend can account for.

   When a higher-tier dependency is necessary, record:

   * which startup hooks, vectors, timers and execution contexts it owns;
   * where callbacks/events originate and whether they run in an ISR, task,
     polling loop or deferred handler, including possible reentrancy;
   * interrupt priority/masking constraints and any ISR-exit context switching;
   * blocking/yield points, background work and interactions with µCNC callbacks;
   * how the step/tick path remains bounded under that activity, with validation
     evidence from §7 and explicit unresolved assumptions.

   Keep these effects within a documented adapter boundary where feasible.
   Code generators must not add an RTOS, callback dispatcher or background task
   merely because a vendor example uses one. Establish the dependency's need
   and execution model before adopting its initialization or driver code.
10. **ISR code is context-aware when an RTOS is present**: `mcu_in_isr_context()`
    (`xPortInIsrContext()` on ESP32, `__get_IPSR() != 0` on ARM), atomic
    builtins (`__atomic_*`), and ISR/thread dual-mode primitives
    (`xSemaphoreTakeFromISR` vs `xSemaphoreTake`, `portYIELD_FROM_ISR`).
11. **Flash-string access is abstracted** via `__rom__`/`rom_*` so the same core
    source runs on Harvard (AVR) and von-Neumann (ARM) machines.
12. **`mcu_delay_cycles` is derived from per-MCU cycle constants**
    (`MCU_CLOCKS_PER_CYCLE`, `MCU_CYCLES_LOOP_OVERHEAD`, `MCU_CYCLES_PER_LOOP`)
    and is used for sub-microsecond timing (step reset, 50 ns delays) without
    hardware timer intervention.

---

## 6. Build integration and size optimization

Backends are consumed by both Makefiles and PlatformIO. Conventions:

* **Makefiles** (`makefiles/<arch>/Makefile`): recursive wildcard collects all
  `*.c` under `uCNC/`; per-arch toolchain + `-mmcu`/`-mcpu`; the backend
  self-selects via `#if (MCU == ...)`. Flags observed across architectures:
  `-Os`, `-ffunction-sections -fdata-sections`, `-Wl,--gc-sections`,
  `-flto -fuse-linker-plugin` (AVR, via `avr_compiler.py`), `-mcall-prologues
  -mrelax -fno-tree-scev-cprop` (AVR), `-specs=nano.specs -lc -lm -lnosys`
  (STM32), `-nostdlib -fno-threadsafe-statics -fno-rtti -fno-exceptions`
  (SAMD21), `-D MCU=MCU_<ARCH> -D BOARD=BOARD_<NAME>`.
* **PlatformIO**: per-family `.ini` files under `platformio.ini` `extra_configs`
  pass `build_flags` (`-D F_CPU=...`, `-D BOARDMAP="..."`), `lib_deps`
  (tinyUSB, Arduino cores, pico-sdk), `board_build.ldscript` (custom `.ld`),
  `extra_scripts` (`avr_compiler.py`, `ucnc_modules.py` module fetcher).
* **Linker scripts** (`stm32f4x.ld`, `stm32h723zg.ld`) derive memory regions
  from macros (`LD_FLASH_OFFSET`, `LD_MAX_SIZE`, `LD_MAX_DATA_SIZE`) so one
  script covers many chips in a family. Custom vector tables are optional and
  gated (`CUSTOM_PRE_MAIN` + `stm32f4x.c`).
* **Debug vs release**: `-Og -g3` debug targets; `-Os` + LTO + gc-sections
  release. Do not ship a backend without both paths documented.

---

## 7. Testing and simulation

Treat compilation, core behavior and physical hardware behavior as separate
evidence. Passing one does not establish the others.

### 7.1 Compile and link matrix

For a new backend, select cases applicable to the current implementation scope
and record build commands and expected results. Expand this matrix as support
grows; unavailable alternate chips/routes are follow-up coverage, not a demand
to implement the whole family before accepting the initial backend:

| Configuration | What it establishes |
|---|---|
| Minimal board with optional peripherals off | Required surface links; unused features do not pull in drivers/SDK dependencies |
| First supported board, normal feature set | Concrete integration of startup, clock, map, drivers and linker layout |
| Alternate/synthetic board | Different GPIO ports/banks, peripheral routes and timer selections work without backend edits |
| Each declared silicon programming-model class | Distinct register/IRQ/clock branches compile against real device headers |
| Enabled UART2/SPI2, ADC, PWM, servo, oneshot and DMA as supported | Optional API implementations and their resource interactions are exercised |
| Invalid route, absent instance and resource collision | Each fails for the intended reason with a useful diagnostic |
| Unrelated backend | Newly added translation units are correctly guarded |
| Default native C99 path and each advertised adapter path | Provider selection, C linkage and dependencies are correct |

Choose synthetic cases to exercise boundary differences, not arbitrary renamed
pins. Cover each distinct resolver branch; exhaust small route tables in
compile-only probes where practical. Include direct, unused and extender pins
and the highest supported bank/bit. Synthetic maps prove compile-time
adaptability, not physical routability. Link representative full images and
inspect symbols/map/disassembly for duplicate vectors, unresolved weak-stub
dependencies, flash/NVM overlap, RAM use and hot-path lookup overhead.

### 7.2 Host regression

The **virtual backend** supports Windows/Linux host IO and a deterministic clock
under `PIO_UNIT_TESTING` (`mcu_unit_test_advance_time`,
`mcu_unit_test_inject`, `mcu_add_event`). Run relevant Unity fixtures through
the configured emulator environment; see `test/README.md` and `virtual/virtual.ini`.
For example, from the repository root with PlatformIO on PATH:

```sh
pio test -e EMULATOR_WINDOWS_TEST
```

These fixtures rely on virtual clock/state/injection helpers. They are not an
unchanged on-device serial test suite, and hardware backends need not implement
those host hooks. Host regression is especially useful when changing shared
HAL/core code; it does not validate a new MCU's registers, pin mux or interrupts.

### 7.3 Hardware acceptance

Use a hardware test harness or adapted protocol tests plus a logic analyzer or
scope. Record the board, part/package, clock, firmware configuration and results:

* step frequency and pulse width at low/high rates, rate changes, stop/restart,
  pending reset handling, and direction setup/hold;
* 1 ms tick and microsecond reads across wraparound and interrupt masking,
  callback non-reentrancy, and worst observed jitter under communications load;
* limit/control/probe/input callbacks, including enable/disable and shared IRQs;
* stream RX overflow and realtime/detached routing, TX progress, peripheral
  timeout behavior, and enabled ADC/PWM/servo/oneshot functions;
* NVM write/flush/reboot readback and the documented interrupted-write behavior;
* buffered-output latency and stop response, DMA/cache visibility and multicore
  synchronization when those mechanisms are used.

State which configurations were only compiled and which were measured. Report
support for the specific implemented chips/features; a validated initial backend
is complete for that scope without claiming exhaustive family support.

---

## 8. Minimum hardware requirements (gate for a new backend)

From the MCU requirements in `README.md`, a target MCU should provide, to be a
*complete* backend:

* at least 2 hardware timers (ITP + RTC; 1 is possible with limitations);
* at least one communications port (UART/USB/...);
* non-volatile memory (optional: settings can be RAM-only);
* PWM capable IO (optional);
* input pins with interrupt-on-change (optional: soft polling with feature loss);
* ADC inputs (optional).

---

## 9. New-backend template checklist (rolled up)

- [ ] `src/hal/mcus/mcus.h`: `MCU_<ARCH>` id added.
- [ ] `src/hal/mcus/<arch>/mcumap_<arch>.h` follows the expansion patterns in
      §3.1 and uses the recommended §3.2 section order. It contains all 192 canonical `DIO<n>`
      blocks, parameterized GPIO/peripheral mappings for the implemented scope,
      and clear resolution points for later chip/instance/route additions.
- [ ] Where alternate selections are implemented, a second or synthetic boardmap
      changes GPIO/peripheral/timer selections without backend source edits;
      otherwise review the selection boundary and record pending coverage.
- [ ] Supported part/package/SDK matrix and source provenance recorded; layout
      separates capabilities, board selections, generated aliases and runtime ownership (§2.2).
- [ ] Route tuples and resource conflicts validated, including intentional
      sharing, with negative compile cases (§3.4 and §7.1).
- [ ] `mcudefs.h` include block added.
- [ ] Runtime modules and mcumap provide each enabled required `mcu.h` entry;
      safe generic defaults are reused, ISRs wired to the §4.3 callbacks, feature-pruned.
- [ ] `#ifndef` guards respected: no symbol defined twice (macro vs function).
- [ ] Delay/cycle macros, `__rom__` family, F_CPU defaults present.
- [ ] `mcu_dotasks()` implemented or left empty (never called from RTC).
- [ ] EEPROM strategy chosen following the §4.5 decision table, honoring
      `NVM_STORAGE_SIZE` (RAM-only only as an explicit board-layer opt-out).
- [ ] Boardmap(s) + `boards_helper.h` entry (or `-D BOARD=BOARD_CUSTOM` path).
- [ ] Build glue includes a C99 vendor-header/SDK path without Arduino whenever
      the vendor kit permits it, with `-Os`, `-ffunction-sections`,
      `-fdata-sections`, and `-Wl,--gc-sections` (+arch flags). Any Arduino/C++ adapter is isolated,
      opt-in, and justified in the backend README.
- [ ] Non-RTOS operation preferred where practical; required SDK/framework/RTOS
      execution paths, IRQ ownership and callback contexts documented (§5.9).
- [ ] `README.md` following the §2.1 skeleton (toolchain/access tier,
      timers/IRQs/priorities, primary stream per board, NVM strategy,
      limitations and deviations).
- [ ] Any `mcu.h` surface addition classified and shipped according to §4.7;
      contract changes also carry an ADR.
- [ ] Reproducible generation record and alias/expansion checks (§3.5).
- [ ] Optional: custom shift provider for IC74HC595 (§4.6).
- [ ] Compile/link matrix, relevant host regression and hardware measurements
      recorded separately (§7), with untested capabilities explicitly identified.

---

## 10. References

* Interface & defaults: `src/hal/mcus/mcu.h`, `src/hal/mcus/mcu.c`
  (weak defaults, generic IO init), `src/hal/mcus/mcudefs.h`, `mcus.h`.
* User guide + canonical pin table: `src/hal/mcus/README.md`.
* Reference backends: `avr/` (minimal Harvard), `stm32f1x/` (CMSIS + tinyUSB +
  flash EEPROM), `samd21/` (register-based peripheral code), `rp2040/`
  (framework + PIO + alarms), `esp32/` + `esp32common/` (RTOS, shared code),
  `virtual/` (simulator + unit-test hooks).
* Build: `makefiles/*/Makefile`, `platformio.ini`, `avr_compiler.py`,
  `ucnc_modules.py`.
* Generator: `docs/mcumap_gen.xlsx`.
* Glossary & decisions: `CONTEXT.md` (repo root), `docs/adr/0001-pin-numbering-space.md`,
  `docs/adr/0002-tiered-peripheral-access.md`.

### 10.1 Backend comparison: what to reuse and what to re-evaluate

Source review, 2026-09-09: covers all 13 hardware backend entry pairs, the
virtual entry pair, family-common code and relevant adapters/build configuration.
This is an authoring-pattern comparison, not certification of every silicon
route or hardware timing behavior. Paths below are relative to this guide.

| Backend | Useful implementation evidence | Adaptation boundary to review |
|---|---|---|
| [AVR](avr/mcumap_avr.h) / [runtime](avr/mcu_avr.c) | Generated port/register aliases, Harvard ROM access, separate ITP/RTC compare handling, hardware EEPROM | Timer width/divider encodings, pin-change groups and direct EEPROM capacity; AVR interrupt-mask conventions are local |
| [STM32F0](stm32f0x/mcumap_stm32f0x.h) / [runtime](stm32f0x/mcu_stm32f0x.c) | Register GPIO, peripheral selection, timer/SysTick separation | Device-specific alternate functions, shared vectors and Cortex-M0 delay/IRQ capabilities |
| [STM32F1](stm32f1x/mcumap_stm32f1x.h) / [runtime](stm32f1x/mcu_stm32f1x.c) | Explicit UART/SPI/I2C remap diagnostics and timer-to-clock/IRQ mapping | AFIO remap model, timer clock derivation and flash geometry; do not transplant F1 GPIO configuration into F4/H7 |
| [STM32F4](stm32f4x/mcumap_stm32f4x.h) / [runtime](stm32f4x/mcu_stm32f4x.c) | AF-based GPIO, DMA/peripheral mappings, separate startup and linker files | Part-specific AF/DMA routes, APB timer clocks, sectors and bootloader placement |
| [STM32H7](stm32h7x/mcumap_stm32h7x.h) / [runtime](stm32h7x/mcu_stm32h7x.c) | H7-specific peripheral code, linker script and optional adapter | Verify clock domains, peripheral IP revisions, DMA-visible memory and cache policy; similarity to F4 is insufficient |
| [SAMD21](samd21/mcumap_samd21.h) / [runtime](samd21/mcu_samd21.c) | SERCOM mux/pad resolution, TC/TCC branches, synchronization waits | Serial-block collisions, valid pad combinations and distinct timer register layouts; register-level code alone does not prove framework-free startup |
| [LPC176X](lpc176x/mcumap_lpc176x.h) / [runtime](lpc176x/mcu_lpc176x.c) | PINSEL route validation, timer/SSP/DMA mappings | Framework-owned SysTick and no-op NVM implementation need explicit treatment in a new backend |
| [RP2040](rp2040/mcumap_rp2040.h) / [runtime](rp2040/mcu_rp2040.c) | Hardware alarm scheduling, PIO extender, SDK SPI/DMA paths, multicore option | Current map includes Arduino and delegates GPIO configuration/ADC/PWM; isolate these dependencies when applying the new-backend baseline |
| [RP2350](rp2350/mcumap_rp2350.h) / [runtime](rp2350/mcu_rp2350.c) | Related alarm/PIO design in a separate backend | Current map uses low-bank `sio_hw->gpio_*` operations and ARM IRQ primitives; this is not proof of all package GPIO banks or CPU modes |
| [ESP8266](esp8266/mcumap_esp8266.h) / [runtime](esp8266/mcu_esp8266.c) | Buffered output with separate signal, UART, SPI, flash and networking modules | Logical writes can update a shadow before physical output; evaluate buffering, stop latency, SDK service context and framework dependencies |
| [ESP32](esp32/mcumap_esp32.h) / [runtime](esp32/mcu_esp32.c) | Family-common services plus target-specific I2S/SPI/NVM files | DMA-capable allocations and direct/buffered IO ownership; inspect selected native/adapter providers |
| [ESP32C3](esp32c3/mcumap_esp32c3.h) / [runtime](esp32c3/mcu_esp32c3.c) | Reuses common signal/task/UART code with its own I2S driver and timer selection | Target-specific IRQ/instruction and peripheral differences; shared SDK brand does not imply identical hardware |
| [ESP32S3](esp32s3/mcumap_esp32s3.h) / [runtime](esp32s3/mcu_esp32s3.c) | Common services with separate GPIO/I2S/GDMA configuration | Bank selection, USB/SDK configuration and DMA request/descriptor semantics |
| [Virtual](virtual/mcumap_virtual.h) / [runtime](virtual/mcu_virtual.c) | Common simulator with separate [Windows](virtual/virtual_windows.c) / [Linux](virtual/virtual_linux.c) services and deterministic test events | Host allocation, time and IO conventions are not MCU register templates; the map has 152 direct `DIO<n>` definitions rather than 192 |

All 13 hardware maps contain 192 direct canonical `DIO<n>` definitions in this
snapshot. That is alias-surface coverage, not proof of complete peripheral route
coverage. [ESP32 common code](esp32common/esp32_common.h) and its guarded runtime
modules are the clearest existing shared-family layout. The root PlatformIO
`[env]` defaults to Arduino, and multiple maps include `Arduino.h` directly;
reuse their hardware algorithms without assuming their dependency boundaries
already satisfy §2's new-backend policy.
