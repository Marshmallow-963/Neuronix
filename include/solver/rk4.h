/**
 * @file rk4.h
 * @brief 4th Order Runge-Kutta (RK4) numerical integrator.
 * * This module provides a generic implementation of the RK4 method for solving
 * systems of Ordinary Differential Equations (ODEs). It uses a pre-allocated
 * buffer to minimize runtime memory allocation overhead.
 */

#ifndef RK4_H
#define RK4_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Function pointer for system derivatives.
 * * User-defined function that calculates the derivatives of the system.
 * * @param state  Current state vector (input).
 * @param deriv  Calculated derivative vector (output).
 * @param params User-defined parameters for the model (e.g., physics constants).
 */
typedef void (*DerivativeFn)(const double *state, double *deriv, void *params);

/**
 * @struct Slopes
 * @brief Internal storage for RK4 intermediate slope calculations.
 * * Holds pointers to the four stages (k1, k2, k3, k4) and a temporary state buffer.
 * These pointers point to sub-sections of the main RK4 buffer.
 */
typedef struct {
    double *k1;        ///< Stage 1: Initial slope.
    double *k2;        ///< Stage 2: Midpoint slope using k1.
    double *k3;        ///< Stage 3: Midpoint slope using k2.
    double *k4;        ///< Stage 4: End-point slope using k3.
    double *tempState; ///< Buffer for intermediate state transitions.
} Slopes;

/**
 * @struct RK4
 * @brief Master structure for the RK4 integrator.
 * * Manages the dimensions of the system, integration step, and memory buffers.
 */
typedef struct {
    size_t n;                   ///< Dimension of the state vector (number of variables).
    double dt;                ///< Time step size.
    void *params;            ///< User parameters passed to the derivative function.
    double *buffer;           ///< Contiguous memory block holding all internal arrays.
    Slopes slopes;           ///< Partitioned pointers into the buffer.
    DerivativeFn derivFunc;  ///< The derivative function to be integrated.
} RK4;

/**
 * @brief Initializes the RK4 integrator.
 * * Allocates a single memory block of size (5 * n) to hold intermediate slopes.
 * * @param integrator Pointer to the RK4 structure.
 * @param func       The derivative function (f in dy/dt = f(t, y)).
 * @param params     Opaque pointer to model parameters.
 * @param n          Number of elements in the state vector.
 * @param dt         Integration time step.
 * @return true      If memory allocation succeeded.
 * @return false     If memory allocation failed.
 */
bool RK4Init(
    RK4 *integrator,
    DerivativeFn func,
    void *params,
    size_t n,
    double dt
);

/**
 * @brief Performs one integration step using the 4th Order Runge-Kutta method.
 * * Updates the provided state vector in-place.
 * * @param integrator Pointer to the initialized RK4 structure.
 * @param state      State vector to be updated (must be of size n).
 * * @note Mathematically, the update follows:
 * y_{n+1} = y_n + (dt/6) * (k1 + 2k2 + 2k3 + k4)
 */
void RK4Calculate(RK4 *integrator, double *state);

/**
 * @brief Frees all dynamically allocated memory used by the integrator.
 * * @param integrator Pointer to the RK4 structure.
 */
void RK4Free(RK4 *integrator);

#endif // RK4_H
