#include <stdio.h>
#include <string.h>

#include "log/logger.h"

#include "neuro_data/neuro_data_trace.h"

#include "network/network_enum.h"
#include "network/network_manager.h"

#include "storage/core/storage_manager.h"
#include "storage/config/storage_strings.h"

#include "storage/core/storage_enums.h"
#include "storage/sql/sql_enums.h"
#include "storage/sql/sql_queries.h"

#include "workspace/core/workspace_state.h"
#include "workspace/storage/workspace_storage.h"
#include "workspace/storage/workspace_storage_state.h"

#include "workspace/strings/workspace_strings.h"

static inline size_t WorkspaceSessionInsert(WorkspaceState *state, const char *notes);

static inline void WorkspaceDetailedSave(
    WorkspaceState *state,
    size_t sessionId,
    size_t startIdx,
    size_t endIdx
);

static inline void WorkspaceTraceBatchSave(
    const char *tableName,
    const bool *connMask,
    bool shouldSave,
    size_t sessionId,
    size_t nrnCount,
    size_t lastSaved,
    size_t currentCount,
    Vector2d **data
);

static inline void* WorkspaceSaveSessionThread(void* arg);

void WorkspaceAutoSaveUpdate(WorkspaceState *state) {
    if (!state || !state->network || state->saving.isSaving) return;

    if (state->saving.activeTracesMask == 0) {
        state->saving.lastSavedDataCount = state->liveData.dataCount;
        return;
    }

    size_t currentData   = state->liveData.dataCount;
    size_t dataChunkSize = BUFFER_CAPACITY / 4;
    size_t lastSavedData = state->saving.lastSavedDataCount;

    if (currentData >= lastSavedData + dataChunkSize) {
        SaveSessionArgs *args = (SaveSessionArgs*)malloc(sizeof(SaveSessionArgs));
        if (!args) return;

        args->state       = state;
        args->notes[0]    = '\0';
        args->isFinalSave = false;

        state->saving.isSaving = true;

        pthread_t saveThread;
        if (pthread_create(&saveThread, NULL, WorkspaceSaveSessionThread, (void*)args) == 0) {
            pthread_detach(saveThread);
        }
        else {
            state->saving.isSaving = false;
            free(args);
        }
    }
}

void WorkspaceSaveSession(WorkspaceState *state) {
    if (!state) return;

    const char *notes = state->ui.saveModal.textBuffer;
    double finalTime = state->runtime.currentTime;

    if (!db) {
        Logger(ERROR, WS_LOG_ERR_DB_NOT_CONNECTED);
        return;
    }

    if (state->saving.isSaving) {
        state->saving.isFinalSaveRequested = true;
        state->ui.saveModal.isOpen = false;
        return;
    }

    SaveSessionArgs *args = (SaveSessionArgs*)malloc(sizeof(SaveSessionArgs));
    if (!args) return;

    args->state = state;
    if (notes) {
        strncpy(args->notes, notes, sizeof(args->notes) - 1);
        args->notes[sizeof(args->notes) - 1] = '\0';
    }
    else args->notes[0] = '\0';

    args->isFinalSave = true;
    state->ui.saveModal.isOpen = false;

    // Caso especial: fechar sessão vazia
    if (state->liveData.dataCount == 0 && state->saving.autoSaveSessionId != 0) {
        StorageSessionMetadataUpdate(args->notes, finalTime, (size_t)state->saving.autoSaveSessionId);
        state->saving.autoSaveSessionId = 0;
        free(args);
        return;
    }

    state->saving.isSaving = true;

    pthread_t saveThread;
    if (pthread_create(&saveThread, NULL, WorkspaceSaveSessionThread, (void*)args) == 0) {
        pthread_detach(saveThread);
    }
    else {
        state->saving.isSaving = false; free(args);
    }
}

static inline size_t WorkspaceSessionInsert(WorkspaceState *state, const char *notes) {
    size_t sessionId = 0;
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, SQL_INSERT_SESSION, SQL_AUTO_LENGTH, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt,   SQL_BIND_IDX_1, notes, SQL_AUTO_LENGTH, SQLITE_STATIC);
        sqlite3_bind_int(stmt,    SQL_BIND_IDX_2, (int)state->network->neuronCount);
        sqlite3_bind_double(stmt, SQL_BIND_IDX_3, state->network->dt);
        sqlite3_bind_double(stmt, SQL_BIND_IDX_4, state->runtime.currentTime);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            sessionId = (size_t)sqlite3_last_insert_rowid(db);
        }
        else Logger(ERROR, WS_LOG_ERR_SESSION_INSERT, sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return sessionId;
}

static inline void* WorkspaceSaveSessionThread(void* arg) {
    SaveSessionArgs *args = (SaveSessionArgs*)arg;
    if (!args || !args->state) {
        if (args) free(args);
        return NULL;
    }

    WorkspaceState *state = args->state;
    bool success = true;

    pthread_mutex_lock(&state->concurrency.simMutex);
        size_t curr = state->liveData.dataCount;
        size_t last = state->saving.lastSavedDataCount;

        args->startDataIdx = last;
        args->endDataIdx   = curr;

        double finalTime = state->runtime.currentTime;

        if (!state->network) {
            pthread_mutex_unlock(&state->concurrency.simMutex);
            state->saving.isSaving = false;
            free(args);
            return NULL;
        }

        NetworkType type = NetworkDetectType(state->network);
    pthread_mutex_unlock(&state->concurrency.simMutex);

    char *errMsg = NULL;
    if (sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", NULL, NULL, &errMsg) != SQLITE_OK) {
        Logger(ERROR, "Falha ao iniciar transação: %s", errMsg);
        free(args);
        return NULL;
    }

    size_t sessionId = state->saving.autoSaveSessionId;
    if (sessionId == 0) {
        if (!args->isFinalSave) {
            uint32_t m = state->saving.activeTracesMask;

            bool hasData = ( m &
                (TRACE_SAVE_VOLTAGE
                | TRACE_SAVE_NEUROTRANS
                | TRACE_SAVE_SYNAPTIC
                | TRACE_SAVE_PHASE_SPACE)
            ) != 0;

            if (type == NET_TYPE_PURE_IZ && (m & TRACE_SAVE_IZ_RECOVERY)) {
                hasData = true;
            }

            if (type == NET_TYPE_PURE_HH  && (
                m & (
                    TRACE_SAVE_HH_IK
                    | TRACE_SAVE_HH_INA
                    | TRACE_SAVE_HH_ILEAK
                    | TRACE_SAVE_HH_GATE_M
                    | TRACE_SAVE_HH_GATE_H
                    | TRACE_SAVE_HH_GATE_N
                )
            )) hasData = true;

            if (!hasData || args->startDataIdx >= args->endDataIdx) {
                sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
                state->saving.isSaving = false;
                free(args);
                return NULL;
            }
        }

        const char *initialNotes = "Autosave Session";
        if (args->notes[0] != '\0') initialNotes = args->notes;
        else if (args->isFinalSave) initialNotes = "Manual Save";

        sessionId = WorkspaceSessionInsert(state, initialNotes);
        if (sessionId == 0) {
            Logger(ERROR, "Não foi possível criar sessão de backup.");
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
            success = false;
        }
        else state->saving.autoSaveSessionId = sessionId;
    }

    if (success) {
        WorkspaceDetailedSave(state, sessionId, args->startDataIdx, args->endDataIdx);

        if (args->isFinalSave || state->saving.isFinalSaveRequested) {
            const char *notesToSave = args->notes;

            if (!args->isFinalSave) {
                notesToSave = state->ui.saveModal.textBuffer;
            }

            StorageSessionMetadataUpdate(notesToSave, finalTime, sessionId);
        }

        if (sqlite3_exec(db, "COMMIT;", NULL, NULL, &errMsg) != SQLITE_OK) {
            Logger(ERROR, "Erro no Commit: %s", errMsg);
            sqlite3_free(errMsg);
            success = false;
        }
    }

    if (success) {
        state->saving.lastSavedDataCount = args->endDataIdx;

        if (state->saving.isFinalSaveRequested) {
            state->saving.isFinalSaveRequested = false;
        }
    }

    state->saving.isSaving = false;

    free(args);
    return NULL;
}

static inline void WorkspaceDetailedSave(WorkspaceState *state, size_t sessionId, size_t startIdx, size_t endIdx) {
    if (!state->network) return;

    NeuroTraceBuffer *liveData = &state->liveData;
    NetworkType type = NetworkDetectType(state->network);

    size_t nrnCount = state->network->neuronCount;
    size_t nrnLimit = (nrnCount > MAX_DETAILED_PLOTS) ? MAX_DETAILED_PLOTS : nrnCount;

    bool *connectedNeurons = (bool*)calloc(nrnCount, sizeof(bool));
    if (connectedNeurons) {
        for (size_t i = 0; i < nrnCount; i++) {
            SynapseEdge *edge = state->network->neurons[i].outboundSynapses;
            if (edge != NULL) connectedNeurons[i] = true;

            while (edge != NULL) {
                if (edge->targetNeuron < nrnCount) {
                    connectedNeurons[edge->targetNeuron] = true;
                }

                edge = edge->nextSynapse;
            }
        }
    }

    uint32_t m = state->saving.activeTracesMask;

    WorkspaceTraceBatchSave(TBL_NEUROTRANS,  connectedNeurons, (m & TRACE_SAVE_NEUROTRANS),  sessionId, nrnLimit, startIdx, endIdx, liveData->ntFractionTraces);
    WorkspaceTraceBatchSave(TBL_PHASE_SPACE, NULL,             (m & TRACE_SAVE_PHASE_SPACE), sessionId, nrnLimit, startIdx, endIdx, liveData->phaseSpaceTraces);
    WorkspaceTraceBatchSave(TBL_SYNAPTIC,    connectedNeurons, (m & TRACE_SAVE_SYNAPTIC),    sessionId, nrnLimit, startIdx, endIdx, liveData->synapticTraces);
    WorkspaceTraceBatchSave(TBL_VOLTAGE,     NULL,             (m & TRACE_SAVE_VOLTAGE),     sessionId, nrnLimit, startIdx, endIdx, liveData->neuronTraces);

    switch (type) {
        case NET_TYPE_PURE_IZ: {
            WorkspaceTraceBatchSave(TBL_IZ_RECOVERY, NULL, (m & TRACE_SAVE_IZ_RECOVERY), sessionId, nrnLimit, startIdx, endIdx, liveData->izRecoveryTraces);
        } break;

        case NET_TYPE_PURE_HH: {
            WorkspaceTraceBatchSave(TBL_HH_IK,    NULL, (m & TRACE_SAVE_HH_IK),    sessionId, nrnLimit, startIdx, endIdx, liveData->hhCurrentPlots.kCurrent);
            WorkspaceTraceBatchSave(TBL_HH_INA,   NULL, (m & TRACE_SAVE_HH_INA),   sessionId, nrnLimit, startIdx, endIdx, liveData->hhCurrentPlots.naCurrent);
            WorkspaceTraceBatchSave(TBL_HH_ILEAK, NULL, (m & TRACE_SAVE_HH_ILEAK), sessionId, nrnLimit, startIdx, endIdx, liveData->hhCurrentPlots.leakCurrent);

            WorkspaceTraceBatchSave(TBL_HH_GATE_M, NULL, (m & TRACE_SAVE_HH_GATE_M), sessionId, nrnLimit, startIdx, endIdx, liveData->hhGatePlots.MGate);
            WorkspaceTraceBatchSave(TBL_HH_GATE_H, NULL, (m & TRACE_SAVE_HH_GATE_H), sessionId, nrnLimit, startIdx, endIdx, liveData->hhGatePlots.HGate);
            WorkspaceTraceBatchSave(TBL_HH_GATE_N, NULL, (m & TRACE_SAVE_HH_GATE_N), sessionId, nrnLimit, startIdx, endIdx, liveData->hhGatePlots.NGate);
        } break;

        default: break;
    }

    if (connectedNeurons) free(connectedNeurons);
}

static inline void WorkspaceTraceBatchSave(const char *tableName, const bool *connMask, bool shouldSave, size_t sessionId, size_t nrnCount, size_t lastSaved, size_t currentCount, Vector2d **data) {
    if (!shouldSave || !data || lastSaved >= currentCount) return;

    char sql[SQL_CMD_BUFFER_SIZE];
    snprintf(sql, sizeof(sql), SQL_INSERT_TRACE_TEMPLATE, tableName);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, SQL_AUTO_LENGTH, &stmt, NULL) != SQLITE_OK) {
        Logger(ERROR, WS_LOG_ERR_PREPARE_TRACE, tableName, sqlite3_errmsg(db));
        return;
    }

    // Bind constante para toda a tabela
    sqlite3_bind_int64(stmt, SQL_BIND_IDX_1, (int64)sessionId);

    for (size_t n = 0; n < nrnCount; n++) {
        if (connMask && !connMask[n]) continue;
        if (!data[n]) continue;

        size_t s1, s2, total;
        Vector2d *p1, *p2;

        NeuroTraceLinearRangeGet(BUFFER_CAPACITY, currentCount, lastSaved, data[n], &total, &p1, &s1, &p2, &s2);

        size_t sizes[] = { s1, s2 };
        Vector2d *chunks[] = { p1, p2 };

        sqlite3_bind_int(stmt, SQL_BIND_IDX_2, (int)n);

        for (int c = 0; c < 2; c++) {
            if (!chunks[c]) continue;

            for (size_t i = 0; i < sizes[c]; i++) {
                sqlite3_bind_double(stmt, SQL_BIND_IDX_3, chunks[c][i].x);
                sqlite3_bind_double(stmt, SQL_BIND_IDX_4, chunks[c][i].y);

                sqlite3_step(stmt);
                sqlite3_reset(stmt);
            }
        }
    }

    sqlite3_finalize(stmt);
}
