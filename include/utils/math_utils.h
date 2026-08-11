#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stddef.h>

size_t TimeToIndex(
    double time,
    double dt,
    size_t offset
);

void Bounds(
    double val,
    double *min,
    double *max
);

void EnsureRange(
    double *min,
    double *max
);

void Margin(double *min, double *max);

void TimeWindow(
    double currentTime,
    double windowSize,
    double dt,
    double *outMin,
    double *outMax
);

#endif // MATH_UTILS_H
