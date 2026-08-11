#ifndef WORKSPACE_ENUMS_H
#define WORKSPACE_ENUMS_H

// ============================================================================
//  1. SIDEBAR MENUS (NAVIGATION)
// ============================================================================

typedef enum {
    MENU_NONE = 0,       ///< No sidebar menu is currently open.

    MENU_CONFIG,
    MENU_DATABASE,       ///< Database selection and loaded sessions.
    MENU_DATA_TRATAMENT, ///< Tau analysis and data manipulation.
    MENU_EDIT,           ///< Network editing tools (Add Neurons/Synapses).

    MENU_COUNT,
} WorkspaceMenuState;

// ============================================================================
//  2. VISUALIZATION MODES (SIMULATION VIEWS)
// ============================================================================


typedef enum {
    VIEW_GLOBAL_DETAILED_PLOT = 0,  ///< Individual rendering of selected neurons.
} WorkspaceGlobalViewMode;

/**
 * @brief Modes for sub-graphs and specific analytical plots.
 */
typedef enum {
    VIEW_MODE_TOPOLOGY = 0,
    VIEW_MODE_DASHBOARD,             ///< Default dashboard with multiple plot layouts.
    VIEW_MODE_NET_CONFIG,
} WorkspaceViewMode;

// ============================================================================
//  3. FOCUS SYSTEM - ANALYZER (UI INTERACTION)
// ============================================================================

typedef enum {
    FOCUS_CFG_NONE = 0,
    FOCUS_CFG_POINTS_TO_SHOW,
    FOCUS_CFG_OFFSET,
    FOCUS_CFG_TIME_WINDOW,
    FOCUS_CFG_STOP_SIM,
} FocusCfg;

typedef enum {
    FOCUS_DATA_NONE = 0,
    FOCUS_DATA_FIRING_PERIOD,
    FOCUS_DATA_RASTER,
    FOCUS_DATA_RETURN_MAP,
    FOCUS_DATA_SPIKE_TIMING,
    FOCUS_DATA_SYNC_REGIME,
} FocusDataTreatment;

typedef enum {
    SESSION_NONE = 0,
    SESSION_1,                         ///< First session block focus.
    SESSION_REFRESH                    ///< Refresh button/action focus.
} FocusSession;

// ============================================================================
//  4. FOCUS SYSTEM - SIMULATION (UI INTERACTION)
// ============================================================================

/**
 * @brief Focusable elements within the Network Edit menu.
 */
typedef enum {
    FOCUS_EDIT_NONE = 0,
    FOCUS_EDIT_ADD_NEURON,       ///< Action button to add a new neuron.
    FOCUS_EDIT_ADD_SYNAPSE,      ///< Action button to add a new synapse.
    FOCUS_EDIT_AUTAPSE_DELAY,    ///< Autapse delay adjustment slider.
    FOCUS_EDIT_CONDUCTANCY,      ///< Synaptic conductance slider.
    FOCUS_EDIT_DELETE,           ///< Clear network button.
    FOCUS_EDIT_SOURCE,
    FOCUS_EDIT_TARGET,
    FOCUS_EDIT_EXT_CURRENT,      ///< External stimulus current slider.
    FOCUS_EDIT_IZ_NEURON,        ///< Izhikevich type selection combo-box.
    FOCUS_EDIT_NEURON_MODEL,     ///< Neuron model selection combo-box.
    FOCUS_EDIT_SYN_MODEL,        ///< Synapse model selection combo-box.
} FocusEdit;

/**
 * @brief Focusable elements within the Visualization menu.
 */
typedef enum {
    FOCUS_VIEW_NONE = 0,
    FOCUS_VIEW_ADJ_MATRIX,
    FOCUS_VIEW_DASHBOARD,
    FOCUS_VIEW_HH_BIO_PARAMS,
    FOCUS_VIEW_NET_DETAILS,
    FOCUS_VIEW_PHASE_SPACE,
    FOCUS_VIEW_SYN_CURRENTS,
    FOCUS_VIEW_TOPOLOGY,
} FocusView;

// Enum para mapear as texturas no array
typedef enum {
    TEX_HH = 0,
    TEX_IZ_CH,
    TEX_IZ_FS,
    TEX_IZ_IB,
    TEX_IZ_LTS,
    TEX_IZ_RS,
    TEX_IZ_RE,
    TEX_IZ_TC
} NeuronTexID;

typedef enum {
    TABLE_ID_UNKNOWN = 0,
    TABLE_ID_RASTER,
    TABLE_ID_IZ_RECOVERY,
    TABLE_ID_HH_IK,
    TABLE_ID_HH_INA,
    TABLE_ID_HH_ILEAK,
    TABLE_ID_HH_GATE_M,
    TABLE_ID_HH_GATE_H,
    TABLE_ID_HH_GATE_N,
    TABLE_ID_NEUROTRANS,
    TABLE_ID_NET_NRN,
    TABLE_ID_NET_SYN,
    TABLE_ID_PHASE_SPACE,
    TABLE_ID_SYNAPTIC,
    TABLE_ID_TAU,
    TABLE_ID_VOLTAGE,
} TableId;

#endif // WORKSPACE_ENUMS_H
