#ifndef UCNC_U8G2_TFT_GFX_H
#define UCNC_U8G2_TFT_GFX_H

#include "../../cnc.h"
#ifdef USE_GRAPHIC_ARDUINO_GFX_LIB

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define U8G2_BTN_BW1 0x01
#define U8G2_BTN_INV 0x80

	int16_t u8g2_GetDisplayWidth(void *u8g2);
	int16_t u8g2_GetDisplayHeight(void *u8g2);
	int8_t u8g2_GetAscent(void *u8g2);
	int8_t u8g2_GetDescent(void *u8g2);
	int16_t u8g2_GetUTF8Width(void *u8g2, const char *str);
	void u8g2_SetFont(void *u8g2, const uint8_t *font);
	void u8g2_SetFontMode(void *u8g2, uint8_t is_transparent);
	void u8g2_ClearBuffer(void *u8g2);
	void u8g2_SendBuffer(void *u8g2);
	int16_t u8g2_DrawStr(void *u8g2, int16_t x, int16_t y, const char *s);
	void u8g2_DrawLine(void *u8g2, int16_t x0, int16_t y0, int16_t x1, int16_t y1);
	void u8g2_SetDrawColor(void *u8g2, uint8_t color);
	void u8g2_DrawBox(void *u8g2, int16_t x, int16_t y, int16_t w, int16_t h);
	void u8g2_DrawFrame(void *u8g2, int16_t x, int16_t y, int16_t w, int16_t h);
	void u8g2_DrawTriangle(void *u8g2, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
	void u8g2_DrawButtonUTF8(void *u8g2, int16_t x, int16_t y, uint8_t flags, int16_t width, int16_t padding_h, int16_t padding_v, const char *text);
	void u8g2_Init(void);
	void u8g2_RenderStartup(void);

#ifdef __cplusplus
}
#endif

#endif
#endif