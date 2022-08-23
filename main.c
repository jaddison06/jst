#include "src/server.h"

#include <stdio.h>

void acceptClient(Socket s) {
    char buf[100];
    while (1) {
        int received = jst_recv(s, buf, 100);
        buf[received] = '\0';
        printf("%s\n", buf);
    }
}

int main() {
    createServer("8888", acceptClient);
    return 0;
}