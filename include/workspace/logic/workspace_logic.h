#ifndef WORKSPACE_LOGIC_H
#define WORKSPACE_LOGIC_H

#include "workspace/core/workspace_state.h"

void *AnalyzerWorkerThread(void *arg);
void AnalyzerStateReset(WorkspaceState *state);

void SimulationCleanup(WorkspaceState *state);
void SimulationDataReset(WorkspaceState *state);
void SimulationUpdate(WorkspaceState *state);
void SimulationNeuronInject(WorkspaceState *state, NeuronModel nrnModel, IzNeuronType izType);
void *SimulationWorkerThread(void *arg);

#endif // WORKSPACE_LOGIC_H
