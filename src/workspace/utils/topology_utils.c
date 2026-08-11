#include <stdio.h>

#include "model/neural/izhikevich/izhikevich_config.h"
#include "model/neural/izhikevich/izhikevich_struct.h"

#include "workspace/utils/topology_utils.h"

static inline const char* IzTypeStringGet(IzNeuronType type);

void NeuronTypeFormat(char *buffer, size_t size, const NeuronNode *node) {
    if (node->type == IZHIKEVICH_MODEL && node->interface.modelData != NULL) {
        IzhikevichModel *model = (IzhikevichModel*)node->interface.modelData;
        snprintf(buffer, size, "IZ_%s", IzTypeStringGet(model->neuron.config.type));
    }
    else snprintf(buffer, size, "HH");
}

static inline const char* IzTypeStringGet(IzNeuronType type) {
    switch (type) {
        case CHATTERING:             return "CH";
        case FAST_SPIKING:           return "FS";
        case INTRINSICALLY_BURSTING: return "IB";
        case LOW_THRESHOLD_SPIKING:  return "LTS";
        case REGULAR_SPIKING:        return "RS";
        case RESONATOR:              return "RZ";
        case THALAMO_CORTICAL:       return "TC";
        default:                     return "UNK";
    }
}
