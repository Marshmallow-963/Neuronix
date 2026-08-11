#ifndef NEURO_DATA_STATE_H
#define NEURO_DATA_STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "neuro_data_enums.h"
#include "neuro_data_config.h"

#include "utils/struct_data.h"

typedef struct {
    double hh_ik;     // Potassium current of hodgkin-huxley model
    double hh_ina;    // Sodium current of hodgkin-huxley model
    double hh_ileak;  // Leak current of hodgkin-huxley model

    double hh_m;
    double hh_n;
    double hh_h;

    double v;
    double syn;
    double ntFrac;
    double phaseY;

    TraceModelType modelType;
} NeuroTraceBackFillData;

/**
 * @struct HodgkinHuxleyGatePlots
 * @brief Stores plot data for HH model gating variables (m, h, n).
 */
typedef struct {
    Vector2d *MGate[MAX_DETAILED_PLOTS];
    Vector2d *HGate[MAX_DETAILED_PLOTS];
    Vector2d *NGate[MAX_DETAILED_PLOTS];
} HodgkinHuxleyIonicGatePlots;

/**
 * @struct HodgkinHuxleyCurrentPlots
 * @brief Stores plot data for HH model ionic currents (IK, INa, ILeak).
 */
typedef struct {
    Vector2d *kCurrent[MAX_DETAILED_PLOTS];
    Vector2d *naCurrent[MAX_DETAILED_PLOTS];
    Vector2d *leakCurrent[MAX_DETAILED_PLOTS];
} HodgkinHuxleyCurrentPlots;

typedef struct {
    bool isCircular;

    HodgkinHuxleyCurrentPlots   hhCurrentPlots;
    HodgkinHuxleyIonicGatePlots hhGatePlots;

    size_t bufferCapacity; ///< Current size of allocated arrays
    size_t dataCount;      ///< Current number of data points collected.

    size_t rasterPointsCount[MAX_DETAILED_PLOTS];
    size_t tauPointsCount[MAX_DETAILED_PLOTS * MAX_DETAILED_PLOTS];

    Vector2d *izRecoveryTraces[MAX_DETAILED_PLOTS];

    Vector2d *neuronTraces[MAX_DETAILED_PLOTS];
    Vector2d *ntFractionTraces[MAX_DETAILED_PLOTS];
    Vector2d *phaseSpaceTraces[MAX_DETAILED_PLOTS];
    Vector2d *rasterTraces[MAX_DETAILED_PLOTS];
    Vector2d *synapticTraces[MAX_DETAILED_PLOTS];
    Vector2d *tauTraces[MAX_DETAILED_PLOTS * MAX_DETAILED_PLOTS];
} NeuroTraceBuffer;

#endif // NEURO_DATA_STATE_H
