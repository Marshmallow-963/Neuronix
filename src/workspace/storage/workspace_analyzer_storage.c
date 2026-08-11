#include <stdio.h>
#include <string.h>

#include "graph/config/graph_config.h"
#include "log/logger.h"

#include "storage/config/storage_config.h"
#include "storage/config/storage_strings.h"
#include "storage/core/storage_enums.h"
#include "storage/sql/sql_enums.h"
#include "storage/sql/sql_queries.h"

#include "workspace/core/workspace_enums.h"
#include "workspace/core/workspace_state.h"
#include "workspace/logic/workspace_logic.h"
#include "workspace/plotting/workspace_plot_enums.h"
#include "workspace/storage/workspace_storage.h"
#include "workspace/storage/workspace_storage_state.h"
#include "workspace/strings/workspace_strings.h"

#include "utils/memory_buffer.h"

static inline size_t WorkspaceTableRowCountGet(WorkspaceState *state);

static inline sqlite3_stmt* WorkspaceStatementPrepare(
    WorkspaceState *state,
    TableId tableId,
    int64 sessionId
);

static inline void WorkspaceDataInsert(
    WorkspaceState *state,
    NeuroTraceBuffer *buf,
    Vector2d **dstPtr,
    size_t *cntPtr,
    size_t *capPtr,
    Vector2d point
);

static inline void* WorkspaceDataLoaderThread(void* arg);

static inline void WorkspacePointersResolve(
    TableId tableId,
    NeuroTraceBuffer *buf,
    int64 resolvedIdx,
    size_t *counts,
    size_t *capacities,
    size_t **outCnt,
    size_t **outCap,
    Vector2d **baseTraceArray,
    Vector2d ***outDest
);

Vector2d** WorkspaceTraceArrayGet(NeuroTraceBuffer *buffer, TableId tableId) {
    if (!buffer) return NULL;

    switch (tableId) {
        case TABLE_ID_VOLTAGE:      return buffer->neuronTraces;
        case TABLE_ID_SYNAPTIC:     return buffer->synapticTraces;
        case TABLE_ID_NEUROTRANS:   return buffer->ntFractionTraces;
        case TABLE_ID_PHASE_SPACE:  return buffer->phaseSpaceTraces;
        case TABLE_ID_IZ_RECOVERY:  return buffer->izRecoveryTraces;

        case TABLE_ID_HH_GATE_M:    return buffer->hhGatePlots.MGate;
        case TABLE_ID_HH_GATE_H:    return buffer->hhGatePlots.HGate;
        case TABLE_ID_HH_GATE_N:    return buffer->hhGatePlots.NGate;

        case TABLE_ID_HH_IK:        return buffer->hhCurrentPlots.kCurrent;
        case TABLE_ID_HH_INA:       return buffer->hhCurrentPlots.naCurrent;
        case TABLE_ID_HH_ILEAK:     return buffer->hhCurrentPlots.leakCurrent;

        // Cuidado com a TBL_TAU, ela usa matriz (MAX_DETAILED_PLOTS * MAX_DETAILED_PLOTS)
        case TABLE_ID_TAU:          return buffer->tauTraces;

        default:                    return NULL;
    }
}

void WorkspaceTableLoad(WorkspaceState *state,TableId tableId, int64 sessionId) {
    if (!state || state->loading.isLoading) return;

    size_t srcId = state->editor.sourceNeuronId;
    size_t tgtId = state->editor.targetNeuronId;

    AnalyzerStateReset(state);

    state->editor.sourceNeuronId = srcId;
    state->editor.targetNeuronId = tgtId;

    state->session.activeTableId = tableId;
    state->session.activeSessionId = sessionId;

    state->loading.activeLoadStmt = WorkspaceStatementPrepare(state, tableId, sessionId);

    if (!state->loading.activeLoadStmt) return;
    state->loading.isLoading = true;

    pthread_t loadThread;
    if (pthread_create(&loadThread, NULL, WorkspaceDataLoaderThread, (void*)state) == 0) {
        pthread_detach(loadThread);
    } else {
        Logger(ERROR, "Falha ao criar a thread de carregamento de dados!");
        state->loading.isLoading = false;
    }
}

static inline void* WorkspaceStaticDataSaveThread(void* arg) {
    WorkspaceState *state = (WorkspaceState*)arg;

    int64_t sessionId = state->session.activeSessionId;
    if (sessionId <= 0) sessionId = (int64_t)state->saving.autoSaveSessionId;

    char *errMsg = NULL;

    // 1. Inicia uma transação imediata para travar o banco e acelerar as inserções
    if (sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", NULL, NULL, &errMsg) != SQLITE_OK) {
        Logger(ERROR, "Falha ao iniciar transacao da matriz Tau: %s", errMsg);
        sqlite3_free(errMsg);
        state->saving.isSaving = false;
        return NULL;
    }

    // 2. Remove registros antigos de Tau DESTA sessao para evitar duplicatas completas
    sqlite3_stmt *delStmt;
    const char *SQL_DELETE_TAU_PAIR = "DELETE FROM Tau WHERE session_id = ? AND neuron_id_1 = ? AND neuron_id_2 = ?;";

    if (sqlite3_prepare_v2(db, SQL_DELETE_TAU_PAIR, SQL_AUTO_LENGTH, &delStmt, NULL) != SQLITE_OK) {
        Logger(ERROR, "Erro ao preparar delecao especifica da matriz Tau: %s", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        state->saving.isSaving = false;
        return NULL;
    }

    // 3. Prepara o statement de insercao reutilizavel (5 colunas)
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, SQL_INSERT_TAU, SQL_AUTO_LENGTH, &stmt, NULL) != SQLITE_OK) {
        Logger(ERROR, "Erro ao preparar insert da matriz Tau: %s", sqlite3_errmsg(db));
        sqlite3_finalize(delStmt);
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        state->saving.isSaving = false;
        return NULL;
    }

    size_t totalPointsSaved = 0;

    // 4. Varre a matriz bidimensional [Source][Target] protegida pelo Mutex
    pthread_mutex_lock(&state->concurrency.anaMutex);
        for (size_t srcId = 0; srcId < MAX_DETAILED_PLOTS; srcId++) {
            for (size_t tgtId = 0; tgtId < MAX_DETAILED_PLOTS; tgtId++) {

                size_t matrixIdx = (srcId * MAX_DETAILED_PLOTS) + tgtId;
                size_t count = state->staticData.tauPointsCount[matrixIdx];
                Vector2d *points = state->staticData.tauTraces[matrixIdx];

                if (count == 0 || !points) continue;

                sqlite3_bind_int64(delStmt, SQL_BIND_IDX_1, sessionId);
                sqlite3_bind_int64(delStmt, SQL_BIND_IDX_2, (int64_t)srcId);
                sqlite3_bind_int64(delStmt, SQL_BIND_IDX_3, (int64_t)tgtId);

                sqlite3_step(delStmt);
                sqlite3_reset(delStmt);

                for (size_t k = 0; k < count; k++) {
                    sqlite3_bind_int64(stmt, SQL_BIND_IDX_1, sessionId);
                    sqlite3_bind_int64(stmt, SQL_BIND_IDX_2, (int64_t)srcId);
                    sqlite3_bind_int64(stmt, SQL_BIND_IDX_3, (int64_t)tgtId);
                    sqlite3_bind_double(stmt, SQL_BIND_IDX_4, points[k].x);
                    sqlite3_bind_double(stmt, SQL_BIND_IDX_5, points[k].y);

                    sqlite3_step(stmt);
                    sqlite3_reset(stmt);
                    totalPointsSaved++;
                }
            }
        }
    pthread_mutex_unlock(&state->concurrency.anaMutex);

    // 5. Finaliza o statement e commita as alterações se houver dados salvos
    sqlite3_finalize(delStmt);
    sqlite3_finalize(stmt);

    if (totalPointsSaved == 0) {
        Logger(INFO, "Nenhum ponto encontrado na matriz Tau para salvar.");
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        state->saving.isSaving = false;
        return NULL;
    }

    if (sqlite3_exec(db, "COMMIT;", NULL, NULL, &errMsg) != SQLITE_OK) {
        Logger(ERROR, "Erro no Commit da matriz Tau: %s", errMsg);
        sqlite3_free(errMsg);
    } else {
        Logger(INFO, "Matriz de Tau integrada a sessao! %zu pontos salvos.", totalPointsSaved);
    }

    // Libera a UI
    state->saving.isSaving = false;
    return NULL;
}

void WorkspaceStaticDataSave(WorkspaceState *state) {
    if (!state || state->saving.isSaving) return;

    // CORREÇÃO: Resgata corretamente a sessão, seja do BD carregado ou da simulação ao vivo
    int64_t sessionId = state->session.activeSessionId;
    if (sessionId <= 0) sessionId = (int64_t)state->saving.autoSaveSessionId;

    if (sessionId <= 0) {
        Logger(WARNING, "Não há sessão ativa para salvar. Simule ou carregue um banco primeiro.");
        return;
    }

    uint32_t mask = state->saving.activeTracesMask;
    if (!(mask & TRACE_SAVE_TAU)) {
        Logger(WARNING, "A gravacao de TAU nao esta habilitada no menu de CONFIG.");
        return;
    }

    state->saving.isSaving = true;

    pthread_t saveThread;
    if (pthread_create(&saveThread, NULL, WorkspaceStaticDataSaveThread, (void*)state) == 0) {
        pthread_detach(saveThread);
    } else {
        Logger(ERROR, "Falha ao criar thread para salvar matriz Tau!");
        state->saving.isSaving = false;
    }
}

static inline size_t WorkspaceTableRowCountGet(WorkspaceState *state) {
    int64 sessionId = (int64)state->session.activeSessionId;

    if (state->session.activeTableId == TABLE_ID_TAU) {
        size_t exactCount = 0;
        sqlite3_stmt *stmt;

        const char *sqlCount = "SELECT COUNT(*) FROM Tau WHERE session_id = ? AND neuron_id_1 = ? AND neuron_id_2 = ?;";

        if (sqlite3_prepare_v2(db, sqlCount, SQL_AUTO_LENGTH, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int64(stmt, SQL_BIND_IDX_1, sessionId);
            sqlite3_bind_int64(stmt, SQL_BIND_IDX_2, (int64_t)state->editor.sourceNeuronId);
            sqlite3_bind_int64(stmt, SQL_BIND_IDX_3, (int64_t)state->editor.targetNeuronId);

            if (sqlite3_step(stmt) == SQLITE_ROW) exactCount = (size_t)sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
        }
        else Logger(ERROR, "Erro ao buscar contagem de linhas para progresso do Tau: %s", sqlite3_errmsg(db));
        return exactCount;
    }

    int neuronCount = 0;
    size_t estimatedCount = 0;
    double dt = 0.01, totalTime = 0.0;

    sqlite3_stmt *stmt;
    char sql[SQL_CMD_BUFFER_SIZE];

    snprintf(sql, sizeof(sql), "SELECT neuron_count, dt, total_time FROM %s WHERE id = ?;", TBL_SESSIONS);
    if (sqlite3_prepare_v2(db, sql, SQL_AUTO_LENGTH, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, SQL_BIND_IDX_1, sessionId);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            neuronCount = sqlite3_column_int(stmt, SQL_COL_0);
            dt = sqlite3_column_double(stmt, SQL_COL_1);
            totalTime = sqlite3_column_double(stmt, SQL_COL_2);
        }
        sqlite3_finalize(stmt);
    }
    else Logger(ERROR, "Erro ao buscar dados da sessao para o progresso: %s", sqlite3_errmsg(db));

    if (dt <= 0.0) dt = 0.01;
    if (neuronCount <= 0) neuronCount = 1;

    size_t pointsPerNeuron = (size_t)(totalTime / dt);
    estimatedCount = pointsPerNeuron * (size_t)neuronCount;
    return estimatedCount;
}

static inline sqlite3_stmt* WorkspaceStatementPrepare(
    WorkspaceState *state, TableId tableId, int64 sessionId
) {
    char sql[MAX_SQL_QUERY_LEN];
    sqlite3_stmt *stmt = NULL;

    if (tableId == TABLE_ID_TAU) {
        if (sqlite3_prepare_v2(db, SQL_LOAD_TAU_PAIR, SQL_AUTO_LENGTH, &stmt, NULL) != SQLITE_OK) {
            Logger(ERROR, WS_LOG_ERR_PREPARE_LOAD, sqlite3_errmsg(db));
            return NULL;
        }

        sqlite3_bind_int64(stmt, SQL_BIND_IDX_1, sessionId);
        sqlite3_bind_int64(stmt, SQL_BIND_IDX_2, (int64_t)state->editor.sourceNeuronId);
        sqlite3_bind_int64(stmt, SQL_BIND_IDX_3, (int64_t)state->editor.targetNeuronId);
        return stmt;
    }
    else {
        // Obtenha a string APENAS aqui para montar o SQL dinâmico
        const char *dbTableName = WorkspaceTableIdToString(tableId);
        snprintf(sql, sizeof(sql), SQL_LOAD_TRACE_TEMPLATE, dbTableName);

        if (sqlite3_prepare_v2(db, sql, SQL_AUTO_LENGTH, &stmt, NULL) != SQLITE_OK) {
            Logger(ERROR, WS_LOG_ERR_PREPARE_LOAD, sqlite3_errmsg(db));
            return NULL;
        }

        sqlite3_bind_int(stmt, SQL_BIND_IDX_1, (int)sessionId);
        return stmt;
    }
}

static inline void* WorkspaceDataLoaderThread(void* arg) {
    WorkspaceState *state = (WorkspaceState*)arg;

    state->loading.targetDataCount = WorkspaceTableRowCountGet(state);
    state->loading.loadingProgress = 0.0f;

    size_t currentRow = 0;
    sqlite3_stmt *stmt = state->loading.activeLoadStmt;
    NeuroTraceBuffer *buf = &state->staticData;

    // Agora recuperamos o Enum diretamente do estado
    TableId currentTableId = state->session.activeTableId;
    Vector2d **baseTraceArray = WorkspaceTraceArrayGet(buf, currentTableId);

    // Sem strcmp
    bool isTau = (currentTableId == TABLE_ID_TAU);
    size_t arraySize = isTau ? (MAX_DETAILED_PLOTS * MAX_DETAILED_PLOTS) : MAX_DETAILED_PLOTS;

    // Alocação dinâmica limpa para evitar Stack Overflow na pilha da Thread
    size_t *tmpCounts = calloc(arraySize, sizeof(size_t));
    size_t *tmpCapacities = calloc(arraySize, sizeof(size_t));

    int maxNeuronIdFound = INVALID_NEURON_ID;
    double localXMin = BOUND_MAX,  localYMin = BOUND_MAX;
    double localXMax = -BOUND_MAX, localYMax = -BOUND_MAX;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int64 resolvedIdx = 0;
        double time = 0.0;
        double value = 0.0;

        if (isTau) {
            int id1 = sqlite3_column_int(stmt, 0);
            int id2 = sqlite3_column_int(stmt, 1);
            time    = sqlite3_column_double(stmt, 2);
            value   = sqlite3_column_double(stmt, 3);
            resolvedIdx = (id1 * MAX_DETAILED_PLOTS) + id2;

            if (id1 > maxNeuronIdFound) maxNeuronIdFound = id1;
            if (id2 > maxNeuronIdFound) maxNeuronIdFound = id2;
        }
        else {
            int nrnId = sqlite3_column_int(stmt, 0);
            time      = sqlite3_column_double(stmt, 1);
            value     = sqlite3_column_double(stmt, 2);
            resolvedIdx = nrnId;

            if (nrnId > maxNeuronIdFound) maxNeuronIdFound = nrnId;
        }

        if (time < localXMin)  localXMin = time;
        if (time > localXMax)  localXMax = time;
        if (value < localYMin) localYMin = value;
        if (value > localYMax) localYMax = value;

        size_t *capPtr = NULL, *cntPtr = NULL;
        Vector2d **destPtr = NULL;
        Vector2d point = (Vector2d){ time, value };

        // Passa o currentTableId no lugar da string
        WorkspacePointersResolve(
            currentTableId, buf, resolvedIdx, tmpCounts,
            tmpCapacities, &cntPtr, &capPtr, baseTraceArray, &destPtr
        );

        if (destPtr && cntPtr && capPtr) {
            WorkspaceDataInsert(state, buf, destPtr, cntPtr, capPtr, point);
        }

        currentRow++;
        if (state->loading.targetDataCount > 0 && (currentRow % 1000 == 0)) {
            state->loading.loadingProgress = ((float)currentRow / (float)state->loading.targetDataCount) * 100.0f;
        }
    }

    state->loading.loadingProgress = 100.0f;

    pthread_mutex_lock(&state->concurrency.anaMutex);
        if (maxNeuronIdFound >= 0) {
            state->session.loadedNeuronCount = (size_t)(maxNeuronIdFound + 1);
            size_t maxCap = 0, maxPoints = 0;

            for (size_t i = 0; i < arraySize; i++) {
                if (tmpCounts[i] > maxPoints) maxPoints = tmpCounts[i];
                if (tmpCapacities[i] > maxCap) maxCap = tmpCapacities[i];

                if (isTau) buf->tauPointsCount[i] = tmpCounts[i];
            }

            if (isTau) buf->bufferCapacity = maxCap;

            else {
                buf->dataCount      = maxPoints;
                buf->bufferCapacity = maxCap;
            }
        }

        sqlite3_finalize(stmt);
        state->loading.activeLoadStmt = NULL;
        state->loading.isLoading = false;
    pthread_mutex_unlock(&state->concurrency.anaMutex);

    free(tmpCounts);
    free(tmpCapacities);
    return NULL;
}

static inline void WorkspaceDataInsert(
    WorkspaceState *state, NeuroTraceBuffer *buf,
    Vector2d **dstPtr, size_t *cntPtr, size_t *capPtr, Vector2d point
) {
    if (!buf || !state || !dstPtr || !cntPtr || !capPtr) return;

    if (*cntPtr >= *capPtr) {
        size_t newCap = (*capPtr == 0) ? INITIAL_TRACE_CAPACITY : (*capPtr * CAPACITY_GROWTH_MULTIPLIER);

        pthread_mutex_lock(&state->concurrency.anaMutex);
            BufferEnsure((void**)dstPtr, true, newCap, sizeof(Vector2d));
        pthread_mutex_unlock(&state->concurrency.anaMutex);

        *capPtr = (size_t)newCap;
    }

    (*dstPtr)[*cntPtr] = point;
    (*cntPtr)++;
}

static inline void WorkspacePointersResolve(
    TableId tableId, NeuroTraceBuffer *buf, int64 resolvedIdx,
    size_t *counts, size_t *capacities, size_t **outCnt, size_t **outCap,
    Vector2d **baseTraceArray, Vector2d ***outDest
) {
    if (!buf || !counts || !capacities || !outCnt || !outCap || !baseTraceArray || !outDest) return;

    // Use a comparação do Enum
    size_t limit = (tableId == TABLE_ID_TAU) ? (MAX_DETAILED_PLOTS * MAX_DETAILED_PLOTS) : MAX_DETAILED_PLOTS;

    if (resolvedIdx < 0 || (size_t)resolvedIdx >= limit) return;

    if (baseTraceArray) {
        *outDest = &baseTraceArray[resolvedIdx];
        *outCnt  = &counts[resolvedIdx];
        *outCap  = &capacities[resolvedIdx];
    }
}
