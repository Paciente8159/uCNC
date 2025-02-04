<p align="center">
<img src="https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true">
</p>


# µCNC
µCNC - Universal CNC firmware for microcontrollers

_**Jump to section**_
* [µCNC HAL](#µcnc-hal)
* [The microcontroller HAL](#the-microcontroller-hal)
   * [The microcontroller IO internal logic](#the-microcontroller-io-internal-logic)
   * [Pin naming conventions](#pin-naming-conventions)
   * [How is a pin evaluated?](#how-is-a-pin-evaluated)
   * [Creating the HAL for a custom MCU](#creating-the-hal-for-a-custom-mcu)
   * [Implementation example for a custom MCU using ArduinoIDE](#implementation-example-for-a-custom-mcu-using-arduinoide)

# µCNC HAL (Hardware Abstraction Layer)
µCNC has several HAL dimensions/layers. The first and most important layer is the microcontroller HAL.
This HAL provides the abstraction layer needed to use all the core functionalities in any microcontroller/PC board.

## The microcontroller HAL
Starting version 1.8, this HAL is actually composed of three parts (instead of 2 like in the previous versions). The MCU, the board/kit HAL and the new IO HAL. 
   * The MCU defines all the basic functions required by the µCNC to write and read on and from the microcontroller I/O. These functions include for example how to set or clear an output pin or read from an input pin.
   * The board contains all the definitions needed to map the µCNC internal named pins to the microcontroller physical pins.
   * From v1.8.0 and newer a new IO HAL was introduced. This allow the precompiller to link at build time the correct I/O function (either direct MCU IO pin or using a supported extender like the IC74HC595).

The way this works is like this:

### The microcontroller IO internal logic   

Every pin inside µCNC has a friendly definition, an intermediate translation definition and an internal number definition.

### Pin naming conventions 
µCNC sets a list of names that are used by the core functions to tell the microcontroller what to do. These have friendly names to make them easy to understand.
These are the fixed names used internally:

#### Output pins - special
+ ```STEP#``` pin defines the step output pin that controls linear actuator driver.
   + ```STEP0 to STEP5``` are the output pins to control the step signal up to 6 independent drivers.
   + ```STEP6 and STEP7``` are the output pins used as shadow registers to drive dual drive linear actuators.
+ ```DIR#``` pin defines the dir output pin that controls the linear actuator driver.
   + ```DIR0 to DIR5``` are the output pins to control the direction signal up to 6 independent drivers
+ ```STEPPER#_ENABLE``` pin defines the enable output pin that controls the linear actuator driver.
   + ```STEPPER0_ENABLE to STEPPER5_ENABLE``` are the output pins to control the enable signal up to 6 independent drivers.
+ ```SERVO#``` pin defines the servo signal output pin that controls common servo motors (1-2ms tON with 20ms period).
   + ```SERVO0 to SERVO7``` are the the servo signal output pins with up to 8 independent servos.

#### Input pins - special
+ ```LIMIT_#``` pin defines the input pin that controls end-stop switch detection.
   + ```LIMIT_X```, ```LIMIT_Y```, ```LIMIT_Z```, ```LIMIT_A```, ```LIMIT_B```, ```LIMIT_C```, ```LIMIT_X2```, ```LIMIT_Y2``` and ```LIMIT_Z2```.
+ ```ESTOP```, ```SAFETY_DOOR```, ```FHOLD``` and ```CS_RES``` pin defines the input pins that controls user actions and safety features.
+ ```PROBE``` pin defines the input pin used for probing and tool length detection.

#### COM pins - special
+ ```TX``` pin defines the UART port tx pin.
+ ```RX``` pin defines the UART port rx.
+ ```TX2``` pin defines the UART2 port tx pin.
+ ```RX2``` pin defines the UART2 port rx.
+ ```USB_DP``` pin defines the USB D+ port pin.
+ ```USB_DM``` pin defines the USB D- port pin.
+ ```SPI_CLK``` pin defines the SPI clock port pin.
+ ```SPI_SDI``` pin defines the SPI data input (MISO) port pin.
+ ```SPI_SDO``` pin defines the SPI data output (MOSI) port pin.
+ ```SPI_CS``` pin defines the SPI chip select port pin.
+ ```I2C_CLK``` pin defines the I2S clock port pin.
+ ```I2C_DATA``` pin defines the I2C data port pin.
+ ```SPI2_CLK``` pin defines the SPI2 clock port pin.
+ ```SPI2_SDI``` pin defines the SPI2 data input (MISO) port pin.
+ ```SPI2_SDO``` pin defines the SPI2 data output (MOSI) port pin.
+ ```SPI2_CS``` pin defines the SPI2 chip select port pin.

#### Output pins - generic
+ ```PWM#``` pin defines a pwm output pin.
   + ```PWM0 to PWM15``` are the pwm output pins.
+ ```SERVO#``` pin defines a pwm output pin.
   + ```SERVO0 to SERVO5``` are the servo output pins.
+ ```DOUT#``` pin defines a generic output pin.
   + ```DOUT0 to DOUT49``` are the generic output pins.

#### Input pins - generic
+ ```ANALOG#``` pin defines an analog input pin.
   + ```ANALOG0 to ANALOG15``` are the analog input pins.
+ ```DIN#``` pin defines a generic input pin.
   + ```DIN0 to DIN49``` are the generic input pins. Pins ```DIN0 to DIN7``` can also be have ISR on change option enabled. In conjunction with ```DIN8 to DIN49``` they can form a pair for the encoder module.

These pins also obey a numbering system to make them transversal between boards and MCU as mapped in the table bellow:

| Pin value | Alias | Pin name |
| --- | --- | --- |
| 1 | DIO1 | STEP0 |
| 2 | DIO2 | STEP1 |
| 3 | DIO3 | STEP2 |
| 4 | DIO4 | STEP3 |
| 5 | DIO5 | STEP4 |
| 6 | DIO6 | STEP5 |
| 7 | DIO7 | STEP6 |
| 8 | DIO8 | STEP7 |
| 9 | DIO9 | DIR0 |
| 10 | DIO10 | DIR1 |
| 11 | DIO11 | DIR2 |
| 12 | DIO12 | DIR3 |
| 13 | DIO13 | DIR4 |
| 14 | DIO14 | DIR5 |
| 15 | DIO15 | DIR6 |
| 16 | DIO16 | DIR7 |
| 17 | DIO17 | STEP0_EN |
| 18 | DIO18 | STEP1_EN |
| 19 | DIO19 | STEP2_EN |
| 20 | DIO20 | STEP3_EN |
| 21 | DIO21 | STEP4_EN |
| 22 | DIO22 | STEP5_EN |
| 23 | DIO23 | STEP6_EN |
| 24 | DIO24 | STEP7_EN |
| 25 | DIO25 | PWM0 |
| 26 | DIO26 | PWM1 |
| 27 | DIO27 | PWM2 |
| 28 | DIO28 | PWM3 |
| 29 | DIO29 | PWM4 |
| 30 | DIO30 | PWM5 |
| 31 | DIO31 | PWM6 |
| 32 | DIO32 | PWM7 |
| 33 | DIO33 | PWM8 |
| 34 | DIO34 | PWM9 |
| 35 | DIO35 | PWM10 |
| 36 | DIO36 | PWM11 |
| 37 | DIO37 | PWM12 |
| 38 | DIO38 | PWM13 |
| 39 | DIO39 | PWM14 |
| 40 | DIO40 | PWM15 |
| 41 | DIO41 | SERVO0 |
| 42 | DIO42 | SERVO1 |
| 43 | DIO43 | SERVO2 |
| 44 | DIO44 | SERVO3 |
| 45 | DIO45 | SERVO4 |
| 46 | DIO46 | SERVO5 |
| 47 | DIO47 | DOUT0 |
| 48 | DIO48 | DOUT1 |
| 49 | DIO49 | DOUT2 |
| 50 | DIO50 | DOUT3 |
| 51 | DIO51 | DOUT4 |
| 52 | DIO52 | DOUT5 |
| 53 | DIO53 | DOUT6 |
| 54 | DIO54 | DOUT7 |
| 55 | DIO55 | DOUT8 |
| 56 | DIO56 | DOUT9 |
| 57 | DIO57 | DOUT10 |
| 58 | DIO58 | DOUT11 |
| 59 | DIO59 | DOUT12 |
| 60 | DIO60 | DOUT13 |
| 61 | DIO61 | DOUT14 |
| 62 | DIO62 | DOUT15 |
| 63 | DIO63 | DOUT16 |
| 64 | DIO64 | DOUT17 |
| 65 | DIO65 | DOUT18 |
| 66 | DIO66 | DOUT19 |
| 67 | DIO67 | DOUT20 |
| 68 | DIO68 | DOUT21 |
| 69 | DIO69 | DOUT22 |
| 70 | DIO70 | DOUT23 |
| 71 | DIO71 | DOUT24 |
| 72 | DIO72 | DOUT25 |
| 73 | DIO73 | DOUT26 |
| 74 | DIO74 | DOUT27 |
| 75 | DIO75 | DOUT28 |
| 76 | DIO76 | DOUT29 |
| 77 | DIO77 | DOUT30 |
| 78 | DIO78 | DOUT31 |
| 79 | DIO79 | DOUT32 |
| 80 | DIO80 | DOUT33 |
| 81 | DIO81 | DOUT34 |
| 82 | DIO82 | DOUT35 |
| 83 | DIO83 | DOUT36 |
| 84 | DIO84 | DOUT37 |
| 85 | DIO85 | DOUT38 |
| 86 | DIO86 | DOUT39 |
| 87 | DIO87 | DOUT40 |
| 88 | DIO88 | DOUT41 |
| 89 | DIO89 | DOUT42 |
| 90 | DIO90 | DOUT43 |
| 91 | DIO91 | DOUT44 |
| 92 | DIO92 | DOUT45 |
| 93 | DIO93 | DOUT46 |
| 94 | DIO94 | DOUT47 |
| 95 | DIO95 | DOUT48 |
| 96 | DIO96 | DOUT49 |
| 100 | DIO100 | LIMIT_X |
| 101 | DIO101 | LIMIT_Y |
| 102 | DIO102 | LIMIT_Z |
| 103 | DIO103 | LIMIT_X2 |
| 104 | DIO104 | LIMIT_Y2 |
| 105 | DIO105 | LIMIT_Z2 |
| 106 | DIO106 | LIMIT_A |
| 107 | DIO107 | LIMIT_B |
| 108 | DIO108 | LIMIT_C |
| 109 | DIO109 | PROBE |
| 110 | DIO110 | ESTOP |
| 111 | DIO111 | SAFETY_DOOR |
| 112 | DIO112 | FHOLD |
| 113 | DIO113 | CS_RES |
| 114 | DIO114 | ANALOG0 |
| 115 | DIO115 | ANALOG1 |
| 116 | DIO116 | ANALOG2 |
| 117 | DIO117 | ANALOG3 |
| 118 | DIO118 | ANALOG4 |
| 119 | DIO119 | ANALOG5 |
| 120 | DIO120 | ANALOG6 |
| 121 | DIO121 | ANALOG7 |
| 122 | DIO122 | ANALOG8 |
| 123 | DIO123 | ANALOG9 |
| 124 | DIO124 | ANALOG10 |
| 125 | DIO125 | ANALOG11 |
| 126 | DIO126 | ANALOG12 |
| 127 | DIO127 | ANALOG13 |
| 128 | DIO128 | ANALOG14 |
| 129 | DIO129 | ANALOG15 |
| 130 | DIO130 | DIN0 |
| 131 | DIO131 | DIN1 |
| 132 | DIO132 | DIN2 |
| 133 | DIO133 | DIN3 |
| 134 | DIO134 | DIN4 |
| 135 | DIO135 | DIN5 |
| 136 | DIO136 | DIN6 |
| 137 | DIO137 | DIN7 |
| 138 | DIO138 | DIN8 |
| 139 | DIO139 | DIN9 |
| 140 | DIO140 | DIN10 |
| 141 | DIO141 | DIN11 |
| 142 | DIO142 | DIN12 |
| 143 | DIO143 | DIN13 |
| 144 | DIO144 | DIN14 |
| 145 | DIO145 | DIN15 |
| 146 | DIO146 | DIN16 |
| 147 | DIO147 | DIN17 |
| 148 | DIO148 | DIN18 |
| 149 | DIO149 | DIN19 |
| 150 | DIO150 | DIN20 |
| 151 | DIO151 | DIN21 |
| 152 | DIO152 | DIN22 |
| 153 | DIO153 | DIN23 |
| 154 | DIO154 | DIN24 |
| 155 | DIO155 | DIN25 |
| 156 | DIO156 | DIN26 |
| 157 | DIO157 | DIN27 |
| 158 | DIO158 | DIN28 |
| 159 | DIO159 | DIN29 |
| 160 | DIO160 | DIN30 |
| 161 | DIO161 | DIN31 |
| 162 | DIO162 | DIN32 |
| 163 | DIO163 | DIN33 |
| 164 | DIO164 | DIN34 |
| 165 | DIO165 | DIN35 |
| 166 | DIO166 | DIN36 |
| 167 | DIO167 | DIN37 |
| 168 | DIO168 | DIN38 |
| 169 | DIO169 | DIN39 |
| 170 | DIO170 | DIN40 |
| 171 | DIO171 | DIN41 |
| 172 | DIO172 | DIN42 |
| 173 | DIO173 | DIN43 |
| 174 | DIO174 | DIN44 |
| 175 | DIO175 | DIN45 |
| 176 | DIO176 | DIN46 |
| 177 | DIO177 | DIN47 |
| 178 | DIO178 | DIN48 |
| 179 | DIO179 | DIN49 |
| 200 | DIO200 | TX |
| 201 | DIO201 | RX |
| 202 | DIO202 | USB_DM |
| 203 | DIO203 | USB_DP |
| 204 | DIO204 | SPI_CLK |
| 205 | DIO205 | SPI_SDI |
| 206 | DIO206 | SPI_SDO |
| 207 | DIO207 | SPI_CS |
| 208 | DIO208 | I2C_CLK |
| 209 | DIO209 | I2C_DATA |
| 210 | DIO210 | TX2 |
| 211 | DIO211 | RX2 |
| 212 | DIO212 | SPI2_CLK |
| 213 | DIO213 | SPI2_SDI |
| 214 | DIO214 | SPI2_SDO |
| 215 | DIO215 | SPI2_CS |

With the introduction of the new IO HAL `src/hal/io_hal.h` all these definitions/values must be interchangable and match the above table.
`src/hal/io_hal.h` and `src/core/io_control.h` will then provide a series of preprocessor macros that translate between a µCNC IO call and the actual MCU IO call, based on all the available information (the IO HAL, the boardmap and the MCU HAL).

This (I hope) will become clearer in the example at the end of this file.

### How is a pin evaluated?

As state before a pin must respect the constrains described in the previous table.
If an IO pin is defined (for example STEP0), then it can only assume 3 values:

It's an MCU IO pin then:
STEP0 must evaluate to value 1 and DIO1 must evaluate to 1 also

It's an extended IO pin then:
STEP0 must evaluate to value 1 and DIO1 must evaluate to -1

If it's and undefined pin then
STEP0 must evaluate to value 0 and DIO1 must evaluate to 0 also

Lets take and example:

When we want to set generic output pin 0 (friendly name DOUT0) pin high we call this:

`io_set_output(DOUT0);`

Internally µCNC starts to decode this by a series of translations performed by the preprocessor, that are performed this way
   * `io_set_output(DOUT0);` -> DOUT0 (friendly name) is converted to a µCNC internal pin value. In this case DOUT0 is number 47. If not defined all pins will return 0.
   * Next this number is prefixed with `DIO` and is converted to an intermediate translation name `DIO47` (or DIO0 if undefined).
   * DIO47 is evaluated also for it's value. If DIO47 is positive then it's an IO pin of the MCU. If is negative then it's an extended pin via a supported extender (currently only IC74HC595 or similar). In our example it will evaluate to 47 (and IO pin of the MCU).
   * `io_set_output(DOUT0);` will then go through several translations and will become `mcu_set_output(STEP0)`. This function or preprocessor macro is part of the mcu template that has to be created for each MCU and implements the way the it does certain tasks, like setting and output pin logic high for example.

This is kind of convoluted work ensures that the once the translation HAL between the MCU and µCNC is correctly created, every part of the core and most modules work as expected in your microcontroller.
A bit further down on this document an example to use the Arduino framework will be provided, to make this easier to understand.

### Creating the HAL for a custom MCU 
Before creating a custom HAL for a custom board/microcontroller the microcontroller must have the following hardware features available: 
   * At least 2 hardware timers (it might be possible with only a single timer but with limitations)
   * At least a communication hardware port
   * A non volatile memory if you need to store the configuration parameter (optional: is possible to work without this feature)
   * PWM hardware IO (optional: is possible to work without this feature)
   * Input pins with interrupt on change (the interrupt on change is optional: is possible to work without this feature by using only soft pooling but some features may not be available)
   * Input pins with ADC (optional: is possible to work without this feature but some feature may not be available)

**After this 4 steps are needed:**

   **1. Implement all functions defined in the mcu.h** 
   
   All functions defined by the ```muc.h``` must be implemented. These are: 

   ```
#ifndef MCU_CALLBACK
#define MCU_CALLBACK
#endif

#ifndef MCU_RX_CALLBACK
#define MCU_RX_CALLBACK MCU_CALLBACK
#endif

#ifndef MCU_IO_CALLBACK
#define MCU_IO_CALLBACK MCU_CALLBACK
#endif

#ifndef F_STEP_MAX
#define F_STEP_MAX 30000
#endif

#define STREAM_UART 1
#define STREAM_UART2 2
#define STREAM_USB 4
#define STREAM_WIFI 8
#define STREAM_BTH 16
#define STREAM_BOARDCAST 255

// defines special mcu to access flash strings and arrays
#ifndef __rom__
#define __rom__
#endif
#ifndef __romstr__
#define __romstr__
#endif
#ifndef __romarr__
#define __romarr__ const uint8_t
#endif
#ifndef rom_strptr
#define rom_strptr *
#endif
#ifndef rom_strcpy
#define rom_strcpy strcpy
#endif
#ifndef rom_strncpy
#define rom_strncpy strncpy
#endif
#ifndef rom_memcpy
#define rom_memcpy memcpy
#endif
#ifndef rom_read_byte
#define rom_read_byte *
#endif

	// the extern is not necessary
	// this explicit declaration just serves to reeinforce the idea that these callbacks are implemented on other µCNC core code translation units
	// these callbacks provide a transparent way for the mcu to call them when the ISR/IRQ is triggered

	MCU_CALLBACK void mcu_step_cb(void);
	MCU_CALLBACK void mcu_step_reset_cb(void);
	MCU_RX_CALLBACK bool mcu_com_rx_cb(uint8_t c);
	MCU_CALLBACK void mcu_rtc_cb(uint32_t millis);
	MCU_IO_CALLBACK void mcu_controls_changed_cb(void);
	MCU_IO_CALLBACK void mcu_limits_changed_cb(void);
	MCU_IO_CALLBACK void mcu_probe_changed_cb(void);
	MCU_IO_CALLBACK void mcu_inputs_changed_cb(void);

/*IO functions*/

/**
 * config a pin in input mode
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_config_input
	void mcu_config_input(uint8_t pin);
#endif

/**
 * config pullup
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_config_pullup
	void mcu_config_pullup(uint8_t pin);
#endif

/**
 * config a pin in output mode
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_config_output
	void mcu_config_output(uint8_t pin);
#endif

/**
 * get the value of a digital input pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_input
	uint8_t mcu_get_input(uint8_t pin);
#endif

/**
 * gets the value of a digital output pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_output
	uint8_t mcu_get_output(uint8_t pin);
#endif

/**
 * sets the value of a digital output pin to logical 1
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_set_output
	void mcu_set_output(uint8_t pin);
#endif

/**
 * sets the value of a digital output pin to logical 0
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_clear_output
	void mcu_clear_output(uint8_t pin);
#endif

/**
 * toggles the value of a digital output pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_toggle_output
	void mcu_toggle_output(uint8_t pin);
#endif

	/**
	 *
	 * This is used has by the generic mcu functions has generic (overridable) IO initializer
	 *
	 * */
	void mcu_io_init(void);

	/**
	 * This can be used to set the defaults state of IO pins on reset. (overridable)
	 * */
#ifndef mcu_io_reset
	void mcu_io_reset(void);
#endif

	/**
	 * initializes the mcu
	 * this function needs to:
	 *   - configure all IO pins (digital IO, PWM, Analog, etc...)
	 *   - configure all interrupts
	 *   - configure uart or usb
	 *   - start the internal RTC
	 * */
	void mcu_init(void);

/**
 * enables the pin probe mcu isr on change
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_enable_probe_isr
	void mcu_enable_probe_isr(void);
#endif

/**
 * disables the pin probe mcu isr on change
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_probe_isr
	void mcu_disable_probe_isr(void);
#endif

/**
 * gets the voltage value of a built-in ADC pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_analog
	uint16_t mcu_get_analog(uint8_t channel);
#endif

/**
 * configs the pwm pin and output frequency
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_config_pwm
	void mcu_config_pwm(uint8_t pin, uint16_t freq);
#endif

/**
 * sets the pwm value of a built-in pwm pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_set_pwm
	void mcu_set_pwm(uint8_t pwm, uint8_t value);
#endif

/**
 * gets the configured pwm value of a built-in pwm pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_pwm
	uint8_t mcu_get_pwm(uint8_t pwm);
#endif

/**
 * sets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_set_servo
	void mcu_set_servo(uint8_t servo, uint8_t value);
#endif

/**
 * gets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_servo
	uint8_t mcu_get_servo(uint8_t servo);
#endif

// ISR
/**
 * enables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_enable_global_isr
	void mcu_enable_global_isr(void);
#endif

/**
 * disables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_global_isr
	void mcu_disable_global_isr(void);
#endif

/**
 * gets global interrupts state on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_global_isr
	bool mcu_get_global_isr(void);
#endif

	// Step interpolator
	/**
	 * convert step rate/frequency to timer ticks and prescaller
	 * */
	void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller);

	/**
	 * convert timer ticks and prescaller to step rate/frequency
	 * */
	float mcu_clocks_to_freq(uint16_t ticks, uint16_t prescaller);

	/**
	 * starts the timer interrupt that generates the step pulses for the interpolator
	 * */
	void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller);

	/**
	 * changes the step rate of the timer interrupt that generates the step pulses for the interpolator
	 * */
	void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller);

	/**
	 * stops the timer interrupt that generates the step pulses for the interpolator
	 * */
	void mcu_stop_itp_isr(void);

/**
 * gets the MCU running time in milliseconds.
 * the time counting is controled by the internal RTC
 * */
#ifndef mcu_millis
	uint32_t mcu_millis(void);
#endif

/**
 * gets the MCU running time in microseconds.
 * the time counting is controled by the internal RTC
 * */
#ifndef mcu_micros
	uint32_t mcu_micros(void);
#endif

/**
 * gets the microsecond portion of the free RTC clock counter (from 0 to 1000).
 * this free runner is always running even during an ISR or atomic operation
 * */
#ifndef mcu_free_micros
	uint32_t mcu_free_micros(void);
#endif

#ifndef mcu_nop
#define mcu_nop() asm volatile("nop\n\t")
#endif

	void mcu_delay_loop(uint16_t loops);

#ifndef mcu_delay_cycles
// set per MCU
#ifndef MCU_CLOCKS_PER_CYCLE
#error "MCU_CLOCKS_PER_CYCLE not defined for this MCU"
#endif
#ifndef MCU_CYCLES_PER_LOOP_OVERHEAD
#error "MCU_CYCLES_PER_LOOP_OVERHEAD not defined for this MCU"
#endif
#ifndef MCU_CYCLES_PER_LOOP
#error "MCU_CYCLES_PER_LOOP not defined for this MCU"
#endif
#ifndef MCU_CYCLES_PER_LOOP_OVERHEAD
#error "MCU_CYCLES_PER_LOOP_OVERHEAD not defined for this MCU"
#endif

#define mcu_delay_cycles(X)                                                                                                               \
	{                                                                                                                                       \
		if (X > (MCU_CYCLES_PER_LOOP + MCU_CYCLES_PER_LOOP_OVERHEAD))                                                                         \
		{                                                                                                                                     \
			mcu_delay_loop((uint16_t)((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP));                                               \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 0)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 1)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 2)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 3)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 4)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 5)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 6)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 7)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 8)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 9)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 10) \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
		}                                                                                                                                     \
		else                                                                                                                                  \
		{                                                                                                                                     \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 0)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 1)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 2)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 3)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 4)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 5)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 6)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 7)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 8)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 9)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 10)                                                                   \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
		}                                                                                                                                     \
	}
#endif

#ifndef mcu_delay_100ns
#define mcu_delay_100ns() mcu_delay_cycles((F_CPU / MCU_CLOCKS_PER_CYCLE / 10000000UL))
#endif

/**
 * provides a delay in us (micro seconds)
 * the maximum allowed delay is 255 us
 * */
#ifndef mcu_delay_us
#define mcu_delay_us(X) mcu_delay_cycles(F_CPU / MCU_CLOCKS_PER_CYCLE / 1000000UL * X)
#endif

#ifdef MCU_HAS_ONESHOT_TIMER
	typedef void (*mcu_timeout_delgate)(void);
	extern MCU_CALLBACK mcu_timeout_delgate mcu_timeout_cb;
/**
 * configures a single shot timeout in us
 * */
#ifndef mcu_config_timeout
	void mcu_config_timeout(mcu_timeout_delgate fp, uint32_t timeout);
#endif

/**
 * starts the timeout. Once hit the the respective callback is called
 * */
#ifndef mcu_start_timeout
	void mcu_start_timeout();
#endif
#endif

	/**
	 * runs all internal tasks of the MCU.
	 * for the moment these are:
	 *   - if USB is enabled and MCU uses tinyUSB framework run tinyUSB tud_task
	 * */
	void mcu_dotasks(void);

	// Non volatile memory
	/**
	 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
	 * */
	uint8_t mcu_eeprom_getc(uint16_t address);

	/**
	 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
	 * */
	void mcu_eeprom_putc(uint16_t address, uint8_t value);

	/**
	 * flushes all recorded registers into the eeprom.
	 * */
	void mcu_eeprom_flush(void);

	typedef union
	{
		uint8_t flags;
		struct
		{
			uint8_t mode : 3;
			uint8_t : 1; // reserved for bit order
			uint8_t enable_dma : 1;
			uint8_t : 3; // reserved
		};
	} spi_config_t;

	// hardware port function calls
	typedef struct spi_port_
	{
		bool isbusy;
		void (*start)(spi_config_t, uint32_t);
		uint8_t (*xmit)(uint8_t);
		bool (*bulk_xmit)(const uint8_t *, uint8_t *, uint16_t);
		void (*stop)(void);
	} spi_port_t;

#ifdef MCU_HAS_SPI
#ifndef mcu_spi_xmit
	uint8_t mcu_spi_xmit(uint8_t data);
#endif

#ifndef mcu_spi_bulk_transfer
	bool mcu_spi_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len);
#endif

#ifndef mcu_spi_start
	void mcu_spi_start(spi_config_t config, uint32_t frequency);
#endif

#ifndef mcu_spi_stop
	void mcu_spi_stop(void);
#endif

#ifndef mcu_spi_config
	void mcu_spi_config(spi_config_t config, uint32_t frequency);
#endif

extern spi_port_t mcu_spi_port;
#endif

#ifdef MCU_HAS_I2C
#ifndef I2C_OK
#define I2C_OK 0
#endif
#ifndef I2C_NOTOK
#define I2C_NOTOK 1
#endif

#ifndef mcu_i2c_send
	// master sends command to slave
	uint8_t mcu_i2c_send(uint8_t address, uint8_t *data, uint8_t datalen, bool release, uint32_t ms_timeout);
#endif
#ifndef mcu_i2c_receive
	// master receive response from slave
	uint8_t mcu_i2c_receive(uint8_t address, uint8_t *data, uint8_t datalen, uint32_t ms_timeout);
#endif

#if defined(MCU_SUPPORTS_I2C_SLAVE) && (I2C_ADDRESS != 0)
#ifndef I2C_SLAVE_BUFFER_SIZE
#define I2C_SLAVE_BUFFER_SIZE 48
#endif
#ifndef mcu_i2c_slave_cb
	MCU_IO_CALLBACK void mcu_i2c_slave_cb(uint8_t *data, uint8_t *datalen);
#endif
#endif

#ifndef mcu_i2c_config
	void mcu_i2c_config(uint32_t frequency);
#endif

#endif

	/**
	 * sends a uint8_t either via uart (hardware, software USB CDC, Wifi or BT)
	 * can be defined either as a function or a macro call
	 * */

#ifdef MCU_HAS_USB
	uint8_t mcu_usb_getc(void);
	uint8_t mcu_usb_available(void);
	void mcu_usb_clear(void);
	void mcu_usb_putc(uint8_t c);
	void mcu_usb_flush(void);
#ifdef DETACH_USB_FROM_MAIN_PROTOCOL
	MCU_RX_CALLBACK void mcu_usb_rx_cb(uint8_t c);
#endif
#endif

#ifdef MCU_HAS_UART
	uint8_t mcu_uart_getc(void);
	uint8_t mcu_uart_available(void);
	void mcu_uart_clear(void);
	void mcu_uart_putc(uint8_t c);
	void mcu_uart_flush(void);
#ifdef DETACH_UART_FROM_MAIN_PROTOCOL
	MCU_RX_CALLBACK void mcu_uart_rx_cb(uint8_t c);
#endif
#endif

#ifdef MCU_HAS_UART2
	uint8_t mcu_uart2_getc(void);
	uint8_t mcu_uart2_available(void);
	void mcu_uart2_clear(void);
	void mcu_uart2_putc(uint8_t c);
	void mcu_uart2_flush(void);
#ifdef DETACH_UART2_FROM_MAIN_PROTOCOL
	MCU_RX_CALLBACK void mcu_uart2_rx_cb(uint8_t c);
#endif
#endif

#ifdef MCU_HAS_WIFI
	uint8_t mcu_wifi_getc(void);
	uint8_t mcu_wifi_available(void);
	void mcu_wifi_clear(void);
	void mcu_wifi_putc(uint8_t c);
	void mcu_wifi_flush(void);
#ifdef DETACH_WIFI_FROM_MAIN_PROTOCOL
	MCU_RX_CALLBACK void mcu_wifi_rx_cb(uint8_t c);
#endif
#endif

#ifdef MCU_HAS_BLUETOOTH
	uint8_t mcu_bt_getc(void);
	uint8_t mcu_bt_available(void);
	void mcu_bt_clear(void);
	void mcu_bt_putc(uint8_t c);
	void mcu_bt_flush(void);
#ifdef DETACH_BLUETOOTH_FROM_MAIN_PROTOCOL
	MCU_RX_CALLBACK void mcu_bt_rx_cb(uint8_t c);
#endif
#endif

#ifndef mcu_getc
#define mcu_getc (&mcu_uart_getc)
#endif
#ifndef mcu_available
#define mcu_available (&mcu_uart_available)
#endif
#ifndef mcu_clear
#define mcu_clear (&mcu_uart_clear)
#endif
#ifndef mcu_putc
#define mcu_putc (&mcu_uart_putc)
#endif
#ifndef mcu_flush
#define mcu_flush (&mcu_uart_flush)
#endif
   ```

Also internally **AT LEAST** these macros need to be defined

```
// needed by software delays
#ifndef MCU_CLOCKS_PER_CYCLE
#define MCU_CLOCKS_PER_CYCLE 1
#endif
#ifndef MCU_CYCLES_PER_LOOP
#define MCU_CYCLES_PER_LOOP 4
#endif
#ifndef MCU_CYCLES_PER_LOOP_OVERHEAD
#define MCU_CYCLES_PER_LOOP_OVERHEAD 11
#endif

/* OR IN ALTERNATIVE YOU CAN DEFINE A CUSTOM CYCLE DELAY FUNCTION LIKE FOR EXAMPLE USED IN ARM */

// needed by software delays
#ifndef MCU_CLOCKS_PER_CYCLE
#define MCU_CLOCKS_PER_CYCLE 1
#endif
#define mcu_delay_cycles(X)     \
	{                           \
		DWT->CYCCNT = 0;        \
		uint32_t t = X;         \
		while (t > DWT->CYCCNT) \
			;                   \
	}
```

  Internally the implementation of the MCU must:

    - use a interrupt timer to call `mcu_step_cb()` and `mcu_step_reset_cb()` alternately (evenly spaced). Both these ISR run once every step at a maximum rate (set by Grbl's setting `$0`)

    - use a 1ms interrupt timer or RTC to generate the running time and call `mcu_rtc_cb(uint32_t millis)`. `mcu_dotasks()` **MUST NOT BE CALLED HERE**

    - if a hardware communications has events or ISR, the ISR must call `mcu_com_rx_cb(uint8_t c)` with the received uint8_t, or if handled internally by library or RTOS send every received uint8_t in the buffer through `mcu_com_rx_cb(uint8_t c)`.

    - if interruptible inputs are used they appropriately call mcu_limits_changed_cb(), mcu_controls_changed_cb(), mcu_probe_changed_cb() and mcu_inputs_changed_cb()

   **2. Map all µCNC internal pin names to I/O pins**

   The HAL must know the I/O pin to modify when the core want's to modify STEP0 pin for example. Again...µCNC doesn't care how it's done as long has it does what it is asked...If the switch is flicked on the bulb should turn on... simple.

   **3. Add the new board and mcu libraries to µCNC**

   * The mcu to the `mcus.h` file and give it an ID. Add the needed libraries to load if the MCU is chosen in the `mcudefs.f` file.
   
   **4. Create the project and build**
   From this point on you just need to create a project to run the program. This can be either a `main` file and a `makefile` and build, or using Arduino IDE to compile the project (the appropriate core/board manager must also be installed).
   Then on main just call the two functions needed to run µCNC. A bare minimum main file should look like this:

```
#include "cnc.h"

void main(void)
{
	//initializes all systems
	cnc_init();

	for(;;)
	{
		cnc_run();
	}

}
```

Again the example bellow will (I hope) help to clarify this.

### Implementation example for a custom MCU using ArduinoIDE

Now for a practical example of how to implement all this.

Let's create a boarmap were we link the internal µCNC pins and then make the MCU execute the desired action.
This example will use Arduino IDE as a base, but you can use (and I recommend) using baremetal or as close to the MCU programming as possible for best performance.

In our boardmap we need to create a way to assign out pins. These names and definitions can be set freely. Accross all MCU it was conventioned to use a friendly_name_BIT and friendly_name_PORT, but this is not mandatory. For example in AVR to set up a pin we do it like this:

```
// assign DOUT0 to pin B4 of the MCU
#define DOUT0_BIT 4
#define DOUT0_PORT B
```

Let's also assume that on your boardmap you want to define pins using Arduino IDE pin numbers with the and you have DOUT0 pin defined like friendly_name_IDEPIN:

`#define DOUT0_IDEPIN 50 // DOUT0 is pin 50 of the IDE`

Assuming that this is an actual IO pin of the MCU we can tell the IO HAL that the pin exists like this:

```
// if DOUT0 IDE pin is defined then define DOUT0 and DIO47 to match the table
#if defined(DOUT0_IDEPIN)
#define DOUT0 47
#define DIO47 47
#define DIO47_IDEPIN DOUT0_IDEPIN // calling DOUT0_IDEPIN or DIO47_IDEPIN will be the same. this will prove usefull later
#endif
```

The IO HAL will try to set this pin by calling the `mcu.h` function `mcu_set_output(pin)`. We can define it like this:

```
// this not a performant way to do this but works as an example
void mcu_set_output(uint8_t pin){
	switch(pin){
		case DOUT0:
			digitalWrite(DOUT0_PIN);
			break;

		...other pins...
	}
}

```

There is a way to improve this and avoid going through a long switch/case statement for all pins. With a bit a preprocessor trickery we can make a direct call (that is why it's possible to define some calls as functions or macros).

Let's define a couple of macros to resolve the friendly name to out friendly_name_IDEPIN definition.

Here is a preprocessor trick. This macro takes 2 arguments, makes some replacements and concatenates the replacement results

```
// Indirect macro access
#ifndef __indirect__
#define __indirect__ex__(X, Y) DIO##X##_##Y
#define __indirect__(X, Y) __indirect__ex__(X, Y)
#endif
```

So if we call `__indirect__(DOUT0, IDEPIN)` it will first evaluate both parameters to resolve them. `DOUT0` will be replaced by 47 and IDEPIN (you cannot have a #define IDEPIN anywere in yor code or this will not work) will remain the same and will be passed to the next macro. Next `47` and `IDEPIN` wil be be concatenated to `DIO` and `_` and will become `DIO47_IDEPIN` that is equivalent to `DOUT0_IDEPIN`.

Resuming: `__indirect__(DOUT0, IDEPIN)` -> `__indirect__ex__(47, IDEPIN)` -> `DIO47_IDEPIN` -> `DOUT0_IDEPIN` -> `50`

We can now define a macro `mcu_set_output` that makes use of this replacement and converts to our Arduino call to `digitalWrite` like this:

```
#define mcu_set_output(X) digitalWrite(__indirect__(X, IDEPIN))
```

Again the preprocessor will convert `mcu_set_output(DOUT0)` to `digitalWrite(50)` like we need it. And this aplicable to all pins.

