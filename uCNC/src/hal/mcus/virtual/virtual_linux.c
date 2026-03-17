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
extern "C" {
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
    struct {
        unsigned long cbInQue;
    } status;
    char port_name[128];
    pthread_t thread;
    volatile bool stop;
} serial_port_t;

serial_port_t g_uart = {0};

/* Forward declarations for callbacks referenced in ioserver code */
extern VIRTUAL_MAP virtualmap;
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
    tty.c_cflag |= CS8;     /* 8 bits */
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

    ssize_t w = write(g_uart.h, buffer, nbChar);
    if (w < 0)
    {
        perror("Serial: write error");
        return false;
    }
    return (size_t)w == nbChar;
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

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
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
            n = write(client_fd, lpvMessage, map_size);
            if (n != (ssize_t)map_size)
            {
                perror("write to socket failed");
                break;
            }

            /* Read back updated map (blocking read) */
            ssize_t total = 0;
            while (total < (ssize_t)map_size)
            {
                n = read(client_fd, lpvMessage + total, map_size - total);
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

typedef int socklen_t;

/* socket_init: nothing to do on Linux; return 0 on success */
int socket_init(void)
{
    return 0;
}

/* BSD-style wrappers mapped to Linux functions */

static int bsd_socket(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

static int bsd_bind(int sockfd, const struct bsd_sockaddr_in *addr, socklen_t addrlen)
{
    if (bind(sockfd, (const struct sockaddr *)addr, addrlen) < 0)
    {
        return -1;
    }
    /* set non-blocking */
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
        return -1;
    return 0;
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
    return setsockopt(sockfd, level, optname, optval, optlen);
}

static int bsd_recv(int sockfd, void *buf, size_t len, int flags)
{
    return recv(sockfd, buf, len, flags);
}

static int bsd_send(int sockfd, const void *buf, size_t len, int flags)
{
    return send(sockfd, buf, len, flags);
}

static int bsd_close(int fd)
{
    return close(fd);
}

socket_device_t wifi_socket = {
    .socket = bsd_socket,
    .bind = bsd_bind,
    .listen = bsd_listen,
    .accept = bsd_accept,
    .recv = bsd_recv,
    .send = bsd_send,
    .close = bsd_close
};

#endif /* ENABLE_SOCKETS */

#ifdef __cplusplus
}
#endif

#endif /* MCU_VIRTUAL_LINUX */
