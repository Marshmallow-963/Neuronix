#ifndef STORAGE_STATE_H
#define STORAGE_STATE_H

#include <stddef.h>

typedef struct {
    const char *const *names;
    const size_t count;
} TableCollection;

typedef struct {
    char notes[256];
    char timestamp[32];

    float totalTime;

    size_t id;
} SessionEntry;

#endif // STORAGE_STATE_H
