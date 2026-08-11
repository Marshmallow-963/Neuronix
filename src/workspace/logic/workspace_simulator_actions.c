#include "network/network_manager.h"

#include "workspace/logic/workspace_actions.h"
#include "workspace/logic/workspace_logic.h"

/* =========================================================================
 * SIMULATOR ACTIONS
 * ========================================================================= */

void AutoSaveToggle(WorkspaceState *state) {
    state->saving.isSaving = !state->saving.isSaving;
}

void NetworkDelete(WorkspaceState *state) {
    if (!state) return;

    pthread_mutex_lock(&state->concurrency.simMutex);
        state->saving.lastSavedDataCount = 0;
        state->saving.autoSaveSessionId = 0;
        state->saving.isSaving = false;

        SimulationDataReset(state);
        SimulationCleanup(state);

        if (state->topology.nrnLayouts != NULL) {
            free(state->topology.nrnLayouts);
            state->topology.nrnLayouts = NULL;
        }

        if (state->topology.synLayouts != NULL) {
            free(state->topology.synLayouts);
            state->topology.synLayouts = NULL;
        }

        state->topology.count = 0;
        state->topology.synapseCount = 0;
    pthread_mutex_unlock(&state->concurrency.simMutex);
}

void NeuronAdd(WorkspaceState *state, NeuronModel nrnModel, IzNeuronType izType) {
    if (!state) return;

    SimulationNeuronInject(state, nrnModel, izType);
}

void SimulationReset(WorkspaceState *state) {
    if (!state) return;

    pthread_mutex_lock(&state->concurrency.simMutex);
        state->saving.lastSavedDataCount = 0;
        state->saving.autoSaveSessionId = 0;
        state->saving.isSaving = false;

        SimulationDataReset(state);
    pthread_mutex_unlock(&state->concurrency.simMutex);
}


void SimulationToggle(WorkspaceState *state) {
    if (state && state->network && state->network->neuronCount > 0) {
        state->runtime.isRunning = !state->runtime.isRunning;
    }
}

void SynapseAdd(WorkspaceState *state) {
    if (!state) return;

    pthread_mutex_lock(&state->concurrency.simMutex);
        if (!state->network) {
            pthread_mutex_unlock(&state->concurrency.simMutex);
            return;
        }

        NeuralNetwork *net = state->network;
        if (net && net->neuronCount > 0) {
            size_t src = state->editor.sourceNeuronId;
            size_t tgt = state->editor.targetNeuronId;

            if (src >= net->neuronCount) src = 0;
            if (tgt >= net->neuronCount) tgt = 0;

            double conductancy = state->params.synapseConductancy;
            size_t selectedIndex = state->editor.activeSynapseModel;

            double delayMs = 0.00f;
            if (src == tgt) delayMs = state->params.autapseDelay;

            SynapseModels synType = (selectedIndex == 0) ? AMPA_MODEL : GABA_A_MODEL;
            NetworkConnectSynapse(net, synType, src, tgt, conductancy, delayMs);

            size_t newSynCount = state->topology.synapseCount + 1;
            SynapseLayout *newSynLayouts = realloc(state->topology.synLayouts, newSynCount * sizeof(SynapseLayout));
            if (newSynLayouts) {
                state->topology.synLayouts = newSynLayouts;
                state->topology.synLayouts[state->topology.synapseCount] = (SynapseLayout){ 0 };
            }
        }
    pthread_mutex_unlock(&state->concurrency.simMutex);
}
