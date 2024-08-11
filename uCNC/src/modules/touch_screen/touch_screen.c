
#include "../../cnc.h"
#include "touch_screen.h"
#include <stdint.h>
#include <stdbool.h>
#include "../softspi.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 *
 * can also be done via hardware SPI and I2C ports of µCNC
 * but is not needed
 *
 * */
#if (TOUCH_SCREEN_INTERFACE == TOUCH_SCREEN_SW_SPI)
// temporary result of reading non existing read pin
#define io0_get_input 0
#define io0_config_input
SOFTSPI(touch_spi, 1000000UL, 0, TOUCH_SCREEN_SPI_MOSI, TOUCH_SCREEN_SPI_MISO, TOUCH_SCREEN_SPI_CLOCK)
// delete temporary definition
#undef io0_get_input
#undef io0_config_input
#define TOUCH_SCREEN_BUS_LOCK LISTENER_SWSPI_LOCK
#elif (TOUCH_SCREEN_INTERFACE == TOUCH_SCREEN_HW_SPI)
HARDSPI(touch_spi, TOUCH_SCREEN_SPI_FREQ, 0, mcu_spi_port);
#define TOUCH_SCREEN_BUS_LOCK LISTENER_HWSPI_LOCK
#elif (TOUCH_SCREEN_INTERFACE == TOUCH_SCREEN_HW_SPI2)
HARDSPI(touch_spi, TOUCH_SCREEN_SPI_FREQ, 0, mcu_spi2_port);
#define TOUCH_SCREEN_BUS_LOCK LISTENER_HWSPI2_LOCK
#endif

	// Global variables
	static uint16_t touch_screen_width;
	static uint16_t touch_screen_height;
	static int16_t touch_screen_xdiff;
	static int16_t touch_screen_ydiff;
	static int16_t touch_screen_adc_xmin;
	static int16_t touch_screen_adc_ymin;
	static float touch_screen_adc_xdiff;
	static float touch_screen_adc_ydiff;

	void touch_screen_init(uint16_t width, uint16_t height)
	{
		spi_config_t conf = {0};
		softspi_config(&touch_spi, conf, TOUCH_SCREEN_SPI_FREQ);
		touch_screen_width = width;
		touch_screen_height = height;
#if (TOUCH_SCREEN_MARGIN != 0)
		float wratio = (float)TOUCH_SCREEN_ADC_MAX / (float)width;
		float hratio = (float)TOUCH_SCREEN_ADC_MAX / (float)height;
		float min = (float)TOUCH_SCREEN_MARGIN;
		float wmax = (float)(width - TOUCH_SCREEN_MARGIN);
		float hmax = (float)(height - TOUCH_SCREEN_MARGIN);
		touch_screen_set_calibration((min * wratio), (min * hratio), (wmax * hratio), (wmax * hratio));
#else
	touch_screen_set_calibration(0, 0, TOUCH_SCREEN_ADC_MAX, TOUCH_SCREEN_ADC_MAX);
#endif

		io_config_output(TOUCH_SCREEN_SPI_CS);
		softspi_start(&touch_spi);
		io_clear_output(TOUCH_SCREEN_SPI_CS);
		softspi_xmit(&touch_spi, (TOUCH_SCREEN_READ_Y | TOUCH_SCREEN_SER_MODE));
		softspi_xmit16(&touch_spi, 0);
		io_set_output(TOUCH_SCREEN_SPI_CS);
		softspi_stop(&touch_spi);
	}

	void touch_screen_get_calibration(uint16_t *x1, uint16_t *y1, uint16_t *x2, uint16_t *y2)
	{
		*x1 = *y1 = TOUCH_SCREEN_MARGIN;
		*x2 = touch_screen_width - TOUCH_SCREEN_MARGIN;
		*y2 = touch_screen_height - TOUCH_SCREEN_MARGIN;
	}

	void touch_screen_set_calibration(float adc_x1, float adc_y1, float adc_x2, float adc_y2)
	{

		touch_screen_xdiff = touch_screen_width - 2 * TOUCH_SCREEN_MARGIN;
		touch_screen_ydiff = touch_screen_height - 2 * TOUCH_SCREEN_MARGIN;
#if (TOUCH_SCREEN_MARGIN != 0)
		touch_screen_adc_xmin = (int16_t)MIN(adc_x1, adc_x2);
		touch_screen_adc_ymin = (int16_t)MIN(adc_y1, adc_y2);
		touch_screen_adc_xdiff = ABS(adc_x2 - adc_x1);
		touch_screen_adc_ydiff = ABS(adc_y2 - adc_y1);
#else
	touch_screen_adc_xmin = 0;
	touch_screen_adc_ymin = 0;
	touch_screen_adc_xdiff = TOUCH_SCREEN_ADC_MAX;
	touch_screen_adc_ydiff = TOUCH_SCREEN_ADC_MAX;
#endif
	}

	static uint16_t FORCEINLINE touch_screen_read_loop(uint8_t ctrl, uint8_t max_samples)
	{
		uint16_t prev = 0xffff, cur = 0xffff;
		uint8_t i = 0;
		softspi_xmit(&touch_spi, ctrl);
		do
		{
			prev = cur;
#ifdef TOUCH_SWAP_ADC_BYTES
			cur = softspi_xmit(&touch_spi, 0);
			cur = (cur << 4) | (softspi_xmit(&touch_spi, ctrl) >> 4); // 16 clocks -> 12-bits (zero-padded at end)
#else
		cur = softspi_xmit(&touch_spi, 0) >> 4;
		cur |= (((uint16_t)softspi_xmit(&touch_spi, ctrl)) << 4); // 16 clocks -> 12-bits (zero-padded at end)
#endif
		} while ((prev != cur) && (++i < max_samples));
		return cur;
	}

	void touch_screen_get_adc(uint16_t *adc_x, uint16_t *adc_y, uint8_t max_samples)
	{
		// Implementation based on TI Technical Note http://www.ti.com/lit/an/sbaa036/sbaa036.pdf

		softspi_start(&touch_spi);
		io_clear_output(TOUCH_SCREEN_SPI_CS);
		*adc_x = touch_screen_read_loop((TOUCH_SCREEN_READ_X | TOUCH_SCREEN_DFR_MODE), max_samples);
		*adc_y = touch_screen_read_loop((TOUCH_SCREEN_READ_Y | TOUCH_SCREEN_DFR_MODE), max_samples);
		// flush
		touch_screen_read_loop(TOUCH_SCREEN_READ_Y, 1);
		io_set_output(TOUCH_SCREEN_SPI_CS);
		softspi_stop(&touch_spi);
	}

	bool touch_screen_is_touching() { return (io_get_pinvalue(TOUCH_SCREEN_TOUCHED) < 1); }

	void touch_screen_get_position(uint16_t *x, uint16_t *y, uint8_t max_samples)
	{
		if (!touch_screen_is_touching())
		{
			*x = *y = 0xffff;
			return;
		}

		uint16_t adc_x, adc_y;
		touch_screen_get_adc(&adc_x, &adc_y, max_samples);

		// Map to (un-rotated) display coordinates
		*x = CALC_X(touch_screen_width, (uint16_t)(touch_screen_xdiff * (adc_x - touch_screen_adc_xmin) / touch_screen_adc_xdiff + TOUCH_SCREEN_MARGIN));
		*y = CALC_Y(touch_screen_height, (uint16_t)(touch_screen_ydiff * (adc_y - touch_screen_adc_ymin) / touch_screen_adc_ydiff + TOUCH_SCREEN_MARGIN));
	}

#ifdef __cplusplus
}
#endif