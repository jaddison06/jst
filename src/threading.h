#pragma once

#include "platform.h"

# if defined(JST_WIN)
#   include <windows.h>
#   include <process.h>

typedef HANDLE jst_thread_t;
typedef HANDLE jst_mutex_t;

#   define jstCreateThread(cb, args) _beginthread(cb, 0, args)
#   define jstThreadJoin(thread) WaitForSingleObject(thread, INFINITE)
#   define jstCreateMutex() CreateMutex(NULL, FALSE, NULL)
#   define jstAcquire(mutex) WaitForSingleObject(mutex, INFINITE)
#   define jstRelease(mutex) ReleaseMutex(mutex)

# elif defined(JST_UNIX)
#   include <pthread.h>

typedef pthread_t jst_thread_t;
typedef pthread_mutex_t jst_mutex_t;

// pthread_create stores thread ID in a variable, DOESN'T return it. workaround!
static jst_thread_t _createThread(void(*cb)(void*), void* args) {
    jst_thread_t out;
    pthread_create(&out, NULL, cb, args);
    return out;
}

#   define jstCreateThread(cb, args) _createThread(cb, args)
#   define jstThreadJoin(thread) pthread_join(thread, NULL)
#   define jstCreateMutex() PTHREAD_MUTEX_INITIALIZER
#   define jstAcquire(mutex) pthread_mutex_lock(&mutex)
#   define jstRelease(mutex) pthread_mutex_unlock(&mutex)

# endif