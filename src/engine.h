#pragma once

typedef struct {

} Engine;

typedef struct {

} Request;

// Called by network thread to get the ball rolling
void addRequest(Engine* engine);

// Called by request worker threads

// okokokokokokokokokokokokokokok
// types are either primitives or user-defined (list? map?)
// can complete or semi-complete primitives: prim request or compound members
// completer methods act on ADDRESSES - engine's responsibility to keep track of object mappings
// CODEGEN - workers get STRUCTS

// ok some sample request worker callbacks

// callbacks get a POINTER to the target (a primitive OR a struct) + a pointer to the engine
// it's *their* responsibility to allocate memory etc where required (todo: this is fucking stupid)

void cbSayHello(Engine* engine, char** target) {
    *target = malloc(14);
    strcpy(target, "Hello, World!");
    markDone(engine, target);
}

void cbIncrementalHello(Engine* engine, char** target) {
    *target = NULL;
    char* hello = "Hello, World!";
    for (int i = 1; i < 15; i++) {
        // stupid example but you get the point - the engine shouldn't care about the contents of the pointer
        *target = realloc(*target, i);
        
        memcpy(target, hello, i);
        markPartial(engine, target, i);
    }
    markDone(engine, target);
}