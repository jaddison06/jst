#pragma once

#include "platform.h"

# if defined(JST_WIN)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <winsock2.h>
#   include <ws2tcpip.h>
# elif defined(JST_UNIX)
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <stdlib.h>
# endif

typedef struct {
#if defined(JST_WIN)
    SOCKET socket;
#elif defined(JST_UNIX)
    int sockfd;
#endif
} Socket;

typedef void(*AcceptCB)(Socket);

void createServer(char* port, AcceptCB acceptClient);
int jst_send(Socket s, char* buf, int len);
int jst_recv(Socket s, char* buf, int len);
int jst_close(Socket s);