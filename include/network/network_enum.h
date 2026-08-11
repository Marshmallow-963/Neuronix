/**
 * @file network_enum.h
 * @brief Definitions for network topologies and classification types.
 */

#ifndef NETWORK_ENUM_H
#define NETWORK_ENUM_H

/**
 * @enum NetworkTopology
 * @brief Defines the structural organization of the network.
 */
typedef enum {
    TOPOLOGY_GRAPH_NEURAL_NETWORK = 0, ///< General directed graph structure.
} NetworkTopology;

/**
 * @enum NetworkType
 * @brief Classifies the network based on the types of neuron models it contains.
 */
typedef enum {
    NETWORK_EMPTY = 0,    ///< Network contains no neurons.
    NET_TYPE_PURE_IZ,     ///< Contains only Izhikevich models.
    NET_TYPE_PURE_HH,     ///< Contains only Hodgkin-Huxley models.
    NET_TYPE_HYBRID       ///< Contains a mix of different neuron models.
} NetworkType;

#endif // NETWORK_TOPOLOGY_H
