#include "../../cnc.h"
#include "xpt2046_touch.h"

#ifdef __cplusplus
extern "C"
{
#endif

	static softspi_port_t *xpt2046_spi;
	static uint16_t xpt2046_w;
	static uint16_t xpt2046_h;
	static uint8_t xpt2046_params;
// calibration parameters
#ifndef XPT2046_SKIP_CALIBRATION
	static uint16_t xpt2046_origin_x;
	static uint16_t xpt2046_origin_y;
	static uint16_t xpt2046_max_x;
	static uint16_t xpt2046_max_y;
#endif

	// last coordinates
	static uint16_t xpt2046_x;
	static uint16_t xpt2046_y;

	static void xpt2046_get_touch_xy(uint16_t *x, uint16_t *y)
	{
		softspi_config(xpt2046_spi, 0, XPT2046_SPI_FREQ);
		io_clear_output(XPT2046_CS);
		uint16_t tmp;

		/// read x
		softspi_xmit(xpt2046_spi, 0xd0);
		softspi_xmit(xpt2046_spi, 0);
		softspi_xmit(xpt2046_spi, 0xd0);
		softspi_xmit(xpt2046_spi, 0);
		softspi_xmit(xpt2046_spi, 0xd0);
		tmp = softspi_xmit(xpt2046_spi, 0);
		tmp = tmp << 5;
		tmp |= 0x1f & (softspi_xmit(xpt2046_spi, 0x90) >> 3);
		*x = tmp;

		serial_print_str("RawX: ");
		serial_print_int(tmp);

		softspi_xmit(xpt2046_spi, 0);
		softspi_xmit(xpt2046_spi, 0x90);
		softspi_xmit(xpt2046_spi, 0);
		softspi_xmit(xpt2046_spi, 0x90);
		tmp = softspi_xmit(xpt2046_spi, 0); // Read first 8 bits
		tmp = tmp << 5;
		tmp |= 0x1f & (softspi_xmit(xpt2046_spi, 0) >> 3); // Read last 8 bits and start new XP conversion

		*y = tmp;

		serial_print_str("RawY: ");
		serial_print_int(tmp);

		io_set_output(XPT2046_CS);
	}

	static uint16_t xpt2046_get_touch_pressure(void)
	{
		softspi_config(xpt2046_spi, 0, XPT2046_SPI_FREQ);
		io_clear_output(XPT2046_CS);

		int16_t tz = 0xFFF;
		softspi_xmit(xpt2046_spi, 0xb0);
		tz += softspi_xmit16(xpt2046_spi, 0xc0) >> 3; // Read Z1 and start Z2 conversion
		tz -= softspi_xmit16(xpt2046_spi, 0x00) >> 3; // Read Z2
		serial_print_str("RawZ: ");
		serial_print_int(tz);
		io_set_output(XPT2046_CS);

		if (tz == 4095)
			tz = 0;

		return (uint16_t)tz;
	}

	static void xpt2046_translate_position(uint16_t *x, uint16_t *y)
	{
		uint16_t x_tmp = *x, y_tmp = *y, xx, yy;
		uint8_t params = xpt2046_params;

		if (!(params & XPT2046_PARAMS_ROTATE))
		{
#ifndef XPT2046_SKIP_CALIBRATION
			xx = (x_tmp - xpt2046_origin_x) * xpt2046_w / xpt2046_max_x;
			yy = (y_tmp - xpt2046_origin_y) * xpt2046_h / xpt2046_max_y;
#else
		xx = x_tmp;
		yy = y_tmp;
#endif
			if ((params & XPT2046_PARAMS_INVERT_X))
			{
				xx = xpt2046_w - xx;
			}
			if ((params & XPT2046_PARAMS_INVERT_Y))
			{
				yy = xpt2046_h - yy;
			}
		}
		else
		{
#ifndef XPT2046_SKIP_CALIBRATION
			xx = (y_tmp - xpt2046_origin_y) * xpt2046_h / xpt2046_max_y;
			yy = (x_tmp - xpt2046_origin_x) * xpt2046_w / xpt2046_max_x;
#else
		xx = y_tmp;
		yy = x_tmp;
#endif
			if ((params & XPT2046_PARAMS_INVERT_X))
			{
				xx = xpt2046_w - xx;
			}
			if ((params & XPT2046_PARAMS_INVERT_Y))
			{
				yy = xpt2046_h - yy;
			}
		}
		*x = xx;
		*y = yy;
	}

	static uint8_t xpt2046_get_touch(uint16_t *x, uint16_t *y, uint16_t threshold)
	{
		static uint8_t last_status;
		static uint16_t z2 = 1, x_tmp2, y_tmp2;
		uint16_t z, x_tmp, y_tmp;

		// Wait until pressure stops increasing to debounce pressure
		uint8_t status = last_status;
		switch (status)
		{
		case XPT2046_NO_TOUCH:
#ifndef XPT2046_USE_TOUCH_IRQ
			z = xpt2046_get_touch_pressure();
			if (z > z2)
			{
				z2 = z;
				return XPT2046_NO_TOUCH;
			}
			else
			{
				z2 = 1;
			}

			if (z <= threshold)
			{
				return XPT2046_NO_TOUCH;
			}
#else
serial_print_str("irq: ");
		serial_print_int(io_get_input(XPT2046_TOUCHED));
		if (io_get_input(XPT2046_TOUCHED))
		{
			return XPT2046_NO_TOUCH;
		}
#endif

			xpt2046_get_touch_xy(&x_tmp, &y_tmp);
			last_status = XPT2046_TOUCH_1ST_SAMPLE;
			x_tmp2 = x_tmp;
			y_tmp2 = y_tmp;
			// exit loop before making a second sample
			return XPT2046_TOUCH_1ST_SAMPLE;
		case XPT2046_TOUCH_1ST_SAMPLE:
// confirm it's still pressing
#ifndef XPT2046_USE_TOUCH_IRQ
			if (xpt2046_get_touch_pressure() <= threshold)
			{
				last_status = XPT2046_NO_TOUCH;
				return XPT2046_NO_TOUCH;
			}
#else
		if (io_get_input(XPT2046_TOUCHED))
		{
			return XPT2046_NO_TOUCH;
		}
#endif
			last_status = XPT2046_TOUCH_2ND_SAMPLE;
		case XPT2046_TOUCH_2ND_SAMPLE:
			// second sample
			xpt2046_get_touch_xy(&x_tmp, &y_tmp);
			last_status = XPT2046_TOUCH_1ST_SAMPLE;
			if (ABS(x_tmp2 - x_tmp) > XPT2046_MAX_ERROR)
			{
				last_status = XPT2046_NO_TOUCH;
				return XPT2046_NO_TOUCH;
			}

			if (ABS(x_tmp2 - y_tmp) > XPT2046_MAX_ERROR)
			{
				last_status = XPT2046_NO_TOUCH;
				return XPT2046_NO_TOUCH;
			}

			*x = x_tmp;
			*y = y_tmp;

			return XPT2046_TOUCH_2ND_SAMPLE;
		}
		return XPT2046_NO_TOUCH;
	}

	void xpt2046_init(uint16_t width, uint16_t height, uint8_t params, softspi_port_t *spi_bus, uint32_t spi_speed)
	{
		xpt2046_spi = spi_bus;
		xpt2046_w = width;
		xpt2046_h = height;
		xpt2046_params = params;
		softspi_config(spi_bus, 0, spi_speed);
		io_clear_output(XPT2046_CS);
	}

	void xpt2046_get_calibration_points(uint16_t *x1, uint16_t *y1, uint16_t *x2, uint16_t *y2) {}
	void xpt2046_set_calibration(uint16_t *x1, uint16_t *y1, uint16_t *x2, uint16_t *y2) {}

	bool xpt2046_get_position(uint16_t *x, uint16_t *y, uint16_t threshold)
	{
		uint16_t x_tmp, y_tmp;
		uint32_t press_timeout = 0;
		static uint8_t last_status = 0;

		if (threshold < 20)
		{
			threshold = 20;
		}
		// lower the threshold to detect release event more reliably
		if (press_timeout > mcu_millis())
		{
			threshold = 20;
		}

		uint8_t status = xpt2046_get_touch(&x_tmp, &y_tmp, threshold);
		while (status != XPT2046_NO_TOUCH)
		{
			status |= last_status;
			last_status = status;
			if (status != XPT2046_TOUCH_TOUCHED)
			{
				break;
			}

			press_timeout = mcu_millis() + 50;
			xpt2046_translate_position(&x_tmp, &y_tmp);
			if (x_tmp >= xpt2046_w || y_tmp >= xpt2046_h)
			{
				break;
			}

			xpt2046_x = x_tmp;
			xpt2046_y = y_tmp;
			*x = x_tmp;
			*y = y_tmp;
			return true;
		}

		press_timeout = 0;
		last_status = 0;
		return false;
	}

#ifdef __cplusplus
}
#endif