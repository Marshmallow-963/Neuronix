#include "raylib.h"

#include "workspace/core/workspace_enums.h"
#include "workspace/core/workspace_state.h"
#include "workspace/logic/workspace_actions.h"
#include "workspace/logic/workspace_logic.h"
#include "workspace/logic/workspace_input.h"

static inline void WorkspaceDataSave(WorkspaceState *state);

void WorkspaceInputHandle(WorkspaceState *state) {
    if (!state) return;

    // Tempo decorrido desde o último frame (usado para o deslizamento suave)
    float dt = GetFrameTime();

    // Modificadores e Teclas de Controle
    bool isCtrlDown = IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_CONTROL);
    bool isF1Down   = IsKeyDown(KEY_F1);
    bool isF2Down   = IsKeyDown(KEY_F2);
    bool isF3Down   = IsKeyDown(KEY_F3);
    bool isTDown    = IsKeyDown(KEY_T);
    bool isZDown    = IsKeyDown(KEY_Z);

    // Gatilhos de Pressionamento Único (Toque único / Tap)
    bool isEnterPressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
    bool isCPressed     = IsKeyPressed(KEY_C);
    bool isDPressed     = IsKeyPressed(KEY_D);
    bool isSPressed     = IsKeyPressed(KEY_S);
    bool isTPressed     = IsKeyPressed(KEY_T);
    bool isOPressed     = IsKeyPressed(KEY_O);
    bool isRPressed     = IsKeyPressed(KEY_R);
    bool isEqualPressed = IsKeyPressed(KEY_EQUAL);
    bool isMinusPressed = IsKeyPressed(KEY_MINUS);

    // Gatilhos de Pressionamento Contínuo (Segurar / Hold)
    bool isEqualDown = IsKeyDown(KEY_EQUAL);
    bool isMinusDown = IsKeyDown(KEY_MINUS);

    // Gatilhos de Repetição Padrão (Ideal para pular valores inteiros)
    bool isEqualRepeat = IsKeyPressedRepeat(KEY_EQUAL);
    bool isMinusRepeat = IsKeyPressedRepeat(KEY_MINUS);

    // --- Ações de UI e Workspace ---
    if (isCtrlDown && isDPressed) state->ui.activeViewMode = VIEW_MODE_DASHBOARD;
    if (isCtrlDown && isCPressed) state->ui.activeViewMode = VIEW_MODE_NET_CONFIG;

    if (isCtrlDown && isOPressed)     AutoSaveToggle(state);
    if (isCtrlDown && isRPressed)     SimulationDataReset(state);
    if (isCtrlDown && isEnterPressed) SimulationToggle(state);
    if (isCtrlDown && isSPressed)     WorkspaceDataSave(state);

    // --- Controle do Analyzer ---
    bool isAnalyzerActive = (state->session.activeTableId != TABLE_ID_UNKNOWN);
    if (isAnalyzerActive) {
        // Usa "Repeat" para lidar com inteiros em incrementos inteiros de 10
        if (isCtrlDown && isTDown) {
            if (isEqualRepeat) state->ui.pointsToShow += 10;
            if (isMinusRepeat) state->ui.pointsToShow -= 10;
        }
    } else {
        if (isCtrlDown && isTPressed) state->ui.activeViewMode = VIEW_MODE_TOPOLOGY;
    }

    // --- Controle de Zoom (Float) ---
    if (isCtrlDown && isZDown) {
        float zoomDelta = 0.0f;

        // Se clicou (1 vez), pula 5. Se está segurando, desliza suavemente 20 unidades por segundo
        if (isEqualPressed) zoomDelta = -5.0f;
        else if (isEqualDown) zoomDelta = -20.0f * dt;

        if (isMinusPressed) zoomDelta = 5.0f;
        else if (isMinusDown) zoomDelta = 20.0f * dt;

        // Aplica o Zoom se houve mudança
        if (zoomDelta != 0.0f) {
            state->ui.timeWindow += zoomDelta;

            if (state->ui.timeWindow <= 2.5f) {
                state->ui.timeWindow = 2.5f; // Limite min
                state->ui.lockBounds = true;
            } else if (state->ui.timeWindow >= 50.0f) {
                state->ui.timeWindow = 50.0f; // Limite max
                state->ui.lockBounds = false; // Retorna ao Auto-Scale
            } else {
                state->ui.lockBounds = true; // Mantém travado em zoom intermediário
            }
        }
    }

    // --- Controle de Parâmetros Contínuos (Float) ---
    float paramSpeed = 0.05f; // Taxa de alteração por segundo ao segurar a tecla

    if (isF1Down) {
        if (isEqualPressed) state->params.autapseDelay += 0.01f;
        else if (isEqualDown) state->params.autapseDelay += paramSpeed * dt;

        if (isMinusPressed) state->params.autapseDelay -= 0.01f;
        else if (isMinusDown) state->params.autapseDelay -= paramSpeed * dt;
    }

    if (isF2Down) {
        if (isEqualPressed) state->params.externalCurrent += 0.01f;
        else if (isEqualDown) state->params.externalCurrent += paramSpeed * dt;

        if (isMinusPressed) state->params.externalCurrent -= 0.01f;
        else if (isMinusDown) state->params.externalCurrent -= paramSpeed * dt;
    }

    if (isF3Down) {
        if (isEqualPressed) state->params.synapseConductancy += 0.01f;
        else if (isEqualDown) state->params.synapseConductancy += paramSpeed * dt;

        if (isMinusPressed) state->params.synapseConductancy -= 0.01f;
        else if (isMinusDown) state->params.synapseConductancy -= paramSpeed * dt;
    }
}

void WorkspaceFocusHandle(WorkspaceState *state) {
    if (!state) return;

    bool isUpPressed   = IsKeyPressed(KEY_UP);
    bool isDownPressed = IsKeyPressed(KEY_DOWN);
    bool isRightSetPressed = IsKeyPressed(KEY_RIGHT);

    if (!isUpPressed && !isDownPressed) return;

    if (state->ui.workBtn == MENU_NONE) {
        if (isUpPressed) state->ui.workBtn = MENU_EDIT;
        else if (isDownPressed) state->ui.workBtn = MENU_CONFIG;
        return;
    } else if (state->ui.workBtn != MENU_NONE) {
        if (isRightSetPressed) state->ui.workBtn = MENU_NONE;
    }

    if (isUpPressed) {
        if (state->ui.workBtn == MENU_CONFIG) state->ui.workBtn = MENU_EDIT;
        else state->ui.workBtn--;
    } else if (isDownPressed) {
        if (state->ui.workBtn == MENU_EDIT) state->ui.workBtn = MENU_CONFIG;
        else state->ui.workBtn++;
    }
}

static inline void WorkspaceDataSave(WorkspaceState *state) {
    if (!state) return;

    state->ui.saveModal.isOpen        = true;
    state->ui.saveModal.editMode      = true;
    state->ui.saveModal.textBuffer[0] = '\0';
}
