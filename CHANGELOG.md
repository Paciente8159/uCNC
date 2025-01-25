![GitHub Logo](https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true)

# µCNC

µCNC - A universal CNC firmware for microcontrollers

# Changelog

## [1.11.2] - 25-01-2025

### Added

- added option to allow rotational axis to ignore limits after homing (#803)
- added option to use HW SPI on digipot module (#808)

### Changed

- improved settings variable types code readability (#821)
- improved homing error codes output/logic (#804)
- modified ESP8266 to use direct GPIO registers (#800)

### Fixed

- fixed var type in some homing settings that caused incorrect value report/storing (#819)
- fixed SPI typo on ESP8266 bulk send (#800)
- fixed compilation error with TOOL_COUNT set to 0 (#814)
- fixed parsing validation for extended commands with axis words (#809)
- fixed 74hc595 module to get pin value mask (#797)
- fixed kinematics default settings values for DELTA types (switched) (#796)

## [1.11.1] - 13-12-2024

[@tomjnixon](https://github.com/tomjnixon)	- fixed ignored G53 introduced by #791 (#793)
																						- fixed status report mask setting introduced in v1.11 (#789)

### Changed

- minor improvements the the AVR ini file and better comments (#784)
- change to the parser to unify G92 and G10 processing path (#791)

### Fixed

- fixed M2/M30 command behavior to match LinuxCNC (#786)
- fixed status report Tool info (#787)
- fixed status report mask setting introduced in v1.11 (#789)
- fixed G10 L20 behavior when the current modal distance mode is in incremental mode (#791)
- fixed cycle feed and spindle update in a canned cycle (#788)
- fixed/improved motion mode validation (#792)
- fixed ignored G53 introduced by #791 (#793)
- fixed step generation ISR algorithm for LPC17XX (#794)

## [1.11.0] - 20-11-2024

### Added

- added support for printing values of numbered and via [messages](https://linuxcnc.org/docs/html/gcode/overview.html#gcode:messages)
- added support for read-only [predifined named parameters](https://linuxcnc.org/docs/html/gcode/overview.html#gcode:predefined-named-parameters). Parameters can also be printed via $# command followed by the parameter number or parameter name (#767)
- added support for [O-Codes similar to LinuxCNC](https://linuxcnc.org/docs/html/gcode/o-code.html). O-Codes or subrotines runs custom GCode code and perform conditional logic and complex loops. O-Codes run from media storage .nc files thus each subrotine is self contained and it's not necessary to declare the SUB and ENDSUB O-Codes. The O-Codes must reside in the root system file of a pre-configured FS (C for MCU Flash FS or D for SD card FS). (#765) (#769)
- settings hardened safety features. On settings loading if an error is detected the machine enters locked mode and requires a full setting reset from the user before allowing unlock. This using the machine if the settings are not correctly loaded preventing failures when using settings from movable media sources like SD cards. Also on settings saving error the system will keep retrying to save the setting in case a write error occurs but allows the program to continue flowing. This option can be turned off by enabling ´DISABLE_SAFE_SETTINGS´ in the cnc_config.h file (#761) (#762)
- self implemented subset/custom of stdio printf helpers. It's now possible to print formated messages via protocol. No need to do specific calls to print variables like numbers, strings, char, ip addresse, bytes, etc... (#760)
- Debug messages now have an intermediate buffer that stores the output before printing it to prevent outputing debug messages in the middle of onging protocol messages (#760)
- SPI with DMA support for RP2040 (#713)
- Added initial support for RP2350 (#770)

### Changed

- all communications calls to output messages now are done via the proto_xxx calls in grbl_protocol.h. No more calls to serial stream directly mixed with protocol calls (#760)
- improvements to the debug message system. Now a single call to DBGMSG macro is used. The same principle of formated messages is applied (#760)
- modified/fixed emulated ONEWIRE transmission to use active output on low level logic (#778)
- improved timeout block macro (#779)(#780)

### Fixed

- added custom MCU build option to fix PIO build errors using overrides (#764)
- fixed/moved all custom PIO configurations generated in the web config builder to the platformio.ini file (fixes custom builds) (#766)
- added missing checks for ENABLE_NAMED_PARAMETERS (#774)
- fix parser backtracking char cleaning (#775)
- several custom printf function fixes (#776)
- fixed softuart module getc that prevented correct transmission start bit detection(#777)
- fixed debug stream print overflow and print guard causing issues when the debug stream was not the default stream (#779)(#781)
- fixed warning on LPC17XX I2C via Arduino Library (#783)

## [1.11.0-rc] - 07-10-2024

### Added

- added support for printing values of numbered and via [messages](https://linuxcnc.org/docs/html/gcode/overview.html#gcode:messages)
- added support for read-only [predifined named parameters](https://linuxcnc.org/docs/html/gcode/overview.html#gcode:predefined-named-parameters). Parameters can also be printed via $# command followed by the parameter number or parameter name (#767)
- added support for [O-Codes similar to LinuxCNC](https://linuxcnc.org/docs/html/gcode/o-code.html). O-Codes or subrotines runs custom GCode code and perform conditional logic and complex loops. O-Codes run from media storage .nc files thus each subrotine is self contained and it's not necessary to declare the SUB and ENDSUB O-Codes. The O-Codes must reside in the root system file of a pre-configured FS (C for MCU Flash FS or D for SD card FS). (#765) (#769)
- settings hardened safety features. On settings loading if an error is detected the machine enters locked mode and requires a full setting reset from the user before allowing unlock. This using the machine if the settings are not correctly loaded preventing failures when using settings from movable media sources like SD cards. Also on settings saving error the system will keep retrying to save the setting in case a write error occurs but allows the program to continue flowing. This option can be turned off by enabling ´DISABLE_SAFE_SETTINGS´ in the cnc_config.h file (#761) (#762)
- self implemented subset/custom of stdio printf helpers. It's now possible to print formated messages via protocol. No need to do specific calls to print variables like numbers, strings, char, ip addresse, bytes, etc... (#760)
- Debug messages now have an intermediate buffer that stores the output before printing it to prevent outputing debug messages in the middle of onging protocol messages (#760)

### Changed

- all communications calls to output messages now are done via the proto_xxx calls in grbl_protocol.h. No more calls to serial stream directly mixed with protocol calls (#760)
- improvements to the debug message system. Now a single call to DBGMSG macro is used. The same principle of formated messages is applied (#760)

### Fixed

- added custom MCU build option to fix PIO build errors using overrides (#764)
- fixed/moved all custom PIO configurations generated in the web config builder to the platformio.ini file (fixes custom builds) (#766)

## [1.10.2] - 07-10-2024

[@ademenev](https://github.com/ademenev)	- new alternative axis configurations for CoreXY kinematics (#750)
[@ricochet1k](https://github.com/ricochet1k)	- fixed STM32 prescaller calculations (#757)

### Added

- new custom PIO scripts to allow automated module download and configuration (#754)
- new alternative axis configurations for CoreXY kinematics (#750)

### Fixed

- fixed STM32 I2C IO alternate function configurations (#755)
- fixed debug stream configuration (#756)
- fixed STM32 prescaller calculations (#757)
- fixed STM32 SPI transmit function (#758)
- several platform/framework version errors and Arduino IDE build errors fixed


## [1.10.1] - 12-09-2024

[@patryk3211](https://github.com/patryk3211)	- modified system menu jog parameters to be globally accessible (#740)
[@etet100](https://github.com/etet100)	- fixed typo on steppers timeout sleep build flag

### Added

- new overridable PIO configuration ini file to integrate with the Web Builder and allow configuration customizations (#746)
- added boardmap for AVR Melzi v1.1.4 board (#749)
- added several PIO environment, one for each architecture to allow compilation for custom boards generated via Web Builder (#752)
- added new option to force HAL to set the IO pin direction every time there is a request to change the pin value. This is now the default for ESP32 to fix GPIO configuration lost mid execution (on resets, drivers reconfigurations, etc..) (#753)

### Changed

- modified system menu jog parameters to be globally accessible (#740)
- several modifications to PIO ini files, boardmaps and configurations to improve board customization via Web Builder (#752)

### Fixed

- locked RP2040 build platform version to prevent compilation issues to breaking changes (#739)
- fixed STM32F4 APB2 configuration macros that could try to use invalid values (#738)
- fixed ESP32 mcumap typos and bugs (#741) (#744)
- fixed software SPI resource locking bug (#743)
- fixed ESP32 SPI initialization functions that cause system crashes (#745)
- fixed issue with ESP32 were GPIO would loose the initial configuration and returned to the state at boot time (#747) (#753)
- fixed typo on steppers timeout sleep build flag
- fixed ESP32 software generated signals for IO extenders (#748)(#753)
- fixed query status response when sender uses a carriage return/linefeed char after the question mark making the firmware respond with an OK before the status (UGS for example) and make the sender fail to recogize the response (#751)

## [1.10.0] - 16-08-2024

[@patryk3211](https://github.com/patryk3211)	- added DMA supported SPI bulk transfers for all STM32 architectures and SAMD21 (#700) (#714)
																							- system_menu_goto is now a global function (#730)
																							- added extra pre-built actions to system menu and added friendly macros to menu pages number system (#718)
																							- additional system menu language strings (#735)
[@fooker](https://github.com/fooker)					- fixed servo pen tool speed range calculation callback (#733)

### Added

- added general support for SPI bulk transfers and modified SPI and SoftSPI HAL. (#701)(#712)
- added DMA supported SPI bulk transfers for all STM32 architectures and SAMD21. (#700) (#714)
- new module resource guard to allow access locks to shared hardware and software resources. simplified/removed soft/hard SPI config struck and lock flags(#716)
- new pin mapping helper that allow an easier pin mapping definition/setup for boardmaps (#722)
- extended generic IO pin range from 32 to 50 (#725)
- new module event listeners with resource sharing lock guard (#728)
- new MKS Robin Nano v3.1 boardmap (#729)
- added support for the SPI2 port on all architectures that support it (#724)
- added extra pre-built actions to system menu and added friendly macros to menu pages number system (#718)
- additional system menu language strings (#735)
- added stepper idle timeout functionality via $1 parameter (#734)

### Changed

- better SPI sharing by separation of hardware and software configurations and settings, and better busy state keeping (#715)
- added function to be able to modify a renderer/action callback for any system menu page and added extensible action code handler (#717)
- locked STM32 framework locking to prevent incorrect PIO configuration errors (#726)
-  system_menu_goto is now a global function. It also makes the function set action timeout and redraw flags to make sure the new screen gets shown even if it's not called from system_menu_action (#730)
- modified configurable STM32F4 and STM32F1 system clocks configuration (#732)

### Fixed

- fixed file system system commands to match #696. This was preventing commands from propagating (#705)
- fixed ESP8266 calls to get mcu time in microseconds and SPI transfer start (#707)
- fixed activity pin declaration on SKR Pro V1.2 board (#723)
- fixed typos (#731)
- fixed missing generic IO pins value reading and status print (#736) (#737)
- fixed servo pen tool speed range calculation callback (#733)

## [1.9.4] - 20-07-2024 (republished)

[@patryk3211](https://github.com/patryk3211) - fixed STM32Fx boards SPI implementation (#699)

### Added

- added HAL control access to the SPI CS pin to avoid having to waste a DOUT pin to control it. (#704)

### Changed

- modified flags for ESP32 to force Arduino SPI version (some boards seem to have problems with the SDK version) (#695)
- modified/merged entry point of user and architecture/board custom Grbl/System commands. This also fixed an issue that made impossible to pass arguments to user commands when the board had custom commands, since the buffer was parsed before command was evaluated for execution. (#696)
- reverted #695 and #691 for ESP32 Arduino SPI code

### Fixed

- fixed STM32Fx boards compilation errors is probe pin was undefined (#698)
- fixed STM32Fx boards SPI implementation (#699)
- fixed ESP32 boards SPI frequency/mode configuration (#703)
- fixed incomplete code propagation of #696. This prevented Grbl/System commands to propagate correctly and directly affected mount and unmount command of the SD card module (#705)
- fixed file system commands parsing

## [1.9.3] - 07-07-2024

[@patryk3211](https://github.com/patryk3211) - fixed homing pulloff fail alarm code (#689) and 2 phase homing cycle (#690)

### Added

- added new option to do a 2 phase homing cycle. Instead of the default fast-seek/slow-pulloff the new option modifies the behavior to fast-seek/fast-pulloff/slow-seek/slow-pulloff. This might increase homing repeatability (#690)

### Changed

- modified homing cycle to make code smaller (#693)
- improved softspi module and exposed HW SPI struct if available via softspi. This as several advantages and allows to share the same HW SPI in different speeds and modes. Also soft SPI now initializes pins on config. This fixed issues were the RTOS or Arduino modified the initial pin setting that was initialized by µCNC. (#691)

### Fixed

- fixed homing pulloff fail alarm code (#689)

## [1.9.2] - 13-06-2024

### Added

- option to allow RS274NGC expression and numbered parameters parsing (#688)

## [1.9.1] - 03-05-2024

### Added

- added new option to run the step generator interpolator in an ISR task and outside of the main loop. This should ensures the interpolator and the step ISR never gets starved and might aid to future code simplification (#685)

### Changed

- modified platformIO configuration files to include common libraries via the main platform.ini file (like u8g2)

### Fixed

- fixed G39/Height map clearing calls (#683)
- fixed file system typo that caused to build errors (#682)

## [1.9.0] - 18-04-2024

### Added

- added support for STM32F0 core (#681)
- new file system module. A C wrapper to work with files (in Flash or external memories) (#674)
- new generic, customizable ring buffer for communications (#676)
- new option to run RP2040 in multicore mode (experimental) (#677)

### Changed

- force DSS mode in THC tool mode (#625)
- decouple step encoder position from the interpolator step position (#630)

### Fixed

- fix systems sync after HMap generation and retration motions between probes (#680)

## [1.9.0-beta] - 11-04-2024

### Added

- new file system module. A C wrapper to work with files (in Flash or external memories) (#674)
- new generic, customizable ring buffer for communications (#676)
- new option to run RP2040 in multicore mode (experimental) (#677)

### Changed

- force DSS mode in THC tool mode (#625)
- decouple step encoder position from the interpolator step position (#630)

### Fixed

- fix systems sync after HMap generation and retration motions between probes (#680)

## [1.8.11] - 03-04-2024

### Fixed

- fixed interpolator deacceleation steps calculations if time slices less then 1 (#678)

## [1.8.10] - 29-03-2024

### Fixed

- prevent floating point round errors in speed calculations when time slices are too small (#673)

## [1.8.9] - 26-03-2024

## Contributors
[@lonelycorn](https://github.com/lonelycorn) - fixed DEBUG_STREAM when only one stream available (#664)
[@kunikonno](https://github.com/kunikonno) - fixed WiFi settings loading at initialization (#659)(#660)

### Added

- new flash files JSON API (RP2040, ESP32 and ESP8266) (#663)

### Changed

- clamp DSS_CUTOFF_FREQ in the sanity check (#670)
- added custom function to allow predefining IO state at startup (#669)
- added spindle relay tool speed range function (#658)

### Fixed

- prevent DEBUG_STREAM printing before initialization (#666)
- fixed DEBUG_STREAM when only one stream available (#664)
- fixed ESP8266 EEPROM WiFi saved settings not loaded at initialization (#660)
- fixed RP2040 EEPROM WiFi saved settings not loaded at initialization (#659)

## [1.8.8] - 03-03-2024

## Contributors
[@lonelycorn](https://github.com/lonelycorn) - modified mcumap macros of ESP32 to improve support for ESP32 family (#649) and removed duplicated servo variables for ESP32 (#648)
[@ademenev](https://github.com/ademenev) - added kinematics option for MP Scara (#645)

### Added

- Fs page update endpoint (#612)
- added option for rotational axis work always in relative distances (#624)
- STM32 NucleoF411RE boardmap with CNC Shield V3 (#628)
- added option to enable translated pins names status print (#634)
- added kinematics option for MP Scara (#645)
- added aditional Grbl emulation level to prevent miss detection of senders (#650)
- added option to do limit detection at the step ISR (#652)

### Changed

- allow execution of main loop events with HOLD condition active (#633)
- modified extended settings event hooks to be separated from other overriding events and propagations methods (#635) (#637) (#641)
- allow detached ports to keep or not an internal buffer (#639)
- cross architecture definition of NVM_STORAGE_SIZE and setting (#643)
- moved spindle restore logic to planner override (#647)
- modified mcumap macros of ESP32 to improve support for ESP32 family (#649) (#654)
- modified endpoints to support handling of wildcard terminators (#655)
- on command error now also the parser is forced to sync with other sub-systems (#657)
- modified behavior of Cycle Start/Resume button to execute only once per press (#657)

### Fixed

- fixed extensions settings event handling that prevented extended settings to be saved (#615)
- fixed STM32F4 compilation error when I2C HW was defined (#615)
- fixed program stall while waiting for timeout condition inside an ISR (#619)
- fixed STM32F4 SPI configuration AFIO fixed (#622)
- fixed range function for Plasma THC (#627)
- fixed STM32F4 APB registers of timers in the mcumap (#629)
- fixed compilation error when tool count was set to 0 (#632)
- fixed realtime command spindle toggle control over the tool (#631)
- fixed spindle restart message spawning (#636)
- fixed uart2 detach from main protocol typo in multiple boards (#638)
- fixed reset command parsing and early execution (#642)
- fixed spindle stop/restore after cancel a jog (#644)
- fixed dwell/delay execute even after a reset occured (#646)
- removed duplicated servo variables for ESP32 (#648)
- fixed low speed clock options for STM32F1 (#653)
- fixed STM32 I2C stop bit logic to prevent trail of pulses at the end of a read operation (#656)
- fixed pending jog motions after jog cancel (#657)
- fixed interpolator acceleration calculations to prevent ultra thin time sampling windows (#657)

## [1.8.7] - 03-02-2024

## Contributors
[@patryk3211](https://github.com/patryk3211) - fixed PID calculations (#604)
[@jarney](https://github.com/jarney) - fixed LIMIT_C mask inverting (#603)

### Added

- added boardmap for ESP32 Wemos D1 R32 with Shield V3 (#610)
- added initial implementation of websocket support (for ESP32, ESP8266 and RP2040) (#608)
- added software emulated serial one-wire (#600)
- added option to enable multistream guard (prevents serial stream to switch until line complete) (#596)
- added direct motion control incremental jog function (#594)
- added ESP32 DMA support via I2S to 74HC595 extender (#584)
- added ESP8266 servo pin support (#584)
- ESP8266 and ESP32 now use step aliasing in step generation to produce smoother speed increments at higher step rates (#584)

### Changed

- ESP32 web server and web socket server running on CPU0 (#608)
- modified module default handler macro to improve event calling rotation (#606) 
- removed all TMC driver support to external module (#599)
- minor changes to reduce code compilation size for UNO board (#593)
- cleaned code for CNC internal states (homing, jog and hold) (#590)

### Fixed

- fixed PID sample period calculation and prevent divide by zero operations (#604)
- fixed LIMIT_C mask inverting option typo (#603)
- fixed ESP8266 wifi listening to the wrong buffer (#601)
- fixed softuart reading failure and potential lock (#600)
- fixed AVR UART ISR macro (for multi and single UART MCUs) (#598)
- fixed tool speed report message (#592)

## [1.8.6] - 17-01-2024

## Contributors
[@patryk3211](https://github.com/patryk3211) - fix STM32F4x PWM configuration macro (#585)

### Changed

- modified emulator stepping timings (more stable and linear) (#587)
- modified probe command. Now with option to do realtime (in step ISR) checking. Also modified probe to do a controlled stop on contact (HOLD) to avoid step loss if probing too fast (#586)

### Fixed

- fix home motion flush after soft reset (#583)
- fix STM32F4x PWM configuration macro (#585)
- fixed G38.x command to allow only defined mantissas (#589)

## [1.8.5] - 12-12-2023

### Added

- added option to allow software homing (#572)

### Changed

- prevent module and main loop reentrancy to allow cnc main loop recall (inside waits and delays) (#576)
- better ESP software delays (#575)
- improved software generated PWM over 74HC595 for ESP32 (#574)

### Fixed

- fix jog flag being incorrectly cleared (#573)
- fix Laser PPI tool and incorrect speed calculations in motion control with Laser PPI option enabled (#578)

## [1.8.4] - 17-11-2023

### Added

- added support for 74HC595 custom shift register using PIO (#568)
- added function to get parser internal position (#537)
- added new debug stream option to use a dedicated COM channel to print debug verbose (#548)
- added option to control how soft limits are treated (alarm or error) and if the program flow continues or holds in case of error (#572)

### Changed

- rebuilded virtual emulator for Windows (#569)

### Fixed

- fixed non linear acceleration steps that generated motor noise on acceleration and deacceleration. These issues were introduce with change #561 (#571)
- reduced itp timer calculations for blocks at speeds bellow the interpolator sample frequency leading to incorrect timing on these slow speed blocks (#571)
- fixed stepper timer running at half the target speed on ESP32 using emulated PWM (#567)
- fixed cooland funtions M7 and M8 (#562)
- fixed step generation is interrupted while processing incomplete command (#565)
- fixed JOG flag being cleared while parsing command (caused with #565) (#573)

## [1.8.3] - 11-11-2023

### Fixed

- fixed motion stall if the motion has an instant jump from speed 0 to the target speed (instant acceleration) causing the motion speed not to be correctly calculated and stalling the whole interpolator queue. (#561)
- fixed error with option STATUS_AUTOMATIC_REPORT_INTERVAL enabled (#559)

## [1.8.2] - 03-11-2023

### Fixed

- fixed random stalls at the end of a deacceleration motion to a full stop. Under particular condition and due to float rounding errors speed may reach negative values and the motion being unable to continue. (#558)

## [1.8.1] - 01-11-2023

### Added

- added new endpoint interface to allow development of web server extension modules for MCU with WiFi (#554)
- added function to get parser internal position (#537)
- added new debug stream option to use a dedicated COM channel to print debug verbose (#548) 

### Changed

- ESP32 web server now runs on a separate task (#554)
- ESP32 moved rtc task to core 1 (#554)
- modified extented codes macro (#538)
- modified alarm logic to allow execution of real time commands even with alarm conditions active (#548)

### Fixed

- fixed USART ISR vector naming on AVR UNO using web builder generated override files (#557)
- fixed settings crc calculation to prevent accept invalid incorrect settings structures with stuffed bytes set to and leading to data offset missmatch (#553)
- fixed startup blocks printing format (#553)
- prevent alarm condition after enabling setting $21 (#549)
- fixed variable edit logic error on system menu (#547)
- fixed some variable types in system menu settings (#547)
- fixed signess of coordinates with option SET_ORIGIN_AT_HOME_POS (#542)

## [1.8.0] - 18-10-2023

### Added

- added status report extender callcack  (#454)
- added Plasma THC velocity anti-dive (#456)
- added initial Scara Kinematics (#460)
- added ESP32 optional optimized compilation using ESPIDF and Arduino as a component

### Changed

- unified interpolator run function and new S-Curve acceleration profiles (#458)
- implemented Plasma THC status report callback (#454)
- plasma THC tool update via PID callback (#453)
- configurable S curve patterns (#459)
- moved custom MCodes to laser ppi compilation unit (#464)
- added new RT Hooks inside interpolator step ITP, to be used by laser PPI and G33 (#464)
- moved all ESP32 I2S IO update calls to core0 (#485)
- added frequency clamp to step to frequency functions (#485)
- complete redesign of multiaxis system (#477)
- new autolevel with multi axis config (#477)
- fixed ITP for multi step linear actuators (#477)
- modified homing and added support for multi axis homing in parallel (#477)
- integrated backlash filtering in the rt segment flags (#477)
- modified cnc delay to run dotasks instead of only io_dotasks (#513)
- prevent re-entrant code inside dotasks events (#513)
- redesign the main loop tasks to prevent re-entrant code on the event callbacks (#513)
- force interlocking check before getting the alarm to force alarm code refresh if alarm condition is triggered by ISR (#513)
- fixed scara kinematics code to match new multi axis (#513)
- complete redesign of the serial communication to deal with multi-stream/sources and allow future expansions (#529)

### Fixed

- step output generation from beta (#457)
- fixed tool helper macros (#468)
- fixed bug in skew compensation (#476)
- fixed 74HC595 concurrency race (#478)
- fixed hold issue on version1.8 that keeps generating steps until the end of the motion (#489)

## [1.7.6] - 17-10-2023

### Changed

- modified/reordered settings display in system menu (#515)
- modified soft reset to improve software controller startup message detection (#531)
- simplified override messages to reduce compilation size (#531)
- modified system menu to fix warnings (#531)

### Fixed

- fixed USB infinite loop on flush call if unconnected (#511)
- fixed RUN state clear after alarm while running (#520)
- fixed pin status report function in command $P (#525)
- fixed IO input and output macros for RP2040 (#526)
- fixed RP2040 input change ISR macro (#526)


## [1.7.5] - 26-09-2023

### Added

- added function to be able to get current active alarm code (used on system menu alarm rendering) (#508)

### Changed

- uniformed architectures UART names (#483)
- STM32F1 modified config to allow use JTAG pins as GPIO (#486)
- modified system menu to allow multiple JOG commands chainned up (#501)
- modified system menu alarm screen condition to prevent alarm screen rendenring on startup lock (#507)

### Fixed

- fixed encoder option typo (#482)
- fixed jog command made permanent changes to parser state (#493) (#495)
- fixed compilation error for 5 or more axis machines (#499)

## [1.7.4] - 16-08-2023

### Added

- added option to allow machine homing using only homing cycle enabled and soft limits (#475)

### Changed

- modified UART TMC to be addressable (#466)

### Fixed

- fixed skew compensation not accepts negative values (#472)
- fixed skew compensation error accumulation over motions (#474)

## [1.8.0-beta] - 20-07-2023

### Added

- new IO HAL that simplifies io control and calls (#443)
- added support for motion control/planner hijack. This allows to stash and restore all current buffered motions to allow execution of a completly new set of intermediate motions (#432)
- added realtime modification of step and dir bits to be executed in the fly (#447)
- added new tool for plasma THC (#447)
- added debugging parsing execution time option (#452)
- added new step/dir output condition filter that prevents motion based on condition assert (#451)
- new set of macros that allow quick custom settings prototyping (#449)

### Changed

- all analog inputs were modified from 8bit resolution to 10bit  (#450)
- complete redesign of PID module and modified tools functions to make use of PID update loop (#449)

## [1.7.3] - 15-07-2023

### Changed

- modified TX protocol to prevent status message print in the middle of other feedback messages (#439)(#446)
- configurable laser PWM min value (#442)

### Fixed

- fixed parser/motion control position unsynched after mid motion error (#438)
- fixed some tools compilation errors with IO extender (#441)

## [1.7.2] - 02-07-2023

## Contributors
[@patryk3211](https://github.com/patryk3211) - allow negative values for some settings, and I2C ISR error recovery for STM32  (#400)(#407)

### Added

- new optional UART2 port (#402)(#403)
- new I2C HAL functions (#407)(#401)(#411)
- ESP32 alternative EEPROM and SPI functions via Arduino (optional) (#423)
- added new `$wifiip` command to print board IP address (#422)

### Changed

- Allow negative values for some settings (#400)
- All TX com ports now have a dedicated ring buffer, improving also WiFi transmission rates (#424)(#425)(#418)
- Pin remapping on 74HC595 for ESP32(#401)
- Multiple JOG commands can now be enqueued and sent to planner to allow smoother motion while jogging via external controller (#427)
- Protocol minor changes to response contamination with status reports (#430)

### Fixed

- MKS DLC32 missing boardmap settings (#420)
- Serial command buffer overflow causes controller to stop accepting new commands (#431)
- fixed broken USB communication on all platforms (#434)
- fixed OTA on ESP32 (#434)
- fixed swapped stepper 1 an 2 pins on MKS DLC32 board (#436)

## [1.7.1] - 13-05-2023

## Contributors
[@patryk3211](https://github.com/patryk3211) - new events at each axis homing (#393)

### Added

- new events at each axis homing start and end to allow custom actions like probe/limit deploying #393

### Changed

- modified `~` char logic to allow passthrough if a Grbl system command is being sent. This allows to write short file names to sd card addon module via commands (#395)
- system menu now displays the axis realtime position while axis is locked for jogging (#398)

### Fixed

- fixed position unshynchronized after cancaling a jog motion (#399)
- system menu jog command bad string initialization leading to random buffer overflow errors (#398)

## [1.7.0] - 12-05-2023

### Added

- added system menu alarm screen rendering logic (#379)
- added system menu modal popup (#380)

### Changed

- system menu tweaks and minor fixes (#391)

## [1.7.0-beta] - 08-05-2023

### Added

- new system menu module to handle all display related logic (#374)(#379)(#380)
- SKR Turbo v1.4 Wifi Serial boardmap config (#385)
- added default PINS for [RepRap Discount Full Graphic Smart Controller](https://reprap.org/wiki/RepRapDiscount_Full_Graphic_Smart_Controller) on several boards (#374)

### Changed

- reviewed TinyUSB mcu macros (#384)
- core module system function declaration tweaks (#373)

### Fixed

- fixed some deprecated PIN checking with ASSERT_PIN that caused compilation issues depending on the enabled options (TMC drivers, and RAMBO digipot and digistep) (#382)
- fixed USB hang on high rate gcode stress with NXP LPC boards (#386)
- fixed extended M Codes parsing for TMC drivers when not all AXIS are defined leading to unexpected behavior (#388)

## [1.6.2] - 04-05-2023

### Fixed

- fixed ```?``` command being cleared before responding depending on the execution point were the call was made. This will make the status command more responsive (#376)
- updated README

## [1.6.1] - 26-04-2023

### Added

- added EEPROM emulation to STM32F4 mcu boards (#370)
- added Bluetooth support for RP2040 (#364)

### Changed

- better STM32 variants support and configuration (#371)

### Fixed

- fixed missing UART RX PULLUP configuration that caused random character input stream on noisy environments, leading to random error messages and deadlocks (#369)
- fixed RAM_ONLY_SETTINGS option that was default in v1.6.0 pre builds (#369)

## [1.6.0] - 17-04-2023

### Added

- added core support for RP2040 MCU (#360)

### Changed

- moved tinyUSB out from the source and into an external library (#359)
- moved override configuration files to root directory of the project (#362)

## [1.6.0-alpha] - 19-01-2023

### Added

- initial release of core support for RP2040 MCU (#360)

## [1.5.7] - 12-01-2023

## Contributors
[@etet100](https://github.com/etet100) - servo bug fix and homing bug testing

### Added

- added support for spindle synched motions (#339)
- added compile option to enable IO alarm debug messages (#341)
- extended dual driver axis up to 4 dual axis using the full 8 stepper drivers control signals (#298)
- added macro to assert IO pins and created UNDEF_PIN macro (#342)
- added support for bilinear height map generation for irregular surfaces via custom G39 and G39.1 (#343)
- added support motion commands modifications at parser level (support for G7/G8 extension module) (#346)
- added boardmap for UNO CNC Shield v3 (Grbl 0.8 mapping) (#348)
- added servo controlled pen holder tool (#351)
- added new options to disable core parsing features to shrink code size (#354)

### Changed

- modified/fixed probing motions and probe status checks (#344)
- redesigned machine interlocking internal states (#353)
- alarms 1, 2 and 3 now require a reset command (all systems are reset) before allow unlocking (#353)
- added alarm condition when limits are hit without motion and position is not lost (#353)
- modified status messages of a resume after hold release (goes idle while starting spindle and then run with motion start) (#353)
- home cycle can now be executed with hard limits disabled (still requires endtops to be wired and configured) (#353)

### Fixed

- fixed data motion block initialization that caused random issues during homing (#350)
- fixed active modal states print added group0 mantissa to groups 1 to 6 (#353)
- fixed random homing error caused by incorrect reading after input inversion for retraction motion (#353)
- fixed multiple drive axis compilation error (#353)
- fixed error loop with ESTOP pressed (#353)
- fixed path mode not being reset on parser reset (#354)
- fixed incorrect offset reference for servo pins with servos not working. (#356)

## [1.5.6] - 28-11-2022

### Added

- new rotary delta kinematic support (#331)
- added entry for modules loaded via web config tool (#328)
- new parser module entry to allow creation of motion commands extensions (G5 and G5.1 are now available via external module) (#337)

### Changed

- migrated ESP32 from Arduino to ESP-IDF (except WiFi and Bluetooth libraries) (#334)(#335)
- dropped Arduino WiFiManager library for ESP32. WiFi and Bluetooth are now controlled via 'Grbl' type commands and are both available (fixed crashing) (#334)(#335)
- balanced ISR load on both cores of the ESP32 (#334)(#335)
- faster IO performance on ESP32, for both direct GPIO and IO expansion via 74HC595 (via I2S, SPI or GPIO) (#334)(#335)
- improved/fixed feed calculations and feedback, to support any type of linear/non-linear kinematics (#329)(#330)
- full motion control, planner and interpolator review, reorganized and optimized (#330)
- RAM optimizations (global and static variables reviewed, for both motion control and planner structures) (#329)

### Fixed

- fixed $ settings error for group settings (example steps per mm) (#327)
- motion control prevent error on linear motion of distance 0 (#327)
- fixed virtual simulator compilation errors (#327)
- fixed some PIO build code that caused ESP32 to crash (#333)

## [1.5.5] - 01-11-2022

### Added

- added dummy configuration override files tu support [µCNC config builder web tool](https://paciente8159.github.io/uCNC-config-builder/) (#325)

### Fixed

- fixed PROBE ISR issues on STM32 (#322)
- removed deprecated config that prevented probe from working correctly if PROBE_ISR was not configured (#322)

## [1.5.4] - 25-10-2022

### Fixed

- fixed stepper enable pins missing call with IC74HC595 module enabled (#320)
- fixed compilation error with parser modules enabled (#320)
- fixed probe IO modules enabling via option (#320)

## [1.5.3] - 22-10-2022

### Added

- added support for comments with ';' char (#291)
- added support for S word clustering format used by Smoothieware (#289)
- added support for external module $ settings (#293)
- added boardmaps for LPC176x boards SKR v1.4 Turbo and MKS Base V1.3 (#267)
- added boardmaps for STM32F4 boards MKS Robin Nano v1.2 and SKR Pro v1.2 (#299)
- added generic purpose ONESHOT timer (#295)(#301)
- added laser PPI with PPI control, Pulse width control and mixed control modes (#295)
- added extension modules $I info message modifier handler (#300)
- added basic/partial support for Powtran 8100 VFD tool (#311)
- Added boardmap for boards MKS DLC32 and MKS Tinybee including new core module for 74HC595 (shift-register) IO expander (#302)

### Changed

- added optimizations to motion control to reduce some redundant operations (#292)
- UART and USB can be used in parallel (#312)
- improved VFD safety if communications fail setting the machine in HOLD state (#317)
- completed Wemos D1 R32 pinout mapping (#318)

### Fixed

- fixed M2/M30 hold with check mode enabled caused program to stall (#297)
- fixed STM32 incorrect BAUDRATE config on other UART ports othern then UART1 (#309)
- fixed ARM us delay that caused deadlocks in the MCU after disabling global interrupts (#309)
- fixed RAMBO read MSTEPS ouput pin states via M351 (#309)
- fixed protocol message contamination with status report when using synchronous TX mode (#314)
- fixed soft UART and SPI causing communications to miss characters from host (#316)

## [1.5.2] - 01-10-2022

### Added

- configurable PWM frequency (#286)

### Fixed

- fixed no command response after a tool command without motion (#284)
- fixed incorrect laser power factor scaling with M4 (#282)
- $P servo report values for AVR (#283)
- fixed tool update with dwell to reach programmed speed

## [1.5.1] - 23-09-2022

### Fixed

- dwell is being executed ahead of time (async) (#276)

## [1.5.0] - 22-09-2022

### Added

- added Hardware I2C and SPI capabilities to several MCU (only ESP8266 and ESP32 not supported for now) that are integrated and used via software libraries (#249)
- added speed config function to MCU and soft SPI (#253)
- added events to support SD card module (#254)
- added LPC176x analog input support (#273)

### Changed

- ARM mcu share the same µs delay function calculated from SysTick clock (no loops or coredebug clocks used) (#249)
- software SPI/UART libraries use atomic operations macros (#249)
- moved activity led code to core (#250)
- better WiFi detect on ESP32 (#251)
- rewritten software I2C (#255)
- migrate LPC176x critical code to bare metal with serveral enhancements for ITP and SERVO ISR (#273)

### Fixed

- prevented dotask event lock reentrancy (#252)
- better delay functions for generic usage (including SPI, I2C and UART software libraries) (#264)
- fixed planner deacceleration calculations to prevent that caused noticeable abrupt stops with fast short motions (image rastering) (#275)

## [1.5.rc] - 2022-09-02

### Added

- added core support for ESP32 with limited functionalities (lacks analog and input ISR) (#237)
- added new io_control events to allow expansion IO modules (#247)
- added mcu_nop generic macro (#248)
- added tool HAL for VFD controlled tools and modbus core module (#239)

### Changed

- refactored event/delegate macros now with a single function declaration/signature. Adding new events for extensions is easier (#234)
- modified GRBL system command parser to enable command extensions via modules (#236)
- modified tools to convert between core speed and tool speed to avoid range/precision compression losses
- modified parser/planner to correctly calculate tool power output when minimal power is not 0 (example: laser minimal power output when S0) (#240)
- better µs delay for all platforms (now accepts 16bit value as argument) and better precision for AVR (#241)(#242)
- adapted bit-banging emulated protocols to fixed duration µs delays and software UART customizable timeout by interface (#242)

### Fixed

- fixed some extended MCodes definitions
- fixed SPI initialization in TMC driver (#242)
- fixed missing mcu_delay_us function on STM32F4 (#242)
- fixed event multiple declarations warnings introduced by (#234) (#243)
- fixed M4 laser mode power output with no motion introduced with #240 (#246)
- fixed AVR pin redefinition warnings (#248)

# Changelog

## [1.5.beta] - 2022-07-29

### Added

- added core support for ESP8266 with limited functionalities (lacks analog and input ISR) (#222)
- added support for NXP LPC176x core (lacks EEPROM and analog) (#227)

### Changed

- configurable RX serial buffer size (#222)
- updated tinyUSB to version 0.13.0 (#227)

### Fixed

## [1.4.7] - 2022-07-29

### Added

- added Marlin M913 command to TMC driver module (#226)

### Changed

- modified TMC driver to include shadow registers of write only (#226)
- modified/unified UART port definitions for all architectures (#230)
- minor optimization with auto-report enabled (#230)
- split platformio files for each board family (#231)
- updated virtual HAL for Windows (#232)

### Fixed

- fixed compilation error on STM32F1 if probe pin is not defined (#225)
- fixed TMC M914 extension command initialization (#226)
- fixed UART Arduino library conflict compilation error on PIO

## [1.4.6] - 2022-06-28

### Added

- added software I2C (bit-banging) (#215)
- removed encoders dependency of modules. Stepper encoders and regular encoders reset is now independent (#216)
- encoders counting pulse and direction can be inverted via setting $8 and $9 (#216)
- modified settings initialization code. Added option to store settings on RAM only (volatile) (#217)
- added option to store G92 offset in non volatile memory to prevent undesired wear (#221)

### Changed

- module system complete restructure. Core complementary modules kept in the core code. Remaining modules pulled to a different repository (#215)
- encoders initial state acquired at startup to prevent initial noise counts (#216)
- fixed dir mask detection for ENC0 and ENC1 (#216)
- moved the mod_cnc_dotasks_hook callback to allow it to work even if hold is active (#218)

### Fixed


## [1.4.5] - 2022-05-17

µCNC version 1.4.5 changes are listed below.
Next release will will have some extensive changes in µCNC modules to make it more easy to add an use modules in a need only basis.

### Added


### Changed

- option to enable self-squaring and dual-drive axis is now independent (#208)
- option to disable homing on each individual axis (#209)
- modified encoder to make it and independent module. This work a bit like a core functionality and not a module. (#213)
- stepper encoders now report directly the position via the interpolator (#213)
- added example of mirror RAMPS board (with encoders) (#213)
- redesigned limit/control/inputs switch logic. All logic is now performed inside the respective mcu callbacks (#212)

### Fixed

- EIMSK not being set on AVR MCU making external Interrupt pins not to trigger (#213)
- fixed ISR mask evaluation that prevented the correct ISR to catch the call leading to fault reset (introduced with #213) (#214)

## [1.4.4] - 2022-05-11

µCNC version 1.4.4 changes are:

## Contributors
[@jimsynz](https://github.com/jimsynz)

### Added

- added support for stall detection in TMC2209 drivers. Added equivalent Marlin M914 to modify sensitivity (#185)
- added support for G10 L20 (change coordinate system relative to current position) (#189)
- added example for BESC spindle (#191)

### Changed

- overridable F_STEP_MAX and F_STEP_MIN (#190)
- removed kinematic transformations filter from JOG motions to prevent unpredictable motions after forced motion systems sync (#195)
- modified Grbl compatible startup message (#196)
- updated M42 to reflect current HAL IO convention (#197)
- added servo pins configurations to RAMPS boardmap (pins D4, D5, D6 and D11) (#197)
- updated $P report to reflect current HAL IO convention (#198)
- improved BESC spindle control with added default values for throttle down, neutral and full positions (#200)
- improved BESC value range calculation (#202)
- modified tool speed feedback. Each tool reports speed directly and can customize the way the speed is reported (#203)
- simplified Encoder module. Can be enabled without extra modules
- added BESC RPM counter based on Encoder module 

### Fixed

- fixed typo in error constant name (#184)
- fixed G49 was not resetting TLO (#188)
- fixed motion systems unsync after recovering from emergency stop (#193)
- emergency stop press was not stopping tool as expected (#192)
- position read from motion control was not reversing user geometry transformations (#195)
- AVR DIN0-7 pins ISR was not enabled (#201)
- fixed error were coordinates would be forgotten/override if applying multiple G10 commands for different axis (ex. G10L20X0 and G10L20Y0) (#204)
- fixed logic error when both limits switches are active for an axis (not dual-drive) and are inverted, trigger would only happen if both were pressed (#205)
- fixed logic ORIGIN of signals when homing leading to incorrect trigger when self-squaring cause by #205 (#207)
- fixed G28-G30 not updating parser position leading to intermediate travel on next command (#211)

## [1.4.3] - 2022-05-02

µCNC version 1.4.3 fixes some bugs and added improvements to dual axis motion systems.

## Contributors
[@jimsynz](https://github.com/jimsynz) - relay controlled relay spindle tool

### Added

- MKS GEN L V1 boardmap based on ramps 1.4 (#175)
- auto status report option to send periodic status info without ? required (#179)
- added relay controlled spindle tool to tools (#176)
- added filter to control pins invert setting to prevent inverting undefined pins and activate it (#183)

### Changed

- dual axis is now more easy to configure and more flexible. Shadow register can be wired to unused steppers (#175)
- it's now possible to send some system commands (information commands) without being in the idle state (#178)
- partially reverted #163 to allow ESTOP invert via $7 also (#182)

### Fixed

- fixed missing stepper 6 and 7 DIR and EN pins from HAL (#175)
- redesigned axis dual endstop trigger with limits inverted (#175)
- wrong limit switch trigger during home with dual axis enabled was not being detected (#175)
- redesigned axis dual limit switch detection. Non dual drive axis limits are combined to detect collisions in min and max (#180)

## [1.4.2] - 2022-04-19

µCNC version 1.4.2 makes some changes to improve compatibility with Grbl software and patches some issues from the previous versions.

### Added


### Changed

- added option to accept G0 and G1 without explicit axis words to target (#169)
- implemented Grbl report status mask (#170)
- full Grbl welcome message emulation to fix recognition by PC softwares (#170)
- modified startup blocks execution to match Grbl behavior (#170)

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
- invalid EEPROM reset and $RST=\* now also clears N0 and N1 blocks as expected ($164)
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
- added digital potentiometer for stepper current regulation via SPI driver support (RAMBO board and similar). Optional M907 command available. (#156)
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
Beta release fixes several issues detected in the alpha version. It also expands the generic pins capabilities for future expansions.

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

µCNC version 1.3.3 aims to addressed several critical bug fixes in the gcode parsing (some of them introduced in the current major release):
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

µCNC version 1.3.2 aims to addressed several critical bug fixes in the gcode parsing (some of them introduced in the current major release):

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
- fixed input/limits/control ISR reentrance in SAMD21 and STM32 (#94)
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
- modified RTC to prevent reentrance inside ISR code in STM32 and SAMD21 (#75)
- modified planner buffer size in AVR size to prevent memory errors (#77)
- planner and interpolator block and segment buffer slots are cleaned before writing data to prevent errors from previous data. (#77)
- step ISR optimizations (for main stepper and idle steppers) are now optional. (#77)
- new AVR make file (#82)
- modified/simplified parser G90/G91 (absolute/relative coordinates) (#83)
- removed planner position tracking. Modified/simplified position tracking inside motion control. This is more memory effective and prevents desynchronization problems between the motion controller and the interpolator (#84)
- moved scheduled code running from hardware implementation to cnc (common) compilation unit. Makes code (pid update or other) architecture agnostic. (#86)

### Fixed

- fixed issue with active CS_RES input that caused resume condition (delay) without active hold present (#75)
- fixed SAMD21 PWM frequency configuration (now is aprox. 976Hz like AVR) (#78)
- real line number (N word) processing was not being read (#81)
- fixed welcome message not being sent after soft reset (#79)
- fixed step generation ISR random problems (stop working). This was caused by problems in the segment buffer read write. Solved by adding atomic lock blocks to the code (#85)
- G28 and G30 now perform in whatever coordinate mode before traveling home (RS274NGC compliant) (#83)

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

- fixed flash eeprom reading that caused SMT32F1 to hang with USB virtual COM port. SMT32F1 default value for erased flash is 0xFF and not 0x00. This caused the startup blocks to read a sequence of 0xFF chars. This was fixed by filtering the accepted values to standard ASCII only. Both serial versions of SMT32F1 and AVR were not affected. (#65)

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
- modified µCNC to execute synchronous motions at motion control level. This reduces the pipeline traveling of the code at the expense of additional restart delay that is neglectable (#59)
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

[1.11.2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.11.2
[1.11.1]: https://github.com/Paciente8159/uCNC/releases/tag/v1.11.1
[1.11.0]: https://github.com/Paciente8159/uCNC/releases/tag/v1.11.0
[1.11.0-rc]: https://github.com/Paciente8159/uCNC/releases/tag/v1.11.0-rc
[1.10.2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.10.2
[1.10.1]: https://github.com/Paciente8159/uCNC/releases/tag/v1.10.1
[1.10.0]: https://github.com/Paciente8159/uCNC/releases/tag/v1.10.0
[1.9.4]: https://github.com/Paciente8159/uCNC/releases/tag/v1.9.4
[1.9.3]: https://github.com/Paciente8159/uCNC/releases/tag/v1.9.3
[1.9.2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.9.2
[1.9.1]: https://github.com/Paciente8159/uCNC/releases/tag/v1.9.1
[1.9.0]: https://github.com/Paciente8159/uCNC/releases/tag/v1.9.0
[1.9.0-beta]: https://github.com/Paciente8159/uCNC/releases/tag/v1.9.0-beta
[1.8.11]: https://github.com/Paciente8159/uCNC/releases/tag/v1.8.11
[1.8.10]: https://github.com/Paciente8159/uCNC/releases/tag/v1.8.10
[1.8.9]: https://github.com/Paciente8159/uCNC/releases/tag/v1.8.9
[1.8.8]: https://github.com/Paciente8159/uCNC/releases/tag/v1.8.8
[1.8.7]: https://github.com/Paciente8159/uCNC/releases/tag/v1.8.7
[1.8.6]: https://github.com/Paciente8159/uCNC/releases/tag/v1.8.6
[1.8.5]: https://github.com/Paciente8159/uCNC/releases/tag/v1.8.5
[1.8.4]: https://github.com/Paciente8159/uCNC/releases/tag/v1.8.4
[1.8.3]: https://github.com/Paciente8159/uCNC/releases/tag/v1.8.3
[1.8.2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.8.2
[1.8.1]: https://github.com/Paciente8159/uCNC/releases/tag/v1.8.1
[1.8.0]: https://github.com/Paciente8159/uCNC/releases/tag/v1.8.0
[1.7.6]: https://github.com/Paciente8159/uCNC/releases/tag/v1.7.6
[1.7.5]: https://github.com/Paciente8159/uCNC/releases/tag/v1.7.5
[1.7.4]: https://github.com/Paciente8159/uCNC/releases/tag/v1.7.4
[1.8.0-beta]: https://github.com/Paciente8159/uCNC/releases/tag/v1.8.0-beta
[1.7.3]: https://github.com/Paciente8159/uCNC/releases/tag/v1.7.3
[1.7.2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.7.2
[1.7.1]: https://github.com/Paciente8159/uCNC/releases/tag/v1.7.1
[1.7.0]: https://github.com/Paciente8159/uCNC/releases/tag/v1.7.0
[1.7.0-beta]: https://github.com/Paciente8159/uCNC/releases/tag/v1.7.0-beta
[1.6.2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.6.2
[1.6.1]: https://github.com/Paciente8159/uCNC/releases/tag/v1.6.1
[1.6.0]: https://github.com/Paciente8159/uCNC/releases/tag/v1.6.0
[1.6.0-alpha]: https://github.com/Paciente8159/uCNC/releases/tag/v1.6.0-alpha
[1.5.7]: https://github.com/Paciente8159/uCNC/releases/tag/v1.5.7
[1.5.6]: https://github.com/Paciente8159/uCNC/releases/tag/v1.5.6
[1.5.5]: https://github.com/Paciente8159/uCNC/releases/tag/v1.5.5
[1.5.4]: https://github.com/Paciente8159/uCNC/releases/tag/v1.5.4
[1.5.3]: https://github.com/Paciente8159/uCNC/releases/tag/v1.5.3
[1.5.2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.5.2
[1.5.1]: https://github.com/Paciente8159/uCNC/releases/tag/v1.5.1
[1.5.0]: https://github.com/Paciente8159/uCNC/releases/tag/v1.5.0
[1.5.rc]: https://github.com/Paciente8159/uCNC/releases/tag/v1.5.rc
[1.5.beta]: https://github.com/Paciente8159/uCNC/releases/tag/v1.5.beta
[1.4.7]: https://github.com/Paciente8159/uCNC/releases/tag/v1.4.7
[1.4.6]: https://github.com/Paciente8159/uCNC/releases/tag/v1.4.6
[1.4.5]: https://github.com/Paciente8159/uCNC/releases/tag/v1.4.5
[1.4.4]: https://github.com/Paciente8159/uCNC/releases/tag/v1.4.4
[1.4.3]: https://github.com/Paciente8159/uCNC/releases/tag/v1.4.3
[1.4.2]: https://github.com/Paciente8159/uCNC/releases/tag/v1.4.2
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
