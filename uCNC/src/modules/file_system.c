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
#include <string.h>

#ifndef freefile_ptr
#define freefile_ptr(x) fs_safe_free(x)
#endif

// file system entry point
fs_t *fs_default_drive;
// current working dir for internal file system (Grbl commands)
static fs_file_info_t fs_cwd __attribute__((unused));
// current working dir for system menu (graphic display)
static fs_file_info_t fs_sm_cwd __attribute__((unused));
// dir level for system menu (graphic display)
static uint8_t dir_level __attribute__((unused));

// current running file
fs_file_t *fs_running_file;
// current pointed file by the system menu
static fs_file_info_t fs_pointed_file;

static char *fs_filename(fs_file_info_t *finfo);

// drive path is /<driver letter>(/<optional file path>)
static fs_t *fs_search_drive(const char *path)
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
static fs_file_t *fs_path_parse(fs_file_info_t *current_path, const char *new_path, const char *mode)
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

	char *token_start = (char *)new_path;

	while (*token_start)
	{
		while (*token_start == '/')
		{
			token_start++;
		}
		char *token_end = token_start;
		int token_len = 0;
		while (*token_end != '/' && *token_end != 0)
		{
			token_end++;
			token_len++;
		}

		if (token_len)
		{
			if (strncmp(token_start, ".", token_len) == 0)
			{
				// Do nothing, it's a reference to the current directory
			}
			else if (strncmp(token_start, "..", token_len) == 0)
			{
				// Move one level up
				char *tail = strrchr(full_path, '/');
				if (tail)
				{
					// clear the remaining string
					*tail = 0;
					memset(&full_path[strlen(full_path)], 0, FS_PATH_NAME_MAX_LEN - strlen(full_path));
				}
			}
			else
			{
				// Add regular directory name to the path
				if ((token_len + 1) > FS_PATH_NAME_MAX_LEN - strlen(full_path))
				{
					// path exceeds the maximum size
					return NULL;
				}
				full_path[strlen(full_path)] = '/';
				strncat(full_path, token_start, token_len);
			}

			token_start = token_end;
		}
	}

	if (!strlen(full_path) && current_path)
	{
		memset(current_path, 0, sizeof(fs_file_info_t));
		return NULL;
	}

	// checks if is a valid drive
	fs_t *fs = fs_search_drive(full_path);
	if (!fs)
	{
		// not a valid drive
		return NULL;
	}

	fs_file_t *fp = NULL;
	char *fp_path = &full_path[2];
	// on the drive root
	if (!strlen(fp_path))
	{
		fp = fs->opendir("/");
		if (current_path)
		{
			current_path->is_dir = true;
		}
	}
	else
	{
		fp = (strcmp(mode, "d") == 0) ? fs->opendir(fp_path) : fs->open(fp_path, mode);
	}

	if (fp)
	{
		fp->fs_ptr = fs;
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

#ifdef ENABLE_MAIN_LOOP_MODULES
DECL_BUFFER(uint8_t, fs_file_buffer, RX_BUFFER_SIZE);
#endif

static uint8_t running_file_getc(void)
{
	uint8_t c = 0;
#ifdef ENABLE_MAIN_LOOP_MODULES
	BUFFER_TRY_DEQUEUE(fs_file_buffer, &c);
#else
	if (fs_running_file)
	{
		int avail = fs_available(fs_running_file);
		if (avail)
		{
			fs_read(fs_running_file, &c, 1);
			avail--;
		}
		// auto close file
		if (!avail)
		{
			fs_close(fs_running_file);
			fs_running_file = NULL;
		}
	}
#endif
	return c;
}

static uint8_t running_file_available()
{
	uint8_t avail = 0;
#ifdef ENABLE_MAIN_LOOP_MODULES
	avail = BUFFER_READ_AVAILABLE(fs_file_buffer);
#else
	if (fs_running_file)
	{
		avail = (uint8_t)MIN(255, fs_available(fs_running_file));
	}
#endif
	return avail;
}

static void running_file_clear()
{
#ifdef ENABLE_MAIN_LOOP_MODULES
	BUFFER_CLEAR(fs_file_buffer);
#endif
	if (fs_running_file)
	{
		fs_close(fs_running_file);
	}
}

#ifdef ENABLE_PARSER_MODULES

#ifdef ENABLE_MAIN_LOOP_MODULES
bool running_file_loop(void *args)
{
	if (fs_running_file)
	{
		size_t r = BUFFER_WRITE_AVAILABLE(fs_file_buffer);
		uint8_t tmp[RX_BUFFER_SIZE];
		size_t read = fs_read(fs_running_file, tmp, r);
		uint8_t w = 0;
		BUFFER_WRITE(fs_file_buffer, tmp, read, w);
		if (read < r || !fs_available(fs_running_file))
		{
			fs_close(fs_running_file);
			fs_running_file = NULL;
		}
	}

	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(cnc_dotasks, running_file_loop);
#endif

static void fs_dir_list(void)
{
	// if current working directory not initialized
	if (!strlen(fs_cwd.full_name))
	{
		proto_print("Available drives");
		proto_print(MSG_EOL);
		fs_t *drive = fs_default_drive;
		while (drive)
		{
			proto_print("<drive>\t");
			proto_putc('/');
			proto_putc(drive->drive);
			proto_print(MSG_EOL);
			drive = drive->next;
		}
		return;
	}

	// current dir
	proto_print("Index of /");
	proto_printf("%s", fs_filename(&fs_cwd));
	proto_print(MSG_EOL);

	fs_file_t *dir = fs_opendir(fs_cwd.full_name);

	if (dir)
	{
		fs_file_info_t finfo;
		while (fs_next_file(dir, &finfo))
		{
			if (finfo.is_dir)
			{ /* It is a directory */
				proto_print("<dir>\t");
			}
			else
			{ /* It is a file. */
				proto_print("     \t");
			}

			proto_printf("%s", fs_filename(&finfo));
			proto_print(MSG_EOL);
		}

		fs_close(dir);
	}
}

void fs_cd(char *params)
{
	fs_file_t *dir = fs_path_parse(&fs_cwd, params, "d");
	if (dir)
	{
		if (dir->file_info.is_dir)
		{
			proto_printf("%s >", fs_cwd.full_name);
		}
		else
		{
			proto_printf("%s  is not a dir!", params);
		}
		fs_close(dir);
	}
	else if (strlen(fs_cwd.full_name))
	{
		proto_printf("%s dir not found!", params);
	}

	proto_print(MSG_EOL);
}

void fs_file_print(char *params)
{
	fs_file_t *fp = fs_path_parse(&fs_cwd, params, "r");
	if (fp)
	{
		while (fs_available(fp))
		{
			char c = 0;
			/* Read the data */
			if (!fs_read(fp, (uint8_t *)&c, sizeof(char)))
			{
				proto_info("File read error!");
				break;
			}
			proto_putc(c);
		}

		fs_close(fp);
		proto_info("File ended");
		return;
	}
	else
	{
		proto_info("File not found!");
	}

	proto_print(MSG_EOL);
}

void fs_file_run(char *params)
{
	char *file;
	uint32_t startline = 1;
	file = params;

	if (params[0] == '@')
	{
		startline = (uint32_t)strtol(&params[1], &file, 10);
	}

	while (*file == ' ')
	{
		file++;
	}

	fs_file_t *fp = fs_path_parse(&fs_cwd, file, "r");

	if (fp)
	{
		startline = MAX(1, startline);
		proto_info("Running file from line - %lu", startline);
#ifdef DECL_SERIAL_STREAM
#ifdef ENABLE_MAIN_LOOP_MODULES
		// prefill buffer
		BUFFER_CLEAR(fs_file_buffer);
		size_t r = BUFFER_WRITE_AVAILABLE(fs_file_buffer);
		uint8_t tmp[RX_BUFFER_SIZE];
		size_t read = fs_read(fs_running_file, tmp, r);
		uint8_t w = 0;
		BUFFER_WRITE(fs_file_buffer, tmp, read, w);
#endif
		// open a readonly stream
		// the output is sent to the current holding interface
		fs_running_file = fp;
		serial_stream_readonly(&running_file_getc, &running_file_available, &running_file_clear);
		while (--startline)
		{
			parser_discard_command();
		}
#endif
		return;
	}

	proto_info("File read error!");
}

/**
 * Handles grbl commands for the SD card
 * */
bool fs_cmd_parser(void *args)
{
	grbl_cmd_args_t *cmd = (grbl_cmd_args_t *)args;
	char params[RX_BUFFER_CAPACITY]; /* get remaining command parammeters */
	memset(params, 0, sizeof(params));

	if (!strcmp("LS", (char *)(cmd->cmd)))
	{
		fs_dir_list();
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("CD", (char *)(cmd->cmd)))
	{
		int8_t len = parser_get_grbl_cmd_arg(params, RX_BUFFER_CAPACITY);

		if (len < 0)
		{
			*(cmd->error) = STATUS_INVALID_STATEMENT;
			return EVENT_HANDLED;
		}

		fs_cd(params);
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("LPR", (char *)(cmd->cmd)))
	{
		int8_t len = parser_get_grbl_cmd_arg(params, RX_BUFFER_CAPACITY);

		if (len < 0)
		{
			*(cmd->error) = STATUS_INVALID_STATEMENT;
			return EVENT_HANDLED;
		}
		fs_file_print(params);
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	if (!strcmp("RUN", (char *)(cmd->cmd)))
	{
		int8_t len = parser_get_grbl_cmd_arg(params, RX_BUFFER_CAPACITY);

		if (len < 0)
		{
			*(cmd->error) = STATUS_INVALID_STATEMENT;
			return EVENT_HANDLED;
		}
		fs_file_run(params);
		*(cmd->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
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
		endpoint_send(200, NULL, NULL, 0);
		endpoint_send_str(200, "application/json", "{\"result\":\"ok\",\"path\":\"\",\"data\":[");
		while (ptr)
		{
			memset(path, 0, sizeof(path));
			str_snprintf(path, 32, "{\"type\":\"drive\",\"name\":\"%c\"}", ptr->drive);
			ptr = ptr->next;
			if (ptr)
			{
				// trailling comma
				path[strlen(path)] = ',';
			}
			endpoint_send_str(200, "application/json", path);
		}
		endpoint_send_str(200, "application/json", "]}\n");
		// close the stream
		endpoint_send(200, "application/json", NULL, 0);
	}
	else
	{
		endpoint_send_str(200, "application/json", "{\"result\":\"ok\",\"path\":\"\",\"data\":[]}");
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
		uint8_t updatepage[FS_WRITE_GZ_SIZE];
		rom_memcpy(updatepage, fs_write_page, FS_WRITE_GZ_SIZE);
		endpoint_send_header("Content-Encoding", "gzip", true);
		endpoint_send(200, "text/html", updatepage, FS_WRITE_GZ_SIZE);
		return;
	}

	char urlpath[256];
	memset(urlpath, 0, sizeof(urlpath));
	endpoint_request_uri(urlpath, 256);
	char *fs_url = &urlpath[FS_JSON_ENDPOINT_LEN];

	fs_file_info_t finfo;
	fs_file_t *file;

	if (!fs_finfo(fs_url, &finfo))
	{
		endpoint_send_str(404, "application/json", "{\"result\":\"notfound\"}");
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
			str_snprintf(urlpath, 256, "{\"redirect\":\"%s\"}", args);
			endpoint_send_str(303, "application/json", urlpath);
		}
		else
		{
			endpoint_send_str(200, "application/json", "{\"result\":\"ok\"}");
		}

		break;
	default: // handle as get
		// is file
		if (strlen(finfo.full_name) || finfo.is_dir)
		{
			if (finfo.is_dir)
			{
				// start chunck transmition;
				endpoint_send(200, NULL, NULL, 0);
				endpoint_send_str(200, "application/json", "{\"result\":\"ok\",\"path\":\"");
				endpoint_send_str(200, "application/json", fs_url);
				endpoint_send_str(200, "application/json", "\",\"data\":[");
				fs_file_info_t child = {0};
				file = fs_opendir(fs_url);
				bool has_child = fs_next_file(file, &child);

				while (has_child)
				{
					memset(urlpath, 0, 256);
					if (child.is_dir)
					{
						str_snprintf(urlpath, 256, "{\"type\":\"dir\",\"name\":\"%s\",\"attr\":%d}", fs_filename(&child), 0);
					}
					else
					{
						str_snprintf(urlpath, 256, "{\"type\":\"file\",\"name\":\"%s\",\"attr\":0,\"size\":%d,\"date\":0}", fs_filename(&child), child.size);
					}

					has_child = fs_next_file(file, &child);
					if (has_child)
					{
						// trailling comma
						urlpath[strlen(urlpath)] = ',';
					}
					endpoint_send_str(200, "application/json", urlpath);
				}
				endpoint_send_str(200, "application/json", "]}\n");
				// close the stream
				endpoint_send(200, "application/json", NULL, 0);
				fs_close(file);
			}
			else
			{
				file = fs_open(fs_url, "r");
				uint8_t content[ENDPOINT_MAX_CHUNCK_LEN / sizeof(char)];
				if (finfo.size > ENDPOINT_MAX_CHUNCK_LEN)
				{
					endpoint_send(200, NULL, NULL, 0);
				}
				while (fs_available(file))
				{
					size_t content_len = fs_read(file, (uint8_t *)content, ENDPOINT_MAX_CHUNCK_LEN / sizeof(char));
					endpoint_send(200, "application/octet-stream", content, content_len);
				}
				// close the stream
				endpoint_send(200, "application/octet-stream", NULL, 0);
				fs_close(file);
			}
		}
		else
		{
			// is root dir
			fs_json_api();
		}
		break;
	}
}

void fs_json_uploader()
{
	static fs_file_t *file_upload = NULL;
	char urlpath[256];
	memset(urlpath, 0, sizeof(urlpath));
	endpoint_request_uri(urlpath, 256);
	if ((strncmp(urlpath, FS_JSON_ENDPOINT, FS_JSON_ENDPOINT_LEN) != 0) || (endpoint_request_method() != ENDPOINT_POST && endpoint_request_method() != ENDPOINT_PUT))
	{
		return;
	}

	char *file = &urlpath[FS_JSON_ENDPOINT_LEN];

	fs_file_info_t finfo;

	if (!fs_finfo(file, &finfo))
	{
		return;
	}

	endpoint_upload_t upload = endpoint_file_upload_status();
	switch (upload.status)
	{
	case ENDPOINT_UPLOAD_START:
		if (endpoint_request_method() == ENDPOINT_POST)
		{
			uint8_t len = strlen(urlpath);
			if (urlpath[len - 1] != '/' && len < 256)
			{
				urlpath[len] = '/';
				len++;
			}
			// append the file name
			endpoint_file_upload_name(urlpath, 256);
		}
		file_upload = fs_open(file, "w");
		break;
	case ENDPOINT_UPLOAD_PART:
		fs_write(file_upload, upload.data, upload.datalen);
		break;
	default:
		fs_close(file_upload);
		file_upload = NULL;
	}
}

#endif

#ifndef DISABLE_SYSTEM_MENU
void system_menu_render_fs_item(uint8_t render_flags, system_menu_item_t *item)
{
	char buffer[SYSTEM_MENU_MAX_STR_LEN];
	memset(buffer, 0, sizeof(buffer));

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

bool system_menu_action_fs_item(uint8_t action, system_menu_item_t *item)
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
			memset(&fs_sm_cwd, 0, sizeof(fs_sm_cwd));
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
void system_menu_fs_render(uint8_t render_flags)
{
	uint8_t cur_index = g_system_menu.current_index;

	if (render_flags & SYSTEM_MENU_MODE_EDIT)
	{
		system_menu_render_header(fs_filename(&fs_sm_cwd));
		char buffer[SYSTEM_MENU_MAX_STR_LEN];
		memset(buffer, 0, sizeof(buffer));
		rom_strcpy(buffer, __romstr__(FS_STR_FILE_PREFIX FS_STR_SD_CONFIRM));
		system_menu_item_render_label(render_flags, buffer);
		system_menu_item_render_arg(render_flags, fs_filename(&fs_pointed_file));
	}
	else
	{
		uint8_t index = 0;
		// current dir
		if (!strlen(fs_sm_cwd.full_name))
		{
			system_menu_render_header("<Drives>");
			fs_t *drive = fs_default_drive;
			while (drive)
			{
				if (system_menu_render_menu_item_filter(index))
				{
					char buffer[3];
					memset(buffer, 0, sizeof(buffer));
					buffer[0] = '/';
					buffer[1] = drive->drive;
					system_menu_item_render_label(render_flags | ((cur_index == index) ? SYSTEM_MENU_MODE_SELECT : 0), buffer);
					if ((cur_index == index))
					{
						memset(&fs_pointed_file, 0, sizeof(fs_pointed_file));
						strcpy(fs_pointed_file.full_name, buffer);
						fs_pointed_file.is_dir = true;
					}
				}
				index++;
				drive = drive->next;
			}
		}
		else
		{
			system_menu_render_header(fs_filename(&fs_sm_cwd));
			fs_file_t *dir = fs_path_parse(&fs_sm_cwd, ".", "d");
			if (dir)
			{
				if (dir->file_info.is_dir)
				{
					fs_file_info_t finfo = {0};

					while (fs_next_file(dir, &finfo))
					{
						if (system_menu_render_menu_item_filter(index))
						{
							char buffer[SYSTEM_MENU_MAX_STR_LEN];
							memset(buffer, 0, sizeof(buffer));
							buffer[0] = (finfo.is_dir) ? '/' : ' ';
							memcpy(&buffer[1], fs_filename(&finfo), MIN(SYSTEM_MENU_MAX_STR_LEN - 1, strlen(fs_filename(&finfo))));
							system_menu_item_render_label(render_flags | ((cur_index == index) ? SYSTEM_MENU_MODE_SELECT : 0), buffer);
							// stores the current file info
							if ((cur_index == index))
							{
								fs_pointed_file = finfo;
								// memcpy(&current_file, &fno, sizeof(FILINFO));
							}
						}
						index++;
					}
				}
			}

			fs_close(dir);
		}

		g_system_menu.total_items = index;
	}

	system_menu_render_nav_back((g_system_menu.current_index < 0 || g_system_menu.current_multiplier < 0));
	system_menu_render_footer();
}

bool system_menu_fs_action(uint8_t action)
{
	uint8_t render_flags = g_system_menu.flags;
	bool go_back = (g_system_menu.current_index < 0 || g_system_menu.current_multiplier < 0);

	// selects a file or a dir
	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		char buffer[SYSTEM_MENU_MAX_STR_LEN];
		memset(buffer, 0, sizeof(buffer));
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
				fs_running_file = fs_path_parse(&fs_sm_cwd, fs_filename(&fs_pointed_file), "r");
				if (fs_running_file)
				{
					proto_feedback(FS_STR_FILE_RUNNING);
					system_menu_go_idle();
					rom_strcpy(buffer, __romstr__(FS_STR_FILE_RUNNING));
					system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
					grbl_stream_readonly(&running_file_getc, &running_file_available, &running_file_clear);
				}
				else
				{
					rom_strcpy(buffer, __romstr__(FS_STR_FILE_FAILED));
					system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
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
					fs_file_t *fp = fs_path_parse(&fs_sm_cwd, "..", "d");
					if (fp)
					{
						fs_close(fp);
					}
					fs_pointed_file = fs_sm_cwd;
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

			if (strlen(fs_pointed_file.full_name) != 0)
			{
				if (fs_pointed_file.is_dir)
				{
					fs_file_t *fp = fs_path_parse(&fs_sm_cwd, fs_filename(&fs_pointed_file), "d");
					if (fp)
					{
						fs_close(fp);
						fs_pointed_file = fs_sm_cwd;
						g_system_menu.current_index = 0;
						g_system_menu.current_multiplier = 0;
						g_system_menu.total_items = 0;
						dir_level++;
					}
					else
					{
						rom_strcpy(buffer, __romstr__(FS_STR_FILE_FAILED));
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
				rom_strcpy(buffer, __romstr__(FS_STR_FILE_FAILED));
				system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
			}
		}

		return true;
	}

	return false;
}
#endif

void fs_mount(fs_t *drive)
{
	if (!drive)
	{
		return;
	}

	// upcase
	if (drive->drive > 90)
	{
		drive->drive -= 32;
	}

	if (!fs_default_drive)
	{
		fs_default_drive = drive;
	}
	else
	{
		fs_t *ptr = fs_default_drive;
		do
		{
			if (ptr->drive == drive->drive)
			{
				break;
			}
			if (!ptr->next)
			{
				ptr->next = drive;
				drive->next = NULL;
				break;
			}
		} while (1);
	}

	RUNONCE
	{
#ifdef MCU_HAS_ENDPOINTS
		endpoint_add(FS_JSON_ENDPOINT, ENDPOINT_ANY, fs_file_json_api, fs_json_uploader);
		endpoint_add(FS_JSON_ENDPOINT "/*", ENDPOINT_ANY, fs_file_json_api, fs_json_uploader);
#endif
		RUNONCE_COMPLETE();
	}
}

DECL_MODULE(file_system)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(grbl_cmd, fs_cmd_parser);
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, running_file_loop);
#else
#warning "Main loop extensions are not enabled. File running might be slower."
#endif
#else
#warning "Parser extensions are not enabled. File commands will not work."
#endif
}

void fs_unmount(char drive)
{
	if (!fs_default_drive)
	{
		return;
	}

	fs_t *unmt = fs_default_drive;

	// unmount default drive
	if (fs_default_drive->drive == drive)
	{
		fs_default_drive = fs_default_drive->next;
		return;
	}

	// search drive
	while (unmt)
	{
		if (unmt->drive == drive)
		{
			break;
		}
		unmt = unmt->next;
	}

	// unmount drive
	if (unmt)
	{
		fs_t *prev = fs_default_drive;
		while (prev->next != unmt)
		{
			prev = prev->next;
		}

		// unmount
		prev->next = unmt->next;
	}
}

fs_file_t *fs_open(const char *path, const char *mode)
{
	return fs_path_parse(NULL, path, mode);
}

fs_file_t *fs_opendir(const char *path)
{
	return fs_path_parse(NULL, path, "d");
}

void fs_close(fs_file_t *fp)
{
	if (fp)
	{
		if (fp->file_ptr)
		{
			fp->fs_ptr->close(fp);
			freefile_ptr(fp->file_ptr);
		}
		fs_safe_free(fp);
	}
}

size_t fs_read(fs_file_t *fp, uint8_t *buffer, size_t len)
{
	if (fp)
	{
		if (fp->file_ptr)
		{
			return fp->fs_ptr->read(fp, buffer, len);
		}
	}

	return 0;
}

size_t fs_write(fs_file_t *fp, const uint8_t *buffer, size_t len)
{
	if (fp)
	{
		if (fp->file_ptr)
		{
			return fp->fs_ptr->write(fp, buffer, len);
		}
	}

	return 0;
}

bool fs_seek(fs_file_t *fp, uint32_t position)
{
	if (fp)
	{
		if (fp->file_ptr)
		{
			return fp->fs_ptr->seek(fp, position);
		}
	}

	return false;
}

int fs_available(fs_file_t *fp)
{
	if (fp)
	{
		if (fp->file_ptr)
		{
			return fp->fs_ptr->available(fp);
		}
	}

	return 0;
}

bool fs_next_file(fs_file_t *fp, fs_file_info_t *finfo)
{
	if (fp)
	{
		if (fp->file_ptr)
		{
			if (finfo)
			{
				memset(finfo, 0, sizeof(fs_file_info_t));
			}
			return fp->fs_ptr->next_file(fp, finfo);
		}
	}

	return false;
}

bool fs_finfo(const char *path, fs_file_info_t *finfo)
{
	if (!finfo)
	{
		return false;
	}

	memset(finfo, 0, sizeof(fs_file_info_t));

	// file system root
	if (strlen(path) <= 1)
	{
		return true;
	}

	fs_t *fs = fs_search_drive(path);
	if (!fs)
	{
		return false;
	}

	if (!strlen(&path[2]))
	{
		return fs->finfo("/", finfo);
	}

	return fs->finfo(&path[2], finfo);
}

static char *fs_filename(fs_file_info_t *finfo)
{
	if (finfo)
	{
		char *name = strrchr(finfo->full_name, '/');
		name = (!name) ? (finfo->full_name) : (name + 1);
		return name;
	}

	return (char *)"";
}

bool fs_remove(const char *path)
{
	fs_t *fs = fs_search_drive(path);
	if (fs)
	{
		return fs->remove(&path[2]);
	}

	return false;
}

bool fs_mkdir(const char *path)
{
	fs_t *fs = fs_search_drive(path);
	if (fs)
	{
		return fs->mkdir(&path[2]);
	}
	return false;
}

bool fs_rmdir(const char *path)
{
	fs_t *fs = fs_search_drive(path);
	if (fs)
	{
		return fs->rmdir(&path[2]);
	}
	return false;
}
