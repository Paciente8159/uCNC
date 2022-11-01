<p align="center">
<img src="https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true">
</p>

# µCNC

µCNC - Universal CNC firmware for microcontrollers

# IMPORTANT NOTE

By default and as a safety measure µCNC control inputs (Emergency stop, Safety door, Hold, Cycle start-resume), as well as limit switches and probe, are held high by the microcontroller input weak-pull up resistors. If left unconnected or connected to normally opened switches these inputs will be in an active/triggered state and the controller may lock all motions.
There are a few ways you can reconfigure µCNC to enable normal operation [here](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#%C2%B5cnc-wiring).

Also beware that the main branch contain the latest changes, including bug fixes and some intermediate changes. Some of theses changes are very extent and can come with issues and although new PR are tested, they might have unexpected errors or behavior. If you find and issue please report. Also prefer the latest release version over the master branch.

## About µCNC

Heavily inspired by [Grbl](https://github.com/gnea/grbl) and [LinuxCNC](http://linuxcnc.org/), µCNC started to take shape in the second half of 2019, in an attempt to come out with a G-Code interpreter/controller software/firmware that is both compact and powerful as Grbl and flexible and modular as LinuxCNC, with the following goals in mind:

1. µCNC is written in C (GNU99 compliant). This gives the advantage of being supported for a large number of CPU/MCU and compilers available.
2. Modular library based:
   - Independent hardware. All MCU/hardware operations are written in a single translation unit that acts like a standardized HAL interface, leaving the CNC controlling code independent of the MCU architecture has long has it has the necessary abilities to execute code and respond to interrupts in a predictable away. Porting µCNC for a different MCU should be fairly straight forward.
   - Independent kinematics. Another dimension of the HAL is the possibility of defining how the translation between machine coordinates and the motions is translated back and forth. This should theoretically allow µCNC to be easily adaptable to several types of machines like cartesian, coreXY, deltas and others. µCNC supports up to 6 axis.
   - As of version 1.2.0 the addition of a HAL config that allow the user to build link inputs and outputs of the controller to specific functions or modules (like using a generic input has an encoder input or a PWM output has a servo controller with a PID module)
   - As of version 1.3.0 a new dimension to the HAL was added. The tool HAL. This allow to add multiple tools that can perform different task with µCNC.
   - As of version 1.4.0 a special kind of PIN type dedicated to servo motors control was added µCNC.
   - As of version 1.4.0 a new module extension system was introduced. It's now possible to add hooks in the core code and attach multiple listeners to execute additional code further expanding µCNC original capabilities. Modules can be enabled and disabled in the config file to enable feature on per need basis.
3. Compatible with already existing tools and software for Grbl. There is no point in trying to reinvent the wheel (the whole wheel at least :-P). For that reason µCNC uses protocol compatible with Grbl. This allows it to easily integrate with Grbl ecosystem.

You can navigate the [project wiki](https://github.com/Paciente8159/uCNC/wiki) to find out more on how to use it.
You can expand µCNC using via modules. The available modules are at the [µCNC-modules repository](https://github.com/Paciente8159/uCNC-modules)
You can now also use [µCNC config builder web tool](https://paciente8159.github.io/uCNC-config-builder/) to generate the files needed to adapt µCNC to your board.

## Supporting the project

µCNC is a completely free software. It took me a considerable amount of hours and effort to develop and debug so any help is appreciated. Building docs, testing and debugging, whatever. Also if you really like it and want help me keep the project running, you can help me to buy more equipment. Recently I have saved some extra money and bought a laser engraver. This hardware was fundamental to develop and testing version 1.2.0 and beyond. Currently this machine is being used to work on other projects and is running µCNC smoothly. Or you may just want to simply buy me a coffee or two for those extra long nights putting out code ;-)

[![paypal](https://www.paypalobjects.com/webstatic/en_US/i/buttons/PP_logo_h_100x26.png)](https://www.paypal.me/paciente8159)

## Current µCNC status

µCNC current major version is v1.5. You can check all the new features, changes and bug fixes in the [CHANGELOG](https://github.com/Paciente8159/uCNC/blob/master/CHANGELOG.md).
Version 1.5 added a couple of new features.

- added support for ESP8266 MCU and the WeMos D1 boards.
- added support for ESP32 MCU's and the WeMos D1 R32 boards.
- added support for LPC176x MCU and the Re-Arm boards.
- full revision of the modular extension system based on events, delegates and listeners. All events share the same function declaration format. It's now easier to create add new events and handlers to the core code.

Version 1.4 added the following new features.

- added support for STM32F4 MCU and the Blackpill boards.
- new servo PIN type that generates a 50Hz with TOn - 1~2ms needed to control servo type motors.
- support for delta kinematics.
- new modular extension system based on events, delegates and listeners. It's now possible to inject code anywhere inside the core code by creating and adding code hooks that can then call and execute multiple listeners
- added optional variable acceleration step generation (S-Curve speed profile)
- added Trinamic drivers basic support. For now only TMC drivers with UART are available. TMC2208 as been tested with success.
- added support for a subset of canned cycles (G8x), enabled via config file
- added module for BLtouch probe

Version 1.3 added the following new features.

- added support for SAMD21 MCU and the Arduino Zero/M0 boards.
- new HAL for tool change and management.
- modified file structure and modified tinyUSB source code files to allow compiling and loading the firmware with Arduino IDE.
- gcode parser is extendable via modules
- PID module initial implementation
- encoder/counter module implemented

Version 1.2 added lot of new features needed for the future hardware/features support and some important bug fixes.
These include:

- the new HAL configuration file that introduces a more flexible way to modify the HAL and give customization power of LinuxCNC.
- the addition off new PID and encoder modules to be used by the new HAL config, powered by an internal RTC clock.
- integration [tinyUSB](https://github.com/hathach/tinyusb), a complete USB stack frame that simplifies the creation of HAL code for new MCU.
- the addition of an option for a 16bit version of the bresenham line algorithm that can improve step rate for weak 8bit processors or for specific applications like laser engraving.

### G-Codes support

µCNC for now supports most of the RS274NGC v3:

```
List of Supported G-Codes since µCNC 1.3.0:
  - Non-Modal Commands: G4, G10*, G28, G30, G53, G92, G92.1, G92.2, G92.3
  - Motion Modes: G0, G1, G2, G3, G38.2, G38.3, G38.4, G38.5, G80, G81*, G82*, G83*, G85*, G86*, G89*
  - Feed Rate Modes: G93, G94
  - Unit Modes: G20, G21
  - Distance Modes: G90, G91
  - Plane Select Modes: G17, G18, G19
  - Tool Length Offset Modes: G43, G43.1*, G49
  - Cutter Compensation Modes: G40
  - Coordinate System Modes: G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3
  - Control Modes: G61, G61.1, G64
  - Program Flow: M0, M1, M2, M30(same has M2), M60(same has M0)
  - Coolant Control: M7, M8, M9
  - Spindle Control: M3, M4, M5
  - Tool Change: M6
  - Valid Non-Command Words: A, B, C, F, H, I, J, K, L, N, P, Q, R, S, T, X, Y, Z
  - Outside the RS274NGC scope
    - Servo Control: M10*
    - General Pin Control: M42*
    - Trinamic settings: M350* (set/get microsteps), M906* (set/get current), 913* (stealthchop threshold), 914* (stall sensitivity-stallGuard capable chips only), 920* (set/get register)
    - Digital pins/trimpot settings: M351* (set/get microsteps), M907* (set/get current via digipot)
	- Laser PPI M126*(mode) M127*(PPI) and M128*(Pulse width)
    - Valid Non-Command Words: E (used by 3D printing firmware like [Marlin](https://github.com/MarlinFirmware/Marlin)) (currently not used)

* see notes

```

NOTES:

- _also G10 L2 P28 and P30 to set homing coordinates_
- _also G10 L2 P0 to set the current coordinates system offset_
- _G59.1, G59.2 and G59.3 can be enabled in config_
- _G43.1 was kept to be back compatible with Grbl using Z word to set the offset_
- _M1 stop condition can be set in HAL file_
- _M6 additional tools can be defined in HAL file_
- _M10 only active if servo motors are configured_
- _M42 configurable via additional module. Provides a way to set any kind of digital output, PWM or Servo PIN_

**ALL custom G/M codes require at least ENABLE_PARSER_MODULES option enabled**

TODO List of G-Codes in µCNC future releases:

### µCNC capabilities

µCNC currently supports up to (depending on the MCU/board capabilities):

```
  - 6 independent axis
  - 8* stepper step/dir drivers
  - 9* limit switches (interrupt driven)
  - 1 probe switch (interrupt driven)
  - 1 feed hold input (interrupt driven)
  - 1 cycle start/resume input (interrupt driven)
  - 1 emergency stop (interrupt driven)
  - 1 door open switch (interrupt driven)
  - 16 pwm outputs
  - 16 analog inputs
  - 32* generic digital inputs
  - 32* generic digital outputs
  - 6 servo control outputs (50Hz-PPM)

* see notes

```

NOTES:

- _6 steppers + 2 extra that can be configured to mirror 2 of the other 6 for dual drive axis_
- _6 limit switch (one per axis) plus 3 optional second axis X, Y or Z support dual endstops_
- _Generic inputs support interrupts on the first 8 pins. Prior to version 1.4 the number of generic inputs was limited to 16._
- _Prior to version 1.4 the number of generic outputs was limited to 16._

µCNC with a configuration similar to Grbl is be able to keep up to 30KHz step rate for a 3 axis machine on an Arduino Uno at 16Mhz. (the stated rate depends on the length of the segments too, since many short length segments don't allow full speed to be achieved). For this specific type of use (like in laser engraving) a 16-bit version of stepping algorithm is possible pushing the theoretical step rate limit to 40KHz on a single UNO board.

### Current µCNC supported hardware

µCNC initial development was done both around Arduino UNO board just like GRBL. But µCNC can also be installed in other AVR boards like Arduino Mega (for Ramps), or similar boards (like Rambo). Other MCU's have and will be integrated in µCNC:

I used several UNO emulators but debugging was not easy. So a kind of virtual board (Windows PC) was created to test µCNC core code independently.
It can run on:

- AVR (Arduino UNO/MEGA)
- STM32F1 (Bluepill) - v1.1.x
- SAMD21 (Arduino Zero/M0) - v1.3.x
- STM32F4 (Blackpill) - v1.4.x (Does not emulate EEPROM)
- ESP8266 - v1.5.x (supports wifi connection via telnet, lacks analog and input isr)
- ESP32 - v1.5.x (supports wifi connection via telnet and bluetooth, lacks analog and input isr)
- NXP LPC1768 - v1.5.x (eeprom emulation and analog still being developed) 
- Windows PC (used for simulation/debugging only - ISR on Windows doesn't allow to use it as a real alternative)

### µCNC roadmap

A couple of changes were introduced with version 1.2.0 of µCNC to prepare for future and easier expansions.
These changes are:

- Add some extra functionalities like tool speed encoding, positional encoders, etc...
- Change the files path structure to be more organic and well organized
- Move all USB stack related functionalities to a third-party library called [tinyUSB](https://github.com/hathach/tinyusb), opening the possibilities and speed development for new MCU's

Future versions are in plan for:

- Add more MCU HAL (RP2040 may be implemented in a near future)
- Add support for graphical LCD
- Add more GCode features and hardware modules
- Add additional kinematics

### Building µCNC

For building µCNC go ahead to the [makefiles](https://github.com/Paciente8159/uCNC/blob/master/makefiles) folder of the target MCU and follow the instructions specific to your device.
Version 1.3.0 restructured the project so that it can easily be opened, configured, compiled and loaded via Arduino IDE environment. Just go to the [uCNC](https://github.com/Paciente8159/uCNC/blob/master/uCNC) folder and open uCNC.ino. See how to build the project for your board in the [wiki](https://github.com/Paciente8159/uCNC/wiki).
