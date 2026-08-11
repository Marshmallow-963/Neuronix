/**
 * @file ampa_gaba_a_struct.h
 * @brief Defines the core data structures for the AMPA/GABA-A synapse simulation.
 */
#ifndef AMPA_GABA_A_STRUCT_H
#define AMPA_GABA_A_STRUCT_H

#include "solver/rk4.h"

/**
 * @struct ReceptorParams
 * @brief Pointers to the parameters defining the post-synaptic receptor kinetics.
 * These point to locations in the 'internalBuffer'.
 */
typedef struct {
    double *eRev;      ///< Pointer to reversal potential (E_rev)
    double *gMax;      ///< Pointer to maximum conductance (g_max)
    double *alphaRate; ///< Pointer to connection rate (alpha_r)
    double *betaRate;  ///< Pointer to disconnection rate (beta_r)
} ReceptorParams;

/**
 * @struct NeurotransmitterParams
 * @brief Pointers to parameters and external states for neurotransmitter (NT) release.
 */
typedef struct {
    double *vP;        ///< Pointer to midpoint voltage for NT release
    double *kP;        ///< Pointer to steepness factor for NT release
    double *tMax;      ///< Pointer to maximum NT concentration
    double *vPre;      ///< Pointer to the PRE-synaptic neuron's voltage
    double *vPost;     ///< Pointer to the POST-synaptic neuron's voltage
    double *iSynPost;  ///< Pointer to the POST-synaptic neuron's synaptic current variable (Isyn)
} NeurotransmitterParams;

/**
 * @struct SynapseState
 * @brief Pointers to the dynamic state variables of the synapse.
 */
typedef struct {
    double *synCurrent;      ///< Pointer to the calculated synaptic current (I_syn)
    double *openChannels;    ///< Pointer to the fraction of open channels (r) - This is the main state variable
    double *ntConcentration; ///< Pointer to the calculated neurotransmitter concentration (T)
} SynapseState;

/**
 * @struct AmpaGabaaSynapse
 * @brief Groups all components of a single synapse.
 */
typedef struct {
    SynapseState state;
    ReceptorParams receptor;
    NeurotransmitterParams ntParams;
} AmpaGabaaSynapse;

/**
 * @struct AmpaGabaaModel
 * @brief The main structure that encapsulates the entire synapse model.
 */
typedef struct {
    RK4 integrator;             ///< Runge-Kutta 4 integrator instance
    double *stateVector;         ///< Contiguous buffer for the state variable (r)
    double *internalBuffer;      ///< Contiguous buffer for all parameters and other states
    AmpaGabaaSynapse synapse;   ///< The synapse instance
} AmpaGabaaModel;

#endif // AMPA_GABA_A_STRUCT_H
