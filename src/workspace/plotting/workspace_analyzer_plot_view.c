#include "stdio.h"
#include "string.h"

#include "graph/core/graph_state.h"
#include "graph/ui/graph_plot.h"

#include "neuro_data/neuro_data_state.h"

#include "workspace/plotting/workspace_plot_enums.h"
#include "workspace/config/workspace_ui_config.h"
#include "workspace/storage/workspace_storage.h"
#include "workspace/strings/workspace_strings.h"
#include "workspace/plotting/workspace_plot_view.h"
#include "workspace/plotting/workspace_plot_logic.h"

// =========================================================================
// PROTÓTIPOS DAS FUNÇÕES INTERNAS DE DESENHO
// =========================================================================

static inline void WorkspaceMultiLinePlotDraw(
    WorkspaceState *state,
    Rectangle layout,
    const char *yLabel,
    int slot,
    size_t totalCount,
    Vector2d **data2D
);

static inline void WorkspaceRasterPlotDraw(WorkspaceState *state, Rectangle layout, int slot);

static inline void WorkspaceTauPlotDraw(
    WorkspaceState *state,
    Rectangle layout,
    size_t id1,
    size_t id2,
    int slot
);

void WorkspaceStaticPlotsDraw(WorkspaceState *state, Rectangle layout) {
    if (!state || state->session.activeTableId == TABLE_ID_UNKNOWN) return;

    size_t selectedNrn1 = state->editor.sourceNeuronId;
    size_t selectedNrn2 = state->editor.targetNeuronId;
    size_t idx = (selectedNrn1 * MAX_DETAILED_PLOTS) + selectedNrn2;

    bool hasTreatmentData = (selectedNrn1 < MAX_DETAILED_PLOTS && selectedNrn2 < MAX_DETAILED_PLOTS)
                          ? (state->staticData.tauPointsCount[idx] > 0) : false;

    bool hasRasterData = (state->session.loadedNeuronCount > 0)
                       ? (state->staticData.rasterPointsCount[0] > 0) : false;

    // --- MODO TELA CHEIA ---
    if (state->session.activeTableId == TABLE_ID_TAU) {
        if (hasTreatmentData) WorkspaceTauPlotDraw(state, layout, selectedNrn1, selectedNrn2, SLOT_ANALYZER_TAU);
        return;
    }

    if (state->session.activeTableId == TABLE_ID_RASTER) {
        if (hasRasterData) WorkspaceRasterPlotDraw(state, layout, WorkspaceTableToSlot(TABLE_ID_RASTER));
        return;
    }

    if (state->staticData.dataCount == 0) return;

    int baseSlot = WorkspaceTableToSlot(state->session.activeTableId);
    if (baseSlot == -1) return;

    // --- MODO SPLIT ---
    Rectangle topLayout = layout;
    Rectangle bottomLayout = {0};
    bool needsSplit = (hasTreatmentData || hasRasterData);

    if (needsSplit) {
        float halfHeight = (layout.height - DETAIL_GAP) / 2.0f;
        topLayout = (Rectangle){layout.x, layout.y, layout.width, halfHeight};
        bottomLayout = (Rectangle){layout.x, layout.y + halfHeight + DETAIL_GAP, layout.width, halfHeight};
    }

    // --- PLOT 1: DADOS ESTÁTICOS PADRÃO ---
    size_t staticPoints = state->staticData.dataCount;
    Vector2d **staticData2D = WorkspaceTraceArrayGet(&state->staticData, state->session.activeTableId);

    if (staticData2D && staticPoints > 0) {
        const char *yLabel = WorkspaceTableToYLabel(state->session.activeTableId);
        WorkspaceMultiLinePlotDraw(state, topLayout, yLabel, baseSlot, staticPoints, staticData2D);
    }

    // --- PLOT 2: DADOS SECUNDÁRIOS ---
    if (hasTreatmentData) {
        WorkspaceTauPlotDraw(state, bottomLayout, selectedNrn1, selectedNrn2, SLOT_ANALYZER_TAU);
    } else if (hasRasterData) {
        WorkspaceRasterPlotDraw(state, bottomLayout, WorkspaceTableToSlot(TABLE_ID_RASTER));
    }
}

static inline void WorkspaceMultiLinePlotDraw(
    WorkspaceState *state, Rectangle layout, const char *yLabel, int slot,
    size_t totalCount, Vector2d **data2D
) {
    if (!data2D || totalCount == 0) return;

    size_t visStartIdx = 0, visEndIdx = 0, decimateStep = 1;

    WorkspacePlotMultiLinePrepareRange(
        state, slot, data2D, totalCount,
        &visStartIdx, &visEndIdx, &decimateStep
    );

    PlotView *v = PlotStateGetView(slot);
    if (!v) return;

    static bool ptrsInitialized = false;
    static Vector2d renderBuffer2D[MAX_DETAILED_PLOTS][MAX_RENDER_POINTS];
    static Vector2d *renderBuffer2DPtrs[MAX_DETAILED_PLOTS];

    size_t neurons = state->session.loadedNeuronCount;
    if (neurons > MAX_DETAILED_PLOTS) neurons = MAX_DETAILED_PLOTS;

    if (!ptrsInitialized) {
        for (size_t n = 0; n < MAX_DETAILED_PLOTS; n++) renderBuffer2DPtrs[n] = renderBuffer2D[n];
        ptrsInitialized = true;
    }

    size_t decimateCount = 0;
    size_t safeLimit = MAX_RENDER_POINTS - 1;

    pthread_mutex_lock(&state->concurrency.anaMutex);
        for (size_t j = visStartIdx; j <= visEndIdx && decimateCount < safeLimit; j += decimateStep) {
            for (size_t n = 0; n < neurons; n++) {
                if (data2D[n]) renderBuffer2D[n][decimateCount] = data2D[n][j];
            }
            decimateCount++;
        }
    pthread_mutex_unlock(&state->concurrency.anaMutex);

    PlotMultiLineDraw(
        layout, WS_STR_GRAPH_TIME_MS, yLabel,
        v->xMin, v->xMax, v->yMin, v->yMax,
        MARGIN_MODE_Y, 0, decimateCount, neurons, renderBuffer2DPtrs
    );
}

static inline void WorkspaceRasterPlotDraw(WorkspaceState *state, Rectangle layout, int slot) {
    if (!state) return;

    size_t neurons = state->session.loadedNeuronCount;
    if (neurons == 0) return;

    WorkspacePlotRasterBoundsApply(state, slot);

    PlotView *v = PlotStateGetView(slot);
    if (!v) return;

    PlotRasterDraw(
        layout, WS_STR_GRAPH_TIME_MS, "Neuron ID",
        v->xMin, v->xMax, v->yMin, v->yMax,
        MARGIN_MODE_Y, neurons,
        state->staticData.rasterPointsCount,
        state->staticData.rasterTraces
    );
}

static inline void WorkspaceTauPlotDraw(WorkspaceState *state, Rectangle layout, size_t id1, size_t id2, int slot) {
    if (!state || id1 >= MAX_DETAILED_PLOTS || id2 >= MAX_DETAILED_PLOTS) return;

    size_t matrixIdx = (id1 * MAX_DETAILED_PLOTS) + id2;
    size_t count = state->staticData.tauPointsCount[matrixIdx];

    if (count == 0) return;

    Vector2d *tauMatrixPtr = state->staticData.tauTraces[matrixIdx];
    if (!tauMatrixPtr) return;

    WorkspacePlotTreatmentBoundsApply(state);

    PlotView *v = PlotStateGetView(slot);
    if (!v) return;

    size_t pointsToShow = state->ui.pointsToShow;
    size_t visibleCount = (count > pointsToShow) ? pointsToShow : count;

    char titleBuffer[64];
    snprintf(titleBuffer, sizeof(titleBuffer), "Tau Matrix (%zu -> %zu)", id1, id2);

    PlotSerieDraw(
        layout, titleBuffer, "Tau Value (ms)",
        v->xMin, v->xMax, v->yMin, v->yMax,
        MARGIN_MODE_Y, 0, visibleCount, tauMatrixPtr
    );
}
