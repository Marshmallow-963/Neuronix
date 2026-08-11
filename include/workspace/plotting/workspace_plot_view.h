#ifndef WORKSPACE_PLOT_VIEW_H
#define WORKSPACE_PLOT_VIEW_H

#include "network/network_enum.h"

#include "workspace/core/workspace_state.h"
#include "workspace/core/workspace_enums.h"
#include "workspace/strings/workspace_strings.h"
#include "workspace/plotting/workspace_plot_enums.h"

// =========================================================================
// HELPERS DE RÓTULOS
// =========================================================================

static inline const char* WorkspaceTableToYLabel(TableId tableId) {
    switch (tableId) {
        case TABLE_ID_VOLTAGE:     return WS_STR_GRAPH_POTENTIAL_MV;
        case TABLE_ID_SYNAPTIC:    return WS_STR_GRAPH_SYNAPTIC_PA;
        case TABLE_ID_NEUROTRANS:  return WS_STR_GRAPH_R_CHANNEL;
        case TABLE_ID_IZ_RECOVERY: return WS_STR_GRAPH_RECOVERY_U;
        case TABLE_ID_HH_GATE_M:
        case TABLE_ID_HH_GATE_H:
        case TABLE_ID_HH_GATE_N:   return WS_STR_GRAPH_PROB_0_1;
        case TABLE_ID_HH_IK:
        case TABLE_ID_HH_INA:
        case TABLE_ID_HH_ILEAK:    return WS_STR_GRAPH_CURRENT_PA;
        default:                   return "";
    }
}

static inline const char* WorkspaceSlotToYLabel(int slot, NetworkType netType) {
    switch (slot) {
        case SLOT_SIMULATOR_VOLTAGE:     return WS_STR_GRAPH_POTENTIAL_MV;
        case SLOT_SIMULATOR_IZ_RECOVERY: return WS_STR_GRAPH_RECOVERY_U;
        case SLOT_SIMULATOR_NT_FRACTION: return WS_STR_GRAPH_R_CHANNEL;
        case SLOT_SIMULATOR_SYNAPTIC:    return WS_STR_GRAPH_SYNAPTIC_PA;
        case SLOT_SIMULATOR_CURRENT:     return WS_STR_GRAPH_CURRENT_PA;
        case SLOT_SIMULATOR_GATE:        return WS_STR_GRAPH_GATE_N;
        case SLOT_SIMULATOR_PHASE:
            return (netType == NET_TYPE_PURE_IZ) ? WS_STR_GRAPH_RECOVERY_U : WS_STR_GRAPH_GATE_N;
        default:                         return "";
    }
}

// =========================================================================
// PROTÓTIPOS DE RENDERIZAÇÃO
// =========================================================================

void WorkspaceLivePlotsDraw(WorkspaceState *state, Rectangle layout);
void WorkspaceStaticPlotsDraw(WorkspaceState *state, Rectangle layout);

#endif // WORKSPACE_PLOT_VIEW_H
