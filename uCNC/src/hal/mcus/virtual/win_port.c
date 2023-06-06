#include "../../../cnc.h"
#if (MCU == MCU_VIRTUAL_WIN)

/**
 * GENERIC
 * **/
static void port_init(win_port_t *port)
{
    DWORD rxThreadID;
    DWORD txThreadID;
    DWORD dwWaitResult;

    port->io.tx.bufferMutex = CreateMutex(
        NULL,  // default security attributes
        FALSE, // initially not owned
        NULL); // unnamed mutex

    if (port->io.tx.bufferMutex == NULL)
    {
        printf("CreateMutex error: %lu\n", GetLastError());
        return;
    }

    port->io.tx.txReady = CreateEvent(
        NULL,               // default security attributes
        FALSE,              // manual-reset event
        FALSE,              // initial state is nonsignaled
        TEXT("SocketReady") // object name
    );

    if (port->io.tx.txReady == NULL)
    {
        printf("CreateEvent failed (%lu)\n", GetLastError());
        return;
    }

    port->io.rx.rxReady = CreateEvent(
        NULL,               // default security attributes
        FALSE,              // manual-reset event
        FALSE,              // initial state is nonsignaled
        TEXT("SocketReady") // object name
    );

    if (port->io.rx.rxReady == NULL)
    {
        printf("CreateEvent failed (%lu)\n", GetLastError());
        return;
    }

    port->rxThread = CreateThread(
        NULL,             // default security attributes
        0,                // use default stack size
        port->rxCallback, // thread function name
        &(port->io),      // argument to thread function
        0,                // use default creation flags
        &rxThreadID);     // returns the thread identifier

    if (port->rxThread == NULL)
    {
        printf("CreateThread failed (%lu)\n", GetLastError());
        return;
    }

    dwWaitResult = WaitForSingleObject(
        port->io.rx.rxReady, // event handle
        INFINITE);           // indefinite wait

    switch (dwWaitResult)
    {
    // Event object was signaled
    case WAIT_OBJECT_0:
        printf("Connection started\n");
        port->txThread = CreateThread(
            NULL,             // default security attributes
            0,                // use default stack size
            port->txCallback, // thread function name
            &(port->io),      // argument to thread function
            0,                // use default creation flags
            &txThreadID);     // returns the thread identifier

        if (port->txThread == NULL)
        {
            printf("CreateThread failed (%lu)\n", GetLastError());
            return;
        }
        break;

    // An error occurred
    default:
        printf("Wait error (%lu)\n", GetLastError());
        return;
    }
}

void port_write(win_port_t *port, char *buff, int len)
{
    DWORD dwWaitResult;
    dwWaitResult = WaitForSingleObject(
        port->io.tx.bufferMutex, // handle to mutex
        INFINITE);               // no time-out interval
    port->io.tx.empty = false;
    switch (dwWaitResult)
    {
    // The thread got ownership of the mutex
    case WAIT_OBJECT_0:
        memcpy((char*)port->io.tx.buffer, buff, len);
        ReleaseMutex(port->io.tx.bufferMutex);
        port->io.tx.empty = true;
        break;

    // The thread got ownership of an abandoned mutex
    case WAIT_ABANDONED:
        printf("Wait error (%lu)\n", GetLastError());
        return;
    }

    SetEvent(port->io.tx.txReady);
}

/**
 * CONSOLE
 * **/
DWORD WINAPI consoleserver(LPVOID lpParam)
{
    win_port_io_t *io = lpParam;
    char recvbuf[256];

    memset(recvbuf, 0, 256);
    SetEvent(io->rx.rxReady);
    // Receive until the peer shuts down the connection
    do
    {
        unsigned char c = getchar();
        switch (c)
        {
        default:
            if (io->rx.rxHandler)
            {
                io->rx.rxHandler(c);
            }
            break;
        }

    } while (1);

    return -1;
}

DWORD WINAPI consoleclient(LPVOID lpParam)
{
    win_port_io_t *io = lpParam;
    DWORD dwWaitResult;
    int iResult = 0;

    while (1)
    {
        dwWaitResult = WaitForSingleObject(
            io->tx.txReady, // event handle
            INFINITE);      // indefinite wait

        switch (dwWaitResult)
        {
        // Event object was signaled
        case WAIT_OBJECT_0:
            dwWaitResult = WaitForSingleObject(
                io->tx.bufferMutex, // handle to mutex
                INFINITE);          // no time-out interval

            switch (dwWaitResult)
            {
            // The thread got ownership of the mutex
            case WAIT_OBJECT_0:
                iResult = strlen((char*)io->tx.buffer);
                /*for (int k = 0; k < iResult; k++)
                {
                    putchar(com_buffer[k]);
                }*/
                break;

            // The thread got ownership of an abandoned mutex
            case WAIT_ABANDONED:
                printf("Wait error (%lu)\n", GetLastError());
                return -1;
            }

            break;

        // An error occurred
        default:
            printf("Serial client thread error (%lu)\n", GetLastError());
            return -1;
        }

        memset((char*)io->tx.buffer, 0, 256);
        ReleaseMutex(io->tx.bufferMutex);
    }
    return -1;
}

void console_init(win_port_t *port)
{
    port->rxCallback = &consoleserver;
    port->txCallback = &consoleclient;
    port_init(port);
}

/**
 * SOCKET
 * **/
static DWORD WINAPI socketserver(LPVOID lpParam)
{
    win_port_io_t *io = lpParam;
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[256];
    int recvbuflen = 256;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return -1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, io->portname, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return -1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }

    // Accept a client socket
    io->tx.txHandle = malloc(sizeof(SOCKET));
    (*((SOCKET *)io->tx.txHandle)) = accept(ListenSocket, NULL, NULL);
    if ((*((SOCKET *)io->tx.txHandle)) == INVALID_SOCKET)
    {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }

    // No longer need server socket
    closesocket(ListenSocket);
    memset(recvbuf, 0, sizeof(recvbuf));
    SetEvent(io->rx.rxReady);
    // Receive until the peer shuts down the connection
    do
    {

        iResult = recv((*((SOCKET *)io->tx.txHandle)), recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            recvbuflen = strlen(recvbuf);
            for (int i = 0; i < recvbuflen; i++)
            {
                if (io->rx.rxHandler)
                    io->rx.rxHandler(recvbuf[i]);
            }
            memset(recvbuf, 0, sizeof(recvbuf));
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else
        {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket((*((SOCKET *)io->tx.txHandle)));
            WSACleanup();
            return -1;
        }

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown((*((SOCKET *)io->tx.txHandle)), SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket((*((SOCKET *)io->tx.txHandle)));
        WSACleanup();
        return -1;
    }

    // cleanup
    closesocket((*((SOCKET *)io->tx.txHandle)));
    WSACleanup();

    return -1;
}

static DWORD WINAPI socketclient(LPVOID lpParam)
{
    win_port_io_t *io = lpParam;
    DWORD dwWaitResult;
    char buffer[256];
    int iSendResult;
    int iResult;

    while (1)
    {
        dwWaitResult = WaitForSingleObject(
            io->tx.txReady, // event handle
            INFINITE);      // indefinite wait

        switch (dwWaitResult)
        {
        // Event object was signaled
        case WAIT_OBJECT_0:
            dwWaitResult = WaitForSingleObject(
                io->tx.bufferMutex, // handle to mutex
                INFINITE);          // no time-out interval

            switch (dwWaitResult)
            {
            // The thread got ownership of the mutex
            case WAIT_OBJECT_0:
                iResult = strlen((char*)io->tx.buffer);

                iSendResult = send((*((SOCKET *)io->tx.txHandle)), io->tx.buffer, iResult, 0);
                if (iSendResult == SOCKET_ERROR)
                {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket((*((SOCKET *)io->tx.txHandle)));
                    WSACleanup();
                }
                memset(io->tx.buffer, 0, 256);
                ReleaseMutex(io->tx.bufferMutex);
                break;

            // The thread got ownership of an abandoned mutex
            case WAIT_ABANDONED:
                printf("Wait error (%lu)\n", GetLastError());
                return -1;
            }

            break;

        // An error occurred
        default:
            printf("Socket client thread error (%lu)\n", GetLastError());
            return -1;
        }
    }

    return -1;
}

void socket_init(win_port_t *port)
{
    port->rxCallback = socketserver;
    port->txCallback = socketclient;
    port_init(port);
}

/**
 * UART PORT
 * **/
static DWORD WINAPI uartserver(LPVOID lpParam)
{
    win_port_io_t *io = lpParam;
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {MAXDWORD, 0, 0, 0, 0};
    io->tx.txHandle = NULL;
    DWORD dwRes;
    DWORD dwCommEvent;
    DWORD dwStoredFlags;
    DWORD dwOvRes;
    BOOL fWaitingOnStat = FALSE;
    OVERLAPPED osStatus = {0};

    fprintf(stderr, "Opening serial port %s...", io->portname);
    io->tx.txHandle = CreateFileA(
        io->portname, GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (io->tx.txHandle == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Error\n");
        return 1;
    }
    else
        fprintf(stderr, "OK\n");

    // Set COM port timeout settings
    if (!SetCommTimeouts(io->tx.txHandle, &timeouts))
    {
        fprintf(stderr, "Error setting timeouts\n");
        CloseHandle(io->tx.txHandle);
        return 1;
    }

    PurgeComm(io->tx.txHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    dwStoredFlags = EV_RXCHAR;
    if (!SetCommMask(io->tx.txHandle, dwStoredFlags))
    {
        fprintf(stderr, "Error setting COM mask\n");
        CloseHandle(io->tx.txHandle);
        return 1;
    }

    osStatus.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osStatus.hEvent == NULL)
    {
        fprintf(stderr, "Error setting COM event\n");
        CloseHandle(io->tx.txHandle);
        return 1;
    }

    for (;;)
    {
        // Issue a status event check if one hasn't been issued already.
        if (!fWaitingOnStat)
        {
            if (!WaitCommEvent(io->tx.txHandle, &dwCommEvent, &osStatus))
            {
                if (GetLastError() == ERROR_IO_PENDING)
                    fWaitingOnStat = TRUE;
                // The original text is: bWaitingOnStatusHandle = TRUE;
                else
                    // Error in WaitCommEvent function; abort.
                    break;
            }
        }

        SetEvent(io->rx.rxReady);
        // Check on overlapped operation.
        // if (fWaitingOnStat)
        // {
        // Wait a little while for an event to occur.
        dwRes = WaitForSingleObject(osStatus.hEvent, INFINITE);
        switch (dwRes)
        {
        // Event occurred.
        case WAIT_OBJECT_0:
            if (!GetOverlappedResult(io->tx.txHandle, &osStatus, &dwOvRes, FALSE))
            {
                // An error occurred in the overlapped operation;
                // call GetLastError to find out what it was
                // and abort if it is fatal.
                fprintf(stderr, "Error %lu in COM event", GetLastError());
                break;
            }
            else
            {
                // Status event is stored in the event flag
                // specified in the original WaitCommEvent call.
                // Deal with the status event as appropriate.
                char recvbuf[256];
                int recvbuflen = 256;
                DWORD dwRead = 0;
                switch (dwCommEvent)
                {
                case EV_RXCHAR:
                    do
                    {
                        if (ReadFile(io->tx.txHandle, &recvbuf, recvbuflen, &dwRead, &osStatus))
                        {
                            // Read a byte and process it.
                            recvbuflen = strlen(recvbuf);
                            recvbuflen = (dwRead > recvbuflen) ? recvbuflen : dwRead;
                            for (int i = 0; i < recvbuflen; i++)
                            {
                                putchar(recvbuf[i]);
                                if (io->rx.rxHandler)
                                {
                                    io->rx.rxHandler(recvbuf[i]);
                                }
                            }
                            memset(recvbuf, 0, sizeof(recvbuf));
                        }
                        else
                        {
                            // An error occurred when calling the ReadFile function.
                            fprintf(stderr, "Error %lu reading COM", GetLastError());
                            break;
                        }

                    } while (dwRead);
                    break;
                default:
                    break;
                }
            }

            // Set fWaitingOnStat flag to indicate that a new
            // WaitCommEvent is to be issued.
            fWaitingOnStat = FALSE;
            break;
        }
        // }
    }

    CloseHandle(osStatus.hEvent);
}

static DWORD WINAPI uartclient(LPVOID lpParam)
{
    win_port_io_t *io = lpParam;
    DWORD dwWaitResult;
    OVERLAPPED osWrite = {0};
    char buffer[256];
    int iSendResult;
    int iResult;

    DWORD dNoOfBytesWritten;

    while (1)
    {
        dwWaitResult = WaitForSingleObject(
            io->tx.txReady, // event handle
            INFINITE);      // indefinite wait
        osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        switch (dwWaitResult)
        {
        // Event object was signaled
        case WAIT_OBJECT_0:
            dwWaitResult = WaitForSingleObject(
                io->tx.bufferMutex, // handle to mutex
                INFINITE);          // no time-out interval

            switch (dwWaitResult)
            {
            // The thread got ownership of the mutex
            case WAIT_OBJECT_0:
                iResult = strlen(io->tx.buffer);
                if (!WriteFile(io->tx.txHandle, io->tx.buffer, iResult, &dNoOfBytesWritten, &osWrite))
                {
                    if (GetLastError() != ERROR_IO_PENDING)
                    {
                        // WriteFile failed, but isn't delayed. Report error and abort.
                        fprintf(stderr, "Error %lu in Writing to Serial Port", GetLastError());
                    }
                }
                break;

            // The thread got ownership of an abandoned mutex
            case WAIT_ABANDONED:
                printf("Wait error (%lu)\n", GetLastError());
                return;
            }

            break;

        // An error occurred
        default:
            printf("Serial client thread error (%lu)\n", GetLastError());
            return;
        }

        CloseHandle(osWrite.hEvent);
        memset(io->tx.buffer, 0, 256);
        ReleaseMutex(io->tx.bufferMutex);
    }
}

void uart_init(win_port_t *port)
{
    port->rxCallback = uartserver;
    port->txCallback = uartclient;
    port_init(port);
}
#endif
