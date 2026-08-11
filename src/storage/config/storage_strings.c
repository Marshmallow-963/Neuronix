#include <stddef.h>
#include "storage/config/storage_strings.h"

// ==============================================================================
// ARRAYS INTERNOS (Agrupados por Domínio de Funcionalidade)
// ==============================================================================

// --- Todas as tabelas de rastreamento (Trace) ---
static const char *const TRACE_TABLES_INTERNAL[] = {
    // Tensão e Espaço de Fase
    TBL_AVERAGE_VOLTAGE,
    TBL_PHASE_SPACE,
    TBL_VOLTAGE,

    // Dinâmica Sináptica e Modelos de Izhikevich
    TBL_IZ_RECOVERY,
    TBL_NEUROTRANS,
    TBL_SYNAPTIC,

    // Correntes de Hodgkin-Huxley
    TBL_HH_IK,
    TBL_HH_INA,
    TBL_HH_ILEAK,

    // Comportas (Gates) de Hodgkin-Huxley
    TBL_HH_GATE_M,
    TBL_HH_GATE_H,
    TBL_HH_GATE_N,
};

// --- Tabelas de Trace customizáveis pelo usuário (UI / Bitmask) ---
// ==============================================================================
// Nota: A ordem abaixo DEVE corresponder exatamente aos bits de TraceSaveMask
// ==============================================================================
static const char *const TRACE_CONFIG_INTERNAL[] = {
    TBL_IZ_RECOVERY,  // Bit 0

    TBL_HH_IK,        // Bit 1
    TBL_HH_INA,       // Bit 2
    TBL_HH_ILEAK,     // Bit 3

    TBL_HH_GATE_M,    // Bit 4
    TBL_HH_GATE_H,    // Bit 5
    TBL_HH_GATE_N,    // Bit 6

    TBL_NET_NRN,      // Bit 7
    TBL_NET_SYN,      // Bit 8
    TBL_NEUROTRANS,   // Bit 9
    TBL_PHASE_SPACE,  // Bit 10
    TBL_SYNAPTIC,     // Bit 11

    TBL_TAU,          // bit 12
    TBL_VOLTAGE,      // Bit 13
};

// --- Tabelas globais do sistema que usam chaves AUTOINCREMENT ---
static const char *const SEQUENCE_TABLES_INTERNAL[] = {
    TBL_AVERAGE_VOLTAGE,
    TBL_NEUROTRANS,
    TBL_HH_GATE_M,
    TBL_HH_GATE_H,
    TBL_HH_GATE_N,
    TBL_HH_IK,
    TBL_HH_INA,
    TBL_HH_ILEAK,
    TBL_PHASE_SPACE,
    TBL_IZ_RECOVERY,
    TBL_SESSIONS,
    TBL_SPIKES,
    TBL_SYNAPTIC,
    TBL_TAU,
    TBL_VOLTAGE,
};

// ==============================================================================
// DEFINIÇÕES DAS COLECÕES GLOBAIS (Structs)
// ==============================================================================

const TableCollection ALL_TRACE_TABLES = {
    .names = TRACE_TABLES_INTERNAL,
    .count = sizeof(TRACE_TABLES_INTERNAL) / sizeof(TRACE_TABLES_INTERNAL[0])
};

const TableCollection CONFIG_TRACE_TABLES = {
    .names = TRACE_CONFIG_INTERNAL,
    .count = sizeof(TRACE_CONFIG_INTERNAL) / sizeof(TRACE_CONFIG_INTERNAL[0])
};

const TableCollection SEQUENCE_RESET_TABLES = {
    .names = SEQUENCE_TABLES_INTERNAL,
    .count = sizeof(SEQUENCE_TABLES_INTERNAL) / sizeof(SEQUENCE_TABLES_INTERNAL[0])
};

// Database Connection & Init
const char *MSG_CLEAN_COMPLETE  = "Database cleared. Auto-increment sequences reset.\n";
const char *MSG_DB_INIT_SUCCESS = "SQLite database successfully initialized.\n";

// Session Persistence
const char *MSG_DELETE_SUCCESS = "Success: Session ID %zu deleted.\n";
const char *MSG_SESSION_SAVED  = "Database: Session %zu persisted (%zu spikes).\n";

// Error Messages - General
const char *MSG_DELETE_ERROR  = "Error deleting session: %s\n";
const char *MSG_PREPARE_ERROR = "Error preparing command: %s\n";

// Warnings
const char *WARN_UNKNOWN_VIEW = "Unknown visualization mode during save operation.";
