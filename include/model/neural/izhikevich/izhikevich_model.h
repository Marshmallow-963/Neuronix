/**
 * @file izhikevich_model.h
 * @brief Public interface (API) for the Izhikevich neuron model simulation.
 *
 * This module implements the efficient Izhikevich model, which reproduces
 * the firing patterns of many biological neurons using a system of two
 * ordinary differential equations.
 * * References:
 * Izhikevich, E. M. (2003). "Simple model of spiking neurons".
 */
#ifndef IZHIKEVICH_MODEL_H
#define IZHIKEVICH_MODEL_H

#include <stdbool.h>

#include "izhikevich_config.h"
#include "izhikevich_struct.h"

/**
 * @brief Allocates and initializes a new Izhikevich neuron model.
 *
 * Allocates memory for the model structure, the integrator (RK4), and the
 * state buffers. It initializes the parameters (a, b, c, d) according
 * to the specified neuron type.
 *
 * @param type The desired firing pattern configuration (e.g., REGULAR_SPIKING, CHATTERING).
 * @param dt The fixed simulation time step in milliseconds (ms).
 * @return IzhikevichModel* A pointer to the newly allocated model, or NULL if allocation fails.
 */
IzhikevichModel* IzhikevichInitModel(const IzNeuronType type, const double dt);

/**
 * @brief Updates the external input current (I_ext) applied to the neuron.
 *
 * @param model Pointer to the Izhikevich model instance.
 * @param iExt The magnitude of the external current (arbitrary units, typically pA or similar scale).
 * @return true if the current was successfully set; false if the model pointer is NULL.
 */
void IzhikevichSetExternalCurrent(IzhikevichModel *model, double iExt);

/**
 * @brief Advances the neuron simulation by one time step.
 *
 * Solves the differential equations for membrane potential (v) and recovery variable (u)
 * using the RK4 method. It also handles the spike reset mechanism:
 * if v >= 30mV, then v is reset to c, and u is reset to u + d.
 *
 * @param model Pointer to the Izhikevich model instance.
 * @return double The updated membrane potential (v). If a spike occurred during this step,
 * it returns the peak constant (IZHIKEVICH_SPIKE_PEAK) to visualize the spike.
 */
double IzhikevichUpdateModel(IzhikevichModel *model);

/**
 * @brief Frees all memory resources associated with the Izhikevich model.
 *
 * Destroys the integrator and releases the allocated buffers for state and parameters.
 *
 * @param model Pointer to the model structure to be freed.
 * @return true if memory was successfully freed; false if the pointer was already NULL.
 */
void IzhikevichFreeModel(IzhikevichModel *model);

#endif // IZHIKEVICH_MODEL_H
