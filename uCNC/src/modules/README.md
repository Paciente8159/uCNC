<p align="center">
<img src="https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true">
</p>


# µCNC
µCNC - Universal CNC firmware for microcontrollers

## Built in modules

µCNC as available several build in modules. These modules are:

### system_languages.h / language/
Provides language strings and localization assets used by the firmware.
Includes definitions for menu text, system messages, and kinematics‑related strings.
You can provide a language file for your own language. By default english is used.

### digimstep.c / digimstep.h
Implements support for digital microstepping drivers controlled via digital IO pins (like in the RAMBO board).
Provides compatibility macros and integrates with the system menu for configuration of microstepping parameters.
M351 command is also available with this module (similar to Marlin)

### digipot.c / digipot.h
Driver for digital potentiometers used to adjust current or other analog parameters of stepper drivers like in the RAMBO board.
Supports hardware/software SPI and exposes configuration options through the system menu.
M907 command is also available with this module (similar to Marlin)

### encoder.c / encoder.h
Handles encoder reading, atomic operations, and hook‑based callbacks for responsive input handling. Supports multiple types of encoders interfaces (AB input encoders, I2C encoders and SSI encoders), and custom interfaces can also be implemented. Encoders also can work as RPM meters.
Used for jog wheels, spindle feedback, or other closed‑loop features (stepper position loop feedback for example).

### endpoint.h / websocket.h
Defines JSON API endpoints used by the communication and file‑system layers and for a websocket communication channel to control the board via ethernet/wireless.
Forms part of the interface for external tools and UI integrations.

### file_system.c / file_system.h
Implements the internal file‑system layer.
Includes non‑blocking enqueue/dequeue buffering and optimizations for O‑code stack handling.
Used for SD card or virtual filesystem operations.

### ic74hc165.h
Driver for the 74HC165 parallel‑in serial‑out shift register.
Enables expansion of input pins for switches, sensors, and other digital inputs.

### ic74hc595.h
Driver for the 74HC595 serial‑in parallel‑out shift register.
Enables expansion of output pins for LEDs, relays, and other digital outputs.

### modbus.c / modbus.h
Implements Modbus communication support emulation.
Includes compatibility fixes and formatting improvements.
Used for industrial communication with external devices like spindles.

### pid.c / pid.h
Implements simple generic PID controllers for closed‑loop control tasks such as spindle speed or temperature regulation, etc...
Includes code cleanup and performance optimizations.

### softi2c.c / softi2c.h
Implements the I2C API interface used in µCNC and all I2C communications should be done via this module.
This API can use hardware available I2C port or use generic pins to emulate communications in a non-blocking way.

### softspi.c / softspi.h
Implements the SPI API interface used in µCNC and all SPI communications should be done via this module.
This API can use any of the 2 hardware available SPI ports (with or without DMA/ISR) or use generic pins to emulate communications in a non-blocking way.

### softuart.c / softuart.h
Implements the UART/OneWire API interface used in µCNC and all serial uart communications should be done via this module.
This API can use generic pins to emulate communications in a non-blocking way.

### system_menu.c / system_menu.h
This API provides some abstractions and act as an engine to render µCNC UI interfaces in graphic displays or LCD.
This API provides the rendering primitives to draw several custom screens like:
  - the boot screen
  - the alarm screen message
  - idle information screen
  - settings display and edit
Multiple primitives for events also are available like:
  - menu forward/backward
  - menu enter/exit
  - screen idle timeout
  - other user custom actions

Further information is [detailed here](https://github.com/Paciente8159/uCNC/blob/master/uCNC/src/modules/system_menu.md)
