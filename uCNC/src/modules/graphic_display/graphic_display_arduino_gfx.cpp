#include "../../../cnc_config.h"
#define GRAPHIC_DISPLAY_USE_ARDUINO_GFX_LIB

#ifdef GRAPHIC_DISPLAY_USE_ARDUINO_GFX_LIB
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "Arduino_uCNC_SPI.h"

static Arduino_DataBus *bus;
static Arduino_GFX *gfx;
static void *graphic_port;

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../cnc.h"
#include "graphic_display.h"
#include "../softspi.h"
#include "../softi2c.h"

	int16_t gd_font_height(void)
	{
		// return u8g2_GetAscent(U8G2) - u8g2_GetDescent(U8G2);
		return 0;
	}

	void gd_init(display_driver_t *gdriver, void *port_interface)
	{
		graphic_port = port_interface;
		//bus = new Arduino_uCNC_SPI((softspi_port_t *)graphic_port, DOUT7, GRAPHIC_DISPLAY_SPI_CS, true);
		bus = new Arduino_HWSPI(33,25,18,23,39);
		gdriver->init();
		mcu_set_output(DOUT11);
		gfx->begin();
		gfx->fillScreen(RED);
		gdriver->width = 320;
		gdriver->height = 240;
	}

	void gd_clear()
	{
		// u8g2_ClearBuffer(U8G2);
	}

	void gd_flush()
	{
		// u8g2_SendBuffer(U8G2);
	}

	void gd_draw_startup(void)
	{
		// u8g2_ClearBuffer(U8G2);
		// char buff[SYSTEM_MENU_MAX_STR_LEN];
		// rom_strcpy(buff, __romstr__("µCNC"));
		// u8g2_ClearBuffer(U8G2);
		// u8g2_SetFont(U8G2, u8g2_font_9x15_t_symbols);
		// u8g2_DrawUTF8X2(U8G2, (u8g2_GetDisplayWidth(U8G2) / 2 - u8g2_GetUTF8Width(U8G2, buff)), gd_str_justify_center(), buff);
		// rom_strcpy(buff, __romstr__(("v" CNC_VERSION)));
		// u8g2_SetFont(U8G2, u8g2_font_6x12_tr);
		// u8g2_DrawStr(U8G2, gd_str_align_center(buff), gd_str_justify_end(), buff);
		// u8g2_SendBuffer(U8G2);
	}

	void gd_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
	{
		// u8g2_DrawLine(U8G2, x0, y0, x1, y1);
	}

	void gd_draw_rectangle(int16_t x0, int16_t y0, int16_t w, int16_t h)
	{
		// u8g2_DrawFrame(U8G2, x0, y0, w, h);
	}

	void gd_draw_rectangle_fill(int16_t x0, int16_t y0, int16_t w, int16_t h, bool invert)
	{
		// if (invert)
		// {
		// 	u8g2_SetDrawColor(U8G2, 0);
		// }
		// u8g2_DrawBox(U8G2, x0, y0, w, h);
		// if (invert)
		// {
		// 	u8g2_SetDrawColor(U8G2, 1);
		// }
	}

	void gd_draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2)
	{
		// u8g2_DrawTriangle(U8G2, x0, y0, x1, y1, x2, y2);
	}

	void gd_draw_string(int16_t x0, int16_t y0, const char *s)
	{
		// u8g2_DrawStr(U8G2, x0, y0, s);
	}

	void gd_draw_string_inv(int16_t x0, int16_t y0, const char *s, bool invert)
	{
		// if (invert)
		// {
		// 	u8g2_SetDrawColor(U8G2, 0);
		// }
		// u8g2_DrawStr(U8G2, x0, y0, s);
		// if (invert)
		// {
		// 	u8g2_SetDrawColor(U8G2, 1);
		// }
	}

	void gd_draw_button(int16_t x0, int16_t y0, const char *s, int16_t minw, int16_t p_h, int16_t p_v, bool invert, bool frameless)
	{
		// uint8_t mode = (!frameless) ? U8G2_BTN_BW1 : U8G2_BTN_BW0;
		// mode |= (!invert) ? 0 : U8G2_BTN_INV;
		// u8g2_DrawButtonUTF8(U8G2, x0, y0, mode, minw, p_h, p_v, s);
	}

	int16_t gd_str_width(const char *s)
	{
		// return u8g2_GetUTF8Width(U8G2, s);
		return 0;
	}

	int16_t gd_str_align_start(const char *s)
	{
		return 0;
	}

	int16_t gd_str_align_center(const char *s)
	{
		// return ((u8g2_GetDisplayWidth(U8G2) - gd_str_width(s)) >> 1);
		return 0;
	}

	int16_t gd_str_align_end(const char *s)
	{
		// return (u8g2_GetDisplayWidth(U8G2) - gd_str_width(s));
		return 0;
	}

	int16_t gd_str_justify_start(void)
	{
		// return u8g2_GetAscent(U8G2);
		return 0;
	}

	int16_t gd_str_justify_center(void)
	{
		// return (u8g2_GetDisplayHeight(U8G2) + gd_font_height()) / 2;
		return 0;
	}

	int16_t gd_str_justify_end(void)
	{
		// return u8g2_GetDisplayHeight(U8G2) + u8g2_GetDescent(U8G2);
		return 0;
	}

	/**
	 * Create some U8G2 display drivers
	 */

	void st7796_320x240_spi_init()
	{
		gfx = new Arduino_ST7796(bus,27,0,false,480, 320);
	}

	const display_driver_t gd_st7796_320x240_spi = {
			.width = 320,
			.height = 240,
			.init = &st7796_320x240_spi_init};

#ifdef __cplusplus
}
#endif
#endif