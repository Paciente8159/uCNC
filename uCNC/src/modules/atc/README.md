# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

# About ATC for µCNC

This module adds ATC-automatic tool changer scripts to µCNC.

## Adding ATC to µCNC

This module requires the controller to have some sort of file system mounted (either internal flash or an external SD Card).
This module opens and runs .nc files when a tool change is performed and allows the user to run custom GCode scripts to automate tool changing actions.

The tool expects that for each tool that you have configured, a /<drive>/atc/tool<XX>mnt.nc and /<drive>/atc/tool<XX>umnt.nc is available. If the file exists it runs the script defined by the user. This can include any GCode sequence that performs the necessary action to load or unload a tool.

By default the flash file system drive <C> (if available) is searched. This can be modified to make use of an external drive like the SD card module <D> drive

1. Copy the the `atc` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(atc);
```

3. You must configure which drive is to be used by adding in `cnc_hal_overrides.h`

for the MCU flash FS (if available)

```
#define ATC_FS_DRIVE 'C'
```

or for the SD card module

```
#define ATC_FS_DRIVE 'D'
```

4. The last step is to enable `ENABLE_MAIN_LOOP_MODULES` and `ENABLE_ATC_HOOKS` inside `cnc_config.h`
