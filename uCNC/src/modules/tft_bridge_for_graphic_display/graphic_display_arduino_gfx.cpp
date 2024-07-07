#include <Arduino.h>
#include <U8g2lib.h>
#include <Arduino_GFX_Library.h>
#include "Arduino_uCNC_SPI.h"

static Arduino_DataBus *bus;
static Arduino_GFX *gfx;
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
#include "FreeMono12pt7b.h"

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
		return 20;
		uint16_t w, h = font_height;
		if (!h)
		{
			int16_t x, y;
			gfx->setFont(&FreeMono12pt7b /*u8g2_font_9x15_tf*/);
			gfx->getTextBounds(__romstr__("Ig"), 1, 1, &x, &y, &w, &h, false);
			font_height = h;
		}
		return (int16_t)h;
	}

	int16_t gd_line_height(void)
	{
		return (gd_font_height() + 7);
	}

	void gd_init(display_driver_t *driver, void *port_interface)
	{
		io_config_output(GRAPHIC_DISPLAY_SPI_DC);
		io_config_output(GRAPHIC_DISPLAY_SPI_CS);
		io_config_output(GRAPHIC_DISPLAY_BKL);
		// bus = new Arduino_uCNC_SPI(port_interface, GRAPHIC_DISPLAY_SPI_DC, GRAPHIC_DISPLAY_SPI_CS, false);
		bus = new Arduino_ESP32SPIDMA(17, 15, 18, 23, 19, VSPI, false);
		// bus = new Arduino_HWSPI(50, 53);

		driver->init();
		io_set_pinvalue(GRAPHIC_DISPLAY_BKL, 0);
		cnc_delay_ms(50);
		io_set_pinvalue(GRAPHIC_DISPLAY_BKL, 1);
		io_set_output(GRAPHIC_DISPLAY_RST);
		cnc_delay_ms(200);
		io_clear_output(GRAPHIC_DISPLAY_RST);
		cnc_delay_ms(200);
		io_set_output(GRAPHIC_DISPLAY_RST);
		gfx->begin(GRAPHIC_DISPLAY_SPI_FREQ);
		cnc_delay_ms(100);
		graphic_last_line_offset = -1;
	}

	void gd_clear()
	{
		gfx->startWrite();
		gfx->writeFillRect(0, 0, gfx->width(), gfx->height(), BLACK);
		// gfx->fillScreen(BLACK);
	}

	void gd_flush()
	{
		// #ifdef GRAPHIC_DISPLAY_IS_TOUCH
		// draws menu for touch
		gd_draw_button(0, gfx->height() - (gd_line_height() * 3), "Down", gfx->width() / 3, gd_line_height() * 3, false, false);
		gd_draw_button(gfx->width() / 3, gfx->height() - (gd_line_height() * 3), "Up", gfx->width() / 3, gd_line_height() * 3, false, false);
		gd_draw_button(2 * gfx->width() / 3, gfx->height() - (gd_line_height() * 3), "Enter", gfx->width() / 3, gd_line_height() * 3, false, false);
		// #endif
		gfx->flush();
		// g->drawBitmap(0,0,gfx->getFramebuffer(), gfx->width(), gfx->height(), WHITE, BLACK);
	}

	void gd_draw_startup(void)
	{
		char buff[SYSTEM_MENU_MAX_STR_LEN];
		memset(buff, 0, sizeof(buff));
		gfx->setTextColor(WHITE);
		gfx->setFont(&FreeMono12pt7b /*u8g2_font_9x15_tr*/);
		gfx->setTextSize(2);
		rom_strcpy(buff, __romstr__("uCNC"));
		gfx->fillScreen(BLACK);
		gd_draw_string(gd_str_align_center(buff), ((gfx->height() >> 1) - gd_font_height()), buff);
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

	void gd_draw_button(int16_t x0, int16_t y0, const char *s, int16_t minw, int16_t minh, bool invert, bool frameless)
	{
		int16_t txt_color = WHITE;
		int16_t bg_color = BLACK;
		if (invert)
		{
			txt_color = BLACK;
			bg_color = WHITE;
		}

		if (minw < 0)
		{
			minw = ABS(minw);
			x0 -= MAX(gd_str_width(s) + 7, minw);
		}

		if (minh < 0)
		{
			minh = ABS(minh);
			y0 -= MAX(minh, gd_line_height());
		}

		int16_t w = MAX(minw, gd_str_width(s) + 7);
		int16_t h = MAX(minh, gd_line_height());
		gfx->fillRect(x0, y0, w, h, bg_color);
		if (!frameless)
		{
			gfx->drawRect(x0, y0, w, h, txt_color);
		}
		gd_draw_string_inv(x0 + 3, y0 + 2, s, invert);
	}

	int16_t gd_str_width(const char *s)
	{
		uint16_t w, h;
		int16_t x, y;
		gfx->getTextBounds(s, 1, 1, &x, &y, &w, &h, false);
		return (int16_t)w;
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
		return (line - 1) * lh + offset;
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
		gfx = new Arduino_ST7796(bus, -1, 1, false);
		// gfx = new Arduino_Canvas(g->width(), g->height(), g);
#ifdef GRAPHIC_DISPLAY_IS_TOUCH
		xpt2046_init(420, 380, XPT2046_ROT0, 1000000UL, 0);
#endif
	}

	DISPLAY_INIT(ili9341_240x320_spi)
	{
		gfx = new Arduino_ILI9341(bus, -1, 0, false);
		// gfx = new Arduino_Canvas(g->width(), g->height(), g);
#ifdef GRAPHIC_DISPLAY_IS_TOUCH
		xpt2046_init(240, 320, XPT2046_ROT0, 1000000UL, 0);
#endif
	}

	DECL_DISPLAY(ili9341_240x320_spi, 240, 320);

#ifdef GRAPHIC_DISPLAY_IS_TOUCH
	// overrides the controller o read the touch
	bool graphic_display_rotary_encoder_control_sample(void *args)
	{
		uint16_t x, y;
		xpt2046_get_position(&x, &y);
		if (y > (gfx->height() - (gd_line_height() * 3)) && y < gfx->height())
		{
			if (x > 0 && x < (gfx->width() / 3))
			{
				graphic_display_rotary_encoder_counter--;
			}
			if (x > (gfx->width() / 3) && x < (2 * (gfx->width() / 3)))
			{
				graphic_display_rotary_encoder_counter++;
			}
			if (x > (2 * (gfx->width() / 3)) && x < gfx->width())
			{
				graphic_display_rotary_encoder_pressed = 1;
			}
		}
	}
#endif

#ifdef __cplusplus
}
#endif

extern "C" DECL_DISPLAY(st7796_480x320_spi, 420, 380);