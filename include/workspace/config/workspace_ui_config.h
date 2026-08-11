#ifndef WORKSPACE_UI_CONFIG_H
#define WORKSPACE_UI_CONFIG_H

// =========================================================================
// GENERAL THEME & LAYOUT (FONTS & SPACING)
// =========================================================================
#define BTN_FONTSIZE          10
#define LBL_FONTSIZE          10
#define TITLE_FONTSIZE        14
#define TXT_FONTSIZE          20

#define BIG_GAP               15.0f
#define DETAIL_GAP            2.0f
#define FOCUS_THICKNESS       2.0f
#define GAP                   5.0f
#define GROUP_SPACING         25.0f
#define LINE_SPACING          20.0f
#define NET_CFG_LINE_SPACING  16.0f
#define NET_CFG_SECTION_GAP   12.0f
#define PADDING               10.0f
#define PANEL_ALPHA           0.5f

// =========================================================================
// LIVE VIEW PROPORTIONS (SIMULATION)
// =========================================================================
#define PLOT_GRID_THRESHOLD 16
#define PLOT_MATRIX_RATIO   0.65f

// =========================================================================
// PLOT & VIEW MODIFIERS (ANALYZER)
// =========================================================================
#define MAX_RENDER_POINTS  8192
#define TAU_ZOOM_OFFSET    100
#define VIEW_MARGIN_FACTOR 0.2f

// =========================================================================
// SIMULATION: NETWORK TOPOLOGY GRAPH VIEW
// =========================================================================
#define TOPOLOGY_TARGET_NEURON_SIZE       48.0f
#define TOPOLOGY_NEURON_LABEL_FONT_SIZE   10
#define TOPOLOGY_NEURON_LABEL_OFFSET_Y    12
#define TOPOLOGY_VOLTAGE_TRACE_SCALE      0.15f
#define TOPOLOGY_VOLTAGE_TRACE_MAX_OFFSET 12.0f

#define TOPOLOGY_AUTAPSE_BASE_ANGLE       (-PI / 2.0f)
#define TOPOLOGY_AUTAPSE_ANGLE_STEP       0.4f
#define TOPOLOGY_AUTAPSE_OFFSET           10.0f
#define TOPOLOGY_AUTAPSE_LOOP_BASE_HEIGHT 35.0f
#define TOPOLOGY_AUTAPSE_LOOP_STEP_HEIGHT 10.0f

#define TOPOLOGY_SYNAPSE_BASE_ANGLE_OFFSET 0.20f
#define TOPOLOGY_SYNAPSE_ANGLE_STEP        0.30f
#define TOPOLOGY_SYNAPSE_BASE_CURVATURE    22.0f
#define TOPOLOGY_SYNAPSE_CURVE_INTENSITY   15.0f
#define TOPOLOGY_SYNAPSE_SEGMENTS          16
#define TOPOLOGY_SYNAPSE_LINE_THICKNESS    2.0f
#define TOPOLOGY_SYNAPSE_HITBOX_MARGIN     6
#define TOPOLOGY_SYNAPSE_ARROW_WIDTH       8.0f
#define TOPOLOGY_SYNAPSE_BALL_RADIUS       4.0f
#define TOPOLOGY_SYNAPSE_ALPHA             0.6f

#define TOPOLOGY_TOOLTIP_FONT_SIZE        10
#define TOPOLOGY_TOOLTIP_LINE_SPACING     4
#define TOPOLOGY_TOOLTIP_PADDING_X        12
#define TOPOLOGY_TOOLTIP_PADDING_Y        10
#define TOPOLOGY_TOOLTIP_MOUSE_OFFSET     15
#define TOPOLOGY_TOOLTIP_BOUNDS_MARGIN    5
#define TOPOLOGY_TOOLTIP_ALPHA            0.85f
#define TOPOLOGY_TOOLTIP_LINE_BUFFER_SIZE 64
#define TOPOLOGY_ID_BUFFER_SIZE           32

// =========================================================================
// SYSTEM INPUT DEFAULTS
// =========================================================================
#define DEFAULT_KEY_REPEAT_DELAY 0.4f
#define DEFAULT_KEY_REPEAT_TIMER 0.0f
#define DEFAULT_TIME_WINDOW      50

// =========================================================================
// UI CONTROLS & SLIDERS (SIMULATION)
// =========================================================================
#define MAX_AUTAPSE_DELAY 100.0f
#define MAX_CONDUCTANCE   500.0f
#define MAX_CURRENT       500.0f
#define MAX_DELAY         100.0f
#define MIN_NET_VALUE     1
#define MIN_TIME_WINDOW   20
#define MIN_VAL_FLOAT     0.0f
#define SLIDER_STEP       0.01f

// =========================================================================
// WIDGET DIMENSIONS & GEOMETRY
// =========================================================================
#define BTN_HEIGHT 30.0f
#define BTN_WIDTH  120.0f
#define LBL_HEIGHT 20.0f
#define TAB_HEIGHT 35.0f

#define X_ORIGIN         0
#define Y_ORIGIN         0
#define SIDEBAR_BTN_SIZE 40
#define SUBMENU_WIDTH    250

#endif // WORKSPACE_UI_CONFIG_H
