#include <string.h>

#include "utils/memory_buffer.h"

#include "model/neural/izhikevich/izhikevich_struct.h"
#include "model/neural/hodgkin-huxley/hodgkin_huxley_rates.h"
#include "model/neural/hodgkin-huxley/hodgkin_huxley_config.h"
#include "model/neural/hodgkin-huxley/hodgkin_huxley_struct.h"

#include "neuro_data/neuro_data_trace.h"

bool NeuroTraceCapacityEnsure(NeuroTraceBuffer *buf, size_t neededCount, size_t nrnCount, bool exactSize) {
    if (!buf) return false;

    bool needResize = false;
    size_t newCap = buf->bufferCapacity;

    // --- MODO SIMULATOR (CIRCULAR) ---
    if (buf->isCircular) {
        if (buf->bufferCapacity >= BUFFER_CAPACITY) return false;
        newCap = BUFFER_CAPACITY;
        needResize = true;
    }
    // --- MODO ANALYZER (LINEAR/INFINITO) ---
    else {
        needResize = (neededCount >= buf->bufferCapacity) || (buf->bufferCapacity == 0);
        if (needResize || exactSize) {
            if (exactSize) newCap = neededCount;
            else {
                size_t currentCap = buf->bufferCapacity;
                newCap = (currentCap == 0) ? TIME_CAPACITY : currentCap * 2;
                if (newCap < neededCount) newCap = neededCount + TIME_CAPACITY;
            }
        }
    }

    if (!needResize && !exactSize) return false;

    bool doResize = true;
    size_t size = sizeof(Vector2d);
    size_t limit = (nrnCount < MAX_DETAILED_PLOTS) ? nrnCount : MAX_DETAILED_PLOTS;

    for (size_t i = 0; i < limit; i++) {
        BufferEnsure((void**)&buf->izRecoveryTraces[i], doResize, newCap, size);

        BufferEnsure((void**)&buf->hhCurrentPlots.kCurrent[i],    doResize, newCap, size);
        BufferEnsure((void**)&buf->hhCurrentPlots.naCurrent[i],   doResize, newCap, size);
        BufferEnsure((void**)&buf->hhCurrentPlots.leakCurrent[i], doResize, newCap, size);

        BufferEnsure((void**)&buf->hhGatePlots.MGate[i], doResize, newCap, size);
        BufferEnsure((void**)&buf->hhGatePlots.NGate[i], doResize, newCap, size);
        BufferEnsure((void**)&buf->hhGatePlots.HGate[i], doResize, newCap, size);

        BufferEnsure((void**)&buf->neuronTraces[i],     doResize, newCap, size);
        BufferEnsure((void**)&buf->ntFractionTraces[i], doResize, newCap, size);
        BufferEnsure((void**)&buf->phaseSpaceTraces[i], doResize, newCap, size);
        BufferEnsure((void**)&buf->synapticTraces[i],   doResize, newCap, size);

        BufferEnsure((void**)&buf->rasterTraces[i], doResize, newCap, size);
    }

    buf->bufferCapacity = newCap;
    return needResize;
}

void NeuroTraceCountersReset(NeuroTraceBuffer *buf) {
    if (!buf) return;
    buf->dataCount = 0;
    memset(buf->tauPointsCount,    0, sizeof(buf->tauPointsCount));
    memset(buf->rasterPointsCount, 0, sizeof(buf->rasterPointsCount));
}

void NeuroTraceCleanup(NeuroTraceBuffer *buf) {
    if (!buf) return;

    for (size_t i = 0; i < MAX_DETAILED_PLOTS; i++) {
        for (size_t j = 0; j < MAX_DETAILED_PLOTS; j++) {
            BufferFree((void**)&buf->tauTraces[(i * MAX_DETAILED_PLOTS) + j]);
        }
    }

    for (size_t i = 0; i < MAX_DETAILED_PLOTS; i++) {
        BufferFree((void**)&buf->neuronTraces[i]);
        BufferFree((void**)&buf->synapticTraces[i]);
        BufferFree((void**)&buf->ntFractionTraces[i]);
        BufferFree((void**)&buf->phaseSpaceTraces[i]);

        BufferFree((void**)&buf->rasterTraces[i]);

        BufferFree((void**)&buf->izRecoveryTraces[i]);

        BufferFree((void**)&buf->hhGatePlots.MGate[i]);
        BufferFree((void**)&buf->hhGatePlots.HGate[i]);
        BufferFree((void**)&buf->hhGatePlots.NGate[i]);

        BufferFree((void**)&buf->hhCurrentPlots.kCurrent[i]);
        BufferFree((void**)&buf->hhCurrentPlots.naCurrent[i]);
        BufferFree((void**)&buf->hhCurrentPlots.leakCurrent[i]);
    }

    buf->dataCount      = 0;
    buf->bufferCapacity = 0;
    memset(buf->tauPointsCount,    0, sizeof(buf->tauPointsCount));
    memset(buf->rasterPointsCount, 0, sizeof(buf->rasterPointsCount));
}

void NeuroTraceBackFill(const NeuroTraceBackFillData *data, NeuroTraceBuffer *buf, bool doResize, size_t nrnIdx, size_t newCap, double dt) {
    if (!buf || nrnIdx >= MAX_DETAILED_PLOTS) return;

    size_t count = buf->dataCount;

    BufferEnsureBackfill((void**)&buf->neuronTraces[nrnIdx],     doResize, newCap, count, 0, dt, data->v);
    BufferEnsureBackfill((void**)&buf->synapticTraces[nrnIdx],   doResize, newCap, count, 0, dt, data->syn);
    BufferEnsureBackfill((void**)&buf->ntFractionTraces[nrnIdx], doResize, newCap, count, 0, dt, data->ntFrac);
    BufferEnsureBackfill((void**)&buf->phaseSpaceTraces[nrnIdx], doResize, newCap, count, data->v, 0, data->phaseY);

    switch (data->modelType) {
        case TRACE_MODEL_IZHIKEVICH: {
            BufferEnsureBackfill((void**)&buf->izRecoveryTraces[nrnIdx], doResize, newCap, count, 0, dt, data->phaseY);
        } break;

        case TRACE_MODEL_HODGKIN_HUXLEY: {
            BufferEnsureBackfill((void**)&buf->hhGatePlots.MGate[nrnIdx], doResize, newCap, count, 0, dt, data->hh_m);
            BufferEnsureBackfill((void**)&buf->hhGatePlots.NGate[nrnIdx], doResize, newCap, count, 0, dt, data->hh_n);
            BufferEnsureBackfill((void**)&buf->hhGatePlots.HGate[nrnIdx], doResize, newCap, count, 0, dt, data->hh_h);

            BufferEnsureBackfill((void**)&buf->hhCurrentPlots.kCurrent[nrnIdx],    doResize, newCap, count, 0, dt, data->hh_ik);
            BufferEnsureBackfill((void**)&buf->hhCurrentPlots.naCurrent[nrnIdx],   doResize, newCap, count, 0, dt, data->hh_ina);
            BufferEnsureBackfill((void**)&buf->hhCurrentPlots.leakCurrent[nrnIdx], doResize, newCap, count, 0, dt, data->hh_ileak);
        } break;

        default: break;
    }
}

void NeuroTraceDetailedDataAdd(const NeuroTraceBackFillData *data, NeuroTraceBuffer *buf, size_t nrnIdx, double time) {
    if (!buf || nrnIdx >= MAX_DETAILED_PLOTS || buf->bufferCapacity == 0) return;

    size_t idx = buf->isCircular ? (buf->dataCount % buf->bufferCapacity) : buf->dataCount;

    // Otimização de registradores locais (Evita carregar o array do ponteiro base repetidas vezes)
    Vector2d *vTrace  = buf->neuronTraces[nrnIdx];
    Vector2d *sTrace  = buf->synapticTraces[nrnIdx];
    Vector2d *ntTrace = buf->ntFractionTraces[nrnIdx];
    Vector2d *psTrace = buf->phaseSpaceTraces[nrnIdx];

    if (vTrace)  vTrace[idx]  = (Vector2d){ time, data->v };
    if (sTrace)  sTrace[idx]  = (Vector2d){ time, data->syn };
    if (ntTrace) ntTrace[idx] = (Vector2d){ time, data->ntFrac };
    if (psTrace) psTrace[idx] = (Vector2d){ data->v, data->phaseY };

    // Gravações específicas por modelo
    switch (data->modelType) {
        case TRACE_MODEL_IZHIKEVICH: {
            Vector2d *izRec = buf->izRecoveryTraces[nrnIdx];
            if (izRec) izRec[idx] = (Vector2d){ time, data->phaseY };
        } break;

        case TRACE_MODEL_HODGKIN_HUXLEY: {
            // Agrupamento local para otimizar o cache L1 da CPU
            Vector2d *mG = buf->hhGatePlots.MGate[nrnIdx];
            Vector2d *nG = buf->hhGatePlots.NGate[nrnIdx];
            Vector2d *hG = buf->hhGatePlots.HGate[nrnIdx];

            if (mG) mG[idx] = (Vector2d){ time, data->hh_m };
            if (nG) nG[idx] = (Vector2d){ time, data->hh_n };
            if (hG) hG[idx] = (Vector2d){ time, data->hh_h };

            Vector2d *iK  = buf->hhCurrentPlots.kCurrent[nrnIdx];
            Vector2d *iNa = buf->hhCurrentPlots.naCurrent[nrnIdx];
            Vector2d *iL  = buf->hhCurrentPlots.leakCurrent[nrnIdx];

            if (iK)  iK[idx]  = (Vector2d){ time, data->hh_ik };
            if (iNa) iNa[idx] = (Vector2d){ time, data->hh_ina };
            if (iL)  iL[idx]  = (Vector2d){ time, data->hh_ileak };
        } break;

        default: break;
    }
}

void NeuroTraceLinearRangeGet(
    size_t capacity, size_t currentCount, size_t lastSavedCount, Vector2d *array, size_t *outCount,
    Vector2d **outPart1, size_t *outSize1, Vector2d **outPart2, size_t *outSize2
) {
    if (!array || !outCount) return;

    size_t totalNew = currentCount - lastSavedCount;

    // Proteção contra estouro e aliasing em buffers circulares
    if (totalNew > capacity) {
        totalNew = capacity;
        lastSavedCount = currentCount - totalNew;
    }

    size_t endIdx   = currentCount % capacity;
    size_t startIdx = lastSavedCount % capacity;

    // Se startIdx < endIdx, os dados novos estão contíguos em memória (comum no modo Linear)
    if (startIdx < endIdx || totalNew == 0) {
        *outPart1 = &array[startIdx];
        *outSize1 = totalNew;
        *outPart2 = NULL;
        *outSize2 = 0;
    }
    // Se startIdx >= endIdx, houve quebra de borda (exclusivo do modo Circular)
    else {
        *outPart1 = &array[startIdx];
        *outSize1 = capacity - startIdx;
        *outPart2 = &array[0];
        *outSize2 = endIdx;
    }

    *outCount = totalNew;
}

void NeuroTraceNeuronsReset(NeuralNetwork *net) {
    if (!net) return;

    for (size_t i = 0; i < net->neuronCount; i++) {
        NeuronNode *node = &net->neurons[i];
        switch (node->type) {
            case IZHIKEVICH_MODEL: {
                IzhikevichModel *iz = (IzhikevichModel*)node->interface.modelData;
                iz->neuron.state.v[0] = iz->neuron.config.c;
                iz->neuron.state.u[0] = iz->neuron.config.b * iz->neuron.config.c;

                if (iz->neuron.currents.Isyn) *iz->neuron.currents.Isyn = 0.0f;
                node->previousVolt = iz->neuron.config.c;
            } break;

            case HODGKIN_HUXLEY_MODEL: {
                HodgkinHuxleyModel *hh = (HodgkinHuxleyModel*)node->interface.modelData;
                hh->neuron.state.v[0] = HH_SQUID_AXON.restingPotential;

                hh->neuron.state.m[0] = GATE_REST_M;
                hh->neuron.state.n[0] = GATE_REST_N;
                hh->neuron.state.h[0] = GATE_REST_H;

                hh->neuron.currents.iL[0]  = 0.00f;
                hh->neuron.currents.iK[0]  = 0.00f;
                hh->neuron.currents.iNa[0] = 0.00f;

                if (hh->neuron.currents.iSyn) *hh->neuron.currents.iSyn = 0.0f;
                node->previousVolt = HH_SQUID_AXON.restingPotential;
            } break;

            default: break;
        }
    }
}

void NeuroTraceNodeExtract(NeuroTraceBackFillData *data, NeuronNode *node) {
    if (!data || !node) return;

    data->v   = *(node->interface.membranePotential);
    data->syn = (node->interface.synapticCurrent) ? *(node->interface.synapticCurrent) : 0.0f;

    data->ntFrac =
        (node->outboundSynapses && node->outboundSynapses->wrapper->physics.openChannels)
        ? *(node->outboundSynapses->wrapper->physics.openChannels)
        : 0.0f;

    switch (node->type) {
        case IZHIKEVICH_MODEL: {
            IzhikevichModel *iz = (IzhikevichModel*)node->interface.modelData;
            data->modelType = TRACE_MODEL_IZHIKEVICH;
            data->phaseY = iz->neuron.state.u[0];
        } break;

        case HODGKIN_HUXLEY_MODEL: {
            data->modelType = TRACE_MODEL_HODGKIN_HUXLEY;
            HodgkinHuxleyModel *hh = (HodgkinHuxleyModel*)node->interface.modelData;

            data->hh_m = hh->neuron.state.m[0];
            data->hh_n = hh->neuron.state.n[0];
            data->hh_h = hh->neuron.state.h[0];

            data->hh_ik    = hh->neuron.currents.iK[0];
            data->hh_ina   = hh->neuron.currents.iNa[0];
            data->hh_ileak = hh->neuron.currents.iL[0];

            data->phaseY = hh->neuron.state.n[0];
        } break;

        default: break;
    }
}

void NeuroTraceTauMatrixDataAdd(NeuroTraceBuffer *buf, size_t id1, size_t id2, double xVal, double yVal) {
    if (!buf || buf->bufferCapacity == 0) return;
    if (id1 >= MAX_DETAILED_PLOTS || id2 >= MAX_DETAILED_PLOTS) return;

    size_t matrixIdx = (id1 * MAX_DETAILED_PLOTS) + id2;
    size_t idx = buf->isCircular ? (buf->tauPointsCount[matrixIdx] % buf->bufferCapacity) : buf->tauPointsCount[matrixIdx];

    Vector2d *tauTrace = buf->tauTraces[matrixIdx];
    if (tauTrace) {
        tauTrace[idx] = (Vector2d){ xVal, yVal };
        buf->tauPointsCount[matrixIdx]++;
    }
}
