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

#ifndef FS_STR_DIR_PREFIX
#define FS_STR_DIR_PREFIX "Directory "
#endif
#ifndef FS_STR_FILE_NOT_FOUND
#define FS_STR_FILE_NOT_FOUND " not found!\n"
#endif
#ifndef FS_STR_FILE_NOT_DIR
#define FS_STR_FILE_NOT_DIR " is not a dir!\n"
#endif
#ifndef FS_FILE_READ_ERROR
#define FS_FILE_READ_ERROR "File read error!"
#endif
#ifndef FS_FILE_READ_END
#define FS_FILE_READ_END "File read finished"
#endif
#ifndef FS_FILE_RUN
#define FS_FILE_RUN "Running file from line "
#endif
#ifndef FS_MAX_PATH_LEN
#define FS_MAX_PATH_LEN 128
#endif

fs_filesystem_t *fs_default_drive;
// current working dir
static char fs_cwd[FS_MAX_PATH_LEN];
fs_file_t *fs_running_file;

static fs_filesystem_t *fs_search_drive(char *path)
{
	if (!fs_default_drive)
	{
		return NULL;
	}

	if (path[0] != '/' || path[2] != '/')
	{
		return NULL;
	}

	fs_filesystem_t *ptr = fs_default_drive;
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

#ifdef ENABLE_PARSER_MODULES
static void fs_dir_list(void)
{
	// current dir
	protocol_send_string(__romstr__(FS_STR_DIR_PREFIX));
	serial_print_str(fs_cwd);
	protocol_send_string(MSG_EOL);

	fs_file_t *dir = NULL;
	dir = fs_open(fs_cwd, "r");
	if (!dir)
	{
		protocol_send_string(__romstr__(FS_STR_FILE_NOT_FOUND));
		return;
	}

	if (!dir->is_dir)
	{
		protocol_send_string(__romstr__(FS_STR_FILE_NOT_DIR));
	}
	else
	{
		while (true)
		{
			char filename[32];
			memset(filename, 0, 32);
			fs_file_t *nextfile = dir->next_file(filename);
			if (!nextfile)
			{
				break;
			}
			if (nextfile->is_dir)
			{ /* It is a directory */
				protocol_send_string(__romstr__("<dir>\t"));
			}
			else
			{ /* It is a file. */
				protocol_send_string(__romstr__("     \t"));
			}

			uint8_t i = strlen(filename);
			for (uint8_t j = 0; j < i; j++)
			{
				serial_putc(filename);
			}
			protocol_send_string(MSG_EOL);
		}
	}

	fs_close(dir);
}

static char *fs_parentdir(void)
{
	char *tail = strrchr(fs_cwd, '/');
	if (!tail)
	{
		// is in root
		tail = fs_cwd;
	}

	*tail = 0;

	return tail;
}

// emulates basic chdir and opens the dir or file if it exists
static fs_file_t *fs_chfile(const char *newdir, char *mode)
{
	int16_t state = 0;
	uint16_t len = strlen(fs_cwd);
	char *tail = &fs_cwd[len];
	fs_file_t *fp = NULL;

	if (*newdir == '/')
	{
		// root dir
		tail = fs_cwd;
		len = 0;
		*tail = 0;
		newdir++;
	}
	else if (len)
	{
		*tail = '/';
		tail++;
	}

	for (;;)
	{
		*tail = *newdir;
		switch (*newdir)
		{
		case '/':
		case 0:
			switch (state)
			{
			case 2:
				*tail = 0;
				tail = fs_parentdir();
				// continue
			case 1:
				// deletes dot or slash
				*tail = 0;
				tail = fs_parentdir();
				*tail = *newdir;
				len = strlen(fs_cwd);
				break;
			default:
				if (state && state < FS_MAX_PATH_LEN)
				{
					// path with only dots not allowed
					*tail = 0;
					tail = fs_parentdir();
					return NULL;
				}
			}

			*tail = 0;
			fp = fs_open(fs_cwd, mode);
			// checks if is valid dir
			if (fp)
			{
				// reached the end?
				if (!*newdir)
				{
					if (len)
					{
						tail--;
						if (*tail == '/')
						{
							*tail = 0;
						}
					}

					// if it's a file rewind to the working directory
					if (!fp->is_dir)
					{
						// rewind
						tail = fs_parentdir();
					}

					return fp;
				}

				fs_close(fp);
				return NULL;
			}

			*tail = *newdir;
			state = 0;
			break;
		default:
			state++;
			state = (*newdir == '.') ? state : FS_MAX_PATH_LEN;
			break;
		}

		newdir++;
		len++;
		tail++;
		*tail = 0;

		if (len >= FS_MAX_PATH_LEN)
		{
			// clamp
			tail = strrchr(fs_cwd, '/');
			*tail = '/';
			tail++;
			*tail = 0;
			return NULL;
		}
	}

	// never reaches
	return NULL;
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

	fs_file_t *fp = fs_chfile(newdir, "r");
	if (fp)
	{
		if (strlen(fs_cwd))
		{
			serial_print_str(fs_cwd);
		}
		else
		{
			serial_putc('/');
		}
		serial_putc(">");
		fs_close(fp);
	}
	else
	{
		serial_print_str(newdir);
		protocol_send_feedback(__romstr__(FS_STR_FILE_NOT_FOUND));
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

	fs_file_t *fp = fs_chfile(file, "r");
	if (fp)
	{
		while (fp->available())
		{
			memset(file, 0, RX_BUFFER_CAPACITY);
			i = (uint8_t)fp->read(file, RX_BUFFER_CAPACITY - 1); /* Read the data */
			if (!i)
			{
				protocol_send_feedback(__romstr__(FS_FILE_READ_ERROR));
				break;
			}
			file[i] = 0;
			serial_print_str(file);
		}

		fs_close(fp);
		protocol_send_feedback(__romstr__(FS_FILE_READ_END));
		return;
	}
	else
	{
		protocol_send_feedback(__romstr__(FS_STR_FILE_NOT_FOUND));
	}

	protocol_send_string(MSG_EOL);
}

static uint8_t fs_getc(void)
{
	uint8_t c = 0;
	if (fs_running_file)
	{
		int avail = fs_running_file->available();
		if (avail)
		{
			fs_running_file->read(&c, 1);
			// auto close file
			if (--avail)
			{
				fs_close(fs_running_file);
			}
		}
	}
	return 0;
}

static uint8_t fs_available()
{
	uint8_t avail = 0;
	if (fs_running_file)
	{
		uint8_t avail = (uint8_t)MIN(255, fs_running_file->available());
	}

	return avail;
}

static void fs_clear()
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

	fs_file_t *fp = fs_chfile(file, "r");

	if (fp)
	{
		protocol_send_string(MSG_START);
		protocol_send_string(__romstr__(FS_FILE_RUN " - "));
		serial_print_int(startline);
		protocol_send_string(MSG_END);
#ifdef DECL_SERIAL_STREAM
		// open a readonly stream
		// the output is sent to the current holding interface
		fs_running_file = fp;
		serial_stream_readonly(&fs_getc, &fs_available, &fs_clear);
		while (startline)
		{
			parser_discard_command();
		}
#endif
		return;
	}

	protocol_send_feedback(__romstr__(FS_FILE_READ_ERROR));
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

void fs_mount(fs_filesystem_t *drive)
{
	if (!fs_default_drive)
	{
		fs_default_drive = drive;
		return;
	}

	fs_filesystem_t *ptr = fs_default_drive;
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

#ifdef ENABLE_PARSER_MODULES
	RUNONCE
	{
		ADD_EVENT_LISTENER(grbl_cmd, fs_cmd_parser);
		RUNONCE_COMPLETE();
	}
#else
#warning "Parser extensions are not enabled. File commands will not work."
#endif
}

void fs_unmount(fs_filesystem_t *drive)
{
	if (!fs_default_drive)
	{
		return;
	}

	fs_filesystem_t *ptr = fs_default_drive;

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

fs_file_t *fs_open(char *path, char *mode)
{
	fs_filesystem_t *fs = fs_search_drive(path);
	if (!fs)
	{
		return NULL;
	}
	return fs->open(path, mode);
}

void fs_close(fs_file_t *file)
{
	file->close();
	file = NULL;
}