#ifndef GRAPH_PLOT_H
#define GRAPH_PLOT_H

#include <stddef.h>

#include "raylib.h"

#include "graph/config/graph_enums.h"
#include "utils/struct_data.h"

void PlotMultiLineDraw(
    Rectangle area,
    const char *xLab,
    const char *yLab,
    double xMin,
    double xMax,
    double yMin,
    double yMax,
    PlotMarginMode marginMode,
    size_t startIdx,
    size_t visibleCount,
    size_t multiCount,
    Vector2d **multiData
);

void PlotMultiLineSameColorDraw(
    Rectangle area,
    const char *xLab,
    const char *yLab,
    double xMin,
    double xMax,
    double yMin,
    double yMax,
    PlotMarginMode marginMode,
    size_t startIdx,
    size_t visibleCount,
    size_t multiCount,
    size_t baseColorIndex,
    Vector2d **multiData
);

void PlotRasterDraw(
    Rectangle area,
    const char *xLab,
    const char *yLab,
    double xMin,
    double xMax,
    double yMin,
    double yMax,
    PlotMarginMode marginMode,
    size_t multiCount,
    size_t *counts,
    Vector2d **multiData
);

void PlotScanLinesDraw(
    Rectangle area,
    double xMin,
    double xMax,
    double scanTime,
    Color color
);

void PlotScatterDraw(
    Rectangle area,
    const char *xLab,
    const char *yLab,
    double xMin,
    double xMax,
    double yMin,
    double yMax,
    PlotMarginMode marginMode,
    size_t startIdx,
    size_t visibleCount,
    Vector2d *data
);

void PlotSerieDraw(
    Rectangle area,
    const char *xLab,
    const char *yLab,
    double xMin,
    double xMax,
    double yMin,
    double yMax,
    PlotMarginMode marginMode,
    size_t startIdx,
    size_t visibleCount,
    Vector2d *data
);

void PlotSingleLineDraw(
    Rectangle area,
    const char *xLab,
    const char *yLab,
    double xMin,
    double xMax,
    double yMin,
    double yMax,
    PlotMarginMode marginMode,
    size_t startIdx,
    size_t visibleCount,
    Vector2d *data
);

#endif // GRAPH_PLOT_H
