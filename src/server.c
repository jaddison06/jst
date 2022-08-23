#include "server.h"

#if defined(JST_WIN)

#include <stdbool.h>
#include <stdlib.h>
#include <process.h>

#include "panic.h"

static bool sockets_initialized = false;

static void shutdown() {
    if (!sockets_initialized) return;

    WSACleanup();

    sockets_initialized = false;
}

static void ensureInit() {
    if (sockets_initialized) return;

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) panic("Failed to initialize WinSock2");

    atexit(shutdown);
    sockets_initialized = true;
}

void createServer(char* port, AcceptCB acceptClient) {
    ensureInit();

    SOCKET server;
    struct addrinfo *result = NULL, hints;

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int result = getaddrinfo(NULL, port, &hints, &result);
    if (result != 0) panic("getaddrinfo failed");

    server = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (server == INVALID_SOCKET) {
        freeaddrinfo(result);
        panic("socket create failed");
    }

    result = bind(server, result->ai_addr, (int)result->ai_addrlen);
    if (result == SOCKET_ERROR) {
        closesocket(server);
        freeaddrinfo(result);
        panic("bind failed");
    }

    freeaddrinfo(result);

    result = listen(server, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        closesocket(server);
        panic("listen failed");
    }

    Socket client;
    while (1) {
        client.socket = accept(server, NULL, NULL);
        if (client.socket == INVALID_SOCKET) {
            closesocket(server);
            panic("accept failed");
        }
        // On Windows, spawn a new thread for each client
        _beginthread(acceptClient, 0, &client);
    }
    
    // todo: close socket?
}

int jst_send(Socket s, char* buf, int len) {
    send(s.socket, buf, len, 0);
}

int jst_recv(Socket s, char* buf, int len) {
    recv(s.socket, buf, len, 0);
}

int jst_close(Socket s) {
    closesocket(s.socket);
}

#elif defined(JST_UNIX)

#include "panic.h"

void createServer(char* port, AcceptCB acceptClient) {
    int sockfd, newsockfd;
    struct sockaddr_in server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) panic("create socket failed");
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(port));
    if (bind(sockfd, (struct sockaddr*) &server, sizeof(server)) < 0) panic("bind failed");
    listen(sockfd, 5);
}

int jst_send(Socket s, char* buf, int len) {
    send(s.sockfd, buf, len, 0);
}

int jst_recv(Socket s, char* buf, int len) {
    recv(s.sockfd, buf, len, 0);
}

int jst_close(Socket s) {
    close(s.sockfd);
}

#endif // JST_UNIX