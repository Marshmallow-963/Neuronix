#include "graph/core/graph_state.h"

#include "workspace/plotting/workspace_plot_enums.h"
#include "workspace/plotting/workspace_plot_logic.h"

#include "utils/math_utils.h"

// =========================================================================
// SIMULATION (LIVE DATA) LOGIC
// =========================================================================

void WorkspacePlotLiveBoundsApply(WorkspaceState *state) {
    if (!state || !state->network) return;

    PlotStateResetAll();

    double xMinLimit = 0.0, xMaxLimit = 0.0;
    TimeWindow(state->runtime.currentTime, state->ui.timeWindow, state->network->dt, &xMinLimit, &xMaxLimit);

    int timeSlots[] = {
        SLOT_SIMULATOR_GATE, SLOT_SIMULATOR_CURRENT,
        SLOT_SIMULATOR_VOLTAGE, SLOT_SIMULATOR_SYNAPTIC,
        SLOT_SIMULATOR_IZ_RECOVERY, SLOT_SIMULATOR_NT_FRACTION
    };

    int timeSlotsCount = sizeof(timeSlots) / sizeof(timeSlots[0]);

    for (int i = 0; i < timeSlotsCount; i++) {
        PlotView *view = PlotStateGetView(timeSlots[i]);
        if (view) { view->xMin = xMinLimit; view->xMax = xMaxLimit; }
    }

    double dt = state->network->dt;
    if (dt <= 0.0) dt = 0.01;

    NeuroTraceBuffer *buf = &state->liveData;
    size_t cap = buf->isCircular ? buf->bufferCapacity : 0;

    size_t startIdx = TimeToIndex(xMinLimit, dt, 0);
    size_t endIdx   = TimeToIndex(xMaxLimit, dt, 0);

    size_t visibleCount = 0;
    if (buf->isCircular && cap > 0) {
        size_t absoluteCurrentStep = TimeToIndex(state->runtime.currentTime, dt, 0);
        if (endIdx > absoluteCurrentStep) endIdx = absoluteCurrentStep;

        size_t minValid = TimeToIndex(state->runtime.currentTime, dt, cap);
        if (startIdx < minValid) startIdx = minValid;

        if (endIdx > startIdx) visibleCount = endIdx - startIdx;
    }

    else {
        size_t totalPoints = buf->dataCount;
        size_t offsetPoints = TimeToIndex(state->runtime.currentTime, dt, totalPoints);

        startIdx = TimeToIndex(xMinLimit, dt, offsetPoints);
        endIdx   = TimeToIndex(xMaxLimit, dt, offsetPoints);

        if (startIdx >= totalPoints) startIdx = totalPoints > 0 ? totalPoints - 1 : 0;
        if (endIdx > totalPoints) endIdx = totalPoints;

        if (endIdx > startIdx) visibleCount = endIdx - startIdx;
    }

    if (visibleCount == 0) return;

    size_t nrnCount  = state->network->neuronCount;
    size_t plotLimit = (nrnCount < MAX_DETAILED_PLOTS) ? nrnCount : MAX_DETAILED_PLOTS;

    PlotStateUpdateMulti(buf->neuronTraces,     SLOT_SIMULATOR_VOLTAGE,     startIdx, visibleCount, plotLimit, cap);
    PlotStateUpdateMulti(buf->synapticTraces,   SLOT_SIMULATOR_SYNAPTIC,    startIdx, visibleCount, plotLimit, cap);
    PlotStateUpdateMulti(buf->phaseSpaceTraces, SLOT_SIMULATOR_PHASE,       startIdx, visibleCount, plotLimit, cap);
    PlotStateUpdateMulti(buf->ntFractionTraces, SLOT_SIMULATOR_NT_FRACTION, startIdx, visibleCount, plotLimit, cap);

    switch (state->network->neurons->type) {
        case IZHIKEVICH_MODEL: {
            PlotStateUpdateMulti(buf->izRecoveryTraces, SLOT_SIMULATOR_IZ_RECOVERY, startIdx, visibleCount, plotLimit, cap);
        } break;

        case HODGKIN_HUXLEY_MODEL: {
            for(size_t i = 0; i < state->network->neuronCount; i++) {
                PlotStateUpdate(buf->hhCurrentPlots.kCurrent[i],    SLOT_SIMULATOR_CURRENT, startIdx, visibleCount, cap);
                PlotStateUpdate(buf->hhCurrentPlots.naCurrent[i] ,  SLOT_SIMULATOR_CURRENT, startIdx, visibleCount, cap);
                PlotStateUpdate(buf->hhCurrentPlots.leakCurrent[i], SLOT_SIMULATOR_CURRENT, startIdx, visibleCount, cap);

                PlotStateUpdate(buf->hhGatePlots.NGate[i], SLOT_SIMULATOR_GATE, startIdx, visibleCount, cap);
                PlotStateUpdate(buf->hhGatePlots.MGate[i], SLOT_SIMULATOR_GATE, startIdx, visibleCount, cap);
                PlotStateUpdate(buf->hhGatePlots.HGate[i], SLOT_SIMULATOR_GATE, startIdx, visibleCount, cap);
            }
        } break;

        default: break;
    }
}
