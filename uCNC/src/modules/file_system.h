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

#ifndef FS_FILE_NAME_MAX_LEN
#define FS_FILE_NAME_MAX_LEN 32
#endif
#ifndef FS_PATH_NAME_MAX_LEN
#define FS_PATH_NAME_MAX_LEN 256
#endif

	typedef struct fs_file_info_
	{
		char name[FS_FILE_NAME_MAX_LEN];
		bool is_dir;
		uint32_t size;
		uint32_t timestamp;
	}fs_file_info_t;

	typedef struct fs_file_
	{
		fs_file_info_t file_info;
		struct fs_* fs_ptr;
		void* file_ptr;
	} fs_file_t;

	typedef struct fs_
	{
		char drive;
		fs_file_t *(*open)(char *, const char *);
		size_t (*read)(fs_file_t*, uint8_t *, size_t);
		size_t (*write)(fs_file_t*, const uint8_t *, size_t);
		int (*available)(fs_file_t*);
		void (*close)(fs_file_t*);
		bool (*remove)(char *);
		bool (*next_file)(fs_file_t *, fs_file_info_t*);
		bool (*finfo)(char *, fs_file_info_t *);
		struct fs_* next;
	} fs_t;

	void fs_mount(fs_t *drive);
	void fs_unmount(fs_t *drive);
	fs_file_t* fs_open(char *path, const char *mode);
	size_t fs_read(fs_file_t* fp, uint8_t * buffer, size_t len);
	size_t fs_write(fs_file_t* fp, const uint8_t * buffer, size_t len);
	int fs_available(fs_file_t* fp);
	void fs_close(fs_file_t* fp);
	bool fs_remove(char *path);
	bool fs_nextfile(fs_file_t *fp, fs_file_info_t *finfo);

#ifdef __cplusplus
}
#endif

#endif