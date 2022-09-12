/*
	Name: sd_card.c
	Description: SD card module for µCNC.
	This adds SD card support via SPI

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 08-09-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include "../softspi.h"
#include "ffconf.h"
#include "mmcsd.h"
#include "ff.h"
#include "diskio.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef UCNC_MODULE_VERSION_1_5_0_PLUS
#error "This module is not compatible with the current version of µCNC"
#endif

#ifndef SD_CARD_DETECT_PIN
#define SD_CARD_DETECT_PIN DIN19
#endif

enum SD_CARD_STATUS
{
	SD_UNDETECTED = 0,
	SD_DETECTED = 1,
	SD_MOUNTED = 2,
};

static uint8_t sd_card_mounted;
static FATFS fs;

/**
 * Handles SD card in the main loop
 * */
uint8_t sd_card_loop(void *args, bool *handled)
{
#if (!(SD_CARD_DETECT_PIN < 0))
	if (mcu_get_input(SD_CARD_DETECT_PIN) && sd_card_mounted)
	{
		protocol_send_feedback(__romstr__("SD card removed"));
		sd_card_mounted = SD_UNDETECTED;
	}
	else if (!mcu_get_input(SD_CARD_DETECT_PIN) && !sd_card_mounted)
	{
		if (sd_card_mounted != SD_DETECTED)
		{
			protocol_send_feedback(__romstr__("SD card detected"));
			sd_card_mounted = SD_DETECTED;
			cnc_delay_ms(1000);
		}
		 
		if (f_mount(&fs, "", 1) == FR_OK)
		{
			protocol_send_feedback(__romstr__("SD card mounted"));
			sd_card_mounted = SD_MOUNTED;
		}
		else
		{
			protocol_send_feedback(__romstr__("SD card failed to mount"));
		}
	}
#endif

	return STATUS_OK;
}

CREATE_EVENT_LISTENER(cnc_dotasks, sd_card_loop);

/**
 * Handles grbl commands for the SD card
 * */
uint8_t sd_card_cmd(void *args, bool *handled)
{
	grbl_cmd_args_t *cmd = args;

	if (cmd->cmd[0] == 'F' && cmd->cmd[1] == 'M')
	{
		if (sd_card_mounted!= SD_MOUNTED)
		{
			if (f_mount(&fs, "", 1) == FR_OK)
			{
				protocol_send_feedback(__romstr__("SD card mounted"));
				sd_card_mounted = SD_MOUNTED;
			}
			else
			{
				protocol_send_feedback(__romstr__("SD card failed to mount"));
			}
		}
		*handled = true;
		return GRBL_SYSTEM_CMD_EXTENDED;
	}
}

CREATE_EVENT_LISTENER(grbl_cmd, sd_card_cmd);

DECL_MODULE(sd_card)
{
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, sd_card_loop);
#else
#warning "Main loop extensions are not enabled. SD card will not work."
#endif

#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(grbl_cmd, sd_card_cmd);
#else
#warning "Parser extensions are not enabled. SD card commands will not work."
#endif
}
