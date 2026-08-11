#include <stdio.h>
#include <string.h>

#include "log/logger.h"
#include "neuro_data/neuro_data_trace.h"

#include "storage/core/storage_manager.h"

#include "workspace/core/workspace_state.h"
#include "workspace/logic/workspace_actions.h"
#include "workspace/strings/workspace_strings.h"

/* =========================================================================
* ANALYZER ACTIONS
* ========================================================================= */

static inline void TraceReset(WorkspaceState *state);

void RasterPlotDraw(WorkspaceState *state, size_t chunkSize) {
    if (!state) return;

    size_t count = state->staticData.dataCount;
    if (count == 0) {
        Logger(WARNING, "Insufficient data to calculate Raster Plot.");
        return;
    }

    pthread_mutex_lock(&state->concurrency.anaMutex);
        state->analysis.currentAnalysisIndex = 2;
        state->analysis.isAnalyzing = true;
    pthread_mutex_unlock(&state->concurrency.anaMutex);

    Logger(INFO, WS_LOG_INFO_ASYNC_ANALYSIS, state->session.activeTableName, chunkSize);
}

void SessionDelete(WorkspaceState *state, size_t sessionID) {
    if (!state) return;

    if (StorageSessionDelete(sessionID)) {
        state->session.sessionCount = 0;

        if (state->session.expandedSessionId == sessionID) {
            state->session.expandedSessionId = NO_SELECTION;
            state->session.expandedTableCount = 0;
            TraceReset(state);
        }

        if (state->notes.currentNoteSessionId == sessionID) {
            state->notes.currentNoteSessionId = NO_SELECTION;
            state->notes.currentNoteBuffer[0] = '\0';
        }
    }
}

void SessionDownload(WorkspaceState *state, size_t sessionID) {
    if (!state) return;

    state->ui.saveModal.isOpen = true;
    state->ui.saveModal.editMode = true;
    state->ui.saveModal.downloadSessionId = sessionID;
    state->ui.saveModal.downloadTableName[0] = '\0'; // String vazia = Sessão inteira

    // Sugere um nome de arquivo padrão
    snprintf(state->ui.saveModal.textBuffer, sizeof(state->ui.saveModal.textBuffer), "session_%zu_export.csv", sessionID);
}

void TableDelete(WorkspaceState *state, const char *tableName, size_t sessionId) {
    if (!state) return;

    if (StorageSessionDataTableDelete(tableName, sessionId)) {
        Logger(INFO, "No data from Table %s successfully removed from session %zu", tableName, sessionId);
        if (strcmp(state->session.activeTableName, tableName) == 0) TraceReset(state);

        state->session.expandedTableCount = StorageSessionDataTablesGet(state->session.expandedTables, 20, sessionId);
        if (state->session.expandedTableCount == 0) state->session.expandedSessionId = NO_SELECTION;
    }
}

void TableDownload(WorkspaceState *state, const char *tableName, size_t sessionID) {
    if (!state || !tableName) return;

    state->ui.saveModal.isOpen = true;
    state->ui.saveModal.editMode = true;
    state->ui.saveModal.downloadSessionId = sessionID;
    snprintf(state->ui.saveModal.downloadTableName, sizeof(state->ui.saveModal.downloadTableName), "%s", tableName);

    // Sugere um nome de arquivo padrão contendo a tabela e sessão
    snprintf(state->ui.saveModal.textBuffer, sizeof(state->ui.saveModal.textBuffer), "%s_session_%zu.csv", tableName, sessionID);
}

void SpikeTimingCalculate(WorkspaceState *state, size_t chunkSize) {
    if (!state) return;

    size_t srcId = state->editor.sourceNeuronId;
    size_t tgtId = state->editor.targetNeuronId;
    size_t count = state->staticData.dataCount;

    if (srcId >= MAX_DETAILED_PLOTS || tgtId >= MAX_DETAILED_PLOTS) {
        Logger(ERROR, WS_LOG_ERR_NEURON_RANGE);
        return;
    }

    Vector2d *traceSrc = state->staticData.neuronTraces[srcId];
    Vector2d *traceTgt = state->staticData.neuronTraces[tgtId];

    size_t matrixIdx = (srcId * MAX_DETAILED_PLOTS) + tgtId;

    if (!traceSrc || !traceTgt || count == 0) {
        Logger(WARNING, WS_LOG_WARN_INSUFFICIENT_DATA, srcId, tgtId);
        return;
    }

    pthread_mutex_lock(&state->concurrency.anaMutex);
        state->analysis.currentAnalysisIndex = 0;
        state->analysis.isAnalyzing = true;

        state->staticData.tauPointsCount[matrixIdx] = 0;
    pthread_mutex_unlock(&state->concurrency.anaMutex);

    Logger(INFO, WS_LOG_INFO_ASYNC_ANALYSIS, state->session.activeTableName, chunkSize);
}

static inline void TraceReset(WorkspaceState *state) {
    if (!state) return;

    state->session.activeTableId = TABLE_ID_UNKNOWN;
    state->session.activeSessionId = NO_SELECTION;

    pthread_mutex_lock(&state->concurrency.anaMutex);
        state->analysis.isAnalyzing = false;
        state->analysis.currentAnalysisIndex = 0;
        state->analysis.tauAllocCapacity = 0;

        NeuroTraceCleanup(&state->staticData);
    pthread_mutex_unlock(&state->concurrency.anaMutex);
}
