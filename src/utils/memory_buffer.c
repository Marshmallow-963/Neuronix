#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>

#include "utils/struct_data.h"
#include "utils/memory_buffer.h"

bool ArrayResize(void **ptr, size_t count, size_t elementSize) {
    if (!ptr) return false;

    size_t newSize = count * elementSize;
    if (newSize == 0) {
        free(*ptr);
        *ptr = NULL;
        return true;
    }

    void *tmp = realloc(*ptr, newSize);
    if (tmp) {
        *ptr = tmp;
        return true;
    }

    return false; // Falha de alocação: ponteiro original preservado de forma segura
}

size_t MemoryUsageGet(void) {
    struct rusage rUsage;

    getrusage(RUSAGE_SELF, &rUsage);

    // No Linux, o ru_maxrss retorna o uso de memória em Kilobytes (KB).
    // Multiplicamos por 1024 para converter para Bytes.
    return (size_t)(rUsage.ru_maxrss * 1024L);
}

void BufferEnsure(void **buf, bool resize, size_t newCap, size_t elementSize) {
    if (!buf) return;

    if (!*buf) *buf = calloc(newCap, elementSize);
    else if (resize) ArrayResize(buf, newCap, elementSize);
}

void BufferEnsureBackfill(void **buf, bool resize, size_t newCap, size_t count, double startX, double stepX, double fillY) {
    if (!buf) return;

    if (!*buf) {
        *buf = malloc(newCap * sizeof(Vector2d));
        if (*buf) {
            Vector2d *vBuf = (Vector2d*)*buf;
            for (size_t i = 0; i < count; i++) vBuf[i] = (Vector2d){startX + ((double)i * stepX), fillY};
        }
    }

    else if (resize) {
        void *temp = realloc(*buf, newCap * sizeof(Vector2d));
        if (temp) *buf = temp;
    }
}

void BufferFree(void **ptr) {
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    };
}
