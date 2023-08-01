<p align="center">
<img src="https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true">
</p>


# µCNC
µCNC - Universal CNC firmware for microcontrollers

_**Jump to section**_
* [Available tools in µCNC](#available-tools-in-µcnc)
   * [spindle_pwm](#spindle_pwm)
   * [µCNC creating a dummy tool](#µcnc-creating-a-dummy-tool)

# Available tools in to µCNC

µCNC provides several ready-to-use tools. With little to no configuration these tools a pretty much ready to use.
Some tools may provide extra functionalities like extra settings or extra G/M codes.

## spindle_pwm

Spindle PWM can be used with any common PWM controlled tool.
These are the default µCNC pins/definitions for this tool:

```
// if unset uses PWM0 by default has the spindle speed control
#ifndef SPINDLE_PWM
#define SPINDLE_PWM PWM0
#endif
// if unset uses DOUT0 by default has the spindle dir output control
#ifndef SPINDLE_PWM_DIR
#define SPINDLE_PWM_DIR DOUT0
#endif

// enables coolant pins if this option is enabled
#ifdef ENABLE_COOLANT
// if unset uses DOUT2 by default has the spindle flood output control
#ifndef SPINDLE_PWM_COOLANT_FLOOD
#define SPINDLE_PWM_COOLANT_FLOOD DOUT2
#endif
// if unset uses DOUT3 by default has the spindle mist output control
#ifndef SPINDLE_PWM_COOLANT_MIST
#define SPINDLE_PWM_COOLANT_MIST DOUT3
#endif
#endif

// if unset PWM PID sample rate is 125Hz
// this tool PID settings are as follow
// $300=Kp
// $301=Ki
// $302=Kd
#define SPINDLE_PWM_PID_SAMPLE_RATE_HZ 125

```

## laser_pwm

Laser PWM is very similar to spindle PWM. The main difference is that this tool sets $31=1 (laser mode) on loading and restores it's value on unloading.
These are the default µCNC pins/definitions for this tool:

```
// if unset uses PWM0 by default has the laser power control
#ifndef LASER_PWM
#define LASER_PWM PWM0
#endif

// if unset set PWM frequency to 8Khz to improve laser response. By default µCNC PWM pins works at 1Khz
#ifndef LASER_FREQ
#define LASER_FREQ 8000
#endif

// enables coolant pins if this option is enabled
#ifdef ENABLE_COOLANT
// if unset uses DOUT2 by default has the air assist output control
#ifndef LASER_PWM_AIR_ASSIST
#define LASER_PWM_AIR_ASSIST DOUT2
#endif
#endif

// this sets the minimum power (laser will never fully turn off during engraving and prevents power up delays)
#ifndef LASER_PWM_MIN_VALUE
#define LASER_PWM_MIN_VALUE 2
#endif

```
