/**
 * @file hodgkin_huxley_model.h
 * @brief Public interface (API) for the Hodgkin-Huxley model simulation.
 *
 * Implements the classic biophysical model describing how action potentials
 * in neurons are initiated and propagated. It models the sodium (Na+) and
 * potassium (K+) ion channels using a set of non-linear differential equations.
 */
#ifndef HODGKIN_HUXLEY_MODEL_H
#define HODGKIN_HUXLEY_MODEL_H

#include <stdbool.h>
#include "hodgkin_huxley_struct.h"

/**
 * @brief Allocates and initializes a new Hodgkin-Huxley neuron model.
 *
 * Sets up the data structures, allocates the RK4 integrator, and initializes
 * the state variables (Membrane Voltage V, and gates m, h, n) to their
 * resting equilibrium values.
 *
 * @param dt The simulation time step in milliseconds (ms).
 * @return HodgkinHuxleyModel* A pointer to the initialized model, or NULL if memory allocation fails.
 */
HodgkinHuxleyModel* HodgkinHuxleyInitModel(const double dt);

/**
 * @brief Sets the external input current (Stimulus) for the neuron.
 *
 * @param model Pointer to the Hodgkin-Huxley model.
 * @param iExt The external current density to apply (typically in uA/cm^2).
 * @return true if the value was set; false if the model pointer is NULL.
 */
void HodgkinHuxleySetExternalCurent(HodgkinHuxleyModel *model, double iExt);

/**
 * @brief Advances the simulation by one time step (dt).
 *
 * Updates the membrane potential (V) and the gating variables (m, h, n)
 * by integrating the differential equations.
 *
 * @param model Pointer to the Hodgkin-Huxley model.
 * @return double The new membrane potential (V) in millivolts (mV).
 * Returns 0.0f if the model is NULL.
 */
double HodgkinHuxleyUpdateModel(HodgkinHuxleyModel *model);

/**
 * @brief Frees all resources associated with the Hodgkin-Huxley model.
 *
 * @param model Pointer to the model to be destroyed.
 * @return true on success, false if the pointer was already NULL.
 */
void HodgkinHuxleyFreeModel(HodgkinHuxleyModel *model);

#endif // HODGKIN_HUXLEY_MODEL_H
