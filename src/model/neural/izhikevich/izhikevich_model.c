/**
 * @file izhikevich_model.c
 * @brief Implementation of the Izhikevich neuron simulation logic.
 *
 * This model uses a hybrid simulation approach:
 * 1. RK4 integration for sub-threshold dynamics.
 * 2. A discrete reset mechanism when voltage hits the spike peak.
 */
#include <stdlib.h>
#include <stdbool.h>

#include "solver/rk4.h"

#include "model/neural/izhikevich/izhikevich_config.h"
#include "model/neural/izhikevich/izhikevich_model.h"

// --- Internal Module Constants ---

/** @brief Dimension of the system of ODEs (v, u) */
#define SYS_DIM 2
/** @brief Size of the internal buffer (a, b, c, d, Iext, Isyn) */
#define NUM_CURRENTS_AND_PARAMS 6

// Coefficients for the 'v' derivative: (0.04*v^2 + 5*v + 140)
/** @brief Quadratic coefficient for the v-derivative */
#define QUAD_COEFF   0.04f
/** @brief Linear coefficient for the v-derivative */
#define LINEAR_COEFF 5.0f
/** @brief Constant term for the v-derivative */
#define CONST_TERM   140.0f

// --- Static Forward Declarations ---

/**
 * @brief Allocates the internal buffer and maps all neuron pointers.
 *
 * The internal buffer holds parameters (a,b,c,d) and currents (Iext, Isyn).
 *
 * @param model Pointer to the Izhikevich model.
 * @return true on success, false if memory allocation fails.
 */
static bool AllocateMemory(IzhikevichModel *model);

/**
 * @brief The derivatives function (dy/dt) for the RK4 integrator.
 *
 * Calculates the derivatives for v (dv/dt) and u (du/dt).
 *
 * @param state The current state vector [v, u].
 * @param deriv The output vector where derivatives [v', u'] will be written.
 * @param params A (void*) pointer to the IzhikevichNeuron struct.
 */
static void IzhikevichDerivatives(const double *state, double *deriv, void *params);

// --- Public (API) Function Implementations ---

IzhikevichModel* IzhikevichInitModel(const IzNeuronType type, const double dt) {
    // 1. Allocate main struct
    IzhikevichModel *model = (IzhikevichModel*)calloc(1, sizeof(IzhikevichModel));
    if (!model) return NULL;

    model->stateVector    = NULL;
    model->internalBuffer = NULL;

    // 2. Allocate state vector (v, u)
    model->stateVector = (double*)calloc(SYS_DIM, sizeof(double));

    if (!model->stateVector) {
        IzhikevichFreeModel(model);
        return NULL;
    }

    // Map state pointers
    model->neuron.state.v = &model->stateVector[0];
    model->neuron.state.u = &model->stateVector[1];

    // 3. Allocate internal buffer (for params and currents)
    if (!AllocateMemory(model)) {
        IzhikevichFreeModel(model);
        return NULL;
    }

    // 4. Set parameters and initial currents from the global config
    model->neuron.config = IZHIKEVICH_PARAMETERS[type];

    *(model->neuron.currents.Iext) = 0.0f;
    *(model->neuron.currents.Isyn) = 0.0f;

    // 5. Set initial state (resting)
    // Start 'v' near its reset potential 'c'
    *(model->neuron.state.v) = model->neuron.config.c;
    // Initialize 'u' to be in equilibrium (u = b*v)
    *(model->neuron.state.u) = model->neuron.config.b * (*(model->neuron.state.v));

    // 6. Initialize RK4 integrator
    if (!RK4Init(&model->integrator, IzhikevichDerivatives, &model->neuron, SYS_DIM, dt)) {
        IzhikevichFreeModel(model);
        return NULL;
    }

    return model;
}

void IzhikevichSetExternalCurrent(IzhikevichModel *model, double iExt) {
    if (!model) return;

    *(model->neuron.currents.Iext) = iExt;
}

double IzhikevichUpdateModel(IzhikevichModel *model) {
    if (!model) return 0.00f;

    RK4Calculate(&model->integrator, model->stateVector);

    if (*(model->neuron.state.v) >= IZHIKEVICH_SPIKE_PEAK) {
        *(model->neuron.state.v) = model->neuron.config.c;
        *(model->neuron.state.u) += model->neuron.config.d;
        return IZHIKEVICH_SPIKE_PEAK;
    }

    return *(model->neuron.state.v);
}

double IzhikevichGetRecovery(IzhikevichModel *model) {
    if (!model) return 0.0f;
    return *(model->neuron.state.u);
}

void IzhikevichFreeModel(IzhikevichModel *model) {
    if (!model) return;

    // Free RK4 buffer
    if (model->integrator.buffer) RK4Free(&model->integrator);

    // Free main model buffers
    free(model->internalBuffer);
    free(model->stateVector);

    // Free the struct itself
    free(model);
}

// --- Private (static) Function Implementations ---

static bool AllocateMemory(IzhikevichModel *model) {
    model->internalBuffer = (double*)calloc(NUM_CURRENTS_AND_PARAMS, sizeof(double));

    if (!model->internalBuffer) return false;

    // Map pointers to this buffer
    double *p = model->internalBuffer;
    model->neuron.currents.Iext = p++;
    model->neuron.currents.Isyn = p;

    return true;
}

static void IzhikevichDerivatives(const double *state, double *deriv, void *params) {
    IzhikevichNeuron *neuron = (IzhikevichNeuron*)params;

    // Unpack state
    const double v = state[0];
    const double u = state[1];

    // Unpack params
    const double a = neuron->config.a;
    const double b = neuron->config.b;
    const double I = *(neuron->currents.Iext) + *(neuron->currents.Isyn);

    // Izhikevich's ODEs
    // dv/dt
    deriv[0] = (QUAD_COEFF * v * v) + (LINEAR_COEFF * v) + CONST_TERM - u + I;
    // du/dt
    deriv[1] = a * (b * v - u);
}
