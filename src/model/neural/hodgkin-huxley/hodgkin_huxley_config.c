/**
 * @file hodgkin_huxley_config.c
 * @brief Implementation and definition of default configuration values
 * for the Hodgkin-Huxley model.
 */
#include "model/neural/hodgkin-huxley/hodgkin_huxley_config.h"

/**
 * @brief Pi constant, private to this file.
 *
 * Used to calculate capacitance and conductance values based on membrane area.
 * These specific values are sourced from Tabela 1 (Table 1) of the
 * thesis "Dinâmica de circuitos neuronais", UFAL 2023 by J.M.G.L. Silva.
 */
#ifndef PI
    #define PI 3.14159265358979323846
#endif

/**
 * @brief Definition of the default Hodgkin-Huxley model parameters.
 *
 * These values are based on the original paper, adjusted for
 * the units used in the simulation.
 */
const HodgkinHuxleyConfig HH_SQUID_AXON = {
    .leakConductance      = 2.7 * PI,
    .leakReversal         = 10.613,
    .membraneCapacitancy  = 9.0 * PI,
    .potassiumConductance = 324.0 * PI,
    .potassiumReversal    = -12.0,
    .restingPotential     = 0.00,
    .sodiumConductance    = 1080.0 * PI,
    .sodiumReversal       = 115.0,
};
