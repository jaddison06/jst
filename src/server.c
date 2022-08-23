#include "server.h"

#if defined(JST_WIN)

#include <stdbool.h>
#include <stdlib.h>
#include <process.h>

#include "panic.h"

static bool sockets_initialized = false;

static void jst_shutdown() {
    if (!sockets_initialized) return;

    WSACleanup();

    sockets_initialized = false;
}

static void ensureInit() {
    if (sockets_initialized) return;

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) panic("Failed to initialize WinSock2");

    atexit(jst_shutdown);
    sockets_initialized = true;
}

// I'm using _beginthread cos it's quick and easy, and means I don't have to deal
// with Win32 weirdness (why do you want me to allocate my own heap???????)
// However, it's v picky about the callback - it needs to have a single void* parameter,
// and the ArgList doesn't play very nicely, nor is it well documented. My solution,
// inelegant as it is, is to just move the callback out to a global and then start the
// thread with a trampoline function. If you want to have a crack at making some sense
// of this, see https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/beginthread-beginthreadex
static AcceptCB acceptClient;
static Socket newConn;
static void acceptTrampoline(void* unused) {
    static int clientID = 0;
    // in theoretical theory this should probs have a mutex. we'll see if it shits ig
    int thisClient = clientID++;
    printf("Got client %i\n", thisClient);
    acceptClient(newConn, thisClient);
    printf("Client thread %i terminated\n", thisClient);
}

void createServer(char* port, AcceptCB acceptClient_) {
    ensureInit();

    acceptClient = acceptClient_;

    SOCKET server;
    struct addrinfo *ai = NULL, hints;

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int result = getaddrinfo(NULL, port, &hints, &ai);
    if (result != 0) panic("getaddrinfo failed");

    server = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (server == INVALID_SOCKET) {
        freeaddrinfo(ai);
        panic("socket create failed");
    }

    result = bind(server, ai->ai_addr, (int)ai->ai_addrlen);
    if (result == SOCKET_ERROR) {
        closesocket(server);
        freeaddrinfo(ai);
        panic("bind failed");
    }

    freeaddrinfo(ai);

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
        // On Windows, spawn a new thread for each client (ewwww forking)
        newConn = client;
        _beginthread(acceptTrampoline, 0, NULL);
    }
    
    // todo: close socket?
}

int jst_send(Socket s, char* buf, int len) {
    return send(s.socket, buf, len, 0);
}

int jst_recv(Socket s, char* buf, int len) {
    return recv(s.socket, buf, len, 0);
}

int jst_close(Socket s) {
    return closesocket(s.socket);
}

#elif defined(JST_UNIX)

#include "panic.h"

void createServer(char* port, AcceptCB acceptClient) {
    static int clientID;

    int sockfd;
    struct sockaddr_in server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) panic("create socket failed");
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(port));
    if (bind(sockfd, (struct sockaddr*) &server, sizeof(server)) < 0) panic("bind failed");
    if (listen(sockfd, 10) < 0) panic("listen failed");
    Socket client_sock;
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    while (1) {
        client_sock.sockfd = accept(sockfd, (struct sockaddr*)&client, &client_len);
        // On Unix, each client gets its own process - the client handler can then play with its own sanboxed threads
        // Pretty sure this doesn't need a mutex - it's happening synchronously, before the fork
        int thisClient = clientID++;
        printf("Got client %i\n", thisClient);
        pid_t pid = fork();
        if (pid == 0) {
            // child process
            acceptClient(client_sock, thisClient);
            printf("Client process %i terminating", thisClient);
            exit(0);
        }
    }

}

int jst_send(Socket s, char* buf, int len) {
    return send(s.sockfd, buf, len, 0);
}

int jst_recv(Socket s, char* buf, int len) {
    return recv(s.sockfd, buf, len, 0);
}

int jst_close(Socket s) {
    return close(s.sockfd);
}

#endif // JST_UNIX