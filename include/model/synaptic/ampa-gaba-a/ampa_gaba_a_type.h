#ifndef AMPA_GABA_A_TYPE_H
#define AMPA_GABA_A_TYPE_H

/**
 * @enum AmpaGabaaSynapseType
 * @brief Enumerates the type of synapse (excitatory or inhibitory).
 */
typedef enum {
    AMPA = 0, ///< Excitatory synapse
    GABA_A,    ///< Inhibitory synapse

    SYN_COUNT
} AmpaGabaaSynapseType;

#endif // AMPA_GABA_A_TYPE_H
