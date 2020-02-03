![GitHub Logo](https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true)

# µCNC
µCNC - A universal CNC firmware for microcontrollers

# Changelog

## 1.0.0-alpha.3 - 2020-02-01

### Added
  - Initial versioning system (Semantic Versioning)
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
