#include "engine.h"

#include <stdlib.h>

#include "panic.h"
#include "threading.h"

Engine* createEngine(char* apiDescFile) {
    Engine* out = malloc(sizeof(Engine));
    INIT(out->requests);
    out->api = loadApiDesc(apiDescFile);
    out->objects = malloc(sizeof(Endpoint) * out->api.objects.len);
    out->actions = malloc(sizeof(Endpoint) * out->api.actions.len);
    for (int i = 0; i < out->api.objects.len; i++) {
        out->objects[i].name = malloc(strlen(out->api.objects.root[i].name) + 1);
        strcpy(out->objects[i].name, out->api.objects.root[i].name);
        out->objects[i].cb = NULL;
    }
    for (int i = 0; i < out->api.actions.len; i++) {
        out->actions[i].name = malloc(strlen(out->api.actions.root[i].name) + 1);
        strcpy(out->actions[i].name, out->api.actions.root[i].name);
        out->actions[i].cb = NULL;
    }
}

void destroyEngine(Engine* engine) {
    for (int i = 0; i < engine->api.objects.len; i++) {
        free(engine->objects[i].name);
    }
    for (int i = 0; i < engine->api.objects.len; i++) {
        free(engine->actions[i].name);
    }
    free(engine->objects);
    free(engine->actions);
    freeApiDesc(engine->api);
    DESTROY(engine->requests);
    free(engine);
}

typedef struct {
    RequestCB cb;
    Engine* engine;
    void* target;
} WorkerThreadArgs;

static void newThreadTrampoline(void* args_) {
    WorkerThreadArgs* args = args_;
    args->cb(args->engine, args->target);
    free(args);
}

void addRequest(Engine* engine, Request request) {
    Endpoint* category;
    int cnt;
    switch (request.type) {
        case ReqObject: {
            category = engine->objects;
            cnt = engine->api.objects.len;
            break;
        }
        case ReqAction: {
            category = engine->actions;
            cnt = engine->api.actions.len;
            break;
        }
    }
    if (request.endpointID >= cnt) panic("No such category %i endpoint %i", request.type, request.endpointID);
    if (category[request.endpointID].cb == NULL) panic("Callback not registered for category %i endpoint %i ('%s')", request.type, request.endpointID, category[request.endpointID].name);
    WorkerThreadArgs* args = malloc(sizeof(WorkerThreadArgs));
    args->cb = category[request.endpointID].cb;
    args->engine = engine;
    args->target = NULL;
    jstCreateThread(newThreadTrampoline, args);
}