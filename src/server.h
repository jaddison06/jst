#pragma once

#include "platform.h"

# if defined(JST_WIN)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <winsock2.h>
#   include <ws2tcpip.h>

typedef SOCKET socket_t;

#define jst_send(socket, buf, len) send(socket, buf, len, 0)
#define jst_recv(socket, buf, len) recv(socket, buf, len, 0)
#define jst_close(socket) closesocket(socket)

# elif defined(JST_UNIX)
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <stdlib.h>
#   include <unistd.h>

typedef int socket_t;

#define jst_send(socket, buf, len) send(socket, buf, len, 0)
#define jst_recv(socket, buf, len) recv(socket, buf, len, 0)
#define jst_close(socket) close(socket)

# endif

typedef void(*AcceptCB)(socket_t, int);

void createServer(char* port, AcceptCB acceptClient);