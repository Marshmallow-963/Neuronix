#ifndef GRAPH_CONFIG_H
#define GRAPH_CONFIG_H

#ifndef BOUND_MAX
#define BOUND_MAX 99999.99
#endif

#define DENSITY_SIZE_DENSE 2
#define DENSITY_SIZE_NORMAL 4
#define DENSITY_THRESHOLD 1000

#define EPSILON_ZERO 1e-15 // margem de erro de ponto flutuante

#define PALETTE_SIZE 8

// --- Layout e Geometria ---
#define MIN_AXIS_RANGE 1.0f

#define MARGIN_PERCENT 0.05f
#define MAX_PLOT_VIEWS 64
#define NUM_X_TICKS 10
#define NUM_Y_TICKS 10

#define PLOT_AXIS_MARGIN        40.0f
#define PLOT_BORDER_THICKNESS   1.0f
#define PLOT_HEIGHT_MARGIN_MULT 2.0f
#define PLOT_WIDTH_MARGIN_MULT  2.0f

#define SINGLE_INDEX 0

// --- Tipografia e Posicionamento (Offsets) ---
#define AXIS_FONT_SIZE 10

#define X_LABEL_OFFSET_X -10
#define X_LABEL_OFFSET_Y 5
#define Y_LABEL_OFFSET_X -35
#define Y_LABEL_OFFSET_Y -5

#define X_TITLE_OFFSET_X -20
#define X_TITLE_OFFSET_Y 20
#define Y_TITLE_OFFSET_X -20
#define Y_TITLE_OFFSET_Y -15

#endif // GRAPH_CONFIG_H
