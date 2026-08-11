#ifndef NEURO_ANALYSIS_H
#define NEURO_ANALYSIS_H

#include <stddef.h>
#include <stdbool.h>

#include "utils/struct_data.h"

bool IsSpikePoint(double vPast, double vPresent, double vFuture);

size_t *FilterBurstStarts(
    Vector2d *trace,
    size_t *spikes,
    size_t count,
    size_t *newCount,
    double isiThreshold
);

size_t *Spikes(Vector2d *trace, size_t *spikes, size_t count);

size_t Latency(
    Vector2d *srcTrace,
    size_t *srcSpikes,
    size_t srcCount,
    Vector2d *tgtTrace,
    size_t *tgtSpikes,
    size_t tgtCount,
    Vector2d *outTau,
    double isiThreshold,
    double windowSize
);

#endif // NEURO_ANALYSIS_H
