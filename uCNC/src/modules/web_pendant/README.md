# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About web pendant for µCNC

This module adds web pendant to µCNC. This allows basic control from a small 320x320 html page. This format can be direcly used in 3.2inches touch screens.
This is available for MCU's that support WiFi.

## Adding web pendant to µCNC

To use the web pendant module follow these steps:

1. Copy the the `web_pendant` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(web_pendant);
```

3. The last step is to enable `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`
