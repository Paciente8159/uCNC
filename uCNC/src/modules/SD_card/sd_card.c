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
#include <string.h>

#ifndef UCNC_MODULE_VERSION_1_5_0_PLUS
#error "This module is not compatible with the current version of µCNC"
#endif

#ifndef SD_CARD_DETECT_PIN
#define SD_CARD_DETECT_PIN DIN19
#endif

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 255
#endif

enum SD_CARD_STATUS
{
	SD_UNDETECTED = 0,
	SD_DETECTED = 1,
	SD_MOUNTED = 2,
};

static uint8_t sd_card_mounted;
static FATFS fs;
// current opened file
static FIL fp;
// number of runs to executed the file code
static uint32_t file_runs;

void sd_card_mount(void)
{
	if (sd_card_mounted != SD_MOUNTED)
	{
		if ((f_mount(&fs, "", 1) == FR_OK))
		{
			protocol_send_feedback(__romstr__("SD card mounted"));
			sd_card_mounted = SD_MOUNTED;
			return;
		}

		protocol_send_feedback(__romstr__("SD card failed to mount"));
	}
}

void sd_card_dir_list(void)
{
	FRESULT res;
	UINT i;
	static FILINFO fno;
	DIR dp;

	// current dir
	char curdir[MAX_PATH_LEN];
	if (f_getcwd(curdir, MAX_PATH_LEN) == FR_OK)
	{
		protocol_send_string(__romstr__("Directory of "));
		serial_print_str(curdir);
		serial_print_str(STR_EOL);
		if (f_opendir(&dp, curdir) == FR_OK)
		{

			for (;;)
			{
				res = f_readdir(&dp, &fno); /* Read a directory item */
				if (res != FR_OK || fno.fname[0] == 0)
				{
					break; /* Break on error or end of dir */
				}
				if (fno.fattrib & AM_DIR)
				{ /* It is a directory */
					protocol_send_string(__romstr__("<dir>\t"));
				}
				else
				{ /* It is a file. */
					protocol_send_string(__romstr__("     \t"));
				}

				i = strlen(fno.fname);
				for (uint8_t j = 0; j < i; j++)
				{
					serial_putc(fno.fname[j]);
				}
				protocol_send_string(MSG_EOL);
			}

			f_closedir(&dp);
		}
	}
}

void sd_card_cd(void)
{
	UINT i;
	TCHAR newdir[RX_BUFFER_CAPACITY]; /* File name */

	while (serial_peek() == ' ')
	{
		serial_getc();
	}

	while (serial_peek() != EOL)
	{
		newdir[i++] = serial_getc();
		newdir[i] = 0;
	}

	if (f_chdir(newdir) == FR_OK)
	{
		char curdir[MAX_PATH_LEN];
		if (f_getcwd(curdir, MAX_PATH_LEN) == FR_OK)
		{
			serial_print_str(curdir);
			serial_print_str(">" STR_EOL);
		}
	}
}

void sd_card_file_print(void)
{
	UINT i = 0;
	TCHAR file[RX_BUFFER_CAPACITY]; /* File name */
	FIL tmp;

	while (serial_peek() == ' ')
	{
		serial_getc();
	}

	while (serial_peek() != EOL)
	{
		file[i++] = serial_getc();
		file[i] = 0;
	}

	if (f_open(&tmp, file, FA_READ) == FR_OK)
	{
		while (!f_eof(&tmp))
		{
			f_read(&tmp, file, 100, &i);
			file[i] = 0;
			serial_print_str(file);
			serial_flush();
		}

		f_close(&tmp);
		return;
	}

	protocol_send_feedback(__romstr__("Error printing file"));
}

void sd_card_file_run(void)
{
	UINT i = 0;
	TCHAR args[RX_BUFFER_CAPACITY]; /* get parameters */
	TCHAR *file;

	while (serial_peek() == ' ')
	{
		serial_getc();
	}

	while (serial_peek() != EOL)
	{
		args[i++] = serial_getc();
		args[i] = 0;
	}

	uint32_t runs = (uint32_t)strtol(args, &file, 10);

	while (*file == ' ')
	{
		file++;
	}

	serial_print_str(file);

	if (f_open(&fp, file, FA_READ) == FR_OK)
	{
		file_runs = (runs != 0) ? runs : 1;
		protocol_send_string(MSG_START);
		protocol_send_string(__romstr__("Running file-remaining "));
		serial_print_int(file_runs);
		protocol_send_string(MSG_END);
		return;
	}

	protocol_send_feedback(__romstr__("Error running file"));
}

/**
 * Handles SD card in the main loop
 * */
uint8_t sd_card_loop(void *args, bool *handled)
{
#if (!(SD_CARD_DETECT_PIN < 0))
	if (mcu_get_input(SD_CARD_DETECT_PIN) && sd_card_mounted)
	{
		protocol_send_feedback(__romstr__("SD card removed"));
		if(sd_card_mounted == SD_MOUNTED){
			f_unmount("");
		}
		sd_card_mounted = SD_UNDETECTED;
	}
	else if (!mcu_get_input(SD_CARD_DETECT_PIN) && !sd_card_mounted)
	{
		sd_card_mounted = SD_DETECTED;
		cnc_delay_ms(2000);
		sd_card_mount();
	}
#endif

	uint32_t runs = file_runs;
	while (runs)
	{
		TCHAR buff[32];
		UINT i = 0;
		while (!f_eof(&fp))
		{
			if (serial_get_rx_freebytes() < 32)
			{
				// leaves the loop to enable code to run
				return STATUS_OK;
			}
			f_read(&fp, buff, 32, &i);
			uint8_t j = 0;
			do
			{
				mcu_com_rx_cb(buff[j++]);
			} while (--i);
		}

		if (--runs)
		{
			protocol_send_string(MSG_START);
			protocol_send_string(__romstr__("Running file-remaining "));
			serial_print_int(file_runs);
			protocol_send_string(MSG_END);
			f_lseek(&fp, 0);
		}
		else
		{
			protocol_send_feedback(__romstr__("File finnished"));
			f_close(&fp);
		}

		file_runs = runs;
	}

	return STATUS_OK;
}

CREATE_EVENT_LISTENER(cnc_dotasks, sd_card_loop);

/**
 * Handles grbl commands for the SD card
 * */
uint8_t sd_card_cmd_parser(void *args, bool *handled)
{
	grbl_cmd_args_t *cmd = args;

	strupr((char*)cmd->cmd);

	if (!strcmp("MNT", (char *)(cmd->cmd)))
	{
		sd_card_mount();
		*handled = true;
		return GRBL_SYSTEM_CMD_EXTENDED;
	}

	if (!strcmp("LS", (char *)(cmd->cmd)))
	{
		sd_card_dir_list();
		*handled = true;
		return GRBL_SYSTEM_CMD_EXTENDED;
	}

	if (!strcmp("CD", (char *)(cmd->cmd)))
	{
		sd_card_cd();
		*handled = true;
		return GRBL_SYSTEM_CMD_EXTENDED;
	}

	if (!strcmp("LPR", (char *)(cmd->cmd)))
	{
		sd_card_file_print();
		*handled = true;
		return GRBL_SYSTEM_CMD_EXTENDED;
	}

	if (!strcmp("RUN", (char *)(cmd->cmd)))
	{
		sd_card_file_run();
		*handled = true;
		return GRBL_SYSTEM_CMD_EXTENDED;
	}

	return 0;
}

CREATE_EVENT_LISTENER(grbl_cmd, sd_card_cmd_parser);

DECL_MODULE(sd_card)
{
	file_runs = 0;
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, sd_card_loop);
#else
#warning "Main loop extensions are not enabled. SD card will not work."
#endif

#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(grbl_cmd, sd_card_cmd_parser);
#else
#warning "Parser extensions are not enabled. SD card commands will not work."
#endif
}
