# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About SD Card v2 for µCNC

SD Card v2 allows to add an SD/MMC card support to µCNC via hardware/software SPI.
It requires any 2 µCNC generic digital input pins of the board. It uses software I2C so no dedicated I2C hardware is required.

## Adding SD Card v2 to µCNC

To use the and SD Card v2 follow these steps:

1. Copy the the `sd_card_v2` directory and place it inside the `src/modules/` directory of µCNC
2. If needed you may redifine some IO pin and SPI options. By default this module tries to use the hardware SPI port if available and if not the software SPI pins. Please refer to [PINOUTS.md](https://github.com/Paciente8159/uCNC/blob/master/PINOUTS.md) to check the default pin associations.
To redefine the IO pins and if software or hardware SPI can is used open `cnc_hal_config.h` and add the needed configurations.

```
// By default the file execution stops if a GCode line has an error. This option makes the code execution continue even if an error occurs
// #define SD_CONTINUE_ON_GCODE_ERROR
// uncomment this to force software SPI even if hardware version is available
// #define SD_CARD_USE_SW_SPI
// uncomment to set the ouput pin used for the software SPI clock line
// #define SD_SPI_CLK DOUT30
// uncomment to set the ouput pin used for the software SPI serial data out
// #define SD_SPI_SDO DOUT29
// uncomment to set the input pin used for the software SPI serial data in
// #define SD_SPI_SDI DIN29
// uncomment to set the ouput pin used for the software SPI chip select
// #define SD_SPI_CS SPI_CS
// uncomment to set the input pin used to detect the card presence (this is an optional pin)
// #define SD_CARD_DETECT_PIN DIN19
// uncomment to use Petit FS (default)
// #define SD_FAT_FS 1
// Or for Fat FS (Fat FS will be used by default if ENABLE_SETTINGS_MODULES are enabled)
// #define SD_FAT_FS 2
// uncomment this to disable long file names in Fat FS (default 1)
// #define FF_USE_LFN 0
// uncomment this to change the maximum long file name length (default 64)
// #define FF_LFN_BUF 255
// uncomment to change the maximum path len (default 128)
// #define FS_MAX_PATH_LEN 128
```

4. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(sd_card_v2);
```

5. The last step is to enable `ENABLE_MAIN_LOOP_MODULES` and `ENABLE_PARSER_MODULES`(optional) and `ENABLE_SETTINGS_MODULES`(optional) inside `cnc_config.h`

## Using SD Card v2 on µCNC

SD Card v2 module adds a few system commands that allows you to navigate and execute files inside the SD/MMC card.

* ```$sdmnt``` - mounts the sd card. After mounting you will be at the root of the file system.
* ```$sdunmnt``` - unmounts the sd card.
* ```$ls``` - list all files and directories in the current directory.
* ```$cd <directory name>``` - changes the directory.
* ```$lpr <file name>``` - outputs the file content.
* ```$run <number of loops (optional)> <file name>``` - runs the code inside the file. The number of loops is an optional argument. If not defined or set to 0 the file will run one single time. If set to -1 it will run (almost) indefinitely. Examples:

Run the file one time
```
$run myfile.gcode
```

Run the file 10 times
```
$run 10 myfile.gcode
```
