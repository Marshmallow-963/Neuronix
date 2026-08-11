/**
 * @file ampa_gaba_a_config.h
 * @brief Defines configuration constants, enums, and structs for
 * AMPA/GABA-A synapse models.
 */
#ifndef AMPA_GABA_A_CONFIG_H
#define AMPA_GABA_A_CONFIG_H

/** @brief Default pre-synaptic voltage if not connected */
#define DEFAULT_VOLTAGE -70.0

/** @brief Default initialized state value (e.g., for concentration, current). */
#define SYN_DEFAULT_STATE 0.0

/** @brief Threshold for the diff (vPre - vP)/kP to calculate the sigmoid, avoiding expf underflow. */
#define SYN_SIGMOID_THRESHOLD -10.0

/** @brief Minimum concentration epsilon; values below this are clamped to zero. */
#define SYN_NT_EPSILON 0.001

/**
 * @brief Maximum neurotransmitter concentration (T_max).
 * Used in the sigmoidal function for neurotransmitter release.
 */
extern const double T_MAX;

/**
 * @struct IzhikevichSynapsConfig
 * @brief Synaptic parameters tailored for connection to Izhikevich neurons.
 */
typedef struct {
    double KP;                       ///< Steepness of the NT release sigmoid
    double VP;                       ///< Midpoint voltage of the NT release sigmoid
    double ampaConnectionRate;       ///< Alpha rate for AMPA
    double ampaDisconnectionRate;    ///< Beta rate for AMPA
    double ampaReversalPotential;    ///< Reversal potential (E_rev) for AMPA
    double ampaMaximumConductancy;   ///< Maximum conductance (g_max) for AMPA
    double gaba_aConnectionRate;     ///< Alpha rate for GABA-A
    double gaba_aDisconnectionRate;  ///< Beta rate for GABA-A
    double gaba_aReversalPotential;  ///< Reversal potential (E_rev) for GABA-A
    double gaba_aMaximumConductancy; ///< Maximum conductance (g_max) for GABA-A
} IzhikevichSynapsConfig;

/**
 * @struct HodgkinHuxleySynapseConfig
 * @brief Synaptic parameters tailored for connection to Hodgkin-Huxley neurons.
 */
typedef struct {
    double KP;                       ///< Steepness of the NT release sigmoid
    double VP;                       ///< Midpoint voltage of the NT release sigmoid
    double ampaConnectionRate;       ///< Alpha rate for AMPA
    double ampaDisconnectionRate;    ///< Beta rate for AMPA
    double ampaReversalPotential;    ///< Reversal potential (E_rev) for AMPA
    double ampaMaximumConductancy;   ///< Maximum conductance (g_max) for AMPA
    double gaba_aConnectionRate;     ///< Alpha rate for GABA-A
    double gaba_aDisconnectionRate;  ///< Beta rate for GABA-A
    double gaba_aReversalPotential;  ///< Reversal potential (E_rev) for GABA-A
    double gaba_aMaximumConductancy; ///< Maximum conductance (g_max) for GABA-A
} HodgkinHuxleySynapseConfig;

/** @brief Global instance of synapse config for Izhikevich models. */
extern const IzhikevichSynapsConfig IZ_SYN_CFG;
/** @brief Global instance of synapse config for Hodgkin-Huxley models. */
extern const HodgkinHuxleySynapseConfig HH_SYN_CFG;

#endif // AMPA_GABA_A_CONFIG_H
