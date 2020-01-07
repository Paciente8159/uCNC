# uCNC
uCNC - A universal CNC firmware for microcontrollers

## Introduction
Heavily inspired by the by [Grbl](https://github.com/gnea/grbl) and [LinuxCNC](http://linuxcnc.org/), uCNC started to take shape in the second half of 2019 in an attempt to come out with a G-Code interpreter/controller software/firmware that is both compact and powerfull as Grbl and flexible and modular as LinuxCNC, with the following goals in mind:

1. Hardware independent (or as much as possible). uCNC is writen in plain C (GNU99 compatible). This gives the advantage of being supported for a large number of CPU/MCU and compilers available.
2. All direct hardware operations are defined by a single translation unit leaving the CNC controlling code independent of the CPU/MCU architecture has long has it has the necessary habilities to execute code in a predictable interrupt driven way. Porting uCNC for a different MCU should be fairly straigt forward.
3. Independent and flexible kinematics. This should theoreticly allow for uCNC to be adaptable several types of machines like cartesian, corexy, deltas, (lathes probably??). uCNC supports up to 6 axis.
4. Be compatible with already existing tools and software. There is no point in trying to reinvent the wheel. For that reason uCNC (tries) to use the exact same protocol has Grbl. This allows it to easilly integrate with Grbl ecosystem.

## Current uCNC status
uCNC is still in it's very early stages so it should be only used for test purposes for now.
For production stages [Grbl](https://github.com/gnea/grbl) or other G-Codes interpreters/controllers should be used.

### G-Codes support
uCNC for now supports most of the RS274NGC v3:

```
List of Supported G-Codes in uCNC 0.01:
  - Non-Modal Commands: G4, G10, G28, G30, G53, G92, G92.1, G92.2, G92.3
  - Motion Modes: G0, G1, G2, G3, G80
  - Feed Rate Modes: G94
  - Unit Modes: G20, G21
  - Distance Modes: G90, G91
  - Plane Select Modes: G17, G18, G19
  - Tool Length Offset Modes: G49
  - Cutter Compensation Modes: G40
  - Coordinate System Modes: G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3
  - Control Modes: G61
  - Program Flow: M2, M30(same has M2)
  - Coolant Control: M7 (same has M8), M8, M9
  - Spindle Control: M3, M4, M5
  - Valid Non-Command Words: A, B, C, F, I, J, K, L, N, P, R, S, T, X, Y, Z
```

TODO List of G-Codes in uCNC future releases:
  - Motion Modes: G38.x (probing)
  - Feed Rate Modes: G93
  - Tool Length Offset Modes: G43
  - Program Flow: M0, M1
  - Valid Non-Command Words: E (and a subset of G-Code commands used by 3D printing firmwares like [Marlin](https://github.com/MarlinFirmware/Marlin))

### uCNC capabilities
uCNC currently supports up to:
  - 6 independent axis
  - 6 stepper step/dir drivers
  - 6 limit switches (one per axis)
  - 1 probe switch
  - 1 feed hold input
  - 1 cycle start/resume input
  - 1 emergency stop/door open switch
  - 4 pwm outputs
  - 4 analog inputs
  - 16 digital inputs
  - 16 digital outputs

### Current uCNC supported hardware
uCNC initial development was done both around Arduino UNO capabilities like GRBL.
It can run on:
  - Arduino UNO
  - Windows PC (used for simulation only - ISR on Windows doesn't allow to use it a real alternative)

In the future uCNC will most probably be extended to:
  - Microchip PIC18F
  - ARM
  - Old PC with a RT OS???
  - Other??
