#include <Arduino.h>
#include <U8g2lib.h>
#include <Arduino_GFX_Library.h>
#include "Arduino_uCNC_SPI.h"

// #define GRAPHIC_DISPLAY_USE_CANVAS

static Arduino_DataBus *bus;
#ifdef GRAPHIC_DISPLAY_USE_CANVAS
#include "canvas/Arduino_Canvas_Mono.h"
static Arduino_GFX *gfx;
static Arduino_G *g;
#else
static Arduino_GFX *gfx;
#endif
static int16_t font_height;
static int8_t graphic_last_line_offset;

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../cnc.h"
#include "../graphic_display/graphic_display.h"
#include "../softspi.h"
#include "../softi2c.h"
#include "../system_menu.h"
	// #include "FreeMono12pt7b.h"

	// #define GRAPHIC_DISPLAY_RST 48

#ifndef GRAPHIC_DISPLAY_SPI_DC
#define GRAPHIC_DISPLAY_SPI_DC DOUT11
#endif
#ifndef GRAPHIC_DISPLAY_BKL
#define GRAPHIC_DISPLAY_BKL DOUT12
#endif
#ifndef GRAPHIC_DISPLAY_RST
#define GRAPHIC_DISPLAY_RST DOUT13
#endif

#ifndef GRAPHIC_DISPLAY_SPI_FREQ
#define GRAPHIC_DISPLAY_SPI_FREQ 20000000UL
#endif

	int16_t gd_font_height(void)
	{
		return 9;
		// uint16_t w, h = font_height;
		// if (!h)
		// {
		// 	int16_t x, y;
		// 	gfx->setFont(/*&FreeMono12pt7b */u8g2_font_9x15_tf);
		// 	gfx->getTextBounds(__romstr__("Ig"), 1, 1, &x, &y, &w, &h, false);
		// 	font_height = h;
		// }
		// return (int16_t)h;
	}

	int16_t gd_line_height(void)
	{
		return (gd_font_height() + 8);
	}

	void gd_init(display_driver_t *driver, void *port_interface)
	{
		io_config_output(GRAPHIC_DISPLAY_SPI_DC);
		io_config_output(GRAPHIC_DISPLAY_SPI_CS);
#if ASSERT_PIN(GRAPHIC_DISPLAY_BKL)
		io_config_output(GRAPHIC_DISPLAY_BKL);
#endif
		bus = new Arduino_uCNC_SPI(port_interface, GRAPHIC_DISPLAY_SPI_DC, GRAPHIC_DISPLAY_SPI_CS, false);
		// bus = new Arduino_ESP32SPIDMA(17, 15, 18, 23, 19, VSPI, false);
		// bus = new Arduino_HWSPI(50, 53);

		driver->init();
		io_set_pinvalue(GRAPHIC_DISPLAY_BKL, 0);
		cnc_delay_ms(50);
		io_set_pinvalue(GRAPHIC_DISPLAY_BKL, 1);
#if ASSERT_PIN(GRAPHIC_DISPLAY_RST)
		io_set_output(GRAPHIC_DISPLAY_RST);
		cnc_delay_ms(200);
		io_clear_output(GRAPHIC_DISPLAY_RST);
		cnc_delay_ms(200);
		io_set_output(GRAPHIC_DISPLAY_RST);
#endif
		gfx->begin(GRAPHIC_DISPLAY_SPI_FREQ);
		cnc_delay_ms(100);
		graphic_last_line_offset = -1;
	}

	void gd_clear()
	{
		gfx->fillScreen(BLACK);
	}

	void gd_flush()
	{
		// #ifdef GRAPHIC_DISPLAY_IS_TOUCH
		// draws menu for touch
		gd_draw_button(0, gfx->height() - (gd_line_height() * 3), "Down", gfx->width() / 3, gd_line_height() * 3, false, BUTTON_BOX, TEXT_CENTER_CENTER);
		gd_draw_button(gfx->width() / 3, gfx->height() - (gd_line_height() * 3), "Up", gfx->width() / 3, gd_line_height() * 3, false, BUTTON_BOX, TEXT_CENTER_CENTER);
		gd_draw_button(2 * gfx->width() / 3, gfx->height() - (gd_line_height() * 3), "Enter", gfx->width() / 3, gd_line_height() * 3, false, BUTTON_BOX, TEXT_CENTER_CENTER);
		// #endif
		gfx->flush();
	}

	void gd_draw_startup(void)
	{
		char buff[SYSTEM_MENU_MAX_STR_LEN];
		memset(buff, 0, sizeof(buff));
		gfx->setTextColor(WHITE);
		gfx->setFont(/*&FreeMono12pt7b */ u8g2_font_9x15_tf);
		gfx->setTextSize(2);
		rom_strcpy(buff, __romstr__("uCNC"));
		gfx->fillScreen(BLACK);
		gd_draw_string(((gfx->width() >> 1) - gd_str_width(buff)), ((gfx->height() >> 1) - gd_font_height()), buff);
		memset(buff, 0, sizeof(buff));
		gfx->setTextSize(1);
		rom_strcpy(buff, __romstr__(("v" CNC_VERSION)));
		gd_draw_string(gd_str_align_center(buff), (gfx->height() - 2 * gd_font_height()), buff);
	}

	void gd_draw_h_line(int16_t y0)
	{
		gfx->drawFastHLine(0, y0, gfx->width(), WHITE);
	}

	void gd_draw_rectangle(int16_t x0, int16_t y0, int16_t w, int16_t h)
	{
		gfx->drawRect(x0, y0, w, h, WHITE);
	}

	void gd_draw_rectangle_fill(int16_t x0, int16_t y0, int16_t w, int16_t h, bool invert)
	{
		int16_t color = WHITE;
		if (invert)
		{
			color = BLACK;
		}
		gfx->fillRect(x0, y0, w, h, color);
	}

	void gd_draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool invert)
	{
		int16_t color = WHITE;
		if (invert)
		{
			color = BLACK;
		}
		gfx->fillTriangle(x0, y0, x1, y1, x2, y2, color);
	}

	int16_t gd_half_padding()
	{
		return 3;
	}

	void gd_draw_string(int16_t x0, int16_t y0, const char *s)
	{
		gfx->startWrite();
		gfx->fillRect(x0, y0, gfx->width(), gd_line_height(), BLACK);
		gfx->setCursor(x0, y0 + 4 + gd_font_height());
		gfx->print(s);
	}

	void gd_draw_string_inv(int16_t x0, int16_t y0, const char *s, bool invert)
	{
		if (invert)
		{
			gfx->setTextColor(BLACK);
		}
		gfx->setCursor(x0, y0 + 2 + gd_font_height());
		gfx->print(s);
		if (invert)
		{
			gfx->setTextColor(WHITE);
		}
	}

	void gd_draw_button(int16_t x0, int16_t y0, const char *s, int16_t minw, int16_t minh, bool invert, uint8_t frametype, uint8_t text_pos)
	{
		int16_t txt_color = WHITE;
		int16_t bg_color = BLACK;
		int16_t len = gd_str_width(s) + 7;
		int16_t lh = gd_line_height();

		if (invert)
		{
			txt_color = BLACK;
			bg_color = WHITE;
		}

		if (minw < 0)
		{
			minw = ABS(minw);
			x0 -= MAX(len, minw);
		}

		if (minh < 0)
		{
			minh = ABS(minh);
			y0 -= MAX(minh, lh);
		}

		int16_t w = MAX(minw, len);
		int16_t h = MAX(minh, lh);

		gfx->fillRect(x0, y0, w, h, bg_color);
		if (frametype & BUTTON_HOR_BARS)
		{
			gfx->drawFastHLine(x0, y0, w, txt_color);
			gfx->drawFastHLine(x0, y0 + h, w, txt_color);
		}

		if (frametype & BUTTON_VER_BARS)
		{
			gfx->drawFastVLine(x0, y0, h, txt_color);
			gfx->drawFastVLine(x0 + w, y0, h, txt_color);
		}

		switch (text_pos)
		{
		case TEXT_TOP_LEFT:
			x0 += 2;
			y0 += 1;
			break;
		case TEXT_TOP_CENTER:
			x0 += ((w - len) >> 1) + 2;
			y0 += 1;
			break;
		case TEXT_TOP_RIGHT:
			x0 += w - len + 2;
			y0 += 1;
			break;
		case TEXT_CENTER_LEFT:
			x0 += 2;
			y0 += 1;
			break;
		case TEXT_CENTER_CENTER:
			x0 += ((w - len) >> 1) + 2;
			y0 += ((h - lh) >> 1) + 1;
			break;
		case TEXT_CENTER_RIGHT:
			x0 += w - len + 2;
			y0 += ((h - lh) >> 1) + 1;
			break;
		case TEXT_BOTTOM_LEFT:
			x0 += 2;
			y0 += h - lh + 1;
			break;
		case TEXT_BOTTOM_CENTER:
			x0 += ((w - len) >> 1) + 2;
			y0 += h - lh + 1;
			break;
		case TEXT_BOTTOM_RIGHT:
			x0 += w - len + 2;
			y0 += h - lh + 1;
			break;
		}

		gd_draw_string_inv(x0 + 3, y0 + 2, s, invert);
	}

	int16_t gd_str_width(const char *s)
	{
		return strlen(s) * 9;
		// uint16_t w, h;
		// int16_t x, y;
		// gfx->getTextBounds(s, 1, 1, &x, &y, &w, &h, false);
		// return (int16_t)w;
	}

	int16_t gd_str_align_start(const char *s)
	{
		return 0;
	}

	int16_t gd_str_align_center(const char *s)
	{
		return ((gfx->width() - gd_str_width(s)) >> 1);
	}

	int16_t gd_str_align_end(const char *s)
	{
		return (gfx->width() - gd_str_width(s) - 8);
	}

	int16_t gd_str_justify_start(void)
	{
		return 0;
	}

	int16_t gd_str_justify_center(void)
	{
		return (gfx->height() + gd_font_height()) / 2;
	}

	int16_t gd_get_line_top(int8_t line)
	{
		int16_t lh = gd_line_height();
		// line height plus padding
		int8_t offset = graphic_last_line_offset;
		if (offset < 0)
		{
			offset = gfx->height() - lh * floor(gfx->height() / lh);
			graphic_last_line_offset = offset;
		}
		return line * lh + offset;
	}

	uint8_t gd_display_max_lines(void)
	{
		return (uint8_t)floor((gfx->height() * 0.8f) / gd_line_height());
	}

	/**
	 * Create some U8G2 display drivers
	 */

	DISPLAY_INIT(st7796_480x320_spi)
	{
#ifdef GRAPHIC_DISPLAY_USE_CANVAS
		g = new Arduino_ST7796(bus, -1, 1, false);
		gfx = new Arduino_Canvas_Mono(420, 380, g);
#else
	gfx = new Arduino_ST7796(bus, -1, 1, false);
#endif
#ifdef GRAPHIC_DISPLAY_IS_TOUCH
		xpt2046_init(420, 380, XPT2046_ROT0, 1000000UL, 0);
#endif
	}

	DISPLAY_INIT(ili9341_240x320_spi)
	{
#ifdef GRAPHIC_DISPLAY_USE_CANVAS
		g = new Arduino_ILI9341(bus, -1, 0, false);
		gfx = new Arduino_Canvas_Mono(240, 320, g);
#else
	gfx = new Arduino_ILI9341(bus, -1, 0, false);
#endif
#ifdef GRAPHIC_DISPLAY_IS_TOUCH
		xpt2046_init(240, 320, XPT2046_ROT0, 1000000UL, 0);
#endif
	}

#ifdef GRAPHIC_DISPLAY_IS_TOUCH
	// overrides the controller o read the touch
	bool graphic_display_rotary_encoder_control_sample(void *args)
	{
		uint16_t x, y;
		static uint8_t last_pin_state = 0;
		uint8_t pin_state = 0;
		static uint32_t long_press_timeout = 0;
		// btn debounce
		static uint32_t short_press_timeout = 0;

		xpt2046_get_position(&x, &y);
		if (y > (gfx->height() - (gd_line_height() * 3)) && y < gfx->height())
		{
			if (x > 0 && x < (gfx->width() / 3))
			{
				// GRAPHIC_DISPLAY_PREV 4
				pin_state = 4;
				gd_draw_button(0, gfx->height() - (gd_line_height() * 3), "Down", gfx->width() / 3, gd_line_height() * 3, true, BUTTON_BOX, TEXT_CENTER_CENTER);
			}
			if (x > (gfx->width() / 3) && x < (2 * (gfx->width() / 3)))
			{
				// GRAPHIC_DISPLAY_NEXT 2
				pin_state = 2;
				gd_draw_button(gfx->width() / 3, gfx->height() - (gd_line_height() * 3), "Up", gfx->width() / 3, gd_line_height() * 3, true, BUTTON_BOX, TEXT_CENTER_CENTER);
			}
			if (x > (2 * (gfx->width() / 3)) && x < gfx->width())
			{
				// GRAPHIC_DISPLAY_SELECT 1
				pin_state = 1;
				gd_draw_button(2 * gfx->width() / 3, gfx->height() - (gd_line_height() * 3), "Enter", gfx->width() / 3, gd_line_height() * 3, true, BUTTON_BOX, TEXT_CENTER_CENTER);
			}
		}

		if ((pin_state & 1))
		{
			uint32_t long_press = long_press_timeout;
			if (long_press && long_press < mcu_millis())
			{
				// forces a soft reset
				cnc_call_rt_command(0x18);
				long_press_timeout = 0;
			}
		}
		else
		{
			// resets long press timer
			long_press_timeout = 0;
		}

		uint8_t pin_diff = last_pin_state ^ pin_state;
		if (pin_diff)
		{
			// if btn is pressed (1st transition)
			if ((pin_state & 1))
			{
				// set soft reset timeout (5s)
				long_press_timeout = mcu_millis() + 5000;
			}
			last_pin_state = pin_state;
			if (pin_diff & pin_state)
			{
				uint32_t short_press = short_press_timeout;
				if (short_press < mcu_millis())
				{
					short_press_timeout = mcu_millis() + GRAPHIC_DISPLAY_ENCODER_DEBOUNCE_MS;
					graphic_display_rotary_encoder_pressed++;
				}
			}
		}

		return EVENT_CONTINUE;
	}
#endif

#ifdef __cplusplus
}
#endif

extern "C" DECL_DISPLAY(st7796_480x320_spi, 420, 380);
extern "C" DECL_DISPLAY(ili9341_240x320_spi, 240, 320);
