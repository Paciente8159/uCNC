<p align="center">
<img src="https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true">
</p>

# µCNC

µCNC - Universal CNC firmware for microcontrollers

# Building µCNC

To configure µCNC to fit your hardware you can use [µCNC config builder web tool](https://paciente8159.github.io/uCNC-config-builder/) to generate the config override files.
Although most of the options are configurable via the web tool, some options might be missing and you might need to add them manually (regarding tools or addon modules mostly).

# Useful Links

Some useful links with detailed information about µCNC
_**Jump to section**_
* [µCNC Wiki](https://github.com/Paciente8159/uCNC/wiki)
* [µCNC config files](https://github.com/Paciente8159/uCNC/tree/master/uCNC/README.md)
* [µCNC custom modules and events system](https://github.com/Paciente8159/uCNC/blob/master/uCNC/src/README.md)
* [µCNC kinematics HAL](https://github.com/Paciente8159/uCNC/blob/master/uCNC/src/hal/kinematics/README.md)
* [µCNC MCU HAL](https://github.com/Paciente8159/uCNC/blob/master/uCNC/src/hal/mcus/README.md)
* [µCNC tool HAL](https://github.com/Paciente8159/uCNC/tree/master/uCNC/src/hal/tools/README.md)
	* [µCNC existing tools configuration](https://github.com/Paciente8159/uCNC/blob/master/uCNC/src/hal/tools/tools/README.md)
* [µCNC default pinouts](https://github.com/Paciente8159/uCNC/blob/master/PINOUTS.md)


# VERSION 1.15+ NOTES

Version 1.15 introduces the following changes:
  - added new embroidery tool mode and new embroidery tool based on a stepper motor to control the needle. This tool mode is able to run the needle motor and the axis with different speed profiles to target specific motion needs of this type of tool.
  - added new encoder module enhancements with support for I2C and SSI encoders 
  - new hooks/callbacks to allow the creation and usage of custom ATC (automatic tool change) modules
  - added new planner event to allow last minute modifications to motions blocks being sent to the step generator


# IMPORTANT NOTE

The default behavior for µCNC will be as described:

  - All control inputs (Emergency stop, Safety door, Hold, Cycle start-resume), limit switches and probe input are held high by the MCU internal weak pull ups and will be active if left unconnected. If Emergency Stop is active the board will remain in Alarm mode and will not allow you to do much (besides querying the current board status via `?`). There are a few ways you can reconfigure µCNC to enable normal operation [here](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#%C2%B5cnc-wiring)
	- On first time flashing the board, or if the settings structure is modified, the board will be (and remain) in Alarm mode until a full settings reset is performed (`$RST=*`). This forces the user to acknowledge that the settings were modified and that the current parameters are known. This is a safety feature that prevents users to start using the machine with the default values of the settings accidentally (for example if using the SD card as a settings storage source and some sort of reading error occurs). You can revert this behavior to the Grbl default by enabling `DISABLE_SAFE_SETTINGS` option.

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

You can expand µCNC using via modules. The available modules are at the [µCNC-modules repository](https://github.com/Paciente8159/uCNC-modules).

A new [µCNC configuration webapp tool](https://github.com/Paciente8159/uCNC-webconfig) is now available (still in test mode) and will support version 1.12+ and allows you to generate/store your project configuration.
For previous versions, the [old configuration webapp tool](https://paciente8159.github.io/uCNC-config-builder/) will continue to remain avaliable.

You can also reach me at µCNC discord channel

[![µCNC discord channel](https://github.com/Paciente8159/uCNC/blob/master/docs/discord-logo-blue.png)](https://discord.gg/KdtKq9THN9)

## Supporting the project

µCNC is a completely free software. It took me a considerable amount of hours and effort to develop and debug so any help is appreciated. Building docs, testing and debugging, whatever. Also if you really like it and want help me keep the project running, you can help me to buy more equipment. Recently I have saved some extra money and bought a laser engraver. This hardware was fundamental to develop and testing version 1.2.0 and beyond. Currently this machine is being used to work on other projects and is running µCNC smoothly. Or you may just want to simply buy me a coffee or two for those extra long nights putting out code ;-)

[![paypal](https://www.paypalobjects.com/webstatic/en_US/i/buttons/PP_logo_h_100x26.png)](https://www.paypal.me/paciente8159)

## Current µCNC status

µCNC current major version is v1.15. You can check all the new features, changes and bug fixes in the [CHANGELOG](https://github.com/Paciente8159/uCNC/blob/master/CHANGELOG.md).

Version 1.15 added the following new major features.
  - added new embroidery tool mode and new embroidery tool based on a stepper motor to control the needle. This tool mode is able to run the needle motor and the axis with different speed profiles to target specific motion needs of this type of tool.
  - added new encoder module enhancements with support for I2C and SSI encoders 
  - new hooks/callbacks to allow the creation and usage of custom ATC (automatic tool change) modules. A new ATC module will also be made available.
  - added new planner event to allow last minute modifications to motions blocks being sent to the step generator. 

Version 1.14 added the following new major features.
  - new atomic primitives (similar to C11) including atomic CAS and basic semaphores.
  - ring buffer non-blocking methods.
  - new Grbl compatibility level (level 3) that is even less restrctive. This should improve Grbl GUI initial connection to the controller since this does not block as much. The Emergency Stop input behavior is also modified (change state only) and does not lock out if active.
  - new embroidary tool type support.
  - improved support for encoders (still in development)

Version 1.13 added the following new major features.
  - added initial suport for ESP32-S3 and ESP-C3 variants. This is support is not yet complete, as some features are missing (like BLE support). Other features like I2S support for custom IO shifters are still under test.

Version 1.12 added the following new major features.
  - added suport for STM32H7 single core MCU family. This is support is not yet complete, as some features are missing (like analog inputs, DMA support for SPI and EEPROM emulation)
	- complete code refactoring for ESP8266 core allowing to amke use of the new shift register to expand IO capabilities on this MCU (output and input) via SPI
	- new shift register that now also supports 74HC165 (along with the 74HC595 previously integrated)
	- slight refactoring of IO HAL to support the new shift register
	- re-introduction of boards friendly names for configuration in Arduino IDE and makefiles

Version 1.11 added the following new major features.
- self implemented subset/custom of stdio printf helpers. It's now possible to print formated messages via protocol. No need to do specific calls to print variables like numbers, strings, char, ip addresse, bytes, etc...
- improvements to the debug message system. Now a single call to DBGMSG macro is used. The same principle of formated messages is applied.
- Debug messages now have an intermediate buffer that stores the output before printing it to prevent outputing debug messages in the middle of onging protocol messages
- Parsing support for [O-Codes](https://linuxcnc.org/docs/html/gcode/o-code.html) (subrotines). These O-Codes can be executed from .nc files in the root dir of a pre-configured file system (either C for MCU flash or D for SD cards)

Version 1.10 added the following new major features.
- added support SPI bulk transfers. This improves SPI transmission speeds while keeping the whole firmware responsive, opening the door for new modules and upgrades to µCNC using SPI driven hardware like TFT displays.
- increased general purpose IO pins from 32 outputs/32 inputs to 50 outputs/50 inputs
- added support for a second SPI port

Version 1.9 added the following new major features.

- added support for STM32F0 MCU and an initial Bluepill example board.
- new File System module. This new file system module acts like a C wrapper for accessing both Flash memory files and external memories (like SD cards), and abstracts the underlaying file systems used (LittleFS, SPIFFS, FatFs, etc...). It also integrates the File System with other modules such as System Menu and Endpoints to allow quicker and transversal development of features accross different MCU
- new generic ring buffer in utils.h. This adds flexibility to make use of generic ring buffer implementations (via macros or pure C implementation with functions, or using custom SDK implementations for a particular architecture/use case). One example is to adapt generic ring buffer access to a multicore MCU.
- new multicore mode for RP2040, using generic ring buffer implementation supported no multicore queue. This allows running all µCNC communications tasks (USB, UART, WIFI, etc...) in one core, while the other core is dedicated to parsing and executing GCode commands.
- Suport for RS274NGC [expressions](https://linuxcnc.org/docs/html/gcode/overview.html#gcode:expressions) and [numbered parameters](https://linuxcnc.org/docs/html/gcode/overview.html#sub:numbered-parameters).

Version 1.8 added the following new major features.

- new IO HAL that simplifies io control and calls.
- added support for motion control/planner hijack. This allows to stash and restore all current buffered motions to allow execution of a completly new set of intermediate motions.
- added realtime modification of step and dir bits to be executed in the fly.
- added new tool for plasma THC.
- all analog inputs were modified from 8bit resolution to 10bit.
- complete redesign of PID module and modified tools functions to make use of PID update loop.
- complete redesign of serial communications to support and deal with multi-stream/origins.
- complete redesign of multi-stepper axis and self-squaring axis.
- initial support for Scara kinematics
- endpoint interface module to allow development of web services and REST modules for WiFi (available on v1.8.1)
- websocket interface module to allow development of web sockets modules for WiFi (available on v1.8.7)

Version 1.7 added a new major feature.

- added system menus module that allows to manage and render user menus in any type of display.

Version 1.6 added a couple of new features.

- added support for RP2040 MCU.
- moved tinyUSB to an external project allowing easier update and integration with both PIO and Arduino IDE.

Version 1.5 added a couple of new features.

- added support for ESP8266 MCU and the WeMos D1 boards.
- added support for ESP32 MCU's and the WeMos D1 R32 boards.
- added support for LPC176x MCU and the Re-Arm boards.
- full revision of the modular extension system based on events, delegates and listeners. All events share the same function declaration format. It's now easier to create add new events and handlers to the core code.

Version 1.4 added the following new features.

- added support for STM32F4 MCU and the Blackpill boards.
- new servo PIN type that generates a 50Hz with TOn (on phase) - 1~2ms (width) needed to control servo type motors.
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
List of Supported G-Codes since µCNC 1.9.2:
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
	- User(read/write) and parser(read only) parameters like #5221 (#5221 returns the G38 result for X axis)
	- Expressions like [1 + acos[0] - [#3 ** [4.0/2]]]
	- O-Codes (must run from files)

  - Outside the RS274NGC scope
    - Bilinear surface mapping: G39,G39.1,G39.2*
    - Servo Control: M10*
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
- _G39,G39.1,G39.2 only active if Height Map enabled_

Other G/M codes available via [external modules](https://github.com/Paciente8159/uCNC-modules)
  - Cubic and quadratic splines: G5/G5.1
  - Lathe radius mode: G7/G8
  - Spindle synchronized motion: G33
  - Stepper enable/disable: M17/M18
  - General Pin Control: M42
  - Enable/disable digital output pin synched/immediately: M62/M63/M64/M65
  - Enable/disable analog output pin synched/immediately: M67/M68
  - Enable/disable a digital output that controls the PSU: M80/M81
  - Smoothieware laser clustering mode modified gcode
  - Support for small LCD crystal displays with I2C interface
  - Support for monochromatic 128x64 displays (like Reprap fullgraphic discount)
  - Mobile web pendant via Wifi
  - BL touch module
  - SD card support using SPI
  - Wait for digital/analog input: M66
  - Set home position from current position: G28.1/G30.1
  - Play tone via PWM pin: M300
  - Trinamic driver support and config commands: M350* (set/get microsteps), M906* (set/get current), 913* (stealthchop threshold), 914* (stall sensitivity-stallGuard capable chips only), 920* (set/get register)

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
  - 50* generic digital inputs
  - 50* generic digital outputs
  - 6 servo control outputs (50Hz-PPM)

* see notes

```

NOTES:

- _6 steppers + 2 extra that can be configured to mirror 2 of the other 6 for dual drive axis_
- _6 limit switch (one per axis) plus 3 optional second axis X, Y or Z support dual endstops_
- _Generic inputs support interrupts on the first 8 pins. Prior to version 1.4 the number of generic inputs was limited to 16. Prior to version 1.10 the number of generic inputs was limited to 32._
- _Prior to version 1.4 the number of generic outputs was limited to 16. Prior to version 1.8 the number of generic outputs was limited to 32._

µCNC with a configuration similar to Grbl is be able to keep up to 30KHz step rate for a 3 axis machine on an Arduino Uno at 16Mhz. (the stated rate depends on the length of the segments too, since many short length segments don't allow full speed to be achieved). For this specific type of use (like in laser engraving) a 16-bit version of stepping algorithm is possible pushing the theoretical step rate limit to 40KHz on a single UNO board.

### µCNC current supported hardware

µCNC initial development was done both around Arduino UNO board just like GRBL. But µCNC can also be installed in other AVR boards like Arduino Mega (for Ramps), or similar boards (like Rambo). Other MCU's have and will be integrated in µCNC:

I used several UNO emulators but debugging was not easy. So a kind of virtual board (Windows PC) was created to test µCNC core code independently.
It can run on:

- AVR (Arduino UNO/MEGA)
- STM32F1 (like the Bluepill) - v1.1.x
- SAMD21 (Arduino Zero/M0) - v1.3.x
- STM32F4 (like the Blackpill) - v1.4.x (Does not emulate EEPROM)
- ESP8266 - v1.5.x/v1.12.x (supports wifi connection via telnet, lacks analog and input isr)
- ESP32 - v1.5.x (supports wifi connection via telnet and bluetooth)
- ESP32-S3 - v1.13.x (initial support, lacks BLE)
- ESP32-C3 - v1.13.x (initial support, lacks BLE)
- NXP LPC1768/9 - v1.5.x (eeprom emulation and analog still being developed)
- RP2040 - v1.6.x (supports wifi connection via telnet and bluetooth)
- RP2040 - v1.9.x (added multicore mode)
- STM32F0 (like the Bluepill) - v1.9.x
- RP2350 - v1.11.x (initial support)
- STM32H7 (single core) - v1.12.x (Still missing some features)
- Windows PC (used for simulation/debugging only - ISR on Windows doesn't allow to use it as a real alternative)

### µCNC current supported kinematics

µCNC is designed to be support both linear and non-linear kinematics and can be extended to support other types of kinematics.
Currently µCNC supports the following kinematics:

- Cartesian
- CoreXY
- Linear delta robot
- Rotary delta robot
- Scara

### µCNC roadmap

A couple of changes were introduced with version 1.2.0 of µCNC to prepare for future and easier expansions.
These changes are:

- Add some extra functionalities like tool speed encoding, positional encoders, etc...
- Change the files path structure to be more organic and well organized
- Move all USB stack related functionalities to a third-party library called [tinyUSB](https://github.com/hathach/tinyusb), opening the possibilities and speed development for new MCU's

Future versions are in plan for:

- Add support for Web interface
- Add more GCode features and hardware modules
- Add additional kinematics
- Add HAL for new MCU

### Building µCNC

For building µCNC go ahead to the [makefiles](https://github.com/Paciente8159/uCNC/blob/master/makefiles) folder of the target MCU and follow the instructions specific to your device.
Version 1.3.0 restructured the project so that it can easily be opened, configured, compiled and loaded via Arduino IDE environment. Just go to the [uCNC](https://github.com/Paciente8159/uCNC/blob/master/uCNC) folder and open uCNC.ino. See how to build the project for your board in the [wiki](https://github.com/Paciente8159/uCNC/wiki).
