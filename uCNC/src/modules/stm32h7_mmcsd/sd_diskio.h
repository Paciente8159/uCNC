/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    sd_diskio.h
  * @brief   Header for sd_diskio.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Note: code generation based on sd_diskio_template.h */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SD_DISKIO_H
#define __SD_DISKIO_H

/* USER CODE BEGIN firstSection */
/* can be used to modify / undefine following code or add new definitions */
/* USER CODE END firstSection */

/* Includes ------------------------------------------------------------------*/
#include "bsp_driver_sd.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* USER CODE BEGIN lastSection */
/* can be used to modify / undefine previous code or add new definitions */
#include "../../cnc.h"
#define PETIT_FAT_FS 1
#define FAT_FS 2

#ifndef SD_FAT_FS
#define SD_FAT_FS PETIT_FAT_FS
#endif

#if (SD_FAT_FS == PETIT_FAT_FS)
#include "../sd_card_v2/petit_fat_fs/pff.h"
#else
#include "../sd_card_v2/fat_fs/ff.h"
#endif

#include "../sd_card_v2/diskio.h"

#ifndef MMCSD_MAX_NCR
#define MMCSD_MAX_NCR 255
#endif
#ifndef MMCSD_MAX_TIMEOUT
#define MMCSD_MAX_TIMEOUT 500
#endif
#define MMCSD_TIMEOUT (mcu_millis() + MMCSD_MAX_TIMEOUT)
#ifndef MMCSD_MAX_BUFFER_SIZE
#define MMCSD_MAX_BUFFER_SIZE 512
#endif

#define _USE_WRITE 1
#define _USE_IOCTL 1
/* USER CODE END lastSection */

#endif /* __SD_DISKIO_H */
