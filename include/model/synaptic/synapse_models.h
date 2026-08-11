/**
 * @file synapse_models.h
 * @brief Catalog of supported synaptic receptor models.
 */

#ifndef SYNAPSE_MODELS_H
#define SYNAPSE_MODELS_H

/**
 * @enum SynapseModels
 * @brief Identifiers for neurotransmitter receptor types.
 */
typedef enum {
    AMPA_MODEL = 0,         ///< Fast excitatory glutamatergic receptors.
    GABA_A_MODEL,           ///< Fast inhibitory GABAergic receptors.

    SYNAPSE_MODEL_COUNT     ///< Total number of supported synapse models.
} SynapseModels;

#endif // SYNAPSE_MODELS_H
