#include <M5Unified.h>
#include <MIDI.h>

#include "app/midi.h"

static bool prevPressed[15] = {false};
static bool firstDraw = true;

void resetNotes()
{
    firstDraw = true;
}

void drawNotes(const Notes15& notes15, const int startY, const int width, const int height, const int spacing)
{
    // Calculate square size to fit 5 columns with spacing in given area
    const int availableWidth = width - (spacing * 6); // 6 spaces: left, 4 between, right
    const int availableHeight = height - (spacing * 4); // 4 spaces: top, 2 between, bottom
    const int squareSize = min(availableWidth / 5, availableHeight / 3);

    // Calculate starting position to center the grid in given area
    const int totalGridWidth = (squareSize * 5) + (spacing * 4);
    const int totalGridHeight = (squareSize * 3) + (spacing * 2);
    const int startX = (width - totalGridWidth) / 2;
    const int gridStartY = startY + (height - totalGridHeight) / 2;

    for (int i = 0; i < 15; i++) {
        const bool isPressed = notes15.get(i) > 0;

        if (firstDraw || isPressed != prevPressed[i]) {
            const int col = i % 5;
            const int row = i / 5;
            const int x = startX + col * (squareSize + spacing);
            const int y = gridStartY + row * (squareSize + spacing);

            const uint16_t fillColor = isPressed ? TFT_WHITE : TFT_BLACK;
            constexpr uint16_t borderColor = TFT_DARKGRAY;

            M5.Display.fillRect(x, y, squareSize, squareSize, fillColor);
            M5.Display.drawRect(x, y, squareSize, squareSize, borderColor);

            prevPressed[i] = isPressed;
        }
    }

    firstDraw = false;
}

void drawButtons(const int startY, const int width, const int height, const bool withSelect)
{
    const int buttonWidth = width / 3;

    // Draw three button frames
    for (int i = 0; i < 3; i++) {
        const int x = i * buttonWidth;
        M5.Display.drawRect(x + 2, startY, buttonWidth - 2, height, TFT_DARKGRAY);
    }

    const int y = startY + height / 2;
    const int size = height / 4;

    // Button B
    const int xb = buttonWidth + buttonWidth / 2;
    M5.Display.fillTriangle(
        xb, y + size / 2, // Bottom point
        xb - size, y - size / 2, // Top left
        xb + size, y - size / 2, // Top right
        TFT_WHITE
    );

    if (withSelect) {
        // Button A
        const int xa = buttonWidth / 2;
        M5.Display.fillTriangle(
            xa - size / 2, y, // Left point
            xa + size / 2, y - size, // Top right
            xa + size / 2, y + size, // Bottom right
            TFT_WHITE
        );

        // Button C
        const int xc = 2 * buttonWidth + buttonWidth / 2;
        M5.Display.fillTriangle(
            xc + size / 2, y, // Right point
            xc - size / 2, y - size, // Top left
            xc - size / 2, y + size, // Bottom left
            TFT_WHITE
        );
    }
}
