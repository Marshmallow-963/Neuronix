/**
 * @file neuron_common.h
 * @brief Polymorphic interface for neuron model abstraction.
 * * Defines the function pointers and structures required to wrap different
 * electrophysiological models (e.g., Izhikevich, Hodgkin-Huxley) into a
 * common interface for the neural network manager.
 */

#ifndef NEURON_COMMON_H
#define NEURON_COMMON_H

#include <stdbool.h>

/**
 * @typedef NeuronUpdateFn
 * @brief Function pointer to advance the neuron's state by one time step (dt).
 * @param modelData Pointer to the specific model's internal structure.
 * @return The new membrane potential (mV) after the update.
 */
typedef double (*NeuronUpdateFn)(void *modelData);

/**
 * @typedef NeuronSetCurrentFn
 * @brief Function pointer to set the external/synaptic input current.
 * @param modelData Pointer to the specific model's internal structure.
 * @param iExt The input current value (typically in pA or nA).
 */
typedef void (*NeuronSetCurrentFn)(void *modelData, double iExt);

/**
 * @typedef NeuronFreeFn
 * @brief Function pointer to clean up specific neuron model memory.
 */
typedef void (*NeuronFreeFn)(void *modelData);

/**
 * @struct NeuronInterface
 * @brief A generic wrapper for any neuron model.
 * * This structure acts as a "Virtual Function Table" (VTable). It stores pointers
 * to model-specific logic and provides direct access to the most frequently
 * accessed variables to avoid the overhead of function calls during rendering.
 */
typedef struct {
    // Direct access pointers for performance and networking
    double *membranePotential;       ///< Pointer to the membrane voltage (V) variable inside modelData.
    double *synapticCurrent;         ///< Pointer to the total input current variable inside modelData.

    NeuronSetCurrentFn setCurrent;  ///< Logic to update the input stimulus.
    NeuronUpdateFn update;          ///< Logic to calculate the next ODE step.
    NeuronFreeFn free;              ///< Logic to deallocate the internal model.

    void *modelData;                ///< Opaque pointer to the concrete model (e.g., IzhikevichModel).
} NeuronInterface;

#endif // NEURON_COMMON_H
