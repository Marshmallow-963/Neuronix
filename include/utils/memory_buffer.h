#ifndef MEMORY_UTILS_H
#define MEMORY_UTILS_H

#include <stddef.h>
#include <stdbool.h>

bool ArrayResize(void **ptr, size_t count, size_t elementSize);

size_t MemoryUsageGet(void);

void BufferEnsure(void **buf, bool resize, size_t newCap, size_t elementSize);

void BufferEnsureBackfill(
    void **buf,
    bool resize,
    size_t newCap,
    size_t count,
    double startX,
    double stepX,
    double fillY
);

void BufferFree(void **ptr);

#endif // MEMORY_UTILS_H
