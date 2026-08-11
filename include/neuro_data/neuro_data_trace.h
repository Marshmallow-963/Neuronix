#ifndef NEURO_DATA_TRACE_H
#define NEURO_DATA_TRACE_H

#include "neuro_data_state.h"
#include "network/network_struct.h"

/* ----------------------------------------------------------------------------
 * ----------------------- FUNÇÕES DE GERENCIAMENTO ---------------------------
 * ------------------------------------------------------------------------- */

bool NeuroTraceCapacityEnsure(
    NeuroTraceBuffer *buf,
    size_t neededCount,
    size_t nrnCount,
    bool exactSize
);

void NeuroTraceCountersReset(NeuroTraceBuffer *buf);
void NeuroTraceCleanup(NeuroTraceBuffer *buf);
void NeuroTraceNeuronsReset(NeuralNetwork *net);

/* ----------------------------------------------------------------------------
 * ----------------------- FUNÇÕES DE EXTRAÇÃO E DADOS ------------------------
 * ------------------------------------------------------------------------- */
void NeuroTraceBackFill(
    const NeuroTraceBackFillData *data,
    NeuroTraceBuffer *buf,
    bool doResize,
    size_t nrnIdx,
    size_t newCap,
    double dt
);

void NeuroTraceDetailedDataAdd(
    const NeuroTraceBackFillData *data,
    NeuroTraceBuffer *buf,
    size_t nrnIdx,
    double time
);

void NeuroTraceLinearRangeGet(
    size_t capacity,
    size_t currentCount,
    size_t lastSavedCount,
    Vector2d *array,
    size_t *outCount,
    Vector2d **outPart1,
    size_t *outSize1,
    Vector2d **outPart2,
    size_t *outSize2
);

void NeuroTraceNodeExtract(NeuroTraceBackFillData *data, NeuronNode *node);

void NeuroTraceTauMatrixDataAdd(
    NeuroTraceBuffer *buf,
    size_t id1,
    size_t id2,
    double xVal,
    double yVal
);

#endif // NEURO_TRACES_UTILS_H
