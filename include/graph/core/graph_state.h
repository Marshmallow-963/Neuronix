/**
 * @file plot_state.h
 * @brief Defines the state structure for plot axis boundaries.
 *
 * This tracks the min/max values for the X and Y axes of all
 * graphs, which is essential for both manual and auto-scaling.
 */

#ifndef GRAPH_STATE_H
#define GRAPH_STATE_H

#include <stddef.h>
#include "graph/config/graph_config.h"
#include "utils/struct_data.h"

typedef struct {
    double xMin; double xMax;
    double yMin; double yMax;
} PlotView;

typedef struct {
    PlotView views[MAX_PLOT_VIEWS];
} PlotState;

// Estado global genérico
extern PlotState PLOT_STATE;

/**
 * @brief Retorna o ponteiro para a visualização em um slot específico.
 * @param index Índice do gráfico (0 até MAX_PLOT_VIEWS - 1)
 */
PlotView *PlotStateGetView(int index);

/**
 * @brief Analisa os dados e expande os limites do PlotView correspondente.
 */
void PlotStateUpdate(
    const Vector2d *data,
    int idxx,
    size_t startIdx,
    size_t visibleCount,
    size_t capacity
);

void PlotStateUpdateMulti(
    Vector2d **multiData,
    int idx,
    size_t startIdx,
    size_t visibleCount,
    size_t multiCount,
    size_t capacity
);

/**
 * @brief Reseta os limites de um slot específico ou de todos.
 */
void PlotStateResetSlot(int index);
void PlotStateResetAll(void);

#endif // GRAPH_STATE_H
