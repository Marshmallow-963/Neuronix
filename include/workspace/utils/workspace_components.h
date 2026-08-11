#ifndef WORKSPACE_COMPONENTS_H
#define WORKSPACE_COMPONENTS_H

#include <stdbool.h>
#include <stddef.h>

#include "raylib.h"

bool Button(Rectangle rec, int icon, const char *txt, bool isFocused);
bool ValueBox(Rectangle rec, const char *label, double *value, double min, double max, bool *editMode, bool isFocused);

void ComboBox(Rectangle rec, const char *txt, int *active, bool isFocused);
void LabelCentered(Rectangle rec, const char *txt);
void PerformanceVerify(Rectangle layout);

void Selector(Rectangle rec, const char *label, int *value, int min, int max, bool isFocused);
void SliderBar(Rectangle rec, double *value, double min, double max, bool isFocused);
void StatusMsg(Rectangle layout, const char *msg, ...);

void TooltipMsg(Rectangle rec, const char *msg);

#endif // WORKSPACE_COMPONENTS_H
