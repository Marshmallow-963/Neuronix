#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/memory_buffer.h"

#include "raygui.h"
#include "raylib.h"

#include "workspace/config/workspace_ui_config.h"
#include "workspace/utils/workspace_components.h"

// =========================================================================
// WIDGET IMPLEMENTATIONS (Alphabetical Order)
// =========================================================================

bool Button(Rectangle rec, int icon, const char *txt, bool isFocused) {
    bool clicked = GuiButton(rec, GuiIconText(icon, txt));

    if (isFocused) DrawRectangleLinesEx(rec, 2.0f, SKYBLUE);

    GuiSetState(STATE_NORMAL);

    return clicked;
}

bool ValueBox(Rectangle rec, const char *label, double *value, double min, double max, bool *editMode, bool isFocused) {
    bool valueChanged = false;
    float labelWidth = 120.0f;
    Rectangle boxRec = { rec.x + labelWidth, rec.y, rec.width - labelWidth, rec.height };

    static char editBuffer[32] = { 0 };

    GuiLabel((Rectangle){ rec.x, rec.y, labelWidth, rec.height }, label);

    if (!(*editMode)) {
        char displayBuffer[32];
        sprintf(displayBuffer, "%.2f", *value);

        if (GuiTextBox(boxRec, displayBuffer, 32, false)) {
            *editMode = true;
            strcpy(editBuffer, displayBuffer);
        }
    }

    else {
        if (GuiTextBox(boxRec, editBuffer, 32, true)) {
            *editMode = false;

            double newValue = strtod(editBuffer, NULL);

            if (newValue < min) newValue = min;
            if (newValue > max) newValue = max;

            *value = newValue;
            valueChanged = true;
        }
    }

    if (isFocused) DrawRectangleLinesEx(boxRec, 2.0f, SKYBLUE);

    return valueChanged;
}

void ComboBox(Rectangle rec, const char *txt, int *active, bool isFocused) {
    GuiComboBox(rec, txt, active);
    if (isFocused) DrawRectangleLinesEx(rec, 2.0f, SKYBLUE);
}

void LabelCentered(Rectangle rec, const char *txt) {
    int prevAlign = GuiGetStyle(LABEL, TEXT_ALIGNMENT);

    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiLabel(rec, txt);
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, prevAlign);
}

void PerformanceVerify(Rectangle layout) {
    int fps = GetFPS();
    float frameTime = GetFrameTime() * 1000; // Convertendo para ms

    // Captura a RAM e converte de Bytes para Megabytes
    double ramMB = (double)MemoryUsageGet() / (1024.0 * 1024.0);

    const char *fpsTxt = TextFormat("FPS: %i", fps);
    const char *ramTxt = TextFormat("RAM: %.2f MB", ramMB);
    const char *ftTxt  = TextFormat("Frametime: %.2f ms", frameTime);

    int ftWidth = MeasureText(ftTxt, 10);
    int ramWidth = MeasureText(ramTxt, 10);
    int blockWidth = (ftWidth > ramWidth) ? ftWidth : ramWidth;

    int posX = (int)(layout.x + layout.width - (float)blockWidth - PADDING * 2);
    int posY = (int)(layout.y + PADDING * 2);

    int lineSpacing = 12; // Espaço vertical entre as linhas

    DrawText(fpsTxt, posX, posY, 10, SKYBLUE);
    DrawText(ramTxt, posX, posY + lineSpacing, 10, SKYBLUE);
    DrawText(ftTxt,  posX, posY + (lineSpacing * 2), 10, SKYBLUE);
}

void Selector(Rectangle rec, const char *label, int *value, int min, int max, bool isFocused) {
    float padding = 5.0f;

    float h = rec.height;
    float w = rec.width - (h * 2) - (padding * 2);

    Rectangle btnMinus = { rec.x, rec.y, h, h };
    DrawRectangleRec(btnMinus, DARKGRAY);

    int txtWidthMinus = MeasureText("-", 20);
    DrawText(
        "-",
        (int)(btnMinus.x + (h - (float)txtWidthMinus) / 2),
        (int)(btnMinus.y + (h - 20) / 2), 20, WHITE
    );

    if (CheckCollisionPointRec(GetMousePosition(), btnMinus) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *value = (*value - 1);
        if (*value < min) *value = max;
    }

    Rectangle centerArea = { rec.x + h + padding, rec.y, w, h };
    DrawRectangleRec(centerArea, Fade(DARKGRAY, 0.3f));

    char valText[32];
    sprintf(valText, "%s: %d", label, *value);

    int txtWidthValue = MeasureText(valText, 10);
    DrawText(
        valText,
        (int)(centerArea.x + (centerArea.width - (float)txtWidthValue)/2),
        (int)(centerArea.y + (h - 10)/2), 10, WHITE
    );

    Rectangle btnPlus = { rec.x + rec.width - h, rec.y, h, h };
    DrawRectangleRec(btnPlus, DARKGRAY);

    int txtWidthPlus = MeasureText("+", 20);
    DrawText(
        "+",
        (int)(btnPlus.x + (h - (float)txtWidthPlus)/2),
        (int)(btnPlus.y + (h - 20)/2), 20, WHITE
    );

    if (CheckCollisionPointRec(GetMousePosition(), btnPlus) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *value = *value + 1;
        if (*value > max) *value = min;
    };

    if (isFocused) DrawRectangleLinesEx(rec, 2.0f, SKYBLUE);
}

void SliderBar(Rectangle rec, double *value, double minValue, double maxValue, bool isFocused) {
    if (!value) return;

    float tempValue = (float)(*value);

    GuiSliderBar(rec, NULL, NULL, &tempValue, (float)minValue, (float)maxValue);

    *value = (double)tempValue;

    if (isFocused) DrawRectangleLinesEx(rec, 2.0f, SKYBLUE);
}

void StatusMsg(Rectangle layout, const char *msg, ...) {
    char buffer[256];

    va_list args;
    va_start(args, msg);
    vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);

    int txtWidth = MeasureText(buffer, 20);
    int x = (int)(layout.x + (layout.width - (float)txtWidth) / 2.0f);
    int y = (int)(layout.y + (layout.height - 20) / 2.0f);

    DrawText(buffer, x, y, 20, GRAY);
}

void TooltipMsg(Rectangle rec, const char *msg) {
    if (!msg || msg[0] == '\0') return;

    float fontSize = BTN_FONTSIZE, padding = GAP;
    int txtWidth = MeasureText(msg, (int)fontSize);

    Rectangle tooltipRec = {
        rec.x - (float)txtWidth - (padding * 2) - fontSize,
        rec.y + (rec.height / 2) - (fontSize / 2.0f) - padding,
        (float)txtWidth + (padding * 2),
        (float)fontSize + (padding * 2)
    };

    // Desenha Fundo
    DrawRectangleRec(tooltipRec, GetColor(0x222222FF)); // Cinza muito escuro
    DrawRectangleLinesEx(tooltipRec, 1, GetColor(0x666666FF)); // Borda sutil

    DrawText(
        msg, (int)(tooltipRec.x + padding),
        (int)(tooltipRec.y + padding), (int)fontSize, RAYWHITE
    );
}
