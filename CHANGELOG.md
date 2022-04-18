![GitHub Logo](https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true)

# µCNC

µCNC - A universal CNC firmware for microcontrollers

# Changelog

## [1.4.2] - Unreleased

µCNC version 1.4.2 has the following changes

### Added


### Changed

- added option to accept G0 and G1 without explicit axis words to target (#169)
- implemented Grbl report status mask (#170)

### Fixed

- fixed parser wrong coordinates after coordinate offset command changes axis followed by a motion command with implicit target coordinates modified by the previous command, resulting in unexpected motion path (#170)
- forces run state flag during dwell (reported idle state) (#170)


## [1.4.1] - 2022-04-17

µCNC version 1.4.1 has an important bug fix regarding laser mode and M3 tool mode

### Added


### Changed


### Fixed

- fixed laser not turning on with laser mode enabled and after new feed motion command is issued without S word update and M3 active and the machine starts from an Idle state (#168)


## [1.4.0] - 2022-04-15

µCNC version 1.4.0 packs lots of new features as well as the initial support for SMT32F4 core MCU's, Trinamic drivers, canned cycles, Linear delta robot kinematics and more.

A special thank you note to Alexandros Angelidis (bastardazzo) for all the time and testing of the delta kinematics HAL.

### Added

- added inventables x-controller board (#163)
- added emergency stop logic invert option on main config file (#163)
- added build info command $I (optional via config file) ($164)
- added option to make homing has the machine origin (#162)

### Changed

- moved pin pullup configuration to HAL config file (#163)
- modified DSS for 32bit bresenham to allow bigger division factors (allowing lower end speeds) (#165)

### Fixed

- fixed tmc soft uart input pins (#163)
- fixed missing feedback message after settings reset command $RST ($164)
- invalid eeprom reset and $RST=\* now also clears N0 and N1 blocks as expected ($164)
- forced control pin checking to prevent undesired motion unlock while control pin state still active ($164)
- fixed tool length offset setting index
- fixed rounding error when printing float numbers close to the next integer value (#166)
- fixed homing when soft limits are enabled (#162)
- fixed sticking jog command (#167)

## [1.4.0-rc] - 2022-04-09

µCNC version 1.4.0 packs lots of new features as well as the initial support for SMT32F4 core MCU's, Trinamic drivers, canned cycles and more.

### Added

- added platformIO.ini file the µCNC project (#161)

### Changed

- improved subsegment planner computations by skipping junction speed calculations after initial calculation for first subsegment (#160)
- optimized the MKS DLC boardmap file to reuse UNO boardmap

### Fixed

- fixed random angle error calculation between planner line segments that cause random speed drops (#158)
- fixed compilation errors for main config file options (#159)
- fixed unit vector calculation in motion control line function (#160)

## [1.4.0-beta2] - 2022-04-01

µCNC version 1.4.0 packs lots of new features as well as the initial support for SMT32F4 core MCU's.
Beta2 adds a subset of canned cycles G codes to µCNC.
It also adds Trinamic driver support as well as RAMBO board hardware improvements.

### Added

- added canned cycles G81, G82, G83, G85, G86 and G89 to parser (enabled via config file) (#151)
- added softuart (bit-banging) module (#152)
- added TMC drivers integration. UART drivers are supported. SPI drivers are not tested. Optional M350, M906 and M920 commands available. (#155)
- added digital MSTEP pin support (RAMBO board and similar). Optional M351 command available. (#156)
- added digital potentiometer for stepper current regulationvia SPI driver support (RAMBO board and similar). Optional M907 command available. (#156)
- added softspi (bit-banging) module (#156)

### Changed

- modified homing (each kinematic has a fully custom homing motion and coordinate set) (#154)
- motion control modification to remove forward kinematic calculations from main motion pipeline (#154)
- parser detects duplicated words in command (#156)
- updated RAMPS and RAMBO boards pinout definitions with new features (#156)

### Fixed

- fixed dirmask bug introduced with #148 (#153)
- fixed stepper enable bitmask and motor locking/unlocking
- segmented motions for delta kinematics to improve non linearity of the towers motions (#154)
- fixed delta errors cause by stepping optimizations for linear motion systems (#154)
- fixed module ADDLISTENER macro. When adding multiple listeners to the same event only the last one was being called. (#155)
- added missing pins control for new pins (16 to 31) code to io_control core (#156)

## [1.4.0-beta] - 2022-03-15

µCNC version 1.4.0 packs lots of new features as well as the initial support for SMT32F4 core MCU's.
Beta release fixes several issues detected in the alpha version. It also expans the generic pins capabilities for future expansions.

### Added

- Output steps count on $P command print (#150)

### Changed

- renamed PWM OCR to PWM CHANNEL and all boardmaps for AVR (#147)
- modified UBRR calculation for AVR (match datasheet) (#147)
- expanded generic input and output IO from 16 to 32 (#149)
- added SPI pins definitions (SPI not implemented) (#149)

### Fixed

- added missing globals to init functions with FORCE_GLOBALS_TO_0 enabled (#147)
- fixed PWM config macro (was unstable on Mega boards) (#147)
- fixed set PWM macro for AVR (cause issues on Mega boards) (#148)
- fixed several C99/GNU99 compliance warnings (#148)
- fixed dir mask implementation ($2) (#148)
- fixed settings crc calculation (without lookup table) (#148)
- fixed output pin toggle macro for AVR (#148)
- fixed AVR RX pin setup (#148)
- fixed BYTE_OPS redefinition (#148)
- fixed preprocessing checking for servo function definitions (#149)

## [1.4.0-alpha] - 2022-02-25

µCNC version 1.4.0 packs lots of new features as well as the initial support for SMT32F4 core MCU's. The features are:

### Added

- added pid_stop action and alarm checking (safety stop) (#108)
- added new interpolator functions to be used by the PID module (#108)
- added S-Curve acceleration by modifying the Riemann sum interpolator to scan acceleration in fixed sample frames. (#137)
- added new SERVO pin type that outputs a 50Hz type PWM. Up to 6 SERVO pins can be configured (#138) (#146)
- added M10 mcode (set servo pin value) to core parser (#141)
- partial implementation supporting STM32F4 core. Lacks Flash EEPROM and analog pin reading is untested. (#139)
- experimental delta kinematics added to HAL (#142)
- new probe deploy/stow hookable callbacks for probing motion (#143)
- added initial implementation of bltouch module (#146)

### Changed

- redesigned µCNC modules for quick prototyping based on C# delegates, events and listeners. This allows adding hooks along the core code that call on modules sub-routines (#144)
- redesigned/simplified tool declaration in cnc_hal_config (#145)

### Fixed

- fixed spelling on the README file (#140)

## [1.3.8] - 2022-02-12

µCNC version 1.3.8 fixes a few bugs for STM32F1 mcus regarding pin configuration, some serial port issues and Flash EEPROM emulation on that same chip. For AVR a configuration fix was added for boards with ATMEGA2560 that prevented correct communication.

### Changed

- modified STM32 file to be flash offset agnostic (Reset vectors and Flash EEPROM) (#133)
- removed USB_VCP and COM macros and replaced by new configuration option INTERFACE (#134)
- modified STM32 USART port configuration to check pin configurations and allow pin remapping (#134)
- serial flush is non blocking (#134)
- step enable setting ($4) implemented (#136)

### Fixed

- fixed baudrate issue for USART (other than 1) by making APB1 and APB2 working frequency match (#132)
- added clock configuration to mcu_init to set correct working speed (72MHz) when compiling via Arduino IDE. (#132)
- fixed STM32 USART preprocessor condition that would not enable IRQ with both ISR TX and RX (#134)
- fixed STM32 USB clock configuration caused by (#132) (#135)
- fixed ATMega2560 boards USART ISR. In these boards the COM must be explicitly defined (#135)
- fixed STM32 pin masking configuration that cause configuration issues in PINs 10,11,14 and 15 of each port (#135)

## [1.3.7] - 2022-01-19

µCNC version 1.3.7 fixes a small bug that prevented µCNC from sending status report with an alarm condition at startup and a couple of bugs with DSS and 16-bit bresenham mode.
Besides that the parser was modified and can now be extended in a modular way, allowing custom gcode to be parsed and executed.

### Added

- parser can be extended to in a modular way to include additional gcode commands (#130)
- added example custom M42 (partial implementation of Marlin M42) (set pin state) (#130)
- force ISR update with every new block (#131)

### Changed

- removed deprecated options USE_SPINDLE and USE_COOLANT (#129)
- all headers are now included via cnc.h (#128)

### Fixed

- fixed tool initialization with FORCE_GLOBALS_TO_0 enabled (#129)
- fixed blocked status report on startup with alarm condition
- fixed planner bug that modified motion control feed rate for segmented motion lines (affected only 16-bit bresenham mode) (#131)
- fixed compilation error with DSS disabled (#131)

## [1.3.6] - 2022-01-16

µCNC version 1.3.6 fixes a bug that prevented the tool PWM from being correctly updated. This caused issues (artifacts) on laser engraving.

### Fixed

- modified interpolator to update the the tool inside the ISR call (not only when speed changed). The ISR was modified to carry segment information to the next generated step without before segment evaluation (#127)

## [1.3.5] - 2022-01-14

µCNC version 1.3.5 added major improvements to the virtual HAL. Although this HAL is only for testing purposes this in an invaluable tool for diagnostics.
It also adds a couple of important fixes that affected step generation with Dynamic Step Spreading enabled and sporadic stack overflow errors caused by nested loops in the cnc delay function.

### Added

- redesigned virtual MCU HAL (Windows OS only) (#122)
- virtual MCU HAL can be connected via sockets (default port 34000) (must be configured) (#122)
- virtual MCU HAL new GUI in C# that enables to interact with the IO (via named pipes) (#122)

### Changed

- on parser reset next status report will print WCO
- modified dual endstop behavior when dual endstops option is not active (#123)
- modified status report to yield better refresh rate (#125)
- modified/simplified realtime commands (reduced code size) (#126)

### Fixed

- cnc delay will be executed without exit even if there is an fault condition in dotasks loop. On input debounce the delay could be shortcuted and a fault condition could be triggered without being real (#125)
- removed deprecated preprocessor condition that was causing delay on motion restart even with laser mode active
- fixed some typos with option FORCE_GLOBALS_TO_0

### Fixed

- clearing interpolator now also resets dss previous value and clears running segment pointer to prevent step contamination from canceled motions (#124)

## [1.3.4] - 2022-01-10

µCNC version 1.3.4 adds a few improvements and also fixes some issues with inverse feedrate mode `G93` and realtime feed overrides.

### Changed

- added `G43.1` again for back compatibility with Grbl. Will work the same way has `G43` (#115)
- added `G43` and `G43.1` violation check against MOTION group commands (#115)
- `M2`can be now cleared with `$X` or `$H` commands (#119)
- modified DSS to force step ISR frequency to update on DSS change (#121)
- removed DSS minimum step limitation to prevent DSS algorithm on/off oscillation resulting in a smoother motion (#121)
- added configurable DSS cutoff frequency (#121)

### Fixed

- fixed overrides bug due to commented code that disable negative accelerations when slowing motion. Also done slight step ISR modifications to DSS calculations. (#116)
- fixed `G93` (inverse feed mode) feedrate calculation (#117)
- fixed `G93` (inverse feed mode) feedrate calculation for arcs (#118)

## [1.3.3] - 2022-01-07

µCNC version 1.3.3 aims to addresse several critical bug fixes in the gcode parsing (some of them introduced in the current major release):
It also removes `G43.1` (non compliant RS274NGC command) and replaces it with the `G43` compliant version. It still accepts the `Z` word. For tool lengths the `H` word should be used.
Tool lengths can be set and stored in settings `$41=<Tool1 offset>..$42=<Tool2 offset>...etc`. Also the default tool loaded at start/reset is stored via setting `$40`.
Encoders are also working in uni and bidirectional mode. Each encoder position is also reported by command `$P` available via `config.h`

### Added

- added configurable default loading tool and tools offset (#109)

### Changed

- modified/fixed PID controller to output positive/negative variation result (#108)
- added CLAMP macro to utils.h (#108)
- variable PID frequency depending on the number of PID defined. Frequency is now 1000/log2(Total PID's) in Hz (#108)
- optimized memory sizes for Kp, Kd and Ki gains (#108)
- PID math executes in 32bit integer math only (#108)
- modified scheduletasks and added isr locking to SAMD21 and STM32 (#108)
- moved some definitions to a new cnc_hal_config_helper.h file that is available via cnc.h (#108)
- hardwired tool pid to PID0 controller (#108)
- modified encoder module to allow it work has a unidirectional encoder (simple counter) (#107)
- added reset calls for motors encoders (#107)
- moved encoder and PID definitions to cnc_hal_config.h (#107)
- modified removed `G43.1` command and added `G43` command has defined in the RS274NGC. A similar command to previous `G43.1` is possible with `G43 Z<value>` (#109)
- probe command returns report like Grbl (#112)
- modified alarm locking and report messages on alarm status. Soft stop alarm require unlock only. Hard stops will cause soft reset on unlock (#111)
- homing motions adjusted to adapt to alarm modifications done in (#111) (#113)

### Fixed

- fixed tool length offset was not affecting the `WCO` position report (#109)
- tool length is set to 0 after reset (#109)
- modified settings change code (smaller and more efficient) (#110)
- fixed feed validation in motion group 0 to include probing commands (`G38.x`) (#112)
- fixed probing commands (`G38.4` and `G38.5`) reverse probe logic triggering (#112)
- fixed fail to probe target message and alarm (#112)
- fixed check mode position update of motion control preventing invalid target errors (#111)
- fixed sticky check mode even after soft-reset (#111)
- partially reverted modifications (#98) and (#84) that caused the the machine real position to diverge due to error accumulation with G91 (relative distance) active (#114)

### Fixed

- fixed PID settings offsets (#110)

## [1.3.2] - 2022-01-05

µCNC version 1.3.2 aims to addresse several critical bug fixes in the gcode parsing (some of them introduced in the current major release):

### Changed

- removed atomic blocks added with (#85) and relocated global ISR unlocking inside step ISR to be executed only in the step calculation section of the code (#101)
- simplified global ISR unlocking inside cnc_scheduletasks (#101)
- included limits and control in a configurable scheduled checking (ISR fail safe) (#101)
- modified SAMD21 compilation flags and board configurations (#101)
- reviewed SAMD21 and STM32 ISR to ensure they run in block mode (only one ISR at the time). ISR unlocking is controller by µCNC to make it more predictable (#101)
- removed duplicate tool pid call (#101)
- modified feed override flags so `M48/M49` will only affect at code execution order (#102)
- modified tool speed update and read functions and integrated HAL tool in the core of µCNC (#106)

### Fixed

- fixed feed override after reaching top speed feed was reset to normal (100%) neglecting feed override value (#102)
- fixed `M48/M49` parsing error (after calling overrides were always turned off) (#102)
- fixed spindle override max and min values (#100)
- fixed arc commands `G2/G3` with `G18` active parsing validation errors and mirrored motion error (#103)
- fixed motion commands (`G0`,`G1`, etc) with active offset (`G92` or `G5x`) introduced with (#83) and a given axis is omitted was reapplying the offset (#103)
- fixed `G4 P` word was not convert from seconds to milliseconds on the parser (#103)
- fixed `G53` with active `G91` (ignores `G91`) and now travels to the absolute position (#103)
- fixed interpolator speed calculations for slow movements with instant max speed that and speed was set to 0 causing µCNC to stop generating steps and not moving (#105)

## [1.3.1] - 2022-01-02

µCNC version 1.3.1 has the following modifications:

### Added

- added generic inputs ISR to AVR and STM32 (#94)
- added encoder specific reset function (#94)
- added extra command $P to list the state of all pins. This output each configured pin state a formatted message (#97)

### Changed

- modified the preprocessor definitions for step generation algorithm and undefined step actuator pins (virtual steppers for servo+encoder) (#92)
- modified makefiles to read a few command options (#96)
- modified ADC sampling frequencies on devices to make them more similar. The sample rate on all devices should be aprox. 125~100KHz (#93)
- modified bresenham algorithm variables initialization. Produces the same result but keeps variables with half the value (doubles the number of supported steps) (#98)

### Fixed

- added tool PID to cnc scheduled tasks (#95)
- fixed encoder module missing dir function and pulse previous state not being stored (#94)
- fixed input/limits/control ISR reentrancy in SAMD21 and STM32 (#94)
- call missing encoder update on input isr callback (#94)
- return argument on get_encoder_pos (#94)

## [1.3.0] - 2021-12-30

Version 1.3 is a major revision an targets add SAMD21 (Arduino Zero and M0) support.
It also adds the new tool HAL that allows to have multiple tools in the µCNC and support for gcode M6 command.
File structure as changed and tinyUSB was modified and integrated to allow compiling code using Arduino IDE.
The final version implements/adds the following improvements:

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
- modified planner buffer size in AVR size to prevent memory errors (#77)
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

- added SAMD21 (Arduino Zero and M0 boards) initial support. This is still an early release so there are still limitations for SAMD21. These are: (#72)
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

- new set of settings commands to control EEPROM/Flash storing (optional build setting ENABLE_EXTRA_SYSTEM_CMDS in cnc_config.h active by default) (#70)

  This set of new commands allow a more granular control over the settings stored in EEPROM/Flash to prevent wearing.
  When enabled all grbl $x=val are only changed in SRAM. To set them in non volatile memory a save command must be issued.
    3 additional commands are added:
    $SS - Settings save. This stores all values to EEPROM/Flash
  $SL - Settings load. This loads all values from EEPROM/Flash
  $SR - Settings reset. This loads all default values from ROM

### Changed

- enabled Grbl startup emulation to improve µCNC compatibility. Many Grbl interfaces expect the grbl startup message and won't recognize µCNC because of that simple fact (#70)
- changed tinyUSB config and descriptors file location and updated makefile for STM32 (#70)

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
- new axis skew compensation (enabled via config file) with configurable via parameters `$37´(XY), `$38´(XZ) and `$39´(YZ) (#23)
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

[1.4.1]: https://github.com/Paciente8159/uCNC/releases/tag/v1.4.1
[1.4.0]: https://github.com/Paciente8159/uCNC/releases/tag/v1.4.0
[1.4.0-rc]: https://github.com/Paciente8159/uCNC/releases/tag/v1.4.0-rc
[1.4.0-beta2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.4.0-beta2
[1.4.0-beta]: https://github.com/Paciente8159/uCNC/releases/tag/v1.4.0-beta
[1.4.0-alpha]: https://github.com/Paciente8159/uCNC/releases/tag/v1.4.0-alpha
[1.3.8]: https://github.com/Paciente8159/uCNC/releases/tag/v1.3.8
[1.3.7]: https://github.com/Paciente8159/uCNC/releases/tag/v1.3.7
[1.3.6]: https://github.com/Paciente8159/uCNC/releases/tag/v1.3.6
[1.3.5]: https://github.com/Paciente8159/uCNC/releases/tag/v1.3.5
[1.3.4]: https://github.com/Paciente8159/uCNC/releases/tag/v1.3.4
[1.3.3]: https://github.com/Paciente8159/uCNC/releases/tag/v1.3.3
[1.3.2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.3.2
[1.3.1]: https://github.com/Paciente8159/uCNC/releases/tag/v1.3.1
[1.3.0]: https://github.com/Paciente8159/uCNC/releases/tag/v1.3.0
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
