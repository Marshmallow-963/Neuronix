#include "workspace/config/workspace_ui_config.h"

#include "workspace/ui/workspace_control_panel.h"
#include "workspace/ui/workspace_screen.h"
#include "workspace/ui/workspace_visualization_panel.h"

#include "workspace/utils/workspace_components.h"

void WorkspaceScreenDraw(WorkspaceState *state) {
    if (!state) return;

    float screenW = (float)GetScreenWidth(), screenH = (float)GetScreenHeight();
    Rectangle fullScreenLayout = { X_ORIGIN, Y_ORIGIN, screenW, screenH };

    WorkspaceVisualizationPanelDraw(state, fullScreenLayout);
    PerformanceVerify(fullScreenLayout);
    WorkspaceControlPanelDraw(state, fullScreenLayout);
}
