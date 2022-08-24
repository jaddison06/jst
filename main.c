#include "src/server.h"
#include "src/vector.h"

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

DECL_VEC(int, IntVec)

void print(IntVec vec) {
    for (int i = 0; i < vec.len; i++) {
        printf("%i", vec.root[i]);
        if (i < vec.len - 1) printf(", ");
    }
    printf("\n");
}

int main() {
    // createServer("8888", acceptClient);
    
    // small vectors test
    IntVec vec;
    INIT(vec);

    int i = 1;
    APPEND(vec, i);
    i++;

    print(vec);

    REMOVE(vec, 0);

    print(vec);

    return 0;
}