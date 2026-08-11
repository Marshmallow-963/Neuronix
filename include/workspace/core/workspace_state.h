#ifndef WORKSPACE_STATE_H
#define WORKSPACE_STATE_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "raylib.h"

// --- Neuro & Math Models ---
#include "model/neural/izhikevich/izhikevich_config.h"
#include "model/synaptic/ampa-gaba-a/ampa_gaba_a_type.h"

#include "network/network_struct.h"
#include "neuro_data/neuro_data_state.h"

// --- Storage ---
#include "storage/config/storage_config.h"
#include "storage/core/storage_state.h"

// --- GUI & Workspace ---
#include "workspace/config/workspace_config.h"
#include "workspace/core/workspace_enums.h"

// ============================================================================
// ESTRUTURAS DE LAYOUT E ESTADO DA TOPOLOGIA
// ============================================================================

typedef struct {
    bool isDragging;      // Para interatividade de clique e arraste
    float scale;          // Caso queira aplicar zoom (default 1.0)
    int neuronIndex;      // Índice correspondente em network->neurons
    Vector2 position;     // Posição central do neurônio na tela
} NeuronLayout;

typedef struct {
    size_t srcIdx;        // Neurônio de origem
    size_t tgtIdx;        // Neurônio de destino
    bool isAmpa;          // Tipo da sinapse (true: AMPA, false: GABA)
    double gMax;          // Condutância máxima
    bool isHovered;       // Indica se o cursor do mouse está sobre esta sinapse
} SynapseLayout;

typedef struct {
    size_t count;                 // Quantidade de neurônios
    size_t synapseCount;          // Quantidade de sinapses registradas
    NeuronLayout *nrnLayouts;     // Array de posições/layouts dos neurônios
    SynapseLayout *synLayouts;    // Array com o estado/layout das sinapses
    int *incomingCounters;        // Contadores para distribuição das linhas
    Texture2D neuronTextures[16]; // Textura de cada tipo de modelo neurônio
} TopologyState;

// =========================================================================
// 1. FOCUS CONTEXTS
// =========================================================================

typedef struct {
    FocusCfg cfgFocus;
    FocusDataTreatment dataFocus;
    FocusEdit editFocus;
    FocusSession dbSessionFocus;
    FocusView viewFocus;
} WorkspaceFocus;

// =========================================================================
// 2. UNIFIED WORKSPACE UI STATE
// =========================================================================

typedef struct {
    char downloadTableName[64];
    char textBuffer[256];

    bool editMode;
    bool isManualSaveTriggered;
    bool isOpen;

    size_t downloadSessionId;
} GuiSaveModalState;

typedef struct {
    // --- Shared / Global UI ---
    bool lockBounds;
    double offset;
    double timeWindow;

    GuiSaveModalState saveModal;
    WorkspaceFocus workFocus;
    WorkspaceMenuState workBtn;

    // --- Simulator Specific UI ---
    WorkspaceViewMode activeViewMode;
    WorkspaceGlobalViewMode globalViewMode;

    size_t pointsToShow;
} WorkspaceUiState;

typedef struct {
    // --- Analyzer Concurrency ---
    volatile bool anaKillThread;
    pthread_mutex_t anaMutex;
    pthread_t anaThread;

    // --- Simulator Concurrency ---
    pthread_mutex_t simMutex;
    pthread_t simThread;
} WorkspaceConcurrency;

// =========================================================================
// 3. WORKSPACE SUB-STATES (Organized Alphabetically)
// =========================================================================

typedef struct {
    bool isAnalyzing;

    size_t analysisChunkSize;
    size_t currentAnalysisIndex;
    size_t fSrcCount;
    size_t *fSrcSpikes;
    size_t fTgtCount;
    size_t *fTgtSpikes;
    size_t lastPairedSrcIdxLatency;
    size_t nSpikesSrc;
    size_t nSpikesTgt;
    size_t sIdxLatency;
    size_t spikeAllocCapacity;
    size_t *srcIdx;
    size_t tauAllocCapacity;
    size_t *tgtIdx;
} WorkspaceAsyncAnalysis;

typedef struct {
    AmpaGabaaSynapseType activeSynapseModel;
    IzNeuronType activeIzhikevichModel;
    NeuronModel activeNeuronModel;

    size_t sourceNeuronId;
    size_t targetNeuronId;
} WorkspaceEditorState;

typedef struct {
    bool isLoading;

    float loadingProgress;

    int maxNeuronIdFound;

    sqlite3_stmt *activeLoadStmt;

    size_t targetDataCount;
    size_t tmpCapacities[MAX_DETAILED_PLOTS];
    size_t tmpCounts[MAX_DETAILED_PLOTS];
} WorkspaceLoadingState;

typedef struct {
    char currentNoteBuffer[NOTE_BUFFER_LEN];
    size_t currentNoteSessionId;
} WorkspaceNotesState;

typedef struct {
    double autapseDelay;
    double externalCurrent;
    double *neuronCurrents;
    double synapseConductancy;
} WorkspaceParameters;

typedef struct {
    double currentTime;
    volatile bool isRunning;
    volatile bool killThread;
} WorkspaceRuntime;

typedef struct {
    bool isSaving;
    bool isFinalSaveRequested;

    uint32_t activeTracesMask;

    size_t autoSaveSessionId;     // Guarda o ID da sessão se estiver aberta
    size_t lastSavedDataCount;    // Último índice salvo dos traços contínuos
    size_t lastSavedRasterCount;  // Último índice salvo dos eventos de raster (spikes)
} WorkspaceSavingState;

typedef struct {
    char expandedTables[MAX_EXPANDED_TABLES][TABLE_NAME_LEN];
    char activeTableName[TABLE_NAME_LEN];
    TableId activeTableId;

    int64 activeSessionId;
    SessionEntry loadedSessions[MAX_CACHED_SESSIONS];

    size_t expandedSessionId;
    size_t expandedTableCount;
    size_t loadedNeuronCount;
    size_t selectedSessionId;
    size_t sessionCount;
} WorkspaceSessionState;

// =========================================================================
// 4. WORKSPACE GLOBAL STATE (The Unified Screen)
// =========================================================================

/**
 * @struct WorkspaceState
 * @brief Estado mestre da aplicação, contendo os módulos unificados de forma plana.
 */
typedef struct {
    WorkspaceUiState ui;

    // --- Data Buffers ---
    NeuroTraceBuffer liveData;
    NeuroTraceBuffer staticData;

    TopologyState topology;

    // --- Network & Simulation ---
    NeuralNetwork *network;
    size_t networkCapacity;
    WorkspaceEditorState editor;
    WorkspaceParameters params;
    WorkspaceRuntime runtime;

    // --- Concurrency (Unificada) ---
    WorkspaceConcurrency concurrency;

    // --- Analysis & Storage ---
    WorkspaceAsyncAnalysis analysis;
    WorkspaceLoadingState loading;
    WorkspaceNotesState notes;
    WorkspaceSavingState saving;
    WorkspaceSessionState session;
} WorkspaceState;

#endif // WORKSPACE_STATE_H
