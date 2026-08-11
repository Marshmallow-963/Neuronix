#include <stddef.h>

#include "utils/struct_data.h"
#include "analysis/statistics.h"

double Average(Vector2d *data, size_t count) {
    if (!data || count == 0) return 0.0f;

    double sum = 0;
    for (size_t i = 0; i < count; i++) sum += data[i].y;
    return (sum / (double)(count));
}
