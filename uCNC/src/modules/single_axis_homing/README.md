# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

# About Single Axis Homing for µCNC

Individual axis homing support for µCNC. Each axis can be homed via $H<axis letter> command

## Adding Single Axis Homing to µCNC

To use the and Single Axis Homing follow these steps:

1. Copy the the `single_axis_homing` directory and place it inside the `src/modules/` directory of µCNC

2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(single_axis_homing);
```

3. The last step is to enable `ENABLE_PARSER_MODULES` inside `cnc_config.h`


