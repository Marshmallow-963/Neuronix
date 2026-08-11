/**
 * @file network_manager.h
 * @brief Management API for the Neural Network graph and simulation loop.
 * * This module provides the high-level interface to create neurons, establish
 * synaptic connections, and advance the simulation state. It abstracts the
 * underlying complexity of heterogeneous neuron models (Izhikevich/Hodgkin Huxley).
 */

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <stdbool.h>

#include "network_enum.h"
#include "network_struct.h"

#include "model/neural/izhikevich/izhikevich_config.h"

/**
 * @brief Allocates and initializes a new neural network container.
 * @param maxNeurons Maximum number of neurons the network can hold.
 * @param dt Integration time step in milliseconds (e.g., 0.1ms).
 * @return Pointer to the allocated NeuralNetwork, or NULL on failure.
 */
NeuralNetwork *NetworkCreate(size_t maxNeurons, double dt);

/**
 * @brief Adds a new neuron node to the network.
 * @param network Pointer to the network instance.
 * @param nrnType The mathematical model type (IZHIKEVICH or HODGKIN_HUXLEY).
 * @param type Specific subtype (only relevant for Izhikevich models).
 */
void NetworkAddNeuron(NeuralNetwork *network, NeuronModel nrnType, IzNeuronType type);

/**
 * @brief Advances the entire network state by one time step (dt).
 * * This follows a two-pass update strategy:
 * 1. Update all neuron membrane potentials and detect spikes.
 * 2. Update all synapses and propagate neurotransmitter currents.
 * * @param network Pointer to the network instance.
 */
void NetworkUpdate(NeuralNetwork *network);

/**
 * @brief Deallocates all memory used by the network, its neurons, and synapses.
 * @param network Pointer to the network instance to be destroyed.
 */
void NetworkFree(NeuralNetwork *network);

/**
 * @brief Establishes a directed synaptic connection between two neurons.
 * * @param network     Pointer to the network instance.
 * @param type        Synapse receptor type (AMPA or GABA_A).
 * @param sourceId    ID of the pre-synaptic neuron.
 * @param targetId    ID of the post-synaptic neuron.
 * @param conductancy Maximum synaptic conductance (weight).
 * @param delayMs     Propagation delay in milliseconds.
 * @return true if connection was successful, false otherwise.
 */
bool NetworkConnectSynapse(
    NeuralNetwork *network,
    SynapseModels type,
    size_t sourceId,
    size_t targetId,
    double conductancy,
    double delayMs
);

bool NetworkResizeCapacity(NeuralNetwork *net, size_t newCapacity);

/**
 * @brief Generates an adjacency matrix representing the network connectivity.
 * @param net Pointer to the network.
 * @return A flat array of ConnectivityCell representing an [N x N] matrix.
 */
ConnectivityCell *NetworkGetAdjacencyMatrix(NeuralNetwork *net);

double NetworkNeuronVoltageGet(const NeuralNetwork *network, size_t neuronId);

double NetworkNeuronExternalCurrentGet(const NeuralNetwork *network, size_t neuronId);

/**
 * @brief Counts the total number of synaptic connections (edges) in the graph.
 * @param net Pointer to the network.
 */
int NetworkGetTotalSynapse(NeuralNetwork *network);

/**
 * @brief Detects if the network is pure (single model) or hybrid (mixed models).
 * @param net Pointer to the network.
 */
NetworkType NetworkDetectType(NeuralNetwork *net);

void NetworkNeuronExternalCurrentSet(NeuralNetwork *network, size_t neuronId, double iExt);

// Em network_manager.c

double NetworkNeuronSynapticCurrentGet(const NeuralNetwork *network, size_t neuronId);

#endif // NETWORK_MANAGER_H
