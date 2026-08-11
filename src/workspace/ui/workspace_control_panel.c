#include <stdio.h>
#include <string.h>

#include "raylib.h"
#include "raygui.h"

#include "model/neural/neuron_models.h"

#include "storage/config/storage_strings.h"
#include "storage/core/storage_manager.h"
#include "storage/core/storage_state.h"

#include "workspace/config/workspace_ui_config.h"
#include "workspace/core/workspace_enums.h"
#include "workspace/core/workspace_state.h"

#include "workspace/logic/workspace_actions.h"
#include "workspace/storage/workspace_storage.h"
#include "workspace/strings/workspace_strings.h"

#include "workspace/themes/worksapce_colors.h"
#include "workspace/ui/workspace_control_panel.h"
#include "workspace/utils/workspace_components.h"

static inline bool SessionItemDraw(WorkspaceState *state, Rectangle rec, SessionEntry *session, size_t index);
static inline bool SessionTablesDraw(WorkspaceState *state, Rectangle rec, size_t sessionID);

static inline TableId WorkspaceStringToTableId(const char *tableName);

static inline void CfgContentDraw(WorkspaceState *state, Rectangle layout);
static inline void DataBaseContentDraw(WorkspaceState *state, Rectangle layout);
static inline void DataTratementContentDraw(WorkspaceState *state, Rectangle layout);
static inline void EditContentDraw(WorkspaceState *state, Rectangle layout);

static inline void SideBarDraw(WorkspaceState *state, Rectangle sideBarRec);
static inline void SubMenuActiveDraw(WorkspaceState *state, Rectangle sidebarRec, float offset);

void WorkspaceControlPanelDraw(WorkspaceState *state, Rectangle layout) {
    if (!state) return;

    static float menuSlideOffset = 0.0f;
    static float sidebarSlideOffset = SIDEBAR_BTN_SIZE;

    bool isMenuOpen = (state->ui.workBtn != MENU_NONE);

    float triggerZoneWidth = SIDEBAR_BTN_SIZE + 20.0f;
    bool isMouseNear = GetMousePosition().x >= (layout.x + layout.width - triggerZoneWidth);

    float targetSidebarOffset = (isMenuOpen || isMouseNear) ? 0.0f : SIDEBAR_BTN_SIZE;

    if (sidebarSlideOffset < targetSidebarOffset) sidebarSlideOffset += 15.0f;
    else if (sidebarSlideOffset > targetSidebarOffset) sidebarSlideOffset -= 15.0f;

    if (sidebarSlideOffset > SIDEBAR_BTN_SIZE) sidebarSlideOffset = SIDEBAR_BTN_SIZE;
    if (sidebarSlideOffset < 0.0f) sidebarSlideOffset = 0.0f;

    float targetMenuOffset = isMenuOpen ? SUBMENU_WIDTH : 0.0f;
    if (menuSlideOffset < targetMenuOffset) menuSlideOffset += 25.0f;
    else if (menuSlideOffset > targetMenuOffset) menuSlideOffset -= 25.0f;

    if (menuSlideOffset > SUBMENU_WIDTH) menuSlideOffset = SUBMENU_WIDTH;
    if (menuSlideOffset < 0.0f) menuSlideOffset = 0.0f;

    Rectangle sidebarRect = {
        layout.x + layout.width - SIDEBAR_BTN_SIZE + sidebarSlideOffset,
        layout.y, SIDEBAR_BTN_SIZE, layout.height
    };

    SideBarDraw(state, sidebarRect);

    if (menuSlideOffset > 0) {
        SubMenuActiveDraw(state, sidebarRect, menuSlideOffset);
    }
}

static inline void SideBarDraw(WorkspaceState *state, Rectangle sideBarRec) {
    if (!state) return;

    static const struct {
        int icon;
        int state;
        const char *tooltip;
    } buttons[] = {
        {ICON_GEAR,            MENU_CONFIG,         "Configurations"},
        {ICON_LENS,            MENU_DATABASE,       WS_TIP_DATABASE},
        {ICON_FILETYPE_BINARY, MENU_DATA_TRATAMENT, WS_TIP_DATA_TRATAMENT},
        {ICON_PENCIL,          MENU_EDIT,           WS_TIP_EDIT_NET},
    };

    int btnCount = sizeof(buttons) / sizeof(buttons[0]);
    float startX = sideBarRec.x, startY = sideBarRec.y;

    for (int i = 0; i < btnCount; i++) {
        Rectangle btnRec = {
            startX,
            startY + ((float)i * SIDEBAR_BTN_SIZE),
            SIDEBAR_BTN_SIZE,
            SIDEBAR_BTN_SIZE
        };

        bool isActive = (state->ui.workBtn == (WorkspaceMenuState)buttons[i].state);
        GuiSetState(isActive ? STATE_PRESSED : STATE_NORMAL);
            if (GuiButton(btnRec, GuiIconText(buttons[i].icon, NULL))) {
                if (isActive) state->ui.workBtn = MENU_NONE;

                else {
                    state->ui.workBtn = (WorkspaceMenuState)buttons[i].state;

                    if (state->ui.workBtn == MENU_DATABASE) {
                        state->session.sessionCount = 0;
                        state->session.expandedSessionId = NO_SELECTION;
                    }
                }
            }
        GuiSetState(STATE_NORMAL);

        if (CheckCollisionPointRec(GetMousePosition(), btnRec) && !isActive) {
            int txtWidth = MeasureText(buttons[i].tooltip, BTN_FONTSIZE);

            float x = btnRec.x - (float)txtWidth - PADDING;
            float y = btnRec.y + BIG_GAP;
            float w = (float)txtWidth + PADDING;

            Rectangle tooltipRec = {x, y, w, LBL_HEIGHT};

            DrawRectangleRec(tooltipRec, BLACK);

            DrawText(
                buttons[i].tooltip, (int)(tooltipRec.x + GAP),
                (int)(tooltipRec.y + GAP), BTN_FONTSIZE, RAYWHITE
            );
        }
    }
}

static inline void SubMenuActiveDraw(WorkspaceState *state, Rectangle sidebarRec, float offset) {
    if (!state) return;

    // O menu aparece à esquerda da sidebar, movendo-se com o offset
    float drawerRecX = sidebarRec.x - offset;
    Rectangle drawerRec = { drawerRecX, sidebarRec.y, SUBMENU_WIDTH, sidebarRec.height };

    DrawRectangleRec(drawerRec, COLOR_BG);
    DrawRectangleLinesEx(drawerRec, 1, COLOR_BORDER);

    // Recorta o conteúdo para não vazar se o menu estiver parcialmente aberto
    BeginScissorMode((int)drawerRec.x, (int)drawerRec.y, (int)drawerRec.width, (int)drawerRec.height);

    Rectangle contentRect = {
        drawerRec.x + PADDING,
        drawerRec.y + PADDING,
        drawerRec.width - (PADDING * 2),
        drawerRec.height - (PADDING * 2)
    };

    switch (state->ui.workBtn) {
        case MENU_CONFIG:         CfgContentDraw(state, contentRect);         break;
        case MENU_DATABASE:       DataBaseContentDraw(state, contentRect);    break;
        case MENU_DATA_TRATAMENT: DataTratementContentDraw(state, contentRect); break;
        case MENU_EDIT:           EditContentDraw(state, contentRect);        break;
        default: break;
    }

    EndScissorMode();
}

static inline bool SessionItemDraw(WorkspaceState *state, Rectangle rec, SessionEntry *session, size_t index) {
    if (!state) return false;

    float h = rec.height;
    float mainW = rec.width - (h * 2) - GAP;

    Rectangle rectMain = { rec.x, rec.y, mainW, h };
    Rectangle rectDow  = { rec.x + mainW + GAP, rec.y, h, h };
    Rectangle rectDel  = { rec.x + mainW + h + GAP * 2, rec.y, h, h };

    bool isFocused = (state->ui.workFocus.dbSessionFocus == (SESSION_1 + index));
    bool isExpanded = (state->session.expandedSessionId == session->id);

    char label[64];
    snprintf(label, sizeof(label), "%s", session->timestamp);
    int icon = isExpanded ? ICON_ARROW_DOWN_FILL : ICON_ARROW_RIGHT_FILL;

    if (Button(rectMain, icon, label, isFocused)) {
        if (isExpanded) {
            state->session.expandedSessionId = NO_SELECTION;
        } else {
            state->session.expandedSessionId = session->id;
            state->session.expandedTableCount = StorageSessionDataTablesGet(state->session.expandedTables, MAX_EXPANDED_TABLES, session->id);
        }
    }

    Rectangle subRec = { rec.x, rec.y + h + GAP, rec.width, h };

    // =========================================================================
    // DESENHO DIRETO DA TOOLTIP À ESQUERDA DO MENU
    // =========================================================================
    if (CheckCollisionPointRec(GetMousePosition(), rectMain)) {
        char notesBuffer[256] = {0};

        if (StorageSessionNotesGet(session->id, notesBuffer, sizeof(notesBuffer))) {
            if (notesBuffer[0] != '\0') {
                int txtWidth = MeasureText(notesBuffer, BTN_FONTSIZE);
                float tw = (float)txtWidth + (PADDING * 2);
                float th = rectMain.height;

                // A borda esquerda da gaveta do menu fica exatamente em (rec.x - PADDING)
                float menuLeftX = rec.x - PADDING;
                float tx = menuLeftX - tw - GAP;
                float ty = rectMain.y;

                Rectangle tooltipRec = { tx, ty, tw, th };

                // Finaliza o ScissorMode para permitir o desenho fora dos limites do menu
                EndScissorMode();

                // Desenha o retângulo e o texto no espaço livre à esquerda
                DrawRectangleRec(tooltipRec, BLACK);
                DrawRectangleLinesEx(tooltipRec, 1.0f, COLOR_BORDER);
                DrawText(
                    notesBuffer,
                    (int)(tx + PADDING),
                    (int)(ty + (th - BTN_FONTSIZE) * 0.5f),
                    BTN_FONTSIZE,
                    RAYWHITE
                );
            }
        }
    }

    if (GuiButton(rectDow, GuiIconText(ICON_FILE_SAVE_CLASSIC, NULL))) {
        SessionDownload(state, session->id);
    }

    if (GuiButton(rectDel, GuiIconText(ICON_BIN, NULL))) {
        SessionDelete(state, session->id);
        return false;
    }

    if (isExpanded) {
        if (!SessionTablesDraw(state, subRec, session->id)) return false;
    }

    return true;
}

static inline bool SessionTablesDraw(WorkspaceState *state, Rectangle rec, size_t sessionID) {
    if (!state) return false;

    float h = rec.height;
    float mainW = rec.width - (h * 2) - GAP;

    for (size_t t = 0; t < state->session.expandedTableCount; t++) {

        Rectangle rectMain = { rec.x, rec.y, mainW, h };
        Rectangle rectDow  = { rec.x + mainW + GAP, rec.y, h, h };
        Rectangle rectDel  = { rec.x + mainW + h + GAP * 2, rec.y, h, h };

        if (Button(rectMain, ICON_WAVE, state->session.expandedTables[t], false)) {
            TableId clickedTableId = WorkspaceStringToTableId(state->session.expandedTables[t]);

            if (clickedTableId != TABLE_ID_UNKNOWN) {
                WorkspaceTableLoad(state, clickedTableId, (int64)sessionID);
            }
        }

        if (GuiButton(rectDow, GuiIconText(ICON_FILE_SAVE_CLASSIC, NULL))) {
            TableDownload(state, state->session.expandedTables[t], sessionID);
        }

        if (GuiButton(rectDel, GuiIconText(ICON_BIN, NULL))) {
            TableDelete(state, state->session.expandedTables[t], sessionID);
            return false;
        }

        rec.y += h + GAP;
    }
    return true;
}

static inline void CfgContentDraw(WorkspaceState *state, Rectangle layout) {
    if (!state) return;

    float x = layout.x, y = layout.y, w = layout.width, h = BTN_HEIGHT;
    LabelCentered((Rectangle){x, y, w, h}, "GRAPH CONFIGURATIONS");
    y += h + GAP;

    for (size_t i = 0; i < CONFIG_TRACE_TABLES.count; i++) {
        uint32_t currentBit = (1 << i);
        bool isEnabled = (state->saving.activeTracesMask & currentBit) != 0;

        char btnLabel[128];
        snprintf(btnLabel, sizeof(btnLabel), "%s %s", isEnabled ? "[X]" : "[ ]", CONFIG_TRACE_TABLES.names[i]);

        // Substitui '_' por ' ' apenas para melhor leitura humana na UI
        for (size_t j = 0; btnLabel[j] != '\0'; j++) {
            if (btnLabel[j] == '_') btnLabel[j] = ' ';
        }

        if (Button((Rectangle){x, y, w, h}, ICON_NONE, btnLabel, false)) {
            // Aplica a operação XOR (^=) para inverter o estado do bit ao clicar
            state->saving.activeTracesMask ^= currentBit;
        }

        y += h + GAP;
    }
}

static inline void DataBaseContentDraw(WorkspaceState *state, Rectangle layout) {
    if (!state) return;

    float x = layout.x, y = layout.y, w = layout.width, h = BTN_HEIGHT;
    LabelCentered((Rectangle){x, y, w, h}, "DATABASE");
    y += h + GAP;

    static size_t lastSessionIdCheck = 0;
    if (state->saving.autoSaveSessionId != lastSessionIdCheck) {
        lastSessionIdCheck = state->saving.autoSaveSessionId;
        state->session.sessionCount = 0;
    }

    if (state->session.sessionCount == 0) {
        state->session.sessionCount = StorageSessionsFetch(state->session.loadedSessions, MAX_CACHED_SESSIONS);
    }

    for (size_t i = 0; i < state->session.sessionCount; i++) {
        Rectangle rowRec = { x, y, w, h };
        if (!SessionItemDraw(state, rowRec, &state->session.loadedSessions[i], i)) break;
        y += h + GAP;

        if (state->session.expandedSessionId == state->session.loadedSessions[i].id) {
            y += ((float)state->session.expandedTableCount * (h + GAP)) + GAP;
        }
    }
}

static inline void DataTratementContentDraw(WorkspaceState *state, Rectangle layout) {
    if (!state) return;

    float h = BTN_HEIGHT, x = layout.x, y = layout.y, w = layout.width;
    LabelCentered((Rectangle){x, y, w, h}, "DATA TRATAMENT");
    y += h + GAP * 2;

    // Firing Period: Focado na análise do período médio de disparo do Receiver normalizado (T_R/T_0) para avaliar o quão mais rápido ele se torna.
    bool isFiringPeriod = state->ui.workFocus.dataFocus == FOCUS_DATA_FIRING_PERIOD;
    if (Button((Rectangle){x, y, w, h}, ICON_FX, "Firing Period", isFiringPeriod)) {
        //FiringPeriodCalculate(state, 100);
    } y += h + GAP;

    bool isRaster = state->ui.workFocus.dataFocus == FOCUS_DATA_RASTER;
    if (Button((Rectangle){x, y, w, h}, ICON_FX, "Raster Plot", isRaster)) {
        RasterPlotDraw(state, 100);
    } y += h + GAP;

    // Return Map: Mapa de retorno (T^R_{i-1} vs T^R_i) crucial para caracterizar o regime quase-periódico durante o Phase-Drift.
    bool isReturnMap = state->ui.workFocus.dataFocus == FOCUS_DATA_RETURN_MAP;
    if (Button((Rectangle){x, y, w, h}, ICON_FX, "Return Map", isReturnMap)) {
        //ReturnMapCalculate(state, 100);
    } y += h + GAP;

    bool isSpikeTiming = state->ui.workFocus.dataFocus == FOCUS_DATA_SPIKE_TIMING;
    if (Button((Rectangle){x, y, w, h}, ICON_FX, "Spike-Timing Diff", isSpikeTiming)) {
        SpikeTimingCalculate(state, 100);
    } y += h + GAP;

    // Synchronization Regimes: Para mapear ou identificar as áreas de transição entre Delayed Synchronization (DS), Anticipated Synchronization (AS) e Phase-Drift (PD).
    bool isSyncRegime = state->ui.workFocus.dataFocus == FOCUS_DATA_SYNC_REGIME;
    if (Button((Rectangle){x, y, w, h}, ICON_FX, "Sync Regimes", isSyncRegime)) {
        //SyncRegimesCalculate(state, 100);
    } y += h + GAP;
}

static inline void EditContentDraw(WorkspaceState *state, Rectangle layout) {
    if (!state) return;

    char txt[64];
    float h = BTN_HEIGHT, lh = LBL_HEIGHT;
    float x = layout.x, y = layout.y, w = layout.width;

    LabelCentered((Rectangle){x, y, w, h}, WS_STR_MENU_TITLE_EDIT);
    y += h + GAP * 2;

    bool isAddNrn = state->ui.workFocus.editFocus == FOCUS_EDIT_ADD_NEURON;
    if (Button((Rectangle){x, y, w, h}, ICON_NONE, WS_STR_BTN_ADD_NEURON, isAddNrn)) {
        NeuronAdd(state, state->editor.activeNeuronModel, state->editor.activeIzhikevichModel);
    }
    y += h + GAP;

    bool isAddSyn = state->ui.workFocus.editFocus == FOCUS_EDIT_ADD_SYNAPSE;
    if (Button((Rectangle){x, y, w, h}, ICON_LINK, WS_STR_BTN_ADD_SYNAPSE, isAddSyn)) SynapseAdd(state);
    y += h + GAP;

    if (state->network ||(state->session.activeTableId != TABLE_ID_UNKNOWN)) {
        int nrnCount = state->network ? (int)state->network->neuronCount : 0;

        bool isSrc = state->ui.workFocus.editFocus == FOCUS_EDIT_SOURCE;
        Selector((Rectangle){x, y, w, h}, "Source", (int*)&state->editor.sourceNeuronId, 0, nrnCount - 1, isSrc);
        y += h + GAP;

        bool isTgt = state->ui.workFocus.editFocus == FOCUS_EDIT_TARGET;
        Selector((Rectangle){x, y, w, h}, "Target", (int*)&state->editor.targetNeuronId, 0, nrnCount - 1, isTgt);
        y += h + GAP;
    }

    bool isDelete = state->ui.workFocus.editFocus == FOCUS_EDIT_DELETE;
    if (Button((Rectangle){x, y, w, h}, ICON_FILE_DELETE, WS_STR_BTN_CLEAR_NET, isDelete)) NetworkDelete(state);
    y += h + GAP;

    sprintf(txt, WS_STR_LBL_AUTAPSE_DEL, state->params.autapseDelay);
    GuiLabel((Rectangle){x, y, w, lh}, txt);
    bool isAutapseDelay = state->ui.workFocus.editFocus == FOCUS_EDIT_AUTAPSE_DELAY;
    SliderBar((Rectangle){x, y += lh, w, 20}, &state->params.autapseDelay, 0, MAX_DELAY, isAutapseDelay);
    y += 20 + GAP;

    sprintf(txt, WS_STR_LBL_CONDUCTANCE, state->params.synapseConductancy);
    GuiLabel((Rectangle){ x, y, w, lh} , txt);
    bool isCond = state->ui.workFocus.editFocus == FOCUS_EDIT_CONDUCTANCY;
    SliderBar((Rectangle){x, y += lh, w, 20}, &state->params.synapseConductancy, 0, MAX_CONDUCTANCE, isCond);
    y += 20 + GAP;

    sprintf(txt, WS_STR_LBL_EXT_CURRENT, state->params.externalCurrent);
    GuiLabel((Rectangle){x, y, w, lh}, txt);
    bool isIExt = state->ui.workFocus.editFocus == FOCUS_EDIT_EXT_CURRENT;
    SliderBar((Rectangle){x, y += lh, w, 20}, &state->params.externalCurrent, 0, MAX_CURRENT, isIExt);
    y += 20 + GAP;

    GuiLabel((Rectangle){x, y, w, lh}, WS_STR_LBL_NRN_MODEL);
    bool isNrnModel = state->ui.workFocus.editFocus == FOCUS_EDIT_NEURON_MODEL;
    ComboBox((Rectangle){x, y += lh, w, h}, WS_STR_OPT_NRN_MODELS, (int*)&state->editor.activeNeuronModel, isNrnModel);
    y += h + GAP;

    if (state->editor.activeNeuronModel == IZHIKEVICH_MODEL) {
        GuiLabel((Rectangle){x, y, w, lh}, WS_STR_LBL_IZ_TYPE);
        bool isIzNrn = state->ui.workFocus.editFocus == FOCUS_EDIT_IZ_NEURON;
        ComboBox((Rectangle){x, y += lh, w, h}, WS_STR_OPT_IZ_MODELS, (int*)&state->editor.activeIzhikevichModel, isIzNrn);
        y += h + GAP;
    }

    GuiLabel((Rectangle){ x, y, w, lh }, WS_STR_LBL_SYN_MODEL);
    bool isSynModel = state->ui.workFocus.editFocus == FOCUS_EDIT_SYN_MODEL;
    ComboBox((Rectangle){x, y += lh, w, h}, WS_STR_OPT_SYN_MODELS, (int*)&state->editor.activeSynapseModel, isSynModel);
    y += h + GAP;
}


static inline TableId WorkspaceStringToTableId(const char *tableName) {
    if (strcmp(tableName, TBL_IZ_RECOVERY) == 0) return TABLE_ID_IZ_RECOVERY;

    if (strcmp(tableName, TBL_HH_IK)     == 0)   return TABLE_ID_HH_IK;
    if (strcmp(tableName, TBL_HH_INA)    == 0)   return TABLE_ID_HH_INA;
    if (strcmp(tableName, TBL_HH_ILEAK)  == 0)   return TABLE_ID_HH_ILEAK;
    if (strcmp(tableName, TBL_HH_GATE_M) == 0)   return TABLE_ID_HH_GATE_M;
    if (strcmp(tableName, TBL_HH_GATE_H) == 0)   return TABLE_ID_HH_GATE_H;
    if (strcmp(tableName, TBL_HH_GATE_N) == 0)   return TABLE_ID_HH_GATE_N;

    if (strcmp(tableName, TBL_NET_NRN)     == 0) return TABLE_ID_NET_NRN;
    if (strcmp(tableName, TBL_NET_SYN)     == 0) return TABLE_ID_NET_SYN;
    if (strcmp(tableName, TBL_NEUROTRANS)  == 0) return TABLE_ID_NEUROTRANS;
    if (strcmp(tableName, TBL_PHASE_SPACE) == 0) return TABLE_ID_PHASE_SPACE;
    if (strcmp(tableName, TBL_SYNAPTIC)    == 0) return TABLE_ID_SYNAPTIC;
    if (strcmp(tableName, TBL_TAU)         == 0) return TABLE_ID_TAU;
    if (strcmp(tableName, TBL_VOLTAGE)     == 0) return TABLE_ID_VOLTAGE;

    return TABLE_ID_UNKNOWN; // Tabela não reconhecida (fallback de segurança)
}
