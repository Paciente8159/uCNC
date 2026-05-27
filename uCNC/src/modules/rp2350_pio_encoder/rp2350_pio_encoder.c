/*
	Name: rp2350_pio_encoder.c
	Description: RP2040/RP2350 PIO hardware-assisted encoder backend for uCNC.
*/

#include "../../cnc.h"
#include "../encoder.h"

#if (ENCODERS > 0) && (ENC0_TYPE == ENC_TYPE_CUSTOM) && defined(ENC0_USE_HARDWARE_COUNTER) && (ENC0_USE_HARDWARE_COUNTER) && (ENC0_HW_COUNTER_TYPE == ENCODER_HW_PIO) && (MCU == MCU_RP2040 || MCU == MCU_RP2350 || defined(PICO_RP2040) || defined(PICO_RP2350))

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"

#ifndef ENC0_PULSE_GPIO
#error "ENC0_PULSE_GPIO must be defined for RP PIO encoder"
#endif

#ifndef ENC0_PIO_INDEX
#define ENC0_PIO_INDEX 0
#endif

#ifndef ENC0_PIO_SM
#define ENC0_PIO_SM 0
#endif

#ifndef ENC0_PIO_PROGRAM_OFFSET
#define ENC0_PIO_PROGRAM_OFFSET 0
#endif

#ifndef ENC0_MAX_STEP_RATE
#define ENC0_MAX_STEP_RATE 0
#endif

static bool rp_pio_encoder_ready;
static PIO rp_pio_encoder_pio;

static const uint16_t quadrature_encoder_program_instructions[] = {
	0x000f, 0x000e, 0x0015, 0x000f,
	0x0015, 0x000f, 0x000f, 0x000e,
	0x000e, 0x000f, 0x000f, 0x0015,
	0x000f, 0x0015,
	0x008f,
	0xa0c2,
	0x8000,
	0x60c2,
	0x4002,
	0xa0e6,
	0xa0a6,
	0xa04a,
	0x0097,
	0xa04a,
};

static const struct pio_program quadrature_encoder_program = {
	.instructions = quadrature_encoder_program_instructions,
	.length = 24,
	.origin = ENC0_PIO_PROGRAM_OFFSET,
};

static PIO enc0_get_pio(void)
{
#if ENC0_PIO_INDEX == 1
	return pio1;
#else
	return pio0;
#endif
}

static void quadrature_encoder_program_init_inline(PIO pio, uint sm, uint pin, int max_step_rate)
{
	pio_sm_config c;

	pio_sm_set_consecutive_pindirs(pio, sm, pin, 2, false);
	pio_gpio_init(pio, pin);
	pio_gpio_init(pio, pin + 1);
	gpio_pull_up(pin);
	gpio_pull_up(pin + 1);

	c = pio_get_default_sm_config();
	sm_config_set_wrap(&c, 15, 23);
	sm_config_set_in_pins(&c, pin);
	sm_config_set_jmp_pin(&c, pin);
	sm_config_set_in_shift(&c, false, false, 32);
	sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_NONE);

	if (max_step_rate == 0)
	{
		sm_config_set_clkdiv(&c, 1.0f);
	}
	else
	{
		sm_config_set_clkdiv(&c, (float)clock_get_hz(clk_sys) / (10.0f * (float)max_step_rate));
	}

	pio_sm_init(pio, sm, ENC0_PIO_PROGRAM_OFFSET, &c);
	pio_sm_set_enabled(pio, sm, true);
}

static int32_t quadrature_encoder_get_count_inline(PIO pio, uint sm)
{
	uint32_t ret = 0;
	int n = pio_sm_get_rx_fifo_level(pio, sm) + 1;
	while (n-- > 0)
	{
		ret = pio_sm_get_blocking(pio, sm);
	}
	return (int32_t)ret;
}

static void encoder_rp_pio_init(void)
{
	rp_pio_encoder_pio = enc0_get_pio();
	pio_add_program_at_offset(rp_pio_encoder_pio, &quadrature_encoder_program, ENC0_PIO_PROGRAM_OFFSET);
	quadrature_encoder_program_init_inline(rp_pio_encoder_pio, ENC0_PIO_SM, ENC0_PULSE_GPIO, ENC0_MAX_STEP_RATE);
}

static int32_t read_encoder_rp_pio(void)
{
	if (!rp_pio_encoder_ready)
	{
		return 0;
	}
	return quadrature_encoder_get_count_inline(rp_pio_encoder_pio, ENC0_PIO_SM);
}

#if defined(ENC0_INDEX_GPIO) && !ENC0_VIRTUAL_INDEX_ONLY
static void enc0_index_gpio_isr(uint gpio, uint32_t events)
{
	(void)gpio;
	(void)events;
	encoder_record_index_reference(ENC0, read_encoder_rp_pio());
}

static void enc0_index_gpio_isr_init(void)
{
	gpio_init(ENC0_INDEX_GPIO);
	gpio_set_dir(ENC0_INDEX_GPIO, GPIO_IN);
	gpio_pull_up(ENC0_INDEX_GPIO);
	gpio_set_irq_enabled_with_callback(ENC0_INDEX_GPIO, GPIO_IRQ_EDGE_RISE, true, &enc0_index_gpio_isr);
}
#endif

int32_t enc_custom_read(uint8_t i)
{
	return (i == ENC0) ? read_encoder_rp_pio() : 0;
}

DECL_MODULE(rp2350_pio_encoder)
{
	if (rp_pio_encoder_ready)
	{
		return;
	}

	encoder_rp_pio_init();
#if defined(ENC0_INDEX_GPIO) && !ENC0_VIRTUAL_INDEX_ONLY
	enc0_index_gpio_isr_init();
#endif
	rp_pio_encoder_ready = true;
}

#else

DECL_MODULE(rp2350_pio_encoder)
{
}

#endif
