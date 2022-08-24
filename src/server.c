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

typedef struct {
    AcceptCB acceptClient;
    socket_t newConn;
} ClientThreadArgs;

static void acceptTrampoline(void* args_) {
    ClientThreadArgs* args = args_;
    static int clientID = 0;
    // in theoretical theory this should probs have a mutex. we'll see if it shits ig
    int thisClient = clientID++;
    printf("Got client %i\n", thisClient);
    args->acceptClient(args->newConn, thisClient);
    printf("Client thread %i terminated\n", thisClient);
    free(args);
}

void createServer(char* port, AcceptCB acceptClient) {
    ensureInit();

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

    socket_t client;
    while (1) {
        client = accept(server, NULL, NULL);
        if (client == INVALID_SOCKET) {
            closesocket(server);
            panic("accept failed");
        }
        // On Windows, spawn a new thread for each client (ewwww forking)
        ClientThreadArgs* args = malloc(sizeof(ClientThreadArgs));
        args->acceptClient = acceptClient;
        args->newConn = client;
        _beginthread(acceptTrampoline, 0, args);
    }
    
    // todo: close socket?
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
            printf("Client process %i terminating\n", thisClient);
            exit(0);
        }
    }

}

#endif // JST_UNIX