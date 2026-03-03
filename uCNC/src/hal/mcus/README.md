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
	 * [Custom MCU IO initialization and reset logic](#custom-mcu-io-initialization-and-reset-logic)
	 

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

**µCNC design and program flow:**
µCNC is designed to work with the current core execution flow:
   * µCNC makes no assumptions about the number of CPU cores it is running as long as memory is shared between all cores.
      * In the case multiple core architectures is up to implementation to ensure that the concurrency safeguard macros/methods are implemented.
	  * There are some degrees of freedom of what can be executed in diferent CPU's (for example running communications in a single core and µCNC main loop in the other)
   * µCNC does not make assumptions if the mcu callbacks are running inside ISR handlers, events or RTOS threads, as long as they fufill the timing requirements to produce the expected result.
   * µCNC expects mcu callback function to **not be reentrant**. Reentrancy will cause context loss and unpredictable results. That means the while executing a callback (example mcu_step_cb) this function does not get called again (with all the respective context swithing) while still being excuted. But no assumptions are made about ISR nesting and priority. That means that while running a callback (example mcu_step_cb) another callback can be called (example mcu_step_reset_cb pausing mcu_step_cb and then resuming after mcu_step_reset_cb finnishes).


**After this 4 steps are needed:**

   **1. Implement all functions defined in the mcu.h** 
   
   All functions defined by the [```mcu.h```](https://github.com/Paciente8159/uCNC/blob/master/uCNC/src/hal/mcus/mcu.h) must be implemented.
   You can see this [file here](https://github.com/Paciente8159/uCNC/blob/master/uCNC/src/hal/mcus/mcu.h)

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

### Custom MCU IO initialization and reset logic

Custom MCU initialization and reset logic can be done via override of the `void __attribute__((weak)) mcu_io_reset(void)` function.
Any code you need to run after powerup and after the MCU initialization and before all sub-systems are initialized.
Here is an example of how to implement internal weak pulldown resistors for the limit switches on the STM32F4 MCU. By adding a .c file the root of the project and adding this code that can be done:

```
#include "src/cnc.h"

#if (MCU == MCU_STM32F4X)

#define GPIO_IN_PULLDOWN 0x02
#define mcu_config_pulldown(diopin)                                                                \
	{                                                                                              \
		__indirect__(diopin, GPIO)->PUPDR &= ~(GPIO_RESET << ((__indirect__(diopin, BIT)) << 1));    \
		__indirect__(diopin, GPIO)->PUPDR |= (GPIO_IN_PULLDOWN << ((__indirect__(diopin, BIT)) << 1)); \
	}

void mcu_io_reset(void)
{
	mcu_config_pulldown(LIMIT_X);
	mcu_config_pulldown(LIMIT_Y);
	mcu_config_pulldown(LIMIT_Z);
}

#endif
```