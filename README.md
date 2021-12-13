<p align="center">
<img src="https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true">
</p>


# µCNC
µCNC - Universal CNC firmware for microcontrollers

## About µCNC
Heavily inspired by the by [Grbl](https://github.com/gnea/grbl) and [LinuxCNC](http://linuxcnc.org/), µCNC started to take shape in the second half of 2019 in an attempt to come out with a G-Code interpreter/controller software/firmware that is both compact and powerful as Grbl and flexible and modular as LinuxCNC, with the following goals in mind:

1. µCNC is written in C (GNU99 compliant). This gives the advantage of being supported for a large number of CPU/MCU and compilers available.
2. Modular library based:
   - Independent hardware. All MCU/hardware operations are written in a single translation unit that acts like a standardized HAL interface, leaving the CNC controlling code independent of the MCU architecture has long has it has the necessary abilities to execute code and respond to interrupts in a predictable. Porting µCNC for a different MCU should be fairly straight forward.
   - Independent kinematics. Another dimension of the HAL is the possibility of defining how the translation between machine coordinates and the motions is translated back and forth. This should theoretically allow µCNC to be easily adaptable to several types of machines like cartesian, coreXY, deltas and others. µCNC supports up to 6 axis.
   - Has of version 1.2.0 the addition of a HAL config that allow the user to build link inputs and outputs of the controller to specific functions or modules (like using a generic input has an encoder input or a PWM output has a servo controller with a PID module)
3. Compatible with already existing tools and software for Grbl. There is no point in trying to reinvent the wheel (the hole wheel at least :-P). For that reason µCNC (tries) to use the exact same protocol has Grbl. This allows it to easily integrate with Grbl ecosystem.

## Supporting the project
µCNC is a completely free software. It took me a considerable amount of hours and effort to develop and debug so any help is appreciated. Building docs, testing and debugging, whatever. Also if you really like it and want help me keep the project running, you can help me to buy more equipment. Recently I have saved some extra money and bought a laser engraver. This hardware was fundamental to develop and testing version 1.2.0. Currently this machine is being used to work on other projects and is running µCNC smoothly. Or you may just want to simply buy me a coffee or two for those extra long nights putting out code ;-)

[![paypal](https://www.paypalobjects.com/webstatic/en_US/i/buttons/PP_logo_h_100x26.png)](https://www.paypal.me/paciente8159)

## Current µCNC status
µCNC current major version is v1.3. You can check all the new features, changes and bug fixes in the [CHANGELOG](https://github.com/Paciente8159/uCNC/blob/master/CHANGELOG.md).
Version 1.3 added support for SAMD21 MCU and the Arduino Zero/M0 boards.
Version 1.2 added lot of new features needed for the future hardware/features support and some important bug fixes.
These include:

  - the new HAL configuration file that introduces a more flexible way to modify the HAL and give customization power of LinuxCNC.
  - the addition off new PID and encoder modules to be used by the new HAL config, powered by an internal RTC clock.
  - integration [tinyUSB](https://github.com/hathach/tinyusb), a complete USB stack frame that simplifies the creation of HAL code for new MCU.
  - the addition of an option for a 16bit version of the bresenham line algorithm that can improve step rate for weak 8bit processors or for specific applications like laser engraving.

### G-Codes support
µCNC v1.2.1 added additional Gcode support.
µCNC for now supports most of the RS274NGC v3:

```
List of Supported G-Codes since µCNC 1.2.1:
  - Non-Modal Commands: G4, G10*, G28, G30, G53, G92, G92.1, G92.2, G92.3
  - Motion Modes: G0, G1, G2, G3, G38.2, G38.3, G38.4, G38.5, G80
  - Feed Rate Modes: G93, G94
  - Unit Modes: G20, G21
  - Distance Modes: G90, G91
  - Plane Select Modes: G17, G18, G19
  - Tool Length Offset Modes: G43.1 G49
  - Cutter Compensation Modes: G40
  - Coordinate System Modes: G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3
  - Control Modes: G61, G61.1, G64
  - Program Flow: M0, M1, M2, M30(same has M2), M60(same has M0)
  - Coolant Control: M7, M8, M9
  - Spindle Control: M3, M4, M5
  - Valid Non-Command Words: A, B, C, F, I, J, K, L, N, P, R, S, T, X, Y, Z
  - Valid Non-Command Words: E (used by 3D printing firmwares like [Marlin](https://github.com/MarlinFirmware/Marlin)) (currently not used)

  _* also G10 L2 P28 and P30 to set homing coordinates_
  _* also G10 L2 P0 to set the current coordinates system offset_
```

TODO List of G-Codes in µCNC future releases:
  - extending the capabilities and functions of the new HAL config

### µCNC capabilities
µCNC currently supports up to (depending on the MCU/board capabilities):
  - 6 independent axis 
  - 8* stepper step/dir drivers (6 steppers + 2 extra that can be configured to mirror 2 of the other 6 for dual drive axis)
  - 9* limit switches (6 limit switch (one per axis) plus 3 optional second axis X, Y or Z support dual endstops) (interrupt driven)
  - 1 probe switch (interrupt driven)
  - 1 feed hold input (interrupt driven)
  - 1 cycle start/resume input (interrupt driven)
  - 1 emergency stop (interrupt driven)
  - 1 door open switch (interrupt driven)
  - 16 pwm outputs
  - 16 analog inputs
  - 16 generic digital inputs
  - 16 generic digital outputs

µCNC with a configuration similar to Grbl is be able to keep up to 30KHz step rate for a 3 axis machine on an Arduino Uno at 16Mhz. (the stated rate depends on the length of the segments too, since many short length segments don't allow full speed to be achieved). For this specific type of use (like in laser engraving) a 16bit version of stepping algorithm is possible pushing the theoretical step rate limit to 40KHz on a single UNO board.

### Current µCNC supported hardware
µCNC initial development was done both around Arduino UNO board just like GRBL. But µCNC can also be installed in other AVR boards like Arduino Mega (for Ramps), or similar boards (like Rambo). With v1.1.0 STM32F10x was added.

I used several UNO emulators but debugging was not easy. So a kind of virtual board (Windows PC) was created to test µCNC core code independently.
It can run on:
  - Arduino UNO
  - Arduino MEGA
  - STM32F1 Blue Pill
  - Windows PC (used for simulation only - ISR on Windows doesn't allow to use it a real alternative)

### µCNC roadmap
A couple were introduced with version 1.2.0 of µCNC to prepare for future and easier expansions.
These changes are:
  - Add some extra functionalities like tool speed encoding, positional encoders, etc...
  - Change the files path structure to be more organic and well organized
  - Move all USB stack related functionalities to a third-party library called [tinyUSB](https://github.com/hathach/tinyusb), opening the possibilities and speed development for new MCU's

Future versions are in plan for:
  - Possibly integrate µCNC with Arduino IDE across all platforms (in study)
  - Add more hardware configurations (SAMD21 development has started)

### Building µCNC
For building µCNC go ahead to the [makefiles](https://github.com/Paciente8159/uCNC/blob/master/makefiles) folder of the target MCU and follow the instructions specific to your device.
