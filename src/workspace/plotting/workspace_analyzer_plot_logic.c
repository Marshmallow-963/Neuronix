#include "stdio.h"
#include "string.h"

#include "graph/core/graph_state.h"
#include "neuro_data/neuro_data_state.h"

#include "workspace/plotting/workspace_plot_enums.h"
#include "workspace/config/workspace_ui_config.h"
#include "workspace/storage/workspace_storage.h"
#include "workspace/plotting/workspace_plot_logic.h"

#include "utils/math_utils.h"

// =========================================================================
// HELPERS INTERNOS DE BUSCA TEMPORAL (EIXO X)
// =========================================================================

static inline size_t LowerBoundX(const Vector2d *trace, size_t count, double xVal) {
    size_t low = 0, high = count;
    while (low < high) {
        size_t mid = low + (high - low) / 2;
        if (trace[mid].x < xVal) low = mid + 1;
        else high = mid;
    }
    return low;
}

static inline size_t UpperBoundX(const Vector2d *trace, size_t count, double xVal) {
    size_t low = 0, high = count;
    while (low < high) {
        size_t mid = low + (high - low) / 2;
        if (trace[mid].x <= xVal) low = mid + 1;
        else high = mid;
    }
    return low;
}

// =========================================================================
// CÁLCULOS DE LIMITES E JANELAMENTO (LÓGICA)
// =========================================================================

void WorkspacePlotStaticBoundsApply(WorkspaceState *state) {
    if (!state) return;

    int slot = WorkspaceTableToSlot(state->session.activeTableId);
    if (slot == -1) return;

    PlotView *v = PlotStateGetView(slot);
    if (!v) return;

    double xMax = 0.0;
    double offset = state->ui.offset;

    NeuroTraceBuffer *data = &state->staticData;
    Vector2d **trace = WorkspaceTraceArrayGet(data, state->session.activeTableId);

    if (trace) {
        for (size_t i = 0; i < state->session.loadedNeuronCount; i++) {
            if (trace[i] && data->dataCount > 0) {
                xMax = trace[i][data->dataCount - 1].x;
                break;
            }
        }
    } else if (state->session.activeTableId == TABLE_ID_RASTER) {
        for (size_t i = 0; i < state->session.loadedNeuronCount; i++) {
            size_t count = state->staticData.rasterPointsCount[i];
            if (count > 0 && state->staticData.rasterTraces[i]) {
                double last = state->staticData.rasterTraces[i][count - 1].x;
                if (last > xMax) xMax = last;
            }
        }
    }

    // O valor base 50.0 representa 100% da visualização
    double zoomFactor = state->ui.timeWindow / 50.0;
    if (zoomFactor <= 0.0) zoomFactor = 0.01;

    // Travas de segurança para garantir que o zoom funcione de forma fluida
    if (zoomFactor < 0.05) zoomFactor = 0.05; // Limite máximo de aproximação (5% do gráfico)
    if (zoomFactor > 1.0) zoomFactor = 1.0;   // Limite máximo de afastamento (100% do gráfico)

    if (zoomFactor >= 1.0 && offset <= 0.0) {
        v->xMin = 0.0;
        v->xMax = xMax;
    } else {
        double calculatedWindow = xMax * zoomFactor;
        TimeWindow(xMax - offset, calculatedWindow, 0.0, &v->xMin, &v->xMax);
    }
}

void WorkspacePlotRasterBoundsApply(WorkspaceState *state, int slot) {
    if (!state) return;

    WorkspacePlotStaticBoundsApply(state);

    PlotView *v = PlotStateGetView(slot);
    if (!v) return;

    size_t neurons = state->session.loadedNeuronCount;
    v->yMin = 0.0;
    v->yMax = (double)(neurons + 1);
}

void WorkspacePlotTreatmentBoundsApply(WorkspaceState *state) {
    if (!state || state->staticData.dataCount == 0) return;

    size_t srcId = state->editor.sourceNeuronId;
    size_t tgtId = state->editor.targetNeuronId;

    if (srcId >= MAX_DETAILED_PLOTS || tgtId >= MAX_DETAILED_PLOTS) return;

    size_t matrixIdx = (srcId * MAX_DETAILED_PLOTS) + tgtId;
    size_t count = state->staticData.tauPointsCount[matrixIdx];

    if (count == 0) return;

    PlotView *v = PlotStateGetView(SLOT_ANALYZER_TAU);
    if (!v) return;

    size_t startIdx = 0;
    size_t pointsToShow = state->ui.pointsToShow;
    size_t endIdx = (count > pointsToShow) ? pointsToShow - 1 : count - 1;

    Vector2d *tauPoints = state->staticData.tauTraces[matrixIdx];
    if (!tauPoints) return;

    // Limites originais baseados nos dados disponíveis
    double currentXMax = tauPoints[endIdx].x;
    double currentXMin = tauPoints[startIdx].x;

    // Aplicando Zoom no Eixo X
    double zoomFactor = state->ui.timeWindow / 50.0;
    if (zoomFactor < 0.05) zoomFactor = 0.05;
    if (zoomFactor > 1.0) zoomFactor = 1.0;

    double offset = state->ui.offset;

    if (zoomFactor >= 1.0 && offset <= 0.0) {
        v->xMin = currentXMin;
        v->xMax = currentXMax;
    } else {
        double calculatedWindow = currentXMax * zoomFactor;
        TimeWindow(currentXMax - offset, calculatedWindow, currentXMin, &v->xMin, &v->xMax);
    }

    // --- CORREÇÃO DO EIXO Y ---
    // Inicializa min/max dinamicamente apenas com pontos dentro do intervalo visível no Eixo X
    bool limitsFound = false;
    double tauMinY = 0.0;
    double tauMaxY = 0.0;

    for (size_t i = startIdx; i <= endIdx; i++) {
        if (tauPoints[i].x >= v->xMin && tauPoints[i].x <= v->xMax) {
            if (!limitsFound) {
                tauMinY = tauPoints[i].y;
                tauMaxY = tauPoints[i].y;
                limitsFound = true;
            } else {
                Bounds(tauPoints[i].y, &tauMinY, &tauMaxY);
            }
        }
    }

    if (limitsFound) {
        EnsureRange(&tauMinY, &tauMaxY);
        v->yMin = tauMinY;
        v->yMax = tauMaxY;
    }
}

void WorkspacePlotMultiLinePrepareRange(
    WorkspaceState *state, int slot, Vector2d **data2D, size_t totalCount,
    size_t *outVisStartIdx, size_t *outVisEndIdx, size_t *outDecimateStep
) {
    if (!state || !data2D || totalCount == 0) return;

    // 1. Aplica o Zoom/Offset no Eixo X
    WorkspacePlotStaticBoundsApply(state);

    PlotView *v = PlotStateGetView(slot);
    if (!v) return;

    size_t neurons = state->session.loadedNeuronCount;
    if (neurons > MAX_DETAILED_PLOTS) neurons = MAX_DETAILED_PLOTS;

    Vector2d *refTrace = NULL;
    for (size_t n = 0; n < neurons; n++) {
        if (data2D[n]) { refTrace = data2D[n]; break; }
    }

    size_t visStartIdx = 0;
    size_t visEndIdx = totalCount > 0 ? totalCount - 1 : 0;

    if (refTrace && totalCount > 0) {
        visStartIdx = LowerBoundX(refTrace, totalCount, v->xMin);
        size_t upper = UpperBoundX(refTrace, totalCount, v->xMax);
        visEndIdx = (upper > 0) ? upper - 1 : 0;
    }

    if (visStartIdx >= totalCount) visStartIdx = 0;
    if (visEndIdx < visStartIdx) visEndIdx = visStartIdx;

    size_t visibleCount = visEndIdx - visStartIdx + 1;
    size_t safeLimit = MAX_RENDER_POINTS - 1;
    size_t decimateStep = (visibleCount > safeLimit) ? (visibleCount / safeLimit) : 1;
    if (decimateStep == 0) decimateStep = 1;

    if (outVisStartIdx)  *outVisStartIdx = visStartIdx;
    if (outVisEndIdx)    *outVisEndIdx = visEndIdx;
    if (outDecimateStep) *outDecimateStep = decimateStep;

    // 2. CORREÇÃO DO EIXO Y: Percorre TODOS os pontos visíveis (j++) e não apenas decimados
    if (!state->ui.lockBounds) {
        bool limitsFound = false;
        double calculatedYMin = 0.0, calculatedYMax = 0.0;

        pthread_mutex_lock(&state->concurrency.anaMutex);
        for (size_t j = visStartIdx; j <= visEndIdx; j++) {
            for (size_t n = 0; n < neurons; n++) {
                if (data2D[n]) {
                    double ptY = data2D[n][j].y;
                    if (!limitsFound) {
                        calculatedYMin = ptY;
                        calculatedYMax = ptY;
                        limitsFound = true;
                    } else {
                        if (ptY < calculatedYMin) calculatedYMin = ptY;
                        if (ptY > calculatedYMax) calculatedYMax = ptY;
                    }
                }
            }
        }
        pthread_mutex_unlock(&state->concurrency.anaMutex);

        if (limitsFound) {
            // Opcional: Adiciona 5% de margem no topo e na base para o sinal não encostar na borda da tela
            double range = calculatedYMax - calculatedYMin;
            double margin = (range > 0.0) ? (range * 0.05) : 1.0;

            calculatedYMin -= margin;
            calculatedYMax += margin;

            EnsureRange(&calculatedYMin, &calculatedYMax);
            v->yMin = calculatedYMin;
            v->yMax = calculatedYMax;
        }
    }
}
