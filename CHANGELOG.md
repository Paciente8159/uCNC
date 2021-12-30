![GitHub Logo](https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true)

# µCNC
µCNC - A universal CNC firmware for microcontrollers

# Changelog

## [1.3.rc2] - Unreleased

Version 1.3 is a major revision an targets add SAMD21 (Arduino Zero and M0) support.
It also adds the new tool HAL that allows to have multiple tools in the µCNC and support for gcode M6 command.
File structure as changed and tinyUSB was modified and integrated to allow compiling code using Arduino IDE.
RC2 adds/fixes the following issue:

### Added
  - implemented SAMD21 digital input ISR and io_inputs_isr (#89)
  - implemented STM32F1 analog input reading (#88)
  - implemented io_control analog reading function (#88)
  - implemented SAMD21 analog reading function (#90)
  - implemented io_inputs_isr on AVR and STM32 (#91)

### Changed
  - modified file structure and tinyUSB was modified and integrated to allow compiling code using Arduino IDE (#87)

## [1.3.rc] - 2021-12-26

Version 1.3 is a major revision an targets add SAMD21 (Arduino Zero and M0) support. It also adds the new tool HAL that allows to have multiple tools in the µCNC and support for gcode M6 command.
RC adds/fixes the following issue:

### Added
  - added the new tool HAL. This new HAL sacrifices a bit of step rate performance in favor of the possibility of being able to use multiple tools. (#77) (#80)
  - added extra delay for coolant on resume from any hold (hold, safety-door) to emulate Grbl (#76)
  - implemented Hardware UART in SAMD21 (#75)
  - real line number status (ignores N word and outputs the real line number of the command) (#81)

### Changed
  - modified some NVIC IRQ and global interrupt enable and disable inside ISR code in STM32 and SAMD21 (#75)
  - modified RTC to prevent reentrancy inside ISR code in STM32 and SAMD21 (#75)
  - modified planner buffer size in AVR size to prevent memory errors  (#77)
  - planner and interpolator block and segment buffer slots are cleaned before writting data to prevent errors from previous data. (#77)
  - step ISR optimizations (for main stepper and idle steppers) are now optional. (#77)
  - new AVR make file (#82)
  - modified/simplified parser G90/G91 (absolute/relative coordinates) (#83)
  - removed planner position tracking. Modified/simplified position tracking inside motion control. This is more memory effective and prevents desynchronization problems between the motion controller and the interpolator (#84)
  - moved scheduled code running from hardware implementation to cnc (common) compilation unit. Makes code (pid update or other) architecture agnostic. (#86)

### Fixed
  - fixed issue with active CS_RES input that caused resume condition (delay) without active hold present (#75)
  - fixed SAMD21 PWM frequency configuration (now is aprox 976Hz like AVR) (#78)
  - real line number (N word) processing was not being read (#81)
  - fixed welcome message not being sent after soft reset (#79)
  - fixed step generation ISR random problems (stop working). This was caused by problems in the segment buffer read write. Solved by adding atomic lock blocks to the code (#85)
  - G28 and G30 now perform in whatever coordinate mode before travelling home (RS274NGC compliant) (#83)

## [1.3.b2] - 2021-12-14

Version 1.3 is a major revision an targets add SAMD21 (Arduino Zero and M0) support.
Beta2 fixes the following issue:

### Added
  - added SAMD21 extra debug flags for GDB (#74)

### Changed
  - cnc spindle speed and resume delay bypass if set to 0 (#73)
  - cnc spindle at cnc stop has now moved to the interpolator unit (#74)

### Fixed
  - while executing real time commands with the CS_RES pin always active caused and infinite loop inside the tool delay (caused stack overflow error on SAMD21) (#73)
  - executing a soft reset caused unowned exception fault on SAMD21. Recoded alarm to prevent calling itself. This also repeats the emergency stop message while the ESTOP is pressed. (#74)
  - removed generic clock output code(used to debug the generic clock working). (#74)

## [1.3.b] - 2021-12-13

Version 1.3 is a major revision an targets add SAMD21 (Arduino Zero and M0) support.
The following things were changed:

### Added
  - added SAMD21 (Arduino Zero and M0 boards) initial support. This is still an early release so there are still limitations for SAMD21. These are:  (#72)
    - Interrupt driven inputs not implemented (only working via soft pooling)
    - Serial port COM not implemented. (limited to Virtual COM port via USB)
    - Analog inputs not implemented. (this feature is outside of the Grbl scope)
  - AVR and STM32 remain unchanged.

### Changed
  - modified setting $0=val to work the same way has Grbl (#71)

## [1.2.4] - 2021-12-10

Version 1.2.4 is a minor revision and improves a couple of functionalities.
The following things were changed:

### Added
  - new set of settings commands to control EEPROM/Flash storing (optional build setting ENABLE_SETTING_EXTRA_CMDS in cnc_config.h active by default) (#70)

    This set of new commands allow a more granular control over the settings stored in EEPROM/Flash to prevent wearing.
    When enabled all grbl $x=val are only changed in SRAM. To set them in non volatile memory a save command must be issued.
    3 additional commands are added:
    $SS - Settings save. This stores all values to EEPROM/Flash
    $SL - Settings load. This loads all values from EEPROM/Flash
    $SR - Settings reset. This loads all default values from ROM

### Changed
  - enabled Grbl startup emulation to improve µCNC compatibility. Many Grbl interfaces expect the grbl startup message and won't recognize µCNC because of that simple fact (#70)
  - changed tinyUSB config and descriptors file location and updated makefile for STM32  (#70)

## [1.2.3] - 2021-12-08

Version 1.2.3 is a minor revision and improves a couple of functionalities.
The following things were changed:

### Changed
  - added parser unlocking on soft-reset to unlock board after end program M codes M2 and M30 (#69)
  - option USE_COOLANT active by default (#69)

### Fixed
  - fixed typo that cause compilation error on option USE_COOLANT (#69)
  - fixed code compilation errors without SPINDLE_PWM defined (#69)

## [1.2.2] - 2021-08-08

Version 1.2.2 is a minor revision an targets a couple of issues on the STM32.
The following things were changed:

### Changed
  - minor modification to startup blocks calling. They are now executed as soon as they are called. On error the next block is not executed. (#67)
  - improved SMT32F1 EEPROM-Flash emulation. Now uses a single page that is copied to RAM before being stored. This also fixes EEPROM using on STM32F1 making it fully functional. HAL was readapted for mcu.h. On AVR added dummy function to support new HAL. (#66)

### Fixed
  - fixed flash eeprom reading that caused SMT32F1 to hang with USB virtual COM port. SMT32F1 default value for errased flash is 0xFF and not 0x00. This caused the startup blocks to read a sequence of 0xFF chars. This was fixed by filtering the accepted values to standard asccii only. Both serial versions of SMT32F1 and AVR were not affected. (#65)

## [1.2.1] - 2021-08-06

Version 1.2.1 is a minor revision from the previous version. This version aims mainly to improve the overall response of µCNC and fix a few bugs.
The following things were changed:

### Added
  - added support for G10 L2 P0 (current coordinate system) (#61)
  - implemented codes M0 and M1. M60 is also supported but behave like M0 (#58)

### Changed
  - revised and improved interlocking system that is more straight forward (code flow). Also this makes µCNC act more inline with the interface described by Grbl (#63)
  - all hard/soft limit alarms cause the firmware to lock until software reset is issued as described by Grbl (#63)
  - readapted homing and probing to the new interlocking logic (#63)
  - startup code improvements (#62)
  - modified µCNC to execute synchronous motions at motion control level. This reduces the pipeline travelling of the code at the expense of additional restart delay that is neglectable (#59)
  - dropped the Abort status in favor of the Alarm status to be more Grbl compliant (#59)
  - blocked status reports during startup blocks to prevent startup block ill-formated strings that were causing the interface software to correctly recognize the responses (#59)

### Fixed
  - fixed axis drifting after homing. This happened on all motions until an explicit coordinate was set for that axis (#63)
  - fixed hidden probe alarm status that only showed if other input alarms were active (#63)
  - G92 and G5x.x offset calculations (#61)
  - fixed inch report mode converted values output (#60)

## [1.2.0] - 2021-07-31

Version 1.2.0 is a major revision from the previous version that packs lots of new features and bug fixes.
Some of the major new features of this version are:
  - the new HAL configuration file that introduces a more flexible way to modify the HAL and give customization power of LinuxCNC.
  - the addition off new PID and encoder modules to be used by the new HAL config, powered by an internal RTC clock.
  - integration [tinyUSB](https://github.com/hathach/tinyusb), a complete USB stack frame that simplifies the creation of HAL code for new MCU.
  - the addition of an option for a 16-bit version of the bresenham line algorithm that can improve step rate for weak 8-bit processors or for specific applications like laser engraving.
  - several revisions, improvements and important bug fixes in the core of µCNC to generate reliable stepping code.

### Added
  - added basic settings for Grbl if startup emulation enabled (this includes $1-it's not used and always returns 0 and $11-that sets the G64-cosine factor. This value should be between -1 and 1. If 0 it acts as G61-exact path mode and -1 acts as G61.1 exact stop mode) (#55)
  - $0-max step rate is now used to top limit stepping frequency (#55)
  - implemented stm32f1 EEPROM emulation in flash (with limitations) (#54)
  - added new option for the 16-bit bresenham (instead of the 32-bit) version of the stepping generator algorithm (#49)
  - added new PID and encoder modules. PID parameters can be stored via commands $2x0 - Kp, $2x1 - Ki and $2x2 - Kd (#42)
  - integrated [tinyUSB](https://github.com/hathach/tinyusb) and adapted the core to use it (optional) (#41)
  - new HAL configuration file
  - added internal RTC (#38)

### Changed
  - cleaned code, redundant function call, unnecessary volatile attributes from variables and unused variable in the motion control, planner and interpolator stages of the core code (#52)
  - modified planner paths of motion and motionless actions (#51)
  - added main stepping and idle information to speed up general stepping calculations in the stepping ISR (#51)
  - optimization for synchronous serial TX with direct serial output without buffer (#50)
  - modified ring buffer in TX to consume chars without waiting for a CR or LF (#49)
  - modified motion control to reduce number of planner blocks for motions of length 0 (#49)
  - improved real time status report (?) (no longer needing and empty buffer to send report-more responsive) (#48)
  - response protocol collisions avoidance (#48)
  - report will now always report at least 3 axis even if less than 3 are configured (to keep report structure needed by interface softwares) (#48)
  - virtual MCU update to reflect current µCNC interface (#47)
  - added option to disable controls or limits IO globally (#45)
  - completely new file structure

### Fixed
  - fixed planner speed profile calculations that was missing speed change between blocks and was causing random miss stepping calculations (#52)
  - added initial NULL char sending after configuration of UART to force TXE hardware set for STM32 and deleted duplicate SYNC TX config for STM32 (#50)
  - fixed inch report mode setting that was hidden (#49)
  - fixed error message on disabling soft limits command with homing disabled (#49)
  - small step ISR code fixing (#46)
  - fixed stepper enable pin set/reset to match most stepper drivers (negative logic) (#42)

## [1.1.2] - 2021-06-23

Version 1.1.2 fixes a critical error on the STM32 HAL that cause several IO problems. µCNC core and AVR have no changes from the previous version:

### Changed
  - fixed STM32 HAL pin configuration macros caused bad pin configurations leading to unpredictable behavior. (#40)

## [1.1.1] - 2021-06-17

Version 1.1.1 comes with added features and improvements over the previous version. It also fixes a couple of bugs of the previous implementation. These are:

### Added
  - new mcu internal RTC to provide a running time reference (#38)
  - new build option to emulate Grbl startup message so that it can be recognized by several Grbl GUI applications like Candle (#36)
  - new software limit switch debouncing delay configurable via EEPROM (option/command $26=) (#34)

### Changed
  - improved laser mode to be compliant to Grbl's laser mode. Laser mode also has auto shutdown feature when motion stops (#29)
  - checks if DSS setting value is valid (#30)
  - improved fast math functions (more stability) and added new fast math pow2 function (#33)

### Fixed
  - fixed AVR HAL output pin toggle function. (#38) 
  - fixed hardware serial on ST32F10x HAL to work in sync mode (in async the communication breaks) (#37).
  - coolant/mist on/off functions and overrides (#28)
  - fixed parser active modal groups report (#28)
  - fixed active tools report (#28)
  - fixed DSS oversampling that was not reseted after motion end (#30)
  - fixed probing ISR tripping at startup by forcing probe_isr_disable after mcu_init (#32)



## [1.1.0] - 2020-08-09

Version 1.1.0 comes with many added features and improvements over the previous version. It also fixes many of the bugs and limitations of the previous implementation. These are:

### Added
  - new planner mode (linear actuator driven) that can be enabled via config file. This plans acceleration and deceleration based on the motion change in the linear actuators and not the cartesian axis. This should be advantageous on mechanically heavy machines. Also an option to enforce cold start motions (if any linear actuator starts at velocity 0 all other go to a full stop when ending the previous move - hybrid G61 and G61.1) (#23)
  - new backlash compensation (enabled via config file) with configurable via parameters `$140´(X), `$141´(Y), `$142´(Z), `$143´(A), `$144´(B) and `$145´ (C) (#23)
  - new axis skew compensation (enabled via config file)  with configurable via parameters `$37´(XY), `$38´(XZ) and `$39´(YZ) (#23)
  - new DSS (dynamic step spread) algorithm (similar to Grbl's AMASS (enabled via config file). This distributes step execution at lower step rates so that the vibration noise produced is reduced. (#22) (#18)
  - new STM32F10x HAL (#15). Although not all features are available it is usable. Boardmap for blue pill board available.

### Changed
  - modified distribution of main (to follow #15)
  - improved C99 standard compliance of code
  - new completely redesigned parser/gcode interpreter (easier to debug). A single functions is called to read and execute both gcode and grbl commands. (#21)
  - complete active modal codes report (#20)
  - improved separation of comment message processing and command echo (debug purposes only)

### Fixed
  - fixed AVR HAL pwm generation was not working for all channels. Also analog reading is now available. (#25) 
  - fixed overflow serial error that occur if 128 bytes (Grbl's limit) were sent to the buffer.
  - fixed several compilation errors with other configurations
  - fixed avr mapfile for grbl

## [1.0.0] - 2020-07-30

### Changed
  - modified makefile and instructions for AVR
  - improved soft polling of input limits and control pins (#11)
  - improved HAL design for future microcontrollers (#13)

### Fixed
  - fixed parsing error in check mode (planner position not updated after linear motions)
  - fixed feed override caused feed to go to 0 above 180% feed override value
  - change macro and library dependencies so that the option for fast Sqrt function works (AVR problem only)
  - fixed feed issue while G20(inches) was active that made the internal feed state value to always be converted from inches to mm even if not explicitly declared in the gcode command. This caused the feed rate to decay to 0. (#16)
  - fixed jog state that was permanently on after a finished (not aborted) jog motion. New jog commands were also not accepted while in this state (#19)
  - fixed parsing of reset commands ($RST=) that accepted non regular/incomplete forms of the command (#14)
  - fixed README
  - fixed devCpp project file (compilation errors) 

## [1.0.0-rc] - 2020-07-11

### Added
  - added possibility of changing values of G28 and G30 commands via G10 L2 P28 and G10 L2 P30.
  - added configuration for using software pulling input limit/control pins

### Changed
  - configuration now simplified (choose board only and the mcu is selected to match the board)

### Fixed
  - fixed code without probe
  - fixed code without spindle
  - compliance to strict-prototype functions

## [1.0.0-beta.2] - 2020-02-09

### Added
  - added AVR support by adding external interrupt pins to the limits and control pins range
  - new dual drive axis support with self squaring homing motion and locking of one stepper in the interpolator stage with the addition of two mirror step pins (with minimal performance loss)
  - added initial laser mode support
  - new filter to limits ISR to support dual drive auto squaring with IO overlapping (shared limit switches)

### Changed
  - complete HAL redesigned to allow independent function pins
  - new generic AVR HAL implementation
  - modified system exec_flags to prevent unnecessary locks
  - modified cartesian kinematics (configurable) to fit several types of machines (without the need to code a full kinematics)
  - separated the mcu and board now have individual configuration files
  - updated README.md to reflect changes of beta.2

### Fixed
  - fixed bug introduced in the beta patch to the homing motion (no limit_flag)
  - minor modification of mc_arc to prevent error on non existent axis
  - adapted protocol to send a minimum of 3 axis coordinates (yield error on interface with OpenCNCPilot and probably other software too)
  - fixed probe enable and disable functions (now use direct IO on AVR)

## [1.0.0-beta] - 2020-02-04

### Added
  - added initial TLO (tool length offset)
  - added support for new motion modes (G61.1 and G64)
  - added initial laser mode support
  - added CHANGELOG.md

### Changed
  - modified parser coordinate calculation and non modal gcodes handling so the remaining code executes as it should after these commands
  - redesigned parameters now loaded directly from eeprom and reduced ram usage (about 10% on Arduino UNO)
  - planner now holds up to 15 motions (stable no memory overflow)
  - redesigned spindle control to modify pwm according to motion (used in laser mode)
  - simplified step ISR functions (faster execution times - under 23ms for 3 stepper motors)
  - cleaned cnc_doevents() and some real-time commands functions handling

### Fixed
  - fixed settings version compare operation that caused eeprom reset at power up

## 1.0.0-alpha.3 - 2020-02-01

### Added
  - initial versioning system (Semantic Versioning)
  - can now process custom messages on comments and echo them (as explained in RS274NGC)
  - added dummy user transformation functions to kinematics
  - motion control probing (G38.x commands)
  - new mcu erase eeprom function
  - added startup blocks functionalities
  - new build version info file
  - added state "Check" to status report
  - spindle delays after start/stop on hold state and motion exact stop mode on spindle speed change

### Changed
  - redesigned the real-time commands response system
  - several modifications to serial functions (new ISR read char, new EOL char and overflow error detection mechanics). Also char to upper and other char processing stages moved from ISR to make it more slim
  - more optimizations on the serial ISR functions
  - interpolator blocks are no longer monitored (they are overwritten on the fly when computing new planner block)
  - changed mc_dwell resolution to 0.1s (less interrupts on the stepper ISR)
  - redesigned settings with crc checksum on all cnc and parser parameters
  - redesigned command echo with char indication parser error
  - added several escapes in loops with cnc_doevent() (avoid locks and exit loops on system abort)
  - interpolator buffer management optimizations
  - new strategy to handle startup blocks by serial read functions redirection
  - small planner buffer optimizations
  - several small optimizations regarding volatile variables

### Fixed
  - parser fixed by repositioning the commands discard function from the main loop to the parser internals and running only when needed (this would cause elimination of a hole line of unprocessed g-code after a line with error had been completely read by parser)
  - arc motion control generator (G2, G3 commands) had several errors caused by bad ABS macro math function definition (utils.h) and the incremental error compensation stage
  - planner forces motions with dwells to start with zero speed (so stop motion previous to dwell is executed)
  - improved stability that randomly caused locks
  - fixed serial extended ascii problem
  - fixed bug on feed overrides that had no effect if a single line was on the interpolator/planner buffer
  - fixed spindle pwm (MAX speed set output to 0)
  - fixed planner acceleration (error on angle factor calculation between vectors)
  - fixed gcode lines parsing error on Hold (they would return error)
  - fixed VERY STUPID bug that caused no error on virtual mcu but made avr mcu eeprom function not work properly
  - fixed EXEC_FLAGS to fix locked state behavior

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

[1.3.rc2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.3.rc2
[1.3.rc]: https://github.com/Paciente8159/uCNC/releases/tag/v1.3.rc
[1.3.b2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.3.b2
[1.3.b]: https://github.com/Paciente8159/uCNC/releases/tag/v1.3.b
[1.2.4]: https://github.com/Paciente8159/uCNC/releases/tag/v1.2.4
[1.2.3]: https://github.com/Paciente8159/uCNC/releases/tag/v1.2.3
[1.2.2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.2.2
[1.2.1]: https://github.com/Paciente8159/uCNC/releases/tag/v1.2.1
[1.2.0]: https://github.com/Paciente8159/uCNC/releases/tag/v1.2.0
[1.1.2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.1.2
[1.1.1]: https://github.com/Paciente8159/uCNC/releases/tag/v1.1.1
[1.1.0]: https://github.com/Paciente8159/uCNC/releases/tag/v1.1.0
[1.0.0]: https://github.com/Paciente8159/uCNC/releases/tag/v1.0.0
[1.0.0-rc]: https://github.com/Paciente8159/uCNC/releases/tag/v1.0.0-rc
[1.0.0-beta.2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.0.0-beta.2
[1.0.0-beta]: https://github.com/Paciente8159/uCNC/releases/tag/v1.0.0-beta
