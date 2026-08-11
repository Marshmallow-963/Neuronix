/**
 * @file ampa_gaba_a_config.c
 * @brief Defines the constant parameter values for the synapse models.
 */
#include "model/synaptic/ampa-gaba-a/ampa_gaba_a_config.h"

/**
 * @brief Maximum neurotransmitter concentration (T_max).
 */
const double T_MAX = 1.0;

/**
 * @brief Default synapse parameters for Izhikevich neurons.
 */

const IzhikevichSynapsConfig IZ_SYN_CFG = {
    // Parâmetros da concentração de neurotransmissores [T](v_pre)
    .KP                       = 5.0f, // Kp: constante de inclinação da função sigmoide
    .VP                       = 2.0f, // Vp: constante de potencial da função sigmoide

    // Parâmetros para a sinapse excitatória (AMPA)
    .ampaConnectionRate       = 1.1f,  // αE: taxa de conexão (mM ms)^{-1}
    .ampaDisconnectionRate    = 0.30f, // βE: taxa de desconexão (ms^{-1})
    .ampaReversalPotential    = 0.0f,  // EE: potencial de reversão excitatório (mV)
    .ampaMaximumConductancy   = 0.0f,  // gE: condutância sináptica excitatória, Default to 0, set externally

    // Parâmetros para a autapse inibitória (GABA_A)
    .gaba_aConnectionRate     = 5.0f,   // αI: taxa de conexão (mM ms)^{-1}
    .gaba_aDisconnectionRate  = 0.188f,  // βI: taxa de desconexão (ms⁻¹)
    .gaba_aReversalPotential  = -80.0f, // EI: potencial de reversão inibitório (mV)
    .gaba_aMaximumConductancy = 0.0f    // gI: condutância autáptica inibitória, Default to 0, set externally
};

/**
 * @brief Default synapse parameters for Hodgkin-Huxley neurons.
 * Note the different V_p and reversal potentials, scaled for the HH voltage range.
 */
const HodgkinHuxleySynapseConfig HH_SYN_CFG = {
    .KP                       = 5.0,
    .VP                       = 62.0, // V_p adjusted for HH spiking
    .ampaConnectionRate       = 1.1,
    .ampaDisconnectionRate    = 0.19, // Slower decay for HH
    .ampaReversalPotential    = 60.0, // E_rev adjusted for HH
    .ampaMaximumConductancy   = 0.0,  // Default to 0, set externally
    .gaba_aConnectionRate     = 5.0,
    .gaba_aDisconnectionRate  = 0.18,
    .gaba_aReversalPotential  = -80.0,
    .gaba_aMaximumConductancy = 0.0   // Default to 0, set externally
};
