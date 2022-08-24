#include "panic.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void panic(char* format, ...) {
    va_list va;
    va_start(va, format);
    vprintf(format, va);
    va_end(va);
    exit(1);
}