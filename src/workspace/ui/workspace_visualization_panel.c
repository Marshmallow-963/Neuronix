#include <stdio.h>
#include "raylib.h"

#include "neuro_data/neuro_data_config.h"

#include "workspace/core/workspace_enums.h"
#include "workspace/plotting/workspace_plot_view.h"
#include "workspace/strings/workspace_strings.h"
#include "workspace/topology/graph_renderer.h"
#include "workspace/ui/workspace_net_config.h"
#include "workspace/utils/workspace_components.h"

static inline void DashboardDraw(WorkspaceState *state, Rectangle layout);

void WorkspaceVisualizationPanelDraw(WorkspaceState *state, Rectangle layout) {
    if (!state) return;

    switch (state->ui.activeViewMode) {
        case VIEW_MODE_DASHBOARD:  DashboardDraw(state, layout);       break;
        case VIEW_MODE_NET_CONFIG: NetConfigDraw(state, layout);       break;
        case VIEW_MODE_TOPOLOGY:   NetTopologyDraw(state, layout);     break;
        default: break;
    }

    int posX = (int)layout.x + 10, posY = (int)layout.y + 10;
    if (state->saving.isSaving) {
        DrawText("Autosaving...", posX, posY, 10, SKYBLUE);
    }

    char timeStr[64];
    if (state->ui.activeViewMode == VIEW_MODE_TOPOLOGY) {
        snprintf(timeStr, sizeof(timeStr), "Time: %.0f ms", state->runtime.currentTime);
        DrawText(timeStr, posX, posY + 10, 10, SKYBLUE);
    }
}

static inline void DashboardDraw(WorkspaceState *state, Rectangle layout) {
    if (!state) return;

    // 1. Bloqueio de carregamento imediato (evita renderizar com buffers incompletos)
    if (state->loading.isLoading) {
        StatusMsg(layout, WS_STR_MSG_LOADING_TABLE, state->loading.loadingProgress);
        return;
    }

    // 2. Determinação exata da existência de dados estáticos baseada na tabela ativa
    bool haveStaticData = false;
    bool isTauTable = (state->session.activeTableId == TABLE_ID_TAU);

    if (isTauTable) {
        size_t idx = (state->editor.sourceNeuronId * MAX_DETAILED_PLOTS) + state->editor.targetNeuronId;
        if (state->editor.sourceNeuronId < MAX_DETAILED_PLOTS && state->editor.targetNeuronId < MAX_DETAILED_PLOTS) {
            haveStaticData = (state->staticData.tauPointsCount[idx] > 0);
        }
    } else {
        haveStaticData = (state->staticData.dataCount > 0);
    }

    bool isAnalyzerActive = (state->session.activeTableId != TABLE_ID_UNKNOWN);

    // 3. FLUXO DO ANALISADOR (DADOS CARREGADOS DO BANCO / HISTÓRICOS)
    if (isAnalyzerActive) {
        WorkspaceStaticPlotsDraw(state, layout);

        // Caso o usuário clique em processar dados ou recalcular a matriz
        if (state->analysis.isAnalyzing) {
            DrawRectangleRec(layout, Fade(BLACK, 0.4f));  // Escurece o gráfico estático de leve ao fundo
            StatusMsg(layout, isTauTable ? "Recalculating Tau Data..." : "Processing Data...", 0.0f);
        }
        // Caso específico do Tau selecionado sem pontos calculados ainda
        else if (isTauTable && !haveStaticData) {
            StatusMsg(layout, "No Tau data found. Click 'Tau Analysis' to generate.", 0.0f);
        }

        return;
    }

    // 4. FLUXO DE SIMULAÇÃO EM TEMPO REAL (FALLBACK PADRÃO)
    if (!state->network) {
        StatusMsg(layout, WS_STR_MSG_INIT_OR_SELECT);
        return;
    }

    WorkspaceLivePlotsDraw(state, layout);
}
