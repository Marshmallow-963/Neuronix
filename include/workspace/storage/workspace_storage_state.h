#ifndef WORKPACE_STORAGE_STATE_H
#define WORKPACE_STORAGE_STATE_H

#include "workspace/core/workspace_state.h"

extern sqlite3 *db;

typedef struct {
    bool isFinalSave;
    char notes[512];

    size_t startDataIdx;
    size_t endDataIdx;
    size_t startRasterIdx;
    size_t endRasterIdx;

    WorkspaceState *state;
} SaveSessionArgs;

typedef struct {
    const char *tableName;
    size_t offset;
} plotDrawMapEntry;

extern const plotDrawMapEntry plotDrawMap[];
extern const size_t numMappings;

#endif // WORKPACE_STORAGE_STATE_H
