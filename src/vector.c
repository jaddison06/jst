#include "vector.h"

void _append(void** vec, void* item, size_t size, int* currentLength, int* currentCapacity) {
    if (*currentLength == *currentCapacity) {
        *vec = realloc(*vec, *currentCapacity * size * 2);
        *currentCapacity *= 2;
    }
    memcpy(&((*(char**)vec)[(*currentLength) * size]), item, size);
    *currentLength += 1;
}