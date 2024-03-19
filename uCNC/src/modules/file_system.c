/*
	Name: file_system.c
	Description: File system interface for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 16-03-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"
#include "file_system.h"
#include "endpoint.h"
#include "system_menu.h"

// file system entry point
fs_t *fs_default_drive;
// current working dir for internal file system (Grbl commands)
static fs_file_info_t fs_cwd;
// current working dir for system menu (graphic display)
static fs_file_info_t fs_sm_cwd;
// dir level for system menu (graphic display)
static uint8_t dir_level;

// current running file
fs_file_t *fs_running_file;

static char *fs_filename(fs_file_info_t *finfo);

// drive path is /<driver letter>(/<optional file path>)
static fs_t *fs_search_drive(char *path)
{
	if (!fs_default_drive || strlen(path) < 2)
	{
		return NULL;
	}

	if (path[0] != '/')
	{
		return NULL;
	}

	// should be /<drive letter>/<path>
	// or just /<drive letter>
	if (strlen(path) > 2)
	{
		// invalid path
		if (path[2] != '/')
		{
			return NULL;
		}
	}

	fs_t *ptr = fs_default_drive;
	// upper case
	char c = path[1];
	if (c > 90)
	{
		c -= 32;
	}
	do
	{
		char drv = ptr->drive;
		if (drv > 90)
		{
			drv -= 32;
		}
		if (drv == c)
		{
			return ptr;
		}

		ptr = ptr->next;
	} while (ptr);

	return NULL;
}

// emulates basic chdir and opens the dir or file if it exists
static fs_file_t *fs_path_parse(fs_file_info_t *current_path, char *new_path, const char *mode)
{
	// current_path never exceeds FS_MAX_PATH_LEN
	char full_path[FS_PATH_NAME_MAX_LEN];

	if (new_path)
	{
		if (new_path[0] == '/' || !current_path)
		{
			memset(full_path, 0, FS_PATH_NAME_MAX_LEN);
		}
		else
		{
			strncpy(full_path, current_path->full_name, FS_PATH_NAME_MAX_LEN);
		}
	}
	else
	{
		return NULL;
	}

	// full path always starts with '/'
	full_path[0] = '/';

	char *token;
	token = strtok(new_path, "/");
	while (token != NULL)
	{
		if (strcmp(token, ".") == 0)
		{
			// Do nothing, it's a reference to the current directory
		}
		else if (strcmp(token, "..") == 0)
		{
			// Move one level up
			char *tail = strrchr(full_path, '/');
			// clear the remaining string
			*(tail + 1) = 0;
		}
		else
		{
			// Add regular directory name to the path
			if (strlen(token) > FS_PATH_NAME_MAX_LEN - strlen(full_path))
			{
				// path exceeds the maximum size
				return NULL;
			}
			strcat(full_path, token);
		}

		token = strtok(NULL, "/");
	}

	// checks if is a valid drive
	fs_t *fs = fs_search_drive(full_path);
	if (!fs)
	{
		// not a valid drive
		return NULL;
	}

	// tests opening the new dir/file
	fs_file_t *fp = fs->open(&full_path[2], mode);
	if (fp)
	{
		if (current_path)
		{
			memset(current_path->full_name, 0, FS_PATH_NAME_MAX_LEN);
			// not a dir then rewind
			if (!fp->file_info.is_dir)
			{
				char *tail = strrchr(full_path, '/');
				*tail = 0;
			}
			strcpy(current_path->full_name, full_path);
		}
		return fp;
	}

	// failed to open the new dir/file
	return NULL;
}

#ifdef ENABLE_PARSER_MODULES
static void fs_dir_list(void)
{
	// if current working directory not initialized
	if (!strlen(fs_cwd.full_name))
	{
		protocol_send_string(__romstr__("Available drives"));
		protocol_send_string(MSG_EOL);
		fs_t *drive = fs_default_drive;
		while (drive)
		{
			protocol_send_string(__romstr__("<drive>\t"));
			serial_putc('/');
			serial_putc(drive->drive);
			protocol_send_string(MSG_EOL);
		}
		return;
	}

	// current dir
	protocol_send_string(__romstr__("Index of /"));
	serial_print_str(fs_filename(&fs_cwd));
	protocol_send_string(MSG_EOL);

	fs_file_t *dir = fs_open(fs_cwd.full_name, "r");

	while (true)
	{
		fs_file_info_t finfo;
		if (!fs_nextfile(dir, &finfo))
		{
			break;
		}
		if (finfo.is_dir)
		{ /* It is a directory */
			protocol_send_string(__romstr__("<dir>\t"));
		}
		else
		{ /* It is a file. */
			protocol_send_string(__romstr__("     \t"));
		}

		serial_print_str(fs_filename(&finfo));
		protocol_send_string(MSG_EOL);
	}

	fs_close(dir);
}

void fs_cd(void)
{
	uint8_t i = 0;
	char newdir[RX_BUFFER_CAPACITY]; /* File name */

	while (serial_peek() == ' ')
	{
		serial_getc();
	}

	while (serial_peek() != EOL)
	{
		newdir[i++] = serial_getc();
	}

	newdir[i] = 0;

	fs_file_t *dir = fs_path_parse(&fs_cwd, newdir, "r");
	if (dir)
	{
		if (dir->file_info.is_dir)
		{
			serial_print_str(fs_cwd.full_name);
			serial_putc('/');
			serial_putc(">");
		}
		else
		{
			serial_print_str(newdir);
			protocol_send_feedback(__romstr__(" is not a dir!"));
		}
		fs_close(dir);
	}
	else
	{
		serial_print_str(newdir);
		protocol_send_feedback(__romstr__("Dir not found!"));
	}

	protocol_send_string(MSG_EOL);
}

void fs_file_print(void)
{
	uint8_t i = 0;
	char file[RX_BUFFER_CAPACITY]; /* File name */

	while (serial_peek() == ' ')
	{
		serial_getc();
	}

	while (serial_peek() != EOL)
	{
		file[i++] = serial_getc();
	}

	file[i] = 0;

	fs_file_t *fp = fs_path_parse(&fs_cwd, file, "r");
	if (fp)
	{
		while (fs_available(fp))
		{
			memset(file, 0, RX_BUFFER_CAPACITY);
			i = (uint8_t)fs_read(fp, (uint8_t *)file, RX_BUFFER_CAPACITY - 1); /* Read the data */
			if (!i)
			{
				protocol_send_feedback(__romstr__("File read error!"));
				break;
			}
			file[i] = 0;
			serial_print_str(file);
		}

		fs_close(fp);
		protocol_send_feedback(__romstr__("File ended"));
		return;
	}
	else
	{
		protocol_send_feedback(__romstr__("File not found!"));
	}

	protocol_send_string(MSG_EOL);
}

static uint8_t running_file_getc(void)
{
	uint8_t c = 0;
	if (fs_running_file)
	{
		int avail = fs_available(fs_running_file);
		if (avail)
		{
			fs_read(fs_running_file, &c, 1);
			// auto close file
			if (--avail)
			{
				fs_close(fs_running_file);
				fs_running_file = NULL;
			}
		}
	}
	return 0;
}

static uint8_t running_file_available()
{
	uint8_t avail = 0;
	if (fs_running_file)
	{
		uint8_t avail = (uint8_t)MIN(255, fs_available(fs_running_file));
	}

	return avail;
}

static void running_file_clear()
{
	if (fs_running_file)
	{
		fs_close(fs_running_file);
	}
}

void fs_file_run(void)
{
	uint8_t i = 0;
	char args[RX_BUFFER_CAPACITY]; /* get parameters */
	char *file;
	uint32_t startline = 0;

	while (serial_peek() == ' ')
	{
		serial_getc();
	}

	while (serial_peek() != EOL)
	{
		args[i++] = serial_getc();
	}

	args[i] = 0;

	if (args[0] == '@')
	{
		startline = (uint32_t)strtol(&args[1], &file, 10);
	}

	while (*file == ' ')
	{
		file++;
	}

	fs_file_t *fp = fs_path_parse(&fs_cwd, file, "r");

	if (fp)
	{
		protocol_send_string(MSG_START);
		protocol_send_string(__romstr__("Running file from line - "));
		serial_print_int(startline);
		protocol_send_string(MSG_END);
#ifdef DECL_SERIAL_STREAM
		// open a readonly stream
		// the output is sent to the current holding interface
		fs_running_file = fp;
		serial_stream_readonly(&running_file_getc, &running_file_available, &running_file_clear);
		while (startline)
		{
			parser_discard_command();
		}
#endif
		return;
	}

	protocol_send_feedback(__romstr__("File read error!"));
}

/**
 * Handles grbl commands for the SD card
 * */
bool fs_cmd_parser(void *args)
{
	grbl_cmd_args_t *cmd = args;

	strupr((char *)cmd->cmd);

	if (!strcmp("LS", (char *)(cmd->cmd)))
	{
		fs_dir_list();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("CD", (char *)(cmd->cmd)))
	{
		fs_cd();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("LPR", (char *)(cmd->cmd)))
	{
		fs_file_print();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("RUN", (char *)(cmd->cmd)))
	{
		fs_file_run();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return GRBL_SYSTEM_CMD_EXTENDED_UNSUPPORTED;
}

CREATE_EVENT_LISTENER(grbl_cmd, fs_cmd_parser);
#endif

#ifdef MCU_HAS_ENDPOINTS

#ifndef FS_JSON_ENDPOINT
#define FS_JSON_ENDPOINT "/fs-json"
#endif
#define FS_JSON_ENDPOINT_LEN (strlen(FS_JSON_ENDPOINT))

void fs_json_api(void)
{
	fs_t *ptr = fs_default_drive;

	if (ptr)
	{
		char path[32];
		endpoint_send(200, NULL, NULL);
		endpoint_send(200, "application/json", "{\"result\":\"ok\",\"path\":\"\",\"data\":[");
		while (ptr)
		{
			memset(path, 0, sizeof(path));
			snprintf(path, 32, "{\"type\":\"drive\",\"name\":\"%c\"},", ptr->drive);
			ptr = ptr->next;
			if (ptr)
			{
				// trailling comma
				path[strlen(path)] = ',';
			}
			endpoint_send(200, "application/json", path);
		}
		endpoint_send(200, "application/json", "]}\n");
		// close the stream
		endpoint_send(200, "application/json", NULL);
	}
	else
	{
		endpoint_send(200, "application/json", "{\"result\":\"ok\",\"path\":\"\",\"data\":[]}");
	}
}

void fs_file_json_api()
{
	bool update = false;

	if (endpoint_request_hasargs())
	{
		char arg = 0;
		endpoint_request_arg("update", &arg, 1);
		if (arg && arg != '0')
		{
			update = true;
		}
	}

	// updated page
	if (update && endpoint_request_method() == ENDPOINT_GET)
	{
		char updatepage[FS_WRITE_GZ_SIZE];
		rom_memcpy(updatepage, fs_write_page, FS_WRITE_GZ_SIZE);
		endpoint_send_header("Content-Encoding", "gzip", true);
		endpoint_send(200, "text/html", updatepage);
		return;
	}

	char urlpath[256];
	memset(urlpath, 0, sizeof(urlpath));
	endpoint_request_uri(urlpath, 256);
	char *fs_url = &urlpath[FS_JSON_ENDPOINT_LEN];

	fs_file_t *file = fs_open(fs_url, "r");

	if (!file)
	{
		endpoint_send(404, "application/json", "{\"result\":\"notfound\"}");
		return;
	}

	char args[128];
	memset(args, 0, sizeof(args));

	switch (endpoint_request_method())
	{
	case ENDPOINT_DELETE:
		fs_remove(fs_url);
		__FALL_THROUGH__
	case ENDPOINT_PUT:
	case ENDPOINT_POST:
		endpoint_request_arg("redirect", args, 128);
		if (strlen(args))
		{
			endpoint_send_header("Location", args, true);
			memset(urlpath, 0, sizeof(urlpath));
			snprintf(urlpath, 256, "{\"redirect\":\"%s\"}", args);
			endpoint_send(303, "application/json", urlpath);
		}
		else
		{
			endpoint_send(200, "application/json", "{\"result\":\"ok\"}");
		}

		break;
	default: // handle as get
		if (file->file_info.is_dir)
		{
			// start chunck transmition;
			endpoint_send(200, NULL, NULL);
			endpoint_send(200, "application/json", "{\"result\":\"ok\",\"path\":\"");
			endpoint_send(200, "application/json", fs_url);
			endpoint_send(200, "application/json", "\",\"data\":[");
			fs_file_info_t child = {0};

			while (fs_nextfile(file, &child))
			{
				memset(urlpath, 0, 256);
				if (child.is_dir)
				{
					snprintf(urlpath, 256, "{\"type\":\"dir\",\"name\":\"%s\",\"attr\":%d},", fs_filename(&child), 0);
				}
				else
				{
					snprintf(urlpath, 256, "{\"type\":\"file\",\"name\":\"%s\",\"attr\":0,\"size\":%d,\"date\":0}", fs_filename(&child), child.size);
				}

				if (fs_nextfile(file, &child))
				{
					// trailling comma
					urlpath[strlen(urlpath)] = ',';
				}
				endpoint_send(200, "application/json", urlpath);
			}
			endpoint_send(200, "application/json", "]}\n");
			// close the stream
			endpoint_send(200, "application/json", NULL);
		}
		else
		{
			char content[ENDPOINT_MAX_CHUNCK_LEN / sizeof(char)];
			if (file->file_info.size > ENDPOINT_MAX_CHUNCK_LEN)
			{
				endpoint_send(200, "application/octet-stream", NULL);
			}
			while (fs_available(file))
			{
				fs_read(file, (uint8_t *)content, ENDPOINT_MAX_CHUNCK_LEN);
				endpoint_send(200, "application/octet-stream", content);
			}
			// close the stream
			endpoint_send(200, "application/octet-stream", "");
		}
		break;
	}

	fs_close(file);
}

void fs_json_uploader()
{
}

#endif

static void system_menu_render_fs_item(uint8_t render_flags, system_menu_item_t *item)
{
	char buffer[SYSTEM_MENU_MAX_STR_LEN];

	if (!fs_default_drive)
	{
		rom_strcpy(buffer, __romstr__(FS_STR_UNMOUNTED));
	}
	else if (fs_running_file)
	{
		rom_strcpy(buffer, __romstr__(FS_STR_FILE_RUNNING));
	}
	else
	{
		rom_strcpy(buffer, __romstr__(FS_STR_MOUNTED));
	}

	system_menu_item_render_arg(render_flags, buffer);
}

static bool system_menu_action_fs_item(uint8_t action, system_menu_item_t *item)
{
	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		if (fs_running_file)
		{
			// currently running job
			// do nothing
		}
		else
		{
			// go back to root dir
			fs_file_t *fp = fs_path_parse(&fs_sm_cwd, "/", "r");
			fs_close(fp);
			dir_level = 0;
			// goto sd card menu
			g_system_menu.current_menu = 10;
			g_system_menu.current_index = 0;
			g_system_menu.current_multiplier = 0;
			g_system_menu.flags &= ~(SYSTEM_MENU_MODE_EDIT | SYSTEM_MENU_MODE_MODIFY);
		}

		return true;
	}

	return false;
}

// dynamic rendering of the sd card menu
// lists all dirs and files
static void system_menu_sd_card_render(uint8_t render_flags)
{
	uint8_t cur_index = g_system_menu.current_index;

	if (render_flags & SYSTEM_MENU_MODE_EDIT)
	{
		system_menu_render_header(current_file.fname);
		char buffer[SYSTEM_MENU_MAX_STR_LEN];
		memset(buffer, 0, SYSTEM_MENU_MAX_STR_LEN);
		rom_strcpy(buffer, __romstr__(SD_STR_FILE_PREFIX SD_STR_SD_CONFIRM));
		system_menu_item_render_label(render_flags, buffer);
		system_menu_item_render_arg(render_flags, current_file.fname);
	}
	else
	{
		FRESULT res;
		FILINFO fno;
		DIR dp;

		// current dir
		if (!strlen(cwd))
		{
			system_menu_render_header("/\0");
		}
		else
		{
			char *last_slash = strrchr(cwd, '/');
			if (last_slash == NULL)
			{
				last_slash = cwd;
			}
			else
			{
				last_slash++;
			}
			system_menu_render_header(last_slash);
		}
		uint8_t index = 0;
		if (sd_opendir(&dp, cwd) == FR_OK)
		{
			for (;;)
			{
				res = sd_readdir(&dp, &fno); /* Read a directory item */
				if (res != FR_OK || fno.fname[0] == 0)
				{
					break; /* Break on error or end of dir */
				}

				if (system_menu_render_menu_item_filter(index))
				{
					char buffer[SYSTEM_MENU_MAX_STR_LEN];
					memset(buffer, 0, SYSTEM_MENU_MAX_STR_LEN);
					buffer[0] = (fno.fattrib & AM_DIR) ? '/' : ' ';
					memcpy(&buffer[1], fno.fname, MIN(SYSTEM_MENU_MAX_STR_LEN - 1, strlen(fno.fname)));
					system_menu_item_render_label(render_flags | ((cur_index == index) ? SYSTEM_MENU_MODE_SELECT : 0), buffer);
					// stores the current file info
					if ((cur_index == index))
					{
						memcpy(&current_file, &fno, sizeof(FILINFO));
					}
				}
				index++;
			}
			g_system_menu.total_items = index;
			sd_closedir(&dp);
		}
	}

	system_menu_render_nav_back((g_system_menu.current_index < 0 || g_system_menu.current_multiplier < 0));
	system_menu_render_footer();
}

bool system_menu_sd_card_action(uint8_t action)
{
	uint8_t render_flags = g_system_menu.flags;
	bool go_back = (g_system_menu.current_index < 0 || g_system_menu.current_multiplier < 0);

	// selects a file or a dir
	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		char buffer[SYSTEM_MENU_MAX_STR_LEN];

		if (render_flags & SYSTEM_MENU_MODE_EDIT)
		{
			// file print or quit
			// if it's over the nav back element
			if (go_back)
			{
				// don't run file and return to render sd content
				g_system_menu.flags &= ~SYSTEM_MENU_MODE_EDIT;
			}
			else
			{
				// run file
				if (sd_chfile(current_file.fname, FA_READ) == FR_OK)
				{
					file_runs = 1;
					protocol_send_string(MSG_START);
					protocol_send_string(__romstr__(SD_STR_FILE_PREFIX SD_STR_SD_RUNNING " - "));
					serial_print_int(file_runs);
					protocol_send_string(MSG_END);
					system_menu_go_idle();
					rom_strcpy(buffer, __romstr__(SD_STR_FILE_PREFIX SD_STR_SD_RUNNING));
					system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
				}
				else
				{
					rom_strcpy(buffer, __romstr__(SD_STR_FILE_PREFIX SD_STR_SD_FAILED));
					system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
					sd_fclose();
				}
			}
		}
		else
		{
			if (go_back)
			{
				if (dir_level)
				{
					// up one dirs
					sd_chfile("..", 0);
					g_system_menu.current_index = 0;
					g_system_menu.current_multiplier = 0;
					g_system_menu.total_items = 0;
					dir_level--;
					return true;
				}
				else
				{
					// return back let system menu handle it
					return false;
				}
			}

			if (current_file.fname[0] != 0)
			{
				if ((current_file.fattrib & AM_DIR))
				{
					if (sd_chfile(current_file.fname, 0) == FR_OK)
					{
						g_system_menu.current_index = 0;
						g_system_menu.current_multiplier = 0;
						g_system_menu.total_items = 0;
						dir_level++;
					}
					else
					{
						rom_strcpy(buffer, __romstr__(SD_STR_SD_PREFIX SD_STR_SD_ERROR));
						system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
					}
				}
				else
				{
					// go to file run or quit menu
					g_system_menu.current_multiplier = 0;
					g_system_menu.flags |= SYSTEM_MENU_MODE_EDIT;
				}
			}
			else
			{
				rom_strcpy(buffer, __romstr__(SD_STR_SD_PREFIX SD_STR_SD_ERROR));
				system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
			}
		}

		return true;
	}

	return false;
}

void fs_mount(fs_t *drive)
{
	if (!fs_default_drive)
	{
		fs_default_drive = drive;
		return;
	}

	fs_t *ptr = fs_default_drive;
	do
	{
		if (ptr->drive == drive->drive)
		{
			return;
		}
		if (!ptr->next)
		{
			break;
		}
	} while (1);

	ptr->next = drive;
	drive->next = NULL;

	RUNONCE
	{
#ifdef ENABLE_PARSER_MODULES
		ADD_EVENT_LISTENER(grbl_cmd, fs_cmd_parser);
#else
#warning "Parser extensions are not enabled. File commands will not work."
#endif

#ifdef MCU_HAS_ENDPOINTS
		endpoint_add(FS_JSON_ENDPOINT, ENDPOINT_ANY, fs_json_api, NULL);
		endpoint_add(FS_JSON_ENDPOINT "/*", ENDPOINT_ANY, fs_file_json_api, fs_json_uploader);
#endif
		RUNONCE_COMPLETE();
	}
}

void fs_unmount(fs_t *drive)
{
	if (!fs_default_drive)
	{
		return;
	}

	fs_t *ptr = fs_default_drive;

	if (ptr->drive == drive->drive)
	{
		fs_default_drive = fs_default_drive->next;
		drive->next = NULL;
	}

	while (ptr->next->drive != drive->drive)
	{
		ptr = ptr->next;
	}

	if (ptr->next->drive == drive->drive)
	{
		ptr->next = drive->next;
		drive->next = NULL;
	}
}

fs_file_t *fs_open(char *path, const char *mode)
{
	return fs_path_parse(NULL, path, mode);
}

void fs_close(fs_file_t *fp)
{
	if (fp)
	{
		fp->fs_ptr->close(fp);
	}
}

size_t fs_read(fs_file_t *fp, uint8_t *buffer, size_t len)
{
	if (fp)
	{
		return fp->fs_ptr->read(fp, buffer, len);
	}

	return 0;
}

size_t fs_write(fs_file_t *fp, const uint8_t *buffer, size_t len)
{
	if (fp)
	{
		return fp->fs_ptr->write(fp, buffer, len);
	}

	return 0;
}

int fs_available(fs_file_t *fp)
{
	if (fp)
	{
		return fp->fs_ptr->available(fp);
	}

	return 0;
}

bool fs_nextfile(fs_file_t *fp, fs_file_info_t *finfo)
{
	if (fp)
	{
		return fp->fs_ptr->next_file(fp, finfo);
	}

	return false;
}

static char *fs_filename(fs_file_info_t *finfo)
{
	if (finfo)
	{
		return (strrchr(finfo->full_name, '/') + 1);
	}

	return "";
}

bool fs_remove(char *path)
{
	fs_t *fs = fs_search_drive(path);
	if (fs)
	{
		return fs->remove(&path[2]);
	}

	return false;
}
