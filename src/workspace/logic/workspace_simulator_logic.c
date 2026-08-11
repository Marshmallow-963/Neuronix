#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <bits/pthreadtypes.h>

#include "graph/core/graph_state.h"

#include "log/logger.h"

#include "network/network_manager.h"
#include "neuro_data/neuro_data_trace.h"

#include "workspace/core/workspace_state.h"
#include "workspace/logic/workspace_logic.h"
#include "workspace/plotting/workspace_plot_logic.h"

/* =========================================================================
 * SIMULATOR LOGIC
 * ========================================================================= */

static inline void SimulationCapacityAndBackfillCheck(WorkspaceState *state, bool capacityGrew);
static inline void SimulationUpdateDetailedTraces(WorkspaceState *state);

void SimulationCleanup(WorkspaceState *state) {
    if (!state) return;

    NeuroTraceCleanup(&state->liveData);

    if (state->topology.incomingCounters) {
        free(state->topology.incomingCounters);
        state->topology.incomingCounters = NULL;
    }

    if (state->topology.nrnLayouts) {
        free(state->topology.nrnLayouts);
        state->topology.nrnLayouts = NULL;
    }

    if (state->topology.synLayouts) {
        free(state->topology.synLayouts);
        state->topology.synLayouts = NULL;
    }

    state->topology.synapseCount = 0;

    if (state->network) {
        NetworkFree(state->network);
        state->network = NULL;
    }
}

void SimulationDataReset(WorkspaceState *state) {
    if (!state) return;

    if (state->saving.isSaving) {
        Logger(WARNING, "Não é possível resetar enquanto um salvamento está em execução!");
        return;
    }

    state->runtime.isRunning   = false;
    state->runtime.currentTime = 0.0f;

    NeuroTraceCountersReset(&state->liveData);
    PlotStateResetAll();
    NeuroTraceNeuronsReset(state->network);

    state->saving.autoSaveSessionId    = 0;
    state->saving.lastSavedDataCount   = 0;
    state->saving.lastSavedRasterCount = 0;
    state->saving.isFinalSaveRequested = false;
    state->saving.isSaving             = false;
}

void SimulationUpdate(WorkspaceState *state) {
    if (!state) return;

    NeuralNetwork *net = state->network;

    if (!state->runtime.isRunning || !net) {
        WorkspacePlotLiveBoundsApply(state);
        return;
    }

    for (int i = 0; i < ITERACTIONS; i++) {
        bool capacityGrew = NeuroTraceCapacityEnsure(
            &state->liveData, state->liveData.dataCount + 1,
            net->neuronCount, false
        );

        SimulationCapacityAndBackfillCheck(state, capacityGrew); // 1.
        NetworkUpdate(net);                                      // 2.
        SimulationUpdateDetailedTraces(state);                   // 3.

        state->runtime.currentTime += state->network->dt;
        state->liveData.dataCount++;
    }

    WorkspacePlotLiveBoundsApply(state);
}

void SimulationNeuronInject(WorkspaceState *state, NeuronModel nrnModel, IzNeuronType izType) {
    pthread_mutex_lock(&state->concurrency.simMutex);

        if (!state->network) {
            state->network = NetworkCreate(state->networkCapacity, DT);
            if (state->networkCapacity > 0) {
                state->topology.nrnLayouts = calloc(state->networkCapacity, sizeof(NeuronLayout));
                state->topology.incomingCounters = calloc(state->networkCapacity, sizeof(int)); // Alocação inicial
            }
        }

        NeuralNetwork *net = state->network;
        if (net && net->neuronCount >= net->capacity) {
            size_t newCap = net->capacity + 6;

            if (NetworkResizeCapacity(net, newCap)) {
                NeuronLayout *newLayouts = realloc(state->topology.nrnLayouts, newCap * sizeof(NeuronLayout));
                int *newCounters = realloc(state->topology.incomingCounters, newCap * sizeof(int)); // Realocação segura

                if (newLayouts) {
                    for (size_t i = net->capacity; i < newCap; i++) {
                        newLayouts[i] = (NeuronLayout){ 0 };
                    }
                    state->topology.nrnLayouts = newLayouts;
                }

                if (newCounters) {
                    for (size_t i = net->capacity; i < newCap; i++) {
                        newCounters[i] = 0;
                    }
                    state->topology.incomingCounters = newCounters;
                }

                state->networkCapacity = newCap;
            }
        }

        if (net && net->neuronCount < net->capacity) {
            NetworkAddNeuron(net, nrnModel, izType);

            size_t newNrnIdx = net->neuronCount - 1;
            NetworkNeuronExternalCurrentSet(net, newNrnIdx, state->params.externalCurrent);

            if (state->topology.nrnLayouts) {
                state->topology.nrnLayouts[newNrnIdx] = (NeuronLayout){
                    .isDragging  = false,
                    .scale       = 1.0f,
                    .neuronIndex = (int)newNrnIdx,
                    .position    = (Vector2){
                        150.0f + (float)(newNrnIdx % 5) * 110.0f,
                        150.0f + ((float)newNrnIdx / 5) * 110.0f
                    }
                };

                state->topology.count = net->neuronCount;
            }
        }
    pthread_mutex_unlock(&state->concurrency.simMutex);
}

void *SimulationWorkerThread(void *arg) {
    WorkspaceState *state = (WorkspaceState*)arg;

    while (!state->runtime.killThread) {
        bool bufferOverloaded = false;

        pthread_mutex_lock(&state->concurrency.simMutex);
            if (state->saving.activeTracesMask == 0) {
                state->saving.lastSavedDataCount = state->liveData.dataCount;
            }

            size_t pending = state->liveData.dataCount - state->saving.lastSavedDataCount;
            if ((double)pending > (BUFFER_CAPACITY * BACKPRESSURE_THRESHOLD)) bufferOverloaded = true;
        pthread_mutex_unlock(&state->concurrency.simMutex);

        if (bufferOverloaded) {
            if (!state->saving.isSaving) {
                Logger(WARNING, "Backpressure atingido mas Auto-save não disparou! Forçando salvamento...");
            }

            usleep(BACKPRESSURE_DELAY_MS); // Pausa curta para o SQLite respirar
            continue;
        }

        pthread_mutex_lock(&state->concurrency.simMutex);
            SimulationUpdate(state);
        pthread_mutex_unlock(&state->concurrency.simMutex);

        usleep(REFRESH_RATE_MS_IDLE);
    }
    return NULL;
}

static inline void SimulationCapacityAndBackfillCheck(WorkspaceState *state, bool capacityGrew) {
    NeuralNetwork    *net = state->network;
    NeuroTraceBuffer *buf = &state->liveData;

    size_t nrnCount = net->neuronCount;
    size_t limit = (nrnCount < MAX_DETAILED_PLOTS) ? nrnCount : MAX_DETAILED_PLOTS;

    for (size_t i = 0; i < limit; i++) {
        bool isNewNeuron = (!buf->neuronTraces[i] && buf->dataCount > 0);

        if (capacityGrew || isNewNeuron) {
            NeuroTraceBackFillData bData = {0};
            NeuroTraceNodeExtract(&bData, &net->neurons[i]);
            NeuroTraceBackFill(&bData, buf, (capacityGrew || isNewNeuron), i, buf->bufferCapacity, net->dt);
        }
    }
}

static inline void SimulationUpdateDetailedTraces(WorkspaceState *state) {
    size_t nrnCount = state->network->neuronCount;
    size_t limit = (nrnCount < MAX_DETAILED_PLOTS) ? nrnCount : MAX_DETAILED_PLOTS;

    for (size_t i = 0; i < limit; i++) {
        NeuroTraceBackFillData liveData = {0};
        NeuroTraceNodeExtract(&liveData, &state->network->neurons[i]);
        NeuroTraceDetailedDataAdd(&liveData, &state->liveData, i, state->runtime.currentTime);
    }
}
