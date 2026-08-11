/**
 * @file synapse_common.h
 * @brief Polymorphic interface and delay management for synaptic models.
 * * This file handles the abstraction of chemical synapses (AMPA/GABA) and
 * provides a wrapper to manage axonal/synaptic propagation delays using
 * a ring buffer mechanism.
 */

#ifndef SYNAPSE_COMMON_H
#define SYNAPSE_COMMON_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @typedef SynapsePhysicsUpdateFn
 * @brief Function pointer to update synaptic conductance or neurotransmitter fraction.
 */
typedef void (*SynapsePhysicsUpdateFn)(void *modelData);

/**
 * @typedef SynapseFreeFn
 * @brief Function pointer to deallocate synaptic model memory.
 */
typedef void (*SynapseFreeFn)(void *modelData);

/**
 * @struct SynapseInterface
 * @brief Generic interface for synaptic kinetic models.
 * * Encapsulates the mathematical behavior of receptors (e.g., opening/closing
 * of ion channels in response to pre-synaptic spikes).
 */
typedef struct {
    double *gMax;                    ///< Pointer to maximum conductance (Weight).
    double *openChannels;            ///< Pointer to the fraction of open channels (r).

    SynapsePhysicsUpdateFn update;  ///< Logic to advance synaptic kinetics.
    SynapseFreeFn free;             ///< Logic to clean up memory.

    void *modelData;                ///< Opaque pointer to model (e.g., AmpaGabaaSynapse).
} SynapseInterface;

/**
 * @struct SynapseWrapper
 * @brief Comprehensive synapse container with delay handling.
 * * Combines the chemical physics with a ring buffer (delay line) to simulate
 * the time it takes for an action potential to reach the post-synaptic neuron.
 */
 typedef struct {
     double *delayBuffer;             ///< Ring buffer storing historical membrane potentials.

     int writeIndex;                 ///< Current head of the buffer.
     int readIndex;                  ///< Current tail of the buffer (delayed signal).

     size_t bufferSize;                 ///< Total capacity of the delay line.
     SynapseInterface physics;       ///< The underlying kinetic model.
 } SynapseWrapper;

#endif // SYNAPSE_COMMON_H
