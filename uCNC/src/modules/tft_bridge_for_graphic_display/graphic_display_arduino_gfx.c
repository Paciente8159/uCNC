/**
 * 
 * Display drivers declarations
 * 
 */

#include "../graphic_display/graphic_display.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern void ili9341_240x320_spi_init();
const display_driver_t gd_ili9341_240x320_spi = {.width = 240, .height = 320, .init = &ili9341_240x320_spi_init}; 

#ifdef __cplusplus
}
#endif