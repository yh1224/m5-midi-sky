#include <M5Unified.h>

#include "app/display.h"
#include "app/midi.h"

static bool prevPressed[15] = {false};

void resetDisplay(const bool settingsMode)
{
    M5.Display.fillScreen(TFT_BLACK);
    drawButtons(208, M5.Display.width(), 32, settingsMode, settingsMode);
}

void drawNotes(const Notes& notes, const int startY, const int width, const int height, const int spacing, const bool firstDraw)
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
        const bool isPressed = notes.get(i) > 0;

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
}

void drawButtons(const int startY, const int width, const int height, const bool buttonA, const bool buttonC)
{
    const int buttonWidth = width / 3;

    // Draw three button frames
    for (int i = 0; i < 3; i++) {
        const int x = i * buttonWidth;
        M5.Display.drawRect(x + 2, startY, buttonWidth - 2, height, TFT_DARKGRAY);
    }

    const int y = startY + height / 2;
    const int size = height / 4;

    // Button A
    if (buttonA) {
        const int xa = buttonWidth / 2;
        M5.Display.fillTriangle(
            xa - size / 2, y, // Left point
            xa + size / 2, y - size, // Top right
            xa + size / 2, y + size, // Bottom right
            TFT_WHITE
        );
    }

    // Button B
    const int xb = buttonWidth + buttonWidth / 2;
    M5.Display.fillTriangle(
        xb, y + size / 2, // Bottom point
        xb - size, y - size / 2, // Top left
        xb + size, y - size / 2, // Top right
        TFT_WHITE
    );

    // Button C
    if (buttonC) {
        const int xc = 2 * buttonWidth + buttonWidth / 2;
        M5.Display.fillTriangle(
            xc + size / 2, y, // Right point
            xc - size / 2, y - size, // Top left
            xc - size / 2, y + size, // Bottom left
            TFT_WHITE
        );
    }
}

void drawKeyboard(const int startY, const int width, const int height, const int baseNote)
{
    const int blackKeyHeight = height * 3 / 5;

    // White keys
    const int whiteKeyNotes[] = {
        0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23, 24, 26, 28, 29, 31, 33, 35,
    };
    constexpr int numWhiteKeys = std::size(whiteKeyNotes);

    // Black keys
    const int blackKeyNotes[] = {
        1, 3, 6, 8, 10, 13, 15, 18, 20, 22, 25, 27, 30, 32, 34,
    };
    // Black key positions relative to white keys
    const int blackKeyPositions[] = {
        0, 1, 3, 4, 5, 7, 8, 10, 11, 12, 14, 15, 17, 18, 19, 21,
    };
    constexpr int numBlackKeys = std::size(blackKeyNotes);

    // Valid keys (C3 to C5)
    const int validKeyNotes[] = {
        0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23, 24,
    };

    const int whiteKeyWidth = width / numWhiteKeys;
    const int blackKeyWidth = whiteKeyWidth * 2 / 3;

    int base = baseNote;
    while (base >= 12) {
        base -= 12;
    }
    bool activeNotes[36] = {false};
    for (const int validKeyNote : validKeyNotes) {
        activeNotes[base + validKeyNote] = true;
    }

    for (int i = 0; i < numWhiteKeys; i++) {
        const int note = whiteKeyNotes[i];
        const int x = i * whiteKeyWidth;
        uint16_t color = activeNotes[note] ? TFT_CYAN : TFT_WHITE;
        M5.Display.fillRect(x, startY, whiteKeyWidth - 1, height, color);
        M5.Display.drawRect(x, startY, whiteKeyWidth - 1, height, TFT_BLACK);
    }
    for (int i = 0; i < numBlackKeys; i++) {
        const int pos = blackKeyPositions[i];
        const int note = blackKeyNotes[i];
        const int x = pos * whiteKeyWidth + whiteKeyWidth - blackKeyWidth / 2;
        uint16_t color = activeNotes[note] ? TFT_CYAN : TFT_BLACK;
        M5.Display.fillRect(x, startY, blackKeyWidth, blackKeyHeight, color);
        M5.Display.drawRect(x, startY, blackKeyWidth, blackKeyHeight, TFT_BLACK);
    }
}
