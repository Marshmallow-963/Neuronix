/**
 * @file plot_state.c
 * @brief Implements the plot state logic, including the reset function.
 */

#include "graph/core/graph_state.h"
#include "graph/config/graph_config.h"

PlotState G_PLOT_STATE;

PlotView *PlotStateGetView(int idx) {
    if (idx >= 0 && idx < MAX_PLOT_VIEWS) return &G_PLOT_STATE.views[idx];
    return &G_PLOT_STATE.views[0];
}

void PlotStateUpdate(const Vector2d *data, int index, size_t startIdx, size_t visibleCount, size_t capacity) {
    if (!data || visibleCount == 0) return;
    if (index < 0 || index >= MAX_PLOT_VIEWS) return;

    PlotView *view = &G_PLOT_STATE.views[index];

    for (size_t i = 0; i < visibleCount; i++) {
        // Se a capacidade for maior que 0 (circular), usa módulo para dar a volta.
        // Caso contrário (linear/estático), acessa diretamente.
        size_t actualIdx = (capacity > 0) ? ((startIdx + i) % capacity) : (startIdx + i);

        if (data[actualIdx].x < view->xMin) view->xMin = data[actualIdx].x;
        if (data[actualIdx].x > view->xMax) view->xMax = data[actualIdx].x;
        if (data[actualIdx].y < view->yMin) view->yMin = data[actualIdx].y;
        if (data[actualIdx].y > view->yMax) view->yMax = data[actualIdx].y;
    }
}

void PlotStateUpdateMulti(Vector2d **multiData, int index, size_t startIdx, size_t visibleCount, size_t multiCount, size_t capacity) {
    if (!multiData || multiCount == 0 || visibleCount == 0) return;

    for (size_t i = 0; i < multiCount; i++) {
        if (multiData[i] == NULL) continue;
        PlotStateUpdate(multiData[i], index, startIdx, visibleCount, capacity);
    }
}

void PlotStateResetSlot(int index) {
    if (index < 0 || index >= MAX_PLOT_VIEWS) return;

    G_PLOT_STATE.views[index].xMin = BOUND_MAX;
    G_PLOT_STATE.views[index].xMax = -BOUND_MAX;
    G_PLOT_STATE.views[index].yMin = BOUND_MAX;
    G_PLOT_STATE.views[index].yMax = -BOUND_MAX;
}

void PlotStateResetAll(void) {
    for (int i = 0; i < MAX_PLOT_VIEWS; i++) PlotStateResetSlot(i);
}
