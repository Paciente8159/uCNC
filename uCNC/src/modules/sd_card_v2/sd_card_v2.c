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
#include "../file_system.h"
#include "sd_messages.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define PETIT_FAT_FS 1
#define FAT_FS 2

#ifdef ENABLE_SETTINGS_MODULES
#ifdef SD_FAT_FS
#undef SD_FAT_FS
#endif
#define SD_FAT_FS FAT_FS
#endif

#ifndef SD_FAT_FS
#define SD_FAT_FS PETIT_FAT_FS
#endif

#if (UCNC_MODULE_VERSION < 10980 || UCNC_MODULE_VERSION > 99999)
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
} FIL;

// file system
static FATFS cfs;
// current opend file
static FIL cwf;

static FORCEINLINE FRESULT sd_mount(FATFS *fs)
{
	cwd[0] = 0;
	return pf_mount(fs);
}

#define sd_unmount(fs) (cwd[0] = 0)

static FORCEINLINE FRESULT sd_fopen(const char *name, BYTE mode)
{
	if (pf_open(cwd) == FR_OK)
	{
		cwf.fs = &cfs;
		return FR_OK;
	}

	return FR_NO_FILE;
}

static FORCEINLINE FRESULT sd_fread(char *buff, UINT btr, UINT *br)
{
	if (!cwf.fs)
	{
		return FR_NOT_OPENED;
	}

	return pf_read(buff, btr, br);
}

static FORCEINLINE FRESULT sd_fwrite(const char *buff, UINT btw, UINT *bw)
{
	if (!cwf.fs)
	{
		return FR_NOT_OPENED;
	}
	return pf_write(buff, btw, bw);
}

static FORCEINLINE FRESULT sd_fseek(DWORD ofs)
{
	if (!cwf.fs)
	{
		return FR_NOT_OPENED;
	}
	return pf_lseek(ofs);
}

static FORCEINLINE FRESULT sd_fclose()
{
	if (cwf.fs)
	{
		cwf.fs = NULL;
		return sd_chfile("..", 0);
	}

	return FR_NOT_OPENED;
}
#define sd_fsync()
static FORCEINLINE FRESULT sd_opendir(DIR *dp, const char *path) { pf_opendir(dj, path); }
static FORCEINLINE FRESULT sd_readdir(DIR *dp, FILINFO *fno) { pf_readdir(dj, fno); }
#define sd_closedir(fno)
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
	if(strlen(path)<2){
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

	if (!sd_fs_finfo(file, &(fp->file_info)))
	{
		free(fp);
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
		modebyte |= FA_WRITE;
	}
	if (strchr(mode, 'a'))
	{
		modebyte |= FA_OPEN_APPEND;
	}
	if (strchr(mode, '+'))
	{
		modebyte |= FA_CREATE_NEW;
	}

	if (sd_fopen(fp->file_ptr, file, modebyte) != FR_OK)
	{
		free(fp->file_ptr);
		free(fp);
		return NULL;
	}

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
			f_closedir(fp->file_ptr);
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
			protocol_send_feedback(__romstr__(SD_STR_SD_PREFIX SD_STR_SD_MOUNTED));
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
#ifdef ENABLE_SETTINGS_MODULES
			settings_init();
#endif
			return;
		}

		protocol_send_feedback(__romstr__(SD_STR_SD_PREFIX SD_STR_SD_ERROR));
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
		protocol_send_feedback(__romstr__(SD_STR_SD_PREFIX SD_STR_SD_NOT_FOUND));
		if (sd_card_mounted == SD_MOUNTED)
		{
			sd_unmount(&cfs);
			fs_unmount('D');
			protocol_send_feedback(__romstr__(SD_STR_SD_PREFIX SD_STR_SD_UNMOUNTED));
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
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER_WITHLOCK(cnc_dotasks, sd_card_dotasks, SD_CARD_BUS_LOCK);

#endif

#ifdef ENABLE_SETTINGS_MODULES
bool sd_settings_load(void *args)
// OVERRIDE_EVENT_HANDLER(settings_load)
{
	if ((sd_card_mounted != SD_MOUNTED))
	{
		return EVENT_CONTINUE;
	}

	UINT i = 0;
	bool result = EVENT_CONTINUE;
	settings_args_t *p = args;
	fs_file_t *fp = fs_open("/D/uCNC.cfg", "r");

	if (fp)
	{
		protocol_send_feedback(__romstr__(SD_STR_SETTINGS_FOUND));
		fs_seek(fp, p->address);
		i = fs_read(fp, p->data, p->size);
		if (p->size == i)
		{
			protocol_send_feedback(__romstr__(SD_STR_SETTINGS_LOADED));
			result = EVENT_HANDLED;
		}
	}
	else
	{
		protocol_send_feedback(__romstr__(SD_STR_SETTINGS_NOT_FOUND));
	}

	fs_close(fp);

	return result;
}

CREATE_EVENT_LISTENER_WITHLOCK(settings_load, sd_settings_load, SD_CARD_BUS_LOCK);

bool sd_settings_save(void *args)
// OVERRIDE_EVENT_HANDLER(settings_save)
{
	if ((sd_card_mounted != SD_MOUNTED))
	{
		return EVENT_CONTINUE;
	}

	UINT i = 0;
	bool result = EVENT_CONTINUE;
	settings_args_t *p = args;

	fs_file_t *fp = fs_open("/D/uCNC.cfg", "a+");

	if (fp)
	{
		fs_seek(fp, p->address);
		i = fs_write(fp, p->data, p->size);
		if (p->size == i)
		{
			protocol_send_feedback(__romstr__(SD_STR_SETTINGS_SAVED));
			result = EVENT_HANDLED;
		}
	}

	fs_close(fp);

	return result;
}

CREATE_EVENT_LISTENER_WITHLOCK(settings_save, sd_settings_save, SD_CARD_BUS_LOCK);

bool sd_settings_erase(void *args)
// OVERRIDE_EVENT_HANDLER(settings_erase)
{
	bool result = EVENT_CONTINUE;

	if ((sd_card_mounted != SD_MOUNTED))
	{
		return EVENT_CONTINUE;
	}

	fs_file_t *fp = fs_open("/D/uCNC.cfg", "w");
	if (fp)
	{
		protocol_send_feedback(__romstr__(SD_STR_SETTINGS_ERASED));
		result = EVENT_HANDLED;
	}

	fs_close(fp);

	return result;
}

CREATE_EVENT_LISTENER_WITHLOCK(settings_erase, sd_settings_erase, SD_CARD_BUS_LOCK);
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
	//starts the file system and system commands
	LOAD_MODULE(file_system);
	// STARTS SYSTEM MENU MODULE
	LOAD_MODULE(system_menu);
	// adds the sd card item to main menu
	DECL_MENU_ENTRY(1, sd_menu, "Drives", NULL, system_menu_render_fs_item, NULL, system_menu_action_fs_item, NULL);

	// sd card file system rendering menu
	DECL_DYNAMIC_MENU(10, 1, system_menu_fs_render, system_menu_fs_action);

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, sd_card_dotasks);
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
#ifdef ENABLE_SETTINGS_MODULES
	ADD_EVENT_LISTENER(settings_load, sd_settings_load);
	ADD_EVENT_LISTENER(settings_save, sd_settings_save);
	ADD_EVENT_LISTENER(settings_erase, sd_settings_erase);
#else
#warning "Settings extension not enabled. SD card stored settings will not work."
#endif
}
