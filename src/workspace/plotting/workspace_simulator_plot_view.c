#include "graph/core/graph_state.h"
#include "graph/ui/graph_plot.h"

#include "network/network_enum.h"
#include "network/network_manager.h"

#include "workspace/config/workspace_ui_config.h"
#include "workspace/strings/workspace_strings.h"
#include "workspace/plotting/workspace_plot_enums.h"
#include "workspace/plotting/workspace_plot_view.h"

static inline void LiveDataDraw(
    WorkspaceState *state,
    Rectangle rec,
    size_t startIdx,
    size_t visibleCount
);

static inline void TracePlotDraw(
    WorkspaceState *state,
    Rectangle layout,
    int slot,
    size_t startIdx,
    size_t visibleCount
);

// =========================================================================
// SIMULATION (LIVE DATA) VIEWS
// =========================================================================

void WorkspaceLivePlotsDraw(WorkspaceState *state, Rectangle layout) {
    pthread_mutex_lock(&state->concurrency.simMutex);
        size_t dataCount = state->liveData.dataCount;
        if (dataCount < 2) {
            pthread_mutex_unlock(&state->concurrency.simMutex);
            return;
        }

        double dt = state->network->dt > 0.0 ? state->network->dt : 0.01;
        PlotView *vTime = PlotStateGetView(SLOT_SIMULATOR_VOLTAGE);

        size_t logicalStart = (size_t)(vTime->xMin / dt);
        size_t logicalEnd   = (size_t)(vTime->xMax / dt);

        size_t oldestLogical = (dataCount > BUFFER_CAPACITY) ? (dataCount - BUFFER_CAPACITY) : 0;
        if (logicalStart < oldestLogical) logicalStart = oldestLogical;
        if (logicalEnd > dataCount) logicalEnd = dataCount;

        size_t requestedPoints = (logicalEnd > logicalStart) ? (logicalEnd - logicalStart) : 0;

        size_t visibleCount = requestedPoints;
        if (visibleCount > BUFFER_CAPACITY) visibleCount = BUFFER_CAPACITY;
        if (visibleCount > dataCount) visibleCount = dataCount;

        size_t part2Count = 0;
        size_t part1Count = visibleCount;
        size_t startIdx = logicalStart % BUFFER_CAPACITY;

        if (startIdx + visibleCount >= BUFFER_CAPACITY) {
            part1Count = BUFFER_CAPACITY - startIdx;
            part2Count = visibleCount - part1Count;
        }

        LiveDataDraw(state, layout, startIdx, part1Count);
        if (part2Count > 0) LiveDataDraw(state, layout, 0, part2Count);
    pthread_mutex_unlock(&state->concurrency.simMutex);
}

static inline void LiveDataDraw(WorkspaceState *state, Rectangle rec, size_t startIdx, size_t visibleCount) {
    if (!state) return;

    float x = rec.x, y = rec.y, w = rec.width, h = rec.height;
    float halfW = (w - DETAIL_GAP) / 2.0f, halfH = (h - DETAIL_GAP) / 2.0f;

    Rectangle recTotal    = {x, y, w, h};
    Rectangle recLeft     = {x, y, halfW, h};

    Rectangle recBotLeft  = {x, y + halfH + DETAIL_GAP, halfW, halfH};
    Rectangle recBotRight = {x + halfW + DETAIL_GAP, y + halfH + DETAIL_GAP, halfW, halfH};
    Rectangle recTopLeft  = {x, y, halfW, halfH};
    Rectangle recTopRight = {x + halfW + DETAIL_GAP, y, halfW, halfH};

    bool hasSynapse = (NetworkGetTotalSynapse(state->network) > 0);
    NetworkType type = NetworkDetectType(state->network);

    switch (type) {
        case NET_TYPE_PURE_IZ: {
            TracePlotDraw(state, (hasSynapse) ? recTopLeft : recLeft,     SLOT_SIMULATOR_VOLTAGE,     startIdx, visibleCount);
            TracePlotDraw(state, (hasSynapse) ? recBotLeft : recBotRight, SLOT_SIMULATOR_IZ_RECOVERY, startIdx, visibleCount);

            if (hasSynapse) {
                TracePlotDraw(state, recTopRight, SLOT_SIMULATOR_NT_FRACTION, startIdx, visibleCount);
                TracePlotDraw(state, recBotRight, SLOT_SIMULATOR_SYNAPTIC,    startIdx, visibleCount);
            }
            else {
                TracePlotDraw(state, recTopRight, SLOT_SIMULATOR_PHASE, startIdx, visibleCount);
            }
        } break;

        case NET_TYPE_PURE_HH: {
            TracePlotDraw(state, recTopLeft, SLOT_SIMULATOR_VOLTAGE, startIdx, visibleCount);
            TracePlotDraw(state, recBotLeft, SLOT_SIMULATOR_PHASE,   startIdx, visibleCount);

            if (hasSynapse) {
                TracePlotDraw(state, recTopRight, SLOT_SIMULATOR_NT_FRACTION, startIdx, visibleCount);
                TracePlotDraw(state, recBotRight, SLOT_SIMULATOR_SYNAPTIC,    startIdx, visibleCount);
            }
            else {
                TracePlotDraw(state, recTopRight, SLOT_SIMULATOR_GATE,    startIdx, visibleCount);
                TracePlotDraw(state, recBotRight, SLOT_SIMULATOR_CURRENT, startIdx, visibleCount);
            }
        } break;

        case NET_TYPE_HYBRID: {
            TracePlotDraw(state, (hasSynapse) ? recLeft : recTotal, SLOT_SIMULATOR_VOLTAGE, startIdx, visibleCount);

            if (hasSynapse) {
                TracePlotDraw(state, recTopRight, SLOT_SIMULATOR_NT_FRACTION, startIdx, visibleCount);
                TracePlotDraw(state, recBotRight, SLOT_SIMULATOR_SYNAPTIC,    startIdx, visibleCount);
            }
        } break;

        default: break;
    }
}

static inline void TracePlotDraw(WorkspaceState *state, Rectangle layout, int slot, size_t startIdx, size_t visibleCount) {
    if (!state || visibleCount == 0) return;

    PlotView *v = PlotStateGetView(slot);
    if (!v) return;

    NetworkType netType = NetworkDetectType(state->network);
    const char *xLabel = (slot == SLOT_SIMULATOR_PHASE) ? WS_STR_GRAPH_POTENTIAL_MV : WS_STR_GRAPH_TIME_MS;
    const char *yLabel = WorkspaceSlotToYLabel(slot, netType);
    PlotMarginMode marginMode = (slot == SLOT_SIMULATOR_PHASE) ? MARGIN_MODE_BOTH : MARGIN_MODE_Y;
    size_t nrnCount = state->network->neuronCount;

    // ---------------------------------------------------------------------
    // CASOS ESPECÍFICOS DOS COMPONENTES HODGKIN-HUXLEY (Multi-camadas)
    // ---------------------------------------------------------------------
    if (slot == SLOT_SIMULATOR_CURRENT || slot == SLOT_SIMULATOR_GATE) {
        NeuroTraceBuffer *buf = &state->liveData;

        for (size_t i = 0; i < nrnCount; i++) {
            size_t layerCount = 3;
            Vector2d *layers[3];

            if (slot == SLOT_SIMULATOR_CURRENT) {
                layers[0] = buf->hhCurrentPlots.kCurrent[i];
                layers[1] = buf->hhCurrentPlots.naCurrent[i];
                layers[2] = buf->hhCurrentPlots.leakCurrent[i];
            } else {
                layers[0] = buf->hhGatePlots.MGate[i];
                layers[1] = buf->hhGatePlots.HGate[i];
                layers[2] = buf->hhGatePlots.NGate[i];
            }

            PlotMultiLineSameColorDraw(
                layout,
                (i == 0) ? xLabel : NULL,
                (i == 0) ? yLabel : NULL,
                v->xMin, v->xMax, v->yMin, v->yMax, MARGIN_MODE_Y,
                startIdx, visibleCount, layerCount, i, layers
            );
        }
        return;
    }

    // ---------------------------------------------------------------------
    // CASOS GENÉRICOS (PlotMultiLineDraw Padrão)
    // ---------------------------------------------------------------------
    void *traces = NULL;

    switch (slot) {
        case SLOT_SIMULATOR_IZ_RECOVERY: traces = state->liveData.izRecoveryTraces; break;
        case SLOT_SIMULATOR_PHASE:       traces = state->liveData.phaseSpaceTraces; break;
        case SLOT_SIMULATOR_NT_FRACTION: traces = state->liveData.ntFractionTraces; break;
        case SLOT_SIMULATOR_SYNAPTIC:    traces = state->liveData.synapticTraces; break;
        case SLOT_SIMULATOR_VOLTAGE:     traces = state->liveData.neuronTraces; break;
        default: return;
    }

    PlotMultiLineDraw(
        layout, xLabel, yLabel, v->xMin, v->xMax, v->yMin, v->yMax,
        marginMode, startIdx, visibleCount, nrnCount, traces
    );
}
