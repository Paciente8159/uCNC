// // #include "socket.h"
// // #include "../net/utils/bsd_socket.h"

// // int bsd_socket(int domain, int type, int protocol)
// // {
// //     int32_t ret;
// //     int i;
// //     int internal_protocol;

// //     if (protocol == IPPROTO_TCP)
// //         internal_protocol = Sn_MR_TCP;
// //     else if (protocol == IPPROTO_UDP)
// //         internal_protocol = Sn_MR_UDP;
// //     else
// //         internal_protocol = Sn_MR_TCP;

// //     for (i = 0; i < (_WIZCHIP_SOCK_NUM_ + 1); i++)
// //     {
// //         if ((getSn_RX_RSR(i) == SOCK_CLOSED) && (i < _WIZCHIP_SOCK_NUM_))
// //         {
// //             break;
// //         }
// //     }

// //     if (i >= _WIZCHIP_SOCK_NUM_)
// //         return -1;

// //     ret = socket(i, internal_protocol, (TEST_PORT + i), 0x00);
// //     // printf("\r\nSocket:%d Port:%d\r\n", i, (TEST_PORT + i));

// //     return ret;
// // }

// // int bsd_accept(int s, struct bsd_sockaddr_in *addr, int *addrlen)
// // {
// //     return 0;
// // }

// // int bsd_bind(int s, const struct bsd_sockaddr_in *name, int namelen)
// // {
// //     return 0;
// // }

// // int bsd_close(int s)
// // {
// //     int32_t ret;

// //     ret = close(s);

// //     return ret;
// // }

// // int bsd_listen(int s, int backlog)
// // {
// //     int32_t ret;

// //     ret = listen(s);

// //     return ret;
// // }

// // int bsd_recv(int s, void *mem, size_t len, int flags)
// // {
// //     int32_t ret;

// //     ret = recv(s, mem, len);

// //     return ret;
// // }

// // int bsd_send(int s, const void *dataptr, size_t size, int flags)
// // {
// //     int32_t ret;

// //     ret = send(s, (uint8_t *)dataptr, size);

// //     return ret;
// // }

// // int bsd_fcntl(int fd, int cmd, long arg)
// // {
// //     (void)fd;
// //     (void)cmd;
// //     (void)arg;
// //     return 0;
// // }

// #include "socket.h"
// #include "../net/utils/bsd_socket.h"

// // Simple FD table mapping BSD-like fds to Wiznet socket numbers and state
// typedef struct
// {
//     uint8_t in_use;      // 0=free, 1=allocated
//     uint8_t sn;          // Wiznet socket number
//     uint8_t protocol;    // Sn_MR_TCP or Sn_MR_UDP
//     uint16_t local_port; // desired local port for bind()
//     uint8_t flags;       // SF_IO_NONBLOCK (etc.) for socket open
// } fd_entry;

// #ifndef MAX_BSD_FD
// #define MAX_BSD_FD _WIZCHIP_SOCK_NUM_
// #endif

// static fd_entry fdtab[MAX_BSD_FD];

// // Helpers
// static int alloc_fd(uint8_t sn, uint8_t protocol, uint16_t lport, uint8_t flags)
// {
//     for (int i = 0; i < MAX_BSD_FD; i++)
//     {
//         if (!fdtab[i].in_use)
//         {
//             fdtab[i].in_use = 1;
//             fdtab[i].sn = sn;
//             fdtab[i].protocol = protocol;
//             fdtab[i].local_port = lport;
//             fdtab[i].flags = flags;
//             return i;
//         }
//     }
//     return -1;
// }

// static int get_sn(int fd)
// {
//     if (fd < 0 || fd >= MAX_BSD_FD || !fdtab[fd].in_use)
//         return -1;
//     return fdtab[fd].sn;
// }

// static void set_sn(int fd, int sn)
// {
//     if (fd < 0 || fd >= MAX_BSD_FD || !fdtab[fd].in_use)
//         return -1;
//     fdtab[fd].sn = sn;
// }

// // Map BSD domain/type to Wiznet protocol
// static int map_proto(int domain, int type, int protocol, uint8_t *out)
// {
//     if (domain != AF_INET)
//         return -1;
//     (void)protocol; // Wiznet ignores protocol for TCP/UDP
//     if (type == SOCK_STREAM)
//     {
//         *out = Sn_MR_TCP;
//         return 0;
//     }
//     else if (type == SOCK_DGRAM)
//     {
//         *out = Sn_MR_UDP;
//         return 0;
//     }
//     else if (type == SOCK_RAW)
//     {
//         *out = Sn_MR_IPRAW;
//         return 0;
//     }
//     return -1;
// }

// // BSD: socket(domain, type, protocol)
// int bsd_socket(int domain, int type, int protocol)
// {
//     uint8_t wiz_proto;
//     if (map_proto(domain, type, protocol, &wiz_proto) != 0)
//         return -1;

//     // Find a free Wiznet socket number
//     for (uint8_t sn = 0; sn < _WIZCHIP_SOCK_NUM_; sn++)
//     {
//         // Try to open with no port yet; we'll set port on bind/listen/connect
//         // Wiznet socket() requires a port; use 0 to auto-assign ephemeral
//         int8_t res = socket(sn, wiz_proto, 0, 0);
//         if (res >= 0)
//         {
//             // force nonblock mode
//             uint8_t mode = SOCK_IO_NONBLOCK;
//             if (ctlsocket((uint8_t)sn, CS_SET_IOMODE, &mode) != SOCK_OK)
//                 return -1;
//             int fd = alloc_fd(sn, wiz_proto, 0, 0);
//             if (fd < 0)
//             {
//                 close(sn);
//                 return -1;
//             }
//             return fd;
//         }
//     }
//     return -1;
// }

// // BSD: bind(sockfd, addr, addrlen)
// int bsd_bind(int sockfd, const struct bsd_sockaddr_in *addr, int addrlen)
// {
//     (void)addrlen;
//     int sn = get_sn(sockfd);
//     if (sn < 0 || !addr)
//         return -1;

//     // Store desired local port for later use when (re)opening the socket
//     uint16_t host_port = addr->sin_port; // already in network order in BSD; our header uses generic struct
//     // Convert to host order as Wiznet expects raw value; ioLibrary socket() uses host order port
//     uint16_t port = (uint16_t)(((host_port & 0x00FFu) << 8) | ((host_port & 0xFF00u) >> 8));

//     // get previous modes
//     uint8_t mode = 0;
//     if (ctlsocket((uint8_t)sn, CS_GET_IOMODE, &mode) != SOCK_OK)
//         return -1;

//     // Re-open the socket with the desired local port (close then open)
//     close((uint8_t)sn);
//     int8_t res = socket((uint8_t)sn, fdtab[sockfd].protocol, port, mode);
//     if (res < 0)
//         return -1;

//     // update the socket number and port
//     fdtab[sockfd].local_port = port;
//     set_sn(sockfd, res);
//     return 0;
// }

// // BSD: listen(sockfd, backlog)
// int bsd_listen(int sockfd, int backlog)
// {
//     (void)backlog; // Wiznet handles single connection per socket
//     int sn = get_sn(sockfd);
//     if (sn < 0)
//         return -1;

//     // Ensure socket is opened with a port (bind may have set it)
//     if (fdtab[sockfd].local_port != 0)
//     {
//         uint8_t mode = 0;
//         if (ctlsocket((uint8_t)sn, CS_GET_IOMODE, &mode) != SOCK_OK)
//             return -1;

//         // Re-open to guarantee port is set on Wiznet side
//         close((uint8_t)sn);
//         int8_t so = socket((uint8_t)sn, fdtab[sockfd].protocol, fdtab[sockfd].local_port, mode);
//         if (so < 0)
//             return -1;
//         set_sn(sockfd, so);
//         sn = so;
//     }

//     // Must be TCP
//     if (fdtab[sockfd].protocol != Sn_MR_TCP)
//         return -1;
//     if (listen((uint8_t)sn) != SOCK_OK)
//         return -1;
//     return 0;
// }

// // BSD: accept(sockfd, addr, addrlen)
// int bsd_accept(int sockfd, struct bsd_sockaddr_in *addr, int *addrlen)
// {
//     int sn = get_sn(sockfd);
//     if (sn < 0)
//         return -1;

//     // Wiznet doesn't create a new socket; the same socket transitions to ESTABLISHED
//     // Block until established or non-blocking mode returns -1 for "would block"
//     while (1)
//     {
//         uint8_t sr = getSn_SR((uint8_t)sn);
//         if (sr == SOCK_ESTABLISHED)
//             break;
//         if (sr == SOCK_CLOSED)
//             return -1;

//         // Check non-blocking mode
//         uint8_t iomode = 0;
//         if (ctlsocket((uint8_t)sn, CS_GET_IOMODE, &iomode) == SOCK_OK && (iomode & 0x01))
//         {
//             // Non-blocking: no connection yet
//             return -1;
//         }
//     }

//     // Fill peer address if requested
//     if (addr && addrlen && *addrlen >= (int)sizeof(struct bsd_sockaddr_in))
//     {
//         // Get peer IP and port via getsockopt
//         uint16_t dport = 0;
//         uint8_t dip[4] = {0};
//         getsockopt((uint8_t)sn, SO_DESTPORT, &dport);
//         getsockopt((uint8_t)sn, SO_DESTIP, dip);

//         addr->sin_family = AF_INET;
//         addr->sin_port = bsd_htons(dport);
//         addr->sin_addr = ((uint32_t)dip[0] << 24) | ((uint32_t)dip[1] << 16) |
//                          ((uint32_t)dip[2] << 8) | ((uint32_t)dip[3]);
//         for (int i = 0; i < 8; i++)
//             addr->sin_zero[i] = 0;
//         *addrlen = sizeof(struct bsd_sockaddr_in);
//     }

//     // Return same fd (Wiznet supports one client per socket)
//     return sockfd;
// }

// // BSD: fcntl(fd, cmd, arg) — support O_NONBLOCK
// int bsd_fcntl(int fd, int cmd, long arg)
// {
//     int sn = get_sn(fd);
//     if (sn < 0)
//         return -1;
//     if (cmd == F_SETFL)
//     {
//         uint8_t mode = (arg & O_NONBLOCK) ? SOCK_IO_NONBLOCK : SOCK_IO_BLOCK;
//         if (ctlsocket((uint8_t)sn, CS_SET_IOMODE, &mode) != SOCK_OK)
//             return -1;
//         return 0;
//     }
//     return -1;
// }

// // BSD: recv — flags ignored for now
// int bsd_recv(int sockfd, void *buf, size_t len, int flags)
// {
//     (void)flags;
//     int sn = get_sn(sockfd);
//     if (sn < 0 || !buf)
//         return -1;

//     if (fdtab[sockfd].protocol == Sn_MR_TCP)
//     {
//         int32_t r = recv((uint8_t)sn, (uint8_t *)buf, (uint16_t)len);
//         if (r < 0)
//             return -1;
//         return (int)r;
//     }
//     else if (fdtab[sockfd].protocol == Sn_MR_UDP)
//     {
//         // uint8_t *rp = (uint8_t *)&rport;
//         int32_t r = recv((uint8_t)sn, (uint8_t *)buf, (uint16_t)len);
//         if (r < 0)
//             return -1;
//         return (int)r;
//     }
//     else
//     {
//         return -1;
//     }
// }

// // BSD: send — flags ignored for now
// int bsd_send(int sockfd, const void *buf, size_t len, int flags)
// {
//     (void)flags;
//     int sn = get_sn(sockfd);
//     if (sn < 0 || !buf)
//         return -1;

//     if (fdtab[sockfd].protocol == Sn_MR_TCP)
//     {
//         int32_t s = send((uint8_t)sn, (uint8_t *)buf, (uint16_t)len);
//         if (s < 0)
//             return -1;
//         return (int)s;
//     }
//     else
//     {
//         // UDP requires destination; BSD send() uses connected UDP sockets.
//         // If needed, you can call setsockopt(SO_DESTIP/SO_DESTPORT) prior to send().
//         uint16_t dport = 0;
//         uint8_t dip[4] = {0};
//         getsockopt((uint8_t)sn, SO_DESTPORT, &dport);
//         getsockopt((uint8_t)sn, SO_DESTIP, dip);
//         if (dip[0] == 0 && dip[1] == 0 && dip[2] == 0 && dip[3] == 0)
//             return -1;

//         int32_t s = send((uint8_t)sn, (uint8_t *)buf, (uint16_t)len);
//         if (s < 0)
//             return -1;
//         return (int)s;
//     }
// }

// // BSD: close
// int bsd_close(int fd)
// {
//     int sn = get_sn(fd);
//     if (sn < 0)
//         return -1;
//     int8_t r = close((uint8_t)sn);
//     fdtab[fd].in_use = 0;
//     return (r == SOCK_OK) ? 0 : -1;
// }


