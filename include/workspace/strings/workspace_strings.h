#ifndef WORKSPACE_STRINGS_H
#define WORKSPACE_STRINGS_H

// =========================================================================
// TOOLTIPS (COMPILATION DEFINES)
// =========================================================================
#define WS_TIP_DATABASE        "Database"
#define WS_TIP_DATA_TRATAMENT  "Data Tratament"
#define WS_TIP_EDIT_NET        "Network Editor"
#define WS_TIP_SIM_CTRL        "Simulation Control"
#define WS_TIP_VISUALS         "Visualizations"

// =========================================================================
// WS SHARED STRINGS (GRAPHICS & PLOTS)
// =========================================================================
#define WS_STR_GRAPH_CURRENT_PA   "Current (pA)"
#define WS_STR_GRAPH_GATE_N       "Gate N"
#define WS_STR_GRAPH_LFP_MV       "Local Field Potential (mV)"
#define WS_STR_GRAPH_NEURON_ID    "Neuron ID"
#define WS_STR_GRAPH_POTENTIAL_MV "Potential (mV)"
#define WS_STR_GRAPH_PROB_0_1     "Probability"
#define WS_STR_GRAPH_R_CHANNEL    "R Channel"
#define WS_STR_GRAPH_RECOVERY_U   "Recovery (u)"
#define WS_STR_GRAPH_SYNAPTIC_PA  "Synaptic (pA)"
#define WS_STR_GRAPH_TIME_MS      "Time (ms)"
#define WS_STR_GRAPH_VARIABLE     "Variable"
#define WS_STR_GRAPH_Y_AXIS       "Y axis"

// =========================================================================
// ERRORS & LOGS
// =========================================================================
extern const char *WS_LOG_ERR_DB_NOT_CONNECTED;
extern const char *WS_LOG_ERR_DATA_INSERT;
extern const char *WS_LOG_ERR_NEURON_RANGE;
extern const char *WS_LOG_ERR_PREPARE_LOAD;
extern const char *WS_LOG_ERR_PREPARE_TRACE;
extern const char *WS_LOG_ERR_SESSION_CREATE;
extern const char *WS_LOG_ERR_SESSION_INSERT;
extern const char *WS_LOG_ERR_TAU_MALLOC;
extern const char *WS_LOG_ERR_TRANS_BEGIN;
extern const char *WS_LOG_ERR_TRANS_COMMIT;
extern const char *WS_LOG_INFO_ASYNC_ANALYSIS;
extern const char *WS_LOG_INFO_LOADED_DATA;
extern const char *WS_LOG_INFO_TAU_CALCULATED;
extern const char *WS_LOG_WARN_INSUFFICIENT_DATA;
extern const char *WS_LOG_WARN_NO_SPIKES_DETECTED;

// =========================================================================
// NET CONFIG STRINGS & FORMATS
// =========================================================================
extern const char *WS_STR_TITLE_NEURON_PARAMS;
extern const char *WS_STR_TITLE_SYNAPSE_PARAMS;

extern const char *WS_STR_HDR_IZ_MODELS;
extern const char *WS_STR_HDR_HH_MODELS;
extern const char *WS_STR_HDR_IZ_SYNAPSE;
extern const char *WS_STR_HDR_HH_SYNAPSE;

extern const char *WS_STR_FMT_IZ_PARAMS;
extern const char *WS_STR_FMT_HH_MEMBRANE;
extern const char *WS_STR_FMT_HH_SODIUM;
extern const char *WS_STR_FMT_HH_POTASSIUM;
extern const char *WS_STR_FMT_HH_LEAK;
extern const char *WS_STR_FMT_SYN_SIGMOID;
extern const char *WS_STR_FMT_SYN_AMPA;
extern const char *WS_STR_FMT_SYN_GABA;

extern const char *WS_STR_IZ_CHATTERING;
extern const char *WS_STR_IZ_FAST_SPIKING;
extern const char *WS_STR_IZ_INTRINSICALLY_BURSTING;
extern const char *WS_STR_IZ_LOW_THRESHOLD_SPIKING;
extern const char *WS_STR_IZ_REGULAR_SPIKING;
extern const char *WS_STR_IZ_RESONATOR;
extern const char *WS_STR_IZ_THALAMO_CORTICAL;

// =========================================================================
// STRINGS (UI)
// =========================================================================
extern const char *WS_STR_BTN_ADD_NEURON;
extern const char *WS_STR_BTN_ADD_SYNAPSE;
extern const char *WS_STR_BTN_ADJ_MATRIX;
extern const char *WS_STR_BTN_CLEAR_NET;
extern const char *WS_STR_BTN_DASHBOARD;
extern const char *WS_STR_BTN_REFRESH_LIST;
extern const char *WS_STR_BTN_RESET_SIM;
extern const char *WS_STR_BTN_SAVE_STATE;
extern const char *WS_STR_BTN_SHOW_BIO_PARAMS;
extern const char *WS_STR_BTN_SHOW_PHASE;
extern const char *WS_STR_BTN_SHOW_SYN_CUR;
extern const char *WS_STR_BTN_START_SIM;
extern const char *WS_STR_BTN_STOP_SIM;
extern const char *WS_STR_BTN_TAU_ANALYSIS;
extern const char *WS_STR_FMT_ADJ_DETAIL;
extern const char *WS_STR_FMT_ADJ_HEADER;
extern const char *WS_STR_FMT_SESSION_ENTRY;
extern const char *WS_STR_FMT_WEIGHT_VAL;
extern const char *WS_STR_LBL_AUTAPSE_DEL;
extern const char *WS_STR_LBL_CONDUCTANCE;
extern const char *WS_STR_LBL_EXT_CURRENT;
extern const char *WS_STR_LBL_INTERVAL_MS;
extern const char *WS_STR_LBL_IZ_TYPE;
extern const char *WS_STR_LBL_LARGE_NET_MODE;
extern const char *WS_STR_LBL_NET_CAPACITY;
extern const char *WS_STR_LBL_NET_TOPOLOGY;
extern const char *WS_STR_LBL_NO_SESSIONS;
extern const char *WS_STR_LBL_NRN_MODEL;
extern const char *WS_STR_LBL_PLAY_SPEED;
extern const char *WS_STR_LBL_SOURCE_ID;
extern const char *WS_STR_LBL_SYN_MODEL;
extern const char *WS_STR_LBL_TARGET_ID;
extern const char *WS_STR_LBL_ZOOM_OUT;
extern const char *WS_STR_MENU_TITLE_CFG;
extern const char *WS_STR_MENU_TITLE_EDIT;
extern const char *WS_STR_MENU_TITLE_EXTRA_PLOTS;
extern const char *WS_STR_MENU_TITLE_HH_PLOTS;
extern const char *WS_STR_MENU_TITLE_PARAMS;
extern const char *WS_STR_MENU_TITLE_SIM;
extern const char *WS_STR_MENU_TITLE_VIEW;
extern const char *WS_STR_MSG_INIT_OR_SELECT;
extern const char *WS_STR_MSG_LOADING_TABLE;
extern const char *WS_STR_MSG_NO_DATA_LOADED;
extern const char *WS_STR_MSG_NO_NOTES;
extern const char *WS_STR_MSG_SELECT_TABLE;
extern const char *WS_STR_OPT_AMPA;
extern const char *WS_STR_OPT_GABA;
extern const char *WS_STR_OPT_GLOBAL_VIEWS;
extern const char *WS_STR_OPT_IZ_MODELS;
extern const char *WS_STR_OPT_NO_CONN_FOUND;
extern const char *WS_STR_OPT_NRN_MODELS;
extern const char *WS_STR_OPT_PLAY_SPEEDS;
extern const char *WS_STR_OPT_SYN_MODELS;
extern const char *WS_STR_OPT_TOPOLOGY;
extern const char *WS_STR_OPT_ZOOM_OPTIONS;
extern const char *WS_STR_SEL_SOURCE_ID;
extern const char *WS_STR_SEL_TARGET_ID;
extern const char *WS_STR_TITLE_CONFIG;
extern const char *WS_STR_TITLE_DATA_TREATMENT;
extern const char *WS_STR_TITLE_SAVED_SESSIONS;
extern const char *WS_STR_TT_CONFIG;
extern const char *WS_STR_TT_DATABASE;
extern const char *WS_STR_TT_DATA_TREATMENT;
extern const char *WS_STR_TXT_CONN_DETAILS;
extern const char *WS_STR_TXT_MORE_HIDDEN;
extern const char *WS_STR_TXT_NO_NET_CREATED;
extern const char *WS_STR_TXT_WAIT_DATA;
extern const char *WS_STR_TXT_WAIT_SPIKES;

// =========================================================================
// TOPOLOGY TOOLTIPS & LABELS FORMATS
// =========================================================================
extern const char *WS_STR_FMT_NEURON_ID;
extern const char *WS_STR_FMT_TOOLTIP_NRN_INFO;
extern const char *WS_STR_FMT_TOOLTIP_I_EXT;
extern const char *WS_STR_FMT_TOOLTIP_VOLT;
extern const char *WS_STR_FMT_TOOLTIP_SYN_NODES;
extern const char *WS_STR_FMT_TOOLTIP_SYN_TYPE;
extern const char *WS_STR_FMT_TOOLTIP_GMAX;

#endif // WORKSPACE_STRINGS_H
