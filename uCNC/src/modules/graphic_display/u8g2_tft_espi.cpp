#include <Arduino.h>
#include <TFT_eSPI.h>
#include <U8g2_for_TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
U8g2_for_TFT_eSPI u8f;
void *u8f_ptr;
uint8_t forecolor = 0;

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../cnc.h"
#include "u8g2_tft_espi.h"

#ifndef TFTWIDTH
#define TFTWIDTH 420
#endif

#ifndef TFTHEIGHT
#define TFTHEIGHT 380
#endif

	int16_t u8g2_GetDisplayWidth(void *u8g2)
	{
		return TFTWIDTH;
	}

	int16_t u8g2_GetDisplayHeight(void *u8g2)
	{
		return TFTHEIGHT;
	}

	int8_t u8g2_GetAscent(void *u8g2)
	{
		return u8f.getFontAscent();
	}

	int8_t u8g2_GetDescent(void *u8g2)
	{
		return u8f.getFontDescent();
	}

	int16_t u8g2_GetUTF8Width(void *u8g2, const char *str)
	{
		return u8f.getUTF8Width(str);
	}

	void u8g2_SetFont(void *u8g2, const uint8_t *font)
	{
		return u8f.setFont(font);
	}

	void u8g2_SetFontMode(void *u8g2, uint8_t is_transparent){
		u8f.setFontMode(is_transparent);
	}

	void u8g2_ClearBuffer(void *u8g2)
	{
		tft.fillScreen(TFT_BLACK);
	}

	void u8g2_SendBuffer(void *u8g2) {}

	int16_t u8g2_DrawStr(void *u8g2, int16_t x, int16_t y, const char *s)
	{
		return u8f.drawStr(x, y, s);
	}

	void u8g2_DrawLine(void *u8g2, int16_t x0, int16_t y0, int16_t x1, int16_t y1)
	{
		// use spi code directly
		if (y0 == y1)
		{
			tft.drawFastHLine(MIN(x0, x1), y0, ABS(x1 - x0), TFT_WHITE);
		}
		else if (x0 == x1)
		{
			tft.drawFastVLine(x0, MIN(y0, y1), ABS(y1 - y0), TFT_WHITE);
		}
		else
		{
			tft.drawLine(x0, y0, x1, y1, TFT_WHITE);
		}
	}

	void u8g2_SetDrawColor(void *u8g2, uint8_t color)
	{
		forecolor = color;
		// use spi code directly
		switch (color)
		{
		case 0:
			u8f.setForegroundColor(TFT_WHITE);
			u8f.setBackgroundColor(TFT_BLACK);
			break;
		case 1:
			u8f.setForegroundColor(TFT_BLACK);
			u8f.setBackgroundColor(TFT_WHITE);
			break;
		default:
			break;
		}
	}

	void u8g2_DrawBox(void *u8g2, int16_t x, int16_t y, int16_t w, int16_t h)
	{
		// use spi code directly
		tft.fillRect(x, y, w, h, (forecolor) ? TFT_WHITE : TFT_BLACK);
	}

	void u8g2_DrawFrame(void *u8g2, int16_t x, int16_t y, int16_t w, int16_t h)
	{
		tft.drawRect(x, y, w, h, (forecolor) ? TFT_BLACK : TFT_WHITE);
	}

	void u8g2_DrawTriangle(void *u8g2, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2)
	{
		// use spi code directly
		tft.drawTriangle(x0, y0, x1, y1, x2, y2, TFT_RED);
	}

	void u8g2_DrawButtonUTF8(void *u8g2, int16_t x, int16_t y, uint8_t flags, int16_t width, int16_t padding_h, int16_t padding_v, const char *text)
	{
		uint8_t color = (flags & U8G2_BTN_INV) ? 1 : 0;
		int16_t height = u8f.getFontAscent() - u8f.getFontDescent();
		width = MAX(u8f.getUTF8Width(text), width);

		// use spi code directly
		// u8f.setForegroundColor(forecolor);
		// tft.fillRect(x - 2, y - u8f.getFontAscent() - 2, width + 4, height + 4, backcolor);
		u8g2_SetDrawColor(u8g2, color);
		u8f.setCursor(x, y); // start writing at this position
		u8f.print(text);
		/* draw the box around the text with a margin*/
		u8g2_DrawFrame(u8g2, x - 2, y - u8f.getFontAscent() - 2, width + 4, height + 4);
		u8g2_SetDrawColor(u8g2, 0);
	}

	void u8g2_Init(void)
	{
		tft.begin();
		tft.setRotation(1);
		tft.fillScreen(TFT_BLACK);

		u8f.begin(tft);
		u8f.setFontMode(0);								 
		u8f.setFontDirection(0);					 // left to right (this is default)
		u8f.setForegroundColor(TFT_WHITE); // apply color
	}

	void u8g2_RenderStartup(void)
	{
	}

#ifdef __cplusplus
}
#endif
