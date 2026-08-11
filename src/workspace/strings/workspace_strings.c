#include "workspace/strings/workspace_strings.h"

// =========================================================================
// ERRORS & LOGS
// =========================================================================
const char *WS_LOG_ERR_NEURON_RANGE        = "Neuron IDs out of operational range: Source %zu, Target %zu";
const char *WS_LOG_ERR_PREPARE_LOAD        = "Failed to prepare load query: %s";
const char *WS_LOG_ERR_TAU_MALLOC          = "Failed to allocate memory for Tau analysis results";

const char *WS_LOG_INFO_ASYNC_ANALYSIS     = "Async Analysis started on table %s. Chunk: %zu points/frame.";
const char *WS_LOG_INFO_LOADED_DATA        = "Loaded data from table %s for session %lld";
const char *WS_LOG_INFO_TAU_CALCULATED     = "Tau calculated successfully";

const char *WS_LOG_WARN_INSUFFICIENT_DATA  = "Insufficient data for tau analysis";
const char *WS_LOG_WARN_NO_SPIKES_DETECTED = "No spikes detected in the selected interval";

const char *WS_LOG_ERR_DATA_INSERT         = "Error inserting data point %zu: %s";
const char *WS_LOG_ERR_DB_NOT_CONNECTED    = "Attempted to save session without a connected database.";
const char *WS_LOG_ERR_PREPARE_TRACE       = "SQL Prepare Error (%s): %s";
const char *WS_LOG_ERR_SESSION_CREATE      = "Error creating session record. Rollback executed.";
const char *WS_LOG_ERR_SESSION_INSERT      = "Failed to execute INSERT SESSION: %s";
const char *WS_LOG_ERR_TRANS_BEGIN         = "Failed to begin database transaction: %s";
const char *WS_LOG_ERR_TRANS_COMMIT        = "Transaction Commit error: %s";

// =========================================================================
// NET CONFIG STRINGS & FORMATS
// =========================================================================
const char *WS_STR_TITLE_NEURON_PARAMS       = "NEURON'S PARAMETERS";
const char *WS_STR_TITLE_SYNAPSE_PARAMS      = "SYNAPSE'S PARAMETERS";

const char *WS_STR_HDR_IZ_MODELS             = "--- Izhikevich Models ---";
const char *WS_STR_HDR_HH_MODELS             = "--- Hodgkin-Huxley (Squid Axon) ---";
const char *WS_STR_HDR_IZ_SYNAPSE            = "--- Izhikevich Synapse Config ---";
const char *WS_STR_HDR_HH_SYNAPSE            = "--- Hodgkin-Huxley Synapse Config ---";

const char *WS_STR_FMT_IZ_PARAMS             = "%-21s | a: %.3f | b: %.3f | c: %.1f mV | d: %.3f";
const char *WS_STR_FMT_HH_MEMBRANE           = "Membrane Capacitance: %.2f | Resting Potential: %.2f mV";
const char *WS_STR_FMT_HH_SODIUM             = "Sodium Conductance: %.2f | Sodium Reversal: %.2f mV";
const char *WS_STR_FMT_HH_POTASSIUM          = "Potassium Conductance: %.2f | Potassium Reversal: %.2f mV";
const char *WS_STR_FMT_HH_LEAK               = "Leak Conductance: %.2f | Leak Reversal: %.2f mV";
const char *WS_STR_FMT_SYN_SIGMOID           = "Sigmoid Slope (Kp): %.2f mV | Activation Voltage (Vp): %.2f mV";
const char *WS_STR_FMT_SYN_AMPA              = "AMPA   -> Connect Rate: %.3f mM^{-1} ms^{-1} | Decay Rate: %.3f ms^{-1} | Reversal: %.1f mV";
const char *WS_STR_FMT_SYN_GABA              = "GABA_A -> Connect Rate: %.3f mM^{-1} ms^{-1} | Decay Rate: %.3f ms^{-1} | Reversal: %.1f mV";

const char *WS_STR_IZ_CHATTERING             = "Chattering";
const char *WS_STR_IZ_FAST_SPIKING           = "Fast Spiking";
const char *WS_STR_IZ_INTRINSICALLY_BURSTING = "Intrinsically Bursting";
const char *WS_STR_IZ_LOW_THRESHOLD_SPIKING  = "Low Threshold Spiking";
const char *WS_STR_IZ_REGULAR_SPIKING        = "Regular Spiking";
const char *WS_STR_IZ_RESONATOR              = "Resonator";
const char *WS_STR_IZ_THALAMO_CORTICAL       = "Thalamo-Cortical";

// =========================================================================
// STRINGS (UI)
// =========================================================================
const char *WS_STR_BTN_REFRESH_LIST     = "Refresh List";
const char *WS_STR_BTN_TAU_ANALYSIS     = "Tau Analysis";

const char *WS_STR_FMT_SESSION_ENTRY    = "Session %d: %s";

const char *WS_STR_LBL_NO_SESSIONS      = "No sessions found";

const char *WS_STR_MSG_NO_DATA_LOADED   = "No data loaded from the database.";
const char *WS_STR_MSG_NO_NOTES         = "No notes";

const char *WS_STR_SEL_SOURCE_ID        = "Source ID";
const char *WS_STR_SEL_TARGET_ID        = "Target ID";

const char *WS_STR_TITLE_CONFIG         = "CONFIGURATIONS";
const char *WS_STR_TITLE_DATA_TREATMENT = "DATA TREATMENT";
const char *WS_STR_TITLE_SAVED_SESSIONS = "SAVED SESSIONS";

const char *WS_STR_TT_CONFIG            = "Configuration";
const char *WS_STR_TT_DATABASE          = "Database";
const char *WS_STR_TT_DATA_TREATMENT    = "Data Treatment";

const char *WS_STR_BTN_ADD_NEURON       = "Add Neuron";
const char *WS_STR_BTN_ADD_SYNAPSE      = "Add Synapse";
const char *WS_STR_BTN_ADJ_MATRIX       = "Adjacency Matrix";
const char *WS_STR_BTN_CLEAR_NET        = "Clear Network";
const char *WS_STR_BTN_DASHBOARD        = "Dashboard";
const char *WS_STR_BTN_RESET_SIM        = "Reset Simulation";
const char *WS_STR_BTN_SAVE_STATE       = "Save State";
const char *WS_STR_BTN_SHOW_BIO_PARAMS  = "Show Biophysical Parameters";
const char *WS_STR_BTN_SHOW_PHASE       = "Show Phase Graph";
const char *WS_STR_BTN_SHOW_SYN_CUR     = "Show Synaptic Currents";
const char *WS_STR_BTN_START_SIM        = "Start Simulation";
const char *WS_STR_BTN_STOP_SIM         = "Stop Simulation";

const char *WS_STR_FMT_ADJ_DETAIL       = " %d. %s | gMax: %.2f nS |";
const char *WS_STR_FMT_ADJ_HEADER       = "[%d] -> [%d] (Count: %d, Net: %.2f)";
const char *WS_STR_FMT_WEIGHT_VAL       = "%.2f";

const char *WS_STR_LBL_AUTAPSE_DEL      = "Autapse Delay: %.2f ms";
const char *WS_STR_LBL_CONDUCTANCE      = "Conductance: %.2f nS";
const char *WS_STR_LBL_EXT_CURRENT      = "External Current: %.2f pA";
const char *WS_STR_LBL_INTERVAL_MS      = "Interval (ms)";
const char *WS_STR_LBL_IZ_TYPE          = "Izhikevich Type:";
const char *WS_STR_LBL_LARGE_NET_MODE   = "Large Network Mode";
const char *WS_STR_LBL_NET_CAPACITY     = "Network Capacity";
const char *WS_STR_LBL_NET_TOPOLOGY     = "Network Topology:";
const char *WS_STR_LBL_NRN_MODEL        = "Neuron Model:";
const char *WS_STR_LBL_PLAY_SPEED       = "Play Speed:";
const char *WS_STR_LBL_SOURCE_ID        = "Source ID";
const char *WS_STR_LBL_SYN_MODEL        = "Synapse Model:";
const char *WS_STR_LBL_TARGET_ID        = "Target ID";
const char *WS_STR_LBL_ZOOM_OUT         = "Zoom out";

const char *WS_STR_MENU_TITLE_CFG         = "CONFIGURATION";
const char *WS_STR_MENU_TITLE_EDIT        = "NETWORK EDIT";
const char *WS_STR_MENU_TITLE_EXTRA_PLOTS = "EXTRA PLOTS";
const char *WS_STR_MENU_TITLE_HH_PLOTS    = "HH EXTRA PLOT";
const char *WS_STR_MENU_TITLE_PARAMS      = "PARAMETERS";
const char *WS_STR_MENU_TITLE_SIM         = "SIMULATION CONTROLS";
const char *WS_STR_MENU_TITLE_VIEW        = "VISUALIZATION MODE";

const char *WS_STR_OPT_AMPA             = "AMPA";
const char *WS_STR_OPT_GABA             = "GABAA";
const char *WS_STR_OPT_GLOBAL_VIEWS     = "Population Average;Population Detailed;Only Voltage Detailed;Raster Plot";
const char *WS_STR_OPT_IZ_MODELS        = "Chattering;Fast Spiking;Intrinsically Bursting;Low Threshold Spiking;Regular Spiking;Resonator;Thalamo Cortical";
const char *WS_STR_OPT_NO_CONN_FOUND    = "No connections found.";
const char *WS_STR_OPT_NRN_MODELS       = "Izhikevich;Hodgkin-Huxley";
const char *WS_STR_OPT_PLAY_SPEEDS      = "1x;2x;3x;4x;5x";
const char *WS_STR_OPT_SYN_MODELS       = "Ampa;Gaba_a";
const char *WS_STR_OPT_TOPOLOGY         = "Graph Neural Network";
const char *WS_STR_OPT_ZOOM_OPTIONS     = "On;Off";

const char *WS_STR_MSG_INIT_OR_SELECT   = "Initialize a network or select a table from database";
const char *WS_STR_MSG_LOADING_TABLE    = "Loading table data from database [%.0f%%]";

const char *WS_STR_TXT_CONN_DETAILS     = "Active Connections Details";
const char *WS_STR_TXT_MORE_HIDDEN      = "... more items hidden ...";
const char *WS_STR_TXT_NO_NET_CREATED   = "No network created.";
const char *WS_STR_TXT_WAIT_DATA        = "Waiting for data...";
const char *WS_STR_TXT_WAIT_SPIKES      = "Waiting for spikes...";

// =========================================================================
// TOPOLOGY TOOLTIPS & LABELS FORMATS
// =========================================================================
const char *WS_STR_FMT_NEURON_ID          = "%zu";
const char *WS_STR_FMT_TOOLTIP_NRN_INFO   = "ID: %d | Type: %s";
const char *WS_STR_FMT_TOOLTIP_I_EXT      = "I_ext: %.2f pA";
const char *WS_STR_FMT_TOOLTIP_VOLT       = "V: %.2f mV";
const char *WS_STR_FMT_TOOLTIP_SYN_NODES  = "Synapse: %zu -> %zu";
const char *WS_STR_FMT_TOOLTIP_SYN_TYPE   = "Type: %s";
const char *WS_STR_FMT_TOOLTIP_GMAX       = "g_max: %.2f nS";
