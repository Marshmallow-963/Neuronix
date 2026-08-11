/**
 * @file gui_plot.c
 * @brief Implementation of the custom plotting widget.
 */

#include "graph/ui/graph_plot.h"
#include "graph/ui/graph_themes.h"

#include "rlgl.h"

/* ----------------------------------------------------------------------------
 * ---------------------------- FORWARD DECLARATION ---------------------------
 * ------------------------------------------------------------------------- */

static inline void PlotAxisLabelsDraw(
    Rectangle plotRect,
    const char *xLabel,
    const char *yLabel,
    double xMin,
    double xMax,
    double yMin,
    double yMax
);

static inline void PlotCoreDraw(
    Rectangle area,
    const char *xLab,
    const char *yLab,
    double xMin,
    double xMax,
    double yMin,
    double yMax,
    PlotStyle style,
    PlotMarginMode margin,
    size_t startIdx,
    size_t visibleCount,
    size_t multiCount,
    Vector2d *data,
    Vector2d **multiData
);

static inline void PlotGridLinesDraw(
    Rectangle plotRect,
    double xMin,
    double xMax,
    double yMin,
    double yMax
);

static inline void PlotLineDraw(
    Rectangle area,
    double xMin,
    double xMax,
    double yMin,
    double yMax,
    size_t startIdx,
    size_t visibleCount,
    size_t index,
    Vector2d *data
);

static inline void PlotPointsDraw(
    Rectangle area,
    double xMin,
    double xMax,
    double yMin,
    double yMax,
    size_t startIdx,
    size_t visibleCount,
    size_t index,
    Vector2d *data
);

/* ----------------------------------------------------------------------------
 * ------------------------------ MAIN FUNCTIONS ------------------------------
 * ------------------------------------------------------------------------- */

void PlotMultiLineDraw(
    Rectangle area, const char *xLab, const char *yLab,
    double xMin, double xMax, double yMin, double yMax,
    PlotMarginMode marginMode, size_t startIdx, size_t visibleCount,
    size_t multiCount, Vector2d **multiData
) {
    PlotCoreDraw(
        area, xLab, yLab, xMin, xMax, yMin, yMax, PLOT_STYLE_LINE,
        marginMode, startIdx, visibleCount, multiCount, NULL, multiData
    );
}

void PlotMultiLineSameColorDraw(
    Rectangle area, const char *xLab, const char *yLab,
    double xMin, double xMax, double yMin, double yMax,
    PlotMarginMode marginMode, size_t startIdx, size_t visibleCount,
    size_t multiCount, size_t baseColorIndex, Vector2d **multiData
) {
    if (visibleCount < 1 && multiCount < 1) return;

    if ((xMax - xMin) < EPSILON_ZERO) { xMin = 0.0f;  xMax = 1.0f; }
    if ((yMax - yMin) < EPSILON_ZERO) { yMin = -1.0f; yMax = 1.0f; }

    bool adjustX = marginMode == MARGIN_MODE_X || marginMode == MARGIN_MODE_BOTH;
    bool adjustY = marginMode == MARGIN_MODE_Y || marginMode == MARGIN_MODE_BOTH;

    double xMargin = (adjustX) ? (xMax - xMin) * MARGIN_PERCENT : 0.0f;
    double yMargin = (adjustY) ? (yMax - yMin) * MARGIN_PERCENT : 0.0f;

    double finalXMin = xMin - xMargin, finalXMax = xMax + xMargin;
    double finalYMin = yMin - yMargin, finalYMax = yMax + yMargin;

    Rectangle plotRect = {
        area.x + PLOT_AXIS_MARGIN,
        area.y + PLOT_AXIS_MARGIN,
        area.width - PLOT_AXIS_MARGIN * PLOT_WIDTH_MARGIN_MULT,
        area.height - PLOT_AXIS_MARGIN * PLOT_HEIGHT_MARGIN_MULT
    };

    BeginScissorMode((int)area.x, (int)area.y, (int)area.width, (int)area.height);
        DrawRectangleRec(area, Fade(BLACK, 0.2f));

        PlotGridLinesDraw(plotRect, xMin, xMax, yMin, yMax);

        if (multiData) {
            for (size_t i = 0; i < multiCount; i++) {
                PlotLineDraw(
                    area, finalXMin, finalXMax, finalYMin, finalYMax,
                    startIdx, visibleCount, baseColorIndex, multiData[i]
                );
            }
        }

        PlotAxisLabelsDraw(plotRect, xLab, yLab, xMin, xMax, yMin, yMax);
    EndScissorMode();

    DrawRectangleLinesEx(area, 1.0f, DARKGRAY);
}

void PlotRasterDraw(
    Rectangle area, const char *xLab, const char *yLab,
    double xMin, double xMax, double yMin, double yMax,
    PlotMarginMode marginMode, size_t multiCount,
    size_t *counts, Vector2d **multiData
) {
    if (multiCount < 1 || !multiData || !counts) return;

    if ((xMax - xMin) < EPSILON_ZERO) { xMin = 0.0f;  xMax = 1.0f; }
    if ((yMax - yMin) < EPSILON_ZERO) { yMin = -1.0f; yMax = 1.0f; }

    bool adjustX = marginMode == MARGIN_MODE_X || marginMode == MARGIN_MODE_BOTH;
    bool adjustY = marginMode == MARGIN_MODE_Y || marginMode == MARGIN_MODE_BOTH;

    double xMargin = (adjustX) ? (xMax - xMin) * MARGIN_PERCENT : 0.0f;
    double yMargin = (adjustY) ? (yMax - yMin) * MARGIN_PERCENT : 0.0f;

    double finalXMin = xMin - xMargin, finalXMax = xMax + xMargin;
    double finalYMin = yMin - yMargin, finalYMax = yMax + yMargin;

    Rectangle plotRect = {
        area.x + PLOT_AXIS_MARGIN,
        area.y + PLOT_AXIS_MARGIN,
        area.width - PLOT_AXIS_MARGIN * PLOT_WIDTH_MARGIN_MULT,
        area.height - PLOT_AXIS_MARGIN * PLOT_HEIGHT_MARGIN_MULT
    };

    BeginScissorMode((int)area.x, (int)area.y, (int)area.width, (int)area.height);
        // 1. Desenha o fundo e grade UMA ÚNICA VEZ
        DrawRectangleRec(area, Fade(BLACK, 0.2f));
        PlotGridLinesDraw(plotRect, xMin, xMax, yMin, yMax);

        // 2. Itera sobre os neurônios usando o tamanho (count) específico de cada um
        for (size_t i = 0; i < multiCount; i++) {
            if (counts[i] > 0 && multiData[i]) {
                PlotPointsDraw(
                    area, finalXMin, finalXMax, finalYMin, finalYMax,
                    0, counts[i], i, multiData[i]
                );
            }
        }

        // 3. Desenha os rótulos UMA ÚNICA VEZ
        PlotAxisLabelsDraw(plotRect, xLab, yLab, xMin, xMax, yMin, yMax);
    EndScissorMode();

    DrawRectangleLinesEx(area, 1.0f, DARKGRAY);
}

void PlotScanLinesDraw(Rectangle area, double xMin, double xMax, double scanTime, Color color) {
    if (scanTime < xMin || scanTime > xMax) return;

    Rectangle plotRec = {
        area.x + PLOT_AXIS_MARGIN,
        area.y + PLOT_AXIS_MARGIN,
        area.width - (PLOT_AXIS_MARGIN * PLOT_WIDTH_MARGIN_MULT),
        area.height - (PLOT_AXIS_MARGIN * PLOT_HEIGHT_MARGIN_MULT)
    };

    double xRange = (xMax - xMin);
    if (xRange <= 0) return;

    float screenX = plotRec.x + (float)((scanTime - xMin) / xRange) * plotRec.width;

    DrawLineEx(
        (Vector2){ screenX, plotRec.y },
        (Vector2){ screenX, plotRec.y + plotRec.height },
        2.0f, Fade(color, 0.6f)
    );
}

void PlotScatterDraw(
    Rectangle area, const char *xLab, const char *yLab,
    double xMin, double xMax, double yMin, double yMax,
    PlotMarginMode marginMode, size_t startIdx, size_t visibleCount, Vector2d *data
) {
    PlotCoreDraw(
        area, xLab, yLab, xMin, xMax, yMin, yMax,
        PLOT_STYLE_SCATTER, marginMode, startIdx, visibleCount, 0, data, NULL
    );
}

void PlotSerieDraw(
    Rectangle area, const char *xLab, const char *yLab,
    double xMin, double xMax, double yMin, double yMax,
    PlotMarginMode marginMode, size_t startIdx, size_t visibleCount, Vector2d *data
) {
    PlotCoreDraw(
        area, xLab, yLab, xMin, xMax, yMin, yMax, PLOT_STYLE_SERIES,
        marginMode, startIdx, visibleCount, 0, data, NULL
    );
}

void PlotSingleLineDraw(
    Rectangle area, const char *xLab, const char *yLab,
    double xMin, double xMax, double yMin, double yMax,
    PlotMarginMode marginMode, size_t startIdx, size_t visibleCount, Vector2d *data
) {
    PlotCoreDraw(
        area, xLab, yLab, xMin, xMax, yMin, yMax, PLOT_STYLE_LINE,
        marginMode, startIdx, visibleCount, 0, data, NULL
    );
}

// --- Internal Helper Implementation ---

static inline void PlotAxisLabelsDraw(
    Rectangle plotRect, const char *xLabel, const char *yLabel,
    double xMin, double xMax, double yMin, double yMax
) {
    // Renderiza todos os textos em sequência contígua. A Raylib agrupa tudo na mesma textura
    double xStep = (xMax - xMin) / (NUM_X_TICKS - 1);
    for (int i = 0; i < NUM_X_TICKS; i++) {
        double val = xMin + (double)i * xStep;
        float xPos = plotRect.x + (float)(((val - xMin) / (xMax - xMin)) * plotRect.width);

        DrawText(
            TextFormat("%.0f", val),
            (int)xPos + X_LABEL_OFFSET_X,
            (int)(plotRect.y + plotRect.height + X_LABEL_OFFSET_Y),
            AXIS_FONT_SIZE, THEME_AXIS_TEXT_COLOR
        );
    }

    double yStep = (yMax - yMin) / (NUM_Y_TICKS - 1);
    for (int i = 0; i < NUM_Y_TICKS; i++) {
        double val = yMin + (double)i * yStep;
        float yPos = plotRect.y + plotRect.height - (float)(((val - yMin) / (yMax - yMin)) * plotRect.height);

        DrawText(
            TextFormat("%.2f", val),
            (int)(plotRect.x + Y_LABEL_OFFSET_X),
            (int)yPos + Y_LABEL_OFFSET_Y,
            AXIS_FONT_SIZE, THEME_AXIS_TEXT_COLOR
        );
    }

    if (xLabel) DrawText(
        xLabel, (int)(plotRect.x + plotRect.width / 2 + X_TITLE_OFFSET_X),
        (int)(plotRect.y + plotRect.height + X_TITLE_OFFSET_Y),
        AXIS_FONT_SIZE, THEME_TITLE_TEXT_COLOR
    );

    if (yLabel) DrawText(
        yLabel, (int)(plotRect.x + Y_TITLE_OFFSET_X),
        (int)(plotRect.y + Y_TITLE_OFFSET_Y), AXIS_FONT_SIZE, THEME_TITLE_TEXT_COLOR
    );
}

static inline void PlotCoreDraw(
    Rectangle area, const char *xLab, const char *yLab, double xMin, double xMax,
    double yMin, double yMax, PlotStyle style, PlotMarginMode marginMode, size_t startIdx,
    size_t visibleCount, size_t multiCount, Vector2d *data, Vector2d **multiData
) {
    if (visibleCount < 1 && multiCount < 1) return;

    if ((xMax - xMin) < EPSILON_ZERO) { xMin = 0.0f;  xMax = 1.0f; }
    if ((yMax - yMin) < EPSILON_ZERO) { yMin = -1.0f; yMax = 1.0f; }

    bool adjustX = marginMode == MARGIN_MODE_X || marginMode == MARGIN_MODE_BOTH;
    bool adjustY = marginMode == MARGIN_MODE_Y || marginMode == MARGIN_MODE_BOTH;

    double xMargin = (adjustX) ? (xMax - xMin) * MARGIN_PERCENT : 0.0f;
    double yMargin = (adjustY) ? (yMax - yMin) * MARGIN_PERCENT : 0.0f;

    double finalXMin = xMin - xMargin, finalXMax = xMax + xMargin;
    double finalYMin = yMin - yMargin, finalYMax = yMax + yMargin;

    Rectangle plotRect = {
        area.x + PLOT_AXIS_MARGIN,
        area.y + PLOT_AXIS_MARGIN,
        area.width - PLOT_AXIS_MARGIN * PLOT_WIDTH_MARGIN_MULT,
        area.height - PLOT_AXIS_MARGIN * PLOT_HEIGHT_MARGIN_MULT
    };

    BeginScissorMode((int)area.x, (int)area.y, (int)area.width, (int)area.height);
        DrawRectangleRec(area, Fade(BLACK, 0.2f));
        PlotGridLinesDraw(plotRect, xMin, xMax, yMin, yMax);

        if (data) {
            if (style == PLOT_STYLE_LINE || style == PLOT_STYLE_SERIES) {
                PlotLineDraw(
                    area, finalXMin, finalXMax, finalYMin, finalYMax,
                    startIdx, visibleCount, SINGLE_INDEX, data
                );
            }

            if (style == PLOT_STYLE_SCATTER || style == PLOT_STYLE_SERIES) {
                PlotPointsDraw(
                    area, finalXMin, finalXMax, finalYMin, finalYMax,
                    startIdx, visibleCount, SINGLE_INDEX, data
                );
            }
        }
        else if (multiData) {
            for (size_t i = 0; i < multiCount; i++) {
                if (style == PLOT_STYLE_LINE || style == PLOT_STYLE_SERIES) {
                    PlotLineDraw(
                        area, finalXMin, finalXMax, finalYMin, finalYMax,
                        startIdx, visibleCount, i, multiData[i]
                    );
                }

                if (style == PLOT_STYLE_SCATTER || style == PLOT_STYLE_SERIES) {
                    PlotPointsDraw(
                        area, finalXMin, finalXMax, finalYMin, finalYMax,
                        startIdx, visibleCount, i, multiData[i]
                    );
                }
            }
        }
        PlotAxisLabelsDraw(plotRect, xLab, yLab, xMin, xMax, yMin, yMax);
    EndScissorMode();

    DrawRectangleLinesEx(area, 1.0f, DARKGRAY);
}

static inline void PlotGridLinesDraw(
    Rectangle plotRect, double xMin, double xMax, double yMin, double yMax
) {
    Color gridColor = Fade(THEME_GRID_COLOR, THEME_GRID_ALPHA);
    Color axisColor = THEME_AXIS_LINE_COLOR;

    rlBegin(RL_LINES);
        rlColor4ub(gridColor.r, gridColor.g, gridColor.b, gridColor.a);

        // Linhas Verticais da Grade
        double xStep = (xMax - xMin) / (NUM_X_TICKS - 1);
        for (int i = 0; i < NUM_X_TICKS; i++) {
            double val = xMin + (double)i * xStep;
            float xPos = plotRect.x + (float)(((val - xMin) / (xMax - xMin)) * plotRect.width);

            rlVertex2f(xPos, plotRect.y);
            rlVertex2f(xPos, plotRect.y + plotRect.height);
        }

        // Linhas Horizontais da Grade
        double yStep = (yMax - yMin) / (NUM_Y_TICKS - 1);
        for (int i = 0; i < NUM_Y_TICKS; i++) {
            double val = yMin + (double)i * yStep;
            float yPos = plotRect.y + plotRect.height - (float)(((val - yMin) / (yMax - yMin)) * plotRect.height);

            rlVertex2f(plotRect.x, yPos);
            rlVertex2f(plotRect.x + plotRect.width, yPos);
        }

        // Bordas e Contornos dos Eixos estruturais
        rlColor4ub(axisColor.r, axisColor.g, axisColor.b, axisColor.a);
        rlVertex2f(plotRect.x, plotRect.y);
        rlVertex2f(plotRect.x, plotRect.y + plotRect.height);
        rlVertex2f(plotRect.x, plotRect.y + plotRect.height);
        rlVertex2f(plotRect.x + plotRect.width, plotRect.y + plotRect.height);
    rlEnd();
}

static inline void PlotLineDraw(
    Rectangle area, double xMin, double xMax, double yMin, double yMax,
    size_t startIdx, size_t visibleCount, size_t index, Vector2d *data
) {
    if (data == NULL || visibleCount < 2) return;

    Color color = PLOT_PALETTE[index % PALETTE_SIZE];

    Rectangle plotRec = {
        area.x + PLOT_AXIS_MARGIN, area.y + PLOT_AXIS_MARGIN,
        area.width - (PLOT_AXIS_MARGIN * PLOT_WIDTH_MARGIN_MULT),
        area.height - (PLOT_AXIS_MARGIN * PLOT_HEIGHT_MARGIN_MULT)
    };

    double xRange = (xMax - xMin); if (xRange == 0.0) xRange = MIN_AXIS_RANGE;
    double yRange = (yMax - yMin); if (yRange == 0.0) yRange = MIN_AXIS_RANGE;

    float scaleX = (float)(plotRec.width / xRange);
    float scaleY = (float)(plotRec.height / yRange);

    float baseX = plotRec.x;
    float baseY = plotRec.y + plotRec.height; // Y na tela cresce para baixo

    size_t end = startIdx + visibleCount;

    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        float prevX = baseX + (float)(data[startIdx].x - xMin) * scaleX;
        float prevY = baseY - (float)(data[startIdx].y - yMin) * scaleY;

        for (size_t i = startIdx + 1; i < end; i++) {
            float currX = baseX + (float)(data[i].x - xMin) * scaleX;
            float currY = baseY - (float)(data[i].y - yMin) * scaleY;

            if ((int)currX == (int)prevX && (int)currY == (int)prevY) continue;

            rlVertex2f(prevX, prevY);
            rlVertex2f(currX, currY);

            prevX = currX;
            prevY = currY;
        }
    rlEnd();
}

static inline void PlotPointsDraw(
    Rectangle area, double xMin, double xMax, double yMin, double yMax,
    size_t startIdx, size_t visibleCount, size_t index, Vector2d *data
) {
    if (data == NULL || visibleCount == 0) return;

    Color color = PLOT_PALETTE[index % PALETTE_SIZE];

    Rectangle plotRec = {
        area.x + PLOT_AXIS_MARGIN,
        area.y + PLOT_AXIS_MARGIN,
        area.width - (PLOT_AXIS_MARGIN * PLOT_WIDTH_MARGIN_MULT),
        area.height - (PLOT_AXIS_MARGIN * PLOT_HEIGHT_MARGIN_MULT)
    };

    double xRange = (xMax - xMin); if (xRange == 0.0) xRange = MIN_AXIS_RANGE;
    double yRange = (yMax - yMin); if (yRange == 0.0) yRange = MIN_AXIS_RANGE;

    float scaleX = (float)(plotRec.width / xRange);
    float scaleY = (float)(plotRec.height / yRange);

    float baseX = plotRec.x;
    float baseY = plotRec.y + plotRec.height;

    float pointSize = (yMax > DENSITY_THRESHOLD) ? DENSITY_SIZE_DENSE : DENSITY_SIZE_NORMAL;
    bool isPixel = (pointSize <= 1.0f);
    float size = isPixel ? 0.5f : pointSize;

    size_t end = startIdx + visibleCount;

    if (isPixel) {
        int lastX = -1, lastY = -1;
        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (size_t i = startIdx; i < end; i++) {
                float cx = baseX + (float)(data[i].x - xMin) * scaleX;
                float cy = baseY - (float)(data[i].y - yMin) * scaleY;

                int iX = (int)cx; int iY = (int)cy;
                if (iX == lastX && iY == lastY) continue; // Otimização para alta densidade
                lastX = iX; lastY = iY;

                rlVertex2f(cx - size, cy - size);
                rlVertex2f(cx - size, cy + size);
                rlVertex2f(cx + size, cy + size);
                rlVertex2f(cx + size, cy - size);
            }
        rlEnd();
    }
    else {
        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (size_t i = startIdx; i < end; i++) {
                float cx = baseX + (float)(data[i].x - xMin) * scaleX;
                float cy = baseY - (float)(data[i].y - yMin) * scaleY;

                // Desenha os quadrados com o tamanho definido por DENSITY_SIZE_NORMAL
                rlVertex2f(cx - size, cy - size);
                rlVertex2f(cx - size, cy + size);
                rlVertex2f(cx + size, cy + size);
                rlVertex2f(cx + size, cy - size);
            }
        rlEnd();
    }
}
