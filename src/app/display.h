#if !defined(APP_DISPLAY_H)
#define APP_DISPLAY_H

#include "app/notes.h"

void resetDisplay(bool settingsMode);

void drawNotes(const Notes& notes, int startY, int width, int height, int spacing, bool firstDraw);

void drawButtons(int startY, int width, int height, bool buttonA, bool buttonC);

void drawKeyboard(int startY, int width, int height, int baseNote);

#endif // !defined(APP_DISPLAY_H)
