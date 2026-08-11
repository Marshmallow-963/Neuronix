#include "utils/math_utils.h"

size_t TimeToIndex(double time, double dt, size_t offset) {
    size_t absIdx = (size_t)(time / dt);
    return (absIdx > offset) ? (absIdx - offset) : 0;
}

void Bounds(double val, double *min, double *max) {
    if (val < *min) *min = val;
    if (val > *max) *max = val;
}

void EnsureRange(double *min, double *max) {
    if (*min == *max) {
        *min -= 1.0;
        *max += 1.0;
    }
}

void Margin(double *min, double *max) {
    if (*max <= *min) return;

    double range = *max - *min;
    double margin = range * 0.05;

    *min -= margin;
    *max += margin;
}

void TimeWindow(double currentTime, double windowSize, double dt, double *outMin, double *outMax) {
    if (windowSize > 0.0) {
        *outMax = currentTime < 0.0 ? 0.0 : currentTime;
        *outMin = (*outMax - windowSize) < 0.0 ? 0.0 : (*outMax - windowSize);
    }
    else {
        *outMin = 0.0;
        *outMax = currentTime > 0.0 ? currentTime : dt;
    }
}
