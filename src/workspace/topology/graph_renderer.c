#include <stddef.h>
#include <stdio.h>

#include "model/neural/izhikevich/izhikevich_config.h"
#include "model/neural/izhikevich/izhikevich_struct.h"
#include "model/neural/hodgkin-huxley/hodgkin_huxley_config.h"
#include "model/neural/neuron_models.h"

#include "network/network_manager.h"
#include "network/network_struct.h"

#include "raylib.h"
#include "workspace/config/workspace_config.h"
#include "workspace/config/workspace_ui_config.h"
#include "workspace/strings/workspace_strings.h"
#include "workspace/topology/graph_renderer.h"
#include "workspace/themes/worksapce_colors.h"
#include "workspace/utils/topology_utils.h"
#include "workspace/utils/workspace_components.h"

// A biblioteca abaixo deve ficar abaixo de "graph_renderer.h"
// Ocorre erro de redefinição do "vector2"
#include "raymath.h"

// ============================================================================
// DECLARAÇÃO DE FUNÇÕES INTERNAS
// ============================================================================

static inline int TopologyInteractionUpdate(
    WorkspaceState *state,
    Rectangle layout,
    float nrnRadius,
    size_t nrnCount,
    Vector2 mousePos
);

static inline Texture2D NeuronTextureGet(WorkspaceState *state, NeuronNode *node);

static inline void NeuronPotentialsGet(
    const NeuronNode *node,
    float *currentVoltage,
    float *restingPotential
);

static inline bool AutapseLineDraw(
    Color color,
    float nrnRadius,
    int connIdx,
    Vector2 pos,
    Vector2 mousePos
);

static inline bool SynapseLineDraw(
    bool synType,
    Color color,
    float nrnRadius,
    int connIdx,
    Vector2 src,
    Vector2 tgt,
    Vector2 mousePos
);

static inline int TopologyConnectionsDraw(
    WorkspaceState *state,
    float nrnRadius,
    size_t nrnCount,
    Vector2 mousePos
);

static inline void TopologyNeuronsDraw(
    WorkspaceState *state,
    float targetSize,
    float nrnRadius,
    size_t nrnCount
);

static inline void TopologyNeuronTooltipDraw(
    WorkspaceState *state,
    Rectangle layout,
    Vector2 mousePos,
    int hoveredIdx
);

static inline void TopologySynapseTooltipDraw(
    WorkspaceState *state,
    Rectangle layout,
    Vector2 mousePos,
    int hoveredIdx
);

// ============================================================================
// FUNÇÃO PRINCIPAL DE RENDERIZAÇÃO DA TOPOLOGIA
// ============================================================================

void NetTopologyDraw(WorkspaceState *state, Rectangle layout) {
    if (!state->network || !state->topology.nrnLayouts) {
        StatusMsg(layout, WS_STR_TXT_NO_NET_CREATED);
        return;
    }

    const float targetSize = TOPOLOGY_TARGET_NEURON_SIZE;
    const float nrnRadius  = targetSize / 2.0f;

    size_t nrnCount   = state->network->neuronCount;
    Vector2 mousePos = GetMousePosition();

    int hoveredNrnIdx = TopologyInteractionUpdate(state, layout, nrnRadius, nrnCount, mousePos);
    int hoveredSynIdx = TopologyConnectionsDraw(state, nrnRadius, nrnCount, mousePos);

    TopologyNeuronsDraw(state, targetSize, nrnRadius, nrnCount);

    if (hoveredNrnIdx != -1) {
        TopologyNeuronTooltipDraw(state, layout, mousePos, hoveredNrnIdx);
    } else if (hoveredSynIdx != -1) {
        TopologySynapseTooltipDraw(state, layout, mousePos, hoveredSynIdx);
    }
}

// ============================================================================
// INTERAÇÃO E RENDERIZAÇÃO DE COMPONENTES
// ============================================================================

static inline int TopologyInteractionUpdate(WorkspaceState *state, Rectangle layout, float nrnRadius, size_t nrnCount, Vector2 mousePos) {
    int hoveredIdx = -1;

    for (size_t i = 0; i < nrnCount; i++) {
        NeuronLayout *posInfo = &state->topology.nrnLayouts[i];

        if (CheckCollisionPointCircle(mousePos, posInfo->position, nrnRadius)) {
            hoveredIdx = (int)i;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) posInfo->isDragging = true;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) posInfo->isDragging = false;

        if (posInfo->isDragging) {
            posInfo->position.x = Clamp(mousePos.x, layout.x + nrnRadius, layout.x + layout.width - nrnRadius);
            posInfo->position.y = Clamp(mousePos.y, layout.y + nrnRadius, layout.y + layout.height - nrnRadius);
        }
    }
    return hoveredIdx;
}

static inline Texture2D NeuronTextureGet(WorkspaceState *state, NeuronNode *node) {
    if (node->type == HODGKIN_HUXLEY_MODEL) return state->topology.neuronTextures[TEX_HH];

    if (node->type == IZHIKEVICH_MODEL && node->interface.modelData != NULL) {
        IzhikevichModel *model = (IzhikevichModel*)node->interface.modelData;
        switch (model->neuron.config.type) {
            case CHATTERING:             return state->topology.neuronTextures[TEX_IZ_CH];
            case FAST_SPIKING:           return state->topology.neuronTextures[TEX_IZ_FS];
            case INTRINSICALLY_BURSTING: return state->topology.neuronTextures[TEX_IZ_IB];
            case LOW_THRESHOLD_SPIKING:  return state->topology.neuronTextures[TEX_IZ_LTS];
            case REGULAR_SPIKING:        return state->topology.neuronTextures[TEX_IZ_RS];
            case RESONATOR:              return state->topology.neuronTextures[TEX_IZ_RE];
            case THALAMO_CORTICAL:       return state->topology.neuronTextures[TEX_IZ_TC];
            default: break;
        }
    }
    return state->topology.neuronTextures[TEX_IZ_RS]; // Fallback
}

static inline int TopologyConnectionsDraw(WorkspaceState *state, float nrnRadius, size_t nrnCount, Vector2 mousePos) {
    if (state->topology.incomingCounters) {
        for (size_t k = 0; k < nrnCount; k++) {
            state->topology.incomingCounters[k] = 0;
        }
    }

    int hoveredSynIdx = -1;
    size_t synIdx = 0;

    for (size_t i = 0; i < nrnCount; i++) {
        NeuronNode *srcNode     = &state->network->neurons[i];
        NeuronLayout *srcLayout = &state->topology.nrnLayouts[i];
        SynapseEdge *edge        = srcNode->outboundSynapses;

        while (edge != NULL) {
            size_t tgtIdx = edge->targetNeuron;

            if (tgtIdx < nrnCount) {
                NeuronLayout *tgtLayout = &state->topology.nrnLayouts[tgtIdx];
                bool isAmpa = (edge->edgeType == AMPA_MODEL);
                Color synColor = isAmpa ? COLOR_PRIMARY : COLOR_SECONDARY;

                int connIdx = 0;
                if (state->topology.incomingCounters) {
                    connIdx = state->topology.incomingCounters[tgtIdx];
                    state->topology.incomingCounters[tgtIdx]++;
                }

                if (state->topology.synLayouts != NULL) {
                    SynapseLayout *syn = &state->topology.synLayouts[synIdx];
                    syn->srcIdx    = i;
                    syn->tgtIdx    = tgtIdx;
                    syn->isAmpa    = isAmpa;
                    syn->gMax      = (edge->wrapper && edge->wrapper->physics.gMax) ? *(edge->wrapper->physics.gMax) : 0.0;
                    syn->isHovered = false;
                }

                bool isHovered = SynapseLineDraw(
                    isAmpa, synColor, nrnRadius, connIdx,
                    srcLayout->position, tgtLayout->position, mousePos
                );

                if (isHovered) {
                    if (state->topology.synLayouts) {
                        state->topology.synLayouts[synIdx].isHovered = true;
                    }

                    if (hoveredSynIdx == -1) hoveredSynIdx = (int)synIdx;
                }

                synIdx++;
            }
            edge = edge->nextSynapse;
        }
    }

    state->topology.synapseCount = synIdx;
    return hoveredSynIdx;
}

static inline void NeuronPotentialsGet(const NeuronNode *node, float *currentVoltage, float *restingPotential) {
    *currentVoltage = 0.0f;
    *restingPotential = 0.0f;

    if (node->interface.membranePotential != NULL) {
        *currentVoltage = (float)(*(node->interface.membranePotential));
    }

    switch (node->type) {
        case IZHIKEVICH_MODEL: {
            if (node->interface.modelData != NULL) {
                const IzhikevichModel *iz = (const IzhikevichModel*)node->interface.modelData;
                *restingPotential = (float)iz->neuron.config.c;
            }
        } break;

        case HODGKIN_HUXLEY_MODEL: {
            *restingPotential = (float)HH_SQUID_AXON.restingPotential;
        } break;

        default: break;
    }
}

// Renderização dedicada para Autapses (Auto-conexões)
static inline bool AutapseLineDraw(Color color, float nrnRadius, int connIdx, Vector2 pos, Vector2 mousePos) {
    bool hovered = false;
    float angle = TOPOLOGY_AUTAPSE_BASE_ANGLE + ((float)connIdx * TOPOLOGY_AUTAPSE_ANGLE_STEP);
    Vector2 loopDir  = { cosf(angle), sinf(angle) };
    Vector2 loopPerp = { -loopDir.y, loopDir.x };

    float offset = TOPOLOGY_AUTAPSE_OFFSET;
    Vector2 startPt    = Vector2Add(pos, Vector2Add(Vector2Scale(loopDir, nrnRadius), Vector2Scale(loopPerp, -offset)));
    Vector2 baseCenter = Vector2Add(pos, Vector2Add(Vector2Scale(loopDir, nrnRadius + 6.0f), Vector2Scale(loopPerp, offset)));
    Vector2 ballCenter = Vector2Add(pos, Vector2Add(Vector2Scale(loopDir, nrnRadius + TOPOLOGY_SYNAPSE_BALL_RADIUS), Vector2Scale(loopPerp, offset)));

    float loopHeight = TOPOLOGY_AUTAPSE_LOOP_BASE_HEIGHT + (float)connIdx * TOPOLOGY_AUTAPSE_LOOP_STEP_HEIGHT;
    Vector2 ctrl = Vector2Add(pos, Vector2Scale(loopDir, nrnRadius + loopHeight));

    const int segments = TOPOLOGY_SYNAPSE_SEGMENTS;
    Vector2 prevPoint = startPt;

    for (int i = 1; i <= segments; i++) {
        float t = (float)i / (float)segments;
        float u = 1.0f - t;

        Vector2 currentPoint = {
            u * u * startPt.x + 2.0f * u * t * ctrl.x + t * t * baseCenter.x,
            u * u * startPt.y + 2.0f * u * t * ctrl.y + t * t * baseCenter.y
        };

        DrawLineEx(prevPoint, currentPoint, TOPOLOGY_SYNAPSE_LINE_THICKNESS, Fade(color, TOPOLOGY_SYNAPSE_ALPHA));

        if (CheckCollisionPointLine(mousePos, prevPoint, currentPoint, TOPOLOGY_SYNAPSE_HITBOX_MARGIN)) {
            hovered = true;
        }

        prevPoint = currentPoint;
    }

    DrawCircleV(ballCenter, TOPOLOGY_SYNAPSE_BALL_RADIUS, color);
    return hovered;
}

static inline bool SynapseLineDraw(bool synType, Color color, float nrnRadius, int connIdx, Vector2 src, Vector2 tgt, Vector2 mousePos) {
    Vector2 dir = Vector2Subtract(tgt, src);
    float distance = Vector2Length(dir);

    if (distance <= 0.0f) {
        return AutapseLineDraw(RED, nrnRadius, connIdx, src, mousePos);
    }

    bool hovered = false;
    dir = Vector2Normalize(dir);

    float baseAngleOffset = TOPOLOGY_SYNAPSE_BASE_ANGLE_OFFSET;
    float angleOffset = baseAngleOffset + ((float)((connIdx + 1.0) / 2) * TOPOLOGY_SYNAPSE_ANGLE_STEP * ((connIdx % 2 == 0) ? 1.0f : -1.0f));

    Vector2 rotatedDir = {
        dir.x * cosf(angleOffset) - dir.y * sinf(angleOffset),
        dir.x * sinf(angleOffset) + dir.y * cosf(angleOffset)
    };

    Vector2 tip = Vector2Subtract(tgt, Vector2Scale(rotatedDir, nrnRadius));
    Vector2 baseCenter = Vector2Subtract(tgt, Vector2Scale(rotatedDir, nrnRadius + TOPOLOGY_SYNAPSE_ARROW_WIDTH));

    Vector2 midPoint = Vector2Scale(Vector2Add(src, baseCenter), 0.5f);
    Vector2 perp = { -dir.y, dir.x };

    float baseCurvature = TOPOLOGY_SYNAPSE_BASE_CURVATURE;
    float curveIntensity = baseCurvature + ((float)connIdx * TOPOLOGY_SYNAPSE_CURVE_INTENSITY * ((connIdx % 2 == 0) ? 1.0f : -1.0f));

    Vector2 ctrl = Vector2Add(midPoint, Vector2Scale(perp, curveIntensity));

    const int segments = TOPOLOGY_SYNAPSE_SEGMENTS;
    Vector2 prevPoint = src;

    for (int i = 1; i <= segments; i++) {
        float t = (float)i / (float)segments;
        float u = 1.0f - t;

        Vector2 currentPoint = {
            u * u * src.x + 2.0f * u * t * ctrl.x + t * t * baseCenter.x,
            u * u * src.y + 2.0f * u * t * ctrl.y + t * t * baseCenter.y
        };

        DrawLineEx(prevPoint, currentPoint, TOPOLOGY_SYNAPSE_LINE_THICKNESS, Fade(color, TOPOLOGY_SYNAPSE_ALPHA));

        if (CheckCollisionPointLine(mousePos, prevPoint, currentPoint, TOPOLOGY_SYNAPSE_HITBOX_MARGIN)) {
            hovered = true;
        }

        prevPoint = currentPoint;
    }

    if (synType) {
        Vector2 perpIndicator = { -rotatedDir.y, rotatedDir.x };
        float arrowWidth = TOPOLOGY_SYNAPSE_ARROW_WIDTH;

        Vector2 leftBase  = Vector2Subtract(baseCenter, Vector2Scale(perpIndicator, arrowWidth / 2.0f));
        Vector2 rightBase = Vector2Add(baseCenter, Vector2Scale(perpIndicator, arrowWidth / 2.0f));

        DrawTriangle(tip, leftBase, rightBase, color);
    } else {
        Vector2 ballCenter = Vector2Subtract(tgt, Vector2Scale(rotatedDir, nrnRadius + TOPOLOGY_SYNAPSE_BALL_RADIUS));
        DrawCircleV(ballCenter, TOPOLOGY_SYNAPSE_BALL_RADIUS, color);
    }

    return hovered;
}

static inline void TopologyNeuronsDraw(WorkspaceState *state, float targetSize, float nrnRadius, size_t nrnCount) {
    for (size_t i = 0; i < nrnCount; i++) {
        NeuronNode   *node    = &state->network->neurons[i];
        NeuronLayout *posInfo = &state->topology.nrnLayouts[i];

        Texture2D tex = NeuronTextureGet(state, node);
        Rectangle sourceRec = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
        Rectangle destRec   = { posInfo->position.x, posInfo->position.y, targetSize, targetSize };
        Vector2 origin      = { nrnRadius, nrnRadius };
        Color tint          = posInfo->isDragging ? LIGHTGRAY : WHITE;

        DrawTexturePro(tex, sourceRec, destRec, origin, 0.0f, tint);

        float currentVoltage, restingPotential;
        NeuronPotentialsGet(node, &currentVoltage, &restingPotential);

        float traceOffset = (currentVoltage + restingPotential) * TOPOLOGY_VOLTAGE_TRACE_SCALE;

        if (traceOffset < 0.0f)                              traceOffset = 0.0f;
        if (traceOffset > TOPOLOGY_VOLTAGE_TRACE_MAX_OFFSET) traceOffset = TOPOLOGY_VOLTAGE_TRACE_MAX_OFFSET;

        // ID do Neurônio
        char idStr[TOPOLOGY_ID_BUFFER_SIZE];
        snprintf(idStr, sizeof(idStr), WS_STR_FMT_NEURON_ID, i);
        int textWidth = MeasureText(idStr, TOPOLOGY_NEURON_LABEL_FONT_SIZE);
        int idX = (int)posInfo->position.x - (textWidth / 2);
        int idY = (int)posInfo->position.y - (int)nrnRadius - TOPOLOGY_NEURON_LABEL_OFFSET_Y;
        DrawText(idStr, idX, idY, TOPOLOGY_NEURON_LABEL_FONT_SIZE, RAYWHITE);

        // Exibe Isyn logo abaixo do nó do neurônio
        char isynStr[32];
        double isyn = NetworkNeuronSynapticCurrentGet(state->network, i);
        snprintf(isynStr, sizeof(isynStr), "Isyn: %.2f uA", isyn);
        int isynWidth = MeasureText(isynStr, 10);
        DrawText(isynStr, (int)posInfo->position.x - (isynWidth / 2), (int)posInfo->position.y + (int)nrnRadius + 4, 10, YELLOW);
    }
}

// ============================================================================
// TOOLTIPS
// ============================================================================

static inline void TopologyNeuronTooltipDraw(WorkspaceState *state, Rectangle layout, Vector2 mousePos, int hoveredIdx) {
    if (hoveredIdx == -1 || state->topology.nrnLayouts[hoveredIdx].isDragging) return;

    NeuronNode *hoveredNode = &state->network->neurons[hoveredIdx];

    char typeStr[ADJ_LABEL_BUFFER_SIZE];
    NeuronTypeFormat(typeStr, sizeof(typeStr), hoveredNode);

    double iext = NetworkNeuronExternalCurrentGet(state->network, (size_t)hoveredIdx);
    double isyn = NetworkNeuronSynapticCurrentGet(state->network, (size_t)hoveredIdx);
    double volt = NetworkNeuronVoltageGet(state->network, (size_t)hoveredIdx);

    char line1[TOPOLOGY_TOOLTIP_LINE_BUFFER_SIZE];
    char line2[TOPOLOGY_ID_BUFFER_SIZE];
    char line3[TOPOLOGY_ID_BUFFER_SIZE];
    char line4[TOPOLOGY_ID_BUFFER_SIZE];

    snprintf(line1, sizeof(line1), WS_STR_FMT_TOOLTIP_NRN_INFO, hoveredIdx, typeStr);
    snprintf(line2, sizeof(line2), WS_STR_FMT_TOOLTIP_I_EXT, iext);
    snprintf(line3, sizeof(line3), "I_syn: %.2f uA", isyn);
    snprintf(line4, sizeof(line3), WS_STR_FMT_TOOLTIP_VOLT, volt);

    int fontSize = TOPOLOGY_TOOLTIP_FONT_SIZE;
    int lineSpacing = TOPOLOGY_TOOLTIP_LINE_SPACING;

    int w1 = MeasureText(line1, fontSize);
    int w2 = MeasureText(line2, fontSize);
    int w3 = MeasureText(line3, fontSize);
    int w4 = MeasureText(line3, fontSize);

    int maxTextWidth = w1;
    if (w2 > maxTextWidth) maxTextWidth = w2;
    if (w3 > maxTextWidth) maxTextWidth = w3;
    if (w4 > maxTextWidth) maxTextWidth = w4;

    int paddingX = TOPOLOGY_TOOLTIP_PADDING_X;
    int paddingY = TOPOLOGY_TOOLTIP_PADDING_Y;

    int boxWidth = maxTextWidth + (paddingX * 2);
    int boxHeight = (4 * fontSize) + (3 * lineSpacing) + (paddingY * 2);

    int boxX = (int)mousePos.x + TOPOLOGY_TOOLTIP_MOUSE_OFFSET;
    int boxY = (int)mousePos.y + TOPOLOGY_TOOLTIP_MOUSE_OFFSET;

    if ((float)(boxX + boxWidth)  > layout.x + layout.width)  boxX = (int)mousePos.x - boxWidth - TOPOLOGY_TOOLTIP_MOUSE_OFFSET;
    if ((float)(boxY + boxHeight) > layout.y + layout.height) boxY = (int)mousePos.y - boxHeight - TOPOLOGY_TOOLTIP_MOUSE_OFFSET;

    if (boxX < (int)layout.x) boxX = (int)layout.x + TOPOLOGY_TOOLTIP_BOUNDS_MARGIN;
    if (boxY < (int)layout.y) boxY = (int)layout.y + TOPOLOGY_TOOLTIP_BOUNDS_MARGIN;

    DrawRectangle(boxX, boxY, boxWidth, boxHeight, Fade(BLACK, TOPOLOGY_TOOLTIP_ALPHA));
    DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, GRAY);

    int textY = boxY + paddingY;
    int step = fontSize + lineSpacing;

    DrawText(line1, boxX + paddingX, textY, fontSize, RAYWHITE);
    DrawText(line2, boxX + paddingX, textY + step, fontSize, RAYWHITE);
    DrawText(line3, boxX + paddingX, textY + (step * 2), fontSize, RAYWHITE);
    DrawText(line4, boxX + paddingX, textY + (step * 3), fontSize, RAYWHITE);
}

static inline void TopologySynapseTooltipDraw(WorkspaceState *state, Rectangle layout, Vector2 mousePos, int hoveredIdx) {
    if (hoveredIdx < 0 || !state->topology.synLayouts) return;

    const SynapseLayout *syn = &state->topology.synLayouts[hoveredIdx];
    const char *typeStr = syn->isAmpa ? WS_STR_OPT_AMPA : WS_STR_OPT_GABA;

    char line1[TOPOLOGY_TOOLTIP_LINE_BUFFER_SIZE];
    char line2[TOPOLOGY_ID_BUFFER_SIZE];
    char line3[TOPOLOGY_ID_BUFFER_SIZE];

    snprintf(line1, sizeof(line1), WS_STR_FMT_TOOLTIP_SYN_NODES, syn->srcIdx, syn->tgtIdx);
    snprintf(line2, sizeof(line2), WS_STR_FMT_TOOLTIP_SYN_TYPE, typeStr);
    snprintf(line3, sizeof(line3), WS_STR_FMT_TOOLTIP_GMAX, syn->gMax);

    int fontSize = TOPOLOGY_TOOLTIP_FONT_SIZE;
    int lineSpacing = TOPOLOGY_TOOLTIP_LINE_SPACING;

    int w1 = MeasureText(line1, fontSize);
    int w2 = MeasureText(line2, fontSize);
    int w3 = MeasureText(line3, fontSize);

    int maxTextWidth = w1;
    if (w2 > maxTextWidth) maxTextWidth = w2;
    if (w3 > maxTextWidth) maxTextWidth = w3;

    int paddingX = TOPOLOGY_TOOLTIP_PADDING_X;
    int paddingY = TOPOLOGY_TOOLTIP_PADDING_Y;

    int boxWidth = maxTextWidth + (paddingX * 2);
    int boxHeight = (3 * fontSize) + (2 * lineSpacing) + (paddingY * 2);

    int boxX = (int)mousePos.x + TOPOLOGY_TOOLTIP_MOUSE_OFFSET;
    int boxY = (int)mousePos.y + TOPOLOGY_TOOLTIP_MOUSE_OFFSET;

    if ((float)(boxX + boxWidth)  > layout.x + layout.width)  boxX = (int)mousePos.x - boxWidth - TOPOLOGY_TOOLTIP_MOUSE_OFFSET;
    if ((float)(boxY + boxHeight) > layout.y + layout.height) boxY = (int)mousePos.y - boxHeight - TOPOLOGY_TOOLTIP_MOUSE_OFFSET;

    if (boxX < (int)layout.x) boxX = (int)layout.x + TOPOLOGY_TOOLTIP_BOUNDS_MARGIN;
    if (boxY < (int)layout.y) boxY = (int)layout.y + TOPOLOGY_TOOLTIP_BOUNDS_MARGIN;

    DrawRectangle(boxX, boxY, boxWidth, boxHeight, Fade(BLACK, TOPOLOGY_TOOLTIP_ALPHA));
    DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, GRAY);

    Color typeColor = syn->isAmpa ? COLOR_PRIMARY : COLOR_SECONDARY;

    DrawText(line1, boxX + paddingX, boxY + paddingY, fontSize, RAYWHITE);
    DrawText(line2, boxX + paddingX, boxY + paddingY + fontSize + lineSpacing, fontSize, typeColor);
    DrawText(line3, boxX + paddingX, boxY + paddingY + (fontSize + lineSpacing) * 2, fontSize, RAYWHITE);
}
