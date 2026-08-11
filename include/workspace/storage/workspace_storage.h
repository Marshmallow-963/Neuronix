#ifndef WORKSPACE_STORAGE_H
#define WORKSPACE_STORAGE_H

#include "workspace/core/workspace_state.h"

Vector2d** WorkspaceTraceArrayGet(NeuroTraceBuffer *buffer, TableId tableId);

void WorkspaceStaticDataSave(WorkspaceState *state);
void WorkspaceTableLoad(WorkspaceState *state, TableId tableId, int64 sessionId);

void WorkspaceAutoSaveUpdate(WorkspaceState *state);
void WorkspaceSaveSession(WorkspaceState *state);

#endif // WORKSPACE_STORAGE_H
