#include <stdio.h>

#include "model/neural/hodgkin-huxley/hodgkin_huxley_config.h"
#include "model/synaptic/ampa-gaba-a/ampa_gaba_a_config.h"

#include "workspace/config/workspace_ui_config.h"
#include "workspace/strings/workspace_strings.h"
#include "workspace/ui/workspace_net_config.h"

static inline void LineTextDraw(
    float x,
    float *y,
    float spacing,
    Color color,
    int fontSize,
    const char *fmt,
    ...
);

static inline void SectionHeaderDraw(
    const char *title,
    float x,
    float *y,
    float spacing
);

void NetConfigDraw(WorkspaceState *state, Rectangle layout) {
    if (!state) return;

    float availableWidth = layout.width - (PADDING * 2.0f) - GAP;
    float contentHeight  = layout.height - (PADDING * 2.0f);
    float halfWidth      = availableWidth * 0.5f;

    Rectangle leftPanel  = { layout.x + PADDING, layout.y + PADDING, halfWidth, contentHeight };
    Rectangle rightPanel = { leftPanel.x + halfWidth + GAP, layout.y + PADDING, halfWidth, contentHeight };

    int separatorX = (int)(layout.x + (layout.width / 2.0f));
    DrawLine(separatorX, (int)(layout.y + PADDING), separatorX, (int)(layout.y + layout.height - PADDING), Fade(GRAY, 0.5f));

    static const char *IZ_TYPE_NAMES[IZ_NEURON_COUNT] = {
        [CHATTERING]             = "Chattering",
        [FAST_SPIKING]           = "Fast Spiking",
        [INTRINSICALLY_BURSTING] = "Intrinsically Bursting",
        [LOW_THRESHOLD_SPIKING]  = "Low Threshold Spiking",
        [REGULAR_SPIKING]        = "Regular Spiking",
        [RESONATOR]              = "Resonator",
        [THALAMO_CORTICAL]       = "Thalamo-Cortical"
    };

    float offsetY = leftPanel.y;

    int leftTxtWidth = MeasureText(WS_STR_TITLE_NEURON_PARAMS, TITLE_FONTSIZE);
    DrawText(WS_STR_TITLE_NEURON_PARAMS, (int)(leftPanel.x + (leftPanel.width - (float)leftTxtWidth) / 2.0f), (int)offsetY, TITLE_FONTSIZE, RAYWHITE);
    offsetY += GROUP_SPACING;

    SectionHeaderDraw(WS_STR_HDR_IZ_MODELS, leftPanel.x, &offsetY, NET_CFG_LINE_SPACING);

    for (int i = 0; i < IZ_NEURON_COUNT; i++) {
        const IzhikevichConfig *cfg = &IZHIKEVICH_PARAMETERS[i];
        LineTextDraw(
            leftPanel.x, &offsetY, NET_CFG_LINE_SPACING, LIGHTGRAY, LBL_FONTSIZE,
            WS_STR_FMT_IZ_PARAMS, IZ_TYPE_NAMES[i], cfg->a, cfg->b, cfg->c, cfg->d
        );
    }
    offsetY += NET_CFG_SECTION_GAP;

    SectionHeaderDraw(WS_STR_HDR_HH_MODELS, leftPanel.x, &offsetY, NET_CFG_LINE_SPACING);
    LineTextDraw(leftPanel.x, &offsetY, NET_CFG_LINE_SPACING, LIGHTGRAY, LBL_FONTSIZE, WS_STR_FMT_HH_MEMBRANE,  HH_SQUID_AXON.membraneCapacitancy,  HH_SQUID_AXON.restingPotential);
    LineTextDraw(leftPanel.x, &offsetY, NET_CFG_LINE_SPACING, LIGHTGRAY, LBL_FONTSIZE, WS_STR_FMT_HH_SODIUM,    HH_SQUID_AXON.sodiumConductance,    HH_SQUID_AXON.sodiumReversal);
    LineTextDraw(leftPanel.x, &offsetY, NET_CFG_LINE_SPACING, LIGHTGRAY, LBL_FONTSIZE, WS_STR_FMT_HH_POTASSIUM, HH_SQUID_AXON.potassiumConductance, HH_SQUID_AXON.potassiumReversal);
    LineTextDraw(leftPanel.x, &offsetY, NET_CFG_LINE_SPACING, LIGHTGRAY, LBL_FONTSIZE, WS_STR_FMT_HH_LEAK,      HH_SQUID_AXON.leakConductance,      HH_SQUID_AXON.leakReversal);

    offsetY = rightPanel.y;

    int rightTxtWidth = MeasureText(WS_STR_TITLE_SYNAPSE_PARAMS, TITLE_FONTSIZE);
    DrawText(WS_STR_TITLE_SYNAPSE_PARAMS, (int)(rightPanel.x + (rightPanel.width - (float)rightTxtWidth) / 2.0f), (int)offsetY, TITLE_FONTSIZE, RAYWHITE);
    offsetY += GROUP_SPACING;

    SectionHeaderDraw(WS_STR_HDR_IZ_SYNAPSE, rightPanel.x, &offsetY, NET_CFG_LINE_SPACING);
    LineTextDraw(rightPanel.x, &offsetY, NET_CFG_LINE_SPACING, LIGHTGRAY, LBL_FONTSIZE, WS_STR_FMT_SYN_SIGMOID, IZ_SYN_CFG.KP, IZ_SYN_CFG.VP);
    LineTextDraw(rightPanel.x, &offsetY, NET_CFG_LINE_SPACING, LIGHTGRAY, LBL_FONTSIZE, WS_STR_FMT_SYN_AMPA,    IZ_SYN_CFG.ampaConnectionRate,   IZ_SYN_CFG.ampaDisconnectionRate, IZ_SYN_CFG.ampaReversalPotential);
    LineTextDraw(rightPanel.x, &offsetY, NET_CFG_LINE_SPACING, LIGHTGRAY, LBL_FONTSIZE, WS_STR_FMT_SYN_GABA,    IZ_SYN_CFG.gaba_aConnectionRate, IZ_SYN_CFG.gaba_aDisconnectionRate, IZ_SYN_CFG.gaba_aReversalPotential);
    offsetY += NET_CFG_SECTION_GAP;

    SectionHeaderDraw(WS_STR_HDR_HH_SYNAPSE, rightPanel.x, &offsetY, NET_CFG_LINE_SPACING);
    LineTextDraw(rightPanel.x, &offsetY, NET_CFG_LINE_SPACING, LIGHTGRAY, LBL_FONTSIZE, WS_STR_FMT_SYN_SIGMOID, HH_SYN_CFG.KP, HH_SYN_CFG.VP);
    LineTextDraw(rightPanel.x, &offsetY, NET_CFG_LINE_SPACING, LIGHTGRAY, LBL_FONTSIZE, WS_STR_FMT_SYN_AMPA,    HH_SYN_CFG.ampaConnectionRate,   HH_SYN_CFG.ampaDisconnectionRate, HH_SYN_CFG.ampaReversalPotential);
    LineTextDraw(rightPanel.x, &offsetY, NET_CFG_LINE_SPACING, LIGHTGRAY, LBL_FONTSIZE, WS_STR_FMT_SYN_GABA,    HH_SYN_CFG.gaba_aConnectionRate, HH_SYN_CFG.gaba_aDisconnectionRate, HH_SYN_CFG.gaba_aReversalPotential);
}

static inline void LineTextDraw(float x, float *y, float spacing, Color color, int fontSize, const char *fmt, ...) {
    char buffer[256];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    DrawText(buffer, (int)x, (int)*y, fontSize, color);
    *y += spacing;
}

static inline void SectionHeaderDraw(const char *title, float x, float *y, float spacing) {
    DrawText(title, (int)x, (int)*y, 10, SKYBLUE);
    *y += spacing;
}
