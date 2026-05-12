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

#include "../../../modules/endpoint.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
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
    /* Link with Ws2_32.lib when building on Windows */
    /* In MinGW-w64: add -lws2_32 */

    typedef int socklen_t;

    /* Initialise Winsock 2.2 – call once at startup before using sockets */
    int socket_init(void)
    {
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData);
    }

    /* BSD-style wrappers mapped to Winsock functions */

    static int bsd_socket(int domain, int type, int protocol)
    {
        return (int)WSASocket(domain, type, protocol, NULL, 0, 0);
    }

    static int bsd_bind(int sockfd, const struct bsd_sockaddr_in *addr, socklen_t addrlen)
    {
        if (bind(sockfd, (const struct sockaddr *)addr, addrlen) < 0)
        {
            return -1;
        }
        u_long mode = 1;

        return ioctlsocket(sockfd, FIONBIO, (u_long *)&mode);
    }

    static int bsd_listen(int sockfd, int backlog)
    {
        return listen(sockfd, backlog);
    }

    static int bsd_accept(int sockfd, struct bsd_sockaddr_in *addr, socklen_t *addrlen)
    {
        return accept(sockfd, (struct sockaddr *)addr, addrlen);
    }

    static int bsd_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
    {
        return setsockopt(sockfd, level, optname, (const char *)optval, optlen);
    }

    static int bsd_recv(int sockfd, void *buf, size_t len, int flags)
    {
        return recv(sockfd, (char *)buf, (int)len, flags);
    }

    static int bsd_send(int sockfd, const void *buf, size_t len, int flags)
    {
        return send(sockfd, (const char *)buf, (int)len, flags);
    }

    static int bsd_close(int fd)
    {
        return closesocket(fd);
    }

    socket_device_t wifi_socket = {.socket = bsd_socket, .bind = bsd_bind, .listen = bsd_listen, .accept = bsd_accept, .recv = bsd_recv, .send = bsd_send, .close = bsd_close};

#endif

#ifdef __cplusplus
}
#endif

#endif
