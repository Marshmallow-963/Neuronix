#include "core/app_config.h"
#include "core/app_lifecycle.h"

#include "generated/neurons/hh_squid_axon_neuron.h"

#include "generated/neurons/iz_ch_neuron.h"
#include "generated/neurons/iz_fs_neuron.h"
#include "generated/neurons/iz_ib_neuron.h"
#include "generated/neurons/iz_lts_neuron.h"
#include "generated/neurons/iz_re_neuron.h"
#include "generated/neurons/iz_rs_neuron.h"
#include "generated/neurons/iz_tc_neuron.h"

#include "generated/ui/fundoNeuroLab_r800.h"
#include "generated/ui/logoNeuroLab_r800.h"

#include "graph/core/graph_state.h"

#include "log/logger.h"

#include "raylib.h"
#include "raygui.h"

#include "storage/core/storage_manager.h"

#include "workspace/core/workspace_enums.h"
#include "workspace/core/workspace_lifecycle.h"
#include "workspace/logic/workspace_logic.h"

static inline Texture2D LoadTextureFromQOIMemory(const unsigned char *fileData, int dataSize);
static inline void TopologyTexturesLoad(WorkspaceState *state);

void AppInitialize(WorkspaceState *state, Texture2D *bgTexture) {
    SetConfigFlags(FLAG_WINDOW_HIDDEN | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_ALWAYS_RUN | FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_SIZE, WINDOW_SIZE, TXT_APP_TITLE);

    int monitor = GetCurrentMonitor();
    SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));

    GuiLoadStyle(PATH_STYLE_DARK);
    SetTargetFPS(FPS);

    WorkspaceStateLoadDefaults(state);
    WorkspaceThreadsInit(state);
    PlotStateResetAll();
    StorageInit(PATH_DATA);

    PlotStateResetAll();
    Logger(INFO, "Log System Initialized");

    Image logoImg = LoadImageFromMemory(".qoi", logoNeuroLab_r800_qoi, (int)logoNeuroLab_r800_qoi_len);
    if (logoImg.data != NULL) {
        ImageFormat(&logoImg, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        ImageResize(&logoImg, ICON_SIZE, ICON_SIZE);
        SetWindowIcon(logoImg);
        UnloadImage(logoImg);
    }

    *bgTexture = LoadTextureFromQOIMemory(fundoNeuroLab_r800_qoi, (int)fundoNeuroLab_r800_qoi_len);
    TopologyTexturesLoad(state);

    ClearWindowState(FLAG_WINDOW_HIDDEN);
}

void AppShutdown(WorkspaceState *state, Texture2D *bgTexture) {
    WorkspaceThreadsShutdown(state);

    if (bgTexture->id > 0) UnloadTexture(*bgTexture);

    SimulationCleanup(state);
    SimulationDataReset(state);

    StorageClose();
    CloseWindow();
}

static inline void TopologyTexturesLoad(WorkspaceState *state) {
    state->topology.neuronTextures[TEX_HH] = LoadTextureFromQOIMemory(hh_squid_axon_neuron_qoi, (int)hh_squid_axon_neuron_qoi_len);

    state->topology.neuronTextures[TEX_IZ_CH]  = LoadTextureFromQOIMemory(iz_ch_neuron_qoi,  (int)iz_ch_neuron_qoi_len);
    state->topology.neuronTextures[TEX_IZ_FS]  = LoadTextureFromQOIMemory(iz_fs_neuron_qoi,  (int)iz_fs_neuron_qoi_len);
    state->topology.neuronTextures[TEX_IZ_IB]  = LoadTextureFromQOIMemory(iz_ib_neuron_qoi,  (int)iz_ib_neuron_qoi_len);
    state->topology.neuronTextures[TEX_IZ_LTS] = LoadTextureFromQOIMemory(iz_lts_neuron_qoi, (int)iz_lts_neuron_qoi_len);
    state->topology.neuronTextures[TEX_IZ_RE]  = LoadTextureFromQOIMemory(iz_re_neuron_qoi,  (int)iz_re_neuron_qoi_len);
    state->topology.neuronTextures[TEX_IZ_RS]  = LoadTextureFromQOIMemory(iz_rs_neuron_qoi,  (int)iz_rs_neuron_qoi_len);
    state->topology.neuronTextures[TEX_IZ_TC]  = LoadTextureFromQOIMemory(iz_tc_neuron_qoi,  (int)iz_tc_neuron_qoi_len);
}

static inline Texture2D LoadTextureFromQOIMemory(const unsigned char *fileData, int dataSize) {
    Image img = LoadImageFromMemory(".qoi", fileData, dataSize);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}
