/* flash_fs.c — C99 LittleFS shim for uCNC fs_t on ESP8266 internal flash
 *
 * This module provides the flash_fs_* functions (C API) equivalent to the
 * C++ wrappers used in esp8266_wifi.cpp, but implemented directly on top
 * of the LittleFS C API (lfs.h).
 *
 * Assumptions:
 *  - A global mounted LittleFS instance is available as `extern lfs_t g_lfs;`
 *    along with a configured/mounted lfs_config elsewhere in the program.
 *  - The drive path (/C/) is handled by the higher-level file_system code.
 *  - Timestamps are not available via the LittleFS C API; set to 0.
 *
 * References:
 *  - uCNC fs_t/fs_file_t API: file_system.h
 *  - C++ reference shim (ESP8266 + Arduino): esp8266_wifi.cpp
 *  - LittleFS C API: lfs.h
 */

#include "../../../cnc.h"
#if (MCU == MCU_ESP8266)

/* flash_fs.c - LittleFS C99 shim for ESP8266 internal flash
 *
 * Requires:
 *   - ESP8266 core flash HAL: flash_hal.h (FS_PHYS_* and flash_hal_{read,write,erase})
 *   - LittleFS C sources/headers available in include path: lfs.h, lfs.c, etc.
 *   - Your generic FS interface: file_system.h (fs_t, fs_file_t, fs_file_info_t)
 *
 * Notes:
 *   - Drive prefix (/C/) is filtered by the outer file_system layer, so paths here are root-based.
 *   - Timestamps are set to 0 (LittleFS attrs can be added later if needed).
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "flash_hal.h"										// FS_PHYS_ADDR, FS_PHYS_SIZE, FS_PHYS_PAGE, FS_PHYS_BLOCK, flash_hal_{read,write,erase}
#include "../../../modules/file_system.h" // fs_t, fs_file_t, fs_file_info_t
#include "lfs.h"													// LittleFS C API

/* ---------- Internal state ---------- */

typedef struct
{
	uint32_t start;
	uint32_t block_size;
} flash_ctx_t;

static lfs_t g_lfs;
static struct lfs_config g_cfg;
static flash_ctx_t g_ctx;
static uint8_t g_mounted = 0;

typedef enum
{
	FLASH_HANDLE_FILE = 0,
	FLASH_HANDLE_DIR = 1
} flash_handle_type_t;

typedef struct
{
	flash_handle_type_t type;
	char base_path[FS_PATH_NAME_MAX_LEN]; // for DIR: where we're iterating
	union
	{
		lfs_file_t file;
		lfs_dir_t dir;
	} u;
} flash_handle_t;

/* ---------- Flash HAL bindings (LittleFS block device) ---------- */

static int lfs_flash_read(const struct lfs_config *c,
													lfs_block_t block,
													lfs_off_t off,
													void *dst,
													lfs_size_t size)
{
	const flash_ctx_t *ctx = (const flash_ctx_t *)c->context;
	uint32_t addr = ctx->start + (block * ctx->block_size) + off;
	return (flash_hal_read(addr, size, (uint8_t *)dst) == FLASH_HAL_OK) ? 0 : -1;
}

static int lfs_flash_prog(const struct lfs_config *c,
													lfs_block_t block,
													lfs_off_t off,
													const void *buffer,
													lfs_size_t size)
{
	const flash_ctx_t *ctx = (const flash_ctx_t *)c->context;
	uint32_t addr = ctx->start + (block * ctx->block_size) + off;
	return (flash_hal_write(addr, size, (const uint8_t *)buffer) == FLASH_HAL_OK) ? 0 : -1;
}

static int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block)
{
	const flash_ctx_t *ctx = (const flash_ctx_t *)c->context;
	uint32_t addr = ctx->start + (block * ctx->block_size);
	return (flash_hal_erase(addr, ctx->block_size) == FLASH_HAL_OK) ? 0 : -1;
}

static int lfs_flash_sync(const struct lfs_config *c)
{
	(void)c;
	return 0;
}

/* ---------- Mount/format helpers ---------- */

static void flash_fs_config_init(void)
{
	memset(&g_cfg, 0, sizeof(g_cfg));
	g_ctx.start = FS_PHYS_ADDR;				/* start offset within flash address space */
	g_ctx.block_size = FS_PHYS_BLOCK; /* erase block size */

	g_cfg.context = &g_ctx;
	g_cfg.read = lfs_flash_read;
	g_cfg.prog = lfs_flash_prog;
	g_cfg.erase = lfs_flash_erase;
	g_cfg.sync = lfs_flash_sync;

	g_cfg.read_size = FS_PHYS_PAGE; /* must divide block_size */
	g_cfg.prog_size = FS_PHYS_PAGE; /* must divide block_size */
	g_cfg.block_size = FS_PHYS_BLOCK;
	g_cfg.block_count = FS_PHYS_SIZE / FS_PHYS_BLOCK; /* total blocks */
	g_cfg.cache_size = FS_PHYS_PAGE;									/* typical: page size */
	g_cfg.lookahead_size = 64;												/* multiple of 8, tune as needed */
	g_cfg.block_cycles = 512;													/* wear-leveling (typical) */
}

static bool flash_fs_mount_if_needed(void)
{
	if (g_mounted)
		return true;

	flash_fs_config_init();
	int rc = lfs_mount(&g_lfs, &g_cfg);
	if (rc != 0)
	{
		/* Try format once then re-mount */
		if (lfs_format(&g_lfs, &g_cfg) != 0)
		{
			return false;
		}
		rc = lfs_mount(&g_lfs, &g_cfg);
		if (rc != 0)
		{
			return false;
		}
	}
	g_mounted = 1;
	return true;
}

/* ---------- Mode parsing ---------- */

static int flash_fs_mode_to_lfs_flags(const char *mode)
{
	/* Support: "r", "w", "a", "r+", "w+", "a+" with optional 'b' */
	int flags = LFS_O_RDONLY;
	if (!mode || !*mode)
		return flags;

	const char m0 = mode[0];
	const int plus = (strchr(mode, '+') != NULL);

	if (m0 == 'r')
	{
		flags = plus ? LFS_O_RDWR : LFS_O_RDONLY;
	}
	else if (m0 == 'w')
	{
		flags = plus ? (LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC)
								 : (LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
	}
	else if (m0 == 'a')
	{
		flags = plus ? (LFS_O_RDWR | LFS_O_CREAT | LFS_O_APPEND)
								 : (LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND);
	}
	return flags;
}

/* Recursively create intermediate directories for a path, ignoring the final filename. */
static void flash_fs_make_parents_if_needed(const char *path)
{
	if (!path)
		return;

	char *scratch = (char *)malloc(FS_PATH_NAME_MAX_LEN);
	if (!scratch)
		return;

	strncpy(scratch, path, FS_PATH_NAME_MAX_LEN - 1);
	scratch[FS_PATH_NAME_MAX_LEN - 1] = '\0';

	char *p = scratch;
	/* Skip leading '/' if present */
	if (*p == '/')
		p++;

	while ((p = strchr(p, '/')) != NULL)
	{
		*p = '\0';
		if (scratch[0] != '\0')
		{
			(void)lfs_mkdir(&g_lfs, scratch);
		}
		*p = '/';
		p++;
	}

	free(scratch);
}

/* ---------- Required API (C wrappers) ---------- */

int flash_fs_available(fs_file_t *fp)
{
	if (!fp || !fp->file_ptr)
		return 0;
	if (!flash_fs_mount_if_needed())
		return 0;

	flash_handle_t *h = (flash_handle_t *)fp->file_ptr;
	if (h->type != FLASH_HANDLE_FILE)
		return 0;

	lfs_soff_t here = lfs_file_tell(&g_lfs, &h->u.file);
	lfs_soff_t fsize = lfs_file_size(&g_lfs, &h->u.file);
	if (here < 0 || fsize < 0)
		return 0;

	lfs_soff_t rem = fsize - here;
	return (rem > 0) ? (int)rem : 0;
}

void flash_fs_close(fs_file_t *fp)
{
	if (!fp || !fp->file_ptr)
		return;
	if (!flash_fs_mount_if_needed())
		return;

	flash_handle_t *h = (flash_handle_t *)fp->file_ptr;
	if (h->type == FLASH_HANDLE_FILE)
	{
		(void)lfs_file_close(&g_lfs, &h->u.file);
	}
	else
	{
		(void)lfs_dir_close(&g_lfs, &h->u.dir);
	}
	/* Memory for fp->file_ptr is freed by the outer layer (see fs_safe_free) */
}

bool flash_fs_remove(const char *path)
{
	if (!flash_fs_mount_if_needed())
		return false;
	if (!path || !*path)
		return false;
	return (lfs_remove(&g_lfs, path) == 0);
}

bool flash_fs_next_file(fs_file_t *fp, fs_file_info_t *finfo)
{
	if (!fp || !fp->file_ptr || !finfo)
		return false;
	if (!flash_fs_mount_if_needed())
		return false;

	flash_handle_t *h = (flash_handle_t *)fp->file_ptr;
	if (h->type != FLASH_HANDLE_DIR)
		return false;

	struct lfs_info info;
	while (true)
	{
		int rc = lfs_dir_read(&g_lfs, &h->u.dir, &info);
		if (rc < 0)
			return false; /* error */
		if (rc == 0)
			return false; /* end of dir */

		/* Skip "." and ".." if present */
		if ((strcmp(info.name, ".") == 0) || (strcmp(info.name, "..") == 0))
		{
			continue;
		}

		/* Build full path: base_path + "/" + name */
		memset(finfo->full_name, 0, sizeof(finfo->full_name));
		if (h->base_path[0] == '\0' || (strcmp(h->base_path, "/") == 0))
		{
			snprintf(finfo->full_name, sizeof(finfo->full_name), "/%s", info.name);
		}
		else
		{
			snprintf(finfo->full_name, sizeof(finfo->full_name), "%s/%s", h->base_path, info.name);
		}

		finfo->is_dir = (info.type == LFS_TYPE_DIR);
		finfo->size = (uint32_t)info.size;
		finfo->timestamp = 0; /* not tracked here */

		return true;
	}
}

size_t flash_fs_read(fs_file_t *fp, uint8_t *buffer, size_t len)
{
	if (!fp || !fp->file_ptr || !buffer || len == 0)
		return 0;
	if (!flash_fs_mount_if_needed())
		return 0;

	flash_handle_t *h = (flash_handle_t *)fp->file_ptr;
	if (h->type != FLASH_HANDLE_FILE)
		return 0;

	lfs_ssize_t rc = lfs_file_read(&g_lfs, &h->u.file, buffer, (lfs_size_t)len);
	return (rc < 0) ? 0 : (size_t)rc;
}

size_t flash_fs_write(fs_file_t *fp, const uint8_t *buffer, size_t len)
{
	if (!fp || !fp->file_ptr || !buffer || len == 0)
		return 0;
	if (!flash_fs_mount_if_needed())
		return 0;

	flash_handle_t *h = (flash_handle_t *)fp->file_ptr;
	if (h->type != FLASH_HANDLE_FILE)
		return 0;

	lfs_ssize_t rc = lfs_file_write(&g_lfs, &h->u.file, buffer, (lfs_size_t)len);
	return (rc < 0) ? 0 : (size_t)rc;
}

bool flash_fs_info(const char *path, fs_file_info_t *finfo)
{
	if (!flash_fs_mount_if_needed())
		return false;
	if (!path || !*path || !finfo)
		return false;

	struct lfs_info info;
	int rc = lfs_stat(&g_lfs, path, &info);
	if (rc != 0)
		return false;

	memset(finfo->full_name, 0, sizeof(finfo->full_name));
	strncpy(finfo->full_name, path, sizeof(finfo->full_name) - 1);
	finfo->is_dir = (info.type == LFS_TYPE_DIR);
	finfo->size = (uint32_t)info.size;
	finfo->timestamp = 0; /* not tracked here */
	return true;
}

fs_file_t *flash_fs_open(const char *path, const char *mode)
{
	if (!flash_fs_mount_if_needed())
		return NULL;
	if (!path || !*path)
		return NULL;

	int flags = flash_fs_mode_to_lfs_flags(mode);

	fs_file_t *fp = (fs_file_t *)calloc(1, sizeof(fs_file_t));
	if (!fp)
		return NULL;

	flash_handle_t *h = (flash_handle_t *)calloc(1, sizeof(flash_handle_t));
	if (!h)
	{
		fs_safe_free(fp);
		return NULL;
	}

	/* If creating, ensure intermediate dirs exist (like Arduino wrapper does) */
	if ((flags & LFS_O_CREAT) && strchr(path, '/'))
	{
		flash_fs_make_parents_if_needed(path);
	}

	/* Try opening as file first */
	if (lfs_file_open(&g_lfs, &h->u.file, path, flags) == 0)
	{
		h->type = FLASH_HANDLE_FILE;

		/* Fill file_info */
		memset(fp->file_info.full_name, 0, sizeof(fp->file_info.full_name));
		strncpy(fp->file_info.full_name, path, sizeof(fp->file_info.full_name) - 1);
		fp->file_info.is_dir = false;
		fp->file_info.size = (uint32_t)lfs_file_size(&g_lfs, &h->u.file);
		fp->file_info.timestamp = 0;

		fp->file_ptr = h;
		/* fp->fs_ptr set by caller when wiring fs_t; no need to set here */
		return fp;
	}

	/* If it's a directory, open as dir (like Arduino File can represent dirs) */
	if (lfs_dir_open(&g_lfs, &h->u.dir, (*path) ? path : "/") == 0)
	{
		h->type = FLASH_HANDLE_DIR;
		memset(h->base_path, 0, sizeof(h->base_path));
		if (!path || !*path)
		{
			strcpy(h->base_path, "/");
		}
		else
		{
			strncpy(h->base_path, path, sizeof(h->base_path) - 1);
		}

		/* Populate file_info for a dir handle */
		memset(fp->file_info.full_name, 0, sizeof(fp->file_info.full_name));
		strncpy(fp->file_info.full_name, (*path) ? path : "/", sizeof(fp->file_info.full_name) - 1);
		fp->file_info.is_dir = true;
		fp->file_info.size = 0;
		fp->file_info.timestamp = 0;

		fp->file_ptr = h;
		return fp;
	}

	/* Open failed */
	fs_safe_free(h);
	fs_safe_free(fp);
	return NULL;
}

fs_file_t *flash_fs_opendir(const char *path)
{
	/* Simply open as a directory */
	return flash_fs_open((path && *path) ? path : "/", "r");
}

bool flash_fs_seek(fs_file_t *fp, uint32_t position)
{
	if (!fp || !fp->file_ptr)
		return false;
	if (!flash_fs_mount_if_needed())
		return false;

	flash_handle_t *h = (flash_handle_t *)fp->file_ptr;
	if (h->type != FLASH_HANDLE_FILE)
		return false;

	lfs_soff_t rc = lfs_file_seek(&g_lfs, &h->u.file, (lfs_soff_t)position, LFS_SEEK_SET);
	return (rc >= 0);
}

bool flash_fs_mkdir(const char *path)
{
	if (!flash_fs_mount_if_needed())
		return false;
	if (!path || !*path)
		return false;
	return (lfs_mkdir(&g_lfs, path) == 0);
}

bool flash_fs_rmdir(const char *path)
{
	if (!flash_fs_mount_if_needed())
		return false;
	if (!path || !*path)
		return false;
	/* LittleFS removes empty directories with lfs_remove */
	return (lfs_remove(&g_lfs, path) == 0);
}

fs_t flash_fs = {
			.drive = 'C',
			.open = flash_fs_open,
			.read = flash_fs_read,
			.write = flash_fs_write,
			.seek = flash_fs_seek,
			.available = flash_fs_available,
			.close = flash_fs_close,
			.remove = flash_fs_remove,
			.opendir = flash_fs_opendir,
			.mkdir = flash_fs_mkdir,
			.rmdir = flash_fs_rmdir,
			.next_file = flash_fs_next_file,
			.finfo = flash_fs_info,
			.next = NULL};

#endif
