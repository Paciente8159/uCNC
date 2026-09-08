# µCNC MCU Backend Context

Terms used across the µCNC MCU backend layer (`uCNC/src/hal/mcus/`) and its governing document (`BACKEND_GUIDE.md`). This glossary is the authoritative vocabulary; deviations in new backends or docs should be corrected to it.

## Layers

**MCU HAL**:
The interface contract between µCNC core and the microcontroller: the `mcu_*` function/macro surface declared in `mcu.h` and given generic defaults in `mcu.c`.
_Avoid_: backend (see below), HAL

**MCU backend**:
The family-specific implementation of the MCU HAL for one chip family (e.g. AVR, STM32F4, RP2040): a `mcu_<arch>.c` implementation unit, a `mcumap_<arch>.h` compile-time map, optional platform glue files, and a `README.md`. `BACKEND_GUIDE.md` defines the normative MCU HAL behavior and API rules while documenting recommended patterns for producing backends with similar structure and performance.
_Avoid_: port, driver, HAL implementation

**mcumap**:
The compile-time map header (`mcumap_<arch>.h`) of a backend: pin-to-register aliases, feature flags, timer allocation, and macro implementations of the `mcu_*` calls. Everything it can express as a preprocessor macro is expressed as one.

**Board HAL**:
The board-level definitions (`src/hal/boards/*/boardmap_*.h`) that name physical pins (friendly pin names) and select the MCU (`#define MCU MCU_<ARCH>`).
_Avoid_: boardmap (the artifact, not the layer)

**IO HAL**:
The pin-dispatch layer (`src/hal/io_hal.h`) that resolves every friendly pin name to either an MCU pin (positive `DIO<n>`) or an extender pin (negative `DIO<n>`), at compile time.

## Pins

**Friendly pin name**:
The µCNC-internal name for a pin function (e.g. `STEP0`, `DOUT0`, `LIMIT_X`, `PWM3`), defined by a boardmap and translated by the IO HAL.
_Avoid_: pin alias, logical pin

**Canonical pin number**:
The fixed numeric identity of a friendly pin (e.g. `STEP0 = 1`, `DOUT0 = 47`), shared by every backend via the README table. Range `1..215` is frozen for compatibility; `216..254` is reserved for future official expansion (communications or special applications mainly); values above that and negative values are not canonical MCU pins.

**Extended pin**:
A pin served by an IO extender (e.g. IC74HC595), encoded as a negative `DIO<n>` value by the boardmap. Negative values are exclusively the IO HAL extender mechanism; backends never define them.
_Avoid_: virtual pin, soft pin

## Backend construction

**Feature flag**:
A `MCU_HAS_*` (or `MCU_SUPPORTS_*`) preprocessor flag that the mcumap derives from the board's pin and peripheral definitions, used to compile peripherals and code paths out when unused.

**Weak default**:
A `__attribute__((weak))` generic implementation in `mcu.c` (e.g. `mcu_io_init`, `mcu_eeprom_getc`, `mcu_spi_start`) that a backend overrides with a strong symbol, or that a macro replaces entirely via the `#ifndef` guard in `mcu.h`.

**Macro-or-function duality**:
The substitution mechanism where every `mcu.h` entry is wrapped in `#ifndef <name>`: the mcumap may replace it with a macro (direct register write, zero cost), otherwise the backend or a weak default must provide a function.

**Access tier**:
The allowed depth of the platform abstraction used by a backend, chosen bare-metal-first: direct register writes (preferred), vendor SDK drivers (when the hardware requires it), or Arduino/RTOS cores (only behind explicit `USE_ARDUINO_*_LIBRARY` / `ARDUINO_ARCH_*` / `TARGET_*` flags). The chosen tier is documented in the backend README.

**Contract change**:
A modification of the MCU HAL surface itself (the `mcu.h`/`mcu.c` interface), as opposed to a backend-local implementation detail. Contract changes require an ADR and follow the extension protocol (guard + prototype + weak default shipped together).

**Extension protocol**:
The rule for growing the MCU HAL surface: every new `mcu.h` entry ships with its `#ifndef` guard, its prototype, and a weak default in `mcu.c` in the same commit. Backend-private helpers (e.g. `mcu_config_output_af` on STM32F1) live outside mcu.h and are not part of the protocol.

**Expected-behavior contract**:
The principle governing timed/ISR behavior: there is no absolute way to design internal timers; any mechanism (compare channels, ARR-halving, accumulator at `ITP_SAMPLE_RATE`, alarm pools, buffered IO as in ESP8266, virtual ticks) is acceptable as long as the observable behavior is reproduced — a step pulse with roughly 50% duty per event pair, a monotonic 1 ms tick feeding `mcu_rtc_cb`, and `mcu_dotasks()` never called from the RTC path.

**Primary stream**:
The port that carries the main Grbl protocol on a given board. Chosen by preference order when several exist: UART first, then USB, then WiFi/ethernet, then Bluetooth/other; the multistream layer (`grbl_stream.c`) registers all others. The backend README must state which stream is primary per supported board.

**Custom shift provider**:
An optional backend capability that replaces the default soft-shift of the IC74HC595 extender with dedicated hardware: the mcumap defines `IC74HC595_CUSTOM_SHIFT_IO` and the backend provides a `shift_register_io_pins()` callback (PIO state machines on RP2040/2350, I2S+DMA on ESP32).
