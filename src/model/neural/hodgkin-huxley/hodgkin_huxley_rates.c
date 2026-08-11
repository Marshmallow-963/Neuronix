/**
 * @file hodgkin_huxley_rates.c
 * @brief Implementation of the rate functions (Alpha and Beta) for the
 * HH model's ionic gates.
 */

#include <math.h>

#include "model/neural/hodgkin-huxley/hodgkin_huxley_rates.h"

double AlphaM(double v) {
    if (v == 25.0) return 1.0;

    return (25.0 - v) / (10.0 * (exp((25.0 - v) / 10.0) - 1.0));
}

double BetaM(double v) {
    return 4.0 * exp(-v / 18.0);
}

double AlphaH(double v) {
    return 0.07 * exp(-v / 20.0);
}

double BetaH(double v) {
    return 1.0 / (exp((30.0 - v)/ 10.0) + 1.0);
}

double AlphaN(double v) {
    if (v == 10.0) return 0.1;

    return (10.0 - v) / (100.0 * (exp((10.0 - v) / 10.0) - 1.0));
}

double BetaN(double v) {
    return 0.125 * exp(-v / 80.0);
}
