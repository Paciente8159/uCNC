#include "../../../cnc.h"

#ifdef MCU_VIRTUAL_WIN

int main (void){
    ucnc_init();
    for(;;)
    {
        ucnc_run();
    }
}


	/**
	 *
	 *
	 *  IO simulation and handling via named pipe for external app
	 *
	 *
	 * */
	typedef struct virtual_map_t
	{
		uint32_t special_outputs;
		uint64_t outputs;
		uint8_t pwm[16];
		uint8_t servos[6];
		uint32_t special_inputs;
		uint64_t inputs;
		uint8_t analog[16];
	} VIRTUAL_MAP;

    static volatile VIRTUAL_MAP virtualmap;

    uint8_t mcu_get_pin_offset(uint8_t pin)
	{
		if (pin >= 1 && pin <= 24)
		{
			return pin - 1;
		}
		else if (pin >= DOUT_PINS_OFFSET && pin < (DOUT_PINS_OFFSET + 50))
		{
			return pin - DOUT_PINS_OFFSET;
		}
		if (pin >= 100 && pin <= 113)
		{
			return pin - 100;
		}
		else if (pin >= DIN_PINS_OFFSET && pin < (DIN_PINS_OFFSET + 50))
		{
			return pin - DIN_PINS_OFFSET;
		}

		return -1;
	}

	void mcu_config_input(uint8_t pin)
	{
	}

	void mcu_config_pwm(uint8_t pin, uint16_t freq)
	{
	}

	uint8_t mcu_get_input(uint8_t pin)
	{
		uint8_t offset = mcu_get_pin_offset(pin);
		if (offset > 49)
		{
			return 0;
		}
		if (pin >= DIN0)
		{
			return (virtualmap.inputs & (1 << offset)) ? 1 : 0;
		}
		else
		{
			return (virtualmap.special_inputs & (1 << offset)) ? 1 : 0;
		}

		return 0;
	}

	void mcu_config_output(uint8_t pin)
	{
	}

	/**
	 * gets the value of a digital output pin
	 * can be defined either as a function or a macro call
	 * */
	uint8_t mcu_get_output(uint8_t pin)
	{
		uint8_t offset = mcu_get_pin_offset(pin);
		if (offset >= 50)
		{
			return 0;
		}

		if (pin >= DOUT0)
		{
			return (virtualmap.outputs & (1 << offset)) ? 1 : 0;
		}
		else
		{
			return (virtualmap.special_outputs & (1 << offset)) ? 1 : 0;
		}

		return 0;
	}

	/**
	 * sets the value of a digital output pin to logical 1
	 * can be defined either as a function or a macro call
	 * */
	void mcu_set_output(uint8_t pin)
	{
		uint8_t offset = mcu_get_pin_offset(pin);
		if (offset >= 50)
		{
			return;
		}

		if (pin >= DOUT0)
		{
			virtualmap.outputs |= (1UL << offset);
		}
		else
		{
			virtualmap.special_outputs |= (1UL << offset);
		}
	}

	/**
	 * sets the value of a digital output pin to logical 0
	 * can be defined either as a function or a macro call
	 * */
	void mcu_clear_output(uint8_t pin)
	{
		uint8_t offset = mcu_get_pin_offset(pin);
		if (offset >= 50)
		{
			return;
		}

		if (pin >= DOUT0)
		{
			virtualmap.outputs &= ~(1UL << offset);
		}
		else
		{
			virtualmap.special_outputs &= ~(1UL << offset);
		}
	}

	/**
	 * toggles the value of a digital output pin
	 * can be defined either as a function or a macro call
	 * */
	void mcu_toggle_output(uint8_t pin)
	{
		uint8_t offset = mcu_get_pin_offset(pin);
		if (offset >= 50)
		{
			return;
		}

		if (pin >= DOUT0)
		{
			virtualmap.outputs ^= (1UL << offset);
		}
		else
		{
			virtualmap.special_outputs ^= (1UL << offset);
		}
	}

	uint16_t mcu_get_analog(uint8_t channel)
	{
		channel -= ANALOG0;
		return virtualmap.analog[channel];
	}

	// Outputs
	void mcu_set_pwm(uint8_t pwm, uint8_t value)
	{
		pwm -= PWM0;
		virtualmap.pwm[pwm] = value;
	}

	uint8_t mcu_get_pwm(uint8_t pwm)
	{
		pwm -= PWM0;
		return virtualmap.pwm[pwm];
	}

	void mcu_set_servo(uint8_t servo, uint8_t value)
	{
		servo -= SERVO0;
		virtualmap.servos[value] = value;
	}

	uint8_t mcu_get_servo(uint8_t servo)
	{
		servo -= SERVO0;
		return virtualmap.servos[servo];
	}

	void mcu_enable_probe_isr(void)
	{
	}
	void mcu_disable_probe_isr(void)
	{
	}

#endif