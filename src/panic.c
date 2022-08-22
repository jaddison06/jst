#include "panic.h"

#include <stdio.h>
#include <stdlib.h>

void panic(char* msg) {
    printf("\033[31m%s\033[0m\n", msg);
    exit(1);
}