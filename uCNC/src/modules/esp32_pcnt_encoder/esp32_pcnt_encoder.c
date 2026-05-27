/*
	Name: esp32_pcnt_encoder.c
	Description: ESP32 PCNT hardware counter backend for uCNC encoders.
*/

#include "../../cnc.h"
#include "../encoder.h"

#if (ENCODERS > 0) && (ENC0_TYPE == ENC_TYPE_CUSTOM) && defined(ENC0_USE_HARDWARE_COUNTER) && (ENC0_USE_HARDWARE_COUNTER) && (ENC0_HW_COUNTER_TYPE == ENCODER_HW_PCNT) && (MCU == MCU_ESP32 || MCU == MCU_ESP32S3 || MCU == MCU_ESP32C3)

#include "driver/gpio.h"
#include "driver/pcnt.h"

#ifndef ENC0_PCNT_UNIT
#define ENC0_PCNT_UNIT PCNT_UNIT_0
#endif

#ifndef ENC0_PCNT_CHANNEL_A
#define ENC0_PCNT_CHANNEL_A PCNT_CHANNEL_0
#endif

#ifndef ENC0_PCNT_CHANNEL_B
#define ENC0_PCNT_CHANNEL_B PCNT_CHANNEL_1
#endif

#ifndef ENC0_PCNT_RECENTER_THRESHOLD
#define ENC0_PCNT_RECENTER_THRESHOLD 20000
#endif

#ifndef ENC0_PCNT_FILTER
#define ENC0_PCNT_FILTER 0
#endif

#ifndef ENC0_PULSE_GPIO
#error "ENC0_PULSE_GPIO must be defined for ESP32 PCNT encoder"
#endif

#ifndef ENC0_DIR_GPIO
#error "ENC0_DIR_GPIO must be defined for ESP32 PCNT encoder"
#endif

static bool esp32_pcnt_encoder_ready;
static int32_t esp32_pcnt_encoder_offset;

static void encoder_esp32_pcnt_init(void)
{
	const int gpio_a = ENC0_PULSE_GPIO;
	const int gpio_b = ENC0_DIR_GPIO;

	pcnt_config_t ch_a = {
		.pulse_gpio_num = gpio_a,
		.ctrl_gpio_num = gpio_b,
		.lctrl_mode = PCNT_MODE_REVERSE,
		.hctrl_mode = PCNT_MODE_KEEP,
		.pos_mode = PCNT_COUNT_INC,
		.neg_mode = PCNT_COUNT_DEC,
		.counter_h_lim = 32767,
		.counter_l_lim = -32768,
		.unit = (pcnt_unit_t)ENC0_PCNT_UNIT,
		.channel = (pcnt_channel_t)ENC0_PCNT_CHANNEL_A,
	};

	pcnt_config_t ch_b = {
		.pulse_gpio_num = gpio_b,
		.ctrl_gpio_num = gpio_a,
		.lctrl_mode = PCNT_MODE_KEEP,
		.hctrl_mode = PCNT_MODE_REVERSE,
		.pos_mode = PCNT_COUNT_INC,
		.neg_mode = PCNT_COUNT_DEC,
		.counter_h_lim = 32767,
		.counter_l_lim = -32768,
		.unit = (pcnt_unit_t)ENC0_PCNT_UNIT,
		.channel = (pcnt_channel_t)ENC0_PCNT_CHANNEL_B,
	};

	pcnt_unit_config(&ch_a);
	pcnt_unit_config(&ch_b);

#if ENC0_PCNT_FILTER
	pcnt_set_filter_value((pcnt_unit_t)ENC0_PCNT_UNIT, ENC0_PCNT_FILTER);
	pcnt_filter_enable((pcnt_unit_t)ENC0_PCNT_UNIT);
#else
	pcnt_filter_disable((pcnt_unit_t)ENC0_PCNT_UNIT);
#endif

	pcnt_counter_pause((pcnt_unit_t)ENC0_PCNT_UNIT);
	pcnt_counter_clear((pcnt_unit_t)ENC0_PCNT_UNIT);
	pcnt_counter_resume((pcnt_unit_t)ENC0_PCNT_UNIT);
}

static int32_t read_encoder_esp32_pcnt(void)
{
	int16_t value = 0;
	int32_t position;

	if (!esp32_pcnt_encoder_ready)
	{
		return 0;
	}

	pcnt_get_counter_value((pcnt_unit_t)ENC0_PCNT_UNIT, &value);
	position = esp32_pcnt_encoder_offset + (int32_t)value;

	if (value >= ENC0_PCNT_RECENTER_THRESHOLD || value <= -ENC0_PCNT_RECENTER_THRESHOLD)
	{
		pcnt_counter_pause((pcnt_unit_t)ENC0_PCNT_UNIT);
		pcnt_counter_clear((pcnt_unit_t)ENC0_PCNT_UNIT);
		pcnt_counter_resume((pcnt_unit_t)ENC0_PCNT_UNIT);
		esp32_pcnt_encoder_offset = position;
	}

	return position;
}

#if defined(ENC0_INDEX_GPIO) && !ENC0_VIRTUAL_INDEX_ONLY
static void IRAM_ATTR enc0_index_gpio_isr(void *arg)
{
	int16_t raw = 0;
	(void)arg;
	pcnt_get_counter_value((pcnt_unit_t)ENC0_PCNT_UNIT, &raw);
	encoder_record_index_reference(ENC0, esp32_pcnt_encoder_offset + (int32_t)raw);
}

static void enc0_index_gpio_isr_init(void)
{
	gpio_set_intr_type((gpio_num_t)ENC0_INDEX_GPIO, GPIO_INTR_POSEDGE);
	gpio_isr_handler_add((gpio_num_t)ENC0_INDEX_GPIO, enc0_index_gpio_isr, NULL);
}
#endif

int32_t enc_custom_read(uint8_t i)
{
	return (i == ENC0) ? read_encoder_esp32_pcnt() : 0;
}

DECL_MODULE(esp32_pcnt_encoder)
{
	if (esp32_pcnt_encoder_ready)
	{
		return;
	}

	encoder_esp32_pcnt_init();
#if defined(ENC0_INDEX_GPIO) && !ENC0_VIRTUAL_INDEX_ONLY
	gpio_install_isr_service(0);
	enc0_index_gpio_isr_init();
#endif
	esp32_pcnt_encoder_ready = true;
}

#else

DECL_MODULE(esp32_pcnt_encoder)
{
}

#endif
