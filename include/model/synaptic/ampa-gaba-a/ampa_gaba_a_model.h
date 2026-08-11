/**
 * @file ampa_gaba_a_model.h
 * @brief Public interface (API) for the AMPA/GABA-A synapse model simulation.
 *
 * This module provides a unified model for chemical synapses, capable of simulating
 * both excitatory (AMPA) and inhibitory (GABA-A) dynamics. It models the release
 * of neurotransmitters in the cleft and the resulting postsynaptic current.
 */
#ifndef AMPA_GABA_A_MODEL_H
#define AMPA_GABA_A_MODEL_H

#include "model/neural/neuron_models.h"

#include "model/synaptic/ampa-gaba-a/ampa_gaba_a_type.h"
#include "model/synaptic/ampa-gaba-a/ampa_gaba_a_struct.h"

/**
 * @brief Allocates memory for and initializes a new synapse model.
 *
 * This function creates the synapse instance, configuring it based on the
 * type of neurotransmitter (excitatory/inhibitory) and the target neuron.
 *
 * @param synType The specific type of synapse: AMPA (Excitatory) or GABA_A (Inhibitory).
 * @param nrnType The type of the post-synaptic neuron (Hodgkin-Huxley or Izhikevich),
 * which determines the parameter set used.
 * @param dt The simulation time step in milliseconds (ms).
 * @return AmpaGabaaModel* A pointer to the allocated model, or NULL if memory allocation fails.
 */
AmpaGabaaModel *AmpaGabaaInitModel(AmpaGabaaSynapseType synType, NeuronModel nrnType, double dt);

/**
 * @brief Connects the synapse model to the pre- and post-synaptic neurons.
 *
 * This function establishes the "wiring" by storing pointers to the voltage variables
 * of the connected neurons. This allows the synapse to read the pre-synaptic voltage
 * (to trigger release) and affect the post-synaptic current.
 *
 * @param model Pointer to the synapse model.
 * @param preVolt Pointer to the membrane voltage variable of the PRE-synaptic neuron.
 * @param postVolt Pointer to the membrane voltage variable of the POST-synaptic neuron.
 * @param postIsyn Pointer to the synaptic current accumulator of the POST-synaptic neuron.
 * The calculated current will be ADDED to this variable.
 * @return true if connection was successful; false if any pointer is NULL.
 */
void AmpaGabaaConnectSynapse(AmpaGabaaModel *model, double *preVolt, double *postVolt, double *postIsyn);

/**
 * @brief Configures the maximum conductance (g_max) of the synapse.
 *
 * This parameter effectively sets the "weight" or strength of the synaptic connection.
 * Higher conductance leads to stronger postsynaptic currents.
 *
 * @param model Pointer to the synapse model.
 * @param g The desired maximum conductance (typically in mS/cm^2 or similar).
 * @return true on success, false if model is NULL.
 */
void AmpaGabaaSetMaximumConductancy(AmpaGabaaModel *model, double g);

/**
 * @brief Advances the synapse simulation by one time step.
 *
 * 1. Calculates neurotransmitter concentration in the cleft based on Pre-synaptic voltage.
 * 2. Integrates the fraction of open receptors (r) using RK4.
 * 3. Computes the synaptic current: I_syn = g_max * r * (V_post - E_rev).
 *
 * @param model Pointer to the synapse model.
 * @return true on success, false if model is NULL.
 */
void AmpaGabaaUpdateModel(AmpaGabaaModel *model);

/**
 * @brief Frees all memory associated with the synapse model.
 *
 * @param model Pointer to the model to be freed.
 * @return true if memory was freed, false if the pointer was already NULL.
 */
void AmpaGabaaFreeModel(AmpaGabaaModel *model);

#endif // AMPA_GABA_A_MODEL_H
