/**
 * @file neuron_models.h
 * @brief Catalog of supported neuronal mathematical models.
 */

#ifndef NEURON_MODELS_H
#define NEURON_MODELS_H

/**
 * @enum NeuronModel
 * @brief Identifiers for the different neuron models available in the system.
 */
typedef enum {
    IZHIKEVICH_MODEL = 0,    ///< Computationally efficient model for various firing patterns.
    HODGKIN_HUXLEY_MODEL,    ///< Biologically detailed model based on ion channel kinetics.

    NEURON_MODEL_COUNT       ///< Total number of supported models.
} NeuronModel;

#endif // NEURON_MODELS_H
