#include "storage/sql/sql_queries.h"

// ==============================================================================
// 1. DATA DEFINITION (DDL) - CRIAÇÃO DE TABELAS
// ==============================================================================

const char *SQL_CREATE_NETWORK_NEURONS =
    "CREATE TABLE IF NOT EXISTS Network_Neurons ("
    "session_id INTEGER, "
    "neuron_id INTEGER, "
    "neuron_type TEXT, "
    "i_ext REAL, "
    "PRIMARY KEY (session_id, neuron_id), "
    "FOREIGN KEY(session_id) REFERENCES Simulation_Sessions(id) ON DELETE CASCADE"
");";

const char *SQL_CREATE_NETWORK_SYNAPSES =
    "CREATE TABLE IF NOT EXISTS Network_Synapses ("
    "session_id INTEGER, "
    "source_id INTEGER, "
    "target_id INTEGER, "
    "weight REAL, "
    "synapse_type TEXT, " // 'AMPA' ou 'GABA'
    "PRIMARY KEY (session_id, source_id, target_id), "
    "FOREIGN KEY(session_id) REFERENCES Simulation_Sessions(id) ON DELETE CASCADE"
");";

const char *SQL_CREATE_SESSIONS =
    "CREATE TABLE IF NOT EXISTS Simulation_Sessions ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "timestamp DATETIME DEFAULT (datetime('now', 'localtime')),"
    "notes TEXT, "
    "neuron_count INTEGER, "
    "dt REAL, "
    "total_time REAL "
");";

const char *SQL_CREATE_SPIKES =
    "CREATE TABLE IF NOT EXISTS Spike_Events ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "session_id INTEGER, "
    "neuron_id INTEGER, "
    "spike_time REAL, "
    "FOREIGN KEY(session_id) REFERENCES Simulation_Sessions(id) "
    "ON DELETE CASCADE"
");";

const char *SQL_CREATE_TAU =
    "CREATE TABLE IF NOT EXISTS Tau ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "session_id INTEGER, "
    "neuron_id_1 INTEGER, " // Primeiro neurônio do par
    "neuron_id_2 INTEGER, " // Segundo neurônio do par
    "xVal REAL, yVal REAL, "
    "FOREIGN KEY(session_id) REFERENCES Simulation_Sessions(id) "
    "ON DELETE CASCADE"
");";

const char *SQL_CREATE_TRACE_TEMPLATE =
    "CREATE TABLE IF NOT EXISTS %s ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "session_id INTEGER, "
    "neuron_id INTEGER, "
    "xVal REAL, yVal REAL, "
    "FOREIGN KEY(session_id) REFERENCES Simulation_Sessions(id) "
    "ON DELETE CASCADE"
");";

// ==============================================================================
// 2. DATA MANIPULATION (DML) - SELEÇÃO E DELEÇÃO
// ==============================================================================

// --- Seleção Geral ---
const char *SQL_SELECT_SESSIONS_LIMIT =
    "SELECT id, timestamp, notes, total_time "
    "FROM Simulation_Sessions "
    "ORDER BY id DESC LIMIT ?;"
;

const char *SQL_COUNT_SESSIONS =
    "SELECT COUNT(*) "
    "FROM Simulation_Sessions;"
;

// --- Seleção Específica (Analyzer) ---
const char *SQL_COUNT_SPIKES_BY_SESSION =
    "SELECT COUNT(*) "
    "FROM Spike_Events "
    "WHERE session_id = ?;"
;

const char *SQL_COUNT_DISTINCT_NEURONS =
    "SELECT COUNT(DISTINCT neuron_id) "
    "FROM Spike_Events "
    "WHERE session_id = ?;"
;

const char *SQL_SELECT_DURATION_BY_ID =
    "SELECT total_time "
    "FROM Simulation_Sessions "
    "WHERE rowid = ?;"
;

const char *SQL_SELECT_DISTINCT_NEURONS =
    "SELECT DISTINCT neuron_id "
    "FROM Spike_Events "
    "WHERE session_id = ? "
    "ORDER BY neuron_id ASC;"
;

const char *SQL_SELECT_NOTES_BY_ID =
    "SELECT notes "
    "FROM Simulation_Sessions "
    "WHERE rowid = ?;"
;

const char *SQL_SELECT_SPIKES_BY_SESSION_ORDERED =
    "SELECT spike_time, neuron_id "
    "FROM Spike_Events "
    "WHERE session_id = ? "
    "ORDER BY spike_time ASC;"
;

// --- Deleção ---
const char *SQL_DELETE_SESSION_BY_ID = "DELETE FROM Simulation_Sessions WHERE id = ?;";

const char *SQL_ENABLE_FOREIGN_KEYS  = "PRAGMA foreign_keys = ON;";
const char *SQL_ENABLE_SYNCHRONOUS   = "PRAGMA synchronous = NORMAL;";
const char *SQL_ENABLE_WAL_MODE      = "PRAGMA journal_mode = WAL;";

const char *SQL_RESET_SEQUENCE       = "DELETE FROM sqlite_sequence WHERE name = '%s';";

// ==============================================================================
// 3. DATA MANIPULATION (DML) - INSERÇÃO E TRANSAÇÕES
// ==============================================================================

// --- Transações ---
const char *SQL_BEGIN_TRANS  = "BEGIN TRANSACTION;";
const char *SQL_COMMIT_TRANS = "COMMIT;";

// --- Inserção de Sessão ---
const char *SQL_INSERT_SESSION ="INSERT INTO Simulation_Sessions (notes, neuron_count, dt, total_time) VALUES (?, ?, ?, ?);";

// --- Inserção de Dados ---
const char *SQL_INSERT_SPIKE = "INSERT INTO Spike_Events (session_id, neuron_id, spike_time) VALUES (?, ?, ?);";
const char *SQL_INSERT_TAU   = "INSERT INTO Tau (session_id, neuron_id_1, neuron_id_2, xVal, yVal) VALUES (?, ?, ?, ?, ?);";
const char *SQL_INSERT_TRACE_TEMPLATE = "INSERT INTO %s (session_id, neuron_id, xVal, yVal) VALUES (?, ?, ?, ?);";

// ==============================================================================
// 4. DATA RETRIEVAL (DQL) - CARREGAMENTO PARA BUFFER (Memory Playback)
// ==============================================================================

const char *SQL_CREATE_SPIKES_INDEX =
    "CREATE INDEX IF NOT EXISTS idx_Spike_Events_session "
    "ON Spike_Events(session_id, spike_time ASC);"
;

const char *SQL_CREATE_TAU_INDEX =
    "CREATE INDEX IF NOT EXISTS idx_tau_pairwise "
    "ON Tau (session_id, neuron_id_1, neuron_id_2);"
;

const char *SQL_CREATE_INDEX_TEMPLATE =
    "CREATE INDEX IF NOT EXISTS idx_%s_session "
    "ON %s(session_id, xVal ASC, neuron_id, yVal);"
;

const char *SQL_LOAD_SPIKES =
    "SELECT neuron_id, spike_time FROM Spike_Events "
    "WHERE session_id = ? ORDER BY spike_time ASC;"
;

const char *SQL_LOAD_TAU_PAIR =
    "SELECT neuron_id_1, neuron_id_2, xVal, yVal FROM Tau "
    "WHERE session_id = ? AND neuron_id_1 = ? AND neuron_id_2 = ? "
    "ORDER BY xVal ASC;";
;

const char *SQL_LOAD_TRACE_TEMPLATE =
    "SELECT neuron_id, xVal, yVal FROM %s "
    "WHERE session_id = ? ORDER BY xVal ASC;"
;
