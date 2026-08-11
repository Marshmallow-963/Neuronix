#define _GNU_SOURCE

#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "analysis/neuro_analysis.h"
#include "neuro_data/neuro_data_trace.h"

#include "workspace/logic/workspace_logic.h"
#include "workspace/core/workspace_state.h"

static inline void RasterPlotBatchProcess(WorkspaceState *state);
static inline void SpikeLatencyBatchProcess(WorkspaceState *state);

void *AnalyzerWorkerThread(void *arg) {
    WorkspaceState *state = (WorkspaceState*)arg;

    while (!state->concurrency.anaKillThread) {
        int taskType = -1;

        pthread_mutex_lock(&state->concurrency.anaMutex);
            if (state->analysis.isAnalyzing) {
                if (state->analysis.currentAnalysisIndex == 0) {
                    state->analysis.currentAnalysisIndex = 1;
                    taskType = 0;
                } else if (state->analysis.currentAnalysisIndex == 2) {
                    state->analysis.currentAnalysisIndex = 3;
                    taskType = 1;
                }
            }
        pthread_mutex_unlock(&state->concurrency.anaMutex);

        if (taskType == 0) SpikeLatencyBatchProcess(state);
        else if (taskType == 1) RasterPlotBatchProcess(state);

        usleep(REFRESH_RATE_MS_IDLE);
    }
    return NULL;
}

void AnalyzerStateReset(WorkspaceState *state) {
    if (!state) return;

    state->loading.activeLoadStmt = NULL;
    state->loading.isLoading = false;
    state->loading.loadingProgress = 0.0f;
    state->loading.targetDataCount = 0;
    state->session.loadedNeuronCount = 0;

    pthread_mutex_lock(&state->concurrency.anaMutex);
        state->analysis.isAnalyzing = false;
        state->analysis.currentAnalysisIndex = 0;
        state->analysis.tauAllocCapacity = 0;

        NeuroTraceCleanup(&state->staticData);
    pthread_mutex_unlock(&state->concurrency.anaMutex);
}

static inline void RasterPlotBatchProcess(WorkspaceState *state) {
    pthread_mutex_lock(&state->concurrency.anaMutex);
        size_t count = state->staticData.dataCount;
        size_t neuronCount = state->session.loadedNeuronCount;
    pthread_mutex_unlock(&state->concurrency.anaMutex);

    if (count == 0 || neuronCount == 0) {
        pthread_mutex_lock(&state->concurrency.anaMutex);
            state->analysis.isAnalyzing = false;
        pthread_mutex_unlock(&state->concurrency.anaMutex);
        return;
    }

    for (size_t i = 0; i < neuronCount; i++) {
        pthread_mutex_lock(&state->concurrency.anaMutex);
            Vector2d *trace = state->staticData.neuronTraces[i];
        pthread_mutex_unlock(&state->concurrency.anaMutex);


        if (!trace) continue;

        size_t spikeCount = 0;
        size_t *spikeIndices = Spikes(trace, &spikeCount, count);

        if (spikeIndices && spikeCount > 0) {
            Vector2d *rasterPoints = malloc(spikeCount * sizeof(Vector2d));

            if (rasterPoints) {
                // Mapeia o tempo do spike no eixo X e o ID do neurônio no eixo Y
                for (size_t s = 0; s < spikeCount; s++) {
                    rasterPoints[s].x = trace[spikeIndices[s]].x;
                    rasterPoints[s].y = (double)(i + 1);
                }

                pthread_mutex_lock(&state->concurrency.anaMutex);
                    if (state->staticData.rasterTraces[i]) {
                        free(state->staticData.rasterTraces[i]);
                    }

                    state->staticData.rasterTraces[i]      = rasterPoints;
                    state->staticData.rasterPointsCount[i] = spikeCount;
                pthread_mutex_unlock(&state->concurrency.anaMutex);
            }
            free(spikeIndices);
        } else {
            pthread_mutex_lock(&state->concurrency.anaMutex);
                if (state->staticData.rasterTraces[i]) {
                    free(state->staticData.rasterTraces[i]);
                    state->staticData.rasterTraces[i] = NULL;
                }

                state->staticData.rasterPointsCount[i] = 0;
            pthread_mutex_unlock(&state->concurrency.anaMutex);
        }
    }

    pthread_mutex_lock(&state->concurrency.anaMutex);
        state->analysis.isAnalyzing = false;
    pthread_mutex_unlock(&state->concurrency.anaMutex);
}

static inline void SpikeLatencyBatchProcess(WorkspaceState *state) {
    pthread_mutex_lock(&state->concurrency.anaMutex);
        size_t srcId = state->editor.sourceNeuronId;
        size_t tgtId = state->editor.targetNeuronId;
        size_t count = state->staticData.dataCount;

        Vector2d *traceSrc = state->staticData.neuronTraces[srcId];
        Vector2d *traceTgt = state->staticData.neuronTraces[tgtId];
    pthread_mutex_unlock(&state->concurrency.anaMutex);

    if (count == 0 || !traceSrc || !traceTgt) {
        pthread_mutex_lock(&state->concurrency.anaMutex);
            state->analysis.isAnalyzing = false;
        pthread_mutex_unlock(&state->concurrency.anaMutex);
        return;
    }

    size_t srcSpikeCount = 0, tgtSpikeCount = 0;
    size_t *srcSpikes = Spikes(traceSrc, &srcSpikeCount, count);
    size_t *tgtSpikes = Spikes(traceTgt, &tgtSpikeCount, count);

    // Buffer temporário local para armazenar os resultados do cálculo
    Vector2d *tempTau = malloc(srcSpikeCount * sizeof(Vector2d));
    size_t validPairs = 0;

    if (srcSpikes && tgtSpikes && tempTau) {
        validPairs = Latency(traceSrc, srcSpikes, srcSpikeCount, traceTgt, tgtSpikes, tgtSpikeCount, tempTau, 10, 50);
    }

    if (validPairs > 0 && tempTau) {
        size_t matrixIdx = (srcId * MAX_DETAILED_PLOTS) + tgtId;

        pthread_mutex_lock(&state->concurrency.anaMutex);
            // 1. Libera o ponteiro antigo se ele já existia para evitar memory leak de análises passadas
            if (state->staticData.tauTraces[matrixIdx]) {
                free(state->staticData.tauTraces[matrixIdx]);
                state->staticData.tauTraces[matrixIdx] = NULL;
            }

            // 2. Aloca dinamicamente o espaço exato e cirúrgico para a quantidade de pares válidos
            state->staticData.tauTraces[matrixIdx] = malloc(validPairs * sizeof(Vector2d));

            if (state->staticData.tauTraces[matrixIdx]) {
                // Copia os valores reais calculados pelo Analyzer
                memcpy(state->staticData.tauTraces[matrixIdx], tempTau, validPairs * sizeof(Vector2d));
                state->staticData.tauPointsCount[matrixIdx] = validPairs;
            } else {
                state->staticData.tauPointsCount[matrixIdx] = 0;
            }

            state->analysis.isAnalyzing = false;
        pthread_mutex_unlock(&state->concurrency.anaMutex);
    } else {
        size_t matrixIdx = (srcId * MAX_DETAILED_PLOTS) + tgtId;

        pthread_mutex_lock(&state->concurrency.anaMutex);
            // Se a análise não gerou pares válidos, limpa dados residuais anteriores deste par
            if (state->staticData.tauTraces[matrixIdx]) {
                free(state->staticData.tauTraces[matrixIdx]);
                state->staticData.tauTraces[matrixIdx] = NULL;
            }
            state->staticData.tauPointsCount[matrixIdx] = 0;
            state->analysis.isAnalyzing = false;
        pthread_mutex_unlock(&state->concurrency.anaMutex);
    }

    if (srcSpikes) free(srcSpikes);
    if (tgtSpikes) free(tgtSpikes);
    if (tempTau)   free(tempTau);
}
