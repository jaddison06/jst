#pragma once

#include <stddef.h>

#include "vector.h"
#include "ApiDesc.h"

typedef enum {
    ReqObject,
    ReqAction
} RequestType;

typedef struct {
    RequestType type;
    int endpointID;
} Request;

DECL_VEC(Request, RequestList)

struct Engine;

typedef void(*RequestCB)(struct Engine*, void*);

typedef struct {
    char* name;
    RequestCB cb;
} Endpoint;

typedef struct Engine {
    ApiDesc api;
    RequestList requests;
    Endpoint* objects;
    Endpoint* actions;
} Engine;

Engine* createEngine(char* apiDescFile);
void destroyEngine(Engine* engine);

// Called by network thread to get the ball rolling
void addRequest(Engine* engine, Request request);

// Called by request worker threads

void markDone(Engine* engine, void* target);
void markPartial(Engine* engine, void* target, size_t done);


// okokokokokokokokokokokokokokok
// types are either primitives or user-defined (list? map?)
// can complete or semi-complete primitives: prim request or compound members
// completer methods act on ADDRESSES - engine's responsibility to keep track of object mappings
// CODEGEN - workers get STRUCTS

// ok some sample request worker callbacks

// callbacks get a POINTER to the target (a primitive OR a struct) + a pointer to the engine
// it's *their* responsibility to allocate memory etc where required (todo: this is fucking stupid)

/*
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
*/