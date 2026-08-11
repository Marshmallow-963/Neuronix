/*
 * @file hodgkin_huxley_config.h
 * @brief defines the configuration structure and constants parameters
 * for the Hodgkin-Huxley model.
 */
#ifndef HODGKIN_HUXLEY_CONFIG_H
#define HODGKIN_HUXLEY_CONFIG_H

/*
 * @struct HodgkinHuxleyConfig
 * @brief Stores the constant biophysical parameters of the model.
 *
 * These values are typically derived from experiments (like those of Hodgkin and Huxley)
 * and are used to initialize the model's parameters.
 */
typedef struct {
    double restingPotential;
    double membraneCapacitancy;
    double leakReversal;
    double sodiumReversal;
    double potassiumReversal;
    double leakConductance;
    double sodiumConductance;
    double potassiumConductance;
} HodgkinHuxleyConfig;

/**
 * @brief Global, immutable configuration of the neuron parameters.
 *
 * External instance of the default HH model parameters.
 */
extern const HodgkinHuxleyConfig HH_SQUID_AXON;

#endif // HODGKIN_HUXLEY_CONFIG_H
