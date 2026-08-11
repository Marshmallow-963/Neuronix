#ifndef WORKSPACE_PLOT_ENUMS_H
#define WORKSPACE_PLOT_ENUMS_H

#include "storage/config/storage_strings.h"
#include "workspace/core/workspace_enums.h"

typedef enum {
    SLOT_ANALYZER_CURRENT = 0,
    SLOT_ANALYZER_GATE,
    SLOT_ANALYZER_IZ_RECOVERY,
    SLOT_ANALYZER_NT_FRACTION,
    SLOT_ANALYZER_RASTER,
    SLOT_ANALYZER_PHASE,
    SLOT_ANALYZER_SYNPATIC,
    SLOT_ANALYZER_TAU,
    SLOT_ANALYZER_VOLTAGE,

    SLOT_SIMULATOR_CURRENT,
    SLOT_SIMULATOR_GATE,
    SLOT_SIMULATOR_NT_FRACTION,
    SLOT_SIMULATOR_PHASE,
    SLOT_SIMULATOR_IZ_RECOVERY,
    SLOT_SIMULATOR_SYNAPTIC,
    SLOT_SIMULATOR_VOLTAGE,
} Slots;

static inline int WorkspaceTableToSlot(TableId tableId) {
    switch (tableId) {
        case TABLE_ID_IZ_RECOVERY: return SLOT_ANALYZER_IZ_RECOVERY;

        case TABLE_ID_HH_IK:
        case TABLE_ID_HH_INA:
        case TABLE_ID_HH_ILEAK:    return SLOT_ANALYZER_CURRENT;

        case TABLE_ID_HH_GATE_M:
        case TABLE_ID_HH_GATE_H:
        case TABLE_ID_HH_GATE_N:   return SLOT_ANALYZER_GATE;

        case TABLE_ID_NEUROTRANS:  return SLOT_ANALYZER_NT_FRACTION;
        case TABLE_ID_PHASE_SPACE: return SLOT_ANALYZER_PHASE;
        case TABLE_ID_RASTER:      return SLOT_ANALYZER_RASTER;
        case TABLE_ID_SYNAPTIC:    return SLOT_ANALYZER_SYNPATIC;
        case TABLE_ID_TAU:         return SLOT_ANALYZER_TAU;
        case TABLE_ID_VOLTAGE:     return SLOT_ANALYZER_VOLTAGE;
        default:                   return -1;
    }
}

static inline const char* WorkspaceTableIdToString(TableId tableId) {
    switch (tableId) {
        case TABLE_ID_TAU:          return TBL_TAU;
        case TABLE_ID_VOLTAGE:      return TBL_VOLTAGE;
        case TABLE_ID_SYNAPTIC:     return TBL_SYNAPTIC;
        case TABLE_ID_NEUROTRANS:   return TBL_NEUROTRANS;
        case TABLE_ID_IZ_RECOVERY:  return TBL_IZ_RECOVERY;
        case TABLE_ID_PHASE_SPACE:  return TBL_PHASE_SPACE;
        case TABLE_ID_HH_GATE_M:    return TBL_HH_GATE_M;
        case TABLE_ID_HH_GATE_H:    return TBL_HH_GATE_H;
        case TABLE_ID_HH_GATE_N:    return TBL_HH_GATE_N;
        case TABLE_ID_HH_IK:        return TBL_HH_IK;
        case TABLE_ID_HH_INA:       return TBL_HH_INA;
        case TABLE_ID_HH_ILEAK:     return TBL_HH_ILEAK;
        default:                    return "";
    }
}

#endif // WORKSPACE_PLOT_ENUMS_H
