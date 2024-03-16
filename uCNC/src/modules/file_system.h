/*
	Name: file_system.h
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

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc.h"
#include <stddef.h>

	typedef struct fs_file_
	{
		size_t (*read)(uint8_t *, size_t);
		size_t (*write)(const uint8_t *, size_t);
		int (*available)(void);
		void (*close)(void);
		struct fs_file_* (*next_file)(char*);
		bool is_dir;
		uint32_t size;
		uint32_t timestamp;
	} fs_file_t;

	typedef struct fs_drive_
	{
		const char drive;
		fs_file_t *(*open)(char *, char *);
		void (*close)(fs_file_t *);
		struct fs_drive_ *next;
	} fs_filesystem_t;

	void fs_mount(fs_filesystem_t *drive);
	void fs_unmount(fs_filesystem_t *drive);
	fs_file_t* fs_open(char *path, char *mode);
	void fs_close(fs_file_t* file);

#ifdef __cplusplus
}
#endif

#endif