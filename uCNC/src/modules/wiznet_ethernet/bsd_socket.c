#include "socket.h"
#include "../net/utils/bsd_socket.h"

// Simple FD table mapping BSD-like fds to Wiznet socket numbers and state
typedef struct
{
    uint8_t in_use;      // 0=free, 1=allocated
    int8_t sn;           // Wiznet socket number
    uint8_t protocol;    // Sn_MR_TCP or Sn_MR_UDP
    uint16_t local_port; // desired local port for bind()
    uint8_t flags;       // SF_IO_NONBLOCK (etc.) for socket open
} fd_entry;

#ifndef MAX_BSD_FD
#define MAX_BSD_FD _WIZCHIP_SOCK_NUM_
#endif

static fd_entry fdtab[MAX_BSD_FD];

// Helpers
static int alloc_fd(uint8_t sn, uint8_t protocol, uint16_t lport, uint8_t flags)
{
    uint8_t lowest = 0;
    for (int i = 0; i < MAX_BSD_FD; i++)
    {
        if (fdtab[i].in_use && fdtab[i].sn == lowest)
        {
            lowest++;
            i = 0;
        }
    }

    for (int i = 0; i < MAX_BSD_FD; i++)
    {
        if (!fdtab[i].in_use)
        {
            fdtab[i].in_use = 1;
            fdtab[i].sn = lowest;
            fdtab[i].protocol = protocol;
            fdtab[i].local_port = lport;
            fdtab[i].flags = flags;
            return i;
        }
    }
    return -1;
}

static void free_fd(int sockfd)
{
    if (sockfd < 0 || sockfd >= MAX_BSD_FD)
        return -1;
    fdtab[sockfd].in_use = 0;
    fdtab[sockfd].sn = -1;
    fdtab[sockfd].protocol = 0;
    fdtab[sockfd].local_port = 0;
    fdtab[sockfd].flags = 0;
}

static int get_sn(uint8_t sockfd)
{
    if (sockfd < 0 || sockfd >= MAX_BSD_FD || !fdtab[sockfd].in_use)
        return -1;
    return fdtab[sockfd].sn;
}

// Map BSD domain/type to Wiznet protocol
static int map_proto(int domain, int type, int protocol, uint8_t *out)
{
    if (domain != AF_INET)
        return -1;
    (void)protocol; // Wiznet ignores protocol for TCP/UDP
    if (type == SOCK_STREAM)
    {
        *out = Sn_MR_TCP;
        return 0;
    }
    else if (type == SOCK_DGRAM)
    {
        *out = Sn_MR_UDP;
        return 0;
    }
    else if (type == SOCK_RAW)
    {
        *out = Sn_MR_IPRAW;
        return 0;
    }
    return -1;
}

// BSD: socket(domain, type, protocol)
int bsd_socket(int domain, int type, int protocol)
{
    uint8_t wiz_proto;
    if (map_proto(domain, type, protocol, &wiz_proto) != 0)
        return -1;

    // Find a free Wiznet socket number
    for (uint8_t sn = 0; sn < _WIZCHIP_SOCK_NUM_; sn++)
    {
        // allocate a free socket
        int fd = alloc_fd(0, wiz_proto, 0, 0);
        if (fd < 0)
        {
            close(sn);
            return -1;
        }
        return fd;
    }
    return -1;
}

// BSD: bind(sockfd, addr, addrlen)
int bsd_bind(int sockfd, const struct bsd_sockaddr_in *addr, int addrlen)
{
    (void)addrlen;
    if (sockfd < 0 || sockfd >= MAX_BSD_FD)
        return -1;

    if (!addr)
        return -1;

    // Store desired local port for later use when (re)opening the socket
    uint16_t host_port = addr->sin_port; // already in network order in BSD; our header uses generic struct
    // Convert to host order as Wiznet expects raw value; ioLibrary socket() uses host order port
    uint16_t port = (uint16_t)(((host_port & 0x00FFu) << 8) | ((host_port & 0xFF00u) >> 8));

    // update the socket number and port
    fdtab[sockfd].local_port = port;
    return 0;
}

// BSD: listen(sockfd, backlog)
int bsd_listen(int sockfd, int backlog)
{
    (void)backlog; // Wiznet handles single connection per socket
    if (sockfd < 0 || sockfd >= MAX_BSD_FD)
        return -1;

    int sn = socket(0, fdtab[sockfd].protocol, fdtab[sockfd].local_port, fdtab[sockfd].flags);
    if (sn < 0)
    {
        bsd_close(sockfd);
        return -1;
    }

    fdtab[sockfd].sn = sn;

    // Must be TCP
    if (fdtab[sockfd].protocol != Sn_MR_TCP)
        return -1;
    if (listen((uint8_t)sn) != SOCK_OK)
        return -1;
    return 0;
}

// BSD: accept(sockfd, addr, addrlen)
int bsd_accept(int sockfd, struct bsd_sockaddr_in *addr, int *addrlen)
{
    int sn = get_sn(sockfd);
    if (sn < 0)
        return -1;

    // Wiznet doesn't create a new socket; the same socket transitions to ESTABLISHED
    // Block until established or non-blocking mode returns -1 for "would block"
    while (1)
    {
        uint8_t sr = getSn_SR((uint8_t)sn);
        if (sr == SOCK_ESTABLISHED)
            break;
        if (sr == SOCK_CLOSED)
            return -1;

        // Check non-blocking mode
        uint8_t iomode = 0;
        if (ctlsocket((uint8_t)sn, CS_GET_IOMODE, &iomode) == SOCK_OK && (iomode & 0x01))
        {
            // Non-blocking: no connection yet
            return -1;
        }
    }

    uint16_t dport = 0;
    uint8_t dip[4] = {0};
    getsockopt((uint8_t)sn, SO_DESTPORT, &dport);
    getsockopt((uint8_t)sn, SO_DESTIP, dip);

    // Fill peer address if requested
    if (addr && addrlen && *addrlen >= (int)sizeof(struct bsd_sockaddr_in))
    {
        // Get peer IP and port via getsockopt
        addr->sin_family = AF_INET;
        addr->sin_port = bsd_htons(dport);
        addr->sin_addr = ((uint32_t)dip[0] << 24) | ((uint32_t)dip[1] << 16) |
                         ((uint32_t)dip[2] << 8) | ((uint32_t)dip[3]);
        for (int i = 0; i < 8; i++)
            addr->sin_zero[i] = 0;
        *addrlen = sizeof(struct bsd_sockaddr_in);
    }

    int clientfd = alloc_fd(fdtab[sockfd].sn, fdtab[sockfd].protocol, fdtab[sockfd].local_port, fdtab[sockfd].flags);

    if (clientfd < 0)
    {
        // cannot allocate more sockets
        // close connection with current client
        close(fdtab[sockfd].sn);
    }

    // swap sockets
    sn = fdtab[clientfd].sn;
    fdtab[clientfd].sn = fdtab[sockfd].sn;

    // use the clientfd as the new socket index to listen
    sn = socket(sn, fdtab[sockfd].protocol, fdtab[sockfd].local_port, fdtab[sockfd].flags);
    if (sn < 0)
    {
        bsd_close(sockfd);
        return -1;
    }

    if (listen(sn) != SOCK_OK)
    {
        bsd_close(sn);
        return -1;
    }

    // update the listening socket number
    fdtab[sockfd].sn = (uint8_t)sn;

    if (clientfd < 0)
    {
        return -1;
    }

    // return the cliend socket
    return clientfd;
}

// BSD: fcntl(fd, cmd, arg) — support O_NONBLOCK
int bsd_fcntl(int sockfd, int cmd, long arg)
{
    if (sockfd < 0 || sockfd >= MAX_BSD_FD)
        return -1;

    if (cmd == F_SETFL)
    {
        uint8_t mode = (arg & O_NONBLOCK) ? SOCK_IO_NONBLOCK : SOCK_IO_BLOCK;
        fdtab[sockfd].flags = mode;
        return 0;
    }
    return -1;
}

// BSD: recv — flags ignored for now
int bsd_recv(int sockfd, void *buf, size_t len, int flags)
{
    (void)flags;
    int sn = get_sn(sockfd);
    if (sn < 0 || !buf)
        return -1;

    if (fdtab[sockfd].protocol == Sn_MR_TCP)
    {
        int32_t r = recv((uint8_t)sn, (uint8_t *)buf, len);
        if (r == SOCK_BUSY) // on non block mode this will return SOCK_BUSY if not data available (idle)
            return -1;

        if (r < 0)
            return 0;
        return (int)r;
    }
    else if (fdtab[sockfd].protocol == Sn_MR_UDP)
    {
        // uint8_t *rp = (uint8_t *)&rport;
        int32_t r = recv((uint8_t)sn, (uint8_t *)buf, (uint16_t)len);
        if (r < 0)
            return -1;
        return (int)r;
    }
    else
    {
        return -1;
    }
}

// BSD: send — flags ignored for now
int bsd_send(int sockfd, const void *buf, size_t len, int flags)
{
    (void)flags;
    int sn = get_sn(sockfd);
    if (sn < 0 || !buf)
        return -1;

    if (fdtab[sockfd].protocol == Sn_MR_TCP)
    {
        int32_t s = send((uint8_t)sn, (uint8_t *)buf, (uint16_t)len);
        if (s < 0)
            return -1;
        return (int)s;
    }
    else
    {
        // UDP requires destination; BSD send() uses connected UDP sockets.
        // If needed, you can call setsockopt(SO_DESTIP/SO_DESTPORT) prior to send().
        uint16_t dport = 0;
        uint8_t dip[4] = {0};
        getsockopt((uint8_t)sn, SO_DESTPORT, &dport);
        getsockopt((uint8_t)sn, SO_DESTIP, dip);
        if (dip[0] == 0 && dip[1] == 0 && dip[2] == 0 && dip[3] == 0)
            return -1;

        int32_t s = send((uint8_t)sn, (uint8_t *)buf, (uint16_t)len);
        if (s < 0)
            return -1;
        return (int)s;
    }
}

// BSD: close
int bsd_close(int sockfd)
{
    if (sockfd < 0)
        return -1;
    int sn = fdtab[sockfd].sn;
    free_fd(sockfd);
    if (sn < 0)
        return -1;
    int8_t r = close((uint8_t)sn);
    return (r == SOCK_OK) ? 0 : -1;
}
