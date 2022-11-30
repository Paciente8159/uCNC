# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About G33 for µCNC

This module adds custom G33 code to the µCNC parser. This similar to Linux CNC G33 and allows to make motions synched with the spindle in µCNC.

## Adding G33 to µCNC

To use the G33 parser module follow these steps:

1. Copy the the `G33` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(g33);
```

3. The last step is to enable `ENABLE_MAIN_LOOP_MODULES`, `ENABLE_IO_MODULES`, `ENABLE_PARSER_MODULES` and `ENABLE_SETTINGS_MODULES` inside `cnc_config.h`
