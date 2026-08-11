/**
 * @file hodgkin_huxley_model.c
 * @brief Implementation of the Hodgkin-Huxley simulation logic.
 *
 * Manages memory allocation, the update loop (via RK4), and
 * the calculation of the model's differential equations.
 */
#include <stdlib.h>
#include <stdbool.h>

#include "model/neural/hodgkin-huxley/hodgkin_huxley_model.h"
#include "model/neural/hodgkin-huxley/hodgkin_huxley_rates.h"
#include "model/neural/hodgkin-huxley/hodgkin_huxley_config.h"

// --- Internal Module Constants ---

/** @brief Dimension of the system of ODEs (Ordinary Differential Equations).
 * (V, m, h, n) -> 4 variables
 */
#define SYS_DIM 4

/** @brief Number of currents stored in the internal buffer.
 * (iNa, iK, iL, iSyn, iExt) -> 5 currents
 */
#define NUM_CURRENTS 5

// --- static inline Forward Declarations ---
// (Forward declarations for private functions in this file)

/**
 * @brief Allocates the internal current buffer and maps pointers.
 * @param model The HH model.
 * @return true if allocation is successful, false otherwise.
 */
static inline bool AllocCurrents(HodgkinHuxleyModel *model);

/**
 * @brief The derivatives function (dy/dt) for the RK4 integrator.
 *
 * Calculates the derivatives of V, m, h, and n, which are the HH model's ODEs.
 * dV/dt, dV/dt, dV/dt, dV/dt
 *
 * @param state The current state vector [V, m, h, n].
 * @param deriv The output vector where the derivatives [V', m', h', n']
 * will be written.
 * @param params A (void*) pointer to the HodgkinHuxleyNeuron struct,
 * used to access parameters and currents.
 */
static inline void HodgkinHuxleyDerivatives(const double *state, double *deriv, void *params);


// --- Public (API) Function Implementations ---

/**
 * @brief Implementation of the model initialization.
 */
HodgkinHuxleyModel* HodgkinHuxleyInitModel(const double dt) {
    HodgkinHuxleyModel *model = (HodgkinHuxleyModel*)calloc(1, sizeof(HodgkinHuxleyModel));
    if (!model) return NULL;

    model->stateVector    = NULL;
    model->internalBuffer = NULL;

    model->stateVector = (double*)calloc(SYS_DIM, sizeof(double));
    if (!model->stateVector) { HodgkinHuxleyFreeModel(model); return NULL; }

    model->neuron.state.v = &model->stateVector[0];
    model->neuron.state.m = &model->stateVector[1];
    model->neuron.state.h = &model->stateVector[2];
    model->neuron.state.n = &model->stateVector[3];

    model->neuron.params.C   = HH_SQUID_AXON.membraneCapacitancy;
    model->neuron.params.eL  = HH_SQUID_AXON.leakReversal;
    model->neuron.params.eK  = HH_SQUID_AXON.potassiumReversal;
    model->neuron.params.eNa = HH_SQUID_AXON.sodiumReversal;
    model->neuron.params.gL  = HH_SQUID_AXON.leakConductance;
    model->neuron.params.gK  = HH_SQUID_AXON.potassiumConductance;
    model->neuron.params.gNa = HH_SQUID_AXON.sodiumConductance;

    *(model->neuron.state.v) = HH_SQUID_AXON.restingPotential;
    *(model->neuron.state.m) = GATE_REST_M;
    *(model->neuron.state.h) = GATE_REST_H;
    *(model->neuron.state.n) = GATE_REST_N;

    if (AllocCurrents(model) != true) {
        HodgkinHuxleyFreeModel(model);
        return NULL;
    }

    *(model->neuron.currents.iExt) = 0.0f;
    *(model->neuron.currents.iSyn) = 0.0f;

    if (!RK4Init(
        &model->integrator,
        HodgkinHuxleyDerivatives,
        &model->neuron,
        SYS_DIM, dt)
    )
    {
        HodgkinHuxleyFreeModel(model);
        return NULL;
    }

    return model;
}

/**
 * @brief Implementation of the external current setter.
 */
void HodgkinHuxleySetExternalCurent(HodgkinHuxleyModel *model, double iExt) {
    if (!model) return;

    *(model->neuron.currents.iExt) = iExt;
}

double HodgkinHuxleyUpdateModel(HodgkinHuxleyModel *model) {
    if (!model) return 0.0f;

    RK4Calculate(&model->integrator, model->stateVector);

    return *(model->neuron.state.v);
}

void HodgkinHuxleyFreeModel(HodgkinHuxleyModel *model) {
    if (!model) return;

    if (model->integrator.buffer) { RK4Free(&model->integrator); }

    free(model->internalBuffer);
    free(model->stateVector);

    free(model);
}

// --- Private (static) Function Implementations ---

static inline bool AllocCurrents(HodgkinHuxleyModel *model) {
    model->internalBuffer = (double*)calloc(NUM_CURRENTS, sizeof(double));
    if (!model->internalBuffer) return false;

    double *p = model->internalBuffer;
    model->neuron.currents.iNa  = p++;
    model->neuron.currents.iK   = p++;
    model->neuron.currents.iL   = p++;
    model->neuron.currents.iSyn = p++;
    model->neuron.currents.iExt = p;

    return true;
}

static inline void HodgkinHuxleyDerivatives(const double *state, double *deriv, void *params) {
    HodgkinHuxleyNeuron *neuron = (HodgkinHuxleyNeuron*)params;

    const double v = state[0];
    const double m = state[1];
    const double h = state[2];
    const double n = state[3];

    const double iL  = neuron->params.gL * (neuron->params.eL - v);
    const double iK  = neuron->params.gK * n * n * n * n * (neuron->params.eK - v);
    const double iNa = neuron->params.gNa * m * m * m * h * (neuron->params.eNa - v);

    const double I = *(neuron->currents.iExt) + *(neuron->currents.iSyn);

    *(neuron->currents.iL)  = iL;
    *(neuron->currents.iK)  = iK;
    *(neuron->currents.iNa) = iNa;

    deriv[0] = ((iNa + iK + iL) + I) / neuron->params.C;
    deriv[1] = AlphaM(v) * (1.0f - m) - BetaM(v) * m;
    deriv[2] = AlphaH(v) * (1.0f - h) - BetaH(v) * h;
    deriv[3] = AlphaN(v) * (1.0f - n) - BetaN(v) * n;
}
