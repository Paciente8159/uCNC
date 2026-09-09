/* linux_port.c
 *
 * Linux-compatible implementations of the functions from the Windows virtual MCU file.
 * Maintains the same function names and declarations.
 *
 * Compile on Linux with: -pthread
 */

#include "../../../cnc.h"
#if (MCU == MCU_VIRTUAL_LINUX)

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
#include <time.h>
#include <errno.h>

/* Platform includes */
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <dirent.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h>

/* Provide Windows-like types so function signatures remain unchanged */
#ifndef HANDLE
    typedef int HANDLE;
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE (-1)
#endif

    /* Minimal C serial “driver” */
    typedef struct serial_port_
    {
        HANDLE h;
        volatile bool connected;
        unsigned long errors;
        /* Windows COMSTAT not available; emulate minimal status */
        struct
        {
            unsigned long cbInQue;
        } status;
        char port_name[128];
        pthread_t thread;
        volatile bool stop;
    } serial_port_t;

    serial_port_t g_uart = {0};

    /* Forward declarations for callbacks referenced in ioserver code */
    extern void mcu_limits_changed_cb(void);
    extern void mcu_probe_changed_cb(void);
    extern void mcu_controls_changed_cb(void);
    extern void mcu_inputs_changed_cb(void);

    /* ---------------- Serial (termios) ------------------------------------- */

    bool serial_configure(HANDLE h)
    {
        if (h < 0)
        {
            printf("Serial: invalid handle\n");
            return false;
        }

        struct termios tty;
        if (tcgetattr(h, &tty) != 0)
        {
            perror("Serial: tcgetattr failed");
            return false;
        }

        /* Mirror original WindowsSerial: 9600 8N1, DTR enabled (DTR not directly controllable here) */
        cfsetospeed(&tty, B9600);
        cfsetispeed(&tty, B9600);

        tty.c_cflag &= ~PARENB; /* no parity */
        tty.c_cflag &= ~CSTOPB; /* one stop bit */
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;            /* 8 bits */
        tty.c_cflag |= CREAD | CLOCAL; /* enable receiver, ignore modem control lines */

        /* Raw mode */
        tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        tty.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL);
        tty.c_oflag &= ~OPOST;

        /* Non-blocking reads: quick return if no data */
        tty.c_cc[VMIN] = 0;
        tty.c_cc[VTIME] = 1; /* 0.1s */

        if (tcsetattr(h, TCSANOW, &tty) != 0)
        {
            perror("Serial: tcsetattr failed");
            return false;
        }

        /* Flush input/output */
        tcflush(h, TCIOFLUSH);
        return true;
    }

    int serial_read(char *buffer, unsigned int nbChar)
    {
        if (!g_uart.connected)
            return 0;

        if (g_uart.h < 0)
            return 0;

        ssize_t r = read(g_uart.h, buffer, nbChar);
        if (r < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return 0;
            perror("Serial: read error");
            return 0;
        }
        return (int)r;
    }

    bool serial_write(const uint8_t *buffer, unsigned int nbChar)
    {
        if (!g_uart.connected)
            return false;
        if (g_uart.h < 0)
            return false;

        size_t written = 0;
        while (written < nbChar)
        {
            ssize_t w = write(g_uart.h, buffer + written, nbChar - written);
            if (w > 0)
            {
                written += (size_t)w;
                continue;
            }
            if (w < 0 && errno == EINTR)
                continue;
            if (w < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
                return false;

            g_uart.connected = false;
            perror("Serial: write error");
            return false;
        }
        return true;
    }

    /* UART thread: tries to open and keep serial port alive */
    void *uart_thread_fn(void *arg)
    {
        (void)arg;
        while (!g_uart.stop)
        {
            if (!g_uart.connected)
            {
                /* Open serial port (port_name expected like "/dev/ttyUSB0" or "/dev/ttyS0") */
                int fd = open(g_uart.port_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
                if (fd < 0)
                {
                    /* Port not present yet; non-blocking wait/retry */
                    usleep(500 * 1000);
                    continue;
                }

                if (!serial_configure(fd))
                {
                    close(fd);
                    usleep(500 * 1000);
                    continue;
                }

                g_uart.h = fd;
                g_uart.connected = true;
                /* No startup sleep; non-blocking startup */
                // printf("Serial: connected to %s\n", g_uart.port_name);
            }
            else
            {
                /* Check for hang-up/errors occasionally */
                int bytes;
                if (ioctl(g_uart.h, FIONREAD, &bytes) == 0)
                {
                    g_uart.status.cbInQue = (unsigned long)bytes;
                }
                /* Light duty periodic sleep to avoid busy loop */
                usleep(10 * 1000);
            }
        }

        if (g_uart.connected)
        {
            g_uart.connected = false;
            if (g_uart.h >= 0)
                close(g_uart.h);
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
        /* Expect UART_PORT_NAME to be defined (e.g., "/dev/ttyUSB0") */
        strncpy(g_uart.port_name, UART_PORT_NAME, sizeof(g_uart.port_name) - 1);
        g_uart.port_name[sizeof(g_uart.port_name) - 1] = '\0';
        pthread_create(&g_uart.thread, NULL, &uart_thread_fn, NULL);
    }

    /* ---------------- Console helpers (kbhit/getch) ------------------------ */

    /* console_kbhit: non-blocking check for stdin data */
    int console_kbhit(void)
    {
        struct timeval tv = {0, 0};
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
        if (ret > 0 && FD_ISSET(STDIN_FILENO, &fds))
            return 1;
        return 0;
    }

    /* console_getch: blocking single character read without echo */
    int console_getch(void)
    {
        struct termios oldt, newt;
        int ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }

/* ---------------- IO server (Unix domain socket as named pipe) --------- */

/* Path for the Unix domain socket */
#define UCNCCIO_SOCKET_PATH "/tmp/ucncio.sock"

    void *ioserver(void *args)
    {
        (void)args;
        int server_fd = -1, client_fd = -1;
        struct sockaddr_un addr;
        ssize_t n;
        size_t map_size = sizeof(VIRTUAL_MAP);

        /* Ensure old socket removed */
        unlink(UCNCCIO_SOCKET_PATH);

        server_fd = socket(AF_UNIX, 1, 0);
        if (server_fd < 0)
        {
            perror("Create socket failed");
            return NULL;
        }

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, UCNCCIO_SOCKET_PATH, sizeof(addr.sun_path) - 1);

        if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("bind failed");
            close(server_fd);
            return NULL;
        }

        if (listen(server_fd, 5) < 0)
        {
            perror("listen failed");
            close(server_fd);
            return NULL;
        }

        for (;;)
        {
            client_fd = accept(server_fd, NULL, NULL);
            if (client_fd < 0)
            {
                perror("accept failed");
                continue;
            }

            /* Exchange VIRTUAL_MAP repeatedly until client disconnects */
            uint8_t lpvMessage[sizeof(VIRTUAL_MAP)];
            bool fSuccess = true;
            while (fSuccess)
            {
                /* Send current virtualmap */
                memcpy(lpvMessage, (const void *)&virtualmap, map_size);
                size_t sent = 0;
                while (sent < map_size)
                {
                    n = write(client_fd, lpvMessage + sent, map_size - sent);
                    if (n > 0)
                    {
                        sent += (size_t)n;
                        continue;
                    }
                    if (n < 0 && errno == EINTR)
                        continue;
                    if (n < 0)
                        perror("write to socket failed");
                    fSuccess = false;
                    break;
                }
                if (!fSuccess)
                    break;

                /* Read back updated map (blocking read) */
                ssize_t total = 0;
                while (total < (ssize_t)map_size)
                {
                    n = read(client_fd, lpvMessage + total, map_size - total);
                    if (n < 0 && errno == EINTR)
                        continue;
                    if (n <= 0)
                    {
                        if (n == 0)
                            ; /* client closed */
                        else
                            perror("read from socket failed");
                        fSuccess = false;
                        break;
                    }
                    total += n;
                }
                if (!fSuccess)
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
            }

            close(client_fd);
            client_fd = -1;
        }

        /* never reached */
        if (server_fd >= 0)
            close(server_fd);
        unlink(UCNCCIO_SOCKET_PATH);
        return NULL;
    }

    /* ---------------- Timer (POSIX timer_create) -------------------------- */

    static timer_t posix_timer = (timer_t)0;
    static void (*timer_func_handler_pntr)(void) = NULL;

    static void posix_timer_handler(union sigval sv)
    {
        (void)sv;
        if (timer_func_handler_pntr)
            timer_func_handler_pntr();
    }

    /* start_timer: create a periodic timer that calls timer_func_handler every mSec milliseconds */
    int start_timer(int mSec, void (*timer_func_handler)(void))
    {
        struct sigevent sev;
        struct itimerspec its;
        int res;

        timer_func_handler_pntr = timer_func_handler;

        memset(&sev, 0, sizeof(sev));
        sev.sigev_notify = SIGEV_THREAD;
        sev.sigev_value.sival_ptr = &posix_timer;
        sev.sigev_notify_function = (void (*)(union sigval))posix_timer_handler;
        sev.sigev_notify_attributes = NULL;

        res = timer_create(CLOCK_REALTIME, &sev, &posix_timer);
        if (res != 0)
        {
            perror("timer_create failed");
            return 1;
        }

        its.it_value.tv_sec = mSec / 1000;
        its.it_value.tv_nsec = (mSec % 1000) * 1000000;
        its.it_interval = its.it_value;

        if (timer_settime(posix_timer, 0, &its, NULL) != 0)
        {
            perror("timer_settime failed");
            timer_delete(posix_timer);
            posix_timer = (timer_t)0;
            return 1;
        }

        return 0;
    }

    void stop_timer(void)
    {
        if (posix_timer != (timer_t)0)
        {
            timer_delete(posix_timer);
            posix_timer = (timer_t)0;
        }
    }

    /* ---------------- Flash filesystem shim (host FS) --------------------- */

#include "src/modules/file_system.h"

    fs_t flash_fs;

    bool flash_fs_finfo(const char *path, fs_file_info_t *finfo)
    {
        if (!path || !finfo)
            return false;

        char fpath[256];
        if (strcmp("/", path) == 0 || strcmp(".", path) == 0)
        {
            strncpy(fpath, "./", sizeof(fpath) - 1);
            fpath[sizeof(fpath) - 1] = '\0';
        }
        else
        {
            snprintf(fpath, sizeof(fpath), "./%s", path);
        }

        struct stat st;
        if (stat(fpath, &st) != 0)
            return false;

        strncpy(finfo->full_name, path, FS_PATH_NAME_MAX_LEN - 1);
        finfo->full_name[FS_PATH_NAME_MAX_LEN - 1] = '\0';

        finfo->is_dir = S_ISDIR(st.st_mode) ? true : false;
        finfo->size = finfo->is_dir ? 0u : (uint32_t)st.st_size;
        finfo->timestamp = (uint32_t)st.st_mtime;

        return true;
    }

    fs_file_t *flash_fs_opendir(const char *path)
    {
        fs_file_t *fp = (fs_file_t *)calloc(1, sizeof(fs_file_t));
        if (!fp)
            return NULL;
        char dir[256] = ".";
        if (strcmp("/", path))
        {
            /* append slash if needed */
            strncat(dir, "/", sizeof(dir) - strlen(dir) - 1);
            strncat(dir, path, sizeof(dir) - strlen(dir) - 1);
        }

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
        {
            strncat(file, "/", sizeof(file) - strlen(file) - 1);
            strncat(file, path, sizeof(file) - strlen(file) - 1);
        }

        FILE *tmpfile = fopen(file, mode);
        if (!flash_fs_finfo(path, &finfo))
        {
            if (tmpfile)
                fclose(tmpfile);
            return NULL;
        }

        if (!finfo.is_dir)
        {
            fs_file_t *fp = (fs_file_t *)calloc(1, sizeof(fs_file_t));
            if (!fp)
            {
                if (tmpfile)
                    fclose(tmpfile);
                return NULL;
            }
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
            if (fseek((FILE *)fp->file_ptr, (long)position, SEEK_SET) == 0)
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
        if (flash_fs.drive)
            return remove(path) == 0;
        return false;
    }
    bool flash_fs_mkdir(const char *path)
    {
        if (flash_fs.drive)
            return mkdir(path, 0755) == 0;
        return false;
    }
    bool flash_fs_rmdir(const char *path)
    {
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

    /* get_current_dir: fill cwd buffer with current working directory */
    void get_current_dir(char *cwd, size_t len)
    {
        if (!cwd || len == 0)
            return;
        if (getcwd(cwd, len) == NULL)
        {
            /* fallback to empty string */
            cwd[0] = '\0';
        }
    }

    /* ---------------- Sockets (no Winsock init) --------------------------- */

#if defined(ENABLE_SOCKETS)

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../../../modules/net/socket.h"

#define LINUX_SOCKET_MAX_LISTENERS MAX_SOCKETS
#define LINUX_SOCKET_MAX_CLIENTS SOCKET_MAX_CONNECTIONS

#if LINUX_SOCKET_MAX_LISTENERS == 0
#error "LINUX_SOCKET_MAX_LISTENERS must be greater than zero"
#endif

#if LINUX_SOCKET_MAX_CLIENTS == 0
#error "LINUX_SOCKET_MAX_CLIENTS must be greater than zero"
#endif

/*
 * POSIX sockets fd_set stores a fixed number of int values. readfds contains all
 * listeners plus clients whose readable hint is not already latched by uCNC.
 * Fail at compile time instead of letting FD_SET silently exceed capacity.
 */
#if (LINUX_SOCKET_MAX_LISTENERS + LINUX_SOCKET_MAX_CLIENTS) > FD_SETSIZE
#error "Increase FD_SETSIZE or reduce MAX_SOCKETS/SOCKET_MAX_CONNECTIONS"
#endif

/*
 * Backend handles are generation-tagged table references, not raw int
 * values. This prevents a stale uCNC handle from becoming valid again merely
 * because POSIX sockets later reuses the same native int value.
 *
 * The low 16 bits encode (slot << 1) | kind and the high 16 bits encode a
 * non-zero generation. Keeping one kind bit leaves 15 bits for the slot.
 * FD_SETSIZE is normally far smaller, but make the representation limit
 * explicit so a custom configuration cannot silently truncate a slot.
 */
#if LINUX_SOCKET_MAX_LISTENERS > 32767U
#error "LINUX_SOCKET_MAX_LISTENERS exceeds backend handle slot capacity"
#endif

#if LINUX_SOCKET_MAX_CLIENTS > 32767U
#error "LINUX_SOCKET_MAX_CLIENTS exceeds backend handle slot capacity"
#endif

#define LINUX_HANDLE_KIND_CLIENT ((uintptr_t)1U)
#define LINUX_HANDLE_SLOT_SHIFT 1U
#define LINUX_HANDLE_GENERATION_SHIFT 16U
#define LINUX_HANDLE_LOW_MASK ((uintptr_t)0xFFFFU)
#define LINUX_HANDLE_SLOT_MASK ((uintptr_t)0x7FFFU)

#define LINUX_CLIENT_READABLE_BYTES \
	((LINUX_SOCKET_MAX_CLIENTS + 7U) / 8U)

/*
 * Structure-of-arrays storage avoids per-record alignment padding on 64-bit hosts.
 * Keeping the arrays in one aggregate also prevents linker alignment gaps
 * between separate static objects. Native TCP payload stays entirely in
 * POSIX sockets buffers; there is no backend RX copy, TX queue, TX retry offset, or
 * writable-interest state.
 */
typedef struct linux_socket_state_
{
	int listener_sockets[LINUX_SOCKET_MAX_LISTENERS];
	int client_sockets[LINUX_SOCKET_MAX_CLIENTS];
	const socket_device_events_t *events;
	socket_device_token_t client_tokens[LINUX_SOCKET_MAX_CLIENTS];
	uint16_t client_generations[LINUX_SOCKET_MAX_CLIENTS];
	uint16_t listener_generations[LINUX_SOCKET_MAX_LISTENERS];
	uint16_t listener_cursor;
	uint16_t client_cursor;
	uint8_t client_readable_notified[LINUX_CLIENT_READABLE_BYTES];
	uint8_t flags;
} linux_socket_state_t;

#define LINUX_STATE_NET_STARTED (1U << 0)
#define LINUX_STATE_ACCEPT_FIRST (1U << 1)

static linux_socket_state_t linux_state;

#define linux_listener_sockets linux_state.listener_sockets
#define linux_client_sockets linux_state.client_sockets
#define linux_socket_events linux_state.events
#define linux_client_tokens linux_state.client_tokens
#define linux_client_generations linux_state.client_generations
#define linux_listener_generations linux_state.listener_generations
#define linux_listener_cursor linux_state.listener_cursor
#define linux_client_cursor linux_state.client_cursor
#define linux_client_readable_notified linux_state.client_readable_notified

static uint16_t linux_next_generation(uint16_t generation)
{
	++generation;
	if (generation == 0U)
	{
		++generation;
	}
	return generation;
}

static socket_device_handle_t linux_make_handle(bool client,
										   uint16_t slot,
										   uint16_t generation)
{
	uintptr_t value = ((uintptr_t)generation << LINUX_HANDLE_GENERATION_SHIFT) |
					  ((uintptr_t)slot << LINUX_HANDLE_SLOT_SHIFT);

	if (client)
	{
		value |= LINUX_HANDLE_KIND_CLIENT;
	}
	return (socket_device_handle_t)value;
}

/*
 * Decodes only handles produced by linux_make_handle(). Re-encoding and
 * comparing rejects malformed values and, on 64-bit hosts, values with unexpected
 * upper bits without relying on a shift as wide as uintptr_t on 32-bit hosts.
 */
static bool linux_decode_handle(socket_device_handle_t handle,
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

	low = value & LINUX_HANDLE_LOW_MASK;
	decoded_client = (low & LINUX_HANDLE_KIND_CLIENT) != 0U;
	decoded_slot = (uint16_t)((low >> LINUX_HANDLE_SLOT_SHIFT) &
								LINUX_HANDLE_SLOT_MASK);
	decoded_generation =
		(uint16_t)(value >> LINUX_HANDLE_GENERATION_SHIFT);

	if (decoded_generation == 0U ||
		linux_make_handle(decoded_client, decoded_slot,
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

static int linux_resolve_client(socket_device_handle_t handle)
{
	bool client;
	uint16_t slot;
	uint16_t generation;

	if (!linux_decode_handle(handle, &client, &slot, &generation) || !client ||
		slot >= LINUX_SOCKET_MAX_CLIENTS ||
		linux_client_sockets[slot] == -1 ||
		linux_client_generations[slot] != generation)
	{
		return -1;
	}
	return (int)slot;
}

static int linux_find_free_listener(void)
{
	uint16_t i;

	for (i = 0U; i < LINUX_SOCKET_MAX_LISTENERS; ++i)
	{
		if (linux_listener_sockets[i] == -1)
		{
			return (int)i;
		}
	}

	return -1;
}

static int linux_find_free_client(void)
{
	uint16_t i;

	for (i = 0U; i < LINUX_SOCKET_MAX_CLIENTS; ++i)
	{
		if (linux_client_sockets[i] == -1)
		{
			return (int)i;
		}
	}

	return -1;
}

static bool linux_client_readable_is_notified(uint16_t slot)
{
	uint8_t mask = (uint8_t)(1U << (slot & 7U));
	return (linux_client_readable_notified[slot >> 3] & mask) != 0U;
}

static void linux_client_set_readable_notified(uint16_t slot, bool notified)
{
	uint8_t *byte = &linux_client_readable_notified[slot >> 3];
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

static void linux_reset_listener(uint16_t slot)
{
	linux_listener_sockets[slot] = -1;
	/* Preserve generation so the next lifetime receives a different handle. */
}

static void linux_reset_client(uint16_t slot)
{
	linux_client_sockets[slot] = -1;
	linux_client_tokens[slot] = SOCKET_DEVICE_INVALID_TOKEN;
	linux_client_set_readable_notified(slot, false);
	/* Preserve generation so the next lifetime receives a different handle. */
}

/* Errors for which retrying a later non-blocking RX/TX attempt is valid. */
static bool linux_socket_error_is_temporary(int error)
{
	return error == EWOULDBLOCK || error == EAGAIN || error == EINTR ||
		   error == ENOBUFS;
}

/*
 * Maps recv()/send() errors. Reset, abort, timeout and network failures are
 * deliberately normalized to generic fatal ERROR; only recv()==0 is the
 * orderly CLOSED path. EINTR is not retried inside the backend so recv/send
 * remain exactly one native I/O attempt per call.
 */
static int linux_socket_map_io_error(int error)
{
	if (linux_socket_error_is_temporary(error))
	{
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	switch (error)
	{
	case ENOTSOCK:
	case EINVAL:
	case EFAULT:
		return SOCKET_DEVICE_INVALID;
	default:
		return SOCKET_DEVICE_ERROR;
	}
}

static int linux_socket_map_close_error(int error)
{
	return error == EBADF || error == ENOTSOCK || error == EINVAL
			   ? SOCKET_DEVICE_INVALID
			   : SOCKET_DEVICE_ERROR;
}

/*
 * Releases a remotely/fatally closed client before notifying the core. The
 * token is copied first because reset invalidates the backend association.
 * This helper is never used for local close(), which must not emit closed().
 */
static int linux_fail_client(uint16_t slot, int reason)
{
	int native_socket = linux_client_sockets[slot];
	socket_device_token_t token = linux_client_tokens[slot];

	linux_reset_client(slot);
	(void)close(native_socket);
	linux_socket_events->closed(token, reason);
	return reason;
}

/*
 * Compatibility entry point used by the emulator network startup. POSIX
 * sockets need no process-wide initialization.
 */
int socket_init(void)
{
	linux_state.flags |= LINUX_STATE_NET_STARTED;
	return 0;
}

static int linux_set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	return flags < 0 ? -1 : fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static int linux_socket_device_init(const socket_device_events_t *events)
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

	memset(linux_client_readable_notified, 0,
		   sizeof(linux_client_readable_notified));
	for (i = 0U; i < LINUX_SOCKET_MAX_LISTENERS; ++i)
	{
		linux_listener_generations[i] = 0U;
		linux_reset_listener(i);
	}
	for (i = 0U; i < LINUX_SOCKET_MAX_CLIENTS; ++i)
	{
		linux_client_generations[i] = 0U;
		linux_reset_client(i);
	}

	linux_listener_cursor = 0U;
	linux_client_cursor = 0U;
	linux_state.flags = (uint8_t)(linux_state.flags | LINUX_STATE_ACCEPT_FIRST);
	linux_socket_events = events;
	return SOCKET_DEVICE_OK;
}

static socket_device_handle_t linux_socket_listen(
	const socket_device_endpoint_t *endpoint,
	uint8_t backlog)
{
	struct sockaddr_in address;
	int native_socket;
	uint16_t generation;
	int slot;
	int native_backlog;

	if (!linux_socket_events || !endpoint || endpoint->port == 0U)
	{
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	slot = linux_find_free_listener();
	if (slot < 0)
	{
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	native_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (native_socket == -1)
	{
		return SOCKET_DEVICE_INVALID_HANDLE;
	}
	if (native_socket >= FD_SETSIZE)
	{
		(void)close(native_socket);
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	/* Configure non-blocking mode before the socket becomes externally usable. */
	if (linux_set_nonblocking(native_socket) == -1)
	{
		(void)close(native_socket);
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(endpoint->port);
	address.sin_addr.s_addr = htonl(endpoint->address);

	/* A zero backlog is a bounded request for one pending connection. */
	native_backlog = backlog == 0U ? 1 : (int)backlog;
	if (bind(native_socket, (const struct sockaddr *)&address,
			 (socklen_t)sizeof(address)) == -1 ||
		listen(native_socket, native_backlog) == -1)
	{
		(void)close(native_socket);
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	generation = linux_next_generation(linux_listener_generations[slot]);
	linux_listener_generations[slot] = generation;
	linux_listener_sockets[slot] = native_socket;
	return linux_make_handle(false, (uint16_t)slot, generation);
}

static int linux_socket_recv(socket_device_handle_t client,
							   void *destination,
							   size_t capacity)
{
	int slot = linux_resolve_client(client);
	int native_socket;
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

	native_socket = linux_client_sockets[slot];
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
		return linux_fail_client((uint16_t)slot, SOCKET_DEVICE_CLOSED);
	}

	result = linux_socket_map_io_error(errno);
	if (result == SOCKET_DEVICE_WOULD_BLOCK)
	{
		/* A future read-ready observation must be allowed to notify again. */
		linux_client_set_readable_notified((uint16_t)slot, false);
		return result;
	}

	return linux_fail_client((uint16_t)slot, result);
}

static int linux_socket_send(socket_device_handle_t client,
							   const void *source,
							   size_t length)
{
	int slot = linux_resolve_client(client);
	int native_socket;
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

	native_socket = linux_client_sockets[slot];
	native_length = length > (size_t)INT_MAX ? INT_MAX : (int)length;

	/*
	 * Exactly one non-blocking native send attempt. POSIX sockets copies/owns bytes
	 * reported as sent before return, so the caller's source pointer is never
	 * retained. No backend queue, retry offset, or writable event is needed.
	 */
	result = send(native_socket, (const char *)source, native_length,
#ifdef MSG_NOSIGNAL
				  MSG_NOSIGNAL
#else
				  0
#endif
	);
	if (result > 0)
	{
		return result;
	}

	if (result == 0)
	{
		/* No bytes accepted for a non-zero request: temporary backpressure. */
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	result = linux_socket_map_io_error(errno);
	if (result == SOCKET_DEVICE_WOULD_BLOCK)
	{
		return result;
	}

	return linux_fail_client((uint16_t)slot, result);
}

static int linux_socket_close(socket_device_handle_t handle)
{
	bool client;
	uint16_t slot;
	uint16_t generation;
	int native_socket;
	int result;
	int error;

	if (!linux_decode_handle(handle, &client, &slot, &generation))
	{
		return SOCKET_DEVICE_INVALID;
	}

	if (!client)
	{
		if (slot >= LINUX_SOCKET_MAX_LISTENERS ||
			linux_listener_sockets[slot] == -1 ||
			linux_listener_generations[slot] != generation)
		{
			return SOCKET_DEVICE_INVALID;
		}

		native_socket = linux_listener_sockets[slot];
		/* Invalidate first so a reused native int cannot revive this handle. */
		linux_reset_listener(slot);
	}
	else
	{
		if (slot >= LINUX_SOCKET_MAX_CLIENTS ||
			linux_client_sockets[slot] == -1 ||
			linux_client_generations[slot] != generation)
		{
			return SOCKET_DEVICE_INVALID;
		}

		native_socket = linux_client_sockets[slot];
		/* Local close owns no transport event; the core schedules disconnect. */
		linux_reset_client(slot);
	}

	/* close() is bounded with default non-lingering POSIX sockets semantics. */
	result = close(native_socket);
	if (result == 0)
	{
		return SOCKET_DEVICE_OK;
	}
	error = errno;
	return linux_socket_map_close_error(error);
}

/* Accepts at most one native client and emits at most one accepted() event. */
static void linux_poll_accept(const fd_set *readfds,
								uint16_t budget,
								uint16_t *emitted)
{
	uint16_t checked;
	uint16_t start;

	if (*emitted >= budget)
	{
		return;
	}

	start = (uint16_t)(linux_listener_cursor % LINUX_SOCKET_MAX_LISTENERS);
	for (checked = 0U; checked < LINUX_SOCKET_MAX_LISTENERS; ++checked)
	{
		uint16_t listener_slot =
			(uint16_t)((start + checked) % LINUX_SOCKET_MAX_LISTENERS);
		int listener_socket = linux_listener_sockets[listener_slot];
		int client_socket;
		socket_device_handle_t listener_handle;
		socket_device_handle_t client_handle;
		uint16_t client_generation;
		int client_slot;
		socket_device_token_t token;

		if (listener_socket == -1 ||
			!FD_ISSET(listener_socket, readfds))
		{
			continue;
		}

		linux_listener_cursor =
			(uint16_t)((listener_slot + 1U) % LINUX_SOCKET_MAX_LISTENERS);
		client_socket = accept(listener_socket, NULL, NULL);
		if (client_socket == -1)
		{
			/* One native accept attempt per poll keeps work strictly bounded. */
			return;
		}
		if (client_socket >= FD_SETSIZE)
		{
			(void)close(client_socket);
			return;
		}

		client_slot = linux_find_free_client();
		if (client_slot < 0 ||
			linux_set_nonblocking(client_socket) == -1)
		{
			(void)close(client_socket);
			return;
		}

		client_generation =
			linux_next_generation(linux_client_generations[client_slot]);
		linux_client_generations[client_slot] = client_generation;
		linux_client_sockets[client_slot] = client_socket;
		linux_client_tokens[client_slot] = SOCKET_DEVICE_INVALID_TOKEN;
		linux_client_set_readable_notified((uint16_t)client_slot, false);

		listener_handle = linux_make_handle(
			false, listener_slot, linux_listener_generations[listener_slot]);
		client_handle = linux_make_handle(
			true, (uint16_t)client_slot, client_generation);
		token = linux_socket_events->accepted(listener_handle, client_handle);
		++(*emitted);

		if (token == SOCKET_DEVICE_INVALID_TOKEN)
		{
			/* Rejected clients never own a token and never emit closed(). */
			linux_reset_client((uint16_t)client_slot);
			(void)close(client_socket);
			return;
		}

		/*
		 * No asynchronous producer exists in this backend, so the record cannot
		 * change during accepted(). Store the exact opaque token before any later
		 * readable/closed event can be generated.
		 */
		linux_client_tokens[client_slot] = token;
		return;
	}

	/* Rotate the first listener examined even when none was ready. */
	linux_listener_cursor =
		(uint16_t)((start + 1U) % LINUX_SOCKET_MAX_LISTENERS);
}

static void linux_poll_clients(const fd_set *readfds,
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

	start = (uint16_t)(linux_client_cursor % LINUX_SOCKET_MAX_CLIENTS);
	for (checked = 0U;
		 checked < LINUX_SOCKET_MAX_CLIENTS && *emitted < budget;
		 ++checked)
	{
		uint16_t slot =
			(uint16_t)((start + checked) % LINUX_SOCKET_MAX_CLIENTS);
		int native_socket = linux_client_sockets[slot];
		socket_device_token_t token = linux_client_tokens[slot];

		if (native_socket == -1 ||
			token == SOCKET_DEVICE_INVALID_TOKEN ||
			linux_client_readable_is_notified(slot) ||
			!FD_ISSET(native_socket, readfds))
		{
			continue;
		}

		/*
		 * Latch before calling the event sink. The core retains READABLE across
		 * positive recv() results; backend recv() clears this latch only after it
		 * actually observes WOULD_BLOCK so a later arrival can notify again.
		 */
		linux_client_set_readable_notified(slot, true);
		linux_client_cursor =
			(uint16_t)((slot + 1U) % LINUX_SOCKET_MAX_CLIENTS);
		emitted_client_event = true;
		++(*emitted);
		linux_socket_events->readable(token);
	}

	if (!emitted_client_event)
	{
		linux_client_cursor =
			(uint16_t)((start + 1U) % LINUX_SOCKET_MAX_CLIENTS);
	}
}

static void linux_socket_poll(uint16_t budget)
{
	fd_set readfds;
	struct timeval timeout;
	uint16_t i;
	uint16_t emitted = 0U;
	bool watched = false;
	int ready;
	int max_fd = -1;

	if (!linux_socket_events || budget == 0U)
	{
		return;
	}

	FD_ZERO(&readfds);
	for (i = 0U; i < LINUX_SOCKET_MAX_LISTENERS; ++i)
	{
		if (linux_listener_sockets[i] != -1)
		{
			FD_SET(linux_listener_sockets[i], &readfds);
			if (linux_listener_sockets[i] > max_fd)
				max_fd = linux_listener_sockets[i];
			watched = true;
		}
	}
	for (i = 0U; i < LINUX_SOCKET_MAX_CLIENTS; ++i)
	{
		if (linux_client_sockets[i] == -1 ||
			linux_client_tokens[i] == SOCKET_DEVICE_INVALID_TOKEN ||
			linux_client_readable_is_notified(i))
		{
			continue;
		}

		FD_SET(linux_client_sockets[i], &readfds);
		if (linux_client_sockets[i] > max_fd)
			max_fd = linux_client_sockets[i];
		watched = true;
	}

	/* POSIX sockets select() requires at least one non-empty descriptor set. */
	if (!watched)
	{
		return;
	}

	timeout.tv_sec = 0L;
	timeout.tv_usec = 0L;
	/* POSIX select() requires the highest descriptor plus one. */
	ready = select(max_fd + 1, &readfds, NULL, NULL, &timeout);
	if (ready < 0 || ready == 0)
	{
		return;
	}

	/* Alternate phase order so budget==1 cannot starve clients or accepts. */
	if ((linux_state.flags & LINUX_STATE_ACCEPT_FIRST) != 0U)
	{
		linux_poll_accept(&readfds, budget, &emitted);
		linux_poll_clients(&readfds, budget, &emitted);
	}
	else
	{
		linux_poll_clients(&readfds, budget, &emitted);
		linux_poll_accept(&readfds, budget, &emitted);
	}
	linux_state.flags ^= LINUX_STATE_ACCEPT_FIRST;
}

/* Existing emulator integration symbol retained for compatibility. */
socket_device_t wifi_socket = {
	.init = linux_socket_device_init,
	.listen = linux_socket_listen,
	.recv = linux_socket_recv,
	.send = linux_socket_send,
	.close = linux_socket_close,
	.poll = linux_socket_poll};

#endif /* ENABLE_SOCKETS */

#ifdef __cplusplus
}
#endif

#endif /* MCU_VIRTUAL_LINUX */
