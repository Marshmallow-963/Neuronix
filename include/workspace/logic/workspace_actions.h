#ifndef WORKSPACE_ACTIONS_H
#define WORKSPACE_ACTIONS_H

#include "workspace/core/workspace_state.h"

/* --- Analyzer Actions --- */
void RasterPlotDraw(WorkspaceState *state, size_t chunkSize);

void SessionDelete(WorkspaceState *state, size_t sessionID);
void SessionDownload(WorkspaceState *state, size_t sessionID);
void SpikeTimingCalculate(WorkspaceState *state, size_t chunckSize);

void TableDelete(WorkspaceState *state, const char *tableName, size_t sessionID);
void TableDownload(WorkspaceState *state, const char *tableName, size_t sessionID);

/* --- Simulator Actions --- */
void AutoSaveToggle(WorkspaceState *state);

void NeuronAdd(WorkspaceState *state, NeuronModel nrnModel, IzNeuronType izType);
void NetworkDelete(WorkspaceState *state);

void SimulationReset(WorkspaceState *state);
void SimulationToggle(WorkspaceState *state);
void SynapseAdd(WorkspaceState *state);

#endif // WORKSPACE_ACTIONS_H
