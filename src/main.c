#define RAYGUI_IMPLEMENTATION

#include "raygui.h"
#include "raylib.h"

#include "core/app_lifecycle.h"
#include "core/app_render.h"

#include "workspace/core/workspace_state.h"
#include "workspace/storage/workspace_storage.h"

static Texture2D      GLOBAL_BACKGROUND_TEXTURE;
static WorkspaceState GLOBAL_STATE;

int main (void) {
    AppInitialize(&GLOBAL_STATE, &GLOBAL_BACKGROUND_TEXTURE);

    while(!WindowShouldClose()) {
        if (!GLOBAL_STATE.saving.isSaving) {
            WorkspaceAutoSaveUpdate(&GLOBAL_STATE);
        }

        AppRenderFrame(&GLOBAL_STATE, &GLOBAL_BACKGROUND_TEXTURE);
    }

    // Aguarda a thread terminar silenciosamente (busy-wait curto)
    while(GLOBAL_STATE.saving.isSaving) {}

    AppShutdown(&GLOBAL_STATE, &GLOBAL_BACKGROUND_TEXTURE);

    return 0;
}
