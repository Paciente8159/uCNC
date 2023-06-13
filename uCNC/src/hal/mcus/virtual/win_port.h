#ifndef WIN_PORT_H
#define WIN_PORT_H

typedef unsigned long(__attribute__((__stdcall__)) * port_callback)(void *);

typedef struct win_port_rx_
{
    HANDLE rxReady;
    void (*rxHandler)(unsigned char);
} win_port_rx_t;

typedef struct win_port_tx_
{
    int len;
    char buffer[256];
    void *bufferMutex;
    void *txReady;
    void (*txHandler)(void);
    void* txHandle;
} win_port_tx_t;

typedef struct win_port_io_
{
    char portname[32];
    bool isclient;
    volatile struct win_port_rx_ rx;
    volatile struct win_port_tx_ tx;
} win_port_io_t;

typedef struct win_port_
{
    void *rxThread;
    port_callback rxCallback;
    void *txThread;
    port_callback txCallback;
    win_port_io_t io;
} win_port_t;

void console_init(win_port_t *port);
void socket_init(win_port_t *port);
void uart_init(win_port_t *port);

void port_write(win_port_t *port, char *buff, int len);

#endif