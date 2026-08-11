/**
 * @file network_struct.h
 * @brief Core data structures for the neural network graph.
 */

#ifndef NETWORK_STRUCT_H
#define NETWORK_STRUCT_H

#include "model/neural/neuron_common.h"
#include "model/neural/neuron_models.h"

#include "model/synaptic/synapse_common.h"
#include "model/synaptic/synapse_models.h"

/**
 * @struct ConnectivityCell
 * @brief Representation of a single cell in an adjacency matrix.
 */
typedef struct {
    size_t count;       ///< Number of synapses between two specific neurons.
    double netWeight;    ///< Sum of synaptic conductances (Excitatory - Inhibitory).
} ConnectivityCell;

/**
 * @struct SynapseEdge
 * @brief A linked-list node representing a synaptic connection (Edge).
 */
typedef struct SynapseEdge {
    SynapseModels edgeType;          ///< Type of synapse (AMPA/GABA).
    SynapseWrapper *wrapper;         ///< The synaptic physics and delay logic.

    size_t targetNeuron;             ///< ID of the post-synaptic neuron.
    struct SynapseEdge *nextSynapse; ///< Pointer to the next edge in the adjacency list.
} SynapseEdge;

/**
 * @struct NeuronNode
 * @brief A vertex in the neural graph.
 */
typedef struct {
    bool hasSpiked;                       ///< Spike flag for the current time step.
    double previousVolt;                  ///< Membrane potential from the last time step.
    double externalCurrent;               ///< Corrente externa injetada neste neurônio.

    size_t id;                            ///< Unique identifier.
    NeuronModel type;                     ///< Model type (IZ or HH).
    NeuronInterface interface;            ///< Polymorphic interface for updates.

    struct SynapseEdge *outboundSynapses; ///< Head of the linked list of connections.
} NeuronNode;

/**
 * @struct NeuralNetwork
 * @brief Master container for the simulation.
 */
typedef struct {
    double dt;           ///< Integration time step (ms).

    NeuronNode *neurons; ///< Contiguous array of neurons.

    size_t capacity;     ///< Maximum allowed neurons.
    size_t neuronCount;  ///< Current number of neurons.
} NeuralNetwork;

#endif // NETWORK_STRUCT_H
