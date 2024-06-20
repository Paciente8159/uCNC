// #include <Arduino.h>
// #include <U8g2lib.h>
// #include <Arduino_GFX_Library.h>
// #include "Arduino_uCNC_SPI.h"

// static Arduino_DataBus *bus;
// static Arduino_GFX *gfx;
// static void *graphic_port;
// static int16_t font_height;
// static int8_t graphic_last_line_offset;

// #ifdef __cplusplus
// extern "C"
// {
// #endif

// #include "../../cnc.h"
// #include "graphic_display.h"
// #include "../softspi.h"
// #include "../softi2c.h"
// 	// #include "uCNC_logo.h"

// #define GRAPHIC_DISPLAY_RST 48

// #ifndef GRAPHIC_DISPLAY_SPI_DC
// #define GRAPHIC_DISPLAY_SPI_DC DOUT11
// #endif
// #ifndef GRAPHIC_DISPLAY_BKL
// #define GRAPHIC_DISPLAY_BKL DOUT12
// #endif
// #ifndef GRAPHIC_DISPLAY_RST
// #define GRAPHIC_DISPLAY_RST -1
// #endif

// 	int16_t gd_font_height(void)
// 	{
// 		uint16_t w, h = font_height;
// 		if (!h)
// 		{
// 			int x, y;
// 			gfx->setFont(u8g2_font_9x15_tf);
// 			gfx->getTextBounds("Íg", 1, 1, &x, &y, &w, &h);
// 			font_height = h;
// 		}
// 		return h;
// 	}

// 	int16_t gd_line_height(void)
// 	{
// 		return (gd_font_height() + 7);
// 	}

// 	void gd_init(display_driver_t *driver, void *port_interface)
// 	{
// 		graphic_port = port_interface;
// 		bus = new Arduino_uCNC_SPI((softspi_port_t *)graphic_port, GRAPHIC_DISPLAY_SPI_DC, GRAPHIC_DISPLAY_SPI_CS, false);
// 		// bus = new Arduino_ESP32SPI(33, 25, 18, 23, 19, VSPI,false);
// 		// bus = new Arduino_HWSPI(50, 53);

// 		driver->init();
// 		io_set_pinvalue(GRAPHIC_DISPLAY_BKL, 0);
// 		cnc_delay_ms(50);
// 		io_set_pinvalue(GRAPHIC_DISPLAY_BKL, 1);
// 		gfx->begin(40000000);
// 		cnc_delay_ms(100);
// 		gfx->fillScreen(BLACK);
// 		graphic_last_line_offset = -1;
// 	}

// 	void gd_clear()
// 	{
// 		gfx->fillScreen(BLACK);
// 	}

// 	void gd_flush()
// 	{
// 	}

// 	void gd_draw_startup(void)
// 	{
// 		// gfx->flush();
// 		// int16_t x = (display_w - uCNClogo.width) >> 1;
// 		// int16_t y = (display_h - uCNClogo.height) >> 1;
// 		// gfx->draw16bitRGBBitmap(x, y, uCNClogo.data, uCNClogo.width, uCNClogo.height);
// 		// gfx->flush();
// 		// u8g2_ClearBuffer(U8G2);
// 		// char buff[SYSTEM_MENU_MAX_STR_LEN];
// 		// rom_strcpy(buff, __romstr__("µCNC"));
// 		// u8g2_ClearBuffer(U8G2);
// 		// u8g2_SetFont(U8G2, u8g2_font_9x15_t_symbols);
// 		// u8g2_DrawUTF8X2(U8G2, (u8g2_GetDisplayWidth(U8G2) / 2 - u8g2_GetUTF8Width(U8G2, buff)), gd_str_justify_center(), buff);
// 		// rom_strcpy(buff, __romstr__(("v" CNC_VERSION)));
// 		// u8g2_SetFont(U8G2, u8g2_font_6x12_tr);
// 		// u8g2_DrawStr(U8G2, gd_str_align_center(buff), gd_str_justify_end(), buff);
// 		// u8g2_SendBuffer(U8G2);
// 		gfx->setTextColor(WHITE);
// 		gfx->setFont(u8g2_font_9x15_tf);
// 		gfx->println("Hello World!");
// 	}

// 	void gd_draw_h_line(int16_t y0)
// 	{
// 		gfx->drawFastHLine(0, y0, gfx->width(), WHITE);
// 	}

// 	void gd_draw_rectangle(int16_t x0, int16_t y0, int16_t w, int16_t h)
// 	{
// 		gfx->drawRect(x0, y0, w, h, WHITE);
// 	}

// 	void gd_draw_rectangle_fill(int16_t x0, int16_t y0, int16_t w, int16_t h, bool invert)
// 	{
// 		int16_t color = WHITE;
// 		if (invert)
// 		{
// 			color = BLACK;
// 		}
// 		gfx->fillRect(x0, y0, w, h, color);
// 	}

// 	void gd_draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool invert)
// 	{
// 		int16_t color = WHITE;
// 		if (invert)
// 		{
// 			color = BLACK;
// 		}
// 		gfx->fillTriangle(x0, y0, x1, y1, x2, y2, color);
// 	}

// 	int16_t gd_half_padding()
// 	{
// 		return 3;
// 	}

// 	void gd_draw_string(int16_t x0, int16_t y0, const char *s)
// 	{
// 		gfx->startWrite();
// 		gfx->fillRect(x0, y0, gfx->width(), gd_line_height(), BLACK);
// 		gfx->setCursor(x0, y0 + 4 + gd_font_height());
// 		gfx->print(s);
// 	}

// 	void gd_draw_string_inv(int16_t x0, int16_t y0, const char *s, bool invert)
// 	{
// 		if (invert)
// 		{
// 			gfx->setTextColor(BLACK);
// 		}
// 		gfx->setCursor(x0, y0 + 2 + gd_font_height());
// 		gfx->print(s);
// 		if (invert)
// 		{
// 			gfx->setTextColor(WHITE);
// 		}
// 	}

// 	void gd_draw_button(int16_t x0, int16_t y0, const char *s, int16_t minw, bool invert, bool frameless)
// 	{
// 		int16_t txt_color = WHITE;
// 		int16_t bg_color = BLACK;
// 		if (invert)
// 		{
// 			txt_color = BLACK;
// 			bg_color = WHITE;
// 		}

// 		if (minw < 0)
// 		{
// 			minw = ABS(minw);
// 			x0 -= MAX(gd_str_width(s) + 7, minw);
// 		}

// 		int16_t w = MAX(minw, gd_str_width(s) + 7);
// 		gfx->fillRect(x0, y0, w, gd_line_height(), bg_color);
// 		if (!frameless)
// 		{
// 			gfx->drawRect(x0, y0, w, gd_line_height(), txt_color);
// 		}
// 		gfx->setCursor(x0 + 3, y0 + 3 + gd_font_height());
// 		gd_draw_string_inv(x0 + 3, y0, s, invert);
// 	}

// 	int16_t gd_str_width(const char *s)
// 	{
// 		uint16_t w, h;
// 		int x, y;
// 		gfx->getTextBounds(s, 1, 1, &x, &y, &w, &h);
// 		return w;
// 	}

// 	int16_t gd_str_align_start(const char *s)
// 	{
// 		return 0;
// 	}

// 	int16_t gd_str_align_center(const char *s)
// 	{
// 		return ((gfx->width() - gd_str_width(s)) >> 1);
// 	}

// 	int16_t gd_str_align_end(const char *s)
// 	{
// 		return (gfx->width() - gd_str_width(s));
// 	}

// 	int16_t gd_str_justify_start(void)
// 	{
// 		return 0;
// 	}

// 	int16_t gd_str_justify_center(void)
// 	{
// 		return (gfx->height() + gd_font_height()) / 2;
// 	}

// 	int16_t gd_str_justify_end(void)
// 	{
// 		return gfx->height() - gd_line_height();
// 	}

// 	int16_t gd_get_line_top(int8_t line)
// 	{
// 		int16_t lh = gd_line_height();
// 		// line height plus padding
// 		int8_t offset = graphic_last_line_offset;
// 		if (offset < 0)
// 		{
// 			offset = gfx->height() - lh * floor(gfx->height() / lh);
// 			graphic_last_line_offset = offset;
// 		}
// 		return line * lh + offset;
// 	}

// 	uint8_t gd_display_max_lines(void)
// {
// 	return (uint8_t)floor((gfx->height() *0.8f) / gd_line_height());
// }

// 	/**
// 	 * Create some U8G2 display drivers
// 	 */

// 	void st7796_480x320_spi_init()
// 	{
// 		gfx = new Arduino_ST7796(bus, GRAPHIC_DISPLAY_RST, 1, false, 320, 480);
// 	}

// 	const display_driver_t gd_st7796_480x320_spi = {
// 			.width = 480,
// 			.height = 320,
// 			.init = &st7796_480x320_spi_init};

// 	void ili9341_240x320_spi_init()
// 	{
// 		gfx = new Arduino_ILI9341(bus, GRAPHIC_DISPLAY_RST, 0, true);
// 	}

// 	const display_driver_t gd_ili9341_240x320_spi = {
// 			.width = 240,
// 			.height = 320,
// 			.init = &ili9341_240x320_spi_init};

// #ifdef __cplusplus
// }
// #endif