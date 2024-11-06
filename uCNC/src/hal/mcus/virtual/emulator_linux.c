#include "../../../cnc.h"

#if (MCU == MCU_VIRTUAL_LINUX)

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
	 * ISR emulation
	 *
	 * **/
	volatile bool global_isr_enabled = false;
	// enables all interrupts on the mcu. Must be called to enable all IRS functions
	void mcu_enable_global_isr(void)
	{
		global_isr_enabled = true;
	}
	// disables all ISR functions
	void mcu_disable_global_isr(void)
	{
		global_isr_enabled = false;
	}

	bool mcu_get_global_isr(void)
	{
		return global_isr_enabled;
	}

    /**
 *
 *
 * Communications emulation
 * UART -> PC COM
 * UART2 -> console
 *
 * **/
#ifdef MCU_HAS_UART
	WindowsSerial Serial = WindowsSerial(UART_PORT_NAME);
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);

	uint8_t mcu_uart_getc(void)
	{
		uint8_t c = 0;
		BUFFER_DEQUEUE(uart_rx, &c);
		return c;
	}

	uint8_t mcu_uart_available(void)
	{
		return BUFFER_READ_AVAILABLE(uart_rx);
	}

	void mcu_uart_clear(void)
	{
		BUFFER_CLEAR(uart_rx);
	}

	void mcu_uart_putc(uint8_t c)
	{
		while (BUFFER_FULL(uart_tx))
		{
			mcu_uart_flush();
		}
		BUFFER_ENQUEUE(uart_tx, &c);
	}

	void mcu_uart_flush(void)
	{
		while (!BUFFER_EMPTY(uart_tx))
		{
			uint8_t tmp[UART_TX_BUFFER_SIZE + 1];
			memset(tmp, 0, sizeof(tmp));
			uint8_t r = 0;

			BUFFER_READ(uart_tx, tmp, UART_TX_BUFFER_SIZE, r);
			Serial.WriteData(tmp, r);
		}
	}

	void mcu_uart_process()
	{
		char buff[RX_BUFFER_SIZE];

		int count = Serial.ReadData(buff, RX_BUFFER_SIZE);
		for (int i = 0; i < count; i++)
		{
			uint8_t c = buff[i];
			if (mcu_com_rx_cb(c))
			{
				if (BUFFER_FULL(uart_rx))
				{
					c = OVF;
				}
				BUFFER_ENQUEUE(uart_rx, &c);
			}
		}
	}
#endif

#ifdef MCU_HAS_UART2
#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE 64
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#ifndef STDIN_FILENO
  #define STDIN_FILENO 0
#endif
#ifndef FIONREAD
  #define FIONREAD	0x541B
#endif

static struct termios old, current;

/* Initialize new terminal i/o settings */
void initTermios(int echo) 
{
  tcgetattr(0, &old); /* grab old terminal i/o settings */
  current = old; /* make new settings same as old settings */
  current.c_lflag &= ~ICANON; /* disable buffered i/o */
  current.c_iflag          = 0;       /* input mode                */
  current.c_oflag          = 0;       /* output mode               */
  current.c_cc[VMIN]       = CMIN;    /* minimum time to wait      */
  current.c_cc[VTIME]      = CTIME;   /* minimum characters to wait for */
  if (echo) {
      current.c_lflag |= ECHO; /* set echo mode */
  } else {
      current.c_lflag &= ~ECHO; /* set no echo mode */
  }
  tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo) 
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}

/* Read 1 character without echo */
char getch(void) 
{
  return getch_(0);
}

int kbhit(void)
{
  int cnt = 0;
  int error = -1;

  initTermios(0);
    error += ioctl(0, FIONREAD, &cnt);
    resetTermios();
 
  return ( error == 0 ? cnt : -1 );
}

	DECL_BUFFER(uint8_t, uart2_tx, UART2_TX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, uart2_rx, RX_BUFFER_SIZE);

	uint8_t mcu_uart2_getc(void)
	{
		uint8_t c = 0;
		BUFFER_DEQUEUE(uart2_rx, &c);
		return c;
	}

	uint8_t mcu_uart2_available(void)
	{
		return BUFFER_READ_AVAILABLE(uart2_rx);
	}

	void mcu_uart2_clear(void)
	{
		BUFFER_CLEAR(uart2_rx);
	}

	void mcu_uart2_putc(uint8_t c)
	{
		while (BUFFER_FULL(uart2_tx))
		{
			mcu_uart2_flush();
		}
		BUFFER_ENQUEUE(uart2_tx, &c);
	}

	void mcu_uart2_flush(void)
	{
		while (!BUFFER_EMPTY(uart2_tx))
		{
			uint8_t tmp[UART2_TX_BUFFER_SIZE + 1];
			memset(tmp, 0, sizeof(tmp));
			uint8_t r = 0;

			BUFFER_READ(uart2_tx, tmp, UART2_TX_BUFFER_SIZE, r);
			printf("%s", tmp);
		}
	}

	void mcu_uart2_process()
	{
		if (kbhit())
		{
			char c = getch();
			putchar(c);
			if (c == '\r')
			{
				putchar('\n');
			}
			if (mcu_com_rx_cb(c))
			{
				if (BUFFER_FULL(uart2_rx))
				{
					c = OVF;
				}
				BUFFER_ENQUEUE(uart2_rx, &c);
			}
		}
	}
#endif

	void mcu_dotasks()
	{
#ifdef MCU_HAS_UART
		mcu_uart_process();
#endif
#ifdef MCU_HAS_UART2
		mcu_uart2_process();
#endif
	}

	/**
	 *
	 *
	 * EEPROM emulation
	 * Uses a local file to store values
	 *
	 *
	 * **/
	uint8_t mcu_eeprom_getc(uint16_t address)
	{
		FILE *fp = fopen("virtualeeprom", "rb");
		uint8_t c = 0;

		if (fp != NULL)
		{
			if (!fseek(fp, address, SEEK_SET))
			{
				c = getc(fp);
			}

			fclose(fp);
		}

		return c;
	}

	void mcu_eeprom_putc(uint16_t address, uint8_t value)
	{
		FILE *src = fopen("virtualeeprom", "rb+");

		if (!src)
		{
			FILE *dest = fopen("virtualeeprom", "wb");
			fclose(dest);
			src = fopen("virtualeeprom", "rb+");
		}

		/*for(int i = 0; i < address; i++)
		{
			getc(src);
		}*/

		fseek(src, address, SEEK_SET);
		putc((int)value, src);

		fflush(src);
		fclose(src);
	}

	void mcu_eeprom_flush()
	{
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

uint32_t mcu_micros(void){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (uint32_t)(tv.tv_sec*(uint64_t)1000000+tv.tv_usec);
}

uint32_t mcu_millis(void){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (uint32_t)(tv.tv_sec*(uint64_t)1000+(uint64_t)(0.001f*tv.tv_usec));
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

void ticksimul(int signum)
	{
		//		long t = stopCycleCounter();
		//		printf("Elapsed %dus\n\r", (int)((double)t / cyclesPerMicrosecond));
		for (int i = 0; i < (int)ceil(20 * ITP_SAMPLE_RATE / 1000); i++)
		{
			mcu_gen_step();
		}

		mcu_rtc_cb(mcu_millis());
		//		startCycleCounter();
	}

void mcu_init(void)
{
	mcu_io_init();

	// start timer with microsecond resolution
	struct sigaction sa;
	sa.sa_handler = ticksimul;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGALRM, &sa, NULL);

	// Set up the timer to expire after 2 seconds
	struct itimerval timer;
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 1;
	timer.it_interval.tv_sec = 0; // Repeat timer every 2 seconds
	timer.it_interval.tv_usec = 1000;

	// Start the timer
	setitimer(ITIMER_REAL, &timer, NULL);
}

#endif