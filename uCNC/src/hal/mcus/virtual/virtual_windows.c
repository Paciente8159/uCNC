#if defined(_WIN32) || defined(_WIN64)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../../../cnc.h"
#if (MCU == MCU_VIRTUAL_WIN)

#ifdef __cplusplus
extern "C"
{
#endif

/* C99 includes */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <math.h>

/* Platform includes */
#include <pthread.h>

#include <conio.h>

#if defined(ENABLE_SOCKETS)

#include "../../../modules/net/socket.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
    int socket_init(void);
#endif
#include <windows.h>
#include <dirent.h>

    /* Minimal C serial “driver” */
    typedef struct serial_port_
    {
        HANDLE h;
        volatile bool connected;
        DWORD errors;
        COMSTAT status;
        char port_name[128];
        pthread_t thread;
        volatile bool stop;
    } serial_port_t;

    serial_port_t g_uart = {0};

    bool serial_configure(HANDLE h)
    {
        DCB dcbSerialParams;
        memset(&dcbSerialParams, 0, sizeof(dcbSerialParams));
        dcbSerialParams.DCBlength = sizeof(DCB);

        if (!GetCommState(h, &dcbSerialParams))
        {
            printf("Serial: GetCommState failed\n");
            return false;
        }

        /* Mirror original WindowsSerial: 9600 8N1, DTR enabled */
        dcbSerialParams.BaudRate = CBR_9600;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;
        dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

        if (!SetCommState(h, &dcbSerialParams))
        {
            printf("Serial: SetCommState failed\n");
            return false;
        }

        /* Non-blocking reads: quick return if no data */
        COMMTIMEOUTS to = {0};
        to.ReadIntervalTimeout = 1;
        to.ReadTotalTimeoutMultiplier = 0;
        to.ReadTotalTimeoutConstant = 0;
        to.WriteTotalTimeoutMultiplier = 0;
        to.WriteTotalTimeoutConstant = 0;
        if (!SetCommTimeouts(h, &to))
        {
            printf("Serial: SetCommTimeouts failed\n");
            return false;
        }

        PurgeComm(h, PURGE_RXCLEAR | PURGE_TXCLEAR);
        return true;
    }

    int serial_read(char *buffer, unsigned int nbChar)
    {
        if (!g_uart.connected)
            return 0;

        DWORD bytesRead = 0;
        unsigned int toRead = 0;

        ClearCommError(g_uart.h, &g_uart.errors, &g_uart.status);

        if (g_uart.status.cbInQue > 0)
        {
            toRead = (g_uart.status.cbInQue > nbChar) ? nbChar : g_uart.status.cbInQue;
            if (toRead > 0 && ReadFile(g_uart.h, buffer, toRead, &bytesRead, NULL))
            {
                return (int)bytesRead;
            }
        }
        return 0;
    }

    bool serial_write(const uint8_t *buffer, unsigned int nbChar)
    {
        if (!g_uart.connected)
            return false;
        DWORD bytesSent = 0;
        if (!WriteFile(g_uart.h, (void *)buffer, nbChar, &bytesSent, NULL))
        {
            ClearCommError(g_uart.h, &g_uart.errors, &g_uart.status);
            return false;
        }
        return true;
    }

    void *uart_thread_fn(void *arg)
    {
        (void)arg;
        /* Try to connect and keep connection alive */
        while (!g_uart.stop)
        {
            if (!g_uart.connected)
            {
                HANDLE h = CreateFileA(
                    g_uart.port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                if (h == INVALID_HANDLE_VALUE)
                {
                    /* Port not present yet; non-blocking wait/retry */
                    Sleep(500);
                    continue;
                }

                if (!serial_configure(h))
                {
                    CloseHandle(h);
                    Sleep(500);
                    continue;
                }

                g_uart.h = h;
                g_uart.connected = true;
                /* No startup Sleep here; non-blocking startup */
                // printf("Serial: connected to %s\n", g_uart.port_name);
            }
            else
            {
                /* Check for hang-up/errors occasionally */
                ClearCommError(g_uart.h, &g_uart.errors, &g_uart.status);
                /* Light duty periodic sleep to avoid busy loop */
                Sleep(10);
            }
        }

        if (g_uart.connected)
        {
            g_uart.connected = false;
            CloseHandle(g_uart.h);
            g_uart.h = INVALID_HANDLE_VALUE;
        }
        return NULL;
    }

    bool uart_connected(void)
    {
        return g_uart.connected;
    }

    void serial_init(void)
    {
        memset(&g_uart, 0, sizeof(g_uart));
        g_uart.h = INVALID_HANDLE_VALUE;
        g_uart.connected = false;
        g_uart.stop = false;
        strncpy(g_uart.port_name, UART_PORT_NAME, sizeof(g_uart.port_name) - 1);
        pthread_create(&g_uart.thread, NULL, &uart_thread_fn, NULL);
    }

    int console_kbhit(void)
    {
        return _kbhit();
    }

    int console_getch(void)
    {
        return _getch();
    }

    /**
     * IO named pipe
     *
     */

    void *ioserver(void *args)
    {
        (void)args;
        HANDLE hPipe;
        BOOL fSuccess = FALSE;
        DWORD cbRead, cbToWrite, cbWritten;
        LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\ucncio");

        for (;;)
        {
            BOOL fConnected = FALSE;
            hPipe = CreateNamedPipe(
                lpszPipename,
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                sizeof(VIRTUAL_MAP),
                sizeof(VIRTUAL_MAP),
                0,
                NULL);

            if (hPipe == INVALID_HANDLE_VALUE)
            {
                printf("CreateNamedPipe failed, GLE=%lu.\n", GetLastError());
                return NULL;
            }

            fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
            if (fConnected)
            {
                cbToWrite = sizeof(VIRTUAL_MAP);
                uint8_t lpvMessage[sizeof(VIRTUAL_MAP)];
                do
                {
                    memcpy(lpvMessage, (const void *)&virtualmap, sizeof(VIRTUAL_MAP));
                    fSuccess = WriteFile(hPipe, lpvMessage, cbToWrite, &cbWritten, NULL);
                    if (!fSuccess)
                    {
                        printf("WriteFile to pipe failed. GLE=%lu\n", GetLastError());
                        break;
                    }

                    fSuccess = ReadFile(hPipe, lpvMessage, cbToWrite, &cbRead, NULL);
                    if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
                        break;

                    VIRTUAL_MAP *ptr = (VIRTUAL_MAP *)&lpvMessage[0];
                    if (virtualmap.special_inputs != ptr->special_inputs)
                    {
                        uint32_t diff = virtualmap.special_inputs ^ ptr->special_inputs;
                        virtualmap.special_inputs = ptr->special_inputs;
                        if (diff & 0x1FFUL)
                            mcu_limits_changed_cb();
                        if (diff & 0x200UL)
                            mcu_probe_changed_cb();
                        if (diff & 0x3C00UL)
                            mcu_controls_changed_cb();
                    }
                    if (virtualmap.inputs != ptr->inputs)
                    {
                        virtualmap.inputs = ptr->inputs;
                        mcu_inputs_changed_cb();
                    }
                    memcpy((void *)virtualmap.analog, ptr->analog, 16);
                } while (fSuccess);

                if (!fSuccess)
                {
                    printf("ReadFile from pipe failed. GLE=%lu\n", GetLastError());
                }
            }

            CloseHandle(hPipe);
        }
        return NULL;
    }

    HANDLE win_timer;
    void (*timer_func_handler_pntr)(void);
    unsigned long perf_start;
    double cyclesPerMicrosecond;
    double cyclesPerMillisecond;

    VOID CALLBACK timer_sig_handler(PVOID, BOOLEAN);

    int start_timer(int mSec, void (*timer_func_handler)(void))
    {
        timer_func_handler_pntr = timer_func_handler;

        if (CreateTimerQueueTimer(&win_timer, NULL, (WAITORTIMERCALLBACK)timer_sig_handler, NULL, mSec, mSec, WT_EXECUTEINTIMERTHREAD) == 0)
        {
            printf("\nCreateTimerQueueTimer() error\n");
            return (1);
        }

        return (0);
    }

    VOID CALLBACK timer_sig_handler(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
    {
        timer_func_handler_pntr();
    }

    void stop_timer(void)
    {
        DeleteTimerQueueTimer(NULL, win_timer, NULL);
        CloseHandle(win_timer);
    }

    /* ----- Flash filesystem shim (host FS) --------------------------------- */
#include "src/modules/file_system.h"

    fs_t flash_fs;

    bool flash_fs_finfo(const char *path, fs_file_info_t *finfo)
    {
        if (!path || !finfo)
            return false;

        char fpath[256];
        if (strcmp("/", path) == 0 || strcmp(".", path) == 0)
        {
            strncpy(fpath, "./*", sizeof(fpath) - 1);
            fpath[sizeof(fpath) - 1] = '\0';
        }
        else
        {
            snprintf(fpath, sizeof(fpath), "./%s", path);
        }

        WIN32_FIND_DATAA fd = {0};
        HANDLE h = FindFirstFileA(fpath, &fd);
        if (h == INVALID_HANDLE_VALUE)
            return false;

        strncpy(finfo->full_name, path, FS_PATH_NAME_MAX_LEN - 1);
        finfo->full_name[FS_PATH_NAME_MAX_LEN - 1] = '\0';

        finfo->is_dir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
        finfo->size = finfo->is_dir ? 0u : (uint32_t)fd.nFileSizeLow;

        FILETIME ft = fd.ftLastWriteTime;
        ULARGE_INTEGER ull;
        ull.LowPart = ft.dwLowDateTime;
        ull.HighPart = ft.dwHighDateTime;
        uint64_t fileTime = ull.QuadPart;
        fileTime -= 116444736000000000ULL;
        finfo->timestamp = (uint32_t)(fileTime / 10000000ULL);

        FindClose(h);
        return true;
    }

    fs_file_t *flash_fs_opendir(const char *path)
    {
        fs_file_t *fp = (fs_file_t *)calloc(1, sizeof(fs_file_t));
        if (!fp)
            return NULL;
        char dir[256] = ".";
        if (strcmp("/", path))
            strncat(dir, path, sizeof(dir) - 2);

        fs_file_info_t info = {0};
        flash_fs_finfo(path, &info);
        fp->file_ptr = opendir(dir);
        if (fp->file_ptr)
        {
            memcpy(&fp->file_info, &info, sizeof(info));
            return fp;
        }
        fs_safe_free(fp);
        return NULL;
    }

    fs_file_t *flash_fs_open(const char *path, const char *mode)
    {
        fs_file_info_t finfo = {0};
        char file[256] = ".";
        if (strcmp("/", path))
            strncat(file, path, sizeof(file) - 2);

        FILE *tmpfile = fopen(file, mode);

        if (!flash_fs_finfo(path, &finfo))
            return NULL;

        if (!finfo.is_dir)
        {
            fs_file_t *fp = (fs_file_t *)calloc(1, sizeof(fs_file_t));
            if (!fp)
                return NULL;
            fp->file_ptr = tmpfile;
            if (fp->file_ptr)
            {
                memset(fp->file_info.full_name, 0, sizeof(fp->file_info.full_name));
                fp->file_info.full_name[0] = '/';
                fp->file_info.full_name[1] = flash_fs.drive;
                fp->file_info.full_name[2] = '/';
                strncat(fp->file_info.full_name, finfo.full_name, FS_PATH_NAME_MAX_LEN - 3);
                fp->file_info.is_dir = false;
                fp->file_info.size = finfo.size;
                fp->file_info.timestamp = finfo.timestamp;
                fp->fs_ptr = &flash_fs;
                return fp;
            }
            fs_safe_free(fp);
            return NULL;
        }
        else
        {
            return flash_fs_opendir(path);
        }
    }

    size_t flash_fs_read(fs_file_t *fp, uint8_t *buffer, size_t len)
    {
        if (fp && fp->file_ptr)
            return fread(buffer, 1, len, (FILE *)fp->file_ptr);
        return 0;
    }
    size_t flash_fs_write(fs_file_t *fp, const uint8_t *buffer, size_t len)
    {
        if (fp && fp->file_ptr)
            return fwrite(buffer, 1, len, (FILE *)fp->file_ptr);
        return 0;
    }
    bool flash_fs_seek(fs_file_t *fp, uint32_t position)
    {
        if (fp && fp->file_ptr)
        {
            fseek((FILE *)fp->file_ptr, (long)position, SEEK_SET);
            return true;
        }
        return false;
    }
    int flash_fs_available(fs_file_t *fp)
    {
        if (fp && fp->file_ptr)
            return (int)(fp->file_info.size - (uint32_t)ftell((FILE *)fp->file_ptr));
        return 0;
    }
    void flash_fs_close(fs_file_t *fp)
    {
        if (fp && fp->file_ptr)
        {
            if (fp->file_info.is_dir)
                closedir((DIR *)fp->file_ptr);
            else
                fclose((FILE *)fp->file_ptr);
        }
    }
    bool flash_fs_remove(const char *path)
    {
        (void)path;
        if (flash_fs.drive)
            return remove(path) == 0;
        return false;
    }
    bool flash_fs_mkdir(const char *path)
    {
        (void)path;
        if (flash_fs.drive)
            return mkdir(path) == 0;
        return false;
    }
    bool flash_fs_rmdir(const char *path)
    {
        (void)path;
        if (flash_fs.drive)
            return rmdir(path) == 0;
        return false;
    }
    bool flash_fs_next_file(fs_file_t *fp, fs_file_info_t *finfo)
    {
        if (fp && fp->file_ptr)
        {
            struct dirent *entry = readdir((DIR *)fp->file_ptr);
            if (entry)
            {
                flash_fs_finfo(entry->d_name, finfo);
                return true;
            }
        }
        return false;
    }

    void flash_fs_init(void)
    {
        flash_fs.drive = 'C';
        flash_fs.open = flash_fs_open;
        flash_fs.read = flash_fs_read;
        flash_fs.write = flash_fs_write;
        flash_fs.seek = flash_fs_seek;
        flash_fs.available = flash_fs_available;
        flash_fs.close = flash_fs_close;
        flash_fs.remove = flash_fs_remove;
        flash_fs.opendir = flash_fs_opendir;
        flash_fs.mkdir = flash_fs_mkdir;
        flash_fs.rmdir = flash_fs_rmdir;
        flash_fs.next_file = flash_fs_next_file;
        flash_fs.finfo = flash_fs_finfo;
        flash_fs.next = NULL;
        fs_mount(&flash_fs);
    }

    void get_current_dir(char *cwd, size_t len)
    {
        GetCurrentDirectoryA(1024, cwd);
    }

#if defined(ENABLE_SOCKETS)

/* winsock2.h must precede any project header that may include windows.h. */
#include <winsock2.h>
#include <ws2tcpip.h>

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Adjust this include path if the backend is not placed beside socket.h. */
#include "../../../modules/net/socket.h"

#define WINDOWS_SOCKET_MAX_LISTENERS MAX_SOCKETS
#define WINDOWS_SOCKET_MAX_CLIENTS SOCKET_MAX_CONNECTIONS

#if WINDOWS_SOCKET_MAX_LISTENERS == 0
#error "WINDOWS_SOCKET_MAX_LISTENERS must be greater than zero"
#endif

#if WINDOWS_SOCKET_MAX_CLIENTS == 0
#error "WINDOWS_SOCKET_MAX_CLIENTS must be greater than zero"
#endif

/*
 * WinSock fd_set stores a fixed number of SOCKET values. readfds contains all
 * listeners plus clients whose readable hint is not already latched by uCNC.
 * Fail at compile time instead of letting FD_SET silently exceed capacity.
 */
#if (WINDOWS_SOCKET_MAX_LISTENERS + WINDOWS_SOCKET_MAX_CLIENTS) > FD_SETSIZE
#error "Increase FD_SETSIZE or reduce MAX_SOCKETS/SOCKET_MAX_CONNECTIONS"
#endif

/*
 * Backend handles are generation-tagged table references, not raw SOCKET
 * values. This prevents a stale uCNC handle from becoming valid again merely
 * because WinSock later reuses the same native SOCKET value.
 *
 * The low 16 bits encode (slot << 1) | kind and the high 16 bits encode a
 * non-zero generation. Keeping one kind bit leaves 15 bits for the slot.
 * FD_SETSIZE is normally far smaller, but make the representation limit
 * explicit so a custom configuration cannot silently truncate a slot.
 */
#if WINDOWS_SOCKET_MAX_LISTENERS > 32767U
#error "WINDOWS_SOCKET_MAX_LISTENERS exceeds backend handle slot capacity"
#endif

#if WINDOWS_SOCKET_MAX_CLIENTS > 32767U
#error "WINDOWS_SOCKET_MAX_CLIENTS exceeds backend handle slot capacity"
#endif

#define WINDOWS_HANDLE_KIND_CLIENT ((uintptr_t)1U)
#define WINDOWS_HANDLE_SLOT_SHIFT 1U
#define WINDOWS_HANDLE_GENERATION_SHIFT 16U
#define WINDOWS_HANDLE_LOW_MASK ((uintptr_t)0xFFFFU)
#define WINDOWS_HANDLE_SLOT_MASK ((uintptr_t)0x7FFFU)

#define WINDOWS_CLIENT_READABLE_BYTES \
	((WINDOWS_SOCKET_MAX_CLIENTS + 7U) / 8U)

/*
 * Structure-of-arrays storage avoids per-record alignment padding on Win64.
 * Keeping the arrays in one aggregate also prevents linker alignment gaps
 * between separate static objects. Native TCP payload stays entirely in
 * WinSock buffers; there is no backend RX copy, TX queue, TX retry offset, or
 * writable-interest state.
 */
typedef struct windows_socket_state_
{
	SOCKET listener_sockets[WINDOWS_SOCKET_MAX_LISTENERS];
	SOCKET client_sockets[WINDOWS_SOCKET_MAX_CLIENTS];
	const socket_device_events_t *events;
	socket_device_token_t client_tokens[WINDOWS_SOCKET_MAX_CLIENTS];
	uint16_t client_generations[WINDOWS_SOCKET_MAX_CLIENTS];
	uint16_t listener_generations[WINDOWS_SOCKET_MAX_LISTENERS];
	uint16_t listener_cursor;
	uint16_t client_cursor;
	uint8_t client_readable_notified[WINDOWS_CLIENT_READABLE_BYTES];
	uint8_t flags;
} windows_socket_state_t;

#define WINDOWS_STATE_NET_STARTED (1U << 0)
#define WINDOWS_STATE_ACCEPT_FIRST (1U << 1)

static windows_socket_state_t windows_state;

#define windows_listener_sockets windows_state.listener_sockets
#define windows_client_sockets windows_state.client_sockets
#define windows_socket_events windows_state.events
#define windows_client_tokens windows_state.client_tokens
#define windows_client_generations windows_state.client_generations
#define windows_listener_generations windows_state.listener_generations
#define windows_listener_cursor windows_state.listener_cursor
#define windows_client_cursor windows_state.client_cursor
#define windows_client_readable_notified windows_state.client_readable_notified

static uint16_t windows_next_generation(uint16_t generation)
{
	++generation;
	if (generation == 0U)
	{
		++generation;
	}
	return generation;
}

static socket_device_handle_t windows_make_handle(bool client,
										   uint16_t slot,
										   uint16_t generation)
{
	uintptr_t value = ((uintptr_t)generation << WINDOWS_HANDLE_GENERATION_SHIFT) |
					  ((uintptr_t)slot << WINDOWS_HANDLE_SLOT_SHIFT);

	if (client)
	{
		value |= WINDOWS_HANDLE_KIND_CLIENT;
	}
	return (socket_device_handle_t)value;
}

/*
 * Decodes only handles produced by windows_make_handle(). Re-encoding and
 * comparing rejects malformed values and, on Win64, values with unexpected
 * upper bits without relying on a shift as wide as uintptr_t on Win32.
 */
static bool windows_decode_handle(socket_device_handle_t handle,
								  bool *client,
								  uint16_t *slot,
								  uint16_t *generation)
{
	uintptr_t value = (uintptr_t)handle;
	uintptr_t low;
	bool decoded_client;
	uint16_t decoded_slot;
	uint16_t decoded_generation;

	if (handle == SOCKET_DEVICE_INVALID_HANDLE)
	{
		return false;
	}

	low = value & WINDOWS_HANDLE_LOW_MASK;
	decoded_client = (low & WINDOWS_HANDLE_KIND_CLIENT) != 0U;
	decoded_slot = (uint16_t)((low >> WINDOWS_HANDLE_SLOT_SHIFT) &
								WINDOWS_HANDLE_SLOT_MASK);
	decoded_generation =
		(uint16_t)(value >> WINDOWS_HANDLE_GENERATION_SHIFT);

	if (decoded_generation == 0U ||
		windows_make_handle(decoded_client, decoded_slot,
							decoded_generation) != handle)
	{
		return false;
	}

	if (client)
	{
		*client = decoded_client;
	}
	if (slot)
	{
		*slot = decoded_slot;
	}
	if (generation)
	{
		*generation = decoded_generation;
	}
	return true;
}

static int windows_resolve_client(socket_device_handle_t handle)
{
	bool client;
	uint16_t slot;
	uint16_t generation;

	if (!windows_decode_handle(handle, &client, &slot, &generation) || !client ||
		slot >= WINDOWS_SOCKET_MAX_CLIENTS ||
		windows_client_sockets[slot] == INVALID_SOCKET ||
		windows_client_generations[slot] != generation)
	{
		return -1;
	}
	return (int)slot;
}

static int windows_find_free_listener(void)
{
	uint16_t i;

	for (i = 0U; i < WINDOWS_SOCKET_MAX_LISTENERS; ++i)
	{
		if (windows_listener_sockets[i] == INVALID_SOCKET)
		{
			return (int)i;
		}
	}

	return -1;
}

static int windows_find_free_client(void)
{
	uint16_t i;

	for (i = 0U; i < WINDOWS_SOCKET_MAX_CLIENTS; ++i)
	{
		if (windows_client_sockets[i] == INVALID_SOCKET)
		{
			return (int)i;
		}
	}

	return -1;
}

static bool windows_client_readable_is_notified(uint16_t slot)
{
	uint8_t mask = (uint8_t)(1U << (slot & 7U));
	return (windows_client_readable_notified[slot >> 3] & mask) != 0U;
}

static void windows_client_set_readable_notified(uint16_t slot, bool notified)
{
	uint8_t *byte = &windows_client_readable_notified[slot >> 3];
	uint8_t mask = (uint8_t)(1U << (slot & 7U));

	if (notified)
	{
		*byte = (uint8_t)(*byte | mask);
	}
	else
	{
		*byte = (uint8_t)(*byte & (uint8_t)~mask);
	}
}

static void windows_reset_listener(uint16_t slot)
{
	windows_listener_sockets[slot] = INVALID_SOCKET;
	/* Preserve generation so the next lifetime receives a different handle. */
}

static void windows_reset_client(uint16_t slot)
{
	windows_client_sockets[slot] = INVALID_SOCKET;
	windows_client_tokens[slot] = SOCKET_DEVICE_INVALID_TOKEN;
	windows_client_set_readable_notified(slot, false);
	/* Preserve generation so the next lifetime receives a different handle. */
}

/* Errors for which retrying a later non-blocking RX/TX attempt is valid. */
static bool windows_socket_error_is_temporary(int error)
{
	return error == WSAEWOULDBLOCK || error == WSAEINTR ||
		   error == WSAENOBUFS;
}

/*
 * Maps recv()/send() errors. Reset, abort, timeout and network failures are
 * deliberately normalized to generic fatal ERROR; only recv()==0 is the
 * orderly CLOSED path. WSAEINTR is not retried inside the backend so recv/send
 * remain exactly one native I/O attempt per call.
 */
static int windows_socket_map_io_error(int error)
{
	if (windows_socket_error_is_temporary(error))
	{
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	switch (error)
	{
	case WSAENOTSOCK:
	case WSAEINVAL:
	case WSAEFAULT:
		return SOCKET_DEVICE_INVALID;
	default:
		return SOCKET_DEVICE_ERROR;
	}
}

static int windows_socket_map_close_error(int error)
{
	return error == WSAENOTSOCK || error == WSAEINVAL
			   ? SOCKET_DEVICE_INVALID
			   : SOCKET_DEVICE_ERROR;
}

/*
 * Releases a remotely/fatally closed client before notifying the core. The
 * token is copied first because reset invalidates the backend association.
 * This helper is never used for local close(), which must not emit closed().
 */
static int windows_fail_client(uint16_t slot, int reason)
{
	SOCKET native_socket = windows_client_sockets[slot];
	socket_device_token_t token = windows_client_tokens[slot];

	windows_reset_client(slot);
	(void)closesocket(native_socket);
	windows_socket_events->closed(token, reason);
	return reason;
}

/*
 * Compatibility entry point used by the Windows emulator network startup.
 * WSAStartup is reference-counted by WinSock, so this helper is idempotent and
 * performs it exactly once for this backend lifetime.
 */
int socket_init(void)
{
	WSADATA data;
	int result;

	if ((windows_state.flags & WINDOWS_STATE_NET_STARTED) != 0U)
	{
		return 0;
	}

	result = WSAStartup(MAKEWORD(2, 2), &data);
	if (result != 0)
	{
		return result;
	}

	if (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2)
	{
		(void)WSACleanup();
		return WSAVERNOTSUPPORTED;
	}

	windows_state.flags |= WINDOWS_STATE_NET_STARTED;
	return 0;
}

static int windows_socket_device_init(const socket_device_events_t *events)
{
	uint16_t i;

	if (!events || !events->accepted || !events->readable || !events->closed)
	{
		return SOCKET_DEVICE_INVALID;
	}

	/* Do not retain the event table unless all initialization succeeds. */
	if (socket_init() != 0)
	{
		return SOCKET_DEVICE_ERROR;
	}

	memset(windows_client_readable_notified, 0,
		   sizeof(windows_client_readable_notified));
	for (i = 0U; i < WINDOWS_SOCKET_MAX_LISTENERS; ++i)
	{
		windows_listener_generations[i] = 0U;
		windows_reset_listener(i);
	}
	for (i = 0U; i < WINDOWS_SOCKET_MAX_CLIENTS; ++i)
	{
		windows_client_generations[i] = 0U;
		windows_reset_client(i);
	}

	windows_listener_cursor = 0U;
	windows_client_cursor = 0U;
	windows_state.flags = (uint8_t)(windows_state.flags | WINDOWS_STATE_ACCEPT_FIRST);
	windows_socket_events = events;
	return SOCKET_DEVICE_OK;
}

static socket_device_handle_t windows_socket_listen(
	const socket_device_endpoint_t *endpoint,
	uint8_t backlog)
{
	struct sockaddr_in address;
	SOCKET native_socket;
	u_long nonblocking = 1UL;
	uint16_t generation;
	int slot;
	int native_backlog;

	if (!windows_socket_events || !endpoint || endpoint->port == 0U)
	{
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	slot = windows_find_free_listener();
	if (slot < 0)
	{
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	native_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (native_socket == INVALID_SOCKET)
	{
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	/* Configure non-blocking mode before the socket becomes externally usable. */
	if (ioctlsocket(native_socket, FIONBIO, &nonblocking) == SOCKET_ERROR)
	{
		(void)closesocket(native_socket);
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(endpoint->port);
	address.sin_addr.s_addr = htonl(endpoint->address);

	/* A zero backlog is a bounded request for one pending connection. */
	native_backlog = backlog == 0U ? 1 : (int)backlog;
	if (bind(native_socket, (const struct sockaddr *)&address,
			 (int)sizeof(address)) == SOCKET_ERROR ||
		listen(native_socket, native_backlog) == SOCKET_ERROR)
	{
		(void)closesocket(native_socket);
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	generation = windows_next_generation(windows_listener_generations[slot]);
	windows_listener_generations[slot] = generation;
	windows_listener_sockets[slot] = native_socket;
	return windows_make_handle(false, (uint16_t)slot, generation);
}

static int windows_socket_recv(socket_device_handle_t client,
							   void *destination,
							   size_t capacity)
{
	int slot = windows_resolve_client(client);
	SOCKET native_socket;
	int native_capacity;
	int result;

	if (slot < 0)
	{
		return SOCKET_DEVICE_INVALID;
	}
	if (capacity == 0U)
	{
		return 0;
	}
	if (!destination)
	{
		return SOCKET_DEVICE_INVALID;
	}

	native_socket = windows_client_sockets[slot];
	native_capacity = capacity > (size_t)INT_MAX ? INT_MAX : (int)capacity;
	result = recv(native_socket, (char *)destination, native_capacity, 0);

	if (result > 0)
	{
		/*
		 * Keep the readable hint latched. The core keeps READABLE set across
		 * positive reads and calls recv() again until WOULD_BLOCK or closure.
		 * This naturally drains final payload before a later recv()==0 FIN.
		 */
		return result;
	}

	if (result == 0)
	{
		return windows_fail_client((uint16_t)slot, SOCKET_DEVICE_CLOSED);
	}

	result = windows_socket_map_io_error(WSAGetLastError());
	if (result == SOCKET_DEVICE_WOULD_BLOCK)
	{
		/* A future read-ready observation must be allowed to notify again. */
		windows_client_set_readable_notified((uint16_t)slot, false);
		return result;
	}

	return windows_fail_client((uint16_t)slot, result);
}

static int windows_socket_send(socket_device_handle_t client,
							   const void *source,
							   size_t length)
{
	int slot = windows_resolve_client(client);
	SOCKET native_socket;
	int native_length;
	int result;

	if (slot < 0)
	{
		return SOCKET_DEVICE_INVALID;
	}
	if (length == 0U)
	{
		return 0;
	}
	if (!source)
	{
		return SOCKET_DEVICE_INVALID;
	}

	native_socket = windows_client_sockets[slot];
	native_length = length > (size_t)INT_MAX ? INT_MAX : (int)length;

	/*
	 * Exactly one non-blocking native send attempt. WinSock copies/owns bytes
	 * reported as sent before return, so the caller's source pointer is never
	 * retained. No backend queue, retry offset, or writable event is needed.
	 */
	result = send(native_socket, (const char *)source, native_length, 0);
	if (result > 0)
	{
		return result;
	}

	if (result == 0)
	{
		/* No bytes accepted for a non-zero request: temporary backpressure. */
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	result = windows_socket_map_io_error(WSAGetLastError());
	if (result == SOCKET_DEVICE_WOULD_BLOCK)
	{
		return result;
	}

	return windows_fail_client((uint16_t)slot, result);
}

static int windows_socket_close(socket_device_handle_t handle)
{
	bool client;
	uint16_t slot;
	uint16_t generation;
	SOCKET native_socket;
	int result;
	int error;

	if (!windows_decode_handle(handle, &client, &slot, &generation))
	{
		return SOCKET_DEVICE_INVALID;
	}

	if (!client)
	{
		if (slot >= WINDOWS_SOCKET_MAX_LISTENERS ||
			windows_listener_sockets[slot] == INVALID_SOCKET ||
			windows_listener_generations[slot] != generation)
		{
			return SOCKET_DEVICE_INVALID;
		}

		native_socket = windows_listener_sockets[slot];
		/* Invalidate first so a reused native SOCKET cannot revive this handle. */
		windows_reset_listener(slot);
	}
	else
	{
		if (slot >= WINDOWS_SOCKET_MAX_CLIENTS ||
			windows_client_sockets[slot] == INVALID_SOCKET ||
			windows_client_generations[slot] != generation)
		{
			return SOCKET_DEVICE_INVALID;
		}

		native_socket = windows_client_sockets[slot];
		/* Local close owns no transport event; the core schedules disconnect. */
		windows_reset_client(slot);
	}

	/* closesocket() is bounded with default non-lingering WinSock semantics. */
	result = closesocket(native_socket);
	if (result == 0)
	{
		return SOCKET_DEVICE_OK;
	}
	error = WSAGetLastError();
	return windows_socket_map_close_error(error);
}

/* Accepts at most one native client and emits at most one accepted() event. */
static void windows_poll_accept(const fd_set *readfds,
								uint16_t budget,
								uint16_t *emitted)
{
	uint16_t checked;
	uint16_t start;

	if (*emitted >= budget)
	{
		return;
	}

	start = (uint16_t)(windows_listener_cursor % WINDOWS_SOCKET_MAX_LISTENERS);
	for (checked = 0U; checked < WINDOWS_SOCKET_MAX_LISTENERS; ++checked)
	{
		uint16_t listener_slot =
			(uint16_t)((start + checked) % WINDOWS_SOCKET_MAX_LISTENERS);
		SOCKET listener_socket = windows_listener_sockets[listener_slot];
		SOCKET client_socket;
		socket_device_handle_t listener_handle;
		socket_device_handle_t client_handle;
		u_long nonblocking = 1UL;
		uint16_t client_generation;
		int client_slot;
		socket_device_token_t token;

		if (listener_socket == INVALID_SOCKET ||
			!FD_ISSET(listener_socket, readfds))
		{
			continue;
		}

		windows_listener_cursor =
			(uint16_t)((listener_slot + 1U) % WINDOWS_SOCKET_MAX_LISTENERS);
		client_socket = accept(listener_socket, NULL, NULL);
		if (client_socket == INVALID_SOCKET)
		{
			/* One native accept attempt per poll keeps work strictly bounded. */
			return;
		}

		client_slot = windows_find_free_client();
		if (client_slot < 0 ||
			ioctlsocket(client_socket, FIONBIO, &nonblocking) == SOCKET_ERROR)
		{
			(void)closesocket(client_socket);
			return;
		}

		client_generation =
			windows_next_generation(windows_client_generations[client_slot]);
		windows_client_generations[client_slot] = client_generation;
		windows_client_sockets[client_slot] = client_socket;
		windows_client_tokens[client_slot] = SOCKET_DEVICE_INVALID_TOKEN;
		windows_client_set_readable_notified((uint16_t)client_slot, false);

		listener_handle = windows_make_handle(
			false, listener_slot, windows_listener_generations[listener_slot]);
		client_handle = windows_make_handle(
			true, (uint16_t)client_slot, client_generation);
		token = windows_socket_events->accepted(listener_handle, client_handle);
		++(*emitted);

		if (token == SOCKET_DEVICE_INVALID_TOKEN)
		{
			/* Rejected clients never own a token and never emit closed(). */
			windows_reset_client((uint16_t)client_slot);
			(void)closesocket(client_socket);
			return;
		}

		/*
		 * No asynchronous producer exists in this backend, so the record cannot
		 * change during accepted(). Store the exact opaque token before any later
		 * readable/closed event can be generated.
		 */
		windows_client_tokens[client_slot] = token;
		return;
	}

	/* Rotate the first listener examined even when none was ready. */
	windows_listener_cursor =
		(uint16_t)((start + 1U) % WINDOWS_SOCKET_MAX_LISTENERS);
}

static void windows_poll_clients(const fd_set *readfds,
								 uint16_t budget,
								 uint16_t *emitted)
{
	uint16_t checked;
	uint16_t start;
	bool emitted_client_event = false;

	if (*emitted >= budget)
	{
		return;
	}

	start = (uint16_t)(windows_client_cursor % WINDOWS_SOCKET_MAX_CLIENTS);
	for (checked = 0U;
		 checked < WINDOWS_SOCKET_MAX_CLIENTS && *emitted < budget;
		 ++checked)
	{
		uint16_t slot =
			(uint16_t)((start + checked) % WINDOWS_SOCKET_MAX_CLIENTS);
		SOCKET native_socket = windows_client_sockets[slot];
		socket_device_token_t token = windows_client_tokens[slot];

		if (native_socket == INVALID_SOCKET ||
			token == SOCKET_DEVICE_INVALID_TOKEN ||
			windows_client_readable_is_notified(slot) ||
			!FD_ISSET(native_socket, readfds))
		{
			continue;
		}

		/*
		 * Latch before calling the event sink. The core retains READABLE across
		 * positive recv() results; backend recv() clears this latch only after it
		 * actually observes WOULD_BLOCK so a later arrival can notify again.
		 */
		windows_client_set_readable_notified(slot, true);
		windows_client_cursor =
			(uint16_t)((slot + 1U) % WINDOWS_SOCKET_MAX_CLIENTS);
		emitted_client_event = true;
		++(*emitted);
		windows_socket_events->readable(token);
	}

	if (!emitted_client_event)
	{
		windows_client_cursor =
			(uint16_t)((start + 1U) % WINDOWS_SOCKET_MAX_CLIENTS);
	}
}

static void windows_socket_poll(uint16_t budget)
{
	fd_set readfds;
	struct timeval timeout;
	uint16_t i;
	uint16_t emitted = 0U;
	bool watched = false;
	int ready;

	if (!windows_socket_events || budget == 0U)
	{
		return;
	}

	FD_ZERO(&readfds);
	for (i = 0U; i < WINDOWS_SOCKET_MAX_LISTENERS; ++i)
	{
		if (windows_listener_sockets[i] != INVALID_SOCKET)
		{
			FD_SET(windows_listener_sockets[i], &readfds);
			watched = true;
		}
	}
	for (i = 0U; i < WINDOWS_SOCKET_MAX_CLIENTS; ++i)
	{
		if (windows_client_sockets[i] == INVALID_SOCKET ||
			windows_client_tokens[i] == SOCKET_DEVICE_INVALID_TOKEN ||
			windows_client_readable_is_notified(i))
		{
			continue;
		}

		FD_SET(windows_client_sockets[i], &readfds);
		watched = true;
	}

	/* WinSock select() requires at least one non-empty descriptor set. */
	if (!watched)
	{
		return;
	}

	timeout.tv_sec = 0L;
	timeout.tv_usec = 0L;
	/* WinSock ignores the first select() argument; zero timeout never blocks. */
	ready = select(0, &readfds, NULL, NULL, &timeout);
	if (ready == SOCKET_ERROR || ready == 0)
	{
		return;
	}

	/* Alternate phase order so budget==1 cannot starve clients or accepts. */
	if ((windows_state.flags & WINDOWS_STATE_ACCEPT_FIRST) != 0U)
	{
		windows_poll_accept(&readfds, budget, &emitted);
		windows_poll_clients(&readfds, budget, &emitted);
	}
	else
	{
		windows_poll_clients(&readfds, budget, &emitted);
		windows_poll_accept(&readfds, budget, &emitted);
	}
	windows_state.flags ^= WINDOWS_STATE_ACCEPT_FIRST;
}

/* Existing emulator integration symbol retained for compatibility. */
socket_device_t wifi_socket = {
	.init = windows_socket_device_init,
	.listen = windows_socket_listen,
	.recv = windows_socket_recv,
	.send = windows_socket_send,
	.close = windows_socket_close,
	.poll = windows_socket_poll};

#endif /* ENABLE_SOCKETS */


#ifdef __cplusplus
}
#endif

#endif
