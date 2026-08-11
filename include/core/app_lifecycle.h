#ifndef APP_LIFECYCLE_H
#define APP_LIFECYCLE_H

#include "raylib.h"
#include "workspace/core/workspace_state.h"

void AppInitialize(WorkspaceState *state, Texture2D *bgTexture);
void AppShutdown(WorkspaceState *state, Texture2D *bgTexture);

#endif // APP_LIFECYCLE_H
