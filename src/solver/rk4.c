/**
 * @file rk4.c
 * @brief Implementation of the 4th Order Runge-Kutta integration logic.
 */


#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "solver/rk4.h"

/**
 * @details Allocates a contiguous buffer to ensure cache locality for slope updates.
 * The buffer is partitioned into 5 sections: k1, k2, k3, k4, and tempState.
 */
bool RK4Init(RK4 *integrator, DerivativeFn func, void *params, size_t n, double dt) {
    integrator->params    = params;
    integrator->derivFunc = func;
    integrator->dt        = dt;
    integrator->n         = n;

    integrator->buffer = (double*)calloc(5 * n, sizeof(double));
    if (!integrator->buffer) return false;;

    integrator->slopes.k1        = integrator->buffer;
    integrator->slopes.k2        = integrator->buffer + n;
    integrator->slopes.k3        = integrator->buffer + 2 * n;
    integrator->slopes.k4        = integrator->buffer + 3 * n;
    integrator->slopes.tempState = integrator->buffer + 4 * n;

    return true;
}

/**
 * @details Executes the four stages of RK4:
 * 1. Calculate k1 at the start of the interval.
 * 2. Calculate k2 at the midpoint using k1.
 * 3. Calculate k3 at the midpoint using k2.
 * 4. Calculate k4 at the end using k3.
 * Finally, updates the state using a weighted average of the slopes.
 */
void RK4Calculate(RK4 *integrator, double *state) {
    const size_t n = integrator->n;

    const double dt      = integrator->dt;
    const double dtHalf  = dt * 0.5f;
    const double dtSixth = dt / 6.0f;

    void *params = integrator->params;
    DerivativeFn f = integrator->derivFunc;

    double *k1 = integrator->slopes.k1;
    double *k2 = integrator->slopes.k2;
    double *k3 = integrator->slopes.k3;
    double *k4 = integrator->slopes.k4;

    double *tempState = integrator->slopes.tempState;

    // k1 = f(y_n)
    f(state, k1, params);

    // k2 = f(y_n + 0.5 * dt * k1)
    for (size_t i = 0; i < n; i++) tempState[i] = state[i] + dtHalf * k1[i];
    f(tempState, k2, params);

    // k3 = f(y_n + 0.5 * dt * k2)
    for (size_t i = 0; i < n; i++) tempState[i] = state[i] + dtHalf * k2[i];
    f(tempState, k3, params);

    // k4 = f(y_n + dt * k3)
    for (size_t i = 0; i < n; i++) tempState[i] = state[i] + dt * k3[i];
    f(tempState, k4, params);

    // Update state: y_{n+1} = y_n + (dt/6) * (k1 + 2k2 + 2k3 + k4)
    for (size_t i = 0; i < n; i++) {
        state[i] += (k1[i] + 2.0f * (k2[i] + k3[i]) + k4[i]) * dtSixth;
    }
}

/**
 * @details Cleans up the buffer and resets all pointers to NULL to prevent dangling pointers.
 */
void RK4Free(RK4 *integrator) {
    if (integrator && integrator->buffer) {
        free(integrator->buffer);

        integrator->buffer = NULL;

        integrator->slopes.k1 = NULL;
        integrator->slopes.k2 = NULL;
        integrator->slopes.k3 = NULL;
        integrator->slopes.k4 = NULL;

        integrator->slopes.tempState = NULL;
    }
}
