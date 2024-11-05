#include "../../../cnc.h"

#ifdef MCU_VIRTUAL_WIN

int main(void)
{
	ucnc_init();
	for (;;)
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

/**
 *
 *
 * Interpolator timer emulation
 *
 *
 * **/

#ifndef ITP_SAMPLE_RATE
#define ITP_SAMPLE_RATE (F_STEP_MAX * 2)
#endif

#if defined(MCU_HAS_ONESHOT_TIMER)
static uint32_t virtual_oneshot_counter;
static uint32_t virtual_oneshot_reload;
static FORCEINLINE void mcu_gen_oneshot(void)
{
	if (virtual_oneshot_counter)
	{
		virtual_oneshot_counter--;
		if (!virtual_oneshot_counter)
		{
			if (mcu_timeout_cb)
			{
				mcu_timeout_cb();
			}
		}
	}
}
#endif

static volatile uint32_t mcu_itp_timer_reload;
static volatile bool mcu_itp_timer_running;
static FORCEINLINE void mcu_gen_step(void)
{
	static bool step_reset = true;
	static int32_t mcu_itp_timer_counter;

	// generate steps
	if (mcu_itp_timer_running)
	{
		// stream mode tick
		int32_t t = mcu_itp_timer_counter;
		bool reset = step_reset;
		t -= (int32_t)roundf(1000000.0f / (float)ITP_SAMPLE_RATE);
		if (t <= 0)
		{
			if (!reset)
			{
				mcu_step_cb();
			}
			else
			{
				mcu_step_reset_cb();
			}
			step_reset = !reset;
			mcu_itp_timer_counter = mcu_itp_timer_reload + t;
		}
		else
		{
			mcu_itp_timer_counter = t;
		}
	}
}

/**
 * convert step rate to clock cycles
 * */
void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller)
{
	frequency = CLAMP((float)F_STEP_MIN, frequency, (float)F_STEP_MAX);
	// up and down counter (generates half the step rate at each event)
	uint32_t totalticks = (uint32_t)((500000.0f) / frequency);
	*prescaller = 1;
	while (totalticks > 0xFFFF)
	{
		(*prescaller) <<= 1;
		totalticks >>= 1;
	}

	*ticks = (uint16_t)totalticks;
}

float mcu_clocks_to_freq(uint16_t ticks, uint16_t prescaller)
{
	uint32_t totalticks = (uint32_t)ticks * prescaller;
	return 500000.0f / ((float)totalticks);
}

/**
 * starts the timer interrupt that generates the step pulses for the interpolator
 * */

void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	if (!mcu_itp_timer_running)
	{
		mcu_itp_timer_reload = ticks * prescaller;
		mcu_itp_timer_running = true;
	}
	else
	{
		mcu_change_itp_isr(ticks, prescaller);
	}
}

/**
 * changes the step rate of the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	if (mcu_itp_timer_running)
	{
		mcu_itp_timer_reload = ticks * prescaller;
	}
	else
	{
		mcu_start_itp_isr(ticks, prescaller);
	}
}

/**
 * stops the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_stop_itp_isr(void)
{
	if (mcu_itp_timer_running)
	{
		mcu_itp_timer_running = false;
	}
}

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void timer_handler(int signum)
{
	printf("Timer expired!\n");
}

void mcu_init(void)
{
	mcu_io_init();

	// start timer with microsecond resolution
	struct sigaction sa;
	sa.sa_handler = timer_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGALRM, &sa, NULL);

	// Set up the timer to expire after 2 seconds
	struct itimerval timer;
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 1;
	timer.it_interval.tv_sec = 0; // Repeat timer every 2 seconds
	timer.it_interval.tv_usec = 1;

	// Start the timer
	setitimer(ITIMER_REAL, &timer, NULL);
}

#endif