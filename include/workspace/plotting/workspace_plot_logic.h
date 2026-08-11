#ifndef WORKSPACE_PLOT_LOGIC_H
#define WORKSPACE_PLOT_LOGIC_H

#include "workspace/core/workspace_state.h"

void WorkspacePlotLiveBoundsApply(WorkspaceState *state);
void WorkspacePlotStaticBoundsApply(WorkspaceState *state);
void WorkspacePlotTreatmentBoundsApply(WorkspaceState *state);

void WorkspacePlotMultiLinePrepareRange(
    WorkspaceState *state,
    int slot,
    Vector2d **data2D,
    size_t totalCount,
    size_t *outVisStartIdx,
    size_t *outVisEndIdx,
    size_t *outDecimateStep
);

void WorkspacePlotRasterBoundsApply(WorkspaceState *state, int slot);

#endif // WORKSPACE_PLOT_LOGIC_H
