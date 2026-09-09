# µCNC
µCNC - A universal CNC firmware for microcontrollers

## µCNC for PC (Windows and Linux)
µCNC for PC can be built in a couple of ways

## Method one - DevC++
You can use DevC++ (I used portable version 5.11) and open load the uCNC.dev file to load the project and compile. That's it. Don't forget to modify the BOARD definition in the [cnc_config.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/cnc_config.h) to:
```
#define BOARD BOARD_VIRTUAL
```

## Method two - Using the makefile (outdated)
This makefile predates the current project file structure and no longer builds the current sources. Use method three (PlatformIO) instead.

## Method three - PlatformIO
The virtual board is also available as a PlatformIO environment (`EMULATOR_WINDOWS_TEST` and `EMULATOR_LINUX_TEST`), defined in [uCNC/src/hal/mcus/virtual/virtual.ini](https://github.com/Paciente8159/uCNC/blob/master/uCNC/src/hal/mcus/virtual/virtual.ini). This is the method used by the automated test suite (see [test/README.md](https://github.com/Paciente8159/uCNC/blob/master/test/README.md)).
