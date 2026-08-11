/**
 * @file network_manager.c
 * @brief Implementation of the neural network management and processing core.
 * * This file contains the logic for creating heterogeneous neural graphs,
 * allowing Izhikevich and Hodgkin-Huxley models to coexist within the same network.
 * It implements a two-pass update system to ensure the consistency of synaptic signals
 * and utilizes circular buffers to simulate biological propagation delays.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "analysis/neuro_analysis.h"

#include "model/neural/neuron_common.h"
#include "model/neural/neuron_models.h"

#include "model/neural/izhikevich/izhikevich_model.h"
#include "model/neural/izhikevich/izhikevich_struct.h"

#include "model/neural/hodgkin-huxley/hodgkin_huxley_config.h"
#include "model/neural/hodgkin-huxley/hodgkin_huxley_model.h"
#include "model/neural/hodgkin-huxley/hodgkin_huxley_struct.h"

#include "model/synaptic/ampa-gaba-a/ampa_gaba_a_config.h"
#include "model/synaptic/ampa-gaba-a/ampa_gaba_a_model.h"
#include "model/synaptic/ampa-gaba-a/ampa_gaba_a_struct.h"

#include "network/network_struct.h"
#include "network/network_manager.h"

/* --- Internal Function Prototypes (Static) --- */

/**
 * @brief Configures the polymorphic interface for an Izhikevich model.
 * @param interface Pointer to the interface structure to be populated.
 * @param model Pointer to the concrete Izhikevich model data.
 */
static inline void NeuronSetupIzhikevich(NeuronInterface *interface, IzhikevichModel *model);

/**
 * @brief Configures the polymorphic interface for a Hodgkin-Huxley model.
 * @param interface Pointer to the interface structure to be populated.
 * @param model Pointer to the concrete Hodgkin-Huxley model data.
 */
static inline void NeuronSetupHodgkinHuxley(NeuronInterface *interface, HodgkinHuxleyModel *model);

/**
 * @brief Allocates and configures a synapse wrapper, including delay logic.
 * @param type Receptor model (AMPA/GABA).
 * @param targetType Type of the post-synaptic neuron.
 * @param conductancy Maximum conductance (synaptic weight).
 * @param delayMs Propagation delay in milliseconds.
 * @param dt Simulation time step.
 * @return SynapseWrapper* Pointer to the initialized synapse wrapper.
 */
static inline SynapseWrapper *SynapseCreateWrapper(
    NeuronModel targetType,
    SynapseModels type,
    double conductancy,
    double delayMs,
    double dt
);

/**
 * @brief Updates synaptic physics and manages the circular delay buffer.
 * @param sw Pointer to the synapse wrapper.
 */
static inline void SynapseUpdate(SynapseWrapper *sw);

/**
 * @brief Frees memory for a synapse wrapper and its delay buffer.
 */
static inline void SynapseFree(SynapseWrapper *sw);

/* --- Public API Implementation --- */

NeuralNetwork *NetworkCreate(size_t maxNeurons, double dt) {
    if (maxNeurons <= 0 || dt <= 0.0f) return NULL;

    NeuralNetwork *net = (NeuralNetwork*)malloc(sizeof(NeuralNetwork));
    if (!net) return NULL;

    net->neurons = (NeuronNode*)calloc(maxNeurons, sizeof(NeuronNode));
    if (!net->neurons) { free(net); return NULL; }

    net->dt          = dt;
    net->capacity    = maxNeurons;
    net->neuronCount = 0;

    return net;
}

void NetworkAddNeuron(NeuralNetwork *network, NeuronModel nrnType, IzNeuronType izType) {
    if (!network || network->neuronCount >= network->capacity) return;

    bool initialized = false;
    size_t id = network->neuronCount;
    NeuronNode *node = &network->neurons[id];

    memset(node, 0, sizeof(NeuronNode));

    switch (nrnType) {
        case IZHIKEVICH_MODEL: {
            if (izType < CHATTERING || izType > THALAMO_CORTICAL) return;

            IzhikevichModel *iz = IzhikevichInitModel(izType, network->dt);
            if (iz) {
                node->type = IZHIKEVICH_MODEL;
                NeuronSetupIzhikevich(&node->interface, iz);
                node->previousVolt = *(node->interface.membranePotential);
                initialized = true;
            }
        } break;

        case HODGKIN_HUXLEY_MODEL: {
            HodgkinHuxleyModel *hh = HodgkinHuxleyInitModel(network->dt);
            if (hh) {
                node->type = HODGKIN_HUXLEY_MODEL;
                NeuronSetupHodgkinHuxley(&node->interface, hh);
                node->previousVolt = HH_SQUID_AXON.restingPotential;
                initialized = true;
            }
        } break;

        default: return; break;
    }

    if (!initialized) return;

    node->id               = id;
    node->hasSpiked        = false;
    node->outboundSynapses = NULL;

    network->neuronCount++;
}

void NetworkUpdate(NeuralNetwork *network) {
    if (!network) return;

    // 1. Zera as correntes sinápticas acumuladas
    for (size_t  i = 0; i < network->neuronCount; i++) {
        NeuronInterface *ni = &network->neurons[i].interface;
        if (ni->synapticCurrent) *(ni->synapticCurrent) = 0.00f;
    }

    // 2. Atualiza a física das sinapses (buffers circulares de atraso)
    for (size_t i = 0; i < network->neuronCount; i++) {
        SynapseEdge *edge = network->neurons[i].outboundSynapses;
        while (edge != NULL) {
            SynapseUpdate(edge->wrapper);
            edge = edge->nextSynapse;
        }
    }

    // 3. Atualiza a dinâmica intrínseca dos neurônios
    for (size_t i = 0; i < network->neuronCount; i++) {
        NeuronNode *node = &network->neurons[i];
        NeuronInterface *ni = &node->interface;

        ni->setCurrent(ni->modelData, node->externalCurrent);

        double vPast    = node->previousVolt;
        double vPresent = *(ni->membranePotential);
        ni->update(ni->modelData);

        double vFuture  = *(ni->membranePotential);
        node->hasSpiked = IsSpikePoint(vPast, vPresent, vFuture);
        node->previousVolt = vPresent;
    }
}

void NetworkFree(NeuralNetwork *network) {
    if (!network) return;

    for (size_t i = 0; i < network->neuronCount; i++) {
        NeuronNode *node = &network->neurons[i];

        SynapseEdge *edge = node->outboundSynapses;
        while (edge != NULL) {
            SynapseEdge *next = edge->nextSynapse;
            SynapseFree(edge->wrapper);
            free(edge);
            edge = next;
        }

        if (node->interface.free && node->interface.modelData) {
            node->interface.free(node->interface.modelData);
        }
    }

    if (network->neurons) free(network->neurons);
    free(network);
}

bool NetworkConnectSynapse(NeuralNetwork *network, SynapseModels type, size_t sourceId, size_t targetId, double conductancy, double delayMs) {
    if (!network || sourceId >= network->neuronCount || targetId >= network->neuronCount) return false;

    NeuronNode *srcNode = &network->neurons[sourceId];
    NeuronNode *dstNode = &network->neurons[targetId];

    SynapseWrapper *sw = SynapseCreateWrapper(dstNode->type, type, conductancy, delayMs, network->dt);

    if (!sw) return false;

    AmpaGabaaModel *phys = (AmpaGabaaModel*)sw->physics.modelData;

    AmpaGabaaConnectSynapse(
        phys, srcNode->interface.membranePotential,
        dstNode->interface.membranePotential, dstNode->interface.synapticCurrent
    );

    SynapseEdge *edge = (SynapseEdge*)malloc(sizeof(SynapseEdge));
    if (!edge) { SynapseFree(sw); return false; }

    edge->wrapper             = sw;
    edge->edgeType            = type;
    edge->nextSynapse         = srcNode->outboundSynapses;
    edge->targetNeuron        = targetId;
    srcNode->outboundSynapses = edge;

    return true;
}

bool NetworkResizeCapacity(NeuralNetwork *net, size_t newCapacity) {
    if (!net || newCapacity <= net->neuronCount) return false;

    NeuronNode *newNeurons = (NeuronNode*)realloc(net->neurons, newCapacity * sizeof(NeuronNode));
    if (!newNeurons) return false;

    size_t diff = newCapacity - net->capacity;
    memset(&newNeurons[net->capacity], 0, diff * sizeof(NeuronNode));

    net->neurons = newNeurons;
    net->capacity = newCapacity;

    return true;
}

ConnectivityCell *NetworkGetAdjacencyMatrix(NeuralNetwork *net) {
    if (!net || net->neuronCount == 0) return NULL;

    size_t n = net->neuronCount;
    ConnectivityCell *matrix = (ConnectivityCell*)calloc(n * n, sizeof(ConnectivityCell));
    if (!matrix) return NULL;

    for (size_t i = 0; i < n; i++) {
        SynapseEdge *edge = net->neurons[i].outboundSynapses;
        while (edge) {
            size_t targetIndex = edge->targetNeuron;
            if (targetIndex < n) {
                if (edge->wrapper->physics.gMax) {
                    double gMaxVal = *(edge->wrapper->physics.gMax);
                    ConnectivityCell *cell = &matrix[i * n + targetIndex];
                    cell->count++;
                    cell->netWeight += (edge->edgeType == AMPA_MODEL) ? gMaxVal : -gMaxVal;
                }
            }
            edge = edge->nextSynapse;
        }
    }
    return matrix;
}

double NetworkNeuronSynapticCurrentGet(const NeuralNetwork *network, size_t neuronId) {
    if (!network || neuronId >= network->neuronCount) return 0.0;

    const NeuronNode *node = &network->neurons[neuronId];

    if (node->interface.synapticCurrent != NULL) {
        return *(node->interface.synapticCurrent);
    }

    return 0.0;
}

double NetworkNeuronExternalCurrentGet(const NeuralNetwork *network, size_t neuronId) {
    if (!network || neuronId >= network->neuronCount) return 0.0;
    return network->neurons[neuronId].externalCurrent;
}

double NetworkNeuronVoltageGet(const NeuralNetwork *network, size_t neuronId) {
    if (!network || neuronId >= network->neuronCount) return 0.0;

    const NeuronNode *node = &network->neurons[neuronId];

    if (node->interface.membranePotential != NULL) {
        return *(node->interface.membranePotential);
    }

    return node->previousVolt;
}

int NetworkGetTotalSynapse(NeuralNetwork *network) {
    if (!network) return 0;

    int total = 0;
    for (size_t i = 0; i < network->neuronCount; i++) {
        SynapseEdge *edge = network->neurons[i].outboundSynapses;
        while (edge != NULL) {
            total++;
            edge = edge->nextSynapse;
        }
    }
    return total;
}

NetworkType NetworkDetectType(NeuralNetwork *net) {
    if (!net || net->neuronCount == 0) return NETWORK_EMPTY;

    bool hasIz = false, hasHH = false;
    for (size_t i = 0; i < net->neuronCount; i++) {
        switch (net->neurons[i].type) {
            case IZHIKEVICH_MODEL:     hasIz = true; break;
            case HODGKIN_HUXLEY_MODEL: hasHH = true; break;
            default: break;
        }
    }

    if (hasIz && hasHH) return NET_TYPE_HYBRID;
    if (hasIz)          return NET_TYPE_PURE_IZ;
    if (hasHH)          return NET_TYPE_PURE_HH;

    return NETWORK_EMPTY;
}

// --- Implementação das Funções Estáticas (Auxiliares) ---

static inline void NeuronSetupIzhikevich(NeuronInterface *interface, IzhikevichModel *model) {
    interface->modelData         = (void*)model;
    interface->update            = (NeuronUpdateFn)IzhikevichUpdateModel;
    interface->setCurrent        = (NeuronSetCurrentFn)IzhikevichSetExternalCurrent;
    interface->free              = (NeuronFreeFn)IzhikevichFreeModel;
    interface->membranePotential = model->neuron.state.v;
    interface->synapticCurrent   = model->neuron.currents.Isyn;
}

static inline void NeuronSetupHodgkinHuxley(NeuronInterface *interface, HodgkinHuxleyModel *model) {
    interface->modelData         = (void*)model;
    interface->update            = (NeuronUpdateFn)HodgkinHuxleyUpdateModel;
    interface->setCurrent        = (NeuronSetCurrentFn)HodgkinHuxleySetExternalCurent;
    interface->free              = (NeuronFreeFn)HodgkinHuxleyFreeModel;
    interface->membranePotential = model->neuron.state.v;
    interface->synapticCurrent   = model->neuron.currents.iSyn;
}

static inline SynapseWrapper *SynapseCreateWrapper(
    NeuronModel targetType, SynapseModels type, double conductancy, double delayMs, double dt
) {
    SynapseWrapper *sw = (SynapseWrapper*)calloc(1, sizeof(SynapseWrapper));
    if (!sw) return NULL;

    AmpaGabaaModel *phys = AmpaGabaaInitModel((AmpaGabaaSynapseType)type, targetType, dt);
    AmpaGabaaSetMaximumConductancy(phys, conductancy);

    sw->physics.modelData    = phys;
    sw->physics.update       = (SynapsePhysicsUpdateFn)AmpaGabaaUpdateModel;
    sw->physics.free         = (SynapseFreeFn)AmpaGabaaFreeModel;
    sw->physics.gMax         = phys->synapse.receptor.gMax;
    sw->physics.openChannels = phys->synapse.state.openChannels;

    if (delayMs > dt) {
        size_t steps = (size_t)(delayMs / dt);
        sw->bufferSize  = steps + 1;
        sw->delayBuffer = (double*)calloc(sw->bufferSize, sizeof(double));
        sw->writeIndex  = 0;
        sw->readIndex   = 1;
    }

    return sw;
}

static inline void SynapseUpdate(SynapseWrapper *sw) {
    if (!sw) return;

    // 1. Atualiza a cinética da sinapse (calcula r no tempo atual)
    sw->physics.update(sw->physics.modelData);
    AmpaGabaaModel *phys = (AmpaGabaaModel*)sw->physics.modelData;

    if (!phys->synapse.ntParams.iSynPost) return;

    // 2. Aplica a corrente de acordo com a presença ou não de atraso
    if (sw->delayBuffer) {
        double rPresent = phys->stateVector[0];
        sw->delayBuffer[sw->writeIndex] = rPresent;

        double rDelayed = sw->delayBuffer[sw->readIndex];

        double gMax = *(phys->synapse.receptor.gMax);
        double eRev = *(phys->synapse.receptor.eRev);
        double vPost = (phys->synapse.ntParams.vPost) ? *(phys->synapse.ntParams.vPost) : DEFAULT_VOLTAGE;

        double iSynDelayed = gMax * rDelayed * (eRev - vPost);

        // Injeta APENAS a corrente atrasada
        *(phys->synapse.ntParams.iSynPost) += iSynDelayed;

        sw->writeIndex = (sw->writeIndex + 1) % (int)sw->bufferSize;
        sw->readIndex  = (sw->readIndex + 1) % (int)sw->bufferSize;
    } else {
        // Se NÃO há atraso, injeta a corrente instantânea calculada na física
        *(phys->synapse.ntParams.iSynPost) += *(phys->synapse.state.synCurrent);
    }
}

static inline void SynapseFree(SynapseWrapper *sw) {
    if (!sw) return;

    if (sw->physics.free) sw->physics.free(sw->physics.modelData);
    if (sw->delayBuffer) free(sw->delayBuffer);

    free(sw);
}

void NetworkNeuronExternalCurrentSet(NeuralNetwork *network, size_t neuronId, double iExt) {
    if (!network || neuronId >= network->neuronCount) return;

    network->neurons[neuronId].externalCurrent = iExt;
    NeuronInterface *ni = &network->neurons[neuronId].interface;
    if (ni->setCurrent) ni->setCurrent(ni->modelData, iExt);
}
