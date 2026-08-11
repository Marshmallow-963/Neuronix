#ifndef SQL_QUERIES_H
#define SQL_QUERIES_H

// ==============================================================================
// 1. DATA DEFINITION (DDL) - CRIAÇÃO DE TABELAS
// ==============================================================================

extern const char *SQL_CREATE_NETWORK_NEURONS;
extern const char *SQL_CREATE_NETWORK_SYNAPSES;
extern const char *SQL_CREATE_SESSIONS;         // Tabela Mestra (Sessões)
extern const char *SQL_CREATE_SPIKES;
extern const char *SQL_CREATE_TAU;
extern const char *SQL_CREATE_TRACE_TEMPLATE;

// ==============================================================================
// 2. DATA MANIPULATION (DML) - SELEÇÃO E DELEÇÃO
// ==============================================================================

// --- Seleção Geral ---
extern const char *SQL_COUNT_SESSIONS;
extern const char *SQL_SELECT_SESSIONS_LIMIT;

// --- Seleção Específica (Analyzer) ---
extern const char *SQL_COUNT_DISTINCT_NEURONS;
extern const char *SQL_COUNT_SPIKES_BY_SESSION;

extern const char *SQL_SELECT_DURATION_BY_ID;
extern const char *SQL_SELECT_DISTINCT_NEURONS;
extern const char *SQL_SELECT_NOTES_BY_ID;
extern const char *SQL_SELECT_SPIKES_BY_SESSION_ORDERED;

// --- Deleção ---
extern const char *SQL_DELETE_SESSION_BY_ID;

extern const char *SQL_ENABLE_FOREIGN_KEYS;
extern const char *SQL_ENABLE_WAL_MODE;
extern const char *SQL_ENABLE_SYNCHRONOUS;

extern const char *SQL_RESET_SEQUENCE;

// ==============================================================================
// 3. DATA MANIPULATION (DML) - INSERÇÃO E TRANSAÇÕES
// ==============================================================================

// --- Transações ---
extern const char *SQL_BEGIN_TRANS;
extern const char *SQL_COMMIT_TRANS;

// --- Inserção de Sessão ---
extern const char *SQL_INSERT_SESSION;

// --- Inserção de Dados ---
extern const char *SQL_INSERT_SPIKE;
extern const char *SQL_INSERT_TAU;
extern const char *SQL_INSERT_TRACE_TEMPLATE;

// ==============================================================================
// 4. DATA RETRIEVAL (DQL) - CARREGAMENTO PARA BUFFER (Memory Playback)
// ==============================================================================

extern const char *SQL_CREATE_INDEX_TEMPLATE;
extern const char *SQL_CREATE_SPIKES_INDEX;
extern const char *SQL_CREATE_TAU_INDEX;

extern const char *SQL_LOAD_SPIKES;
extern const char *SQL_LOAD_TAU_PAIR;
extern const char *SQL_LOAD_TRACE_TEMPLATE;

#endif // SQL_QUERIES_H
