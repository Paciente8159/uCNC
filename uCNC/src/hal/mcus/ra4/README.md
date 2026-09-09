# Renesas RA4M1 µCNC Backend

## 1. Target chips and tested boards

| Chip | Package | Flash | SRAM | Status |
|------|---------|-------|------|--------|
| R7FA4M1AB3CFP | 100-LQFP | 256 KB | 32 KB | Implemented |
| R7FA4M1AB3CFM | 64-LQFP | 256 KB | 32 KB | Compile-tested |
| R7FA4M1AB3CFL | 48-LQFP | 256 KB | 32 KB | Untested |
| R7FA4M1AB3CNF | 40-QFN | 256 KB | 32 KB | Untested |

### Boards

| Board | Status |
|-------|--------|
| Arduino UNO R4 Minima | Implemented |

### Capability matrix

| Feature | Status |
|---------|--------|
| GPIO (input/output) | Implemented |
| GPIO pull-up | Implemented |
| Step generation (ITP via GPT320) | Implemented |
| RTC 1ms tick (SysTick) | Implemented |
| UART (SCI) | Implemented |
| Servo timer | Not yet implemented |
| Oneshot timer | Implemented (GPT163) |
| PWM (GPT compare) | Implemented |
| Analog input (ADC14) | Not yet implemented |
| USB-CDC | Not yet implemented |
| Flash NVM (data flash) | Not yet implemented |
| DMA | Not yet implemented |
| SPI | Not yet implemented |
| I2C | Not yet implemented |

### Vendor-source provenance

- RA4M1 Group User's Manual: Hardware Rev 1.10 (Sep 29, 2023)
- Renesas FSP documentation for register-level details

## 2. Toolchain and build recipes

### Prerequisites

- ARM GCC toolchain (`arm-none-eabi-gcc`)
- Renesas RA4M1 device header (CMSIS) OR the FSP (Flexible Software Package)
- The backend currently builds with direct register definitions (no SDK dependency for GPIO, timers, UART)

### Build via PlatformIO

```ini
; platformio.ini
[env:uno_r4]
platform = renesas-ra
board = uno_r4
framework = arduino
board_build.mcu = r7fa4m1ab
build_flags = -D MCU=MCU_RA4M1 -D BOARD=BOARD_UNO_R4
```

### Build via Makefile (bare-metal)

```makefile
MCU      = r7fa4m1ab
TARGET   = ucnc_uno_r4
F_CPU    = 48000000
CFLAGS  += -D MCU=MCU_RA4M1 -D BOARD=BOARD_UNO_R4
CFLAGS  += -D F_CPU=$(F_CPU)UL
```

### Access tier

This backend targets tier 1 (direct register access) for GPIO, timers and SCI, with no RTOS or framework dependency for basic operation. The CDC-ACM (USB) and analog (ADC14) implementations will require SDK headers or direct register definitions from the RA4M1 hardware manual.

## 3. Resource allocation

| Resource | Peripheral | IRQ | Priority |
|----------|-----------|-----|----------|
| ITP (step generation) | GPT320 (32-bit) | GPT0_CCMPA / GPT0_CCMPB | 5 |
| RTC (1ms tick) | SysTick | SysTick_Handler | 8 |
| SERVO (50Hz) | GPT162 (16-bit) | GPT2_CCMPA | 6 |
| ONESHOT (timeout) | GPT163 (16-bit) | GPT3_CCMPA | 6 |
| UART (primary) | SCI0 (UART_PORT) | SCI0_RXI / SCI0_TXI | 4 |

### Interrupt priority ordering

Priority order (lower number = higher priority):
1. Input-change (IRQ) - priority 1
2. SPI - priority 3
3. UART (SCI) - priority 4
4. ITP (step) - priority 5
5. Servo / Oneshot - priority 6
6. RTC (SysTick) - priority 8
7. USB - priority 10

## 4. Primary stream

| Board | Stream | Peripheral | Pins |
|-------|--------|-----------|------|
| Arduino UNO R4 | UART | SCI0 | P101 (TX), P100 (RX) |

## 5. NVM strategy

Data flash (8 KB at 0x40100000) is reserved for µCNC settings storage. Implementation using the FCU (Flash Control Unit) FACI commands is planned. For initial bringup, settings can be stored in RAM (volatile).

## 6. Known limitations

- GPIO interrupt-on-change (input ISR) not yet implemented — uses polling fallback
- ADC14 analog input not yet wired
- USB-CDC not yet implemented
- Flash NVM persistence not yet implemented
- DMA channels not yet allocated
- SPI and I2C not yet implemented
- PWM spindle output uses basic GPT compare, not fully parameterized
- Delay loop is approximate (not cycle-accurate for all F_CPU values)
- Clock init assumes HOCO at 48 MHz — external crystal (MOSC) not yet supported
- Only the 100-pin package variant has been considered; 48/64-pin packages will need pin validity checks

## 7. Generation

The DIO pin table and register address macros in `mcumap_ra4m1.h` are hand-written following the canonical µCNC numbering from `BACKEND_GUIDE.md`. A future generator pass should extract pin/port/peripheral data from the Renesas CMSIS header or FSP configuration tool output.

## 8. Resource validation

The mcumap validates:
- Board-defined pin combinations via `#if (defined(PORT) && defined(BIT))` guards
- Timer allocation uniqueness (ITP/SERVO/ONESHOT must use different GPT channels)
- UART availability (port TX/RX must match a supported SCI instance)

The following are **not yet validated** but planned:
- Pin mux conflicts (two peripherals mapped to the same pin)
- Timer channel collisions with PWM outputs
- Shared interrupt vectors