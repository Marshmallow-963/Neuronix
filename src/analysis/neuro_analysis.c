#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "utils/struct_data.h"

#include "analysis/neuro_analysis.h"
#include "analysis/analysis_config.h"

bool IsSpikePoint(double vPast, double vPresent, double vFuture) {
    return (vPresent > THRESHOLD && vPresent > vPast && vPresent > vFuture);
}

size_t *Spikes(Vector2d *trace, size_t *spikes, size_t count) {
    if (!trace || !spikes) return NULL;

    if (count < MIN_TRACE_POINTS) return NULL;

    *spikes = 0;
    size_t *index = malloc(count * sizeof(size_t));

    for (size_t i = 1; i < count - 1; i++) {
        if (IsSpikePoint(trace[i-1].y, trace[i].y, trace[i+1].y)) index[(*spikes)++] = i;
    }

    return index;
}

size_t *FilterBurstStarts(Vector2d *trace, size_t *spikes, size_t count, size_t *newCount, double isiThreshold) {
    if (!trace || !spikes || count == 0) {
        if (newCount) *newCount = 0;
        return NULL;
    }

    size_t *filteredIdx = malloc(count * sizeof(size_t));
    size_t found = 0;

    // O primeiro spike é sempre o início do primeiro burst
    filteredIdx[found++] = spikes[0];

    for (size_t i = 1; i < count; i++) {
        double currentTime = trace[spikes[i]].x;
        double previousTime = trace[spikes[i-1]].x;

        if ((currentTime - previousTime) > isiThreshold) filteredIdx[found++] = spikes[i];
    }

    if (newCount) *newCount = found;
    return filteredIdx;
}

size_t Latency(
    Vector2d *srcTrace, size_t *srcSpikes, size_t srcCount,
    Vector2d *tgtTrace, size_t *tgtSpikes, size_t tgtCount,
    Vector2d *outTau, double isiThreshold, double windowSize
) {
    // Adicionado outTau na verificação de segurança
    if (!srcTrace || !tgtTrace || !srcSpikes || !tgtSpikes || !outTau) return 0;

    // --- FILTRAGEM DE BURSTING ---
    size_t fSrcCount = 0, fTgtCount = 0;
    size_t *fSrcSpikes = FilterBurstStarts(srcTrace, srcSpikes, srcCount, &fSrcCount, isiThreshold);
    size_t *fTgtSpikes = FilterBurstStarts(tgtTrace, tgtSpikes, tgtCount, &fTgtCount, isiThreshold);

    if (fSrcCount == 0 || fTgtCount == 0) {
        // Garantindo que liberamos a memória mesmo se um deles for 0,
        // pois FilterBurstStarts pode retornar NULL ou um array de 1 elemento.
        if (fSrcSpikes) free(fSrcSpikes);
        if (fTgtSpikes) free(fTgtSpikes);
        return 0;
    }

    size_t tIdx = 0; // Agora o ponteiro corre sobre o alvo (Receiver)
    size_t validPairs = 0;
    size_t lastPairedTgtIdx = SIZE_MAX;

    // Percorre os inícios de burst da fonte (Sender)
    for (size_t sIdx = 0; sIdx < fSrcCount; sIdx++) {
        double timeSrc = srcTrace[fSrcSpikes[sIdx]].x;

        // Acha o início de burst do alvo (Receiver) MAIS PRÓXIMO
        while (tIdx + 1 < fTgtCount) {
            double diffAtual   = fabs(timeSrc - tgtTrace[fTgtSpikes[tIdx]].x);
            double diffProximo = fabs(timeSrc - tgtTrace[fTgtSpikes[tIdx + 1]].x);

            // Como o tempo é linear, avançamos enquanto o próximo for mais perto
            if (diffProximo <= diffAtual) tIdx++;
            else break; // A distância começou a aumentar, então tIdx é o vale
        }

        // Calcula o Tau final
        double bestTgtTime = tgtTrace[fTgtSpikes[tIdx]].x;
        double tau = bestTgtTime - timeSrc;
        // Se tau > 0: Receiver disparou DEPOIS do Sender
        // Se tau < 0: Receiver disparou ANTES do Sender

        // Filtro de Intervalo (Windowing) de Causalidade
        if (fabs(tau) <= windowSize) {

            // Garante que não usaremos o MESMO spike do receiver para dois senders diferentes
            // (Apenas um ponto do receiver dentro do intervalo)
            if (tIdx == lastPairedTgtIdx) continue;

            outTau[validPairs].x = (double)(validPairs + 1);
            outTau[validPairs].y = tau;
            validPairs++;

            lastPairedTgtIdx = tIdx; // Tranca o Receiver usado
        }
    }

    // Limpeza dos arrays temporários filtrados
    free(fSrcSpikes); free(fTgtSpikes);
    return validPairs;
}
