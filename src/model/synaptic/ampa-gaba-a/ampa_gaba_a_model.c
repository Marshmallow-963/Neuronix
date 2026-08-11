/**
 * @file ampa_gaba_a_model.c
 * @brief Implementation of the AMPA/GABA-A synapse simulation logic.
 *
 * This model calculates:
 * 1. Neurotransmitter (NT) concentration [T] based on pre-synaptic voltage.
 * 2. The fraction of open channels [r] using an ODE (solved by RK4).
 * 3. The post-synaptic current (I_syn) based on [r] and post-synaptic voltage.
 */

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include "model/synaptic/ampa-gaba-a/ampa_gaba_a_type.h"
#include "model/synaptic/ampa-gaba-a/ampa_gaba_a_model.h"
#include "model/synaptic/ampa-gaba-a/ampa_gaba_a_config.h"

// --- Internal Module Constants ---

/** @brief Dimension of the system of ODEs (only 'r') */
#define SYS_DIM 1
/** @brief Size of the internal buffer (all params and internal states) */
#define INTERNAL_VARS 11

// --- static inline Forward Declarations ---

/**
 * @brief Allocates the internal buffer and maps all synapse pointers.
 *
 * @param model Pointer to the AmpaGabaaModel.
 * @return true on success, false if memory allocation fails.
 */
static inline bool SynapseAllocMemory(AmpaGabaaModel *model);

/**
 * @brief The derivatives function (dy/dt) for the RK4 integrator.
 *
 * Calculates the derivative for 'r' (the fraction of open channels).
 * dr/dt = alpha * [T] * (1 - r) - beta * r
 *
 * @param state The current state vector [r].
 * @param deriv The output vector where the derivative [r'] will be written.
 * @param params A (void*) pointer to the AmpaGabaaSynapse struct.
 */
static inline void AmpaGabaaDerivatives(const double *state, double *deriv, void *params);

// --- Public (API) Function Implementations ---

AmpaGabaaModel *AmpaGabaaInitModel(AmpaGabaaSynapseType synType, NeuronModel nrnType, double dt) {
    AmpaGabaaModel *model = (AmpaGabaaModel*)calloc(1, sizeof(AmpaGabaaModel));
    if (!model) return NULL;

    model->stateVector = (double*)calloc(SYS_DIM, sizeof(double));
    if (!model->stateVector) { free(model); return NULL; }

    model->synapse.state.openChannels = &model->stateVector[0];
    model->stateVector[0] = 0.0f;

    if (!SynapseAllocMemory(model)) { AmpaGabaaFreeModel(model); return NULL; }

    *(model->synapse.ntParams.tMax) = T_MAX;

    switch (nrnType) {
        case IZHIKEVICH_MODEL: {
            *(model->synapse.ntParams.kP) = IZ_SYN_CFG.KP;
            *(model->synapse.ntParams.vP) = IZ_SYN_CFG.VP;

            switch (synType) {
                case AMPA: {
                    *(model->synapse.receptor.alphaRate) = IZ_SYN_CFG.ampaConnectionRate;
                    *(model->synapse.receptor.betaRate)  = IZ_SYN_CFG.ampaDisconnectionRate;
                    *(model->synapse.receptor.eRev)      = IZ_SYN_CFG.ampaReversalPotential;
                    *(model->synapse.receptor.gMax)      = IZ_SYN_CFG.ampaMaximumConductancy;
                } break;

                case GABA_A: {
                    *(model->synapse.receptor.alphaRate) = IZ_SYN_CFG.gaba_aConnectionRate;
                    *(model->synapse.receptor.betaRate)  = IZ_SYN_CFG.gaba_aDisconnectionRate;
                    *(model->synapse.receptor.eRev)      = IZ_SYN_CFG.gaba_aReversalPotential;
                    *(model->synapse.receptor.gMax)      = IZ_SYN_CFG.gaba_aMaximumConductancy;
                } break;

                default: break;
            }
        } break;

        case HODGKIN_HUXLEY_MODEL: {
            *(model->synapse.ntParams.kP) = HH_SYN_CFG.KP;
            *(model->synapse.ntParams.vP) = HH_SYN_CFG.VP;

            switch (synType) {
                case AMPA: {
                    *(model->synapse.receptor.alphaRate) = HH_SYN_CFG.ampaConnectionRate;
                    *(model->synapse.receptor.betaRate)  = HH_SYN_CFG.ampaDisconnectionRate;
                    *(model->synapse.receptor.eRev)      = HH_SYN_CFG.ampaReversalPotential;
                    *(model->synapse.receptor.gMax)      = HH_SYN_CFG.ampaMaximumConductancy;
                } break;

                case GABA_A: {
                    *(model->synapse.receptor.alphaRate) = HH_SYN_CFG.gaba_aConnectionRate;
                    *(model->synapse.receptor.betaRate)  = HH_SYN_CFG.gaba_aDisconnectionRate;
                    *(model->synapse.receptor.eRev)      = HH_SYN_CFG.gaba_aReversalPotential;
                    *(model->synapse.receptor.gMax)      = HH_SYN_CFG.gaba_aMaximumConductancy;
                } break;

                default: break;
            }
        } break;

        default: break;
    }

    *(model->synapse.state.ntConcentration) = SYN_DEFAULT_STATE;
    *(model->synapse.state.synCurrent)      = SYN_DEFAULT_STATE;

    if (!RK4Init(
        &model->integrator,
        AmpaGabaaDerivatives,
        &model->synapse,
        SYS_DIM, dt)
    )
    {
        AmpaGabaaFreeModel(model);
        return NULL;
    }

    return model;
}

void AmpaGabaaUpdateModel(AmpaGabaaModel *model) {
    if (!model) return;

    RK4Calculate(&model->integrator, model->stateVector);

    double r = model->stateVector[0];

    const double gMax = *(model->synapse.receptor.gMax);
    const double eRev = *(model->synapse.receptor.eRev);

    const double vPost = (model->synapse.ntParams.vPost != NULL) ? *(model->synapse.ntParams.vPost) : DEFAULT_VOLTAGE;

    double iSyn = gMax * r * (eRev - vPost);
    *(model->synapse.state.synCurrent) = iSyn;

    /*
    if (model->synapse.ntParams.iSynPost) {
        *(model->synapse.ntParams.iSynPost) += iSyn;
    }
    */
}

void AmpaGabaaConnectSynapse(AmpaGabaaModel *model, double *preVolt, double *postVolt, double *postIsyn) {
    if (!model || !preVolt || !postVolt || !postIsyn) return;

    model->synapse.ntParams.vPre     = preVolt;
    model->synapse.ntParams.vPost    = postVolt;
    model->synapse.ntParams.iSynPost = postIsyn;
}

void AmpaGabaaSetMaximumConductancy(AmpaGabaaModel *model, double g) {
    if (!model) return;

    if (g <= SYN_DEFAULT_STATE) return;
    *(model->synapse.receptor.gMax) = g;
}

void AmpaGabaaFreeModel(AmpaGabaaModel *model) {
    if (!model) return;

    if (model->integrator.buffer) RK4Free(&model->integrator);
    if (model->internalBuffer) free(model->internalBuffer);
    if (model->stateVector) free(model->stateVector);

    free(model);
}

// --- Private (static) Function Implementations ---

static inline bool SynapseAllocMemory(AmpaGabaaModel *model) {
    model->internalBuffer = (double*)calloc(INTERNAL_VARS, sizeof(double));
    if (!model->internalBuffer) return false;

    // Map all pointers (state, receptor, ntParams) to the buffer
    double *p = model->internalBuffer;
    model->synapse.state.synCurrent      = p++;

    model->synapse.state.ntConcentration = p++;
    model->synapse.receptor.alphaRate    = p++;
    model->synapse.receptor.betaRate     = p++;
    model->synapse.receptor.eRev         = p++;
    model->synapse.receptor.gMax         = p++;
    model->synapse.ntParams.vP           = p++;
    model->synapse.ntParams.kP           = p++;
    model->synapse.ntParams.tMax         = p++;
    model->synapse.ntParams.vPre         = p++; // Pointer to external V_pre
    model->synapse.ntParams.vPost        = p++; // Pointer to external V_post
    // Note: iSynPost is handled by AmpaGabaaConnectSynapse, not in this buffer.

    return true;
}

static inline void AmpaGabaaDerivatives(const double *state, double *deriv, void *params) {
    AmpaGabaaSynapse *synapse = (AmpaGabaaSynapse*)params;

    const double r = state[0];
    const double vPre = (synapse->ntParams.vPre != NULL) ? *(synapse->ntParams.vPre) : DEFAULT_VOLTAGE;

    const double kP = *(synapse->ntParams.kP);
    const double vP = *(synapse->ntParams.vP);

    const double tMax = *(synapse->ntParams.tMax);

    double diff = (vPre - vP) / kP;

    double t = 0.0f;
    if (diff > SYN_SIGMOID_THRESHOLD) t = tMax / (1.0f + exp(-diff));
    else if (t < SYN_NT_EPSILON) t = SYN_DEFAULT_STATE;

    *(synapse->state.ntConcentration) = t;

    const double alpha = *(synapse->receptor.alphaRate);
    const double beta  = *(synapse->receptor.betaRate);

    deriv[0] = alpha * t * (1.0f - r) - (beta * r);
}
