#ifndef STORAGE_STRINGS_H
#define STORAGE_STRINGS_H

#include "stddef.h"
#include "storage/core/storage_state.h"

// Table's Name
#define TBL_AVERAGE_VOLTAGE "Average_Voltage"
#define TBL_IZ_RECOVERY     "IZ_Recovery"
#define TBL_HH_IK           "HH_Current_K"
#define TBL_HH_INA          "HH_Current_Na"
#define TBL_HH_ILEAK        "HH_Current_Leak"
#define TBL_HH_GATE_M       "HH_Gate_M"
#define TBL_HH_GATE_H       "HH_Gate_H"
#define TBL_HH_GATE_N       "HH_Gate_N"
#define TBL_NET_NRN         "Network_Neurons"
#define TBL_NET_SYN         "Network_Synapses"
#define TBL_NEUROTRANS      "Neurotransmitter"
#define TBL_PHASE_SPACE     "Phase_Space"
#define TBL_SESSIONS        "Simulation_Sessions"
#define TBL_SPIKES          "Spike_Events"
#define TBL_SYNAPTIC        "Synaptic_Current"
#define TBL_TAU             "Tau"
#define TBL_VOLTAGE         "Voltage"

// --- Coleções de Tabelas Organizadas por Funcionalidade ---
extern const TableCollection ALL_TRACE_TABLES;      // Inicialização e segurança do BD
extern const TableCollection CONFIG_TRACE_TABLES;   // Mapeamento de UI e Bitmask
extern const TableCollection SEQUENCE_RESET_TABLES; // Tabelas com chaves AUTOINCREMENT

// Database Connection & Init
extern const char *MSG_CLEAN_COMPLETE;
extern const char *MSG_DB_INIT_SUCCESS;

// Session Persistence
extern const char *MSG_DELETE_SUCCESS;
extern const char *MSG_SESSION_SAVED;

// Error Messages - General
extern const char *MSG_DELETE_ERROR;
extern const char *MSG_PREPARE_ERROR;

// Warnings
extern const char *WARN_UNKNOWN_VIEW;

// Analyzer Specific Messages
extern const char *ERR_PREPARE_LOAD_SPIKES;
extern const char *MSG_LOADING_SESSION_SPIKES;
extern const char *WARN_ANALYZER_BUFFER_FULL;
extern const char *MSG_LOAD_SPIKES_SUCCESS;

#endif // STORAGE_STRINGS_H
