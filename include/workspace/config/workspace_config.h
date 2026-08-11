#ifndef WORKSPACE_CONFIG_H
#define WORKSPACE_CONFIG_H

#include <stdint.h> // Necessário para SIZE_MAX

// =========================================================================
// GENERAL MATH & LOGIC
// =========================================================================
#define BACKPRESSURE_DELAY_MS  5000   // 5ms para o SQLite respirar
#define BACKPRESSURE_THRESHOLD 0.9
#define CAPACITY_ZERO          0
#define COUNT_ZERO             0
#define REFRESH_RATE_MS_IDLE   16000  // ~60fps idle em usleep
#define STR_EQUAL              0

// =========================================================================
// MEMORY ALLOCATIONS & LIMITS
// =========================================================================
#define ASYNC_BATCH_SIZE           128
#define CAPACITY_GROWTH_MULTIPLIER 2
#define DEFAULT_SPIKE_CAPACITY     2048
#define DEFAULT_TAU_CAPACITY       256
#define INITIAL_CAPACITY_EVENTS    1024
#define INITIAL_TRACE_CAPACITY     1024

// =========================================================================
// NEURAL NETWORK GLOBALS & PHYSICS LOGIC
// =========================================================================
#define BASE_NEURON_ID           0
#define DEFAULT_TARGET_NEURON_ID 1
#define INVALID_NEURON_ID        -1
#define INITIAL_NETWORK_CAPACITY 8
#define NO_SELECTION             SIZE_MAX

#define AVG_FALLBACK -1 // Indica média global quando neuronId = -1
#define DT            0.01
#define INITIAL_TIME  200.0
#define ITERACTIONS   32

// =========================================================================
// TEXT BUFFERS & UI LIMITS (ANALYZER / STORAGE)
// =========================================================================
#define MAX_CACHED_SESSIONS 24
#define MAX_EXPANDED_TABLES 32
#define NOTE_BUFFER_LEN     256
#define TABLE_NAME_LEN      64

// =========================================================================
// SIMULATION: ADJACENCY MATRIX (GRID VIEW)
// =========================================================================
#define ADJ_CELL_MAX_FONT_SIZE         20
#define ADJ_CELL_MIN_FONT_SIZE         10
#define ADJ_CELL_MIN_SIZE_FOR_TEXT     10.0f
#define ADJ_CELL_MIN_SIZE_FOR_TRIANGLE 20.0f
#define ADJ_CELL_PAD_VALUE             1.0f
#define ADJ_FONT_SCALE_DIVISOR         3.0f
#define ADJ_LABEL_BUFFER_SIZE          16
#define ADJ_LABEL_LARGE_FONT_SIZE      12
#define ADJ_LABEL_MARGIN_X             8
#define ADJ_LABEL_MARGIN_Y             4
#define ADJ_LABEL_SMALL_FONT_SIZE      10
#define ADJ_MATRIX_CELL_FADE           0.3f
#define ADJ_MATRIX_LABEL_OFFSET        6.0f
#define ADJ_MATRIX_LINE_FADE           0.5f
#define ADJ_TRI_OFFSET_X_LEFT          8
#define ADJ_TRI_OFFSET_X_RIGHT         2
#define ADJ_TRI_OFFSET_Y_BOTTOM        8
#define ADJ_TRI_OFFSET_Y_TOP           2
#define ADJ_WEIGHT_BUFFER_SIZE         16

// =========================================================================
// SIMULATION: ADJACENCY LIST (DETAILED VIEW)
// =========================================================================
#define ADJ_LIST_DETAIL_BUFFER_SIZE 128
#define ADJ_LIST_DETAIL_FONT_SIZE   12
#define ADJ_LIST_FONT_SIZE          14
#define ADJ_LIST_GROUP_SPACING      5.0f
#define ADJ_LIST_HEADER_BUFFER_SIZE 64
#define ADJ_LIST_HEADER_SIZE        20
#define ADJ_LIST_HIDDEN_FONT_SIZE   10
#define ADJ_LIST_LINE_HEIGHT        20.0f
#define ADJ_LIST_LINE_OFFSET        10.0f
#define ADJ_LIST_PADDING            10.0f
#define ADJ_LIST_TITLE_OFFSET       30.0f

#endif // WORKSPACE_CONFIG_H
