#ifndef SD_MESSAGES_H
#define SD_MESSAGES_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../system_languages.h"

#ifndef SD_STR_SD_PREFIX
#define SD_STR_SD_PREFIX "SD card "
#endif
#ifndef SD_STR_SD_MOUNTED
#define SD_STR_SD_MOUNTED "mounted"
#endif
#ifndef SD_STR_SD_UNMOUNTED
#define SD_STR_SD_UNMOUNTED "unmounted"
#endif
#ifndef SD_STR_SD_NOT_FOUND
#define SD_STR_SD_NOT_FOUND "not found!"
#endif
#ifndef SD_STR_SD_ERROR
#define SD_STR_SD_ERROR "error!"
#endif
#ifndef SD_STR_SETTINGS_FOUND
#define SD_STR_SETTINGS_FOUND SD_STR_SD_PREFIX "settings found"
#endif
#ifndef SD_STR_SETTINGS_LOADED
#define SD_STR_SETTINGS_LOADED SD_STR_SD_PREFIX "settings loaded"
#endif
#ifndef SD_STR_SETTINGS_NOT_FOUND
#define SD_STR_SETTINGS_NOT_FOUND SD_STR_SD_PREFIX "settings not found"
#endif
#ifndef SD_STR_SETTINGS_SAVED
#define SD_STR_SETTINGS_SAVED SD_STR_SD_PREFIX "settings saved"
#endif
#ifndef SD_STR_SETTINGS_ERASED
#define SD_STR_SETTINGS_ERASED SD_STR_SD_PREFIX "settings erased"
#endif

#ifdef __cplusplus
}
#endif

#endif