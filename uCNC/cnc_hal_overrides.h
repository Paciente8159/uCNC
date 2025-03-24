#ifndef CNC_HAL_OVERRIDES_H
#define CNC_HAL_OVERRIDES_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "cnc_hal_reset.h"
#define ESTOP_PULLUP_ENABLE
#define SAFETY_DOOR_PULLUP_ENABLE
#define FHOLD_PULLUP_ENABLE
#define CS_RES_PULLUP_ENABLE
#define LIMIT_X_PULLUP_ENABLE
#define LIMIT_Y_PULLUP_ENABLE
#define LIMIT_Z_PULLUP_ENABLE
#define LIMIT_X2_PULLUP_ENABLE
#define LIMIT_Y2_PULLUP_ENABLE
#define LIMIT_Z2_PULLUP_ENABLE
#define TOOL1 spindle_pwm
#define ENCODERS 0
#define ENABLE_MAIN_LOOP_MODULES
#define ENABLE_PARSER_MODULES
#define SD_CARD_INTERFACE SD_CARD_HW_CUSTOM
#define SD_SPI_CS SPI_CS
#define SD_CARD_DETECT_PIN UNDEF_PIN
#define FF_USE_LFN 1
#define ENABLE_SETTINGS_ON_SD_SDCARD
//Custom configurations


#define LOAD_MODULES_OVERRIDE() ({LOAD_MODULE(stm32_sdio);LOAD_MODULE(sd_card_v2);})

#ifdef __cplusplus
}
#endif
#endif
