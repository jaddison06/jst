#pragma once

#include "../platform.h"

# if defined(JST_WIN)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <winsock2.h>
#   include <ws2tcpip.h>
# else if defined(JST_UNIX)
# endif

typedef struct {
#if defined(JST_WIN)
    SOCKET socket;
#else if defined(JST_UNIX)
    int sockfd;
#endif
} Socket;

typedef void(*AcceptCB)(Socket);

void createServer(char* port, AcceptCB acceptClient);
int jst_send(Socket s, char* buf, int len);
int jst_recv(Socket s, char* buf, int len);
int jst_close(Socket s);