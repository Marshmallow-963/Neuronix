#ifndef WORKSPACE_LIFECYCLE_H
#define WORKSPACE_LIFECYCLE_H

#include "workspace/core/workspace_state.h"

void WorkspaceStateLoadDefaults(WorkspaceState *state);

void WorkspaceThreadsInit(WorkspaceState *state);
void WorkspaceThreadsShutdown(WorkspaceState *state);

#endif // WORKSPACE_LIFECYCLE_H
