#include "src/server.h"
#include "src/threading.h"

#include <stdio.h>

void acceptClient(Socket s, int id) {
    char buf[100];
    while (1) {
        int received = jst_recv(s, buf, 100);
        if (received == 0) break;
        if (received != 100) buf[received] = '\0';
        printf("Client %i: (%i) %s\n", id, received, buf);
    }
    printf("Closed %i\n", id);
}

void threadCB(void* unused) {
    printf("Thread created!!\n");
}

int main() {
    // createServer("8888", acceptClient);
    
    // small threading test
    jst_thread_t threads[10];
    for (int i = 0; i < 10; i++) {
        threads[i] = jstCreateThread(threadCB, NULL);
    }
    for (int i = 0; i < 10; i++) {
        jstThreadJoin(threads[i]);
    }
    return 0;
}