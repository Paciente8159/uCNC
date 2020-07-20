# µCNC
µCNC - A universal CNC firmware for microcontrollers

## µCNC for PC (Windows only for now)
µCNC for PC can be built in a couple of ways

## Method one - DevC++
You can use DevC++ (I used portable version 5.11) and open load the uCNC.dev file to load the project and compile. That's it. Don't forget to modify the BOARD definition in the [config.h](https://github.com/Paciente8159/uCNC/blob/1.0.x/uCNC/config.h) to:
```
#define BOARD BOARD_VIRTUAL
```

## Method two - Using the makefile
First you must have GCC (Mingw) tools installed on your computer.
Then just run the makefile
```
make -f makefile clean all
```

In the build folder you will have your exe file and can test it.
