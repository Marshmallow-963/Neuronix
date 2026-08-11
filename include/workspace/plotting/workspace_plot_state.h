#ifndef WORKSPACE_PLOT_STATE_H
#define WORKSPACE_PLOT_STATE_H

#include <stddef.h>
#include <stdbool.h>

#include "workspace/core/workspace_enums.h"

typedef struct {
    TableId tableId;
    const char *yLabel;
    size_t countOffset;
    size_t dataOffset;
} PlotMapEntry;

extern const PlotMapEntry plotMap[];
extern const size_t plotMapSize;

#endif // WORKSPACE_PLOT_STATE_H
