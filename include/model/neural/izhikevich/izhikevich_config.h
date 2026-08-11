/**
 * @file izhikevich_config.h
 * @brief Defines the configuration structures, types, and parameter sets
 * for the Izhikevich neuron model.
 */
#ifndef IZHIKEVICH_CONFIG_H
#define IZHIKEVICH_CONFIG_H

/**
 * @brief The peak voltage (in mV) that triggers a spike and reset.
 */
#define IZHIKEVICH_SPIKE_PEAK 30.00

/**
 * @enum IzNeuronType
 * @brief Enumerates the different firing patterns available for the Izhikevich model.
 */
typedef enum {
    CHATTERING = 0,
    FAST_SPIKING,
    INTRINSICALLY_BURSTING,
    LOW_THRESHOLD_SPIKING,
    REGULAR_SPIKING,
    RESONATOR,
    THALAMO_CORTICAL,

    IZ_NEURON_COUNT
} IzNeuronType;

/**
 * @struct IzhikevichConfig
 * @brief Holds the four parameters (a, b, c, d) that define a specific
 * Izhikevich neuron firing pattern.
 *
 * @param a The time scale of the recovery variable 'u'.
 * @param b The sensitivity of 'u' to the membrane potential 'v'.
 * @param c The after-spike reset value of the membrane potential 'v'.
 * @param d The after-spike reset value for the recovery variable 'u'.
 */
typedef struct {
    IzNeuronType type;
    double a;
    double b;
    double c;
    double d;
} IzhikevichConfig;

/**
 * @brief Array of preset configurations for different neuron types.
 */
extern const IzhikevichConfig IZHIKEVICH_PARAMETERS[IZ_NEURON_COUNT];

#endif // IZHIKEVICH_CONFIG_H
