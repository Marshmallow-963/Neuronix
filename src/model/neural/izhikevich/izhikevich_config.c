/**
 * @file izhikevich_config.c
 * @brief Defines the constant parameter values for the Izhikevich model.
 */
#include "model/neural/izhikevich/izhikevich_config.h"

/**
 * @brief Global array of parameters for different neuron types.
 *
 * Sourced from Izhikevich, E. M. (2003). "Simple model of spiking neurons".
 */
 const IzhikevichConfig IZHIKEVICH_PARAMETERS[IZ_NEURON_COUNT] = {
     // Agora o compilador amarra a struct na exata posição do enum
     [CHATTERING]             = {.type = CHATTERING,             .a = 0.02f, .b = 0.20f, .c = -50.0f, .d =  2.0f},
     [FAST_SPIKING]           = {.type = FAST_SPIKING,           .a = 0.010f, .b = 0.20f, .c = -65.0f, .d =  2.0f},
     [INTRINSICALLY_BURSTING] = {.type = INTRINSICALLY_BURSTING, .a = 0.02f, .b = 0.20f, .c = -55.0f, .d =  4.0f},
     [LOW_THRESHOLD_SPIKING]  = {.type = LOW_THRESHOLD_SPIKING,  .a = 0.02f, .b = 0.25f, .c = -65.0f, .d =  2.0f},
     [REGULAR_SPIKING]        = {.type = REGULAR_SPIKING,        .a = 0.02f, .b = 0.20f, .c = -65.0f, .d =  8.0f},
     [RESONATOR]              = {.type = RESONATOR,              .a = 0.10f, .b = 0.26f, .c = -60.0f, .d = -1.0f},
     [THALAMO_CORTICAL]       = {.type = THALAMO_CORTICAL,       .a = 0.02f, .b = 0.25f, .c = -65.0f, .d =  0.05f}
};
