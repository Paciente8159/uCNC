# µCNC MCU Backend Authoring Guide

This document combines the normative µCNC **MCU HAL contract** with recommended
backend-authoring patterns. The keywords below distinguish the two:

* **Required**: observable behavior or API/ABI compatibility that every backend
  must preserve. A deviation is a contract change and requires an ADR.
* **Recommended**: the common implementation or layout. A backend may deviate
  when its architecture requires it and should document the reason.
* **Example**: an observed implementation, not a requirement.

Section 4 and the expected-behavior rules in §3.3 are normative. Directory
layout, section ordering, access mechanisms and implementation techniques are
authoring guidance unless explicitly labeled **Required**.

Reference backends: `avr`, `stm32f0x/f1x/f4x/h7x`, `samd21`, `lpc176x`,
`rp2040/rp2350`, `esp8266`, `esp32/esp32c3/esp32s3`, `virtual`.

It is written for backend authors and code generators. The companion document
[`README.md`](README.md) (same folder as BACKEND_GUIDE.md) is the user-facing guide and contains the
canonical pin numbering table; this spec references it as the single source of
truth for pin numbers. Terminology is pinned in `CONTEXT.md` (repo root);
contract-level decisions live in `docs/adr/`.

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
* The **MCU HAL** (what this spec describes) is two files:
  * `mcumap_<arch>.h` — compile-time glue: pin aliases, register access macros,
    feature flags, macro implementations of `mcu_*` calls.
  * `mcu_<arch>.c` — runtime code: ISRs, peripherals init, functions whose
    behavior cannot be a macro.
* The **IO HAL** (`io_hal.h`) consumes exclusively the MCU HAL surface defined in
  `src/hal/mcus/mcu.h` and the values of `DIO<n>` macros set by the mcumap.

### 1.1 Configuration flow (how `MCU` is selected and processed)

1. Build system passes `-D MCU=MCU_<ARCH>` (and optionally `-D BOARD=BOARD_<NAME>`,
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
   the friendly pins and `#define MCU MCU_<ARCH>`.
4. `boarddefs.h` then includes `src/hal/mcus/mcudefs.h`, which includes
   `mcus.h` (MCU id list) and then the matching `mcumap_<arch>.h`, and finally
   `mcu.h` (the interface contract). See `mcudefs.h:27-124`.
5. Every `mcu_<arch>.c` file in the tree is compiled (makefiles use a recursive
   `rwildcard` over `*.c`; PlatformIO uses `src_dir=uCNC`), and self-selects with
   `#if (MCU == MCU_<ARCH>)` at the top. This is what allows all backends to live
   in the same tree with zero build-system bookkeeping per backend.

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

1. Target chips / boards supported by this backend.
2. Toolchain and build recipes, including at least one C99/vendor-SDK build that
   does not require Arduino when the vendor kit permits it. State the access
   tier used (§5.9), all optional framework flags, and the reason for any
   unavoidable Arduino or C++ dependency.
3. Resource allocation table: timers/IRQs used (ITP/RTC/SERVO/ONESHOT), the
   assigned interrupt priorities, and the priority ordering chosen (§3.3).
4. Primary stream per supported board (UART1 vs USB-CDC, §4.4).
5. NVM strategy chosen (§4.5) and any board-layer RAM-only opt-out.
6. Known limitations and any deliberate deviations from this spec.

---

## 3. The `mcumap_<arch>.h` contract (compile-time layer)

The mcumap is the heart of the backend. Stateless operations resolvable at
compile time should normally be macros (see §5). The following stable section
order is recommended because it makes generated and hand-written backends easy
to compare; it is not part of the MCU HAL ABI.

### 3.1 MCU-family scope and board independence (Required)

An MCU backend is a reusable description of an **MCU family**, not an
implementation of the first development board used to test it. The boardmap
selects pins, peripheral instances, and resource assignments; the mcumap must
provide the machinery needed for other valid selections without editing the
backend.

A new backend is incomplete if it only supports the pins and routes used by its
initial board (for example, only the Arduino UNO R4 wiring of an RA4M1). Before
calling a backend complete, its author must inventory the target family from
the vendor datasheet, user manual, and device headers and implement:

* all canonical µCNC friendly-pin blocks in the pin-numbering table, including
  axes, enables, PWM, servo, generic digital outputs and inputs, limits,
  controls, analog inputs, and communications pins. Each block is conditional
  on the boardmap's `<PIN>_PORT`/`<PIN>_BIT` or equivalent definitions, so unused
  roles compile away. Mature mcumaps define all 192 current canonical `DIO<n>`
  entries even though no single board uses all of them;
* generic GPIO token expansion for every GPIO port and bit exposed by the
  supported MCU family/package set, rather than a table of Arduino board pin
  numbers or a switch containing only the initial board's pins;
* every usable instance and valid pin-routing/remap choice for UART/USART, SPI,
  I2C, ADC, PWM/timers, input interrupts, USB, and other capabilities advertised
  by the backend. Boardmap-selected instances and routes must either resolve to
  the correct registers/interrupts/alternate-function settings or fail at
  compile time with a precise unsupported-combination error;
* configurable timer, channel, IRQ, DMA, and peripheral allocation wherever the
  hardware offers choices. Do not bake the initial board's assignments into
  runtime code; derive them from boardmap/configuration macros;
* family/package capability guards for real silicon differences. It is valid to
  exclude a route that does not exist on a selected part, but not to omit a
  documented route merely because the first board does not expose it.

The backend need not create boardmaps for every possible PCB. It must make a
new boardmap possible without changing `mcu_<arch>.c` and, except when adding
support for a genuine new silicon variant or package capability, without
changing `mcumap_<arch>.h`.

Reviewers should test this boundary with at least one synthetic or second
boardmap that changes GPIO ports and peripheral/timer selections. Successfully
building only the initial development board is smoke coverage, not evidence of
a complete mcumap.

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
   ESP32 `rsr.ccount`, SAMD21 DWT). A `mcu_nop()` may be provided too.
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
8. **Token-pasting helpers** — mandatory, used by every macro below:
   ```c
   #define __helper_ex__(left, mid, right) (left##mid##right)
   #define __helper__(left, mid, right)    (__helper_ex__(left, mid, right))
   #ifndef __indirect__
   #define __indirect__ex__(X, Y) DIO##X##_##Y
   #define __indirect__(X, Y)    __indirect__ex__(X, Y)
   #endif
   ```
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
   #define STEP0_BIT (STEP0_BIT)       /* passthrough where *_BIT is already the value */
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
   * The table is long and mechanical: existing headers are **generated from
     `docs/mcumap_gen.xlsx`** (see the "Autogenerated macros" comments). Reuse
     that generator for new backends.
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

The allocator produces, from a plain number, the peripheral pointer, the clock
enable bit, the clock source and the **IRQ handler name** via token pasting
(so a single `mcu_<arch>.c` works for any timer choice made by the board):

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

Special cases handled in the same block:
* shared IRQ lines (e.g. `TIM1_UP_TIM10_IRQHandler`, `TIM6_DAC_IRQHandler`,
  `USART3_8_IRQHandler`) — map by timer number with `#if` chains;
* different register families (SAMD21: `TCCn` vs `TCn` chosen by `ITP_TIMER < 3`);
* no hardware timers (RP2040: hardware **alarm** timers with
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
| AVR | Timer compare: OCR A = period, OCR B = half period | Same timer, compare A | RTC ISR also multiplexes servo pulses; defines `mcu_start_step_reset_timeout()` |
| STM32 F1/F4/F0/H7 | Single timer ISR, `PSC/ARR` halving + `resetstep` toggle | SysTick increments `mcu_runtime_ms`, pends low-priority `PendSV_Handler` | NVIC priority table in the mcumap |
| SAMD21 | TC/TCC compare + toggle | SysTick → PendSV | TCC vs TC selected by timer index |
| LPC176X | `LPC_TIMx` compare | SysTick → PendSV | Needs framework clock |
| RP2040/RP2350 | Hardware alarm slot, ISR re-arms + toggles | Alarm re-enqueued each 1 ms | One alarm pool shared with servo/oneshot |
| ESP32 S3/C3 | Timer-group ISR, accumulator at `ITP_SAMPLE_RATE = F_STEP_MAX * 2` | FreeRTOS task (`mcu_rtc_task`) + `esp_system_get_time()` | `mcu_gen_step/pwm/servo` run in the ISR |
| ESP8266 | `timer1` ISR + **buffered IO** (`out_io_buffer`, ~10 ms precomputed) | `os_timer` 1 ms | Deliberately different design forced by timer scarcity — still satisfies the contract |
| Virtual | Software tick `mcu_gen_step()` at `2×F_STEP_MAX` | `tickcount` sampling | Deterministic under `PIO_UNIT_TESTING` |

Recommended interrupt priority topology (where priorities are programmable,
e.g. ARM NVIC): input-change and ITP at the top, then the RTC tick, then
servo/oneshot, then comms (UART/USB). Follow the `NVIC_*_IRQ_Pri` table pattern
(`mcumap_stm32f1x.h:62-70`) and document the assigned values in the backend
README (§2.1).

---

## 4. The `mcu_<arch>.c` contract (runtime layer)

### 4.1 The "macro-or-function" duality (the #1 performance rule)

`mcu.h` declares every IO/time operation inside `#ifndef` guards:

```c
#ifndef mcu_set_output
void mcu_set_output(uint8_t pin);
#endif
```

Consequences, by design:
1. If the mcumap defines `mcu_set_output` as a macro (register write), the
   guard suppresses the prototype **and** any function definition, so the call
   site compiles to a single store — no function call, no `switch` on pin.
2. If the mcumap does **not** define it, a real C function must exist somewhere
   (the backend, or a weak default in `mcu.c`).
3. This is why IO (and `mcu_enable/disable_global_isr`, `mcu_get_analog`,
   `mcu_config_pwm`, `mcu_get/set_pwm`, probe ISR, `mcu_millis`, ...) is
   *macro-first* in every existing backend, and only falls back to functions for
   stateful things (servo, eeprom, streams).

Writing the IO macros — canonical shapes across all backends:

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
The compiler sees literal peripheral addresses and bit numbers, so the whole IO
layer costs zero runtime overhead and zero code size per pin.

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
| `mcu_freq_to_clocks(float, *ticks, *prescaller)` / `mcu_clocks_to_freq` | `frequency = CLAMP(F_STEP_MIN, frequency, F_STEP_MAX)`; convert Hz → ticks/prescaler with **integer shifts only** (`clocks = F_CPU/freq`, `while (clocks > 0xFFFF) { clocks >>= k; prescaller++ }`) so the encodings are cheap and match the hardware divider table. `ticks/prescaller` form an opaque pair to the core; the backend defines the encoding. |
| `mcu_start_itp_isr(ticks, prescaller)` | Program timer channel A (=period) and B (=half period), reset counter, clear flags, enable both compare interrupts, start clock. |
| `mcu_change_itp_isr(ticks, prescaller)` | Same minus enable/re-enable; used to change step rate mid-motion. |
| `mcu_stop_itp_isr(void)` | Stop clock; usually *leave the interrupt mask enabled* (the RTC logic uses it as a "stepping in progress" gate; see the comment in the AVR implementation). |
| `mcu_start_step_reset_timeout()` | Optional empty macro declared near the step-interpolator API in `mcu.h`. Hook to shorten/re-arm the step pulse or re-enable interrupts right after a step event; called by the interpolator. Only AVR defines it today. |
| `mcu_millis`, `mcu_micros`, `mcu_free_micros` | `mcu_runtime_ms` incremented by the 1 ms tick; `mcu_micros = 1000*ms + free_micros`. |
| `MCU_ITP_ISR` | Alternating `mcu_step_cb()` / `mcu_step_reset_cb()` (see §3.3). |
| RTC/PendSV ISR | `mcu_runtime_ms++; pends low-priority task` → `mcu_rtc_cb(mcu_runtime_ms)`. |
| `mcu_uart_init/getc/available/clear/putc/flush` + `mcu_uart2_*` | Buffers are global, `DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE)` + `DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE)` and initialized in `mcu_coms_init` (`BUFFER_INIT`). RX ISR: `if (mcu_com_rx_cb(c)) BUFFER_TRY_ENQUEUE(uart_rx, &c); else STREAM_OVF(c);`. If `DETACH_UART_FROM_MAIN_PROTOCOL`, route to `mcu_uart_rx_cb(c)` instead (weak empty default in `mcu.c`). |
| `mcu_usb_*` (if `MCU_HAS_USB`) | tinyUSB device (include `<tusb_ucnc.h>`): `mcu_usb_init → tusb_cdc_init`, USB IRQ → `tusb_cdc_isr_handler`, `mcu_dotasks → tusb_cdc_task()` + drain `tusb_cdc_read()` into `mcu_com_rx_cb`. Non-tinyUSB platforms wrap their stack (Arduino `Serial`, `USBCDC`). |
| `mcu_spi_init/config/start/stop/xmit/bulk_transfer` + `mcu_spi2_*` (if `MCU_HAS_SPI[2]`) | `mcu.c` provides weak defaults for init/config/start/stop/bulk transfer. The byte `mcu_spi[_2]_xmit` primitive is required from the backend or mcumap. Bulk transfer rolls over that primitive with `BULK_SPI_TIMEOUT` + `TASK_YIELD()`. `mcu_spi_port`/`mcu_spi2_port` are generic, non-weak function tables `{isbusy, start, xmit, bulk_xmit, stop}` initialized by `mcu.c`; customize behavior by overriding the weak functions or macro substitution points, not by defining a second table. |
| `mcu_i2c_init/config/send/receive` (if `MCU_HAS_I2C`) | Master API with `ms_timeout`; slave support when `MCU_SUPPORTS_I2C_SLAVE && I2C_ADDRESS != 0` calls `mcu_i2c_slave_cb` (weak default in mcu.c). |
| `mcu_set_servo/get_servo` | Body wrapped `#if SERVOS_MASK > 0`; servo pulse train is multiplexed into a shared timer ISR with per-servo `_FRAME` compile-time `#if` pruning (AVR/STM32/SAMD21 pattern). |
| `mcu_config_timeout/start_timeout` (if `MCU_HAS_ONESHOT_TIMER`) | `mcu_timeout_cb` delegate stored globally in `mcu.c`; ISR calls it. `MCU_ONESHOT_ISR`. |
| Input-change ISRs | Per-port/per-line handlers routing to `mcu_limits_changed_cb`, `mcu_controls_changed_cb`, `mcu_probe_changed_cb`, `mcu_inputs_changed_cb`, pruned at compile time by pin bit-masks (AVR `PCINTA_LIMITS_MASK & ...`, STM32 `ALL_EXTIBITMASK`). When ISR-less, the soft-polling path (`FORCE_SOFT_POLLING`) feeds the same callbacks from IO processing. |
| `mcu_eeprom_getc/putc/flush` | Weak non-volatile stubs in `mcu.c`; real backends override with one of the strategies in §4.5. All honor `NVM_STORAGE_SIZE` (default `0x400`). |
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

Each enabled port is exposed as a stream with a fixed shape: `mcu_<port>_init`,
`mcu_<port>_getc`, `mcu_<port>_available`, `mcu_<port>_clear`,
`mcu_<port>_putc`, `mcu_<port>_flush`, gated by the matching `MCU_HAS_<PORT>`
flag (`MCU_HAS_UART`, `MCU_HAS_UART2`, `MCU_HAS_USB`, `MCU_HAS_BLUETOOTH`, and
`ENABLE_SOCKETS` for the telnet stream). TX/RX ring buffers are declared with
`DECL_BUFFER` (`uart_rx`/`uart_tx`, `uart2_*`, `usb_*`, `bt_*`) and initialized
in `mcu_coms_init()`.

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
| Hardware EEPROM | AVR (`avr/eeprom.h`) | The MCU has dedicated EEPROM; simplest and cheapest. |
| Flash-page emulation | STM32F1/F0 (inverted-bit pages, `FLASH_CR` sequence), SAMD21 (row erase + `NVMCTRL`), ESP8266 (sector wear-levelling) | No EEPROM, flash budget allows stealing pages at the top of the image; needs a bootloader offset. |
| NVS/Preferences blob | ESP32 (`nvs_set_blob` or Arduino `EEPROM` lib) | An RTOS/SoC NVS service exists; shadow copy + commit on flush. |
| File-based | Virtual (`virtualeeprom` file) | Host platforms. |
| None (RAM-only) | LPC176X baseline, STM32 with `DISABLE_EEPROM_EMULATION`, boards with `RAM_ONLY_SETTINGS` | Acceptable when no NVM exists; must be an explicit board-layer opt-out, not a silent backend default. |

`RAM_ONLY_SETTINGS` and `DISABLE_EEPROM_EMULATION` are **board-layer** flags
(defined in boardmaps or build flags, e.g. `boardmap_skr3.h`), never set by the
mcumap. On flash targets, writes are guarded with interrupts disabled and
flushed via `mcu_eeprom_flush`, never byte-by-byte from `mcu_eeprom_putc`.

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

These are the properties every existing backend exhibits; a new backend should
ideally preserve them to reach the same build-time efficiency and flexibility:

1. **Macros replace functions whenever the operation is stateless and
   resolvable at compile time.** Cost: zero (no call, no switch, no RAM).
   Prefer register writes through `__indirect__`/`__helper__` token pasting.
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
5. **Integer math in ISRs.** All hot paths (`mcu_freq_to_clocks`,
   accumulator steppers, PWM scaling) use integer arithmetic and shifts;
   floating point appears only at configuration time.
6. **Static allocation, no malloc**: all buffers are compile-time
   (`DECL_BUFFER`/`BUFFER_INIT`, static shadows, predefined tables). Memory
   layout is deterministic at build time.
7. **`FORCEINLINE` (`__attribute__((always_inline)) inline`) for hot helpers**
   (`mcu_outputs_init`, servo helpers, `mcu_gen_*`), `static` where possible.
8. **Constants are compile-time**: `F_CPU`, `BAUDRATE`, `SPI_FREQ`, `I2C_FREQ`,
   `ITP_TIMER`, `NVIC_*_IRQ_Pri` must be resolvable by the preprocessor
   (hence the `#warning` if `F_CPU` is not a constant). Board overrides win over
   mcumap defaults (`#ifndef` chains).
9. **Tiered peripheral access, bare-metal C99 first** (the "access tier" of
   CONTEXT.md). For new backends this is a selection rule, not merely a size
   preference. Use the first viable tier:
   1. direct register access using vendor device/CMSIS headers;
   2. the vendor's C SDK or narrowly selected C drivers when they are required
      for complex hardware such as clocks, USB, networking, or flash;
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

* The **virtual backend** (`MCU_VIRTUAL_WIN`/`MCU_VIRTUAL_LINUX`) is the
  reference for a fully simulated MCU: software tick (`EMULATION_MS_TICK`),
  `mcu_gen_step()` driven by `ITP_SAMPLE_RATE`, file-backed EEPROM
  (`virtualeeprom`), named-pipe/Unix-socket IO server, VCD step stimulus, and a
  deterministic **unit-test clock** behind `PIO_UNIT_TESTING`
  (`mcu_unit_test_advance_time`, `mcu_unit_test_inject`, `mcu_add_event`).
* All regression tests (`test/`) run the *whole firmware* through the virtual
  backend with Unity. That deterministic-clock contract (`PIO_UNIT_TESTING`:
  `mcu_unit_test_advance_time`, `mcu_unit_test_inject`, `mcu_add_event`) is
  **only expected of host-platform backends**; a microcontroller backend does
  not need to re-create it.
* Hardware backends validate timing and behavior with the same core tests run
  on-device via the serial console (or through a host-ported virtual backend
  when one exists for the family).

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
- [ ] `src/hal/mcus/<arch>/mcumap_<arch>.h` follows the MCU-family scope rule in
      §3.1 and the §3.2 section order. It contains all 192 canonical `DIO<n>`
      blocks, generic GPIO support across the supported family/package set, and
      configurable mappings for all advertised peripheral instances/routes;
      it is not limited to the initial board.
- [ ] A second or synthetic boardmap changes GPIO ports plus at least one
      UART/SPI/I2C/timer selection and builds without backend source edits.
- [ ] `mcudefs.h` include block added.
- [ ] `src/hal/mcus/<arch>/mcu_<arch>.c` implements every `mcu.h` entry
      (function or macro), ISRs wired to the §4.3 callbacks, feature-pruned.
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
- [ ] `README.md` following the §2.1 skeleton (toolchain/access tier,
      timers/IRQs/priorities, primary stream per board, NVM strategy,
      limitations and deviations).
- [ ] Any `mcu.h` surface addition classified and shipped according to §4.7;
      contract changes also carry an ADR.
- [ ] Optional: custom shift provider for IC74HC595 (§4.6); mcumap regenerated
      via `docs/mcumap_gen.xlsx` generator to stay consistent with other
      backends.
- [ ] Compile with `MCU=<arch>` and validate: run the `test/` suite through the
      virtual backend on a host (host backends), or the same core tests
      on-device via the serial console (hardware backends).

---

## 10. References

* Interface & defaults: `src/hal/mcus/mcu.h`, `src/hal/mcus/mcu.c`
  (weak defaults, generic IO init), `src/hal/mcus/mcudefs.h`, `mcus.h`.
* User guide + canonical pin table: `src/hal/mcus/README.md`.
* Reference backends: `avr/` (minimal Harvard), `stm32f1x/` (CMSIS + tinyUSB +
  flash EEPROM), `samd21/` (bare-metal ARM, no framework), `rp2040/`
  (framework + PIO + alarms), `esp32/` + `esp32common/` (RTOS, shared code),
  `virtual/` (simulator + unit-test hooks).
* Build: `makefiles/*/Makefile`, `platformio.ini`, `avr_compiler.py`,
  `ucnc_modules.py`.
* Generator: `docs/mcumap_gen.xlsx`.
* Glossary & decisions: `CONTEXT.md` (repo root), `docs/adr/0001-pin-numbering-space.md`,
  `docs/adr/0002-tiered-peripheral-access.md`.
