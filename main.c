#include "src/server.h"

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

int main() {
    createServer("8888", acceptClient);
    return 0;
}