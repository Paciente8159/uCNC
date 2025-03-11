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

#include "src/cnc.h"
#include "src/modules/file_system.h"
#include "sd_messages.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define PETIT_FAT_FS 1
#define FAT_FS 2

#ifdef ENABLE_SETTINGS_MODULES
#ifdef ENABLE_SETTINGS_ON_SD_SDCARD
#undef SD_FAT_FS
#define SD_FAT_FS FAT_FS
#endif
#else
#undef ENABLE_SETTINGS_ON_SD_SDCARD
#endif

#ifndef SD_FAT_FS
#define SD_FAT_FS FAT_FS
#endif

#if (UCNC_MODULE_VERSION < 11100 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#ifndef SD_CARD_DETECT_PIN
#define SD_CARD_DETECT_PIN DIN19
#endif

#ifndef SD_CONTINUE_ON_GCODE_ERROR
#define SD_STOP_ON_GCODE_ERROR
#endif

#ifndef FS_MAX_PATH_LEN
#define FS_MAX_PATH_LEN 128
#endif

#if (SD_FAT_FS == PETIT_FAT_FS)
#include "petit_fat_fs/pffconf.h"
#include "petit_fat_fs/pff.h"

#define FA_READ 0x01
#define FA_WRITE 0x02
#define FA_OPEN_EXISTING 0x00
#define FA_CREATE_NEW 0x04
#define FA_CREATE_ALWAYS 0x08
#define FA_OPEN_ALWAYS 0x10
#define FA_OPEN_APPEND 0x30

typedef struct
{
	FATFS *fs;
	DWORD fptr;
} FIL;

// file system
static FATFS cfs;
// current opend file
static FIL cwf;
static char cwd[FS_MAX_PATH_LEN];

static FORCEINLINE FRESULT sd_mount(FATFS *fs)
{
	cwd[0] = 0;
	return pf_mount(fs);
}

#define sd_unmount(fs) (cwd[0] = 0)

static FORCEINLINE FRESULT sd_fopen(FIL *fp, const char *name, BYTE mode)
{
	if (pf_open(cwd) == FR_OK)
	{
		cwf.fs = &cfs;
		return FR_OK;
	}

	return FR_NO_FILE;
}

static FORCEINLINE FRESULT sd_fread(FIL *fp, void *buff, UINT btr, UINT *br)
{
	if (!cwf.fs)
	{
		return FR_NOT_OPENED;
	}

	return pf_read(buff, btr, br);
}

static FORCEINLINE FRESULT sd_fwrite(FIL *fp, const char *buff, UINT btw, UINT *bw)
{
	if (!cwf.fs)
	{
		return FR_NOT_OPENED;
	}
	return pf_write(buff, btw, bw);
}

static FORCEINLINE FRESULT sd_fseek(FIL *fp, DWORD ofs)
{
	if (!cwf.fs)
	{
		return FR_NOT_OPENED;
	}
	return pf_lseek(ofs);
}

static FORCEINLINE FRESULT sd_fclose(FIL *fp)
{
	if (cwf.fs)
	{
		cwf.fs = NULL;
		return FR_OK;
	}

	return FR_NOT_OPENED;
}
#define sd_fsync(x)
#define sd_fileinfo(x, y) FR_OK
#define sd_closedir(x)
#define sd_remove(X) FR_OK
#define sd_mkdir(x) FR_OK
static FORCEINLINE FRESULT sd_opendir(DIR *dp, const char *path) { return pf_opendir(dp, path); }
static FORCEINLINE FRESULT sd_readdir(DIR *dp, FILINFO *fno) { return pf_readdir(dp, fno); }
bool sd_eof()
{
	if (cwf.fs)
	{
		return (cwf.fs->fsize == cwf.fs->fptr);
	}
	return true;
}
#else
#include "fat_fs/ffconf.h"
#include "fat_fs/ff.h"

// file system
static FATFS cfs;

static FORCEINLINE FRESULT sd_mount(FATFS *fs) { return f_mount(fs, "/", 1); }
static FORCEINLINE FRESULT sd_unmount(FATFS *fs) { return f_mount(NULL, "/", 0); }
static FORCEINLINE FRESULT sd_fopen(FIL *fp, const char *path, uint8_t mode) { return f_open(fp, path, mode); }
static FORCEINLINE FRESULT sd_fread(FIL *fp, void *buff, size_t btr, size_t *br) { return f_read(fp, buff, btr, br); }
static FORCEINLINE FRESULT sd_fwrite(FIL *fp, void *buff, size_t btw, size_t *bw) { return f_write(fp, buff, btw, bw); }
static FORCEINLINE FRESULT sd_fseek(FIL *fp, uint32_t ofs) { return f_lseek(fp, ofs); }
static FORCEINLINE FRESULT sd_fclose(FIL *fp) { return f_close(fp); }
static FORCEINLINE FRESULT sd_fsync(FIL *fp) { return f_sync(fp); }
static FORCEINLINE FRESULT sd_opendir(DIR *dp, const char *path) { return f_opendir(dp, path); }
static FORCEINLINE FRESULT sd_readdir(DIR *dp, FILINFO *fno) { return f_readdir(dp, fno); }
static FORCEINLINE FRESULT sd_closedir(DIR *dp) { return f_closedir(dp); }
static FORCEINLINE FRESULT sd_eof(FIL *fp) { return f_eof(fp); }
static FORCEINLINE FRESULT sd_fileinfo(const char *path, FILINFO *fno) { return f_stat(path, fno); }
static FORCEINLINE FRESULT sd_remove(const char *path) { return f_unlink(path); }
static FORCEINLINE FRESULT sd_mkdir(const char *path) { return f_mkdir(path); }
#endif

#include "diskio.h"

#if (SD_CARD_INTERFACE == SD_CARD_SW_SPI)
#define SD_CARD_BUS_LOCK LISTENER_SWSPI_LOCK
#elif (SD_CARD_INTERFACE == SD_CARD_HW_SPI)
#define SD_CARD_BUS_LOCK LISTENER_HWSPI_LOCK
#elif (SD_CARD_INTERFACE == SD_CARD_HW_SPI2)
#define SD_CARD_BUS_LOCK LISTENER_HWSPI2_LOCK
#else
#define SD_CARD_BUS_LOCK LISTENER_NO_LOCK
#endif

enum SD_CARD_STATUS
{
	SD_UNDETECTED = 0,
	SD_DETECTED = 1,
	SD_MOUNTED = 2,
};

static uint8_t sd_card_mounted;
fs_t sd_fs;

bool sd_fs_finfo(const char *path, fs_file_info_t *finfo)
{
	FILINFO info;
	if (!finfo)
	{
		return false;
	}
	memset(finfo, 0, sizeof(fs_file_info_t));

	// root dir
	if (strlen(path) < 2)
	{
		strcpy(finfo->full_name, "/");
		finfo->is_dir = true;
		return true;
	}

	if (sd_fileinfo(path, &info) == FR_OK)
	{
		strcpy(finfo->full_name, path);
		finfo->is_dir = (info.fattrib && AM_DIR);
		finfo->size = info.fsize;
		finfo->timestamp = ((uint32_t)info.fdate << 16) | info.ftime;
		return true;
	}
	return false;
}

fs_file_t *sd_fs_open(const char *file, const char *mode)
{
	fs_file_t *fp = calloc(1, sizeof(fs_file_t));

	if (!fp)
	{
		return NULL;
	}

	fp->file_ptr = calloc(1, sizeof(FIL));
	if (!fp->file_ptr)
	{
		free(fp);
		return NULL;
	}

	uint8_t modebyte = 0;
	if (strchr(mode, 'r'))
	{
		modebyte |= FA_READ;
	}
	if (strchr(mode, 'w'))
	{
		modebyte |= FA_CREATE_ALWAYS | FA_WRITE;
	}
	if (strchr(mode, 'a'))
	{
		modebyte |= FA_OPEN_APPEND | FA_WRITE;
	}
	if (strchr(mode, '+'))
	{
		modebyte |= FA_READ | FA_WRITE;
	}
	if (strchr(mode, 'x'))
	{
		modebyte |= FA_CREATE_NEW;
	}

	if (sd_fopen(fp->file_ptr, file, modebyte) != FR_OK)
	{
		free(fp->file_ptr);
		free(fp);
		return NULL;
	}

	sd_fs_finfo(file, &(fp->file_info));

	return fp;
}

size_t sd_fs_read(fs_file_t *fp, uint8_t *buffer, size_t len)
{
	size_t result = 0;
	sd_fread(fp->file_ptr, buffer, len, &result);
	return result;
}

size_t sd_fs_write(fs_file_t *fp, const uint8_t *buffer, size_t len)
{
	size_t result = 0;
	sd_fwrite(fp->file_ptr, (void *)buffer, len, &result);
	sd_fsync(fp->file_ptr);
	return result;
}

int sd_fs_available(fs_file_t *fp)
{
	FIL *ptr = fp->file_ptr;
	return (fp->file_info.size - ptr->fptr);
}

void sd_fs_close(fs_file_t *fp)
{
	if (!fp)
	{
		return;
	}

	if (fp->file_ptr)
	{
		if (fp->file_info.is_dir)
		{
			sd_closedir(fp->file_ptr);
		}
		else
		{
			sd_fclose(fp->file_ptr);
		}
	}
}

bool sd_fs_remove(const char *path)
{
	return (sd_remove(path) == FR_OK);
}

bool sd_fs_next_file(fs_file_t *fp, fs_file_info_t *finfo)
{
	FILINFO info;
	if (sd_readdir(fp->file_ptr, &info) != FR_OK)
	{
		return false;
	}
	if (!strlen(info.fname))
	{
		return false;
	}
	strcpy(finfo->full_name, fp->file_info.full_name);
	strcat(finfo->full_name, "/");
	strcat(finfo->full_name, info.fname);
	finfo->is_dir = (info.fattrib & AM_DIR);
	fp->file_info.size = info.fsize;
	fp->file_info.timestamp = ((uint32_t)info.fdate << 16) | info.ftime;

	return true;
}

fs_file_t *sd_fs_opendir(const char *file)
{
	DEBUG_STR("open dir\n\r");
	DEBUG_STR(file);
	DEBUG_STR("\n\r");
	fs_file_t *fp = calloc(1, sizeof(fs_file_t));

	if (!fp)
	{
		return NULL;
	}

	DEBUG_STR("clear file info\n\r");
	memset(&(fp->file_info), 0, sizeof(fs_file_info_t));
	DEBUG_STR("creating dir ptr\n\r");
	fp->file_ptr = calloc(1, sizeof(DIR));
	if (!fp->file_ptr)
	{
		free(fp);
		return NULL;
	}
	DEBUG_STR("opening dir\n\r");
	if (sd_opendir(fp->file_ptr, file) == FR_OK)
	{
		// is a dir
		DEBUG_STR("dir opened\n\r");
		strcpy(fp->file_info.full_name, file);
		fp->file_info.is_dir = true;
		return fp;
	}

	free(fp->file_ptr);
	free(fp);
	return NULL;
}

bool sd_fs_rmdir(const char *path)
{
	return (sd_remove(path) == FR_OK);
}

bool sd_fs_mkdir(const char *path)
{
	return (sd_mkdir(path) == FR_OK);
}

bool sd_fs_seek(fs_file_t *fp, uint32_t offset)
{
	return (sd_fseek(fp->file_ptr, offset) == FR_OK);
}

void sd_card_mount(void)
{
	if (sd_card_mounted != SD_MOUNTED)
	{
		if ((sd_mount(&cfs) == FR_OK))
		{
			proto_feedback(SD_STR_SD_PREFIX SD_STR_SD_MOUNTED);
			sd_fs.drive = 'D';
			sd_fs.open = sd_fs_open;
			sd_fs.read = sd_fs_read;
			sd_fs.write = sd_fs_write;
			sd_fs.seek = sd_fs_seek;
			sd_fs.available = sd_fs_available;
			sd_fs.close = sd_fs_close;
			sd_fs.remove = sd_fs_remove;
			sd_fs.opendir = sd_fs_opendir;
			sd_fs.mkdir = sd_fs_mkdir;
			sd_fs.rmdir = sd_fs_rmdir;
			sd_fs.next_file = sd_fs_next_file;
			sd_fs.finfo = sd_fs_finfo;
			sd_fs.next = NULL;
			fs_mount(&sd_fs);
			sd_card_mounted = SD_MOUNTED;
#ifdef ENABLE_SETTINGS_ON_SD_SDCARD
			RUNONCE
			{ // clear the read error
				g_settings_error &= ~SETTINGS_READ_ERROR;
				// reload all stored settings
				settings_init();
				// reload all non volatile parser parameters
				parser_parameters_load();
				// reinitialize kinematics since some kinematics depend on settings data
				kinematics_init();
				RUNONCE_COMPLETE();
			}
#endif
			return;
		}

		proto_feedback(SD_STR_SD_PREFIX SD_STR_SD_ERROR);
	}
}

// #ifdef SD_STOP_ON_GCODE_ERROR
// // uint8_t sd_card_stop_onerror(void *args)
// OVERRIDE_EVENT_HANDLER(cnc_exec_cmd_error)
// {
// 	sd_fclose();
// 	serial_clear();
// 	// *handled = true;
// 	return EVENT_CONTINUE;
// }

// // CREATE_EVENT_LISTENER(cnc_exec_cmd_error, sd_card_stop_onerror);
// #endif

#ifdef ENABLE_MAIN_LOOP_MODULES
/**
 * Handles SD card in the main loop
 * */
bool sd_card_dotasks(void *args)
{
#if (ASSERT_PIN(SD_CARD_DETECT_PIN))
	if (mcu_get_input(SD_CARD_DETECT_PIN) && sd_card_mounted)
	{
		proto_feedback(SD_STR_SD_PREFIX SD_STR_SD_NOT_FOUND);
		if (sd_card_mounted == SD_MOUNTED)
		{
			sd_unmount(&cfs);
			fs_unmount('D');
			proto_feedback(SD_STR_SD_PREFIX SD_STR_SD_UNMOUNTED);
		}
		sd_card_mounted = SD_UNDETECTED;
		g_system_menu.flags |= SYSTEM_MENU_MODE_REDRAW;
	}
	else if (!mcu_get_input(SD_CARD_DETECT_PIN) && !sd_card_mounted)
	{
		sd_card_mounted = SD_DETECTED;
		cnc_delay_ms(2000);
		sd_card_mount();
		g_system_menu.flags |= SYSTEM_MENU_MODE_REDRAW;
	}
#endif
#ifdef ENABLE_SETTINGS_ON_SD_SDCARD
	static uint32_t retry = 0;
	// the previous write failed
	if (g_settings_error & SETTINGS_WRITE_ERROR)
	{
		if (retry < mcu_millis())
		{
			retry = mcu_millis() + 5000; // retry every 5 seconds
			// try again
			g_settings_error &= ~SETTINGS_WRITE_ERROR;
			// save all settings and parameters again
			settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
			parser_parameters_save();
		}
	}
#endif
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER_WITHLOCK(cnc_dotasks, sd_card_dotasks, SD_CARD_BUS_LOCK);

#ifdef ENABLE_SETTINGS_ON_SD_SDCARD
bool sd_card_reset_settings(void *args)
{
	if (sd_card_mounted == SD_MOUNTED)
	{
		// clear the read error
		g_settings_error &= ~SETTINGS_READ_ERROR;
		// reload all stored settings
		settings_init();
		// reload all non volatile parser parameters
		parser_parameters_load();
		// reinitialize kinematics since some kinematics depend on settings data
		kinematics_init();
	}

	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER_WITHLOCK(cnc_reset, sd_card_reset_settings, SD_CARD_BUS_LOCK);
#endif
#endif

#ifdef ENABLE_SETTINGS_ON_SD_SDCARD
fs_file_t *settings_fp;
void nvm_start_read(uint16_t address)
{
	if ((sd_card_mounted != SD_MOUNTED))
	{
		g_settings_error |= SETTINGS_READ_ERROR;
		return;
	}

	settings_fp = fs_open("/D/uCNC.cfg", "r");
	if (!settings_fp)
	{
		g_settings_error |= SETTINGS_READ_ERROR;
	}

	fs_seek(settings_fp, address);
}

void nvm_start_write(uint16_t address)
{
	if ((sd_card_mounted != SD_MOUNTED))
	{
		g_settings_error |= SETTINGS_WRITE_ERROR;
		return;
	}

	settings_fp = fs_open("/D/uCNC.cfg", "a+");
	if (!settings_fp)
	{
		g_settings_error |= SETTINGS_WRITE_ERROR;
	}

	fs_seek(settings_fp, address);
}

uint8_t nvm_getc(uint16_t address)
{
	uint8_t c = 255;
	if (settings_fp)
	{
		if (!fs_read(settings_fp, &c, 1))
		{
			g_settings_error |= SETTINGS_READ_ERROR;
		}
	}

	return c;
}

void nvm_putc(uint16_t address, uint8_t c)
{
	if (settings_fp)
	{
		if (!fs_write(settings_fp, &c, 1))
		{
			g_settings_error |= SETTINGS_WRITE_ERROR;
		}
	}
}

void nvm_end_read(void)
{
	if (settings_fp)
	{
		fs_close(settings_fp);
		settings_fp = NULL;
	}
}

void nvm_end_write(void)
{
	// same thing
	nvm_end_read();
}

#endif

#ifdef ENABLE_PARSER_MODULES
/**
 * Handles grbl commands for the SD card
 * */
bool sd_card_cmd_parser(void *args)
{
	grbl_cmd_args_t *cmd = args;

	strupr((char *)cmd->cmd);

	if (!strcmp("SDMNT", (char *)(cmd->cmd)))
	{
		sd_card_mount();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("SDUNMNT", (char *)(cmd->cmd)))
	{
		if (sd_card_mounted == SD_MOUNTED)
		{
			sd_unmount(&cfs);
			fs_unmount('D');
			sd_card_mounted = SD_DETECTED;
		}
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER_WITHLOCK(grbl_cmd, sd_card_cmd_parser, SD_CARD_BUS_LOCK);
#endif

DECL_MODULE(sd_card_v2)
{
	// starts the file system and system commands
	LOAD_MODULE(file_system);
	// STARTS SYSTEM MENU MODULE
	LOAD_MODULE(system_menu);
	// adds the sd card item to main menu
	DECL_MENU_ENTRY(1, sd_menu, "Drives", NULL, system_menu_render_fs_item, NULL, system_menu_action_fs_item, NULL);

	// sd card file system rendering menu
	DECL_DYNAMIC_MENU(10, 1, system_menu_fs_render, system_menu_fs_action);

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, sd_card_dotasks);
#ifdef ENABLE_SETTINGS_ON_SD_SDCARD
	ADD_EVENT_LISTENER(cnc_reset, sd_card_reset_settings);
#endif
#else
#warning "Main loop extensions are not enabled. SD card will not work."
#endif

#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(grbl_cmd, sd_card_cmd_parser);
#ifdef SD_STOP_ON_GCODE_ERROR
// ADD_EVENT_LISTENER(cnc_exec_cmd_error, sd_card_stop_onerror);
#endif
#else
#warning "Parser extensions are not enabled. SD card commands will not work."
#endif
}
