![GitHub Logo](https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true)

# µCNC
µCNC - A universal CNC firmware for microcontrollers

# Changelog

## [1.0.0-beta.2] - 2020-02-09

### Added
  - added AVR support by adding external interrupt pins to the limits and control pins range
  - new dual drive axis support with self squaring homing motion and locking of one stepper in the interpolator stage with the addition of two mirror step pins (with minimal performance loss)
  - added initial laser mode support
  - new filter to limits ISR to support dual drive auto squaring with IO overlapping (shared limit switches)

### Changed
  - complete HAL redesigned to allow independent function pins
  - new generic AVR HAL implementation
  - modified system exec_flags to prevent unecessary locks
  - modified cartesian kinematics (configurable) to fit several types of machines (without the need to code a full kinematics)
  - separeted the mcu and board now have individual configuration files
  - updated README.md to reflect changes of beta.2

### Fixed
  - fixed bug introduced in the beta patch to the homing motion (no limit_flag)
  - minor modification of mc_arc to prevent error on non existant axis
  - adapted protocol to send a minimum of 3 axis coordinates (yeld error on interface with OpenCNCPilot and probably other software too)
  - fixed probe enable and disable functions (now use direct IO on AVR)

## [1.0.0-beta] - 2020-02-04

### Added
  - added initial TLO (tool length offset)
  - added support for new motion modes (G61.1 and G64)
  - added initial laser mode support
  - added CHANGELOG.md

### Changed
  - modified parser coordinate calculation and non modal gcodes handling so the remaining code executes as it should after these commands
  - redesined parameters now loaded directly from eeprom and reduced ram usage (about 10% on Arduino Uno)
  - planner now holds up to 15 motions (stable no memory overflow)
  - redesigned spindle control to modify pwm according to motion (used in laser mode)
  - simplified step ISR functions (faster execution times - under 23ms for 3 stepper motors)
  - cleaned cnc_doevents() and some realtime commands functions handling

### Fixed
  - fixed settings version compare operation that caused eeprom reset at power up

## 1.0.0-alpha.3 - 2020-02-01

### Added
  - initial versioning system (Semantic Versioning)
  - can now process custom messages on comments and echo them (as explained in RS274NGC)
  - added dummy user transformation funtions to kinematics
  - motion control probing (G38.x commands)
  - new mcu erase eeprom function
  - added startup blocks functionalities
  - new build version info file
  - added state "Check" to status report
  - spindle delays after start/stop on hold state and motion exact stop mode on spindle speed change

### Changed
  - redesigned the realtime commands response system
  - several modifications to serial functions (new ISR read char, new EOL char and overflow error detection mechanics). Also char to upper and other char processing stages moved from ISR to make it more slim
  - more optimizations on the serial ISR functions
  - interpolator blocks are no longer monitored (they are overwritten on the fly when computing new planner block)
  - changed mc_dwell resolution to 0.1s (less interrupts on the stepper ISR)
  - redesigned settings with crc checksum on all cnc and parser parameters
  - redesigned command echo with char indication parser error
  - added several escapes in loops with cnc_doevent() (avoid locks and exit loops on system abort)
  - interpolator buffer managment optimizations
  - new strategy to handle startup blocks by serial read functions redirection
  - small planner buffer optimizations
  - several small optimizations regarding volatile variables

### Fixed
  - parser fixed by repositioning the commands discard function from the main loop to the parser internals and running only when needed (this would cause elimination of a hole line of unprocessed g-code after a line with error had been completly read by parser)
  - arc motion control generator (G2, G3 commands) had several errors caused by bad ABS macro math function definition (utils.h) and the incremental error compensation stage
  - planner forces motions with dwells to start with zero speed (so stop motion previous to dwell is executed)
  - improved stability that randomly caused locks
  - fixed serial extended ascii problem
  - fixed bug on feed overrides that had no effect if a single line was on the interpolator/planner buffer
  - fixed spindle pwm (MAX speed set output to 0)
  - fixed planner acceleration (error on angle factor calculation between vectors)
  - fixed gcode lines parsing error on Hold (they would return error)
  - fixed VERY STUPID bug that caused no error on virtual mcu mcu but made avr mcu eeprom function not work properly
  - fixed EXEC_FLAGS to fix locked state behaviour

## 1.0.0-alpha.2 - 2020-01-25

### Added
  - initial versioning system (Semantic Versioning)

### Changed
  - more modifications to the mcu functions file for AVR/atmega family devices

## 0.01a - (this should be 1.0.0-alpha) - 2020-01-25

### Added
  - initial mcumap for Arduino Mega - Rambo board

### Changed
  - modified mcumap for generic pins
  - modified mcu functions file for AVR to be more compatible with the atmega family devices

### Fixed
  - coolant/Mist turn on/off

## 0.01 - Pre-release version - 2020-01-23

### Initial release

[1.0.0-beta.2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.0.0-beta.2
[1.0.0-beta]: https://github.com/Paciente8159/uCNC/releases/tag/v1.0.0-beta
