#define _GNU_SOURCE

#include "log/logger.h"

#include "storage/core/storage_enums.h"

#include "workspace/config/workspace_config.h"

#include "workspace/core/workspace_enums.h"
#include "workspace/core/workspace_state.h"
#include "workspace/core/workspace_lifecycle.h"

#include "workspace/logic/workspace_logic.h"

static inline void MutexRecursiveInit(pthread_mutex_t *mutex);
static inline void ThreadAndMutexDestroy(pthread_t *thread, pthread_mutex_t *mutex);

void WorkspaceStateLoadDefaults(WorkspaceState *state) {
    if (!state) return;

    // =========================================================================
    // 1. ANALYSIS & STORAGE INITIALIZATION
    // =========================================================================

    state->analysis = (WorkspaceAsyncAnalysis) {
        .analysisChunkSize       = 0,
        .currentAnalysisIndex    = 0,
        .fSrcCount               = 0,
        .fSrcSpikes              = NULL,
        .fTgtCount               = 0,
        .fTgtSpikes              = NULL,
        .isAnalyzing             = false,
        .lastPairedSrcIdxLatency = 0,
        .nSpikesSrc              = 0,
        .nSpikesTgt              = 0,
        .sIdxLatency             = 0,
        .spikeAllocCapacity      = 0,
        .srcIdx                  = NULL,
        .tauAllocCapacity        = 0,
        .tgtIdx                  = NULL
    };

    state->loading = (WorkspaceLoadingState) {
        .activeLoadStmt   = NULL,
        .isLoading        = false,
        .loadingProgress  = 0.0f,
        .maxNeuronIdFound = 0,
        .targetDataCount  = 0,
        .tmpCapacities   = { 0 },
        .tmpCounts       = { 0 }
    };

    state->notes = (WorkspaceNotesState) {
        .currentNoteBuffer    = { 0 },
        .currentNoteSessionId = NO_SELECTION
    };

    state->saving = (WorkspaceSavingState) {
        .activeTracesMask     = TRACE_SAVE_NONE,
        .autoSaveSessionId    = 0,
        .isFinalSaveRequested = false,
        .isSaving             = false,
        .lastSavedDataCount   = 0,
        .lastSavedRasterCount = 0,
    };

    state->session = (WorkspaceSessionState) {
        .activeSessionId    = (int64)NO_SELECTION,
        .activeTableName    = { 0 },
        .expandedSessionId  = NO_SELECTION,
        .expandedTableCount = 0,
        .expandedTables     = { { 0 } },     // 2 pares (Array 2D)
        .loadedNeuronCount  = 0,
        .loadedSessions     = {
            {
                .id = 0,
                .notes = { 0 },
                .timestamp = { 0 },
                .totalTime = 0.0f,
            }
        },
        .selectedSessionId  = NO_SELECTION,
        .sessionCount       = CAPACITY_ZERO
    };

    state->staticData = (NeuroTraceBuffer) {
        .isCircular       = false, // IMPORTANTE: Analyzer é estático (linear)
        .bufferCapacity   = CAPACITY_ZERO,
        .dataCount        = COUNT_ZERO,

        .hhCurrentPlots = {
            .kCurrent    = { NULL },
            .leakCurrent = { NULL },
            .naCurrent   = { NULL }
        },

        .hhGatePlots = {
            .HGate = { NULL },
            .MGate = { NULL },
            .NGate = { NULL }
        },

        .izRecoveryTraces = { NULL },
        .neuronTraces     = { NULL },
        .ntFractionTraces = { NULL },
        .phaseSpaceTraces = { NULL },
        .synapticTraces   = { NULL },

        .tauTraces = { NULL },
        .rasterTraces = { NULL },
    };

    // =========================================================================
    // 2. SIMULATION & NETWORK INITIALIZATION
    // =========================================================================

    state->concurrency = (WorkspaceConcurrency) {
        .anaKillThread = false
        // Mutexes e Pthreads serão inicializados corretamente via
        // pthread_mutex_init / pthread_create depois, então o resto
        // será zerado implicitamente pelo compilador aqui.
    };

    state->editor = (WorkspaceEditorState) {
        .activeIzhikevichModel = REGULAR_SPIKING,
        .activeNeuronModel     = IZHIKEVICH_MODEL,
        .activeSynapseModel    = AMPA,

        .sourceNeuronId = BASE_NEURON_ID,
        .targetNeuronId = DEFAULT_TARGET_NEURON_ID,
    };

    state->liveData = (NeuroTraceBuffer) {
        .isCircular     = true, // IMPORTANTE: Simulator roda em janela (circular)
        .bufferCapacity = CAPACITY_ZERO,
        .dataCount      = COUNT_ZERO,

        .hhCurrentPlots = {
            .kCurrent    = { NULL },
            .leakCurrent = { NULL },
            .naCurrent   = { NULL }
        },

        .hhGatePlots = {
            .HGate = { NULL },
            .MGate = { NULL },
            .NGate = { NULL }
        },

        .izRecoveryTraces = { NULL },
        .neuronTraces     = { NULL },
        .ntFractionTraces = { NULL },
        .phaseSpaceTraces = { NULL },
        .synapticTraces   = { NULL },

        // Não são usados no liveData
        .rasterTraces = { NULL },
        .tauTraces = { NULL },
    };

    state->network         = NULL;
    state->networkCapacity = 8;

    state->params = (WorkspaceParameters) {
        .autapseDelay       = 1.0,
        .externalCurrent    = 10.0,
        .neuronCurrents     = NULL,
        .synapseConductancy = 0.3
    };

    state->runtime = (WorkspaceRuntime) {
        .currentTime = 0.00,
        .isRunning   = false,
        .killThread  = false
    };

    state->topology = (TopologyState){
        .nrnLayouts       = NULL,
        .incomingCounters = NULL,
        .synLayouts       = NULL,
        .count            = 0,
        .synapseCount     = 0,
        .neuronTextures   = { { 0 } },
    };

    // =========================================================================
    // 3. WORKSPACE UNIFIED UI INITIALIZATION
    // =========================================================================

    state->ui = (WorkspaceUiState) {
        // --- Shared / Global UI ---
        .lockBounds = true,
        .offset = 0.0,
        .timeWindow = 50.0,

        .saveModal  = { 0 },
        .workBtn    = 0,

        .pointsToShow = 100,

        // --- Simulator Specific UI ---
        .activeViewMode = VIEW_MODE_TOPOLOGY,
        .globalViewMode = VIEW_GLOBAL_DETAILED_PLOT,

        // --- Workspace Focus Ctx ---
        .workFocus = (WorkspaceFocus) {
            .dataFocus      = FOCUS_DATA_NONE,
            .dbSessionFocus = 0,
            .editFocus      = FOCUS_EDIT_NONE,
            .viewFocus      = FOCUS_VIEW_NONE
        }
    };
}

void WorkspaceThreadsInit(WorkspaceState *state) {
    if (!state) return;

    MutexRecursiveInit(&state->concurrency.simMutex);
    MutexRecursiveInit(&state->concurrency.anaMutex);

    state->concurrency.anaKillThread = false;
    state->runtime.killThread = false;

    // Disparo das threads de trabalho
    pthread_create(&state->concurrency.anaThread, NULL, AnalyzerWorkerThread, state);
    pthread_create(&state->concurrency.simThread, NULL, SimulationWorkerThread, state);

    Logger(INFO, "Workspace Threads and Mutexes initialized.");
}

void WorkspaceThreadsShutdown(WorkspaceState *state) {
    if (!state) return;

    // Finaliza Analyzer
    state->concurrency.anaKillThread = true;
    state->runtime.killThread = true;

    ThreadAndMutexDestroy(&state->concurrency.anaThread, &state->concurrency.anaMutex);
    ThreadAndMutexDestroy(&state->concurrency.simThread, &state->concurrency.simMutex);

    Logger(INFO, "Workspace Threads and Mutexes destroyed.");
}

static inline void MutexRecursiveInit(pthread_mutex_t *mutex) {
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

static inline void ThreadAndMutexDestroy(pthread_t *thread, pthread_mutex_t *mutex) {
    if (thread && *thread != 0) {
        pthread_join(*thread, NULL);
        *thread = 0;
    }

    if (mutex) pthread_mutex_destroy(mutex);
}
