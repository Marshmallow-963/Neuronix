#include "raygui.h"
#include "raylib.h"

#include "core/app_render.h"

#include "storage/core/storage_manager.h"
#include "workspace/config/workspace_config.h"
#include "workspace/config/workspace_ui_config.h"
#include "workspace/logic/workspace_input.h"
#include "workspace/storage/workspace_storage.h"
#include "workspace/themes/worksapce_colors.h"
#include "workspace/ui/workspace_screen.h"

static bool SaveModal(WorkspaceUiState *ui);

void AppRenderFrame(WorkspaceState *state, Texture2D *bgTexture) {
    if (!state) return;

    float x   = X_ORIGIN, y = Y_ORIGIN;
    float w   = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    float bgW = (float)bgTexture->width, bgH = (float)bgTexture->height;

    WorkspaceInputHandle(state);
    WorkspaceFocusHandle(state);

    BeginDrawing();
        ClearBackground(COLOR_BG);

        if (bgTexture->id > 0) {
            Vector2 origin = {x, y};
            Rectangle dstRec = {x, y, w, h};
            Rectangle srcRec = {x, y, bgW, bgH};

            DrawTexturePro(*bgTexture, srcRec, dstRec, origin, 0.0f, WHITE);
            DrawRectangleRec(dstRec, ColorAlpha(COLOR_BG, PANEL_ALPHA));
        }

        WorkspaceScreenDraw(state);

        // --- PROCESSAMENTO DO MODAL (SAVE / DOWNLOAD) ---
        if (state->ui.saveModal.isOpen) {
            if (SaveModal(&state->ui)) {
                // Checa se o acionamento foi para DOWNLOAD/EXPORT
                if (state->ui.saveModal.downloadSessionId != NO_SELECTION) {
                    const char *filePath = state->ui.saveModal.textBuffer;
                    size_t sId = state->ui.saveModal.downloadSessionId;

                    if (state->ui.saveModal.downloadTableName[0] != '\0') {
                        // Exporta apenas a tabela selecionada
                        StorageTableExport(filePath, state->ui.saveModal.downloadTableName, sId);
                    } else {
                        // Exporta a sessão completa
                        StorageSessionExport(filePath, sId);
                    }

                    // Reseta o estado do download
                    state->ui.saveModal.downloadSessionId = NO_SELECTION;
                    state->ui.saveModal.downloadTableName[0] = '\0';
                }
                else {
                    // Caso contrário, executa o salvamento de sessão tradicional
                    bool isAnalyzerActive = (state->session.activeTableId != TABLE_ID_UNKNOWN);
                    if (isAnalyzerActive) WorkspaceStaticDataSave(state);
                    else WorkspaceSaveSession(state);
                }
            }
        }
    EndDrawing();
}

static bool SaveModal(WorkspaceUiState *ui) {
    if (!ui->saveModal.isOpen) return false;

    float modalWidth  = 400.0f;
    float modalHeight = 200.0f;

    Rectangle modalRec = {
        ((float)GetScreenWidth() - modalWidth) / 2.0f,
        ((float)GetScreenHeight() - modalHeight) / 2.0f,
        modalWidth, modalHeight
    };

    DrawRectangleRec(modalRec, ColorAlpha(DARKGRAY, 0.95f));
    DrawRectangleLinesEx(modalRec, FOCUS_THICKNESS, RAYWHITE);

    bool isDownloadMode = (ui->saveModal.downloadSessionId != NO_SELECTION);

    const char *title = isDownloadMode ? "Export / Download File" : "Save Session";
    const char *label = isDownloadMode ? "File name (.csv):" : "Add note or description:";
    const char *btnText = isDownloadMode ? "Export" : "Save";

    if (GuiWindowBox(modalRec, title)) {
        ui->saveModal.isOpen = false;
        ui->saveModal.downloadSessionId = NO_SELECTION; // Reseta estado
    }

    float padX = PADDING * 2.0f;
    float contentW = modalWidth - (padX * 2.0f);
    float currentY = modalRec.y + TAB_HEIGHT + GAP;

    GuiLabel((Rectangle){modalRec.x + padX, currentY, contentW, LBL_HEIGHT}, label);
    currentY += LBL_HEIGHT + GAP;

    size_t size = sizeof(ui->saveModal.textBuffer);
    float textBoxHeight = BTN_HEIGHT + PADDING;
    Rectangle textBoxRec = {modalRec.x + padX, currentY, contentW, textBoxHeight};

    if (GuiTextBox(textBoxRec, ui->saveModal.textBuffer, (int)size, ui->saveModal.editMode)) {
        ui->saveModal.editMode = !(ui->saveModal.editMode);
    }

    float totalBtnWidth = (BTN_WIDTH * 2.0f) + BIG_GAP;
    float btnY   = (modalRec.y + modalHeight) - BTN_HEIGHT - (PADDING * 2.0f); // Ancorado no fundo
    float startX = modalRec.x + ((modalWidth - totalBtnWidth) / 2.0f);

    bool confirmed = GuiButton((Rectangle){startX, btnY, BTN_WIDTH, BTN_HEIGHT}, btnText);
    if (confirmed) ui->saveModal.isOpen = false;

    if (GuiButton((Rectangle){startX + BTN_WIDTH + BIG_GAP, btnY, BTN_WIDTH, BTN_HEIGHT}, "Cancel")) {
        ui->saveModal.isOpen = false;
        ui->saveModal.downloadSessionId = NO_SELECTION; // Reseta estado
    }

    return confirmed;
}
