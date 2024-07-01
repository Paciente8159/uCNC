#ifndef XPT2046_TOUCH_H
#define XPT2046_TOUCH_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "../softspi.h"

#ifndef XPT2046_CS
#define XPT2046_CS DOUT13
#endif
#ifndef XPT2046_TOUCHED
#define XPT2046_TOUCHED DIN17
#endif
#ifndef XPT2046_MARGIN
#define XPT2046_MARGIN 20
#endif
#ifndef XPT2046_MAX_ERROR
#define XPT2046_MAX_ERROR 20
#endif

#ifndef XPT2046_SPI_FREQ
#define XPT2046_SPI_FREQ 1000000UL
#endif

#define XPT2046_SKIP_CALIBRATION
#define XPT2046_USE_TOUCH_IRQ

#define XPT2046_NO_TOUCH 0
#define XPT2046_TOUCH_1ST_SAMPLE 1
#define XPT2046_TOUCH_2ND_SAMPLE 2
#define XPT2046_TOUCH_TOUCHED 3

#define XPT2046_PARAMS_ROTATE 1
#define XPT2046_PARAMS_INVERT_X 2
#define XPT2046_PARAMS_INVERT_Y 4

void xpt2046_init(uint16_t width, uint16_t height, uint8_t params, softspi_port_t *spi_bus, uint32_t spi_speed);
void xpt2046_get_calibration_points(uint16_t *x1, uint16_t *y1, uint16_t *x2, uint16_t *y2);
void xpt2046_set_calibration(uint16_t *x1, uint16_t *y1, uint16_t *x2, uint16_t *y2);
bool xpt2046_get_position(uint16_t *x, uint16_t *y,uint16_t threshold);

#ifdef __cplusplus
}
#endif
#endif