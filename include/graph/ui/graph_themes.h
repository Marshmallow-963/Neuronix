#ifndef GRAPH_THEMES_H
#define GRAPH_THEMES_H

#include "raylib.h"
#include "graph/config/graph_config.h"

// --- Paleta de Dados ---
extern const Color PLOT_PALETTE[PALETTE_SIZE];

// --- Cores do Gráfico (Background, Eixos, Textos) ---
#define THEME_AXIS_LINE_COLOR WHITE
#define THEME_AXIS_TEXT_COLOR GRAY

#define THEME_GRID_COLOR LIGHTGRAY
#define THEME_GRID_ALPHA 0.15f

#define THEME_PLOT_BG_COLOR     BLACK
#define THEME_PLOT_BG_ALPHA     0.2f
#define THEME_PLOT_BORDER_COLOR DARKGRAY

#define THEME_TITLE_TEXT_COLOR WHITE

#endif // GRAPH_THEMES_H
