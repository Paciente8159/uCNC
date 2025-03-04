<p align="center">
<img src="https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true">
</p>


# µCNC
µCNC - Universal CNC firmware for microcontrollers

_**Jump to section**_
* [Available tools in µCNC](#available-tools-in-µcnc)
   * [spindle_pwm](#spindle_pwm)
   * [laser_pwm](#laser_pwm)
   * [laser_ppi](#laser_ppi)
   * [pen_servo](#pen_servo)
   * [spindle_relay](#spindle_relay)
   * [spindle_besc](#spindle_besc)
   * [vfd_pwm](#vfd_pwm)
   * [vfd_modbus](#vfd_modbus)
   * [plasma_thc](#plasma_thc)

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
// if unset uses DOUT2 by default has the tool flood output control
#ifndef SPINDLE_PWM_COOLANT_FLOOD
#define SPINDLE_PWM_COOLANT_FLOOD DOUT2
#endif
// if unset uses DOUT3 by default has the tool mist output control
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
Like in Grbl, M4 - dynamic laser power is available, making the PWM output dimmer in proportion to the motion acceleration/deacceleration to improve engraving quality. 
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
// M5 will still turn laser completely off. M3 S0 or M4 S0 will output PWM min.
#ifndef LASER_PWM_MIN_VALUE
#define LASER_PWM_MIN_VALUE 2
#endif

```


## laser_ppi

Laser PPI is designed to control laser in pulse per inch mode (PPI).
This is actually achieved by treating the laser output control pin as an additional stepper motor.
There are several advantages to this. The first one is that the energy of the laser is tightly coupled with the motion. That ensures that the output result will be the same regardless of the engraving speed.
There is a tradeoff. To ensure that the laser control pulse width is achieved, the maximum achievable speed might be reduced.

Laser PPI is able to use not only generic output pins but also PWM pins by changing the configuration on tool loading.
Laser PPI has 3 working modes.
  - PPI mode, by setting $32=2. This mode will ensure that the programmed number of pulses per inch with a set width will be output and is configured by setting $33. The number of pulses will be affected by the tool S parameter. For example if you define a PPI value of 254 (254 pulses per inch) and set the S to 500 (in a range of 0-1000, or in other words 50%) the PPI output will be adjusted to 50% (or 127PPI). This is similar to the ways M4 works in laser PWM. The pulse width is fixed and is configured by setting $34 (in us-microseconds)
  - PPI PW mode, by setting $32=4. This mode will ensure that the programmed number of pulses per inch with a set width will be output and is configured by setting $33. In this case the tool S parameter will affect the pulse width set by $34. For example if you define a PPI width value of 900us and set the S to 500 (in a range of 0-1000, or in other words 50%) the PPI pulse width output will be adjusted to 50% (or 450us).
  - PPI mixed mode, by setting $32=6. This mode will ensure that the programmed number of pulses per inch with a set width will be output and is configured by setting $33. This mode makes a ratio mix of both the above mentioned modes settings $35(PPI blend ratio) and $36(PPI width blend ratio). For example if you define a PPI value of 254 and a width value of 900us and $35 = 0.25 (25%) and  $36 = 0.75 (75%) and set the S to 500 (in a range of 0-1000, or in other words 50%) the PPI will be adjusted in 0.25 of 50% of the value (reduced in 12.5% of 254 ~= 222PPI) and pulse width output will be of 0.75 of 50% (reduced in 37.5% of 900us ~= 563us).

This tool also enables a few extra MCode commands that allow to directly modify settings $32, $33 and $34. These are

```
M126 P<mode>

mode = 0 disables PPI mode
mode = 1 enables PPI mode
mode = 2 PPI PW mode
mode = 3 PPI mixed mode

```

```
M127 P<pulses per inch value>

```

```
M128 P<pulse width in us value>

```

These are the default µCNC pins/definitions for this tool:

```
// if unset uses PWM0 by default has the laser PPI control
#ifndef LASER_PPI
#define LASER_PPI PWM0
#endif

// enables coolant pins if this option is enabled
#ifdef ENABLE_COOLANT
// if unset uses DOUT2 by default has the air assist output control
#ifndef LASER_PPI_AIR_ASSIST
#define LASER_PPI_AIR_ASSIST DOUT2
#endif
#endif

```

## pen_servo

Pen servo uses a SERVO type pin (Pulse width 0.5ms~2.5ms @ 50Hz) to control a small DC servo.
These are the default µCNC pins/definitions for this tool:

```
// if unset uses SERVO0 by default has the pen servo control
#ifndef PEN_SERVO
#define PEN_SERVO SERVO0
#endif

// defaults the servo ouput value for the pen to be low position to 50 (0-255 range)
#ifndef PEN_SERVO_LOW
#define PEN_SERVO_LOW 50
#endif
// defaults the servo ouput value for the pen to be high position to 255 (0-255 range)
#ifndef PEN_SERVO_HIGH
#define PEN_SERVO_HIGH 255
#endif

```

## spindle_relay

Spindle relay uses relays to control a spindle tool.
These are the default µCNC pins/definitions for this tool:

```
// if unset uses DOUT0 by default has the turn spindle on and forward output control
#ifndef SPINDLE_RELAY_FWD
#define SPINDLE_RELAY_FWD DOUT0
#endif
// if unset uses DOUT0 by default has the turn spindle on and reverse output control
#ifndef SPINDLE_RELAY_REV
#define SPINDLE_RELAY_REV DOUT1
#endif

// enables coolant pins if this option is enabled
#ifdef ENABLE_COOLANT
// if unset uses DOUT2 by default has the spindle flood output control
#ifndef SPINDLE_RELAY_COOLANT_FLOOD
#define SPINDLE_RELAY_COOLANT_FLOOD DOUT2
#endif
// if unset uses DOUT3 by default has the spindle mist output control
#ifndef SPINDLE_RELAY_COOLANT_MIST
#define SPINDLE_RELAY_COOLANT_MIST DOUT3
#endif
#endif

```

## spindle_besc

Spindle BESC uses brushless motor controlled by a BESC.
These are the default µCNC pins/definitions for this tool:

```
// if unset uses SERVO0 by default has the BESC speed signal control
#ifndef SPINDLE_BESC_SERVO
#define SPINDLE_BESC_SERVO SERVO0
#endif
// if unset uses DOUT0 by default has the BESC relay power on control control
#ifndef SPINDLE_BESC_POWER_RELAY
#define SPINDLE_BESC_POWER_RELAY DOUT0
#endif

// enables coolant pins if this option is enabled
#ifdef ENABLE_COOLANT
// if unset uses DOUT2 by default has the spindle flood output control
#ifndef SPINDLE_BESC_COOLANT_FLOOD
#define SPINDLE_BESC_COOLANT_FLOOD DOUT2
#endif
// if unset uses DOUT3 by default has the spindle mist output control
#ifndef SPINDLE_BESC_COOLANT_MIST
#define SPINDLE_BESC_COOLANT_MIST DOUT3
#endif
#endif

```

## vfd_pwm

This is similar to spindle PWM. The main difference is that it uses an RPM feedback with an analog input.
These are the default µCNC pins/definitions for this tool:

```
// if unset uses PWM0 by default has the VFD PWM speed signal control
#ifndef VFD_PWM
#define VFD_PWM PWM0
#endif
// if unset uses DOUT0 by default has the vfd dir output control
#ifndef VFD_PWM_DIR
#define VFD_PWM_DIR DOUT0
#endif

// enables coolant pins if this option is enabled
#ifdef ENABLE_COOLANT
// if unset uses DOUT2 by default has the tool flood output control
#ifndef VFD_PWM_COOLANT_FLOOD
#define VFD_PWM_COOLANT_FLOOD DOUT2
#endif
// if unset uses DOUT3 by default has the tool mist output control
#ifndef VFD_PWM_COOLANT_MIST
#define VFD_PWM_COOLANT_MIST DOUT3
#endif
#endif

// if unset uses ANALOG0 input by default has the VFD input speed feedback control
#ifndef VFD_PWM_ANALOG_FEEDBACK
#define VFD_PWM_ANALOG_FEEDBACK ANALOG0
#endif

// if unset VFD_PWM PID sample rate is 125Hz
// this tool PID settings are as follow
// $304=Kp
// $305=Ki
// $306=Kd
#ifndef VFD_PWM_PID_SAMPLE_RATE_HZ
#define VFD_PWM_PID_SAMPLE_RATE_HZ 125
#endif

```


## vfd_modbus

This tool controls a VFD using modbus communication protocol.
The modbus is supported over the softuart module that implement a pure software (bit-banging) uart emulation.

Currently supports:
  - HUANYANG type1 (tested)
  - HUANYANG type2 (untested)
  - YL620 (partially tested)

These are the default µCNC pins/configurations for this tool:

```
// defines default coolant pins
#ifdef ENABLE_COOLANT
// if unset uses DOUT2 by default has the tool flood output control
#ifndef VFD_COOLANT_FLOOD
#define VFD_COOLANT_FLOOD DOUT1
#endif
// if unset uses DOUT3 by default has the tool mist output control
#ifndef VFD_COOLANT_MIST
#define VFD_COOLANT_MIST DOUT2
#endif
#endif

// defines VFD report mode
// if false returns programmed speed
// if true return speed read from VFD (if VFD supports this)
#ifndef GET_SPINDLE_TRUE_RPM
#define GET_SPINDLE_TRUE_RPM false
#endif

// defines default softuart pins and vfd communication settings
#ifndef VFD_TX_PIN
// uses DOUT27 as the default UART TX pin on softuart
#define VFD_TX_PIN DOUT27
#endif
// uses DIN27 as the default UART RX pin on softuart
#ifndef VFD_RX_PIN
#define VFD_RX_PIN DIN27
#endif
// uses a default baudrate of 9600 for the softuart
#ifndef VFD_BAUDRATE
#define VFD_BAUDRATE 9600
#endif
// uses a default value of 100ms as a communication timeout
#ifndef VFD_TIMEOUT
#define VFD_TIMEOUT 100
#endif
// uses a default value of 100ms as a communication retry in case of a timeout
#ifndef VFD_RETRY_DELAY_MS
#define VFD_RETRY_DELAY_MS 100
#endif

// by default it will put the machine on HOLD if a communication error happens with the VFD
// you can add this to override that behavior
// #define IGNORE_VFD_COM_ERRORS

// sets the VFD communication address
#define VFD_ADDRESS 8
// sets the number of retries in case of a communication error
#define VFD_MAX_COMMAND_RETRIES 2

```

Most VFD don't follow any type of standard regarding modbus implementation. This tool has a set of formated blocks for 3 different brands of VFD but possibly can be extended to be used with others.

A VFD custom VFD might be added by defining the message settings and the VFD settings needed to convert frequency to speed and vice versa

```
/**
 *
 * VFD Commands are an array of 6 (if first byte is 0) or 7(otherwise) bytes that have the following information
 *
 * {tx command length, rx command length, MODBUS command type, start address high byte, start address low byte, value high byte, value low byte}
 *
 * A typical MODBUS command is usually 8 <vfd address (1byte) + MODBUS command type (1byte) + start address (2byte) + value (2byte) + CRC (2byte)>
 * A tx command length of 0 will mute the command and rx length should be ommited
 *
 * Some VFD do not use the standard MODBUS message format and some times the command length is different (like the Huanyang Type1)
 * 
 * Apart from defining the needed VFD commands, 4 additional definitions must be set to allow converting from tool speed to vfd speed and backward
 * Usually on VFD's this is a convertion from VFD frequency to spindle RPM
 *
 * */
```

Let's digest the set RPM command for the HUANYANG type1 with the above information

```
// 7 bytes
#define VFD_SETRPM_CMD                                         \
	{                                                          \
		7/*tx command length*/, 6/*rx command length*/, MODBUS_FORCE_SINGLE_COIL/*MODBUS command type*/, 0x02/*start address high byte*/, 0x00/*start address low byte*/, 0x00/*value high byte*/, 0x00/*value low byte*/ \
	}
```

Here is an example of a 6 byte command (dummy command)

```
// 6 bytes
#define VFD_RPM_HZ_CMD                                           \
	{                                                            \
		0/*tx command length (dummy command)*/, MODBUS_READ_HOLDING_REGISTERS/*MODBUS command type*/, 0xB0/*start address high byte*/, 0x05/*start address low byte*/, 0x00/*value high byte*/, 0x02/*value low byte*/ \
	}
```

Here is the example for the HUANYANG type1 that does not use 8 byte length commands

```
#if (VFD_CONTROLLER == VFD_HUANYANG_TYPE1)
#define VFD_SETRPM_CMD                                         \
	{                                                          \
		7, 6, MODBUS_FORCE_SINGLE_COIL, 0x02, 0x00, 0x00, 0x00 \
	}
#define VFD_GETRPM_CMD                                            \
	{                                                             \
		8, 8, MODBUS_READ_INPUT_REGISTERS, 0x03, 0x01, 0x00, 0x00 \
	}
// sets fixed freq 400.00Hz -> 40000
#define VFD_RPM_HZ_CMD                                        \
	{                                                         \
		8, 8, MODBUS_READ_COIL_STATUS, 0x03, 0x05, 0x00, 0x00 \
	}
#define VFD_CW_CMD                                                  \
	{                                                               \
		6, 6, MODBUS_READ_HOLDING_REGISTERS, 0x01, 0x01, 0x00, 0x00 \
	}
#define VFD_CCW_CMD                                                 \
	{                                                               \
		6, 6, MODBUS_READ_HOLDING_REGISTERS, 0x01, 0x11, 0x00, 0x00 \
	}
#define VFD_STOP_CMD                                                \
	{                                                               \
		6, 6, MODBUS_READ_HOLDING_REGISTERS, 0x01, 0x08, 0x00, 0x00 \
	}
#define VFD_IN_MULT g_settings.spindle_max_rpm
#define VFD_IN_DIV vfd_state.rpm_hz
#define VFD_OUT_MULT vfd_state.rpm_hz
#define VFD_OUT_DIV g_settings.spindle_max_rpm
```

Here is the example for the YL620 that uses 8 byte length commands

```
#if (VFD_CONTROLLER == VFD_YL620)
#define VFD_SETRPM_CMD                                              \
	{                                                               \
		8, 8, MODBUS_PRESET_SINGLE_REGISTER, 0x20, 0x01, 0x00, 0x00 \
	}
#define VFD_GETRPM_CMD                                              \
	{                                                               \
		8, 8, MODBUS_READ_HOLDING_REGISTERS, 0x20, 0x0B, 0x00, 0x01 \
	}
#define VFD_RPM_HZ_CMD                                           \
	{                                                            \
		0, MODBUS_READ_HOLDING_REGISTERS, 0xB0, 0x05, 0x00, 0x02 \
	}
#define VFD_CW_CMD                                                  \
	{                                                               \
		8, 8, MODBUS_PRESET_SINGLE_REGISTER, 0x20, 0x00, 0x00, 0x12 \
	}
#define VFD_CCW_CMD                                                 \
	{                                                               \
		8, 8, MODBUS_PRESET_SINGLE_REGISTER, 0x20, 0x00, 0x00, 0x22 \
	}
#define VFD_STOP_CMD                                                \
	{                                                               \
		8, 8, MODBUS_PRESET_SINGLE_REGISTER, 0x20, 0x00, 0x00, 0x01 \
	}
#define VFD_IN_MULT 60.0f
#define VFD_IN_DIV 10.0f
#define VFD_OUT_MULT 10.0f
#define VFD_OUT_DIV 60.0f
#endif
```

Each of these CMDs is then used internally to communicate with the VFD and read/write the VFD coils/registers.

## plasma_thc

This tool controls a plasma torch with automatic height control.
This tool also provides an additional MCode that allows to fully program the plasma startup motion pattern.
While cutting this tool is able to adjust the torch height in real time. By default these adjustments and the status of the plasma arc are controlled by some digital inputs pins (UP/DOWN/ARC_OK). This allows the tool to be controlled by an external module like the Proma THC 150.

But these inputs can be overriden to use any custom method to control these motions/signals, like for example use an analog input to read the torch/metal voltage difference and use a completelly standalone solution (similar to linuxCNC).

This tool is also designed to probe the initial height of the cut using the probe input. This can also be overwriten.

These are the default µCNC pins/configurations for this tool:

```
// if unset uses DIN15 by default has UP input signal control
#ifndef PLASMA_UP_INPUT
#define PLASMA_UP_INPUT DIN15
#endif

// if unset uses DIN14 by default has DOWN input signal control
#ifndef PLASMA_DOWN_INPUT
#define PLASMA_DOWN_INPUT DIN14
#endif

// if unset uses DIN13 by default has ARC OK input signal control
#ifndef PLASMA_ARC_OK_INPUT
#define PLASMA_ARC_OK_INPUT DIN13
#endif

// if unset uses DOUT0 by default has PLASMA ON output signal control
#ifndef PLASMA_ON_OUTPUT
#define PLASMA_ON_OUTPUT DOUT0
#endif

// if unset uses STEPPER2 (usually used in Z axis) as the THC linear actuator. If multiple STEPPERS are used for Z this should be changed
#ifndef PLASMA_STEPPERS_MASK
#define PLASMA_STEPPERS_MASK (1 << 2)
#endif

// if unset creates a virtual µCNC pin to be controlled be M62-M65 extension module to enable or disable THC mode (this requires module M62-M65)
#ifndef PLASMA_THC_ENABLE_PIN
#define PLASMA_THC_ENABLE_PIN 64
#endif
```

Additionally a MCode command M103 is available to control the probing and arc start motion using only M3/M4 S commands.
The command has these arguments

```
M103 I<probe depth in units> J<probe feed in units/s> R<probe retract heigth in units> K<probe depth in units> F<cut feed in units/s> D<Velocity Anti-Dive ration (0 to 100 (%))> P<cut dwell time in seconds> L<max number of arc start retries>
```

After these parameters are set THC must be enabled via M62(synched) or M64(immediately).

With THC enable the M3/M4 commands will do the following motions before igniting the torch.

  - check if ARC OK signal is off
  - turn the torch off
  - probes down until it hits the metal or fails if after the max probe distance to the metal is not found and retries
  - retracts up until it looses contact or fails if after the max probe distance to the metal is not found and retries
  - retracts (fast move) to the initial cut height
  - turns the torch on, waits the programmed time to form the puddle and read if the arc is ok
  - if arc is ok, travells at the cut feed to the final cut height

After this the motion cut executes. During this time the THC constantly monitors the arc ok signal and halts if it fails, and the up and down signals to see if it as to adjust the cutting height.

